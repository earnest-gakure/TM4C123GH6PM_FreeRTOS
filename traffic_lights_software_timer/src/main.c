/**
 * debounce button and toggle LED using software timer
*/

#include "TM4C123GH6PM.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"


  
void gpio_init(){
    SYSCTL->RCGCGPIO |= (1<<5);
    GPIOF->LOCK = 0x4C4F434B;
    GPIOF->CR   = 0xFF;
    GPIOF->PUR |= (1<<0) | (1<<4); //enable pul-up resister

    GPIOF->DIR |= (1<<1) | (1<<2) | (1<<3);
    GPIOF->DEN |=  (1<<0) |(1<<1) | (1<<2) | (1<<3) | (1<<4);
    GPIOF->DATA &= ~( (1<<1) | (1<<2) | (1<<3));

}


int main(void)
{
    gpio_init();
    
    vTaskStartScheduler();
    while (1);
}


