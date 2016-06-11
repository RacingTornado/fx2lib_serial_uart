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
#include "timer_lib.h"

extern void fast_uart(unsigned char a);

struct timer
{
    //The number of ticks after which the task should fire.
    //This keeps getting incremented until it finally rolls over and starts back
    //A short is about 2 bytes on SDCC. So 65535 is the maximum count it can measure.
    unsigned short expiry;
    //The period after which the timer should fire. This field is a constant
    unsigned short periodic;
    void (*callback)(void *);
};

static __xdata struct timer fx2_timer[MAX_TIMERS];
//Incremented via ISR
__xdata  volatile unsigned short fx2_tick = 0;
static __xdata unsigned char to_load;
__xdata unsigned char interval;
static __xdata unsigned char i;


__xdata short periodic;
void (*callback)(void *);

void timerlib_init(CLK_SPD clk)
{

    CKCON |= CKCON_T0;
    TMOD  |= T0_MODE_REL;
    switch(clk)
    {
        case CLK_12M:
        {
            to_load = (interval*1000)/(83*4);
            to_load = 0xff - to_load;
            fast_uart(to_load);
        }
        break;
        case CLK_24M:
        {
            to_load = (interval*1000)/(83*2);
            to_load = 0xff - to_load;
            fast_uart(to_load);
        }
        break;
        case CLK_48M:
        {
            to_load = (interval*1000)/(83);
            to_load = 0xff - to_load;
            fast_uart(to_load);

        }
        break;
        default:
        {

        }


    }

    TH0 = to_load;
    TL0 = to_load;


}



void create_timer()

{

    //The following parameters need to be set before a call to this function.
    //int periodic, void (*callback)(void *), void *arg
    /* Find a free timer */
    for (i = 0; i < MAX_TIMERS; i++) {
        if (_timer[i].callback == NULL) {
            break;
        }
    }

    /* Make sure a valid timer is found */
    if (i < MAX_TIMERS) {
        DISABLE_INTERRUPTS();
        /* Set up the timer */
        //The periodic holds the number of ticks after which the timer should fire.
        if (periodic != 0) {
            _timer[i].periodic = periodic ;
        } else {
            _timer[i].periodic = 0;
        }

       _timer[i].callback = callback;
       _timer[i].expiry = _timer_tick + _timer[i].periodic;

       ENABLE_INTERRUPTS();
       handle = i;
    }



}

//Delete timer which has type handle.
void delete_timer(__xdata unsigned char handle)
{
    if (handle < MAX_TIMERS) {
        DISABLE_INTERRUPTS();
        /* Clear the callback to delete the timer */
        _timer[handle].callback = NULL;
        ENABLE_INTERRUPTS();
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

void service_timer()
{


     for (i = 0; i < MAX_TIMERS; i++) {
        /* If the timer is enabled and expired, invoke the callback */
        //If a valid callback exists, and the value of count in the timer expiry
        //has reached timer_tick. This may not always work, so we change the value to greater than
        if ((_timer[i].callback != NULL) && (_timer_tick > _timer[i].expiry )) {
            _timer[i].callback();

            if (_timer[i].periodic > 0) {
                /* Timer is periodic, calculate next expiration */
                _timer[i].expiry += _timer[i].periodic;
            } else {
                /* If timer is not periodic, clear the callback to disable */
                _timer[i].callback = NULL;
            }
        }
    }

}

