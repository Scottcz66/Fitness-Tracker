/*
 * DataAcquisition.c
 *The module that initialize the acclemeter and functions of get data from the acclemeter.
 *
 *  Created on: 21/04/2022
 *      Authors: Zheng chao Franco Lyu
 *  Group: 62
 */

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/i2c.h"
#include "acc.h"
#include "i2c_driver.h"
#include "circBufT.h"
#include "driverlib/adc.h"
#include <math.h>

typedef struct{
    int16_t x;
    int16_t y;
    int16_t z;
} vector3_t;



/********************************************************
//Initialize the acclemeter.
 ********************************************************/
void
initAccl (void)
{
    char    toAccl[] = {0, 0};  // parameter, value

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);
    SysCtlPeripheralReset(SYSCTL_PERIPH_I2C0);

    GPIOPinTypeI2C(I2CSDAPort, I2CSDA_PIN);
    GPIOPinTypeI2CSCL(I2CSCLPort, I2CSCL_PIN);
    GPIOPinConfigure(I2CSCL);
    GPIOPinConfigure(I2CSDA);

    I2CMasterInitExpClk(I2C0_BASE, SysCtlClockGet(), true);

    GPIOPinTypeGPIOInput(ACCL_INT2Port, ACCL_INT2);

    toAccl[0] = ACCL_DATA_FORMAT;
    toAccl[1] = (ACCL_RANGE_2G | ACCL_FULL_RES);
    I2CGenTransmit(toAccl, 1, WRITE, ACCL_ADDR);

    toAccl[0] = ACCL_PWR_CTL;
    toAccl[1] = ACCL_MEASURE;
    I2CGenTransmit(toAccl, 1, WRITE, ACCL_ADDR);


    toAccl[0] = ACCL_BW_RATE;
    toAccl[1] = ACCL_RATE_100HZ;
    I2CGenTransmit(toAccl, 1, WRITE, ACCL_ADDR);

    toAccl[0] = ACCL_INT;
    toAccl[1] = 0x00;
    I2CGenTransmit(toAccl, 1, WRITE, ACCL_ADDR);

    toAccl[0] = ACCL_OFFSET_X;
    toAccl[1] = 0x00;
    I2CGenTransmit(toAccl, 1, WRITE, ACCL_ADDR);

    toAccl[0] = ACCL_OFFSET_Y;
    toAccl[1] = 0x00;
    I2CGenTransmit(toAccl, 1, WRITE, ACCL_ADDR);

    toAccl[0] = ACCL_OFFSET_Z;
    toAccl[1] = 0x00;
    I2CGenTransmit(toAccl, 1, WRITE, ACCL_ADDR);
}


/********************************************************
 * Function to read accelerometer
 ********************************************************/
vector3_t
getAccl (void)
{
    char fromAccl[] = {0, 0, 0, 0, 0, 0, 0};
    vector3_t acceleration;
    uint8_t bytesToRead = 6;

    fromAccl[0] = ACCL_DATA_X0;
    I2CGenTransmit(fromAccl, bytesToRead, READ, ACCL_ADDR);
    acceleration.x = (fromAccl[2] << 8) | fromAccl[1];
    acceleration.y = (fromAccl[4] << 8) | fromAccl[3];
    acceleration.z = (fromAccl[6] << 8) | fromAccl[5];

    return acceleration;
}


/********************************************************
//Caculate the mean value of buffer which store data from the acclemeter.
 ********************************************************/
int16_t getAccl_Mean(circBuf_t* dataBuff, int buffer_size) {
    int16_t i;
    int16_t sum = 0;

    for (i = 0; i < buffer_size; i++) {
        sum = sum + readCircBuf(dataBuff);
    }

    sum = (2 * sum + buffer_size) / 2 / buffer_size;

    return sum;
}



/********************************************************
//calculate the angle for step detection.
 ********************************************************/
int16_t getNorm(vector3_t acc) {

    int16_t x = acc.x *100 / 230;
    int16_t y = acc.y *100 / 230;
    int16_t z = acc.z *100 / 230;
    int16_t norm = sqrt(x * x + y * y + z * z);

    return norm;
}
