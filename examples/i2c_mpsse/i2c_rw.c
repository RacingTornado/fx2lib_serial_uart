#include <fx2regs.h>
#include <fx2macros.h>
#include <serial.h>
#include <delay.h>

void i2c_bitbang()
{

__asm
anl _IE, #0x7f
orl _OEA,#0x28
;cpl _PA3
anl _IE, #0x7f
;mov a,_IOA
;mov a,0x80
xrl 0x80,#0x20
;clr _PA3;
;anl _PA3, #0xf7
mov r1, #08
mov a , #44
0001$:
rrc a;
;clr _PA5;
;anl _PA5, #0xdf
jc 0003$;
;clr _PA3;
;anl _PA3,#0xf7
ljmp 0002$;
0003$:
;setb _PA3;
;orl _PA3,#0x08
0002$:
;setb _PA5;
;orl _PA5,#0x20
djnz r1, 0001$
orl _IE,#0x80
__endasm;


}


void temp_call()
{
__asm

cpl a
cpl a

__endasm;

}
