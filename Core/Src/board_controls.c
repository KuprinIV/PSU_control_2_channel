/*
 * board_controls.c
 *
 *  Created on: 29 дек. 2023 г.
 *      Author: Kuprin_IV
 */

#include "board_controls.h"
#include "psu_channel_ctrl.h"
#include "main.h"

#define ABS(x) (x) >= 0 ? (x):(-(x))

static void BC_ScanEncoder(EncoderState* ec, EncoderHW* ehw);
static void BC_ScanButton(ButtonState* btn, ButtonHW* bhw);

static EncoderHW voltageSetEncsHw[2] = {{TIM8, EC4S_GPIO_Port, EC4S_Pin}, {TIM4, EC3S_GPIO_Port, EC3S_Pin}};
static EncoderHW currentLimitSetEncsHw[2] = {{TIM3, EC2S_GPIO_Port, EC2S_Pin}, {TIM2, EC1S_GPIO_Port, EC1S_Pin}};
static ButtonHW psuChannelsEnableCtrlBtnHw[2] = {{BTN2_GPIO_Port, BTN2_Pin}, {BTN1_GPIO_Port, BTN1_Pin}};
static ControlsState psu_controls_state = {{{0, NotPressed, 0, 0}, {0, NotPressed, 0, 0}}, {{0, NotPressed, 0, 0}, {0, NotPressed, 0, 0}}, {{NotPressed, 0}, {NotPressed, 0}}};

/**
  * @brief  Scan PSU controls state
  * @param  controls - data structure pointer with PSU controls state
  * @retval None
  */
void BC_ScanControls(void)
{
	static uint16_t scan_idx;

	if(scan_idx < STATE_SCAN_PERIOD_MS)
	{
		scan_idx++;
	}
	else
	{
		scan_idx = 0;

		// scan encoders state
		BC_ScanEncoder(&psu_controls_state.voltageSetEnc[0], &voltageSetEncsHw[0]);
		BC_ScanEncoder(&psu_controls_state.voltageSetEnc[1], &voltageSetEncsHw[1]);
		BC_ScanEncoder(&psu_controls_state.currentLimitSetEnc[0], &currentLimitSetEncsHw[0]);
		BC_ScanEncoder(&psu_controls_state.currentLimitSetEnc[1], &currentLimitSetEncsHw[1]);

		// scan PSU channels enable control buttons
		BC_ScanButton(&psu_controls_state.psuEnableCtrlBtns[0], &psuChannelsEnableCtrlBtnHw[0]);
		BC_ScanButton(&psu_controls_state.psuEnableCtrlBtns[1], &psuChannelsEnableCtrlBtnHw[1]);

		// handle state by the PSU control
		PSU_handleControls(&psu_controls_state);
	}
}

/**
  * @brief  Scan encoder state
  * @param  ec - data structure pointer with encoder state
  * @param	ehw - data structure pointer with encoder hardware parameters
  * @retval None
  */
static void BC_ScanEncoder(EncoderState* enc, EncoderHW* ehw)
{
	// check is 'enc' NULL
	if(enc == NULL) return;

	uint8_t is_encoder_state_changed = 0;
	uint8_t is_btn_state_changed = 0;
	ButtonPressState ec_btn_prev_state = enc->btn_state;

	static uint8_t btn_pin_state_prev;
	static uint16_t long_btn_press_cntr;
	static uint8_t is_long_btn_press_detected;
	static uint8_t is_initial_state_got;
	static uint32_t encoder_cntr_prev;
	static int8_t offset;
	int8_t delta = 0;
	uint8_t abs_delta = 0;

	// get current parameter values
	uint8_t btn_pin_state = (HAL_GPIO_ReadPin(ehw->btn_port, ehw->btn_pin) == GPIO_PIN_RESET);
	uint32_t encoder_cntr = (ehw->scan_timer->CNT);

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

		if(enc->counter_offset != 0)
		{
			is_encoder_state_changed = 1;
		}

		if(enc->btn_state != ec_btn_prev_state)
		{
			is_btn_state_changed = 1;
		}
	}
	else
	{
		// set initial encoder state parameters
		btn_pin_state_prev = btn_pin_state;
		encoder_cntr_prev = encoder_cntr;
		is_initial_state_got = 1;
	}

	// set is encoder state changed
	enc->is_enc_state_changed = is_encoder_state_changed;
	enc->is_btn_state_changed = is_btn_state_changed;
}

/**
  * @brief  Scan button state
  * @param  btn - enum pointer with button state
  * @retval None
  */
static void BC_ScanButton(ButtonState* btn, ButtonHW* bhw)
{
	static uint8_t btn_pin_state_prev;
	uint8_t btn_pin_state = 0;

	btn_pin_state = (HAL_GPIO_ReadPin(bhw->btn_port, bhw->btn_pin) == GPIO_PIN_RESET);

	if(btn_pin_state == 1 && btn_pin_state_prev == 0) // button is pressed
	{
		btn->btn_state = Pressed;
		btn->is_state_changed = 1;
	}
	else if(btn_pin_state == 0 && btn_pin_state_prev == 1) // button is released
	{
		btn->btn_state = NotPressed;
		btn->is_state_changed = 1;
	}
	else
	{
		btn->is_state_changed = 0;
	}

	// update button pin previous state
	btn_pin_state_prev = btn_pin_state;
}
