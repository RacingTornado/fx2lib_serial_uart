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

#define MAX_TIMERS 5

#define CKCON_T0 0x08
#define CKCON_T1 0x10
#define CKCON_T2 0x20
#define T0_MODE_REL 0x02

extern void fast_uart(unsigned char a);

struct timer
{
    unsigned short expiry;
    unsigned short periodic;
    void (*callback)(void *);
    void *arg;
};

static __xdata struct timer fx2_timer[MAX_TIMERS];
static __xdata  volatile unsigned short fx2_tick = 0;



void timerlib_init(CLK_SPD clk)
{

    CKCON |= CKCON_T0;
    TMOD  |= T0_MODE_REL;
    switch(clk)
    {
        case CLK_12M:
        {
            fast_uart(0x32);
        }
        break;
        case CLK_24M:
        {

        }
        break;
        case CLK_48M:
        {

        }
        break;
        default:
        {

        }


    }


}


void timer_interrupt_enable()
{
    ENABLE_TIMER0();

}

void timer_start()
{
    TR0 = 1;
}
