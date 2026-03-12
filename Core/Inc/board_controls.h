/*
 * board_controls.h
 *
 *  Created on: 29 дек. 2023 г.
 *      Author: Kuprin_IV
 */

#ifndef INC_BOARD_CONTROLS_H_
#define INC_BOARD_CONTROLS_H_

#include "stm32f4xx_hal.h"

#define STATE_SCAN_PERIOD_MS		20
#define LONG_PRESS_TICKS			(2000/STATE_SCAN_PERIOD_MS) // 2 sec

typedef enum {NotPressed = 0, Pressed = 1, LongPressed = 2} ButtonState;

typedef struct
{
	int8_t counter_offset;
	ButtonState btn_state;
	TIM_TypeDef* scan_timer;
	GPIO_TypeDef* btn_port;
	uint16_t btn_pin;
}EncoderState;

uint8_t BC_ScanEncoder(EncoderState* ec);

#endif /* INC_BOARD_CONTROLS_H_ */
