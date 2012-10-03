#ifndef _TIMER_H
#define _TIMER_H

#define HZ 100
#define PRESCALE 8
#define TIMER_TGT ((F_CPU/PRESCALE)/HZ)

extern volatile int jiffies;
#endif
