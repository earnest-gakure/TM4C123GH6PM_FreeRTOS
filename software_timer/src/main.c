/**
 * blinking an led using a software timer
*/

#include "TM4C123GH6PM.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include <time.h>

//timer 
BaseType_t xtimerStarted;
static TimerHandle_t xtimer;
    
void gpio_init(){
    SYSCTL->RCGCGPIO |= (1<<5);
    GPIOF->LOCK = 0x4C4F434B;
    GPIOF->CR   = 0xFF;

    GPIOF->DIR |= (1<<1) | (1<<2) | (1<<3);
    GPIOF->DEN |= (1<<1) | (1<<2) | (1<<3);
    GPIOF->DATA &= ~( (1<<1) | (1<<2) | (1<<3));

}
/* callback function*/
void vtimer_callback(TimerHandle_t xTimer)
{
    /* Toggle the LED */
    GPIOF->DATA ^= (1<<1); // Toggle the red LED 
}

int main(void)
{
    gpio_init();
    //create timer
    xtimer = xTimerCreate("led timer", pdMS_TO_TICKS(3000), pdTRUE, 0, vtimer_callback);
    if(xtimer != NULL){
        //start timer
        xtimerStarted = xTimerStart(xtimer, 0);
    }
    vTaskStartScheduler();
    while (1);
}
