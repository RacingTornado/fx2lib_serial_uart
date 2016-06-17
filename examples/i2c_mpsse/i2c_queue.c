/*
I2C Queue for reading and writing data;;
*/


#define I2C_ELEMENTS 5
#define I2C_SIZE (I2C_ELEMENTS + 1)
__xdata unsigned char I2C_Data_TX[I2C_SIZE];
__xdata unsigned char I2CInTX, I2COutTx;

__xdata unsigned char I2C_Data_RX[I2C_SIZE];
__xdata unsigned char I2CInRX, I2COutRX;






//This function is called periodically from main
void i2c_service()
{
//Data has been loaded by the calling function in a buffer
//We need to move the data and set the flag
//QueuePutTX(0x33);
//QueueGetTX(&tx_buffer);
//fast_uart(tx_buffer);
//Check if operation is ongoing






}

















/* Very simple queue
 * These are FIFO queues which discard the new data when full.
 *
 * Queue is empty when in == out.
 * If in != out, then
 *  - items are placed into in before incrementing in
 *  - items are removed from out before incrementing out
 * Queue is full when in == (out-1 + QUEUE_SIZE) % QUEUE_SIZE;
 *
 * The queue will hold QUEUE_ELEMENTS number of items before the
 * calls to QueuePut fail.
 */



void I2CInit(void)
{
    I2CInTX = I2COutTx = 0;
}

__bit I2CPutTX(unsigned char data)
{
    if(I2CInTX == (( I2COutTx - 1 + I2C_SIZE) % I2C_SIZE))
    {
        return 1; /* Queue Full*/
    }

    I2C_Data_TX[I2CInTX] = data;

    I2CInTX = (I2CInTX + 1) % I2C_SIZE;

    return 0; // No errors
}

__bit I2CGetTX(unsigned char *old)
{
    if((I2CInTX == I2COutTx))
    {
        return 1; /* Queue Empty - nothing to get*/
    }

    *old = I2C_Data_TX[I2COutTx];

    I2COutTx = (I2COutTx + 1) % I2C_SIZE;

    return 0; // No errors
}


__bit I2CCheckTX()
{

        //fast_uart(QueueInTX);
        //        fast_uart(QueueOutTX);


        if((I2CInTX == I2COutTx))
    {
        return 1; /* Queue Empty - nothing to get*/
    }

    return 0; // No errors
}




void I2CInitRX(void)
{
    I2CInRX = I2COutRX = 0;
}

__bit I2CPutRX(unsigned char data)
{
    if(I2CInRX == (( I2COutRX - 1 + I2C_SIZE) % I2C_SIZE))
    {
        return 1; /* Queue Full*/
    }

    I2C_Data_RX[I2CInRX] = data;

    I2CInRX = (I2CInRX + 1) % I2C_SIZE;

    return 0; // No errors
}

__bit I2CGetRX(unsigned char *old)
{
    if(I2CInRX == I2COutRX)
    {
        return 1; /* Queue Empty - nothing to get*/
    }

    *old = I2C_Data_RX[I2COutRX];

    I2COutRX = (I2COutRX + 1) % I2C_SIZE;

    return 0; // No errors
}

//This function checks if Queue is full . If full, it returns 1
__bit I2CCheckRX()
{
    if(I2CInRX == (( I2COutRX - 1 + I2C_SIZE) % I2C_SIZE))
    {
        return 1; /* Queue Full*/
    }

    return 0; // No errors
}
