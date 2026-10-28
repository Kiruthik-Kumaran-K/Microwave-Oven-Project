#ifndef TIMER_H
#define TIMER_H

extern volatile unsigned char timer_1s_flag;
extern volatile unsigned char blink_flag;

void init_timer2(void);

#endif