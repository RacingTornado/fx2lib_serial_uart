#include <fx2regs.h>
#include <fx2macros.h>
#include <serial.h>
#include <delay.h>

void i2c_bitbang()
{

__asm
orl _OEA,#0x28
;clr _PA3;
anl _IE, #0x7f
;orl _IOA, #0xf7
mov r1, #08
mov a, #33
0001$:
rrc a;
;clr _PA5;
jc 0003$;
;clr _PA3;
ljmp 0002$;
0003$:
;setb _PA3;
0002$:
;setb _PA5;
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
