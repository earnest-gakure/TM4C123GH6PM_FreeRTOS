/**
 * creating a traffic lights controller using software timers
 * 
*/

#include "TM4C123GH6PM.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

//handles for the timer
static TimerHandle_t xtimer1, xtimer2, xtimer3;

//gpio initallization
void gpio_init(){
    SYSCTL->RCGCGPIO |= (1<<5);
    GPIOF->LOCK = 0x4C4F434B;
    GPIOF->CR   = 0xFF;
    GPIOF->PUR |= (1<<0) | (1<<4); //enable pul-up resister

    GPIOF->DIR |= (1<<1) | (1<<2) | (1<<3);
    GPIOF->DEN |=  (1<<0) |(1<<1) | (1<<2) | (1<<3) | (1<<4);
    GPIOF->DATA &= ~( (1<<1) | (1<<2) | (1<<3));

}
//callback function for th timer
void timer_callback(TimerHandle_t xTimer){
    //read timer id
    uint32_t timer_id = (uint32_t)pvTimerGetTimerID(xTimer);
    switch (timer_id)
    {
    case 1:
        //red LED on 
        GPIOF->DATA = (1<<1);
        //start blue time
        xTimerStart(xtimer2, 0);

        break;
    case 2:
        //blue led on
        GPIOF->DATA = (1<<2);
        //start green timer
        xTimerStart(xtimer3, 0);
        break;
    case 3:
        //green led on
        GPIOF->DATA = (1<<3);
        //start red timer
        xTimerStart(xtimer1, 0);
        break;
    default:
        break;
    }
}


int main(void)
{
    gpio_init();
    
    xtimer1 = xTimerCreate("timer1", pdMS_TO_TICKS(5000),pdFALSE, (void *)1, timer_callback);
    xtimer2 = xTimerCreate("timer2", pdMS_TO_TICKS(4000),pdFALSE, (void *)2, timer_callback);
    xtimer3 = xTimerCreate("timer3", pdMS_TO_TICKS(3000),pdFALSE, (void *)3, timer_callback);

    //start red timer
    xTimerStart(xtimer1, 0);
    vTaskStartScheduler();
    while (1);
}


