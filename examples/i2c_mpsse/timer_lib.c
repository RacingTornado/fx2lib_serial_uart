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

struct timer
{
    unsigned short expiry;
    unsigned short periodic;
    void (*callback)(void *);
    void *arg;
};

__xdata static struct timer fx2_timer[MAX_TIMERS];
__xdata static volatile unsigned short fx2_tick = 0;



void timerlib_init()
{


}
