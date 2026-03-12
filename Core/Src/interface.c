/*
 * interface.c
 *
 *  Created on: 13 мая 2024 г.
 *      Author: Kuprin_IV
 */
#include "interface.h"
#include "lvgl.h"
#include "ili9328.h"
#include <string.h>

// misc functions


// UI control functions
//static void updateEncoderState(void);
//static void updateUI(void);

// callback functions
static void update_display_area_cb(lv_display_t * disp, const lv_area_t * area, uint8_t * color_p);
static void on_dma_transfer_complete_handler(void);

// LVGL frame buffers
static uint16_t buf_1[9600];
static uint16_t buf_2[9600];

// LVGL interface objects
static lv_display_t * disp;
static lv_obj_t * stim_dur_unit_lbl;

// misc variables
static uint8_t is_UI_enabled = 0;
//static uint8_t is_need_to_update_ui = 0;
static uint8_t is_ui_enable_changed = 0;

/********************************************************* driver functions *************************************************************/
/**
 * @brief Initialize LVGL and add interface objects to screen
 * @param: None
 * @return: None
 */
void LCDIF_InitInterface(void)
{
	// init DMA callbacks
	ili9328_SetOnDmaTransferCompleteCb(on_dma_transfer_complete_handler);

	//Initialize LVGL UI library
	lv_init();

	/* Initialize display items */
	disp = lv_display_create(320, 240); /*Basic initialization with horizontal and vertical resolution in pixels*/
	lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565);
	lv_display_set_flush_cb(disp, update_display_area_cb); /*Set a flush callback to draw to the display*/
	lv_display_set_buffers(disp, buf_1, buf_2, sizeof(buf_1), LV_DISPLAY_RENDER_MODE_PARTIAL); /*Set an initialized buffer*/

	// Change the active screen's background color
	lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0xFF0000), LV_PART_MAIN);
	lv_obj_set_style_text_color(lv_screen_active(), lv_color_hex(0xffffff), LV_PART_MAIN);


	// stimulus duration unit label
	stim_dur_unit_lbl = lv_label_create(lv_screen_active());
	lv_label_set_text(stim_dur_unit_lbl, "Test string");
	lv_obj_set_style_text_font(stim_dur_unit_lbl, &lv_font_montserrat_20, 0);
	lv_obj_set_style_text_color(stim_dur_unit_lbl, lv_color_hex(0x858FA2), 0);
	lv_obj_set_style_text_align(stim_dur_unit_lbl, LV_TEXT_ALIGN_CENTER, 0);
	lv_obj_align(stim_dur_unit_lbl, LV_ALIGN_TOP_LEFT, 50, 20);
}

/**
 * @brief Update LVGL timer
 * @param: None
 * @return: None
 */
void LCDIF_UpdateLvglTimer(void)
{
	static uint16_t lvgl_timer_div;
	if(lvgl_timer_div++ >= LVGL_TIMER_DIV) // update timer with 5 ms period
	{
	  lvgl_timer_div = 0;

// UI updates shall be done in this callback to prevent LVGL invalidate and rendering tasks issues!
	  // update UI, if it's needed
//	  if(is_need_to_update_ui)
//	  {
//		  is_need_to_update_ui = 0;
//		  updateUI();
//	  }

	  lv_timer_handler();
	}
}

/**
 * @brief Update LVGL tick
 * @param: None
 * @return: None
 */
void LCDIF_UpdateLvglTick(void)
{
	lv_tick_inc(1);
}

/**
 * @brief UI enable control
 * @param: is_enabled - disable UI: 0, enable UI: 1
 * @return: None
 */
void LCDIF_SetEnabled(uint8_t is_enabled)
{
	is_ui_enable_changed = (is_UI_enabled != is_enabled);
	is_UI_enabled = is_enabled;
}


/********************************************************* UI control functions *************************************************************/

/**
 * @brief Update selected screen object if encoder state is changed
 * @param: enc - encoder data object
 * @return: None
 */
//void updateEncoderState(void)
//{
//
//}


/**
 * @brief Draw probe started stimulation test screen
 * @param: None
 * @return: None
 */
//static void updateUI(void)
//{
//
//}

/********************************************************* callback functions *************************************************************/

/**
 * @brief Display invalidate callback from LVGL
 * @param: disp - LVGL screen object
 * @param: area - LVGL update area object
 * @param: color_p - LVGL pixels data array pointer
 * @return: None
 */
static void update_display_area_cb(lv_display_t * disp, const lv_area_t * area, uint8_t * color_p)
{
	int height = area->y2 - area->y1 + 1;
	int width = area->x2 - area->x1 + 1;

	ili9328_WriteGRAM(area->x1, area->y1, area->x2, area->y2, (uint16_t*)color_p, width*height);
	lv_display_flush_ready(disp);
}

/**
 * @brief Framebuffer DMA transfer complete callback
 * @param: None
 * @return: None
 */
static void on_dma_transfer_complete_handler(void)
{
	/* Inform the graphics library that you are ready with the flushing*/
	lv_display_flush_ready(disp);
}
