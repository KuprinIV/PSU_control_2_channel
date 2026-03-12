#include "ili9328_io.h"
#include "stm32f4xx_hal.h"
#include "main.h"

/**
  * @brief  Initialize the ILI9328 LCD IO
  * @param  None
  * @retval None
  */
void LCD_IO_Init(void)
{
	// make display reset
	HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, 1);
	HAL_Delay(5);
	HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, 0);
	HAL_Delay(5);
	HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, 1);
}

/**
  * @brief  ILI9328 LCD write register
  * @param  Reg - register address
  * @retval None
  */
void LCD_IO_WriteReg(uint8_t Reg)
{
	*(uint16_t*)(LCD_REG) = Reg;
}

/**
  * @brief  ILI9328 LCD write data
  * @param  RegValue - register value
  * @retval None
  */
void LCD_IO_WriteData(uint16_t RegValue)
{
	*(uint16_t*)(LCD_DATA) = RegValue;
}

/**
  * @brief  ILI9328 LCD write multiple data
  * @param  data - data array pointer to write
  * @param size - data array size
  * @retval None
  */
void LCD_IO_WriteMultipleData(uint16_t* data, uint32_t size)
{
	for(uint32_t i = 0; i < size; i++)
	{
		*(uint16_t*)(LCD_DATA) = data[i];
	}
}

/**
  * @brief  ILI9328 LCD read data
  * @param  reg - register address
  * @retval register value
  */
uint16_t LCD_IO_ReadData(uint8_t reg)
{
	*(uint16_t*)(LCD_REG) = reg;
	return *(uint16_t*)(LCD_DATA);
}

/**
  * @brief  ILI9328 LCD delay
  * @param  delay - delay value in ms
  * @retval None
  */
void LCD_Delay(uint32_t delay)
{
	HAL_Delay(delay);
}
