/*
 * psu_channel_ctrl.h
 *
 *  Created on: 15 февр. 2026 г.
 *      Author: Ilya
 */

#ifndef INC_PSU_CHANNEL_CTRL_H_
#define INC_PSU_CHANNEL_CTRL_H_

#include "stm32f4xx_hal.h"
#include "board_controls.h"

// PSU DAC control
#define VOLTAGE_DAC_MAX						3000U
#define CURRENT_DAC_MAX						3200U
#define VOLTAGE_DAC_CALIBRATION_STEP		100U
#define CURRENT_DAC_CALIBRATION_STEP		100U
#define VOLTAGE_MEAS_CALIBRATION_STEP		1000U
#define CURRENT_MEAS_CALIBRATION_STEP		100U
#define DAC_SAVE_EEPROM_TICKS_DELAY			(3000/STATE_SCAN_PERIOD_MS)
#define SCAN_TICKS_DELAY					50

#define VOLTAGE_SET_STEP_FINE				5
#define VOLTAGE_SET_STEP_COARSE				25
#define CURRENT_SET_STEP_FINE				10
#define CURRENT_SET_STEP_COARSE				100

// PSU calibration
#define VOLTAGE_CALIB_STEPS_NUM				(VOLTAGE_DAC_MAX/VOLTAGE_DAC_CALIBRATION_STEP+1)
#define CURRENT_CALIB_STEPS_NUM				(CURRENT_DAC_MAX/CURRENT_DAC_CALIBRATION_STEP+1)
#define CALIBRATION_DATA_PAGE_ADDR			ADDR_FLASH_SECTOR_11
#define CH2_CAL_DATA_OFFSET					0x200
#define CH1_CAL_DATA_ADDR					(CALIBRATION_DATA_PAGE_ADDR)
#define CH2_CAL_DATA_ADDR					(CALIBRATION_DATA_PAGE_ADDR+CH2_CAL_DATA_OFFSET)
#define CALIBRATION_MEAS_DATA_AVGS			10

// calibration status and error codes
#define CAL_DATA_READY						0xA0
#define CAL_FINISHED						0xA1

#define CAL_SUCCESS							0
#define CAL_UNLOCK_FLASH_ERR				0xE0
#define CAL_ERASE_FLASH_ERR					0xE1
#define CAL_PROGRAM_FLASH_ERR				0xE2
#define CAL_NEED_TO_CALIBRATE_ERR			0xE3
#define CAL_CHANNEL_NOT_ENABLED				0xE4

#define VOLTAGE_CAL_LVL						500U
#define CURRENT_CAL_LVL						100U
#define VOLTAGE_DISCH_STEP					200U

/* Base address of the Flash sectors */
#define ADDR_FLASH_SECTOR_0     ((uint32_t)0x08000000) /* Base @ of Sector 0, 16 Kbytes */
#define ADDR_FLASH_SECTOR_1     ((uint32_t)0x08004000) /* Base @ of Sector 1, 16 Kbytes */
#define ADDR_FLASH_SECTOR_2     ((uint32_t)0x08008000) /* Base @ of Sector 2, 16 Kbytes */
#define ADDR_FLASH_SECTOR_3     ((uint32_t)0x0800C000) /* Base @ of Sector 3, 16 Kbytes */
#define ADDR_FLASH_SECTOR_4     ((uint32_t)0x08010000) /* Base @ of Sector 4, 64 Kbytes */
#define ADDR_FLASH_SECTOR_5     ((uint32_t)0x08020000) /* Base @ of Sector 5, 128 Kbytes */
#define ADDR_FLASH_SECTOR_6     ((uint32_t)0x08040000) /* Base @ of Sector 6, 128 Kbytes */
#define ADDR_FLASH_SECTOR_7     ((uint32_t)0x08060000) /* Base @ of Sector 7, 128 Kbytes */
#define ADDR_FLASH_SECTOR_8     ((uint32_t)0x08080000) /* Base @ of Sector 8, 128 Kbytes */
#define ADDR_FLASH_SECTOR_9     ((uint32_t)0x080A0000) /* Base @ of Sector 9, 128 Kbytes */
#define ADDR_FLASH_SECTOR_10    ((uint32_t)0x080C0000) /* Base @ of Sector 10, 128 Kbytes */
#define ADDR_FLASH_SECTOR_11    ((uint32_t)0x080E0000) /* Base @ of Sector 11, 128 Kbytes */

#define UI_UPDATE_MEAS_VOLT_MASK			0x01
#define UI_UPDATE_MEAS_CURR_MASK			0x02
#define UI_UPDATE_CC_CV_MASK				0x04
#define UI_UPDATE_SET_VOLT_MASK				0x08
#define UI_UPDATE_SET_VOLT_STEP_MASK		0x10
#define UI_UPDATE_SET_CURR_MASK				0x20
#define UI_UPDATE_SET_CURR_STEP_MASK		0x40
#define UI_UPDATE_ON_OFF_MASK				0x80

typedef enum
{
	PSU_CHANNEL_1 = 0,
	PSU_CHANNEL_2 = 1,
}PSU_Channel;

typedef enum
{
	PSU_VOLTAGE_CAL = 0,
	PSU_CURRENT_CAL = 1,
}PSU_CalibrationType;

typedef struct
{
	uint16_t voltage_mv;
	int16_t current_ma;
	uint8_t is_current_limit;
}PSU_MeasuredParams;

typedef struct
{
	GPIO_TypeDef* port;
	uint16_t pin;
}PSU_GPIO_CtrlPin;

typedef struct
{
	uint16_t is_enabled;
	uint16_t voltageDacVal;
	uint16_t voltageSetVal;
	uint8_t voltageSetStepIdx;
	uint16_t currentDacVal;
	uint16_t currentSetVal;
	uint8_t currentSetStepIdx;
	uint16_t voltageDacSaveValCntr;
	uint16_t currentDacSaveValCntr;
}ChannelCtrl;

typedef struct
{
	uint16_t channel_id;
	uint16_t voltageStepsNum;
	uint16_t currentStepsNum;
	uint16_t voltageDacStep;
	uint16_t currentDacStep;
	uint16_t voltageMeasStep;
	uint16_t currentMeasStep;
	uint16_t voltageDacCalibrationNodes[VOLTAGE_CALIB_STEPS_NUM];
	uint16_t currentDacCalibrationNodes[CURRENT_CALIB_STEPS_NUM];
	uint16_t voltageMeasCalibrationNodes[VOLTAGE_CALIB_STEPS_NUM];
	int16_t currentMeasCalibrationNodes[CURRENT_CALIB_STEPS_NUM];
}ChannelCalibrationData;

typedef struct
{
	PSU_Channel channel;
	PSU_CalibrationType cal_type;
	uint8_t steps_num;
	uint8_t currentStep;
	uint8_t is_finished;
	uint8_t average_measured_data_cntr;
	uint8_t is_measured_data_ready;
	uint8_t is_state_changed;
	uint8_t is_error;
	uint8_t is_voltage_discharge;
	uint8_t error_code;
}PSU_Calibration;

typedef struct
{
	uint8_t is_status_changed;
	uint8_t status_code;
	uint8_t is_error;
	uint8_t error_code;
}PSU_CalibrationStatus;


typedef struct
{
	uint8_t is_calibration;
	uint8_t is_update_reg;
	ChannelCtrl* channel_set_values;
	PSU_MeasuredParams* channel_measured_data;
	PSU_CalibrationStatus* channel_calibration_status;
}PSU_UI_ChannelData;

void PSU_controlInit(void);
void PSU_enableChannelCtrl(PSU_Channel ch, uint8_t is_enabled);
void PSU_setVoltage(PSU_Channel ch, uint16_t voltage_10mv);
void PSU_setCurrentLimit(PSU_Channel ch, uint16_t current_1ma);
void PSU_setRawVoltageDac(PSU_Channel ch, uint16_t voltage_dac_code);
void PSU_setRawCurrentDac(PSU_Channel ch, uint16_t current_dac_code);
void PSU_getChannelDacValues(PSU_Channel ch, uint16_t* voltage_dac_code, uint16_t* current_dac_code);
void PSU_measureOutputParameters(PSU_Channel ch, PSU_MeasuredParams* psu_mp);
void PSU_calibrationModeCtrl(PSU_Channel ch, uint8_t is_enabled, PSU_CalibrationType cal_type);
void PSU_goToThePrevCalibrationStep(void);
void PSU_goToTheNextCalibrationStep(void);
void PSU_handleControls(ControlsState* controls);
void PSU_updateTickHandler(void);


#endif /* INC_PSU_CHANNEL_CTRL_H_ */
