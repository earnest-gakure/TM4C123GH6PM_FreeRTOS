/**
 * debounce button and toggle LED using software timer
*/

#include "TM4C123GH6PM.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"


//timer 
BaseType_t xtimer1Started, xtimer2Started;
static TimerHandle_t xtimer1 , xtimer2;

//buton
static uint8_t button1_state = 1;
static uint8_t button2_state = 1;    
void gpio_init(){
    SYSCTL->RCGCGPIO |= (1<<5);
    GPIOF->LOCK = 0x4C4F434B;
    GPIOF->CR   = 0xFF;
    GPIOF->PUR |= (1<<0) | (1<<4); //enable pul-up resister

    GPIOF->DIR |= (1<<1) | (1<<2) | (1<<3);
    GPIOF->DEN |=  (1<<0) |(1<<1) | (1<<2) | (1<<3) | (1<<4);
    GPIOF->DATA &= ~( (1<<1) | (1<<2) | (1<<3));

}
/* callback function for buttton 1  timer*/
void vtimer1_callback(TimerHandle_t xTimer)
{
    
        if ((GPIOF->DATA & (1<<4)) == 0x00)
        {
            GPIOF->DATA ^= (1<<1);
        }

}
//callback function for button 2 timer
void vtimer2_callback(TimerHandle_t xTimer)
{
    if ((GPIOF->DATA  & (1<<0)) == 0x00)
    {
            GPIOF->DATA ^= (1<<2);
    }
}
/*task to poll the state of the buttons*/
void vbuttonTask(void *pvParameter){
    while (1)
    {  
        if (((GPIOF->DATA  & (1<<4)) == 0x00) && (button1_state == 1))
        {
            button1_state = 0;
            //reset timer 
            xTimerReset(xtimer1,0);
        }
        else if (((GPIOF->DATA  & (1<<4)) != 0x00) )
        {
            button1_state = 1;
        }

        if (((GPIOF->DATA & (1<<0)) == 0x00) && (button2_state == 1))
        {
            button2_state = 0;
            //reset timer 
            xTimerReset(xtimer2,0);
        } else if (((GPIOF->DATA & (1<<0)) != 0x00))
        {
            button2_state = 1;
        }    
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    

}

int main(void)
{
    gpio_init();
    //create timer
    xtimer1 = xTimerCreate("button 1", pdMS_TO_TICKS(50), pdFALSE, 0, vtimer1_callback);
    xtimer2 = xTimerCreate("button 2", pdMS_TO_TICKS(50), pdFALSE, 0, vtimer2_callback);


    xTaskCreate(vbuttonTask, "buttons", 256, NULL, 1, NULL);
    vTaskStartScheduler();
    while (1);
}


