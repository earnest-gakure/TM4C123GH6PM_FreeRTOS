#include "TM4C123GH6PM.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdbool.h>


#define RED 0x02
#define BLUE 0x04
#define GREEN 0x08
#define B1 0x01
#define B2 0x10
#define NUM 2

QueueHandle_t my_queue;

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


//function to read queue value and turn ON/OFF led 
void led_blink(void *pvparameter){
    uint8_t isr_status = 0;
    /* Default: green on */
    GPIOF->DATA &= ~(RED|BLUE|GREEN);
    GPIOF->DATA |= GREEN;
    while(1){
    
        if(xQueueReceive(my_queue, &isr_status, (TickType_t)2000)){
            if(isr_status == 1){
                GPIOF->DATA &= ~(RED|BLUE|GREEN);
                GPIOF->DATA |= RED; ;
            }
            else if(isr_status == 2){
                GPIOF->DATA &= ~(RED|BLUE|GREEN);
                GPIOF->DATA |= BLUE;
            }
        }else{
                GPIOF->DATA = GREEN;
         }
    }

}

void GPIOF_IRQHandler(void){
      uint8_t isr_status = 0;
      BaseType_t xHigherPriorityTaskWoken = pdFALSE;
      if(GPIOF->MIS & B1){ // GPIOF->RIS & B1
             isr_status = 1;
             //clear interrupt
             GPIOF->ICR = B1 ;
             xQueueSendFromISR(my_queue, &isr_status, &xHigherPriorityTaskWoken);
        }   
      if(GPIOF->MIS & B2 ){// GPIOF->RIS & B2
             isr_status = 2;
             //clear isr
             GPIOF->ICR = B2;
             xQueueSendFromISR(my_queue, &isr_status, &xHigherPriorityTaskWoken);
        }
     portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
      

}

int main(void)
{
    //initialize gpio pins
    gpio_init();
    my_queue = xQueueCreate(5, sizeof( uint8_t));

    xTaskCreate(led_blink, "led blink", 256, NULL, 1, NULL);
    vTaskStartScheduler();
    
    while (1);
}
