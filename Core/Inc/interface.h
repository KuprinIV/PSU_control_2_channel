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

#define LVGL_TIMER_DIV		4

typedef enum {None = 0, CurrentValue = 1, StimFreq = 2, StimDuration = 3} FocusedLabel;

void LCDIF_InitInterface(void);
void LCDIF_UpdateLvglTimer(void);
void LCDIF_UpdateLvglTick(void);


#endif /* INC_INTERFACE_H_ */
