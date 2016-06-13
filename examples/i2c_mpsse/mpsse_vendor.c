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

extern void toggle_pins();
unsigned char k;

BOOL handle_mpsse()
{

softuart_putchar('h');

EP0BCH = 0;
EP0BCL = 3;

for(k=0;k<8;k++)
{
//softuart_putchar(SETUPDAT[k]);
}


for(k=0;k<10;k++)
{
//softuart_putchar(EP0BUF[k]);

}

//SUDPTRCTL = 0x01;


        /*
        +-----------------+--------------------------------------------------------------------+--+
        | BITMODE_RESET   | switch off bitbang mode, back to regular serial/FIFO               |  |
        +-----------------+--------------------------------------------------------------------+--+
        | BITMODE_BITBANG | classical asynchronous bitbang mode, introduced with B-type chips  |  |
        +-----------------+--------------------------------------------------------------------+--+
        | BITMODE_MPSSE   | MPSSE mode, available on 2232x chips                               |  |
        +-----------------+--------------------------------------------------------------------+--+
        | BITMODE_SYNCBB  | synchronous bitbang mode, available on 2232x and R-type chips      |  |
        +-----------------+--------------------------------------------------------------------+--+
        | BITMODE_MCU     | MCU Host Bus Emulation mode, available on 2232x chips              |  |
        +-----------------+--------------------------------------------------------------------+--+
        | BITMODE_OPTO    | Fast Opto-Isolated Serial Interface Mode, available on 2232x chips |  |
        +-----------------+--------------------------------------------------------------------+--+
        | BITMODE_CBUS    | Bitbang on CBUS pins of R-type chips, configure in EEPROM before   |  |
        +-----------------+--------------------------------------------------------------------+--+
        | BITMODE_SYNCFF  | Single Channel Synchronous FIFO mode, available on 2232H chips     |  |
        +-----------------+--------------------------------------------------------------------+--+
        | BITMODE_FT1284  | FT1284 mode, available on 232H chips                               |  |
        +-----------------+--------------------------------------------------------------------+--+

        */

       switch (SETUPDAT[1])
        {
        //Not really sure why this bmRequest is being issue. Just ack it anyway
        //This is also the value for bmRequest when an FTDI_RESET is requested
        case FTDI_RESET:
            {

                //EP0CS |= 0x80;
                //toggle_pins();
                //toggle_pins();
                EP0BUF[0]=01;
                EP0BUF[1]= 06;
                EP0BCH=0;
                EP0BCL=2;
                //SUDPTRL = 03;
                //EP0CS = EP0CS & 0x7f;
                EP0CS |= 0x80;
            }
            break;
        //This is also issued as part of the ftdi_open_device. ACK it anyway without looking at data
        case MPSSE_TWO:
        {
                EP0CS |= 0x80;
                EP0BUF[0] = 1;
                EP0BUF[1] =2;
                EP0BCH= 0;
                EP0BCL = 2;
                SUDPTRL = 2;
        }
        break;
        //This is generally packet number 4 which is for setting latency
        //Ignore this packet too. Just clear so it can be acked.
        case SET_LATENCY:
        {
            EP0BUF[0]=1;
            EP0BUF[1]=2;
            EP0BCH= 0;
            EP0BCL = 2;
            SUDPTRL = 2;
            EP0CS |= 0x80;

        }
        break;

        //This is packet number 4. This defines the FTDI bitbang mode.
        //0x0b enables bitbang mode
        /*
        int ftdi_set_bitmode 	( 	struct ftdi_context *  	ftdi,
		unsigned char  	bitmask,
		unsigned char  	mode
	   )
	   */
	   //TODO
	   //We need to examine the bitmask to set the OEA register

        case SIO_SET_BITMODE_REQUEST:
        {


            EP0BUF[0]= 0;
            EP0BUF[1] = 1;
            EP0BCH = 0;
            EP0BCL = 2;
            SUDPTRL = 3;
            EP0CS |= 0x80;

        }
        break;

        case MPSSE_FIVE:
        {
            EP0BUF[0] = 1;
            EP0BUF[1] = 0;
            EP0BCH = 0;
            EP0BCL = 1;
            SUDPTRL = 3;
            EP0CS |= 0x80;


        }
        break;

        case MPSSE_SIX:
        {
            EP0BUF[0] =0;
            EP0BUF[1] =1;
            EP0BCH = 0;
            EP0BCL = 1;
            SUDPTRL = 3;
            EP0CS |= 0x80;

        }
        break;


        default:
            return FALSE;


        }
        return TRUE;


}
