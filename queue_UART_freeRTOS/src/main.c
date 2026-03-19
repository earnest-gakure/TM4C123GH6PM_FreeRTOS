#include "TM4C123GH6PM.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdint.h>

QueueHandle_t xTxQueue;
QueueHandle_t xRxQueue;

void uart5_send_char(char c); 
/* ------------------------------------------------------------------ */
/*  GPIO F init (red LED for debug)                                     */
/* ------------------------------------------------------------------ */
void gpio_init(void)
{
    SYSCTL->RCGCGPIO |= (1 << 5);
    for(volatile int i = 0; i < 100; i++);
    GPIOF->LOCK = 0x4C4F434B;
    GPIOF->CR   = 0xFF;
    GPIOF->DIR |= 0x02;
    GPIOF->DEN |= 0x02;
}

/* ------------------------------------------------------------------ */
/*  UART5 init — PE4 (TX), PE5 (RX)                                    */
/* ------------------------------------------------------------------ */
void uart5_init(void)
{
    SYSCTL->RCGCUART |= 0x20;
    SYSCTL->RCGCGPIO |= 0x10;
    for(volatile int i = 0; i < 100; i++);

    UART5->CTL  = 0;
    UART5->IBRD = 104;
    UART5->FBRD = 11;
    UART5->CC   = 0;
    UART5->LCRH = 0x60;
    UART5->CTL  = 0x301;

    GPIOE->DEN   = 0x30;
    GPIOE->AFSEL = 0x30;
    GPIOE->AMSEL = 0;
    GPIOE->PCTL  = 0x00110000;

    UART5->ICR = 0xFFF;        /* clear all pending interrupts        */
    UART5->IM  = 0x50;         /* enable RX + receive-timeout (bits 4,6) */

    NVIC_SetPriority(UART5_IRQn, 6);
    NVIC_EnableIRQ(UART5_IRQn);
}

/* ------------------------------------------------------------------ */
/*  UART5 ISR — handles both TX and RX                                  */
/* ------------------------------------------------------------------ */
void UART5_IRQHandler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    uint32_t status = UART5->MIS;

    /* --- RX / receive-timeout --- */
    if (status & 0x50)
    {
        while(!(UART5->FR & (1 << 4)))   /* while RX FIFO not empty */
        {
            char c = (char)(UART5->DR & 0xFF);
            xQueueSendFromISR(xRxQueue, &c, &xHigherPriorityTaskWoken);
        }
        UART5->ICR = 0x50;
    }

    /* --- TX --- */
    if (status & 0x20)
    {
        char c;
        while(!(UART5->FR & (1 << 5)))   /* while TX FIFO not full */
        {
            if (xQueueReceiveFromISR(xTxQueue, &c, &xHigherPriorityTaskWoken) == pdTRUE)
            {
                UART5->DR = c;
            }
            else
            {
                UART5->IM &= ~(1 << 5);  /* queue empty — disable TX interrupt */
                break;
            }
        }
        UART5->ICR = 0x20;
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/* ------------------------------------------------------------------ */
/*  Send string — push first char directly to kick TX, ISR handles rest */
/* ------------------------------------------------------------------ */
static void uart_send_string(const char *str)
{
    while(*str)
    {
        xQueueSend(xTxQueue, (void*)str, portMAX_DELAY);
        str++;
    }

    /* manually write first char to kick TX transition */
    char c;
    if (xQueueReceive(xTxQueue, &c, 0) == pdTRUE)
    {
        while(UART5->FR & (1 << 5));
        UART5->DR = c;
        UART5->IM |= (1 << 5);   /* enable TX interrupt for remaining chars */
    }
}

/* ------------------------------------------------------------------ */
/*  Sender task — transmits "Ping N" every second                       */
/* ------------------------------------------------------------------ */
void sender_task(void *pvParameters)
{
    int  count = 0;
    char msg[32];

    while(1)
    {
        msg[0]='P'; msg[1]='i'; msg[2]='n'; msg[3]='g';
        msg[4]=' '; msg[5]='0'+(count % 10);
        msg[6]='\r'; msg[7]='\n'; msg[8]='\0';

        uart_send_string(msg);
        count++;
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/* ------------------------------------------------------------------ */
/*  Receiver task — reads from RX queue, echoes back with prefix       */
/* ------------------------------------------------------------------ */
void receiver_task(void *pvParameters)
{
    char c;
    char line[64];
    int  idx = 0;

    while(1)
    {
        if (xQueueReceive(xRxQueue, &c, portMAX_DELAY) == pdTRUE)
        {
            if (c == '\r' || c == '\n')
            {
                line[idx] = '\0';
                idx = 0;

                /* direct blocking send — no queue, no conflict */
                uart5_send_char('\r');
                uart5_send_char('\n');
                const char *prefix = "Received: ";
                while(*prefix) uart5_send_char(*prefix++);
                const char *p = line;
                while(*p) uart5_send_char(*p++);
                uart5_send_char('\r');
                uart5_send_char('\n');
            }
            else if (idx < (int)sizeof(line) - 1)
            {
                line[idx++] = c;
            }
        }
    }
}

void uart5_send_char(char c)
{
    while(UART5->FR & 0x20);   /* wait while TX FIFO full */
    UART5->DR = c;
}

/* ------------------------------------------------------------------ */
/*  main                                                                */
/* ------------------------------------------------------------------ */
int main(void)
{
    gpio_init();
    uart5_init();

    xTxQueue = xQueueCreate(64, sizeof(char));
    xRxQueue = xQueueCreate(64, sizeof(char));

    xTaskCreate(sender_task,   "TX", 256, NULL, 2, NULL);
    xTaskCreate(receiver_task, "RX", 256, NULL, 1, NULL);

    vTaskStartScheduler();

    while(1);
}
