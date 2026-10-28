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
    PIR1bits.TMR2IF = 0;

    /* Enable Timer2 interrupt */
    PIE1bits.TMR2IE = 1;

    /* Enable peripheral and global interrupts */
    INTCONbits.PEIE = 1;
    INTCONbits.GIE = 1;

    /*
     * TOUTPS = 1:10
     * TMR2ON = 1
     * T2CKPS = 1:16
     */
    T2CON = 0x4E;
}




