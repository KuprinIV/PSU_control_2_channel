/*
 * board_controls.h
 *
 *  Created on: 29 дек. 2023 г.
 *      Author: Kuprin_IV
 */

#ifndef INC_BOARD_CONTROLS_H_
#define INC_BOARD_CONTROLS_H_

#include "stm32f4xx_hal.h"

#define STATE_SCAN_PERIOD_MS		50
#define LONG_PRESS_TICKS			(2000/STATE_SCAN_PERIOD_MS) // 2 sec

typedef enum {NotPressed = 0, Pressed = 1, LongPressed = 2} ButtonPressState;

typedef struct
{
	int8_t counter_offset;
	ButtonPressState btn_state;
	uint8_t is_enc_state_changed;
	uint8_t is_btn_state_changed;
}EncoderState;

typedef struct
{
	ButtonPressState btn_state;
	uint8_t is_state_changed;
}ButtonState;

typedef struct
{
	TIM_TypeDef* scan_timer;
	GPIO_TypeDef* btn_port;
	uint16_t btn_pin;
}EncoderHW;

typedef struct
{
	GPIO_TypeDef* btn_port;
	uint16_t btn_pin;
}ButtonHW;

typedef struct
{
	EncoderState voltageSetEnc[2];
	EncoderState currentLimitSetEnc[2];
	ButtonState psuEnableCtrlBtns[2];
}ControlsState;

void BC_ScanControls(void);

#endif /* INC_BOARD_CONTROLS_H_ */
