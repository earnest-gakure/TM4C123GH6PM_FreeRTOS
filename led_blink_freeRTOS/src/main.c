#include "TM4C123GH6PM.h"
#include "FreeRTOS.h"
#include "task.h"

void led_task(void *pvParameters)
{
    SYSCTL->RCGCGPIO |= (1<<5);        // enable clock for GPIOF
    while((SYSCTL->PRGPIO & (1<<5))==0);

    GPIOF->DIR |= (1<<1);              // PF1 output
    GPIOF->DEN |= (1<<1);              // digital enable

    while(1)
    {
        GPIOF->DATA ^= (1<<1);         // toggle LED
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

int main(void)
{
    xTaskCreate(led_task,"LED",200,NULL,1,NULL);

    vTaskStartScheduler();

    while(1);
}
