/**
 * 3 "jobs", each represented by a timer. 
 * Each timer toggles an LED at its assigned period. 
 * Pressing SW1 cycles through 3 priority modes that change how fast each job 
 * runs by adjusting their periods with xTimerChangePeriod()
 * SW2 resets everything back to Mode 0.
*/

#include "TM4C123GH6PM.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

//handles for the timer
static TimerHandle_t xtimer1, xtimer2, xtimer3, xtimer5, xtimer6;
uint32_t button1 =1, button2 =1;

//enum type for the 3 states
typedef enum{
    NORMAL,
    BOOST1,
    BOOST2
} states;
states current_state = NORMAL;

//functions
void change_timer_period(int timer1, int timer2, int timer3);
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
//callback function for sw1 and sw2 timer
void switch_callback(TimerHandle_t xTimer){
    uint32_t timer_id = (uint32_t)pvTimerGetTimerID(xTimer);
    switch (timer_id)
    {   case 5:
            if ((GPIOF->DATA & (1<<4)) == 0)
            {
                if (current_state == NORMAL)
                {
                    current_state = BOOST1;
                }else if (current_state == BOOST1)
                {
                    current_state = BOOST2;
                }else if (current_state == BOOST2)
                {
                    current_state = NORMAL;
                }
            }
            //change current state respectively
            switch (current_state)
            {
                case  0:
                    //noraml state
                    change_timer_period(500, 1000, 1500);
                    
                    break;
                case  1:
                    //boost1 state
                    change_timer_period(200, 2000, 3000);
                    
                    break;
                case  2:
                    //boost2 state
                    change_timer_period(2000, 2000, 2000);
                    
                    break;
            }
            break;
        
        case 6:
            if ((GPIOF->DATA & (1<<0)) == 0)
            {
                current_state = NORMAL;
            }      
            //change current state respectively
            switch (current_state)
            {
                case  0:
                    //noraml state
                    change_timer_period(2000, 2000, 2000);
                    
                    break;
                case  1:
                    //boost1 state
                    change_timer_period(500, 1000, 1500);
                    
                    break;
                case  2:
                    //boost2 state
                    change_timer_period(500, 2000, 2000);
                    
                    break;
            }
            break;
    }
}
//callback function for th timer
void jobs_callback(TimerHandle_t xTimer){
    //read timer id
    GPIOF->DATA &= ~((1<<1) | (1<<2) | (1<<3));
    uint32_t timer_id = (uint32_t)pvTimerGetTimerID(xTimer);
    switch (timer_id)
    {
        case 1:
            //toggle RED LED
            GPIOF->DATA ^= (1<<1);
            xTimerStop(xtimer1,0);
            xTimerStart(xtimer2,0);
            break;
        case 2:
            //toggle blue LED
            GPIOF->DATA ^= (1<<2);
            xTimerStop(xtimer2,0);
            xTimerStart(xtimer3,0);
            break;
        case 3:
            //toggle GREEN LED
            GPIOF->DATA ^= (1<<3);
            xTimerStop(xtimer3,0);
            xTimerStart(xtimer1,0);
            break;
            
         
    }
}
//a task that detects and responds to sw1 and sw2 press
void vswitch_task(void * pvParameter){
    
    while(1){
        //poll button state
        if (((GPIOF->DATA & (1<<4)) == 0 ) && (button1 == 1))
        {
            button1 = 0;
            xTimerReset(xtimer5,0);
        }else if (((GPIOF->DATA & (1<<4)) != 0))
        {
            button1 = 1;
        }
        
        //poll button 2
        if (((GPIOF->DATA & (1<<0)) == 0) && (button2 == 1))
        {
            button2 = 0;
            xTimerReset(xtimer6,0);
        }else if (((GPIOF->DATA & (1<<0)) !=0))
        {
            button2 = 1;
        }
        
        

        vTaskDelay(pdMS_TO_TICKS(20));
    }

}
void change_timer_period(int timer1, int timer2, int timer3){
//boost1 state
    xTimerChangePeriod(xtimer1, pdMS_TO_TICKS(timer1), 0);
    xTimerChangePeriod(xtimer2, pdMS_TO_TICKS(timer2), 0);
    xTimerChangePeriod(xtimer3, pdMS_TO_TICKS(timer3), 0);
    
}

int main(void)
{
    gpio_init();
    
    //job timers
    xtimer1 = xTimerCreate("timer1", pdMS_TO_TICKS(500),pdFALSE, (void *)1, jobs_callback);
    xtimer2 = xTimerCreate("timer2", pdMS_TO_TICKS(1000),pdFALSE, (void *)2, jobs_callback);
    xtimer3 = xTimerCreate("timer3", pdMS_TO_TICKS(1500),pdFALSE, (void *)3, jobs_callback);
    
    xtimer5 = xTimerCreate("timer5", pdMS_TO_TICKS(50),pdFALSE, (void *)5, switch_callback);
    xtimer6 = xTimerCreate("timer6", pdMS_TO_TICKS(50),pdFALSE, (void *)6, switch_callback);

    
    //start timers
    xTimerStart(xtimer1,0);
    //xTimerStart(xtimer2,0);
    //xTimerStart(xtimer3,0);
    

    //create sw task
    xTaskCreate(vswitch_task, "switch task", 256, NULL, 1, NULL);

    //start scheduler
    vTaskStartScheduler();
    while (1);
}


