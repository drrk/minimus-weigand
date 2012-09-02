/*
 * Copyright (C) 2012 Kimball Johnson
 * parts Copyright (C) 2009 Chris McClelland
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "bitm.h"
#include "usb.h"

/** LUFA CDC Class driver interface configuration and state information. This structure is
 *  passed to all CDC Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_CDC_Device_t VirtualSerial_CDC_Interface =
	{
		.Config =
			{
				.ControlInterfaceNumber         = 0,

				.DataINEndpointNumber           = CDC_TX_EPNUM,
				.DataINEndpointSize             = CDC_TXRX_EPSIZE,
				.DataINEndpointDoubleBank       = false,

				.DataOUTEndpointNumber          = CDC_RX_EPNUM,
				.DataOUTEndpointSize            = CDC_TXRX_EPSIZE,
				.DataOUTEndpointDoubleBank      = false,

				.NotificationEndpointNumber     = CDC_NOTIFICATION_EPNUM,
				.NotificationEndpointSize       = CDC_NOTIFICATION_EPSIZE,
				.NotificationEndpointDoubleBank = false,
			},
	};

#define WEIGEN_LEN 26
int bit_count=0;

int main(int argc, const char *argv[]) {
    char buf[30];
    // Init system clock, disable WDT...
    char dat[WEIGEN_LEN]; // 26 bit for data
    unsigned char fcode; // for facility code (0..255)
    unsigned int code; // for card number (0..65535)

    clock_prescale_set(clock_div_1);
    MCUSR &= ~(1 << WDRF);
    wdt_disable();
    
    DDRD = 0;   
    bit_set(EIMSK,BIT(INT0));
    bit_set(EIMSK,BIT(INT1));     

    USB_Init();
  

    sei();
    int i=0;
    
    for (;;) {
	//if (bit_is_clear(PIND,1))
	//{
	//	_delay_ms(1);
	//	dat[i]=1; //    bit = 1
	//	i++;
	//	//loop_until_bit_is_set(PIND,1);
	//	_delay_ms(2);
	//}
//	if (bit_is_clear(PIND,0))
//	{
//		dat[i]=0; //    bit = 0
//		i++;
//		//loop_until_bit_is_set(PIND,0);
//		_delay_ms(2);
//	}
/*
	if (i > WEIGEN_LEN ) {
	int b = 0;
	for (int a=9; a>1; a--)
	{
	    if (dat[a]==1) fcode=fcode+(2^b);
	    b++;
        }
        b=0;
	
	for (int a=25; a>9; a--)
	{
	    if (dat[a]==1) code = code + (2^b);
	    b++;
	}
        //Write code
	    snprintf(buf,30,"fcode: %d code: %d\r\n", fcode, code);
            CDC_Device_SendString(&VirtualSerial_CDC_Interface,buf);
	    i = 0;
	}
*/
	_delay_ms(1);
	snprintf(buf,30,"%d\r\n",bit_count);
            CDC_Device_SendString(&VirtualSerial_CDC_Interface,buf);
        CDC_Device_USBTask(&VirtualSerial_CDC_Interface);
        USB_USBTask();
    }
}
/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void)
{
}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void)
{
}

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
    bool success = true;

    success &= CDC_Device_ConfigureEndpoints(&VirtualSerial_CDC_Interface);
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
    CDC_Device_ProcessControlRequest(&VirtualSerial_CDC_Interface);
}
// DATA 0 Int
ISR(INT0_vect, ISR_BLOCK)
{
	bit_count++;
	_delay_us(100);
}


// DATA 1 Int
ISR(INT1_vect, ISR_BLOCK)
{
	bit_count++;
	_delay_us(100);
}

