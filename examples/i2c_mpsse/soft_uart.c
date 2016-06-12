// softuart.c
// AVR-port of the generic software uart written in C
//
// Generic code from
// Colin Gittins, Software Engineer, Halliburton Energy Services
// (has been available from iar.com web-site -> application notes)
//
// Adapted to AVR using avr-gcc and avr-libc
// by Martin Thomas, Kaiserslautern, Germany
// <eversmith@heizung-thomas.de>
// http://www.siwawi.arubi.uni-kl.de/avr_projects
//
// AVR-port Version 0.4  10/2010
//
// ---------------------------------------------------------------------
//
// Remarks from Colin Gittins:
//
// Generic software uart written in C, requiring a timer set to 3 times
// the baud rate, and two software read/write pins for the receive and
// transmit functions.
//
// * Received characters are buffered
// * putchar(), getchar(), kbhit() and flush_input_buffer() are available
// * There is a facility for background processing while waiting for input
// The baud rate can be configured by changing the BAUD_RATE macro as
// follows:
//
// #define BAUD_RATE  19200.0
//
// The function init_uart() must be called before any comms can take place
//
// Interface routines required:
// 1. get_rx_pin_status()
//    Returns 0 or 1 dependent on whether the receive pin is high or low.
// 2. set_tx_pin_high()
//    Sets the transmit pin to the high state.
// 3. set_tx_pin_low()
//    Sets the transmit pin to the low state.
// 4. idle()
//    Background functions to execute while waiting for input.
// 5. timer_set( BAUD_RATE )
//    Sets the timer to 3 times the baud rate.
// 6. set_timer_interrupt( timer_isr )
//    Enables the timer interrupt.
//
// Functions provided:
// 1. void flush_input_buffer( void )
//    Clears the contents of the input buffer.
// 2. char kbhit( void )
//    Tests whether an input character has been received.
// 3. char getchar( void )
//    Reads a character from the input buffer, waiting if necessary.
// 4. void turn_rx_on( void )
//    Turns on the receive function.
// 5. void turn_rx_off( void )
//    Turns off the receive function.
// 6. void putchar( char )
//    Writes a character to the serial port.
//
// ---------------------------------------------------------------------

/*
Remarks By Roaring(for the FX2LP from cypress)




*/











/*
Remarks by Martin Thomas (avr-gcc/avr-libc):
V0.1 (2/2005)
- stdio.h not used
- AVR-Timer in CTC-Mode ("manual" reload may not be accurate enough)
- Global Interrupt Flag has to be enabled (see demo-application)
- Interface timer_set and set_timer_interrupt not used here
- internal_tx_buffer was defined as unsigned char - thas could not
  work since more than 8 bits are needed, changed to unsigned short
- some variables moved from "global scope" into ISR function-scope
- GPIO initialisation included
- Added functions for string-output inspired by P. Fleury's AVR UART-lib.
V0.2 (3/2007)
- adjusted number of RX-bits
- adapted to avr-libc ISR-macro (replaces SIGNAL)
- disable interrupts during timer-init
- used unsigned char (uint8_t) where apropriate
- removed "magic" char checking (0xc2)
- added softuart_can_transmit()
- Makefile based on template from WinAVR 1/2007
- reformated
- extended demo-application to show various methods to
  send a string from flash and RAM
- demonstrate usage of avr-libc's stdio in demo-applcation
- tested with ATmega644 @ 3,6864MHz system-clock using
  avr-gcc 4.1.1/avr-libc 1.4.5 (WinAVR 1/2007)
V0.3 (4/2007)
- better configuration options in softuart.h.
  ->should be easier to adapt to different AVRs
- tested with ATtiny85 @ 1MHz (internal RC) with 2400bps
- AVR-Studio Project-File
V0.4 (10/2010)
- added options for ATmega164P, ATmega32P, ATmega64P
- changed some variable-types from char to unsigned char
- changed some comparisons from <= to ==
- small optimization in ISR for RX with temporary variable
- minor modifications in comments and formating
- added compiler options -fno-inline-small-functions, -Wl,--relax
- renamed flag_tx_ready to flag_tx_busy
- replaced softuart_can_transmit() by softuart_transmit_busy()
- tested with ATmega324PV @ 1MHz internal RC and 2400bps
  (options for ATtiny25/45/85 still available)
- added 3BSD license
- removed redundant zero-init in declaration of qin and qout
*/

/* Copyright (c) 2003, Colin Gittins
   Copyright (c) 2005, 2007, 2010, Martin Thomas
   All rights reserved.
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:
   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.
   * Neither the name of the copyright holders nor the names of
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.
  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE. */


#include <fx2regs.h>
#include <fx2macros.h>
#include <serial.h>
#include <delay.h>
#include <autovector.h>
#include <lights.h>
#include <setupdat.h>
#include <eputils.h>
#include <fx2ints.h>


#define SYNCDELAY SYNCDELAY4

#include "softuart.h"

#define SU_TRUE    1
#define SU_FALSE   0

// startbit and stopbit parsed internally (see ISR)
#define RX_NUM_OF_BITS (8)

// 1 Startbit, 8 Databits, 1 Stopbit = 10 Bits/Frame
#define TX_NUM_OF_BITS (10)

extern void set_tx_pin_high();
//#define get_rx_pin_status()    ( SOFTUART_RXBIT )

unsigned char tx_buffer;
unsigned char rx_buffer;
unsigned char tx_count;
unsigned char rx_count;







/* Queue structure */
#define QUEUE_ELEMENTS 5
#define QUEUE_SIZE (QUEUE_ELEMENTS + 1)
__xdata unsigned char QueueTX[QUEUE_SIZE];
__xdata unsigned char QueueInTX, QueueOutTX;

__xdata unsigned char QueueRX[QUEUE_SIZE];
__xdata unsigned char QueueInRX, QueueOutRX;








//ISR(SOFTUART_T_COMP_LABEL)
//{
//	static unsigned char flag_rx_waiting_for_stop_bit = SU_FALSE;
//	static unsigned char rx_mask;
//
//	static unsigned char timer_rx_ctr;
//	static unsigned char bits_left_in_rx;
//	static unsigned char internal_rx_buffer;
//
//	unsigned char start_bit, flag_in;
//	unsigned char tmp;
//
//	// Transmitter Section
//	if ( flag_tx_busy == SU_TRUE ) {
//
//        //There is data to transmit. The data is located in the buffer
//        //The timer runs 3 times faster. Decrement it each time
//		tmp = timer_tx_ctr;
//		if ( --tmp == 0 ) { // if ( --timer_tx_ctr <= 0 )
//			if ( internal_tx_buffer & 0x01 ) {
//			    //Send out the data bit
//			    //Maybe use some assembly here later
//				set_tx_pin_high();
//			}
//			else {
//                //Send data bit out
//				set_tx_pin_low();
//			}
//			//Right shift and move out the data which has been sent out
//			internal_tx_buffer >>= 1;
//			tmp = 3; // timer_tx_ctr = 3;
//			if ( --bits_left_in_tx == 0 ) {
//                //Finished trasmitting
//				flag_tx_busy = SU_FALSE;
//			}
//		}
//		//Assign thevalue back
//		timer_tx_ctr = tmp;
//	}
//
//	// Receiver Section
//	if ( flag_rx_off == SU_FALSE ) {
//		if ( flag_rx_waiting_for_stop_bit ) {
//			if ( --timer_rx_ctr == 0 ) {
//				flag_rx_waiting_for_stop_bit = SU_FALSE;
//				flag_rx_ready = SU_FALSE;
//				//put in into the buffer
//				inbuf[qin] = internal_rx_buffer;
//				if ( ++qin >= SOFTUART_IN_BUF_SIZE ) {
//					// overflow - reset inbuf-index
//					qin = 0;
//				}
//			}
//		}
//		else {  // rx_test_busy
//			if ( flag_rx_ready == SU_FALSE ) {
//				start_bit = get_rx_pin_status();
//				// test for start bit
//				//If the start bit is low then begin reading data
//				if ( start_bit == 0 ) {
//                    //Set rx_ready to indicate that the receiver is now in operation
//					flag_rx_ready      = SU_TRUE;
//					//Initialize buffer and rx counter
//					internal_rx_buffer = 0;
//					timer_rx_ctr       = 4;
//					bits_left_in_rx    = RX_NUM_OF_BITS;
//					rx_mask            = 1;
//				}
//			}
//			else {  // rx_busy
//				tmp = timer_rx_ctr;
//				if ( --tmp == 0 ) { // if ( --timer_rx_ctr == 0 ) {
//					// rcv
//					tmp = 3;
//					flag_in = get_rx_pin_status();
//					if ( flag_in ) {
//                        //if it is a on then or it with RXMASK
//						internal_rx_buffer |= rx_mask;
//					}
//					rx_mask <<= 1;
//					if ( --bits_left_in_rx == 0 ) {
//                        //wait for stop bit
//						flag_rx_waiting_for_stop_bit = SU_TRUE;
//					}
//				}
//				timer_rx_ctr = tmp;
//			}
//		}
//	}
//}

static void io_init(void)
{
	// TX-Pin as output
	SOFTUART_TXDDR |=  ( 1 << SOFTUART_TXBITNO );
	// RX-Pin as input
	SOFTUART_RXDDR &= ~( 1 << SOFTUART_RXBITNO );
}

void timer_init()
{
	//unsigned char sreg_tmp;

//	sreg_tmp = SREG;
//	cli();
//
//	SOFTUART_T_COMP_REG = SOFTUART_TIMERTOP;     /* set top */
//
//	SOFTUART_T_CONTR_REGA = SOFTUART_CTC_MASKA | SOFTUART_PRESC_MASKA;
//	SOFTUART_T_CONTR_REGB = SOFTUART_CTC_MASKB | SOFTUART_PRESC_MASKB;
//
//	SOFTUART_T_INTCTL_REG |= SOFTUART_CMPINT_EN_MASK;
//
//	SOFTUART_T_CNT_REG = 0; /* reset counter */
//
//	SREG = sreg_tmp;


    TMOD |= 0x20;
    SYNCDELAY;
    TR1 = 0;
    SYNCDELAY;
    TH1 = 0x77;
    SYNCDELAY;
    TL1 = 0x77;
    SYNCDELAY;













}

void softuart_init( void )
{

	set_tx_pin_high(); /* mt: set to high to avoid garbage on init */

	io_init();
	timer_init();
	softuart_turn_rx_on();
	QueueInitRX();
	QueueInitTX();

}

static void idle(void)
{
	// timeout handling goes here
	// - but there is a "softuart_kbhit" in this code...
	// add watchdog-reset here if needed
}

void softuart_turn_rx_on( void )
{
}

void softuart_turn_rx_off( void )
{
}

char softuart_getchar( void )
{
//	char ch;
//    //Read back the data
//	while ( qout == qin ) {
//		//idle();
//		return 0x30;
//	}
//	ch = inbuf[qout];
//	if ( ++qout >= SOFTUART_IN_BUF_SIZE ) {
//		qout = 0;
//	}
//
//	return( ch );
}

unsigned char softuart_kbhit( void )
{
//	return( qin != qout );
}

void softuart_flush_input_buffer( void )
{
//	qin  = 0;
//	qout = 0;
}

unsigned char softuart_transmit_busy( void )
{
//	return ( flag_tx_busy == SU_TRUE ) ? 1 : 0;
}

void softuart_putchar( const char a)
{





    //Keep waiting till current transmission is complete
//	while ( flag_tx_busy == SU_TRUE ) {
//		; // wait for transmitter ready
//		  // add watchdog-reset here if needed;
//	}
//
//	// invoke_UART_transmit
//	timer_tx_ctr       = 3;
//	//Set the number of bits to 10.
//	bits_left_in_tx    = TX_NUM_OF_BITS;
//	//This is to construct the 10 bits which will be sent out via serial
//	internal_tx_buffer = ( ch << 1 ) | 0x200;
//	//The tx_busy flag indicates that a transmit is in progress
//	flag_tx_busy       = SU_TRUE;
}

void softuart_puts( const char *s )
{
//	while ( *s ) {
//		softuart_putchar( *s++ );
//	}
}

void softuart_puts_p( const char *prg_s )
{
//	char c;

//	while ( ( c = pgm_read_byte( prg_s++ ) ) ) {
//		softuart_putchar(c);
//	}
}

unsigned char get_rx_pin_status()
{
//    unsigned char temp;
//
//    SYNCDELAY;
//    SYNCDELAY;
//    temp = IOA;
//    SYNCDELAY;
//    SYNCDELAY;
//    temp = temp >> 1;
//    temp = temp & 0x01;
//    return temp;


  //return SOFTUART_RXBIT;
}


void set_tx_pin_high()
{
//SOFTUART_TXBIT = 1;
//PA0 = 1;
//IOA = 0x01;
//PA0 = 1;
//SYNCDELAY;
}


void set_tx_pin_low()
{
//SOFTUART_TXBIT=0;
//IOA=0x00;
//__asm
//mov 0xb2, #0x01
//clr 0x80
//__endasm;
//SYNCDELAY;
}


unsigned char fast_uart(unsigned char a)

{

    OEA |= 0x04;
//    __asm
//    anl IE, #0x7f;
//    mov a ,dpl
//    clr c
//    mov r0, #0x08;
//    clr _PA2;
//    0005$:
//    mov r1, #0x17;
//    0006$:
//    djnz r1,0006$;
//    nop;
//    nop;
//    0001$:
//    rrc a;
//    jc 0002$;
//    clr _PA2;
//    mov r1,#0x1e;
//    0008$:
//    djnz r1, 0008$
//    nop;
//
//
//
//    ajmp 0007$;
//    0002$:
//    setb _PA2;
//    0003$:
//    mov r1, #0x1f;
//    0004$:
//    djnz r1, 0004$;
//    nop;
//    0007$:
//    djnz r0, 0001$;
//    setb _PA2;
//    orl IE, #0x80;
//
//    __endasm;
__asm

    anl _IE, #0x7f;
    mov a ,dpl
    clr c
    mov r0, #0x08;
    clr _PA2;
    mov r1, #0x17;
    0006$:
    djnz r1,0006$;
    nop;
    nop;
    0001$:
    rrc a;
    mov _PA2, c
    mov r1, #0x1F;
    0004$:
    djnz r1, 0004$;
    nop;
    djnz r0, 0001$;
    setb _PA2;
    orl IE, #0x80;
    __endasm;

    a=20+a;
    return a;

}


void set_resp(unsigned char mask)
{

//OEA |= (0x01 << mask);
OEA |= 0x04;

}


void calculate_delay()
{



}


//This function is called periodically from main
void uart_tx_service()
{
//Data has been loaded by the calling function in a buffer
//We need to move the data and set the flag
QueuePutTX(0x33);
QueueGetTX(&tx_buffer);
fast_uart(tx_buffer);
//Check if operation is ongoing
if(QueueCheckTX()!= 1 && tx_count != 0x80 )
{
    //Load value
    QueueGetTX(tx_buffer);
    //Busy. Operation is ongoing
    //Clear bit 7  indicates that operation has completed.
    tx_count  = 0x80;

}




}


//Called periodically from main
void uart_rx_service()
{

if(QueueCheckRX()!= 1 && rx_count != 0x80 )
{
    //Load value
    QueueGetRX(rx_buffer);
    //Busy. Operation is ongoing
    //Clear bit 7  indicates that operation has completed.
    rx_count  = 0x80;

}


}
















/* Very simple queue
 * These are FIFO queues which discard the new data when full.
 *
 * Queue is empty when in == out.
 * If in != out, then
 *  - items are placed into in before incrementing in
 *  - items are removed from out before incrementing out
 * Queue is full when in == (out-1 + QUEUE_SIZE) % QUEUE_SIZE;
 *
 * The queue will hold QUEUE_ELEMENTS number of items before the
 * calls to QueuePut fail.
 */



void QueueInitTX(void)
{
    QueueInTX = QueueOutTX = 0;
}

__bit QueuePutTX(unsigned char data)
{
    if(QueueInTX == (( QueueOutTX - 1 + QUEUE_SIZE) % QUEUE_SIZE))
    {
        return 1; /* Queue Full*/
    }

    QueueTX[QueueInTX] = data;

    QueueInTX = (QueueInTX + 1) % QUEUE_SIZE;

    return 0; // No errors
}

__bit QueueGetTX(unsigned char *old)
{
    if(QueueInTX == QueueOutTX)
    {
        return 1; /* Queue Empty - nothing to get*/
    }

    *old = QueueTX[QueueOutTX];

    QueueOutTX = (QueueOutTX + 1) % QUEUE_SIZE;

    return 0; // No errors
}


__bit QueueCheckTX()
{
    if(QueueInTX == QueueOutTX)
    {
        return 1; /* Queue Empty - nothing to get*/
    }
    return 0; // No errors
}




void QueueInitRX(void)
{
    QueueInRX = QueueOutRX = 0;
}

__bit QueuePutRX(unsigned char data)
{
    if(QueueInRX == (( QueueOutRX - 1 + QUEUE_SIZE) % QUEUE_SIZE))
    {
        return 1; /* Queue Full*/
    }

    QueueRX[QueueInRX] = data;

    QueueInRX = (QueueInRX + 1) % QUEUE_SIZE;

    return 0; // No errors
}

__bit QueueGetRX(unsigned char *old)
{
    if(QueueInRX == QueueOutRX)
    {
        return 1; /* Queue Empty - nothing to get*/
    }

    *old = QueueRX[QueueOutRX];

    QueueOutRX = (QueueOutRX + 1) % QUEUE_SIZE;

    return 0; // No errors
}


__bit QueueCheckRX()
{
    if(QueueInRX == QueueOutRX)
    {
        return 1; /* Queue Empty - nothing to get*/
    }
    return 0; // No errors
}




























