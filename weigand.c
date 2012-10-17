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
#include "boot.h"

volatile int bit_count = 0;
volatile unsigned char data[7];
volatile int flg_readcard = 0;
volatile int last_read;

void RFID_Init(void)
{
    //_delay_ms(2000);	
    // Set D0 and D1 to inputs, enable pull up and enable their interupts
    // for DATA0/1
    bit_clear(DDRD, BIT(0));   
    bit_clear(DDRD, BIT(1)); 
    bit_set(PORTD,BIT(0));
    bit_set(PORTD,BIT(1));  
    bit_set(EIMSK,BIT(INT0));
    bit_set(EIMSK,BIT(INT1));  

    // Set D5 and D6 to outputs for LEDs
    bit_set(DDRD, BIT(5));
    bit_set(DDRD, BIT(6));   
    bit_set(PORTD,BIT(5));
    bit_set(PORTD,BIT(6));

    // Set C5 to output for buzzer
    bit_set(DDRC, BIT(5));
    bit_set(PORTC,BIT(5));
}

inline void set_data_bit(volatile unsigned char* data, int byte, int bit, int value)
{
    if (value) {
        bit_set(data[byte],BIT(bit));
    } else {
        bit_clear(data[byte],BIT(bit));
    }
}


inline void set_data_bit_sc(volatile unsigned char* data, int bitcount, int value)
{
    set_data_bit(data, bitcount / 8, (7-bitcount % 8), value);
}

void data0_int(void)
{
    last_read = jiffies;
    set_data_bit_sc(data,bit_count,0);
    bit_count++;
    _delay_us(100); // Delay to remove bounce
}

void data1_int(void)
{
    last_read = jiffies;
    set_data_bit_sc(data,bit_count,1);
    bit_count++;
    _delay_us(100); // Delay to remove bounce
}

void RFID_Task(void)
{
    unsigned char byte = 0;
    byte = (unsigned char)CDC_Device_ReceiveByte(&VirtualSerial_CDC_Interface);
    switch (byte)
    {
        case 'x':
        case 'X':
            // Go into DFU mode
            Jump_To_Bootloader();
            break;
        case 'g':
        case 'G':
            //toggle green led briefly (active low)
            bit_clear(PORTD,BIT(5));
            _delay_ms(500);
            bit_set(PORTD,BIT(5));
            break;
        case 'r':
        case 'R':
            //toggle red led briefly (active low)
            bit_clear(PORTD,BIT(6));
            _delay_ms(500);
            bit_set(PORTD,BIT(6));
            break;
        case 'o':
        case 'O':
            //toggle both leds briefly for orange
            bit_clear(PORTD,BIT(6));
            bit_clear(PORTD,BIT(5));
            _delay_ms(500);
            bit_set(PORTD,BIT(6));
            bit_set(PORTD,BIT(5));
            break;
        default:
            //Nothing
            break;
    }

    cli();

    //if it has been 50ms since we last had a rfid bit
    if ((jiffies-last_read) > 50){
        flg_readcard = 1;
    }

    if (bit_count > 0 && (flg_readcard==1 || bit_count >= 56))
    {
	    char buf[17];
        //Write code
        snprintf(buf,17,"%.2x%.2x%.2x%.2x%.2x%.2x%.2x\r\n", data[0], data[1], data[2], data[3], data[4], data[5], data[6]);
        CDC_Device_SendString(&VirtualSerial_CDC_Interface,buf);
        CDC_Device_Flush(&VirtualSerial_CDC_Interface);
        bit_count = 0;
        flg_readcard = 0;
    }
    sei();
}
