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
        uart0_send_char(*str++);
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
static void uart_send_string(const char *str){
    //enqueue the data
    while (*str)
    {
        //enqueue
        xQueueSend(xTxQueue, (void*)str, portMAX_DELAY);
        str++;
    }
    // //enable TX interrupt
    // UART5->IM |= (1 << 5);
    
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

static BaseType_t sim800l_wait_response(char *responseBuffer, uint8_t bufsize, char *expectedrespBuf, uint32_t timeoutMs){
    uint8_t index = 0;
    char data = 0;
    TickType_t timeout = pdMS_TO_TICKS(timeoutMs) ; // convert ms to tick time
    memset(responseBuffer, 0, bufsize);

    while (index < (bufsize - 1))
    {
        if (xQueueReceive(xRxQueue, &data, timeout) == pdTRUE)
        {
            responseBuffer[index++] = data;
            responseBuffer[index]   = '\0';
            if (strstr(responseBuffer, expectedrespBuf) != NULL)
            {
                return pdTRUE;       // found expected response
            }
            
        }
        else{
            return pdFALSE;         // timout
        }
        
    }
    return pdFALSE;
    
}

void sim800l_task(void *pvParameters){

    char response[128];
    vTaskDelay(pdMS_TO_TICKS(4000));
    //send AT commands and wait for response for each of the AT command

    while(1){
        uart_send_string("AT\r\n");
        if (sim800l_wait_response(response, sizeof(response), "OK", 2000) == pdTRUE)
        {
            uart0_print("AT OK\r\n");
        }else
        {
            uart0_print("No response\r\n");
            vTaskDelay(pdMS_TO_TICKS(3000));
            continue;
            
        }

        uart_send_string("AT+CPIN?\r\n");
        if (sim800l_wait_response(response, sizeof(response), "OK", 2000) == pdTRUE)
        {
            uart0_print("SIM Ready\r\n");
        }else
        {
            uart0_print("SIM Not Ready\r\n");
        }

        vTaskDelay(pdMS_TO_TICKS(15000));
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

    

    xTaskCreate(sim800l_task, "sim800l", 512 , NULL, 3, NULL);

    vTaskStartScheduler();

    while(1);
}
