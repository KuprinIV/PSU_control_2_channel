/*
 * ina226.h
 *
 *  Created on: 28 дек. 2024 г.
 *      Author: Ilya
 */

#ifndef SRC_INA226_H_
#define SRC_INA226_H_

#include "stm32f4xx_hal.h"

typedef struct
{
	int16_t current_ma;
	uint16_t voltage_mv;
	uint32_t power_mw;
}INA226_Data;

typedef struct
{
	unsigned mode : 3;
	unsigned shunt_conv_time : 3;
	unsigned vbus_conv_time : 3;
	unsigned averages_num : 3;
	unsigned reserved : 3;
	unsigned reset : 1;
}INA226_Config;

union INA226_ConfigReg
{
	uint16_t reg_value;
	INA226_Config config;
};

#define DEV_ADDR						0x40

// INA226 registers
#define CONFIG_REG						0x00
#define SHUNT_VOLT_REG					0x01
#define BUS_VOLT_REG					0x02
#define POWER_REG						0x03
#define CURRENT_REG						0x04
#define CALIBRATION_REG					0x05
#define MASK_EN_REG						0x06
#define ALERT_LIMIT_REG					0x07
#define DIE_ID_REG						0xFF

// INA226 configuration
#define CONFIG_RST						0x01
#define CONFIG_RSVD						0x04
#define CONFIG_AVGS_1					0x00
#define CONFIG_AVGS_4					0x01
#define CONFIG_AVGS_16					0x02
#define CONFIG_AVGS_64					0x03
#define CONFIG_AVGS_128					0x04
#define CONFIG_AVGS_256					0x05
#define CONFIG_AVGS_512					0x06
#define CONFIG_AVGS_1024				0x07
#define CONFIG_CT_140_US				0x00
#define CONFIG_CT_204_US				0x01
#define CONFIG_CT_332_US				0x02
#define CONFIG_CT_588_US				0x03
#define CONFIG_CT_1_1_MS				0x04
#define CONFIG_CT_2_1_MS				0x05
#define CONFIG_CT_4_2_MS				0x06
#define CONFIG_CT_8_2_MS				0x07
#define CONFIG_MODE_PD_0				0x00
#define CONFIG_MODE_SHUNT_TRIG			0x01
#define CONFIG_MODE_BUS_TRIG			0x02
#define CONFIG_MODE_SHUNT_AND_BUS_TRIG	0x03
#define CONFIG_MODE_PD_1				0x04
#define CONFIG_MODE_SHUNT_CONT			0x05
#define CONFIG_MODE_BUS_CONT			0x06
#define CONFIG_MODE_SHUNT_AND_BUS_CONT	0x07

void INA226_Init(I2C_HandleTypeDef* hi2c);
void INA226_ReadData(I2C_HandleTypeDef* hi2c, INA226_Data* data);


#endif /* SRC_INA226_H_ */
