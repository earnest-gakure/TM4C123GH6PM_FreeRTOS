#include "TM4C123GH6PM.h"

static void delay(volatile unsigned long n){
    while(n--){}
}

int main(){
    /*enable clock for GPIOF */
    SYSCTL->RCGCGPIO |= (1U<<5);
    
    GPIOF->DIR |= (1<<1);
    GPIOF->DEN |= (1<<1);

    while(1){
        GPIOF->DATA |= (1<<1);
        delay(400000);
        GPIOF->DATA &= ~(1<<1);
        delay(400000);
    }
}



