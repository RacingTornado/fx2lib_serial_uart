/**
 * Copyright (C) 2009 Ubixum, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 **/
#include <stdio.h>

#include <fx2regs.h>
#include <fx2macros.h>
#include <serial.h>
#include <delay.h>
#include <autovector.h>
#include <lights.h>
#include <setupdat.h>
#include <eputils.h>
#include <fx2ints.h>
#include "softuart.h"
#include "mpsse.h"

#define SU_TRUE    1
#define SU_FALSE   0

// startbit and stopbit parsed internally (see ISR)
#define RX_NUM_OF_BITS (8)


#define SYNCDELAY SYNCDELAY4
#define REARMVAL 0x80
#define REARM() EP2BCL=REARMVAL




volatile WORD bytes;
volatile __bit gotbuf;
volatile BYTE icount;
volatile __bit got_sud;
volatile unsigned char anotherone;
DWORD lcount;
__bit on;
static unsigned char flag_rx_waiting_for_stop_bit = SU_FALSE;
static unsigned char rx_mask;

static unsigned char timer_rx_ctr;
static unsigned char bits_left_in_rx;
static unsigned char internal_rx_buffer;

unsigned char start_bit, flag_in;
unsigned char pin_data;
unsigned char buf_data;
unsigned char bit_number;
unsigned char pin_state;


typedef enum
{
  START_BIT = 2,
  DATA,
  STOP_BIT
} uart_state_rx;

extern void uart_config ();
extern void ProcessXmitData ();
extern void ProcessRecvData ();
extern void toggle_pins ();
extern void configure_timer ();
extern void start_timer ();
extern void timer_init ();
extern void set_tx_pin_high ();
extern void set_tx_pin_low ();
extern unsigned char get_rx_pin_status ();
extern void configure_drive (unsigned char a, unsigned char b);
extern void toggle_port_value (unsigned char a, unsigned char b);
extern void uart_rx_fill ();
extern void putchar_a (char a);

extern void i2c_addr_logic ();
extern char i2c_data_logic (unsigned char dummyportenselect);
extern void i2c_stop_logic ();

extern void spi_data_logic (unsigned char mosi_data_a,
			    unsigned char master_pin_a);
extern void spi_mosi_data_logic ();
extern void spi_miso_data_logic ();
extern void fast_uart(unsigned char a);
extern void set_resp(unsigned char a);

extern void temp_call ();
extern char xxy (char a, char b);
extern BOOL handle_mpsse ();

extern volatile unsigned char flag_tx_busy;
extern volatile unsigned char timer_tx_ctr;
extern volatile unsigned short internal_tx_buffer;
extern volatile unsigned char bits_left_in_tx;

extern volatile unsigned char flag_rx_off;
extern volatile unsigned char flag_rx_ready;
extern unsigned char qout;
extern volatile char inbuf[SOFTUART_IN_BUF_SIZE];
extern volatile unsigned char qin;

unsigned char bxxy;
unsigned char sbxxy;
unsigned char counter_fast;


uart_state_rx rx_state;

void
main ()
{

  //Commented out, dont know if light exists on Numato board
  //d2off();
  //Setup data available and other init
  got_sud = FALSE;
  gotbuf = FALSE;
  bytes = 0;

  // renumerate
  RENUMERATE_UNCOND ();
  //Call our custom function to do our UART init
  uart_config ();

  //configure_timer();
  //timer_init();

  softuart_init ();
  start_timer ();
  ENABLE_TIMER1 ();

  rx_state = START_BIT;
  configure_drive (0xb0, 0);

  //i2c_bitbang();


  //d();

  //SETCPUFREQ(CLK_48M);
  //SETIF48MHZ();
  sio0_init (57600);

  //Enable USB auto vectored interrupts
  USE_USB_INTS ();
  ENABLE_SUDAV ();
  ENABLE_SOF ();
  ENABLE_HISPEED ();
  ENABLE_USBRESET ();
  EA = 1;			// global interrupt enable

  set_resp(0x02);

  //USBCS |= bmRENUM;
  while (TRUE)
    {
      //toggle_pins();
      //Handles device descriptor requests
      //softuart_putchar(0x3c);
      //softuart_putchar(softuart_getchar());
      //uart_rx_fill();
      //i2c_bitbang();
      //temp_call();

    //toggle_pins();
    counter_fast++;
    if(counter_fast >200)
    {
    OEA |= 0x04;
    fast_uart(0x33);
    counter_fast =0 ;
    }
      if (qin != qout)
	{
	  i2c_addr_logic ();
	  bxxy = i2c_data_logic (0x60);
	  i2c_stop_logic ();
	  softuart_putchar (inbuf[qout]);
	  softuart_putchar (0x3d);
	  softuart_putchar (bxxy);
	  putchar_a (inbuf[qout]);
	  putchar_a (0x25);


	  //spi_data_logic(0x23,0x60);
	  //spi_data_logic(0x56,0x28);
	  //PA2 = 0;
	  //OEA = 0x04;
	  //SYNCDELAY;
	  //PA2 = 1;
	  //SYNCDELAY;
	  //spi_mosi_data_logic();
	  //spi_miso_data_logic();
	  //spi_data_logic(0x23,0x64);
	  //spi_data_logic(0x56,0x2C);
	  //spi_data_logic(0x23,0x64);
	  //spi_mosi_data_logic();
	  //spi_miso_data_logic();
	  //PA2 = 0;
	  //SYNCDELAY;
	  softuart_putchar (inbuf[qout]);
	  softuart_putchar (0x3d);
	  putchar_a (inbuf[qout]);
	  putchar_a (0x25);



	  qout++;
	  if (qout == SOFTUART_IN_BUF_SIZE)
	    {
	      qout = 0;

	    }



	}

      //if ( got_sud ) {
      if (anotherone >0 )
	{
	  toggle_pins ();
	  toggle_pins ();
      toggle_pins();
	  toggle_pins();
	  handle_setupdata ();
	  anotherone --;
	}
	else
	{
        //toggle_pins();
	}



      // Input data on EP1
      if (!(EP1OUTCS & bmEPBUSY))
	{
	  ProcessRecvData ();
	  //toggle_pins();

	}

      // Timer expiration; send buffered data
      if ((TCON & 0x20))
	{
	  ProcessXmitData ();
	}



    }

}

// copied routines from setupdat.h

BOOL
handle_get_descriptor ()
{
  return FALSE;
}

// value (low byte) = ep
#define VC_EPSTAT 0xB1

BOOL
handle_vendorcommand (BYTE cmd)
{

  switch (cmd)
    {

    case VC_EPSTAT:
      {
	__xdata BYTE *pep = ep_addr (SETUPDAT[2]);
	printf ("ep %02x\n", *pep);
	if (pep)
	  {
	    EP0BUF[0] = *pep;
	    EP0BCH = 0;
	    EP0BCL = 1;
	    return TRUE;
	  }
      }
      break;

    default:
      handle_mpsse ();
      printf ("Need to implement vendor command: %02x\n", cmd);
    }
  return FALSE;
}











// this firmware only supports 0,0
BOOL
handle_get_interface (BYTE ifc, BYTE * alt_ifc)
{
  printf ("Get Interface\n");
  if (ifc == 0)
    {
      *alt_ifc = 0;
      return TRUE;
    }
  else
    {
      return FALSE;
    }
}

BOOL
handle_set_interface (BYTE ifc, BYTE alt_ifc)
{
  printf ("Set interface %d to alt: %d\n", ifc, alt_ifc);

  if (ifc == 0 && alt_ifc == 0)
    {
      // SEE TRM 2.3.7
      // reset toggles
      RESETTOGGLE (0x02);
      RESETTOGGLE (0x86);
      // restore endpoints to default condition
      RESETFIFO (0x02);
      EP2BCL = 0x80;
      SYNCDELAY;
      EP2BCL = 0X80;
      SYNCDELAY;
      RESETFIFO (0x86);
      return TRUE;
    }
  else
    return FALSE;
}

__bit on5;
__xdata WORD sofct = 0;
void
sof_isr ()
     __interrupt SOF_ISR __using 1
     {
       ++sofct;
       if (sofct == 8000)
	 {			// about 8000 sof interrupts per second at high speed
	   on5 = !on5;
	   if (on5)
	     {
	       d5on ();
	     }
	   else
	     {
	       d5off ();
	     }
	   sofct = 0;
	 }
       CLEAR_SOF ();
     }

// get/set configuration
BYTE
handle_get_configuration ()
{
  return 1;
}

BOOL
handle_set_configuration (BYTE cfg)
{
  return cfg == 1 ? TRUE : FALSE;	// we only handle cfg 1
}


// copied usb jt routines from usbjt.h
void
sudav_isr ()
     __interrupt SUDAV_ISR
     {

       toggle_pins ();
       toggle_pins ();
       //toggle_pins ();
       //toggle_pins ();

       got_sud = TRUE;
       anotherone++;
       CLEAR_SUDAV ();
     }




     void usbreset_isr ()
     __interrupt USBRESET_ISR
     {
       handle_hispeed (FALSE);
       CLEAR_USBRESET ();
     }
     void hispeed_isr ()
     __interrupt HISPEED_ISR
     {
       handle_hispeed (TRUE);
       CLEAR_HISPEED ();
     }
     void timer1_isr ()
     __interrupt TF1_ISR
     {
//toggle_pins();

//toggle_port_value(0xb0,1);
//get_rx_pin_status();
//toggle_port_value(0xb0,1);
       // Transmitter Section
       if (flag_tx_busy == SU_TRUE)
	 {

	   //There is data to transmit. The data is located in the buffer
	   //The timer runs 3 times faster. Decrement it each time
	   //tmp = timer_tx_ctr;
	   if (--timer_tx_ctr <= 0)
	     {
	       if (internal_tx_buffer & 0x01)
		 {
		   //Send out the data bit
		   //Maybe use some assembly here later
		   set_tx_pin_high ();
		   //toggle_pins();
		 }
	       else
		 {
		   //Send data bit out
		   set_tx_pin_low ();
		 }
	       //Right shift and move out the data which has been sent out
	       internal_tx_buffer >>= 1;
	       timer_tx_ctr = 3;
	       if (--bits_left_in_tx == 0)
		 {
		   //Finished trasmitting
		   flag_tx_busy = SU_FALSE;
		 }
	     }

	 }

       // Receiver Section
       if (flag_rx_off == SU_FALSE)
	 {
	   //if ( flag_rx_waiting_for_stop_bit ) {
	   //      if ( --timer_rx_ctr == 0 ) {
	   //              flag_rx_waiting_for_stop_bit = SU_FALSE;
	   //              flag_rx_ready = SU_FALSE;
	   //put in into the buffer
	   //inbuf[qin] = internal_rx_buffer;
	   //if ( ++qin >= SOFTUART_IN_BUF_SIZE ) {
	   // overflow - reset inbuf-index
	   //      qin = 0;
	   //}
	   //              rx_state = STOP_BIT;
	   //      }
	   //}
	   //else {  // rx_test_busy
	   //      if ( flag_rx_ready == SU_FALSE ) {
	   //start_bit = get_rx_pin_status();
	   //              pin_data= get_rx_pin_status();
	   // test for start bit
	   //If the start bit is low then begin reading data
	   //              if ( (start_bit&0x01) == 0 ) {
	   //Set rx_ready to indicate that the receiver is now in operation
	   //                      flag_rx_ready      = SU_TRUE;
	   //Initialize buffer and rx counter
	   //                      internal_rx_buffer = 0;
	   //                      timer_rx_ctr       = 3;
	   //                      bits_left_in_rx    = RX_NUM_OF_BITS;
	   //                      rx_mask            = 1;
	   //              }
	   //      }
	   //      else {  // rx_busy
	   //tmp = timer_rx_ctr;
	   //               if ( --timer_rx_ctr == 0 ) {
	   // rcv
	   //tmp = 3;
	   //flag_in = get_rx_pin_status();
	   //                      pin_data = get_rx_pin_status();
	   //if ( flag_in ) {
	   //if it is a on then or it with RXMASK
	   //      internal_rx_buffer |= rx_mask;
	   //}
	   //rx_mask <<= 1;
	   //if ( --bits_left_in_rx == 0 ) {
	   //wait for stop bit
	   //flag_rx_waiting_for_stop_bit = SU_TRUE;
	   //}
	   //                      timer_rx_ctr = 3;
	   //              }
	   //timer_rx_ctr = tmp;
	   //timer_rx_ctr=2;
	   //      }

	   if (timer_rx_ctr == 0)
	     {
	       //pin_state= get_rx_pin_status();
	       //pin_data = get_rx_pin_status();
	       pin_state = get_rx_pin_status ();
	       if (pin_state == 0 && rx_state == START_BIT)
		 {
		   rx_state = DATA;
		 }
	       else if (rx_state == DATA)
		 {
		   pin_data |= (pin_state << bit_number);
		   bit_number = bit_number + 1;
		   if (bit_number == 8)
		     {
		       rx_state = STOP_BIT;
		       bit_number = 0;
		     }

		 }
	       else
		 {
		   if (pin_state == 1 && rx_state == STOP_BIT)
		     {
		       rx_state = START_BIT;
		       inbuf[qin] = pin_data;
		       qin++;
		       if (qin == SOFTUART_IN_BUF_SIZE)
			 {
			   qin = 0;
			 }
		       pin_data = 0;
		     }
		 }
	       timer_rx_ctr = 2;
	     }
	   else
	     {
	       timer_rx_ctr = timer_rx_ctr - 1;
	     }



	 }
     }




void
uart_rx_fill ()
{

  switch (rx_state)
    {
    case START_BIT:
      if (pin_data == 0)
	{
	  rx_state = DATA;
	  toggle_port_value (0xb0, 1);
	}
      bit_number = 0;
      break;
    case DATA:
      pin_data |= (pin_data << bit_number);
      bit_number = bit_number + 1;
      if (bit_number == 8)
	{
	  rx_state = STOP_BIT;
	}
      break;
    case STOP_BIT:
      if (pin_data == 1)
	{
	  buf_data = pin_data;
	  softuart_putchar (buf_data);
	  rx_state = START_BIT;
	}
      break;
    }


}
