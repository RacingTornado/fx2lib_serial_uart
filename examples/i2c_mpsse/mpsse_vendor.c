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

BOOL handle_mpsse()
{

toggle_pins();
SUDPTRCTL = 0x01;
        switch (SETUPDAT[0])
        {

        case MPSSE_BITBANG:
            {

                EP0CS |= 0x80;
                EP0BUF[0]=01;
                EP0BUF[1]= 06;
                EP0BCH=0;
                EP0BCL=2;
                SUDPTRL = 03;
            }
            break;
        default:
            return FALSE;


        }
        return TRUE;


}
