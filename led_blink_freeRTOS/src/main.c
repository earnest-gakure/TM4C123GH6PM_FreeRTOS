#include "TM4C123GH6PM.h"
#include "FreeRTOS.h"
#include "task.h"
#include <time.h>

void gpio_init(){
    SYSCTL->RCGCGPIO |= (1<<5);
    GPIOF->LOCK = 0x4C4F434B;
    GPIOF->CR   = 0xFF;

    GPIOF->DIR |= (1<<1) | (1<<2) | (1<<3);
    GPIOF->DEN |= (1<<1) | (1<<2) | (1<<3);

}

void red_led_task(void *pvParameters)
{
    // Enable clock for GPIOF
    gpio_init();
    vTaskDelay(pdMS_TO_TICKS(200));
    while (1)
    {
        GPIOF->DATA |= (1 << 1);
        vTaskDelay(pdMS_TO_TICKS(300));
        GPIOF->DATA &= ~(1<<1);
        vTaskDelay(pdMS_TO_TICKS(300));

    }
}
void green_led_task(void *pvParameters){
   gpio_init();
   //vTaskDelay(pdMS_TO_TICKS(200));

    while(1){
        GPIOF->DATA |= (1<<3);
        vTaskDelay(pdMS_TO_TICKS(600));
        GPIOF->DATA &= ~(1<<3);
        vTaskDelay(pdMS_TO_TICKS(600));
    }
}
void blue_led_task(void *pvParameters){
   gpio_init();
   vTaskDelay(pdMS_TO_TICKS(200));
   
    while(1){
        GPIOF->DATA |= (1<<2);
        vTaskDelay(pdMS_TO_TICKS(900));
        GPIOF->DATA &= ~(1<<2);
        vTaskDelay(pdMS_TO_TICKS(900));
    }
}


int main(void)
{
    xTaskCreate(red_led_task,  "RED  LED", 256, NULL, 1, NULL);
    xTaskCreate(blue_led_task,"BLUE LED",256, NULL, 1 ,NULL );
    xTaskCreate(green_led_task,"GREEN LED",256, NULL, 1 ,NULL );
    
    vTaskStartScheduler();
    while (1);
}
