#include <xc.h>
#include "timer.h"

static unsigned char timer_ticks = 0;

volatile unsigned char blink_flag = 0;

static unsigned char blink_ticks = 0;
static unsigned char blink_state = 0;

void __interrupt() isr(void)
{
    if (PIR1bits.TMR2IF && PIE1bits.TMR2IE)
    {
        PIR1bits.TMR2IF = 0;

        if (++timer_ticks >= 125)
        {
            timer_ticks = 0;
            timer_1s_flag = 1;
        }
        
        if (++blink_ticks >= 31)
        {
            blink_ticks = 0;
            blink_state = !blink_state;
            blink_flag = blink_state;
        }
    }
}


