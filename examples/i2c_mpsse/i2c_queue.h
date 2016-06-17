
//Modify softuart.h and remove the ATMEGA definitions
#ifndef I2CUART
#define I2CUART




void QueueInitTX(void);
__bit I2CPutTX(unsigned char data);
__bit I2CGetTx(unsigned char *old);
__bit I2CCheckTX();

void I2CInitRX(void);
__bit I2CPutRX(unsigned char data);
__bit I2CGetRX(unsigned char *old);
__bit I2CCheckRX();





#define softuart_puts_P(s___) softuart_puts_p(PSTR(s___))
#endif
