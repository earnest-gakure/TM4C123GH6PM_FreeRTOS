#include "TM4C123GH6PM.h"
#include "FreeRTOS.h"
#include "task.h"

void led_task(void *pvParameters)
{
    // Enable clock for GPIOF
    SYSCTL->RCGCGPIO |= (1 << 5);
    while ((SYSCTL->PRGPIO & (1 << 5)) == 0);

    // Unlock GPIOF (required for PF0; harmless for other pins)
    GPIOF->LOCK = 0x4C4F434B;
    GPIOF->CR   = 0xFF;

    // Configure PF1 (Red LED)
    GPIOF->AMSEL &= ~(1 << 1);   // disable analog
    GPIOF->AFSEL &= ~(1 << 1);   // GPIO function, not alternate
    GPIOF->DIR   |=  (1 << 1);   // output
    GPIOF->DEN   |=  (1 << 1);   // digital enable

    // Re-lock
    GPIOF->CR   = 0x00;
    GPIOF->LOCK = 0;

    while (1)
    {
        GPIOF->DATA ^= (1 << 1);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

int main(void)
{
    xTaskCreate(led_task, "LED", 256, NULL, 1, NULL);
    vTaskStartScheduler();
    while (1);
}
