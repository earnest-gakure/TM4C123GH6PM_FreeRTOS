#include "TM4C123GH6PM.h"
#include "FreeRTOS.h"
#include "task.h"
#include <time.h>
#include "queue.h"
#include "stdio.h"

#define MSG_LEN 30

QueueHandle_t myQueue;

//task function to send data to the queue
void myTask1(void *p){
    char myTxBuff[30];
    //create queue
   // myQueue = xQueueCreate(5,sizeof(myTxBuff));
   while(1){
    //sprintf(myTxBuffssage 1");
    xQueueSend(myQueue,myTxBuff, (TickType_t)0);

    vTaskDelay(pdMS_TO_TICKS(1000));

   }

   

}

//task function to read data from the queue
void myTask2(void *p){
    char myRxBuff[30];
    while(1){
        if(myQueue != 0){
            if( xQueueReceive(myQueue, (void*)myRxBuff, (TickType_t)5)){
               // printf("Data received : %s\r\n", myRxBuff);
            }
        }
    }

}


void gpio_init(){
    SYSCTL->RCGCGPIO |= (1<<5);
    GPIOF->LOCK = 0x4C4F434B;
    GPIOF->CR   = 0xFF;

    GPIOF->DIR |= (1<<1) | (1<<2) | (1<<3);
    GPIOF->DEN |= (1<<1) | (1<<2) | (1<<3);

}

int main(void)
{
    myQueue = xQueueCreate(5,sizeof(char([MSG_LEN]))); 
    xTaskCreate(myTask1,  "task 1", 256, NULL, 1, NULL);
    xTaskCreate(myTask2,  "task 2", 256, NULL, 1 ,NULL );
    
    vTaskStartScheduler();
    while (1);
}

