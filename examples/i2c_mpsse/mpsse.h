#ifndef MPSSE_VENDOR
#define MPSSE_VENDOR

#define FTDI_RESET 0x00
#define VENDOR_COMMAND 0x40
#define MPSSE_TWO 0x03
#define SET_LATENCY 0x09
#define SIO_SET_BITMODE_REQUEST 0x0b /* SIO_SET_BITMODE_REQUEST */
#define MPSSE_FIVE 0x0c
#define MPSSE_SIX 0x06


//Taken from ftdi.h
#define MPSSE_WRITE_NEG 0x01   /* Write TDI/DO on negative TCK/SK edge*/
#define MPSSE_BITMODE   0x02   /* Write bits, not bytes */
#define MPSSE_READ_NEG  0x04   /* Sample TDO/DI on negative TCK/SK edge */
#define MPSSE_LSB       0x08   /* LSB first */
#define MPSSE_DO_WRITE  0x10   /* Write TDI/DO */
#define MPSSE_DO_READ   0x20   /* Read TDO/DI */
#define MPSSE_WRITE_TMS 0x40   /* Write TMS/CS */

/* FTDI MPSSE commands */
#define SET_BITS_LOW   0x80
/*BYTE DATA*/
/*BYTE Direction*/
#define SET_BITS_HIGH  0x82
/*BYTE DATA*/
/*BYTE Direction*/
#define GET_BITS_LOW   0x81
#define GET_BITS_HIGH  0x83
#define LOOPBACK_START 0x84
#define LOOPBACK_END   0x85
#define TCK_DIVISOR    0x86
/* H Type specific commands */
#define DIS_DIV_5       0x8a
#define EN_DIV_5        0x8b
#define EN_3_PHASE      0x8c
#define DIS_3_PHASE     0x8d
#define CLK_BITS        0x8e
#define CLK_BYTES       0x8f
#define CLK_WAIT_HIGH   0x94
#define CLK_WAIT_LOW    0x95
#define EN_ADAPTIVE     0x96
#define DIS_ADAPTIVE    0x97
#define CLK_BYTES_OR_HIGH 0x9c
#define CLK_BYTES_OR_LOW  0x9d








enum ftdi_mpsse_mode
    {
        BITMODE_RESET  = 0x00,
        BITMODE_BITBANG= 0x01,
        BITMODE_MPSSE  = 0x02,
        BITMODE_SYNCBB = 0x04,
        BITMODE_MCU    = 0x08,
        /* CPU-style fifo mode gets set via EEPROM */
        BITMODE_OPTO   = 0x10,
        BITMODE_CBUS   = 0x20,
        BITMODE_SYNCFF = 0x40,
        BITMODE_FT1284 = 0x80,
    };








#endif
