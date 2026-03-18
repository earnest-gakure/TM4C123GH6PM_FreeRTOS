#include "TM4C123GH6PM.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <cstdio>
#include <stdbool.h>
#include <stdint.h>


#define RED 0x02
#define BLUE 0x04
#define GREEN 0x08
#define B1 0x01
#define B2 0x10
#define NUM 2

#define TX2 (1 << 4)
#define RX2 (1 << 5)
#define UART_QUEUE_LN 64

QueueHandle_t my_queue;
QueueHandle_t xRxQueue;
QueueHandle_t xTxQueue;

void gpio_init(){
    SYSCTL->RCGCGPIO |= (1<<5);
    GPIOF->LOCK = 0x4C4F434B;
    GPIOF->CR   = 0xFF;
    
    //make GPIOF 0 & 4 INPUT pins and 1,2,3 output pins
    GPIOF->DIR |=RED | BLUE | GREEN;
    GPIOF->DEN |=( RED | BLUE | GREEN | B1 | B2) ;
    
    //enable internal pullup
    GPIOF->PUR = (B1 |B2 ); //= ~0x11
                            //
    //mask port so interrupts can be sent to interrupt controller
    GPIOF->IM  &= ~(B1 | B2);
    //enable interrupt sense
    GPIOF->IS  &=~(B1 | B2); //= ~0x11
    //disable both edges from triggering interrupt
    GPIOF->IBE &= ~(B1 | B2);
    //select falling edge interrupt event
    GPIOF->IEV &= ~(B1 | B2);
    //clear interrupts
    GPIOF->ICR  = (B1 | B2);
    //unmask the port
    GPIOF-> IM  = (B1 |B2);

    //enable GPIOF interrupt in NVIC (IRD 30)
    NVIC_SetPriority(GPIOF_IRQn,6);
    NVIC_EnableIRQ(GPIOF_IRQn);



}
void uart_gpio_init(void){
    //enable clock for GPIOA
    SYSCTL->RCGCGPIO |= (1 << 1) ; 
    SYSCTL->RCGCUART |= (1 << 2) ;       // enable clock for UART 2
    GPIOB->DIR |= TX2;                   //make tx pin output pin, rx0 input by defailt
    GPIOB->DEN |= (TX2 | RX2);
    GPIOB->AFSEL |= (TX2 | RX2);        // enable alternate function
    GPIOB->PCTL |= (1 << 16) | (1 <<17); //set PB4 and PB5 to UART

    //UART set up
    UART2->UARTCTL &= ~(1 << 0);         //disable UART
    UART2->IBRD = 104;                   //integer baude rate - 9600 BAUD RATE
    UART2->FBRD = 11;                    //fraction baude rate
    UART2->LCRH |= 0xE0;                 //8-bit, no parity, FIFO
    UART2->CC = 0x0;                     //SYSTEM CLOCK SELECT
   
  //uart interrupt -> RX 1/8 full, TX 7/8 empty
    UART2->IFLS =0x00;
    //enable rx and receive-timeout interrupt only
    //Tx interrupt enabled later when we have data to send
    UART2->IM = 0x50;
    UART2->ICR = 0xFFF;                 //Clear all interruts
    UART2->CTL = 0x301;                 //enable RX,TX,UART
                                        //
    NVIC_SetPriority(UART2_IRQ,33);
    NVIC_EnableIRQ(UART2_IRQ);
}
void UART2_IRQHandler(void){

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    uint32_t status = UART2->MIS;
    //check if rx FIFO has data
    if(status & (0x50)){
        //read buffer

        //clear interrupt
        UART2->ICR = 50;
    }
    if(status & (0x80)){
        //send data
        
        //clear buffer
        UART2->ICR = 0x80;
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
//function to send string data to TX queue
static void uart_send_string(const char *str){
    while(*str){
        //ensure whole string is sent to queue
        xQueueSend(xTxQueue, str, portMAX_DELAY);
        str++;
    }
    //mask Tx interrupt
    UART2->IM |= 0X80;
}



int main(void)
{
    //initialize gpio pins
    gpio_init();
    uart_gpio_init();
    my_queue = xQueueCreate(5, sizeof( uint8_t));

    xTaskCreate(led_blink, "led blink", 256, NULL, 1, NULL);
    vTaskStartScheduler();
    
    while (1);
}
