#include <xc.h>
#include "timer.h"

volatile unsigned char timer_1s_flag = 0;


void init_timer2(void)
{
    /* Stop Timer2 during configuration */
    T2CON = 0x00;

    /* Timer2 period register */
    PR2 = 249;
    TMR2 = 0;

    /* Clear pending interrupt flag */
    TMR2IF = 0;

    /* Enable Timer2 interrupt */
    TMR2IE = 1;

    /* Enable peripheral and global interrupts */
    PEIE = 1;
    GIE = 1;

    /*
     * TOUTPS = 1:10
     * TMR2ON = 1
     * T2CKPS = 1:16
     */
    T2CON = 0x4E;
}




