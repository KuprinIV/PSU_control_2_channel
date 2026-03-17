/*
 * ina226.c
 *
 *  Created on: 28 дек. 2024 г.
 *      Author: Ilya
 */

#include "ina226.h"

static void INA226_write_register(I2C_HandleTypeDef* hi2c, uint8_t dev_address, uint8_t reg_addr, uint16_t reg_value);
static uint16_t INA226_read_register(I2C_HandleTypeDef* hi2c, uint8_t dev_address, uint8_t reg_addr);
//static uint16_t INA226_correctMeasureVoltage(uint16_t mv);
//static uint16_t INA226_correctMeasureCurrent(int16_t mi);

/**
 * @brief Init INA226 parameters
 * @param None
 * @retval None
 */
void INA226_Init(I2C_HandleTypeDef* hi2c)
{
	/* set maximum expected current to 0,5 A
	 * Current_LSB = 4 A/2^15 = 122,07 uA. Round to 125 uA/bit
	 * Calibration register value: CAL = 0,00512/(Current_LSB*Rshunt)
	 * CAL = 0,00512/(125*10^(-6)*0,02) = 2048 = 0x800
	 */
	union INA226_ConfigReg conf_reg;

	// set INA226 configuration
	conf_reg.config.reset = 0;								// no reset
	conf_reg.config.reserved = CONFIG_RSVD;					// set default value to reserved bits
	conf_reg.config.averages_num = CONFIG_AVGS_16;			// set number of averages to 16
	conf_reg.config.vbus_conv_time = CONFIG_CT_1_1_MS;		// set Vbus conversion time to 1.1 ms
	conf_reg.config.shunt_conv_time = CONFIG_CT_1_1_MS;		// set Vshunt conversion time to 1.1 ms
	conf_reg.config.mode = CONFIG_MODE_SHUNT_AND_BUS_CONT;	// set continuous measure of Vbus and Vshunt mode

	// check is I2C device on bus
	if(HAL_I2C_IsDeviceReady(hi2c, (DEV_ADDR<<1), 3, 100) == HAL_OK)
	{
		// set configuration register
		INA226_write_register(hi2c, DEV_ADDR, CONFIG_REG, conf_reg.reg_value);

		// set calibration register
		INA226_write_register(hi2c, DEV_ADDR, CALIBRATION_REG, 2060);
	}
}

/**
 * @brief Read INA226 data: current, bus voltage and power
 * @param data - data structure pointer
 * @retval None
 */
void INA226_ReadData(I2C_HandleTypeDef* hi2c, INA226_Data* data)
{
	uint16_t bus_volt_data = 0;
	int16_t current_data = 0;
	uint16_t power_data = 0;

	// read bus voltage data
	bus_volt_data = INA226_read_register(hi2c, DEV_ADDR, BUS_VOLT_REG);

	// read current data
	current_data = (int16_t)INA226_read_register(hi2c, DEV_ADDR, CURRENT_REG);

	// read power data
	power_data = INA226_read_register(hi2c, DEV_ADDR, POWER_REG);

	// convert data
	data->voltage_mv = 5*bus_volt_data/4;//INA226_correctMeasureVoltage(bus_volt_data); //(uint16_t)((uint32_t)bus_volt_data*50079/40000 + 2);
	data->current_ma = current_data/8;//INA226_correctMeasureCurrent(current_data); //(current_data > 20) ? (current_data/8 + 13) : (current_data/8);
	data->power_mw = (uint16_t)((uint32_t)power_data*25/8);

}

/**
 * @brief INA226 write register value
 * @param dev_address - I2C device address
 * @param reg_addr - register address
 * @param reg_value - register value to write
 * @retval None
 */
static void INA226_write_register(I2C_HandleTypeDef* hi2c, uint8_t dev_address, uint8_t reg_addr, uint16_t reg_value)
{
	uint8_t data[3] = {0};
	// store data bytes
	data[0] = reg_addr;
	data[1] = (uint8_t)(reg_value>>8);
	data[2] = (uint8_t)(reg_value & 0x00FF);
	HAL_I2C_Master_Transmit(hi2c, (dev_address<<1), data, sizeof(data), 1000);
}

/**
 * @brief INA226 read register value
 * @param dev_address - I2C device address
 * @param reg_addr - register address
 * @retval register value
 */
static uint16_t INA226_read_register(I2C_HandleTypeDef* hi2c, uint8_t dev_address, uint8_t reg_addr)
{
	uint8_t data[2] = {0};
	HAL_I2C_Mem_Read(hi2c, (dev_address<<1), reg_addr, 1, data, 2, 1000);
	return (uint16_t)((data[0]<<8)|(data[1]));
}

///**
// * @brief INA226 correct measured bus voltage
// * @param mv - INA226 measured bus voltage value
// * @retval corrected bus voltage value
// */
//static uint16_t INA226_correctMeasureVoltage(uint16_t mv)
//{
//	if(mv > 0)
//	{
//		if(mv <= 240)
//		{
//			mv = (uint16_t)(((uint32_t)mv*83144 - 771)/65536);
//		}
//		else if(mv > 240 && mv <= 318)
//		{
//			mv = (uint16_t)(((uint32_t)mv*82489 - 727)/65536);
//		}
//		else if(mv > 318 && mv <= 1200)
//		{
//			mv = (uint16_t)(((uint32_t)mv*82095 - 946)/65536);
//		}
//		else
//		{
//			mv = (uint16_t)(((uint32_t)mv*82084 - 975)/65536);
//		}
//	}
//	return mv;
//}
//
///**
// * @brief INA226 correct measured current
// * @param mv - INA226 measured current value
// * @retval corrected current value
// */
//static uint16_t INA226_correctMeasureCurrent(int16_t mi)
//{
//	if(mi >= 0)
//	{
//		if(mi <= 256)
//		{
//			mi = (uint16_t)(((uint32_t)mi*9044 + 470548)/65536);
//		}
//		else if(mi > 256 && mi <= 11200)
//		{
//			mi = (uint16_t)(((uint32_t)mi*8268 + 660362)/65536);
//		}
//		else if(mi > 11200 && mi <= 14913)
//		{
//			mi = (uint16_t)(((uint32_t)mi*8225 + 1196249)/65536);
//		}
//		else if(mi > 14913 && mi <= 15200)
//		{
//			mi = (uint16_t)(((uint32_t)mi*8234 + 632479)/65536);
//		}
//		else
//		{
//			mi = (uint16_t)(((uint32_t)mi*8223 + 1232616)/65536);
//		}
//	}
//	else
//	{
//		mi = 0;
//	}
//	return mi;
//}
