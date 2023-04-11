/*
 * hw_init.c
 *
 *  Created on: Feb 9, 2022
 *      Author: Renee Cousins
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "ioam3358.h"

#define SYSTEM_CLOCK 48000000

// I2C CON register bit definition
#define CON_I2C_EN (1 << 15)

#define CON_STB  (1 << 11)
#define CON_MST  (1 << 10)
#define CON_TRX  (1 << 9)
#define CON_XSA  (1 << 8)
#define CON_XOA0 (1 << 7)
#define CON_XOA1 (1 << 6)
#define CON_XOA2 (1 << 5)
#define CON_XOA3 (1 << 4)
#define CON_STP  (1 << 1)
#define CON_STT  (1 << 0)

#define INTERNAL_CLOCK 12000000

void I2C0Init(uint32_t bus_speed) {
    uint32_t divider = (INTERNAL_CLOCK / 2) / bus_speed;

    /*Enable Interface Clock*/
    CM_WKUP_CLKSTCTRL->BIT.CLKTRCTRL = 2;
    CM_WKUP_L4WKUP_CLKCTRL->BIT.MODULEMODE = 2;
    /*Enable Func Clock*/
    CM_WKUP_I2C0_CLKCTRL->BIT.MODULEMODE = 2;
    /*Wait clocks to get active*/
    while(CM_WKUP_I2C0_CLKCTRL->BIT.IDLEST ||
        CM_WKUP_L4WKUP_CLKCTRL->BIT.IDLEST);

    /*Reset I2C0 module*/
    I2C0_SYSC->BIT.SRST  = 1;
    while(I2C0_SYSC->BIT.SRST);
    /*Disable auto idle*/
    I2C0_SYSC->BIT.AUTOIDLE  = 0;
    /*Prescaler set to 2. ICLK is 24MHz*/
    I2C0_PSC->LONG = (SYSTEM_CLOCK / INTERNAL_CLOCK) - 1;
    /*Set SCLL */
    I2C0_SCLL->LONG = divider - 7;
    /*Set SCLH */
    I2C0_SCLH->LONG = divider - 5;

    /*Pin Config*/

    /*Enable Control module clocks*/
    CM_WKUP_CLKSTCTRL->BIT.CLKTRCTRL = 2;
    CM_WKUP_CONTROL_CLKCTRL->BIT.MODULEMODE = 2;
    /*Wait clock to get active*/
    while(CM_WKUP_CONTROL_CLKCTRL->BIT.IDLEST);
    
    /*I2C0 Enable*/
    I2C0_CON->BIT.I2C_EN = 1;

    I2C0_OA->BIT.OA = 1;

    /*Disable interrupts*/
    I2C0_IRQENABLE_CLR->LONG = I2C_IRQ_ARDY | I2C_IRQ_NACK | I2C_IRQ_AL;
}

/*************************************************************************
 * Function Name: I2C0Send
 * Parameters: uint32_t slave - slave address
 *             uint8_t * data - data pointer
 *             uint32_t cntr  - data size
 *             uint32_t stop  - 0 do not send stop condition
 *                              1 send stop condition
 *
 * Return: uint32_t   0 - send o.k.
 *                    1 - send error.
 *
 * Description: Sends data to slave
 *
 *************************************************************************/
int32_t  I2C0SendCmd(uint32_t slave, uint8_t* cmd, uint32_t cmd_len, uint8_t * data, uint32_t cntr) {
    int status = 0;

    /*Set Slave Addr*/
    I2C0_SA->BIT.SA = slave;
    /*Set data count*/
    I2C0_CNT->BIT.DCOUNT = cntr + cmd_len;
    /*Clear IRQ Status*/
    I2C0_IRQSTATUS->LONG = 0x7FFF;
    /*Set mode*/
    I2C0_CON->LONG = CON_I2C_EN | CON_TRX | CON_MST;

    /*Check for bus busy*/
    while(I2C0_IRQSTATUS_RAW->BIT.BB);

    /*Start command*/
    I2C0_CON->LONG |= CON_STT;

    while(cntr || cmd_len) {
        if(I2C0_IRQSTATUS_RAW->BIT.NACK
        || I2C0_IRQSTATUS_RAW->BIT.AL
        ) break;

        if(I2C0_IRQSTATUS_RAW->BIT.XRDY) {
            if(cmd_len) {
                I2C0_DATA->BIT.DATA = *cmd++;
                cmd_len--;
            } else {
                I2C0_DATA->BIT.DATA = *data++;
                cntr--;
            }
            I2C0_IRQSTATUS->BIT.XRDY = 1;
            if(cntr==0 && cmd_len==0) {
                /*Wait for bus access*/
                while(!I2C0_IRQSTATUS_RAW->BIT.ARDY);
                I2C0_IRQSTATUS->BIT.ARDY = 1;
            }
        }
    }

    /*Stop command*/
    I2C0_CON->LONG |= CON_STP;
    /*Wait for bus stop (free)*/
    while(!I2C0_IRQSTATUS_RAW->BIT.BF);
    /*Flush FIFO*/
    I2C0_BUF->BIT.RXFIFO_CLR = 1;
    I2C0_BUF->BIT.TXFIFO_CLR = 1;
    /*Clear IRQ Status*/
    I2C0_IRQSTATUS->LONG = 0x7FFF;
    /*Clear buffer length*/
    I2C0_CNT->BIT.DCOUNT = 0;
    return cntr == 0;
}

int32_t  I2C0SendCmd_GPFix(uint32_t slave, uint8_t* cmd, uint32_t cmd_len, uint8_t * data, uint32_t cntr) {
    int status = 0;

    /*Set Slave Addr*/
    I2C0_SA->BIT.SA = slave;
    /*Set data count*/
    I2C0_CNT->BIT.DCOUNT = cntr + cmd_len;
    /*Clear IRQ Status*/
    I2C0_IRQSTATUS->LONG = 0x7FFF;
    /*Set mode*/
    I2C0_CON->LONG = CON_I2C_EN | CON_TRX | CON_MST;

    /*Check for bus busy*/
    while(I2C0_IRQSTATUS_RAW->BIT.BB);

    /*Start command*/
    I2C0_CON->LONG |= CON_STT | CON_STP;

    while(cntr || cmd_len) {
        if(I2C0_IRQSTATUS_RAW->BIT.NACK
        || I2C0_IRQSTATUS_RAW->BIT.AL
        ) break;

        if(I2C0_IRQSTATUS_RAW->BIT.XRDY) {
            if(cmd_len) {
                I2C0_DATA->BIT.DATA = *cmd++;
                cmd_len--;
            } else {
                I2C0_DATA->BIT.DATA = *data++;
                cntr--;
            }
            I2C0_IRQSTATUS->BIT.XRDY = 1;
        }
    }
    
    /*Wait for bus stop (free)*/
    while(!I2C0_IRQSTATUS_RAW->BIT.BF);
    /*Flush FIFO*/
    I2C0_BUF->BIT.RXFIFO_CLR = 1;
    I2C0_BUF->BIT.TXFIFO_CLR = 1;
    /*Clear IRQ Status*/
    I2C0_IRQSTATUS->LONG = 0x7FFF;
    /*Clear buffer length*/
    I2C0_CNT->BIT.DCOUNT = 0;
    I2C0_CON->LONG = 0;
    return cntr == 0;
}

/*************************************************************************
 * Function Name: I2C0Read
 * Parameters: uint32_t slave - slave address
 *             uint8_t * data - data pointer
 *             uint32_t cntr  - data size
 *
 * Return: uint32_t   0 - read o.k.
 *                    1 - read error.
 *
 * Description:  Reads data from slave
 *
 *************************************************************************/
int32_t  I2C0ReadCmd(uint32_t slave, uint8_t* cmd, uint32_t cmd_len, uint8_t * data, uint32_t cntr)
{
    /*Set Slave Addr*/
    I2C0_SA->BIT.SA = slave;
    /*Set data count*/
    I2C0_CNT->BIT.DCOUNT = cmd_len;
    /*Clear IRQ Status*/
    I2C0_IRQSTATUS->LONG = 0x7FFF;
    /*Set mode*/
    I2C0_CON->LONG = CON_I2C_EN | CON_TRX | CON_MST;

    /*Check for bus busy*/
    while(I2C0_IRQSTATUS_RAW->BIT.BB);

    /*Start command*/
    I2C0_CON->LONG |= CON_STT;

    while(cmd_len) {
        if(I2C0_IRQSTATUS_RAW->BIT.NACK
        || I2C0_IRQSTATUS_RAW->BIT.AL
        ) break;

        if(I2C0_IRQSTATUS_RAW->BIT.XRDY) {
            I2C0_DATA->BIT.DATA = *cmd++;
            cmd_len--;
            I2C0_IRQSTATUS->BIT.XRDY = 1;
        }
    }

    /*Wait for bus access*/
    while(!I2C0_IRQSTATUS_RAW->BIT.ARDY);
    I2C0_IRQSTATUS->BIT.ARDY = 1;

    /*Set data count*/
    I2C0_CNT->BIT.DCOUNT = cntr;
    /*Restart command in read mode*/
    I2C0_CON->LONG = (I2C0_CON->LONG | CON_STT | CON_STP) & ~CON_TRX;

    while(cntr) {
        if(I2C0_IRQSTATUS_RAW->BIT.NACK
        || I2C0_IRQSTATUS_RAW->BIT.AL
        ) break;

        if(I2C0_IRQSTATUS_RAW->BIT.RRDY) {
            *data++ = I2C0_DATA->BIT.DATA;
            I2C0_IRQSTATUS->BIT.RRDY = 1;
            cntr--;
            if(!cntr) {
                /*Wait for bus access*/
                while(!I2C0_IRQSTATUS_RAW->BIT.ARDY);
                I2C0_IRQSTATUS->BIT.ARDY = 1;
            }
        }
    }

    // /*Stop command*/
    // I2C0_CON->LONG |= CON_STP;
    /*Wait for bus stop (free)*/
    while(!I2C0_IRQSTATUS_RAW->BIT.BF);
    /*Flush FIFO*/
    I2C0_BUF->BIT.RXFIFO_CLR = 1;
    I2C0_BUF->BIT.TXFIFO_CLR = 1;
    /*Clear IRQ Status*/
    I2C0_IRQSTATUS->LONG = 0x7FFF;
    /*Clear buffer length*/
    I2C0_CNT->BIT.DCOUNT = 0;
    return cntr == 0;
}

/*************************************************************************
 * Function Name: I2C0Probe
 * Parameters: uint32_t slave - slave address
 *
 * Return: uint32_t   0 - device absent
 *                    1 - device present
 *
 * Description:  Checks if slave is present
 *
 *************************************************************************/
int32_t  I2C0Probe(uint32_t slave) {
    int present = 0;
    /*Set Slave Addr*/
    I2C0_SA->BIT.SA = slave;
    /*Set data count*/
    I2C0_CNT->BIT.DCOUNT = 1;
    /*Clear IRQ Status*/
    I2C0_IRQSTATUS->LONG = 0x7FFF;
    /*Set mode*/
    I2C0_CON->LONG = CON_I2C_EN | CON_MST;

    /*Check for bus busy*/
    while(I2C0_IRQSTATUS_RAW->BIT.BB);

    /*Start command*/
    I2C0_CON->LONG |= CON_STT | CON_STP;

    while(1) {
        if(I2C0_IRQSTATUS_RAW->BIT.NACK
        || I2C0_IRQSTATUS_RAW->BIT.AL
        ) break;
        if(I2C0_IRQSTATUS_RAW->BIT.RRDY) {
            (void)I2C0_DATA->BIT.DATA;
            /*Wait for bus access*/
            while(!I2C0_IRQSTATUS_RAW->BIT.ARDY);
            present = 1;
            break;
        }
    }

    /*Wait for bus stop (free)*/
    while(!I2C0_IRQSTATUS_RAW->BIT.BF);
    /*Flush FIFO*/
    I2C0_BUF->BIT.RXFIFO_CLR = 1;
    I2C0_BUF->BIT.TXFIFO_CLR = 1;
    /*Clear IRQ Status*/
    I2C0_IRQSTATUS->LONG = 0x7FFF;
    /*Clear buffer length*/
    I2C0_CNT->BIT.DCOUNT = 0;

    return present;
}


