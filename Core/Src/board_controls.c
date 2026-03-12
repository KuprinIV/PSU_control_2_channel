/*
 * board_controls.c
 *
 *  Created on: 29 дек. 2023 г.
 *      Author: Kuprin_IV
 */

#include "board_controls.h"
#include "main.h"

#define ABS(x) (x) >= 0 ? (x):(-(x))

/**
  * @brief  Scan encoder state
  * @params  ec - data structures with encoder state
  * @retval 0 - encoder state isn't changed, 1 - encoder state is changed
  */
uint8_t BC_ScanEncoder(EncoderState* enc)
{
	// check is 'enc' NULL
	if(enc == NULL) return 0;

	uint8_t is_encoder_state_changed = 0;
	ButtonState ec_btn_prev_state = enc->btn_state;

	static uint8_t btn_pin_state_prev;
	static uint16_t long_btn_press_cntr;
	static uint8_t is_long_btn_press_detected;
	static uint8_t is_initial_state_got;
	static uint32_t encoder_cntr_prev;
	static int8_t offset;
	int8_t delta = 0;
	uint8_t abs_delta = 0;

	// get current parameter values
	uint8_t btn_pin_state = (HAL_GPIO_ReadPin(enc->btn_port, enc->btn_pin) == GPIO_PIN_RESET);
	uint32_t encoder_cntr = (enc->scan_timer->CNT);

	if(is_initial_state_got)
	{
		// get encoder button state
		if(btn_pin_state == 1 && btn_pin_state_prev == 0) // button is pressed
		{
			enc->btn_state = Pressed;
		}
		else if(btn_pin_state == 1 && btn_pin_state_prev == 1) // button is holding on
		{
			if(long_btn_press_cntr < LONG_PRESS_TICKS)
			{
				long_btn_press_cntr++;
			}
			else if(!is_long_btn_press_detected)
			{
				is_long_btn_press_detected = 1;
				enc->btn_state = LongPressed;
			}
		}
		else
		{
			enc->btn_state = NotPressed;
			// reset button long press tick counter and long press detection flag
			long_btn_press_cntr = 0;
			is_long_btn_press_detected = 0;
		}

		// get encoder counter offset
		delta = (int8_t)(encoder_cntr_prev - encoder_cntr + offset);
		abs_delta = ABS(delta);

		// because STM32 encoder timer changes counter by 2 per 1 pulse, divide it by 2
		if(abs_delta > 1)
		{
			enc->counter_offset = delta/2;
			offset = delta - 2*enc->counter_offset; // keep reminder from division
			encoder_cntr_prev = encoder_cntr;
		}
		else
		{
			enc->counter_offset = 0;
		}

		// update button pin and encoder counter previous state
		btn_pin_state_prev = btn_pin_state;

		if((enc->btn_state != ec_btn_prev_state) || (enc->counter_offset != 0))
		{
			is_encoder_state_changed = 1;
		}
	}
	else
	{
		// set initial encoder state parameters
		btn_pin_state_prev = btn_pin_state;
		encoder_cntr_prev = encoder_cntr;
		is_initial_state_got = 1;
	}

	return is_encoder_state_changed;
}
