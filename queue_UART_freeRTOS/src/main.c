#include "TM4C123GH6PM.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdint.h>
#include <string.h>

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

    //enable interrupts
    UART5->ICR = 0xFFF;        /* clear all pending interrupts        */
    UART5->IM  = 0x50;         /* enable RX + receive-timeout (bits 4,6) */

    NVIC_SetPriority(UART5_IRQn, 6);
    NVIC_EnableIRQ(UART5_IRQn);


}
/*use uart0 to print on the terminal*/
void uart0_init(void)
{
    // Enable UART0 and GPIOA clocks
    SYSCTL->RCGCUART |= (1 << 0);   // UART0
    SYSCTL->RCGCGPIO |= (1 << 0);   // GPIOA
    for(volatile int i = 0; i < 100; i++);
    
    UART0->CTL  = 0;           // disable UART before config
    UART0->IBRD = 104;         // 9600 baud @ 16MHz
    UART0->FBRD = 11;
    UART0->CC   = 0;
    UART0->LCRH = 0x60;        // 8 bits, no parity, 1 stop
    UART0->CTL  = 0x301;       // enable TX + RX + UART

    // Configure PA0, PA1 as UART
    GPIOA->DEN   |= 0x03;      // PA0, PA1 digital enable
    GPIOA->AFSEL |= 0x03;      // alternate function
    GPIOA->AMSEL &= ~0x03;     // disable analog
    GPIOA->PCTL  = (GPIOA->PCTL & 0xFFFFFF00) | 0x00000011; // U0RX, U0TX
}
/*functions to print debug strings*/
// print a single char
void uart0_send_char(char c)
{
    while(UART0->FR & (1 << 5));  // wait if TX FIFO full
    UART0->DR = c;
}

// print a string
void uart0_print(const char *str)
{
    while(*str)
    {
            uart0_send_char(*str);
        str++;
    }
}
//uart5 isr handler
void UART5_IRQHandler(){
    BaseType_t xHigherPriorityTaskWoken = pdFALSE ; 

    //save interrupt
    uint32_t status = UART5->MIS ;
    //check if rx fifo or receive timeout
    if (status & 0x50)
    {
        //read data and enqueue it
        while (!(UART5->FR & (1 << 4))) //wait until RX FIFO not empty
        {
            //enqueue the data
            char c = (char)(UART5->DR & 0xFF);
            xQueueSendFromISR(xRxQueue, &c, &xHigherPriorityTaskWoken);

        }
        UART5->ICR = 0x50 ; //clear interrupt
    }
    //if TX interrupt is fired
    if(status & 0x20){
        char c;
        while (!(UART5->FR & (1 << 5)))
        {
            //
            if (xQueueReceiveFromISR(xTxQueue, &c, &xHigherPriorityTaskWoken) == pdTRUE)
            {
                UART5->DR = c;
            }else
            {
                /*disable TX interrupt */
                UART5->IM &= ~(1 << 5);
                break;
            }   
            
        }
        UART5->ICR = 0x20 ;
              
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    
}

//function to send string and enable TX
static void uart5_send_string(const char *str){
    //enqueue the data
    while (*str)
    {
        //enqueue
        xQueueSend(xTxQueue, (void*)str, portMAX_DELAY);
        str++;
    }
   
    
    /* manually write first char to kick TX transition */
    char c;
    if (xQueueReceive(xTxQueue, &c, 0) == pdTRUE)
    {
        //while(UART5->FR & (1 << 5));
        UART5->DR = c;
        UART5->IM |= (1 << 5);   /* enable TX interrupt for remaining chars */
    }
}

/*waits for sim800l response*/
/*reads XRxQueue until expected string is found or timeout*/

static BaseType_t sim800l_wait_response(char *buf,
                                         uint8_t bufsize,
                                         const char *expected,
                                         uint32_t timeoutMs)
{
    uint8_t index = 0;
    char data = '\0';
    TickType_t timeout = pdMS_TO_TICKS(timeoutMs);

    memset(buf, 0, bufsize);

    while(index < bufsize - 1)
    {
        if(xQueueReceive(xRxQueue, &data, timeout) == pdTRUE)
        {
            buf[index++] = data;
            buf[index]   = '\0';

            // show raw bytes arriving on terminal for debugging
            // uart0_print("RX: ");
            // uart0_print(buf);
            // uart0_print("\r\n");

            if(strstr(buf, expected) && strstr(buf, "\r\n"))
            {
                return pdTRUE;
            }
        }
        else
        {
            uart0_print("TIMEOUT\r\n");
            return pdFALSE;
        }
    }
    return pdFALSE;
}

/*helper function to read dummy response from the queue*/
void flush_rx_queue(void)
{
    char dummy;
    while(xQueueReceive(xRxQueue, &dummy, 0) == pdTRUE);
}
/*SIM800l task*/
void sim800l_task(void *pvParameters)
{
    char response[128];

    uart0_print("Task started\r\n");

    vTaskDelay(pdMS_TO_TICKS(4000));

    uart0_print("SIM800L boot delay done\r\n");

    while(1)
    {
        /* ── AT handshake ───────────────────────────────────────── */
        uart0_print("Sending AT...\r\n");
        flush_rx_queue();
        uart5_send_string("AT\r\n");

        if(sim800l_wait_response(response, sizeof(response), "OK", 4000)
                == pdTRUE)
        {
            uart0_print(">> AT OK\r\n");
        }
        else
        {
            uart0_print(">> AT failed - retrying\r\n");
            vTaskDelay(pdMS_TO_TICKS(2000));
            continue;
        }

        /* ── SIM card ───────────────────────────────────────────── */
        uart0_print("Checking SIM...\r\n");
        flush_rx_queue();
        uart5_send_string("AT+CPIN?\r\n");

        if(sim800l_wait_response(response, sizeof(response), "READY", 2000)
                == pdTRUE)
        {
            uart0_print(">> SIM Ready\r\n");
        }
        else
        {
            uart0_print(">> SIM Not Ready\r\n");
        }

        /* ── Signal strength ────────────────────────────────────── */
        uart0_print("Checking signal...\r\n");
        flush_rx_queue();
        uart5_send_string("AT+CSQ\r\n");

        if(sim800l_wait_response(response, sizeof(response), "OK", 2000)
                == pdTRUE)
        {
            uart0_print(">> Signal: ");
            uart0_print(response);
            uart0_print("\r\n");
        }
        else
        {
            uart0_print(">> Signal check failed\r\n");
        }

        /* ── Network registration ───────────────────────────────── */
        uart0_print("Checking network...\r\n");
        flush_rx_queue();
        uart5_send_string("AT+CREG?\r\n");

        if(sim800l_wait_response(response, sizeof(response), "OK", 2000)
                == pdTRUE)
        {
            uart0_print(">> Network: ");
            uart0_print(response);
            uart0_print("\r\n");
        }
        else
        {
            uart0_print(">> Network check failed\r\n");
        }
        /*get sim CCCID*/
        uart0_print("checking CCID\r\n");
        flush_rx_queue();
        uart5_send_string("AT+CCID\r\n");

        if(sim800l_wait_response(response, sizeof(response), "OK", 2000) == pdTRUE)
        {
            uart0_print(">> IMEI: ");
            uart0_print(response);
            uart0_print("\r\n");
        }
        else
        {
            uart0_print(">> IMEI get failed\r\n");
        }

        /* check operator*/
        uart0_print("checking operator\r\n");
        flush_rx_queue();
        uart5_send_string("AT+COPS?\r\n");

        if(sim800l_wait_response(response, sizeof(response), "OK", 2000) == pdTRUE)
        {
            uart0_print(">> operator: ");
            uart0_print(response);
            uart0_print("\r\n");
        }
        else
        {
            uart0_print(">> operator get failed\r\n");
        }
        

        uart0_print("---- cycle done ----\r\n");
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}


/* ------------------------------------------------------------------ */
/*  main                                                                */
/* ------------------------------------------------------------------ */
int main(void)
{
    gpio_init();
    uart0_init();
    uart5_init();

    xTxQueue = xQueueCreate(64, sizeof(char));
    xRxQueue = xQueueCreate(64, sizeof(char));

    // ── BARE BLOCKING TEST — no FreeRTOS involved ──
    // manually send a string before scheduler starts
    const char *test = "BOOT OK\r\n";
    while(*test)
    {
        while(UART5->FR & (1 << 5));  // wait if TX FIFO full
        UART5->DR = *test++;
    }
    // if you see "BOOT OK" on terminal → UART5 hardware works
    // if you see nothing → wiring or baud rate problem

    xTaskCreate(sim800l_task, "sim800l", 512, NULL, 3, NULL);
    vTaskStartScheduler();

    while(1);
}
