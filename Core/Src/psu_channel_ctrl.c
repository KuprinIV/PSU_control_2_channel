/*
 * psu_channel_ctrl.c
 *
 *  Created on: 15 февр. 2026 г.
 *      Author: Ilya
 */
#include "psu_channel_ctrl.h"
#include "mcp4725.h"
#include "ina226.h"
#include <string.h>
#include "main.h"

static void PSU_readCalibrationData(ChannelCalibrationData* ch1_calibration, ChannelCalibrationData* ch2_calibration);
static void PSU_saveChannelCalibrationData(PSU_Channel ch, ChannelCalibrationData* ch_calibration);
static uint8_t restoreValueFromCalibrationData(uint16_t value_in, uint16_t* array, uint16_t size, uint16_t step, uint16_t* value_out);
static void checkValueLimits(uint16_t* val, uint16_t min, uint16_t max);

static ChannelCtrl psu_ch1_ctrl = {0, 0, 0, 0, 0, 0, 0, 0xFFFF, 0xFFFF}, psu_ch2_ctrl = {0, 0, 0, 0, 0, 0, 0, 0xFFFF, 0xFFFF};
static ChannelCtrl* psu_channels_ctrl[2] = {&psu_ch1_ctrl, &psu_ch2_ctrl};

static ChannelCalibrationData psu_ch1_calibration, psu_ch2_calibration;
static ChannelCalibrationData* psu_calibration_data[2] = {&psu_ch1_calibration, &psu_ch2_calibration};

static PSU_MeasuredParams psu_ch1_meas_data = {0, 0, 0}, psu_ch2_meas_data = {0, 0, 0};

static PSU_CalibrationStatus psu_ch1_status = {0, 0, 0, 0}, psu_ch2_status = {0, 0, 0, 0};
static PSU_CalibrationStatus* psu_status[2] = {&psu_ch1_status, &psu_ch2_status};

static PSU_GPIO_CtrlPin psu_ch1_en_pin = {DC_EN_CH1_GPIO_Port, DC_EN_CH1_Pin}, psu_ch2_en_pin = {DC_EN_CH2_GPIO_Port, DC_EN_CH2_Pin};
static PSU_GPIO_CtrlPin* psu_en_gpio_pins[2] = {&psu_ch1_en_pin, &psu_ch2_en_pin};
static PSU_GPIO_CtrlPin psu_ch1_cc_cv_pin = {CC_CV_CH1_GPIO_Port, CC_CV_CH1_Pin}, psu_ch2_cc_cv_pin = {CC_CV_CH2_GPIO_Port, CC_CV_CH2_Pin};
static PSU_GPIO_CtrlPin* psu_cc_cv_gpio_pins[2] = {&psu_ch1_cc_cv_pin, &psu_ch2_cc_cv_pin};

extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c2;
I2C_HandleTypeDef* psu_control_i2cs[2] = {&hi2c2, &hi2c1};

static uint8_t is_calibration_mode = 0;
static PSU_Calibration psu_calibration_ctrl;

static uint16_t voltage_cal_measured_data[CALIBRATION_MEAS_DATA_AVGS] = {0};
static int16_t current_cal_measured_data[CALIBRATION_MEAS_DATA_AVGS] = {0};
static uint16_t voltageSetValSteps[3] = {VOLTAGE_SET_STEP_FINE, VOLTAGE_SET_STEP_COARSE, 1};
static int16_t currentSetValSteps[3] = {CURRENT_SET_STEP_FINE, CURRENT_SET_STEP_COARSE, 1};

PSU_UI_ChannelData channel1_ui_data = {0, 0, &psu_ch1_ctrl, &psu_ch1_meas_data, &psu_ch1_status}, channel2_ui_data = {0, 0, &psu_ch2_ctrl, &psu_ch2_meas_data, &psu_ch2_status};
static PSU_UI_ChannelData* channels_ui_data[2] = {&channel1_ui_data, &channel2_ui_data};

/**
 * @brief PSU control initialize
 * @param None
 * @retval None
 */
void PSU_controlInit(void)
{
	// read calibration data
	PSU_readCalibrationData(&psu_ch1_calibration, &psu_ch2_calibration);

	// check DAC I2C connection and read data
	// channel 1
	if(MCP4725_check(psu_control_i2cs[PSU_CHANNEL_1], VOLTAGE_DAC_ADDR))
	{
		psu_ch1_ctrl.voltageDacVal = MCP4725_readDAC_register_and_EEPROM(psu_control_i2cs[PSU_CHANNEL_1], VOLTAGE_DAC_ADDR, DAC_EEPROM_VALUE);
		if(!restoreValueFromCalibrationData(psu_ch1_ctrl.voltageDacVal, psu_ch1_calibration.voltageDacCalibrationNodes, psu_ch1_calibration.voltageStepsNum, psu_ch1_calibration.voltageDacStep, &psu_ch1_ctrl.voltageSetVal))
		{
			psu_ch1_ctrl.voltageSetVal = psu_ch1_ctrl.voltageDacVal;
		}
	}

	if(MCP4725_check(psu_control_i2cs[PSU_CHANNEL_1], CURRENT_DAC_ADDR))
	{
		psu_ch1_ctrl.currentDacVal = MCP4725_readDAC_register_and_EEPROM(psu_control_i2cs[PSU_CHANNEL_1], CURRENT_DAC_ADDR, DAC_EEPROM_VALUE);
		if(!restoreValueFromCalibrationData(psu_ch1_ctrl.currentDacVal, psu_ch1_calibration.currentDacCalibrationNodes, psu_ch1_calibration.currentStepsNum, psu_ch1_calibration.currentDacStep, &psu_ch1_ctrl.currentSetVal))
		{
			psu_ch1_ctrl.currentSetVal = psu_ch1_ctrl.currentDacVal;
		}
	}

	// channel 2
	if(MCP4725_check(psu_control_i2cs[PSU_CHANNEL_2], VOLTAGE_DAC_ADDR))
	{
		psu_ch2_ctrl.voltageDacVal = MCP4725_readDAC_register_and_EEPROM(psu_control_i2cs[PSU_CHANNEL_2], VOLTAGE_DAC_ADDR, DAC_EEPROM_VALUE);
		if(!restoreValueFromCalibrationData(psu_ch2_ctrl.voltageDacVal, psu_ch2_calibration.voltageDacCalibrationNodes, psu_ch2_calibration.voltageStepsNum, psu_ch2_calibration.voltageDacStep, &psu_ch2_ctrl.voltageSetVal))
		{
			psu_ch2_ctrl.voltageSetVal = psu_ch2_ctrl.voltageDacVal;
		}
	}

	if(MCP4725_check(psu_control_i2cs[PSU_CHANNEL_2], CURRENT_DAC_ADDR))
	{
		psu_ch2_ctrl.currentDacVal = MCP4725_readDAC_register_and_EEPROM(psu_control_i2cs[PSU_CHANNEL_2], CURRENT_DAC_ADDR, DAC_EEPROM_VALUE);
		if(!restoreValueFromCalibrationData(psu_ch2_ctrl.currentDacVal, psu_ch2_calibration.currentDacCalibrationNodes, psu_ch2_calibration.currentStepsNum, psu_ch2_calibration.currentDacStep, &psu_ch2_ctrl.currentSetVal))
		{
			psu_ch2_ctrl.currentSetVal = psu_ch2_ctrl.currentDacVal;
		}
	}

	// init INA226 ICs
	// channel 1
	INA226_Init(psu_control_i2cs[PSU_CHANNEL_1]);

	// channel 2
	INA226_Init(psu_control_i2cs[PSU_CHANNEL_2]);
}

/**
 * @brief Read from the flash PSU channel calibration data to set and measure output voltage and current limit
 * @param ch1_calibration - data structure with channel 1 calibration parameters and data
 * @param ch2_calibration - data structure with channel 2 calibration parameters and data
 * @retval None
 */
static void PSU_readCalibrationData(ChannelCalibrationData* ch1_calibration, ChannelCalibrationData* ch2_calibration)
{
	if(ch1_calibration == NULL || ch2_calibration == NULL) return;

// try to read channel 1 calibration data stored into the flash
	uint16_t channel1_id = *(uint16_t*)(CH1_CAL_DATA_ADDR);
	uint16_t voltage_steps_num_ch1 = *(uint16_t*)(CH1_CAL_DATA_ADDR+2);
	uint16_t current_steps_num_ch1 = *(uint16_t*)(CH1_CAL_DATA_ADDR+4);
	uint16_t voltage_dac_step_ch1 = *(uint16_t*)(CH1_CAL_DATA_ADDR+6);
	uint16_t current_dac_step_ch1 = *(uint16_t*)(CH1_CAL_DATA_ADDR+8);
	uint16_t voltage_meas_step_ch1 = *(uint16_t*)(CH1_CAL_DATA_ADDR+10);
	uint16_t current_meas_step_ch1 = *(uint16_t*)(CH1_CAL_DATA_ADDR+12);

	uint16_t ch1_calibration_data_size = 14 + 4*(voltage_steps_num_ch1 + current_steps_num_ch1); // in bytes

	// check is PSU channel calibration data stored into the flash
	if(((channel1_id != 0xFFFF) && (voltage_steps_num_ch1 != 0xFFFF) && (current_steps_num_ch1 != 0xFFFF) &&
			(voltage_dac_step_ch1 != 0xFFFF) && (current_dac_step_ch1 != 0xFFFF) &&
			(voltage_meas_step_ch1 != 0xFFFF) && (current_meas_step_ch1 != 0xFFFF)) &&
			(ch1_calibration_data_size == sizeof(ChannelCalibrationData)))
	{
		memcpy(ch1_calibration, (uint32_t*)(CH1_CAL_DATA_ADDR), ch1_calibration_data_size);
	}
	else
	{
		// fill calibration data by default values
		ch1_calibration->channel_id = PSU_CHANNEL_1;
		ch1_calibration->voltageStepsNum = VOLTAGE_CALIB_STEPS_NUM;
		ch1_calibration->currentStepsNum = CURRENT_CALIB_STEPS_NUM;
		ch1_calibration->voltageDacStep = VOLTAGE_DAC_CALIBRATION_STEP;
		ch1_calibration->currentDacStep = CURRENT_DAC_CALIBRATION_STEP;
		ch1_calibration->voltageMeasStep = VOLTAGE_MEAS_CALIBRATION_STEP;
		ch1_calibration->currentMeasStep = CURRENT_MEAS_CALIBRATION_STEP;

		for(uint16_t i = 0; i < VOLTAGE_CALIB_STEPS_NUM; i++)
		{
			ch1_calibration->voltageDacCalibrationNodes[i] = i*VOLTAGE_DAC_CALIBRATION_STEP;
			ch1_calibration->voltageMeasCalibrationNodes[i] = i*VOLTAGE_MEAS_CALIBRATION_STEP;
		}

		for(uint16_t i = 0; i < CURRENT_CALIB_STEPS_NUM; i++)
		{
			ch1_calibration->currentDacCalibrationNodes[i] = i*CURRENT_DAC_CALIBRATION_STEP;
			ch1_calibration->currentMeasCalibrationNodes[i] = i*CURRENT_MEAS_CALIBRATION_STEP;
		}
	}

// try to read channel 2 calibration data stored into the flash
	uint16_t channel2_id = *(uint16_t*)(CH2_CAL_DATA_ADDR);
	uint16_t voltage_steps_num_ch2 = *(uint16_t*)(CH2_CAL_DATA_ADDR+2);
	uint16_t current_steps_num_ch2 = *(uint16_t*)(CH2_CAL_DATA_ADDR+4);
	uint16_t voltage_dac_step_ch2 = *(uint16_t*)(CH2_CAL_DATA_ADDR+6);
	uint16_t current_dac_step_ch2 = *(uint16_t*)(CH2_CAL_DATA_ADDR+8);
	uint16_t voltage_meas_step_ch2 = *(uint16_t*)(CH2_CAL_DATA_ADDR+10);
	uint16_t current_meas_step_ch2 = *(uint16_t*)(CH2_CAL_DATA_ADDR+12);

	uint16_t ch2_calibration_data_size = 14 + 4*(voltage_steps_num_ch2 + current_steps_num_ch2); // in bytes

	// check is PSU channel calibration data stored into the flash
	if(((channel2_id != 0xFFFF) && (voltage_steps_num_ch2 != 0xFFFF) && (current_steps_num_ch2 != 0xFFFF) &&
			(voltage_dac_step_ch2 != 0xFFFF) && (current_dac_step_ch2 != 0xFFFF) &&
			(voltage_meas_step_ch2 != 0xFFFF) && (current_meas_step_ch2 != 0xFFFF)) &&
			(ch2_calibration_data_size == sizeof(ChannelCalibrationData)))
	{
		memcpy(ch2_calibration, (uint32_t*)(CH2_CAL_DATA_ADDR), ch2_calibration_data_size);
	}
	else
	{
		// fill calibration data by default values
		ch2_calibration->channel_id = PSU_CHANNEL_2;
		ch2_calibration->voltageStepsNum = VOLTAGE_CALIB_STEPS_NUM;
		ch2_calibration->currentStepsNum = CURRENT_CALIB_STEPS_NUM;
		ch2_calibration->voltageDacStep = VOLTAGE_DAC_CALIBRATION_STEP;
		ch2_calibration->currentDacStep = CURRENT_DAC_CALIBRATION_STEP;
		ch2_calibration->voltageMeasStep = VOLTAGE_MEAS_CALIBRATION_STEP;
		ch2_calibration->currentMeasStep = CURRENT_MEAS_CALIBRATION_STEP;

		for(uint16_t i = 0; i < VOLTAGE_CALIB_STEPS_NUM; i++)
		{
			ch2_calibration->voltageDacCalibrationNodes[i] = i*VOLTAGE_DAC_CALIBRATION_STEP;
			ch2_calibration->voltageMeasCalibrationNodes[i] = i*VOLTAGE_MEAS_CALIBRATION_STEP;
		}

		for(uint16_t i = 0; i < CURRENT_CALIB_STEPS_NUM; i++)
		{
			ch2_calibration->currentDacCalibrationNodes[i] = i*CURRENT_DAC_CALIBRATION_STEP;
			ch2_calibration->currentMeasCalibrationNodes[i] = i*CURRENT_MEAS_CALIBRATION_STEP;
		}
	}
}

/**
 * @brief Save to the flash PSU channel calibration data to set and measure output voltage and current limit
 * @param ch - PSU channel number (PSU_CHANNEL_0 or PSU_CHANNEL_1)
 * @param ch_calibration - data structure with channel calibration parameters and data
 * @retval None
 */
static void PSU_saveChannelCalibrationData(PSU_Channel ch, ChannelCalibrationData* ch_calibration)
{
	if(ch_calibration == NULL) return;

	uint16_t calibration_temp_data[CH2_CAL_DATA_OFFSET] = {0};
	uint32_t offset = CH2_CAL_DATA_OFFSET*(uint32_t)ch/2;
	uint32_t data_length_in_bytes = (uint32_t)sizeof(ChannelCalibrationData);

	// fill calibration data structure header
	ch_calibration->channel_id = (uint16_t)ch;
	ch_calibration->voltageStepsNum = VOLTAGE_CALIB_STEPS_NUM;
	ch_calibration->voltageDacStep = VOLTAGE_DAC_CALIBRATION_STEP;
	ch_calibration->voltageMeasStep = VOLTAGE_MEAS_CALIBRATION_STEP;
	ch_calibration->currentStepsNum = CURRENT_CALIB_STEPS_NUM;
	ch_calibration->currentDacStep = CURRENT_DAC_CALIBRATION_STEP;
	ch_calibration->currentMeasStep = CURRENT_MEAS_CALIBRATION_STEP;

	// store flash data into the temp buffer
	memcpy(calibration_temp_data, (uint32_t*)(CALIBRATION_DATA_PAGE_ADDR), sizeof(calibration_temp_data));

	// update channel calibration data in the temp buffer
	memcpy(calibration_temp_data+offset, ch_calibration, data_length_in_bytes);

	// erase flash memory page
	uint32_t page_error = HAL_OK;
	FLASH_EraseInitTypeDef page_erase_params;

	page_erase_params.NbSectors = 1;
	page_erase_params.Sector = CALIBRATION_DATA_PAGE_ADDR;
	page_erase_params.TypeErase = FLASH_TYPEERASE_SECTORS;

	if(HAL_FLASH_Unlock() == HAL_OK) // unlock flash memory
	{
		HAL_FLASHEx_Erase(&page_erase_params, &page_error);

		// if erase operation was successful, save updated calibration data
		if(page_error == 0xFFFFFFFF)
		{
			for(uint16_t i = 0; i < sizeof(calibration_temp_data)/2; i++)
			{
				if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, CALIBRATION_DATA_PAGE_ADDR+2*i, calibration_temp_data[i]) != HAL_OK)
				{
					psu_calibration_ctrl.error_code = CAL_PROGRAM_FLASH_ERR; // emit flash program error
					break;
				}
			}
		}
		else
		{
			psu_calibration_ctrl.error_code = CAL_ERASE_FLASH_ERR; // emit flash erase error
		}

		HAL_FLASH_Lock();
	}
	else
	{
		psu_calibration_ctrl.error_code = CAL_UNLOCK_FLASH_ERR; // emit flash unlock error
	}

	psu_calibration_ctrl.is_error = 1; // set error detection flag
	psu_calibration_ctrl.is_state_changed = 1; // set calibration state changed flag
}

/**
 * @brief Get the range of array element indices within which the value lies
 * @param value_in - value to check
 * @param array - data array pointer
 * @param size - array size
 * @param value_out - output value pointer
 * @retval 0 - output value isn't found, 1 - output value is found
 */
static uint8_t restoreValueFromCalibrationData(uint16_t value_in, uint16_t* array, uint16_t size, uint16_t step, uint16_t* value_out)
{
	uint16_t idx_min = 0, idx_max = 0;
	uint16_t node_min = 0, node_max = 0, nodes_range = 0, reminder = 0;

	// check pointers to null
	if(array == NULL || value_out == NULL)
	{
		return 0;
	}

	// check is input value in array values range
	if(value_in >= array[size-1])
	{
		idx_min = size-2;
		idx_max = size-1;
	}
	else
	{
		// try to find indices range
		for(uint16_t i = 0; i < size-1; i++)
		{
			if(value_in >= array[i] && value_in < array[i+1])
			{
				idx_min = i;
				idx_max = i + 1;
				break;
			}
		}
	}

	// calculate output value
	node_max = array[idx_max];
	node_min = array[idx_min];
	nodes_range = node_max - node_min;
	reminder = value_in - node_min;
	*value_out = step*idx_min + (uint16_t)((uint32_t)reminder*step/nodes_range);

	return 1;
}

/**
  * @brief  Check is set values in limits. if not, set limit value
  * @param  val - set value pointer
  * @param	min - minimum value limit
  * @param	max - maximum value limit
  * @retval None
  */
static void checkValueLimits(uint16_t* val, uint16_t min, uint16_t max)
{
	if(*val < min || *val > 32767) // account, that val is unsigned
	{
		*val = min;
	}
	else if(*val > max)
	{
		*val = max;
	}
}

/**
 * @brief PSU channel enable control
 * @param ch - PSU channel number (PSU_CHANNEL_0 or PSU_CHANNEL_1)
 * @param is_enabled: 0 - disabled, 1 - enable
 * @retval None
 */
void PSU_enableChannelCtrl(PSU_Channel ch, uint8_t is_enabled)
{
	HAL_GPIO_WritePin(psu_en_gpio_pins[ch]->port, psu_en_gpio_pins[ch]->pin, (is_enabled & 0x01));
	psu_channels_ctrl[ch]->is_enabled = (is_enabled & 0x01);
}

/**
 * @brief Set PSU channel output voltage
 * @param ch - PSU channel number (PSU_CHANNEL_0 or PSU_CHANNEL_1)
 * @param voltage_10mv - voltage value with 10 mV step (i.e. 100 is 1 V)
 * @retval None
 */
void PSU_setVoltage(PSU_Channel ch, uint16_t voltage_10mv)
{
	uint16_t index_min = 0;
	uint16_t index_max = 0;
	uint16_t node_max = 0;
	uint16_t node_min = 0;
	uint16_t nodes_range = 0;
	uint16_t reminder = 0;
	uint16_t voltage_dac_value = 0;

	// get voltage DAC calibration step
	uint16_t voltage_dac_cal_step = psu_calibration_data[ch]->voltageDacStep;

	// get calibration data index range
	index_min = voltage_10mv/voltage_dac_cal_step;

	if(index_min >= (psu_calibration_data[ch]->voltageStepsNum-2))
	{
		index_max = psu_calibration_data[ch]->voltageStepsNum-1;
		index_min = index_max - 1;
	}
	else
	{
		index_max = index_min + 1;
	}

	// get range between near calibration nodes
	node_max = psu_calibration_data[ch]->voltageDacCalibrationNodes[index_max];
	node_min = psu_calibration_data[ch]->voltageDacCalibrationNodes[index_min];
	nodes_range = node_max - node_min;

	// get DAC value
	reminder = voltage_10mv - voltage_dac_cal_step*index_min;
	voltage_dac_value = node_min + (uint16_t)((uint32_t)nodes_range*reminder/voltage_dac_cal_step);

	// write DAC value
	PSU_setRawVoltageDac(ch, voltage_dac_value);

}

/**
 * @brief Set PSU channel output current limit
 * @param ch - PSU channel number (PSU_CHANNEL_0 or PSU_CHANNEL_1)
 * @param current_1ma - current limit value with 1 mA step (i.e. 1000 is 1 A)
 * @retval None
 */
void PSU_setCurrentLimit(PSU_Channel ch, uint16_t current_1ma)
{
	uint16_t index_min = 0;
	uint16_t index_max = 0;
	uint16_t node_max = 0;
	uint16_t node_min = 0;
	uint16_t nodes_range = 0;
	uint16_t reminder = 0;
	uint16_t current_dac_value = 0;

	// get current DAC calibration step
	uint16_t current_dac_cal_step = psu_calibration_data[ch]->currentDacStep;

	// get calibration data index range
	index_min = current_1ma/current_dac_cal_step;

	if(index_min >= (psu_calibration_data[ch]->currentStepsNum-2))
	{
		index_max = psu_calibration_data[ch]->currentStepsNum-1;
		index_min = index_max - 1;
	}
	else
	{
		index_max = index_min + 1;
	}

	// get range between near calibration nodes
	node_max = psu_calibration_data[ch]->currentDacCalibrationNodes[index_max];
	node_min = psu_calibration_data[ch]->currentDacCalibrationNodes[index_min];
	nodes_range = node_max - node_min;

	// get DAC value
	reminder = current_1ma - current_dac_cal_step*index_min;
	current_dac_value = node_min + (uint16_t)((uint32_t)nodes_range*reminder/current_dac_cal_step);

	// write DAC value
	PSU_setRawCurrentDac(ch, current_dac_value);
}

/**
 * @brief Set PSU channel output voltage by raw DAC code
 * @param ch - PSU channel number (PSU_CHANNEL_0 or PSU_CHANNEL_1)
 * @param voltage_dac_code - voltage DAC code
 * @retval None
 */
void PSU_setRawVoltageDac(PSU_Channel ch, uint16_t voltage_dac_code)
{
	// limit max DAC code
	if(voltage_dac_code > 4095)
	{
		voltage_dac_code = 4095;
	}

	// write DAC value
	MCP4725_writeDAC_register(psu_control_i2cs[ch], VOLTAGE_DAC_ADDR, voltage_dac_code, 0);
	psu_channels_ctrl[ch]->voltageDacVal = voltage_dac_code;

	// update control data
	if(!channels_ui_data[ch]->is_calibration)
	{
		psu_channels_ctrl[ch]->voltageDacSaveValCntr = 0;
	}
	else
	{
		psu_calibration_ctrl.is_state_changed = 0;
		psu_calibration_ctrl.is_measured_data_ready = 0;
		psu_calibration_ctrl.average_measured_data_cntr = 0;
	}
}

/**
 * @brief Set PSU channel output current limit by raw DAC code
 * @param ch - PSU channel number (PSU_CHANNEL_0 or PSU_CHANNEL_1)
 * @param current_dac_code - current limit DAC code
 * @retval None
 */
void PSU_setRawCurrentDac(PSU_Channel ch, uint16_t current_dac_code)
{
	// limit max DAC code
	if(current_dac_code > 4095)
	{
		current_dac_code = 4095;
	}

	// write DAC value
	MCP4725_writeDAC_register(psu_control_i2cs[ch], CURRENT_DAC_ADDR, current_dac_code, 0);
	psu_channels_ctrl[ch]->currentDacVal = current_dac_code;

	// update control data
	if(!channels_ui_data[ch]->is_calibration)
	{
		psu_channels_ctrl[ch]->currentDacSaveValCntr = 0;
	}
	else
	{
		psu_calibration_ctrl.is_state_changed = 0;
		psu_calibration_ctrl.is_measured_data_ready = 0;
		psu_calibration_ctrl.average_measured_data_cntr = 0;
	}
}

/**
 * @brief Get PSU channel output voltage and current limit DAC codes
 * @param ch - PSU channel number (PSU_CHANNEL_0 or PSU_CHANNEL_1)
 * @param voltage_dac_code - voltage DAC code
 * @param current_dac_code - current limit DAC code
 * @retval None
 */
void PSU_getChannelDacValues(PSU_Channel ch, uint16_t* voltage_dac_code, uint16_t* current_dac_code)
{
	// get channel voltage DAC code
	if(voltage_dac_code != NULL)
	{
		*voltage_dac_code = psu_channels_ctrl[ch]->voltageDacVal;
	}

	// get channel current limit DAC code
	if(current_dac_code != NULL)
	{
		*current_dac_code = psu_channels_ctrl[ch]->currentDacVal;
	}
}

/**
 * @brief Get measured PSU channel output voltage
 * @param ch - PSU channel number (PSU_CHANNEL_0 or PSU_CHANNEL_1)
 * @param psu_mp - PSU measured output parameters data (voltage in mV and current in mA)
 * @retval None
 */
void PSU_measureOutputParameters(PSU_Channel ch, PSU_MeasuredParams* psu_mp)
{
	if(psu_mp == NULL) return;

	INA226_Data ina226_data = {0, 0, 0};
	uint32_t volt_meas_sum = 0;
	int32_t curr_meas_sum = 0;

	uint16_t volt_meas_cal_step = 0;
	uint16_t curr_meas_cal_step = 0;

	// get channel measured parameters
	INA226_ReadData(psu_control_i2cs[ch], &ina226_data);

	if(channels_ui_data[ch]->is_calibration)
	{
		psu_mp->voltage_mv = ina226_data.voltage_mv;
		psu_mp->current_ma = ina226_data.current_ma;

		if(!psu_calibration_ctrl.is_finished && !psu_calibration_ctrl.is_voltage_discharge)
		{
			// check average ready process
			if(psu_calibration_ctrl.average_measured_data_cntr < CALIBRATION_MEAS_DATA_AVGS)
			{
				// store measured data to average
				if(psu_calibration_ctrl.cal_type == PSU_VOLTAGE_CAL)
				{
					voltage_cal_measured_data[psu_calibration_ctrl.average_measured_data_cntr] = psu_mp->voltage_mv;
				}
				else
				{
					current_cal_measured_data[psu_calibration_ctrl.average_measured_data_cntr] = psu_mp->current_ma;
				}

				psu_calibration_ctrl.average_measured_data_cntr++;
			}
			else
			{
				if(!psu_calibration_ctrl.is_measured_data_ready)
				{
					// store DAC value and measured data to calibration data structure
					if(psu_calibration_ctrl.cal_type == PSU_VOLTAGE_CAL)
					{
						psu_calibration_data[psu_calibration_ctrl.channel]->voltageDacCalibrationNodes[psu_calibration_ctrl.currentStep] = psu_channels_ctrl[psu_calibration_ctrl.channel]->voltageDacVal;

						// get averaged measured data
						volt_meas_sum = 0;

						for(uint16_t i = 0; i < CALIBRATION_MEAS_DATA_AVGS; i++)
						{
							volt_meas_sum += voltage_cal_measured_data[i];
						}

						psu_calibration_data[psu_calibration_ctrl.channel]->voltageMeasCalibrationNodes[psu_calibration_ctrl.currentStep] = (uint16_t)(volt_meas_sum/CALIBRATION_MEAS_DATA_AVGS);
						volt_meas_sum = 0;
						memset(voltage_cal_measured_data, 0, sizeof(voltage_cal_measured_data)); // clear averaged data array
					}
					else
					{
						psu_calibration_data[psu_calibration_ctrl.channel]->currentDacCalibrationNodes[psu_calibration_ctrl.currentStep] = psu_channels_ctrl[psu_calibration_ctrl.channel]->currentDacVal;

						// get averaged measured data
						curr_meas_sum = 0;

						for(uint16_t i = 0; i < CALIBRATION_MEAS_DATA_AVGS; i++)
						{
							curr_meas_sum += current_cal_measured_data[i];
						}

						psu_calibration_data[psu_calibration_ctrl.channel]->currentMeasCalibrationNodes[psu_calibration_ctrl.currentStep] = (int16_t)(curr_meas_sum/CALIBRATION_MEAS_DATA_AVGS);
						curr_meas_sum = 0;
						memset(current_cal_measured_data, 0, sizeof(current_cal_measured_data)); // clear averaged data array
					}

					psu_calibration_ctrl.is_measured_data_ready = 1; // set measured data ready flag
					psu_calibration_ctrl.is_state_changed = 1; // set calibration state changed flag
				}
			}
		}
	}
	else
	{
// apply calibration
	// to the measured voltage
		volt_meas_cal_step = psu_calibration_data[ch]->voltageMeasStep;

		if(!restoreValueFromCalibrationData(ina226_data.voltage_mv, psu_calibration_data[ch]->voltageMeasCalibrationNodes, psu_calibration_data[ch]->voltageStepsNum, volt_meas_cal_step, &psu_mp->voltage_mv))
		{
			psu_mp->voltage_mv = ina226_data.voltage_mv;
		}

	// to the measured current
		curr_meas_cal_step = psu_calibration_data[ch]->currentMeasStep;

		if(!restoreValueFromCalibrationData(ina226_data.current_ma, (uint16_t*)psu_calibration_data[ch]->currentMeasCalibrationNodes, psu_calibration_data[ch]->currentStepsNum, curr_meas_cal_step, (uint16_t*)&psu_mp->current_ma))
		{
			psu_mp->current_ma = ina226_data.current_ma;
		}
	}

	// read channel CC/CV mode pin state
	psu_mp->is_current_limit = ((psu_cc_cv_gpio_pins[ch]->port->IDR & psu_cc_cv_gpio_pins[ch]->pin) ? 0 : 1); // 0 - CV mode, 1 - CC mode
}

/**
 * @brief PSU calibration mode control
 * @param is_enabled: 0 - disabled, 1 - enabled
 * @param ch - PSU channel number (PSU_CHANNEL_0 or PSU_CHANNEL_1)
 * @retval None
 */
void PSU_calibrationModeCtrl(PSU_Channel ch, uint8_t is_enabled, PSU_CalibrationType cal_type)
{
	is_calibration_mode = is_enabled;
	channels_ui_data[ch]->is_calibration = is_enabled; // send status to the UI

	if(is_enabled)
	{
		if(psu_channels_ctrl[ch]->is_enabled)
		{
			// init calibration control structure
			psu_calibration_ctrl.cal_type = cal_type; // set calibration type - output voltage or current
			psu_calibration_ctrl.channel = ch; // set channel to calibrate
			psu_calibration_ctrl.currentStep = 0; // init current calibration step position
			psu_calibration_ctrl.steps_num = (cal_type == PSU_VOLTAGE_CAL) ? VOLTAGE_CALIB_STEPS_NUM : CURRENT_CALIB_STEPS_NUM; // define maximum number of steps
			psu_calibration_ctrl.is_finished = 0; // reset calibration process finish flag
			psu_calibration_ctrl.average_measured_data_cntr = 0; // init measured data averages counter
			psu_calibration_ctrl.is_measured_data_ready = 0; // reset averaged measured data ready flag
			psu_calibration_ctrl.is_state_changed = 0; // reset calibration state changed flag
			psu_calibration_ctrl.is_error = 0; // reset error detection flag
			psu_calibration_ctrl.is_voltage_discharge = 0; // reset output voltage discharge flag
			psu_calibration_ctrl.error_code = CAL_SUCCESS;

			// set another DAC channel parameters
			if(cal_type == PSU_VOLTAGE_CAL)
			{
				PSU_setRawCurrentDac(ch, 100);
			}
			else
			{
				PSU_setRawVoltageDac(ch, 300);
			}
		}
		else
		{
			psu_calibration_ctrl.is_error = 1; // set error detection flag
			psu_calibration_ctrl.error_code = CAL_CHANNEL_NOT_ENABLED; // emit channel not enabled error
			psu_calibration_ctrl.is_state_changed = 1; // set calibration state changed flag
		}
	}
	else
	{
		if(psu_calibration_ctrl.is_finished)
		{
			// reset flags
			psu_calibration_ctrl.is_finished = 0;
			psu_calibration_ctrl.is_measured_data_ready = 0;
			psu_calibration_ctrl.is_state_changed = 0;

			// if calibration is finished save data into the flash
//			PSU_saveChannelCalibrationData(psu_calibration_ctrl.channel, psu_calibration_data[psu_calibration_ctrl.channel]);
		}
		else
		{
			// restore calibration data
			PSU_readCalibrationData(&psu_ch1_calibration, &psu_ch2_calibration);
		}
	}
}

/**
 * @brief Go the previous calibration step during calibration process
 * @param None
 * @retval None
 */
void PSU_goToThePrevCalibrationStep(void)
{
	if(is_calibration_mode)
	{
		if(psu_calibration_ctrl.currentStep > 0)
		{
			psu_calibration_ctrl.currentStep--;
		}
	}
}

/**
 * @brief Go the next calibration step during calibration process
 * @param None
 * @retval None
 */
void PSU_goToTheNextCalibrationStep(void)
{
	if(is_calibration_mode)
	{
		if(psu_calibration_ctrl.currentStep < (psu_calibration_ctrl.steps_num - 1))
		{
			psu_calibration_ctrl.currentStep++;
		}
		else
		{
			if(!psu_calibration_ctrl.is_state_changed)
			{
				if(psu_calibration_ctrl.cal_type == PSU_VOLTAGE_CAL && psu_channels_ctrl[psu_calibration_ctrl.channel]->voltageDacVal > 2000)
				{
					psu_calibration_ctrl.is_voltage_discharge = 1; // start smooth voltage discharge
				}
				else
				{
					psu_calibration_ctrl.is_finished = 1;
				}
				psu_calibration_ctrl.is_state_changed = 1;
			}
		}
	}
}

/**
 * @brief PSU handle controls state changing
 * @param controls - data structure pointer with PSU controls state
 * @retval None
 */
void PSU_handleControls(ControlsState* controls)
{
	// reset update registers
	channel1_ui_data.is_update_reg = 0;
	channel2_ui_data.is_update_reg = 0;

// channel 1
	if(channels_ui_data[PSU_CHANNEL_1]->is_calibration)
	{
		// voltage set encoder button press is go to the next step
		if(controls->voltageSetEnc[0].is_btn_state_changed)
		{
			controls->voltageSetEnc[0].is_btn_state_changed = 0;

			if(controls->voltageSetEnc[0].btn_state == Pressed)
			{
				channel1_ui_data.is_update_reg |= UI_UPDATE_SET_VOLT_STEP_MASK;

				if(psu_calibration_ctrl.is_finished)
				{
					if(psu_calibration_ctrl.cal_type == PSU_VOLTAGE_CAL)
					{
						PSU_calibrationModeCtrl(PSU_CHANNEL_1, 1, PSU_CURRENT_CAL); // after the voltage calibration go to the current calibration
					}
					else
					{
						PSU_calibrationModeCtrl(PSU_CHANNEL_1, 0, PSU_VOLTAGE_CAL); // stop channel calibration and save data
					}
				}
				else
				{
					PSU_goToTheNextCalibrationStep();
				}
			}
		}

		// current set encoder button press is go to the previous step
		if(controls->currentLimitSetEnc[0].is_btn_state_changed)
		{
			controls->currentLimitSetEnc[0].is_btn_state_changed = 0;

			if(controls->currentLimitSetEnc[0].btn_state == Pressed)
			{
				channel1_ui_data.is_update_reg |= UI_UPDATE_SET_CURR_STEP_MASK;
				PSU_goToThePrevCalibrationStep();
			}
			else if(controls->currentLimitSetEnc[0].btn_state == LongPressed)
			{
				if(!psu_calibration_ctrl.is_finished)
				{
					channel1_ui_data.is_update_reg |= UI_UPDATE_SET_CURR_STEP_MASK;
					PSU_calibrationModeCtrl(PSU_CHANNEL_1, 0, PSU_VOLTAGE_CAL); // stop channel calibration without data saving
				}
			}
		}

		// update calibration parameter value
		if(controls->voltageSetEnc[0].is_enc_state_changed)
		{
			controls->voltageSetEnc[0].is_enc_state_changed = 0;
			channel1_ui_data.is_update_reg |= UI_UPDATE_SET_VOLT_MASK;

			if(psu_calibration_ctrl.cal_type == PSU_VOLTAGE_CAL)
			{
				psu_channels_ctrl[PSU_CHANNEL_1]->voltageDacVal += controls->voltageSetEnc[0].counter_offset;
				checkValueLimits(&psu_channels_ctrl[PSU_CHANNEL_1]->voltageDacVal, 0, VOLTAGE_DAC_MAX);
				PSU_setRawVoltageDac(PSU_CHANNEL_1, psu_channels_ctrl[PSU_CHANNEL_1]->voltageDacVal);
			}
			else
			{
				psu_channels_ctrl[PSU_CHANNEL_1]->currentDacVal += controls->voltageSetEnc[0].counter_offset;
				checkValueLimits(&psu_channels_ctrl[PSU_CHANNEL_1]->currentDacVal, 0, CURRENT_DAC_MAX);
				PSU_setRawCurrentDac(PSU_CHANNEL_1, psu_channels_ctrl[PSU_CHANNEL_1]->currentDacVal);
			}
		}
	}
	else
	{
	// channel 1 set
		// voltage
		// get step value changing
		if(controls->voltageSetEnc[0].is_btn_state_changed)
		{
			controls->voltageSetEnc[0].is_btn_state_changed = 0;
			channel1_ui_data.is_update_reg |= UI_UPDATE_SET_VOLT_STEP_MASK;

			if(controls->voltageSetEnc[0].btn_state == Pressed)
			{
				psu_ch1_ctrl.voltageSetStepIdx = (psu_ch1_ctrl.voltageSetStepIdx+1)&0x01;
			}
			else if(controls->voltageSetEnc[0].btn_state == LongPressed)
			{
				psu_ch1_ctrl.voltageSetStepIdx = 2;
			}

		}

		// update voltage set value
		if(controls->voltageSetEnc[0].is_enc_state_changed)
		{
			controls->voltageSetEnc[0].is_enc_state_changed = 0;
			channel1_ui_data.is_update_reg |= UI_UPDATE_SET_VOLT_MASK;

			psu_channels_ctrl[PSU_CHANNEL_1]->voltageSetVal += controls->voltageSetEnc[0].counter_offset*voltageSetValSteps[psu_ch1_ctrl.voltageSetStepIdx];
			checkValueLimits(&psu_channels_ctrl[PSU_CHANNEL_1]->voltageSetVal, 0, VOLTAGE_DAC_MAX);
			PSU_setVoltage(PSU_CHANNEL_1, psu_channels_ctrl[PSU_CHANNEL_1]->voltageSetVal);
		}

		// current
		// get step value changing
		if(controls->currentLimitSetEnc[0].is_btn_state_changed)
		{
			controls->currentLimitSetEnc[0].is_btn_state_changed = 0;
			channel1_ui_data.is_update_reg |= UI_UPDATE_SET_CURR_STEP_MASK;

			if(controls->currentLimitSetEnc[0].btn_state == Pressed)
			{
				psu_ch1_ctrl.currentSetStepIdx = (psu_ch1_ctrl.currentSetStepIdx+1)&0x01;
			}
			else if(controls->currentLimitSetEnc[0].btn_state == LongPressed)
			{
				psu_ch1_ctrl.currentSetStepIdx = 2;
			}
		}

		// update current set value
		if(controls->currentLimitSetEnc[0].is_enc_state_changed)
		{
			controls->currentLimitSetEnc[0].is_enc_state_changed = 0;
			channel1_ui_data.is_update_reg |= UI_UPDATE_SET_CURR_MASK;

			psu_channels_ctrl[PSU_CHANNEL_1]->currentSetVal += controls->currentLimitSetEnc[0].counter_offset*currentSetValSteps[psu_ch1_ctrl.currentSetStepIdx];
			checkValueLimits(&psu_channels_ctrl[PSU_CHANNEL_1]->currentSetVal, 0, CURRENT_DAC_MAX);
			PSU_setCurrentLimit(PSU_CHANNEL_1, psu_channels_ctrl[PSU_CHANNEL_1]->currentSetVal);
		}

		// start channel calibration
		if(controls->voltageSetEnc[0].btn_state == Pressed && controls->currentLimitSetEnc[0].btn_state == Pressed)
		{
			if(!is_calibration_mode)
			{
				PSU_calibrationModeCtrl(PSU_CHANNEL_1, 1, PSU_VOLTAGE_CAL); // start channel calibration
			}
		}
	}

	// update channel 1 enable state
	if(controls->psuEnableCtrlBtns[0].is_state_changed)
	{
		controls->psuEnableCtrlBtns[0].is_state_changed = 0;
		channel1_ui_data.is_update_reg |= UI_UPDATE_ON_OFF_MASK;

		if(controls->psuEnableCtrlBtns[0].btn_state == Pressed)
		{
			PSU_enableChannelCtrl(PSU_CHANNEL_1, 1);
		}
		else
		{
			PSU_enableChannelCtrl(PSU_CHANNEL_1, 0);
		}
	}

// channel 2
	if(channels_ui_data[PSU_CHANNEL_2]->is_calibration)
	{
		// voltage set encoder button press is go to the next step
		if(controls->voltageSetEnc[1].is_btn_state_changed)
		{
			controls->voltageSetEnc[1].is_btn_state_changed = 0;

			if(controls->voltageSetEnc[1].btn_state == Pressed)
			{
				channel2_ui_data.is_update_reg |= UI_UPDATE_SET_VOLT_STEP_MASK;

				if(psu_calibration_ctrl.is_finished)
				{
					if(psu_calibration_ctrl.cal_type == PSU_VOLTAGE_CAL)
					{
						PSU_calibrationModeCtrl(PSU_CHANNEL_2, 1, PSU_CURRENT_CAL); // after the voltage calibration go to the current calibration
					}
					else
					{
						PSU_calibrationModeCtrl(PSU_CHANNEL_2, 0, PSU_VOLTAGE_CAL); // stop channel calibration and save data
					}
				}
				else
				{
					PSU_goToTheNextCalibrationStep();
				}
			}
		}

		// current set encoder button press is go to the previous step
		if(controls->currentLimitSetEnc[1].is_btn_state_changed)
		{
			controls->currentLimitSetEnc[1].is_btn_state_changed = 0;

			if(controls->currentLimitSetEnc[1].btn_state == Pressed)
			{
				channel2_ui_data.is_update_reg |= UI_UPDATE_SET_CURR_STEP_MASK;
				PSU_goToThePrevCalibrationStep();
			}
			else if(controls->currentLimitSetEnc[1].btn_state == LongPressed)
			{
				if(!psu_calibration_ctrl.is_finished)
				{
					channel2_ui_data.is_update_reg |= UI_UPDATE_SET_CURR_STEP_MASK;
					PSU_calibrationModeCtrl(PSU_CHANNEL_2, 0, PSU_VOLTAGE_CAL); // stop channel calibration without data saving
				}
			}
		}

		// update calibration parameter value
		if(controls->voltageSetEnc[1].is_enc_state_changed)
		{
			controls->voltageSetEnc[1].is_enc_state_changed = 0;
			channel2_ui_data.is_update_reg |= UI_UPDATE_SET_VOLT_MASK;

			if(psu_calibration_ctrl.cal_type == PSU_VOLTAGE_CAL)
			{
				psu_channels_ctrl[PSU_CHANNEL_2]->voltageDacVal += controls->voltageSetEnc[1].counter_offset;
				checkValueLimits(&psu_channels_ctrl[PSU_CHANNEL_2]->voltageDacVal, 0, VOLTAGE_DAC_MAX);
				PSU_setRawVoltageDac(PSU_CHANNEL_2, psu_channels_ctrl[PSU_CHANNEL_2]->voltageDacVal);
			}
			else
			{
				psu_channels_ctrl[PSU_CHANNEL_2]->currentDacVal += controls->voltageSetEnc[1].counter_offset;
				checkValueLimits(&psu_channels_ctrl[PSU_CHANNEL_2]->currentDacVal, 0, CURRENT_DAC_MAX);
				PSU_setRawCurrentDac(PSU_CHANNEL_2, psu_channels_ctrl[PSU_CHANNEL_2]->currentDacVal);
			}
		}
	}
	else
	{
	// channel 2 set
		// voltage
		// get step value changing
		if(controls->voltageSetEnc[1].is_btn_state_changed)
		{
			controls->voltageSetEnc[1].is_btn_state_changed = 0;
			channel2_ui_data.is_update_reg |= UI_UPDATE_SET_VOLT_STEP_MASK;

			if(controls->voltageSetEnc[1].btn_state == Pressed)
			{
				psu_ch2_ctrl.voltageSetStepIdx = (psu_ch2_ctrl.voltageSetStepIdx+1)&0x01;
			}
			else if(controls->voltageSetEnc[1].btn_state == LongPressed)
			{
				psu_ch2_ctrl.voltageSetStepIdx = 2;
			}

		}

		// update voltage set value
		if(controls->voltageSetEnc[1].is_enc_state_changed)
		{
			controls->voltageSetEnc[1].is_enc_state_changed = 0;
			channel2_ui_data.is_update_reg |= UI_UPDATE_SET_VOLT_MASK;

			psu_channels_ctrl[PSU_CHANNEL_2]->voltageSetVal += controls->voltageSetEnc[1].counter_offset*voltageSetValSteps[psu_ch2_ctrl.voltageSetStepIdx];
			checkValueLimits(&psu_channels_ctrl[PSU_CHANNEL_2]->voltageSetVal, 0, VOLTAGE_DAC_MAX);
			PSU_setVoltage(PSU_CHANNEL_2, psu_channels_ctrl[PSU_CHANNEL_2]->voltageSetVal);
		}

		// current
		// get step value changing
		if(controls->currentLimitSetEnc[1].is_btn_state_changed)
		{
			controls->currentLimitSetEnc[1].is_btn_state_changed = 0;
			channel2_ui_data.is_update_reg |= UI_UPDATE_SET_CURR_STEP_MASK;

			if(controls->currentLimitSetEnc[1].btn_state == Pressed)
			{
				psu_ch2_ctrl.currentSetStepIdx = (psu_ch2_ctrl.currentSetStepIdx+1)&0x01;
			}
			else if(controls->currentLimitSetEnc[1].btn_state == LongPressed)
			{
				psu_ch2_ctrl.currentSetStepIdx = 2;
			}
		}

		// update current set value
		if(controls->currentLimitSetEnc[1].is_enc_state_changed)
		{
			controls->currentLimitSetEnc[1].is_enc_state_changed = 0;
			channel2_ui_data.is_update_reg |= UI_UPDATE_SET_CURR_MASK;

			psu_channels_ctrl[PSU_CHANNEL_2]->currentSetVal += controls->currentLimitSetEnc[1].counter_offset*currentSetValSteps[psu_ch2_ctrl.currentSetStepIdx];
			checkValueLimits(&psu_channels_ctrl[PSU_CHANNEL_2]->currentSetVal, 0, CURRENT_DAC_MAX);
			PSU_setCurrentLimit(PSU_CHANNEL_2, psu_channels_ctrl[PSU_CHANNEL_2]->currentSetVal);
		}

		// start channel calibration
		if(controls->voltageSetEnc[1].btn_state == Pressed && controls->currentLimitSetEnc[1].btn_state == Pressed)
		{
			if(!is_calibration_mode)
			{
				PSU_calibrationModeCtrl(PSU_CHANNEL_2, 1, PSU_VOLTAGE_CAL); // start channel calibration
			}
		}
	}

	// update channel 2 enable state
	if(controls->psuEnableCtrlBtns[1].is_state_changed)
	{
		controls->psuEnableCtrlBtns[1].is_state_changed = 0;
		channel2_ui_data.is_update_reg |= UI_UPDATE_ON_OFF_MASK;

		if(controls->psuEnableCtrlBtns[1].btn_state == Pressed)
		{
			PSU_enableChannelCtrl(PSU_CHANNEL_2, 1);
		}
		else
		{
			PSU_enableChannelCtrl(PSU_CHANNEL_2, 0);
		}
	}

// PSU control tick handler
	  PSU_updateTickHandler();
}

/**
 * @brief Update tick event handler to save DAC EEPROM values
 * @param None
 * @retval None
 */
void PSU_updateTickHandler(void)
{
	// measure output PSU parameters
	PSU_measureOutputParameters(PSU_CHANNEL_1, channel1_ui_data.channel_measured_data);
	PSU_measureOutputParameters(PSU_CHANNEL_2, channel2_ui_data.channel_measured_data);

	channel1_ui_data.is_update_reg |= (UI_UPDATE_MEAS_VOLT_MASK|UI_UPDATE_MEAS_CURR_MASK|UI_UPDATE_CC_CV_MASK);
	channel2_ui_data.is_update_reg |= (UI_UPDATE_MEAS_VOLT_MASK|UI_UPDATE_MEAS_CURR_MASK|UI_UPDATE_CC_CV_MASK);

	// send messages to the user interface if calibration state is changed
	if(psu_calibration_ctrl.is_state_changed)
	{
		psu_calibration_ctrl.is_state_changed = 0; // reset flag to prevent sending multiple messages

		psu_status[psu_calibration_ctrl.channel]->is_status_changed = 1;

		// send error code to the interface
		if(psu_calibration_ctrl.is_error)
		{
			psu_calibration_ctrl.is_error = 0; // reset flag

			psu_status[psu_calibration_ctrl.channel]->is_error = 1;
			psu_status[psu_calibration_ctrl.channel]->error_code = psu_calibration_ctrl.error_code;
		}

		// send measured data ready state
		if(psu_calibration_ctrl.is_measured_data_ready)
		{
			psu_status[psu_calibration_ctrl.channel]->status_code = CAL_DATA_READY;
		}

		// send calibration finished state
		if(psu_calibration_ctrl.is_finished)
		{
			psu_status[psu_calibration_ctrl.channel]->status_code = CAL_FINISHED;
		}
	}

	// discharge output voltage after the voltage calibration
	if(psu_calibration_ctrl.is_voltage_discharge)
	{
		if(psu_channels_ctrl[psu_calibration_ctrl.channel]->voltageDacVal > VOLTAGE_CAL_LVL)
		{
			psu_channels_ctrl[psu_calibration_ctrl.channel]->voltageDacVal -= VOLTAGE_DISCH_STEP;
			PSU_setRawVoltageDac(psu_calibration_ctrl.channel, psu_channels_ctrl[psu_calibration_ctrl.channel]->voltageDacVal);
		}
		else
		{
			psu_calibration_ctrl.is_voltage_discharge = 0;
			psu_calibration_ctrl.is_finished = 1;
			psu_calibration_ctrl.is_state_changed = 1;
		}
	}

	// handle saving DAC value into the EEPROM after the 3 s last edit
	if(!is_calibration_mode)
	{
		// check DAC voltage update counter - if its value wasn't updated during 3 s save value in DAC EEPROM
		// channel 1
		if(psu_ch1_ctrl.voltageDacSaveValCntr < DAC_SAVE_EEPROM_TICKS_DELAY)
		{
			psu_ch1_ctrl.voltageDacSaveValCntr++;
		}
		else if(psu_ch1_ctrl.voltageDacSaveValCntr == DAC_SAVE_EEPROM_TICKS_DELAY)
		{
			MCP4725_writeDAC_register(psu_control_i2cs[PSU_CHANNEL_1], VOLTAGE_DAC_ADDR, psu_ch1_ctrl.voltageDacVal, 1);
			psu_ch1_ctrl.voltageDacSaveValCntr++; // increment counter value to single DAC value saving
		}

		// channel 2
		if(psu_ch2_ctrl.voltageDacSaveValCntr < DAC_SAVE_EEPROM_TICKS_DELAY)
		{
			psu_ch2_ctrl.voltageDacSaveValCntr++;
		}
		else if(psu_ch2_ctrl.voltageDacSaveValCntr == DAC_SAVE_EEPROM_TICKS_DELAY)
		{
			MCP4725_writeDAC_register(psu_control_i2cs[PSU_CHANNEL_2], VOLTAGE_DAC_ADDR, psu_ch2_ctrl.voltageDacVal, 1);
			psu_ch2_ctrl.voltageDacSaveValCntr++; // increment counter value to single DAC value saving
		}

		// check current update counter - if its value wasn't updated during 3 s save value in DAC EEPROM
		// channel 1
		if(psu_ch1_ctrl.currentDacSaveValCntr < DAC_SAVE_EEPROM_TICKS_DELAY)
		{
			psu_ch1_ctrl.currentDacSaveValCntr++;
		}
		else if(psu_ch1_ctrl.currentDacSaveValCntr == DAC_SAVE_EEPROM_TICKS_DELAY)
		{
			MCP4725_writeDAC_register(psu_control_i2cs[PSU_CHANNEL_1], CURRENT_DAC_ADDR, psu_ch1_ctrl.currentDacVal, 1);
			psu_ch1_ctrl.currentDacSaveValCntr++; // increment counter value to single DAC value saving
		}

		// channel 2
		if(psu_ch2_ctrl.currentDacSaveValCntr < DAC_SAVE_EEPROM_TICKS_DELAY)
		{
			psu_ch2_ctrl.currentDacSaveValCntr++;
		}
		else if(psu_ch2_ctrl.currentDacSaveValCntr == DAC_SAVE_EEPROM_TICKS_DELAY)
		{
			MCP4725_writeDAC_register(psu_control_i2cs[PSU_CHANNEL_2], CURRENT_DAC_ADDR, psu_ch2_ctrl.currentDacVal, 1);
			psu_ch2_ctrl.currentDacSaveValCntr++; // increment counter value to single DAC value saving
		}
	}
}
