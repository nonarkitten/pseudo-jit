#include <stdio.h>
#include <stdint.h>
#include <string.h>

void I2C0Init(uint32_t bus_speed);

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
int32_t  I2C0SendCmd(uint32_t slave, uint8_t* cmd, uint32_t cmd_len, uint8_t * data, uint32_t cntr);

/* Version to fix GreenPAK */
int32_t  I2C0SendCmd_GPFix(uint32_t slave, uint8_t* cmd, uint32_t cmd_len, uint8_t * data, uint32_t cntr);

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
int32_t  I2C0ReadCmd(uint32_t slave, uint8_t* cmd, uint32_t cmd_len, uint8_t * data, uint32_t cntr);

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
int32_t  I2C0Probe(uint32_t slave);