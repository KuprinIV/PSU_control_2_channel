/*
 * mcp4725.c
 *
 *  Created on: Nov 26, 2024
 *      Author: Ilya
 */
#include "mcp4725.h"

/**
 * @brief MCP4725 check I2C connection
 * @param dev_address - I2C device address
 * @retval 0 - device not connected, 1 - device is connected
 */
uint8_t MCP4725_check(I2C_HandleTypeDef* hi2c, uint8_t dev_address)
{
	uint8_t res = 0;
	if(HAL_I2C_IsDeviceReady(hi2c, (dev_address<<1), 3, 100) == HAL_OK)
	{
		res = 1;
	}
	return res;
}

/**
 * @brief MCP4725 write DAC register value
 * @param dev_address - I2C device address
 * @param dac_value - DAC register value to write
 * @param is_save_in_eeprom - save DAC value into EEPROM flag: 0 - don't save it, 1 - save it
 * @retval None
 */
void MCP4725_writeDAC_register(I2C_HandleTypeDef* hi2c, uint8_t dev_address, uint16_t dac_value, uint8_t is_save_in_eeprom)
{
	uint8_t data[3] = {0};
	// store data bytes
	// bit 7 - C2, bit 6 - C1, bit 5 - C0, bit 4 - don't care, bit 3 - don't care, bit 2 - PD1, bit 1 - PD0, bit 0 - don't care
	// C2C1C0: 010 - write DAC register, 011 - write DAC register and EEPROM
	// PD1PD0: 00 - normal mode, 01 - power down and out is pulled 1k to ground, 10 - 01 - power down and out is pulled 100k to ground, 11 - power down and out is pulled 500k to ground
	data[0] = (is_save_in_eeprom) ? (0x60) : (0x40);
	data[1] = (uint8_t)(dac_value>>4);
	data[2] = (uint8_t)((dac_value & 0x000F)<<4);
	HAL_I2C_Master_Transmit(hi2c, (dev_address<<1), data, sizeof(data), 1000);
}

/**
 * @brief MCP4725 read DAC register and EEPROM data values
 * @param dev_address - I2C device address
 * @param data_type - result output value: DAC_STATUS - return status byte, DAC_VALUE - return DAC value,
 * DAC_EEPROM_VALUE - return last saved in EEPROM DAC value
 * @retval is selected by data_type value
 */
uint16_t MCP4725_readDAC_register_and_EEPROM(I2C_HandleTypeDef* hi2c, uint8_t dev_address, DacDataType data_type)
{
	uint8_t data[5] = {0};
	uint8_t status = 0;
	uint16_t dac_value = 0, eeprom_value = 0;
	uint16_t result = 0;

	// read DAC EEPROM data
	if(HAL_I2C_Master_Receive(hi2c, (dev_address<<1), data, sizeof(data), 1000) == HAL_OK)
	{
		status = data[0];
		dac_value = (uint16_t)((data[1]<<4)|(data[2]>>4));
		eeprom_value = (uint16_t)(((data[3] & 0x0F)<<8)|data[4]);
	}

	switch(data_type)
	{
		case DAC_STATUS:
			result = (uint16_t)status;
			break;

		case DAC_VALUE:
		default:
			result = dac_value;
			break;

		case DAC_EEPROM_VALUE:
			result = eeprom_value;
			break;
	}

	return result;
}

