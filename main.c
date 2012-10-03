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
#include <LUFA/Version.h>
#include <LUFA/Drivers/Misc/RingBuffer.h>
#include <LUFA/Drivers/USB/USB.h>
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
				.ControlInterfaceNumber   = 0,
				.DataINEndpoint           =
					{
						.Address          = CDC_TX_EPADDR,
						.Size             = CDC_TXRX_EPSIZE,
						.Banks            = 1,
					},
				.DataOUTEndpoint =
					{
						.Address          = CDC_RX_EPADDR,
						.Size             = CDC_TXRX_EPSIZE,
						.Banks            = 1,
					},
				.NotificationEndpoint =
					{
						.Address          = CDC_NOTIFICATION_EPADDR,
						.Size             = CDC_NOTIFICATION_EPSIZE,
						.Banks            = 1,
					},
			},
	};

uint32_t Boot_Key ATTR_NO_INIT;
#define MAGIC_BOOT_KEY 0x4AC59ACE
#define FLASH_SIZE_BYTES 0x4000
#define BOOTLOADER_SEC_SIZE_BYTES 4096
#define BOOTLOADER_START_ADDRESS (FLASH_SIZE_BYTES - BOOTLOADER_SEC_SIZE_BYTES)

void Bootloader_Jump_Check(void) ATTR_INIT_SECTION(3);
void Bootloader_Jump_Check(void)
{
	// If the reset source was the bootloader and the key is correct, clear it and jump to the bootloader
	if ((MCUSR & (1 << WDRF)) && (Boot_Key == MAGIC_BOOT_KEY))
	{
		Boot_Key = 0;
		((void (*)(void))BOOTLOADER_START_ADDRESS)();
	}
}

void Jump_To_Bootloader(void)
{
	// If USB is used, detach from the bus and reset it
	USB_Disable();
	// Disable all interrupts
	cli();
	// Wait two seconds for the USB detachment to register on the host
	Delay_MS(2000);
	// Set the bootloader key to the magic value and force a reset
	Boot_Key = MAGIC_BOOT_KEY;
	wdt_enable(WDTO_250MS);
	for (;;);
}

volatile int bit_count = 0;
volatile unsigned char data[7];
int main(int argc, const char *argv[]) {
	char buf[17];
	int event = 0;
	clock_prescale_set(clock_div_1);
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	// Set D0 and D1 to inputs and enable their interupts
	// for DATA0/1
	bit_clear(DDRD, BIT(0));   
	bit_clear(DDRD, BIT(1));   
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

		
	USB_Init();
	sei();

	unsigned char byte = 0;
	bool command = 0;
	for (;;)
	{
		byte = 0; 
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
		if (bit_count >= 56)
		{
			//Write code
			snprintf(buf,17,"%.2x%.2x%.2x%.2x%.2x%.2x%.2x\r\n", data[0], data[1], data[2], data[3], data[4], data[5], data[6]);
			CDC_Device_SendString(&VirtualSerial_CDC_Interface,buf);
			CDC_Device_Flush(&VirtualSerial_CDC_Interface);
			bit_count = 0;
		}
		sei();
		USB_USBTask();
	}
}

inline set_data_bit_sc(unsigned char* data, int bitcount, int value)
{
	set_data_bit(data, bitcount / 8, (7-bitcount % 8), value);
}

inline set_data_bit(unsigned char* data, int byte, int bit, int value)
{
	if (value) {
		bit_set(data[byte],BIT(bit));
	} else {
		bit_clear(data[byte],BIT(bit));
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
	set_data_bit_sc(data,bit_count,0);
	bit_count++;
	_delay_us(100); // Delay to remove bounce
}

// DATA 1 Int
ISR(INT1_vect, ISR_BLOCK)
{
	set_data_bit_sc(data,bit_count,1);
	bit_count++;
	_delay_us(100); // Delay to remove bounce
}




