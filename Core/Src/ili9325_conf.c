#include "stm32f4xx_hal.h"
#include "ili9325_conf.h"
#include "main.h"

void LCD_IO_Init(void)
{
	// make display reset
	HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, 1);
	HAL_Delay(5);
	HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, 0);
	HAL_Delay(5);
	HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, 1);
}

void LCD_IO_WriteReg(uint8_t Reg)
{
	*(uint16_t*)(LCD_REG) = Reg;
}

void LCD_IO_WriteData(uint16_t RegValue)
{
	*(uint16_t*)(LCD_DATA) = RegValue;
}

void LCD_IO_WriteMultipleData(uint16_t* data, uint32_t size)
{
	for(uint32_t i = 0; i < size; i++)
	{
		*(uint16_t*)(LCD_DATA) = data[i];
	}
}

uint16_t LCD_IO_ReadData(uint8_t reg)
{
	*(uint16_t*)(LCD_REG) = reg;
	return *(uint16_t*)(LCD_DATA);
}

void LCD_Delay(uint32_t delay)
{
	HAL_Delay(delay);
}
