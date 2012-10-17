#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <LUFA/Version.h>
#include <LUFA/Drivers/Misc/RingBuffer.h>
#include <LUFA/Drivers/USB/USB.h>
#include "bitm.h"
#include "usb.h"
#include "timer.h"
void timer_init()
{
    TCCR1B |= (1 << WGM12);
    TIMSK1 |= (1 << OCIE1A);
    OCR1A = TIMER_TGT;
    TCCR1B |= ((1 << CS10) | (1 << CS11)); //Set prescale to /64
}

ISR(TIMER1_COMPA_vect)
{
    jiffies++;
}
