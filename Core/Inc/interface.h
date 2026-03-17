/*
 * interface.h
 *
 *  Created on: 13 мая 2024 г.
 *      Author: Kuprin_IV
 */

#ifndef INC_INTERFACE_H_
#define INC_INTERFACE_H_

//#include "board_controls.h"
#include "ili9328.h"

#define LVGL_TIMER_DIV			4
#define CH1_ITEMS_COLOR			0xFFAF20
#define CH2_ITEMS_COLOR			0x00FFFF
#define TEXT_COLOR				0xFFFFFF
#define BUTTON_DEFAULT_COLOR	0x858FA2

typedef enum {None = 0, CurrentValue = 1, StimFreq = 2, StimDuration = 3} FocusedLabel;

void LCDIF_InitInterface(void);
void LCDIF_UpdateLvglTimer(void);
void LCDIF_UpdateLvglTick(void);


#endif /* INC_INTERFACE_H_ */
