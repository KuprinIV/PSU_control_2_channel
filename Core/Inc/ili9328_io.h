#ifndef __ILI9328_IO_H
#define __ILI9328_IO_H

#include "stm32f4xx_hal.h"

// data write address
#define LCD_DATA                                                 ((uint32_t)0x60020000)
// command write address
#define LCD_REG                                                  ((uint32_t)0x60000000)

void LCD_IO_Init(void);
void LCD_IO_WriteReg(uint8_t Reg);
void LCD_IO_WriteData(uint16_t RegValue);
void LCD_IO_WriteMultipleData(uint16_t* data, uint32_t size);
uint16_t LCD_IO_ReadData(uint8_t reg);
void LCD_Delay(uint32_t delay);

#endif
