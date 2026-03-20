/*
 * interface.c
 *
 *  Created on: 13 мая 2024 г.
 *      Author: Kuprin_IV
 */
#include "interface.h"
#include "lvgl.h"
#include "ili9328.h"
#include "main.h"
#include "psu_channel_ctrl.h"
#include <string.h>

//#define LVGL_TEST

// misc functions


// UI control functions
static void updateUI(void);

// callback functions
static void update_display_area_cb(lv_display_t * disp, const lv_area_t * area, uint8_t * color_p);

// LVGL frame buffers
static uint16_t buf_1[9600];
static uint16_t buf_2[9600];

// LVGL interface objects
static lv_display_t * disp;

#ifndef LVGL_TEST
	static lv_obj_t * meas_volt_ch1_lbl;
	static lv_obj_t * meas_volt_ch2_lbl;
	static lv_obj_t * meas_curr_ch1_lbl;
	static lv_obj_t * meas_curr_ch2_lbl;

	static lv_obj_t * set_volt_ch1_lbl;
	static lv_obj_t * set_volt_ch2_lbl;
	static lv_obj_t * set_curr_ch1_lbl;
	static lv_obj_t * set_curr_ch2_lbl;

	static lv_obj_t * cv_ch1_led;
	static lv_obj_t * cc_ch1_led;
	static lv_obj_t * cv_ch2_led;
	static lv_obj_t * cc_ch2_led;

	static lv_obj_t * on_off_ch1_btn;
	static lv_obj_t * on_off_ch2_btn;
	static lv_obj_t* ch1_btn_label;
	static lv_obj_t* ch2_btn_label;

	static uint32_t set_parameters_lbl_text_colors[3] = {TEXT_COLOR, TEXT_COLOR_STEP_COARSE, TEXT_COLOR_STEP_LOWEST};
#else
	static lv_obj_t * test_lbl;
	static uint16_t test_cntr = 0;
#endif

extern PSU_UI_ChannelData channel1_ui_data, channel2_ui_data;

/********************************************************* driver functions *************************************************************/
/**
 * @brief Initialize LVGL and add interface objects to screen
 * @param: None
 * @return: None
 */
void LCDIF_InitInterface(void)
{
//Initialize LVGL UI library
	lv_init();

/* Initialize display items */
	disp = lv_display_create(320, 240); /*Basic initialization with horizontal and vertical resolution in pixels*/
	lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565);
	lv_display_set_flush_cb(disp, update_display_area_cb); /*Set a flush callback to draw to the display*/
	lv_display_set_buffers(disp, buf_1, buf_2, sizeof(buf_1), LV_DISPLAY_RENDER_MODE_PARTIAL); /*Set an initialized buffer*/

// Change the active screen's background color
	lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0x000000), LV_PART_MAIN);
	lv_obj_set_style_text_color(lv_screen_active(), lv_color_hex(TEXT_COLOR), LV_PART_MAIN);

#ifndef LVGL_TEST
// channels parameters frame rectangles
    // create rectangle object
    lv_obj_t * rect_ch1 = lv_obj_create(lv_screen_active());
    lv_obj_t * rect_set_ch1 = lv_obj_create(lv_screen_active());
    lv_obj_t * rect_ch2 = lv_obj_create(lv_screen_active());
    lv_obj_t * rect_set_ch2 = lv_obj_create(lv_screen_active());

    // set size and position
    lv_obj_set_size(rect_ch1, 157, 235);
    lv_obj_set_size(rect_ch2, 157, 235);
    lv_obj_set_size(rect_set_ch1, 141, 44);
    lv_obj_set_size(rect_set_ch2, 141, 44);

    lv_obj_set_pos(rect_ch1, 2, 2);
    lv_obj_set_pos(rect_ch2, 161, 2);
    lv_obj_set_pos(rect_set_ch1, 10, 138);
    lv_obj_set_pos(rect_set_ch2, 169, 138);

    // set styles
    // set transparent background
    lv_obj_set_style_bg_opa(rect_ch1, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(rect_ch2, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(rect_set_ch1, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(rect_set_ch2, LV_OPA_TRANSP, LV_PART_MAIN);

    // round angles
    lv_obj_set_style_radius(rect_ch1, 10, LV_PART_MAIN);
    lv_obj_set_style_radius(rect_ch2, 10, LV_PART_MAIN);
    lv_obj_set_style_radius(rect_set_ch1, 6, LV_PART_MAIN);
    lv_obj_set_style_radius(rect_set_ch2, 6, LV_PART_MAIN);

    // set border
    lv_obj_set_style_border_width(rect_ch1, 5, LV_PART_MAIN);         // border width is 5 pixels
    lv_obj_set_style_border_width(rect_ch2, 5, LV_PART_MAIN);         // border width is 5 pixels
    lv_obj_set_style_border_width(rect_set_ch1, 1, LV_PART_MAIN);     // border width is 1 pixel
    lv_obj_set_style_border_width(rect_set_ch2, 1, LV_PART_MAIN);     // border width is 1 pixel

    lv_obj_set_style_border_color(rect_ch1, lv_color_hex(CH1_ITEMS_COLOR), LV_PART_MAIN); // yellow(#FFFF00)
    lv_obj_set_style_border_color(rect_ch2, lv_color_hex(CH2_ITEMS_COLOR), LV_PART_MAIN); // blue(#00FFFF)
    lv_obj_set_style_border_color(rect_set_ch1, lv_color_hex(CH1_ITEMS_COLOR), LV_PART_MAIN); // yellow(#FFFF00)
    lv_obj_set_style_border_color(rect_set_ch2, lv_color_hex(CH2_ITEMS_COLOR), LV_PART_MAIN); // blue(#00FFFF)

    lv_obj_set_style_border_opa(rect_ch1, LV_OPA_COVER, LV_PART_MAIN); // not transparent border
    lv_obj_set_style_border_opa(rect_ch2, LV_OPA_COVER, LV_PART_MAIN); // not transparent border
    lv_obj_set_style_border_opa(rect_set_ch1, LV_OPA_COVER, LV_PART_MAIN); // not transparent border
    lv_obj_set_style_border_opa(rect_set_ch2, LV_OPA_COVER, LV_PART_MAIN); // not transparent border

    // remove extended parameters
    lv_obj_set_style_shadow_width(rect_ch1, 0, LV_PART_MAIN);         // remove shadow
    lv_obj_set_style_shadow_width(rect_ch2, 0, LV_PART_MAIN);         // remove shadow
    lv_obj_set_style_shadow_width(rect_set_ch1, 0, LV_PART_MAIN);     // remove shadow
    lv_obj_set_style_shadow_width(rect_set_ch2, 0, LV_PART_MAIN);     // remove shadow

    lv_obj_set_style_outline_width(rect_ch1, 0, LV_PART_MAIN);        // remove outline
    lv_obj_set_style_outline_width(rect_ch2, 0, LV_PART_MAIN);        // remove outline
    lv_obj_set_style_outline_width(rect_set_ch1, 0, LV_PART_MAIN);    // remove outline
    lv_obj_set_style_outline_width(rect_set_ch2, 0, LV_PART_MAIN);    // remove outline

    lv_obj_set_style_pad_all(rect_ch1, 0, LV_PART_MAIN);              // remove margins
    lv_obj_set_style_pad_all(rect_ch2, 0, LV_PART_MAIN);              // remove margins
    lv_obj_set_style_pad_all(rect_set_ch1, 0, LV_PART_MAIN);          // remove margins
    lv_obj_set_style_pad_all(rect_set_ch2, 0, LV_PART_MAIN);          // remove margins

// channels title
    lv_obj_t * ch1_title_lbl = lv_label_create(lv_screen_active());
    lv_label_set_text(ch1_title_lbl, "CH1");
	lv_obj_set_style_text_font(ch1_title_lbl, &lv_font_montserrat_20, LV_PART_MAIN);
	lv_obj_set_style_text_color(ch1_title_lbl, lv_color_hex(TEXT_COLOR), LV_PART_MAIN);
	lv_obj_set_style_bg_color(ch1_title_lbl, lv_color_hex(CH1_ITEMS_COLOR), LV_PART_MAIN);
	lv_obj_set_style_bg_opa(ch1_title_lbl, LV_OPA_COVER, LV_PART_MAIN);
	lv_obj_set_style_text_align(ch1_title_lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
	lv_obj_set_width(ch1_title_lbl, 149);
	lv_obj_align(ch1_title_lbl, LV_ALIGN_TOP_LEFT, 6, 4);

    lv_obj_t * ch2_title_lbl = lv_label_create(lv_screen_active());
    lv_label_set_text(ch2_title_lbl, "CH2");
	lv_obj_set_style_text_font(ch2_title_lbl, &lv_font_montserrat_20, LV_PART_MAIN);
	lv_obj_set_style_text_color(ch2_title_lbl, lv_color_hex(TEXT_COLOR), LV_PART_MAIN);
	lv_obj_set_style_bg_color(ch2_title_lbl, lv_color_hex(CH2_ITEMS_COLOR), LV_PART_MAIN);
	lv_obj_set_style_bg_opa(ch2_title_lbl, LV_OPA_COVER, LV_PART_MAIN);
	lv_obj_set_style_text_align(ch2_title_lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
	lv_obj_set_width(ch2_title_lbl, 149);
	lv_obj_align(ch2_title_lbl, LV_ALIGN_TOP_LEFT, 165, 4);

// channel 1 measured parameters
	meas_volt_ch1_lbl = lv_label_create(lv_screen_active());
    lv_label_set_text(meas_volt_ch1_lbl, "0.00 V");
	lv_obj_set_style_text_font(meas_volt_ch1_lbl, &lv_font_montserrat_32, LV_PART_MAIN);
	lv_obj_set_style_text_color(meas_volt_ch1_lbl, lv_color_hex(TEXT_COLOR), LV_PART_MAIN);
	lv_obj_set_style_text_align(meas_volt_ch1_lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
	lv_obj_set_width(meas_volt_ch1_lbl, 155);
	lv_obj_align(meas_volt_ch1_lbl, LV_ALIGN_TOP_LEFT, 4, 30);

	meas_curr_ch1_lbl = lv_label_create(lv_screen_active());
    lv_label_set_text(meas_curr_ch1_lbl, "0.000 A");
	lv_obj_set_style_text_font(meas_curr_ch1_lbl, &lv_font_montserrat_32, LV_PART_MAIN);
	lv_obj_set_style_text_color(meas_curr_ch1_lbl, lv_color_hex(TEXT_COLOR), LV_PART_MAIN);
	lv_obj_set_style_text_align(meas_curr_ch1_lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
	lv_obj_set_width(meas_curr_ch1_lbl, 155);
	lv_obj_align(meas_curr_ch1_lbl, LV_ALIGN_TOP_LEFT, 4, 67);

// channel 2 measured parameters
	meas_volt_ch2_lbl = lv_label_create(lv_screen_active());
    lv_label_set_text(meas_volt_ch2_lbl, "0.00 V");
	lv_obj_set_style_text_font(meas_volt_ch2_lbl, &lv_font_montserrat_32, LV_PART_MAIN);
	lv_obj_set_style_text_color(meas_volt_ch2_lbl, lv_color_hex(TEXT_COLOR), LV_PART_MAIN);
	lv_obj_set_style_text_align(meas_volt_ch2_lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
	lv_obj_set_width(meas_volt_ch2_lbl, 155);
	lv_obj_align(meas_volt_ch2_lbl, LV_ALIGN_TOP_LEFT, 163, 30);

	meas_curr_ch2_lbl = lv_label_create(lv_screen_active());
    lv_label_set_text(meas_curr_ch2_lbl, "0.000 A");
	lv_obj_set_style_text_font(meas_curr_ch2_lbl, &lv_font_montserrat_32, LV_PART_MAIN);
	lv_obj_set_style_text_color(meas_curr_ch2_lbl, lv_color_hex(TEXT_COLOR), LV_PART_MAIN);
	lv_obj_set_style_text_align(meas_curr_ch2_lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
	lv_obj_set_width(meas_curr_ch2_lbl, 155);
	lv_obj_align(meas_curr_ch2_lbl, LV_ALIGN_TOP_LEFT, 163, 67);

// channel 1 set parameters
	lv_obj_t * set_volt_ch1_title_lbl = lv_label_create(lv_screen_active());
	lv_label_set_text(set_volt_ch1_title_lbl, "Vset");
	lv_obj_set_style_text_font(set_volt_ch1_title_lbl, &lv_font_montserrat_16, LV_PART_MAIN);
	lv_obj_set_style_text_color(set_volt_ch1_title_lbl, lv_color_hex(TEXT_COLOR), LV_PART_MAIN);
	lv_obj_set_style_text_align(set_volt_ch1_title_lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
	lv_obj_set_width(set_volt_ch1_title_lbl, 76);
	lv_obj_align(set_volt_ch1_title_lbl, LV_ALIGN_TOP_LEFT, 8, 140);

	set_volt_ch1_lbl = lv_label_create(lv_screen_active());
	lv_label_set_text(set_volt_ch1_lbl, "0.00 V");
	lv_obj_set_style_text_font(set_volt_ch1_lbl, &lv_font_montserrat_16, LV_PART_MAIN);
	lv_obj_set_style_text_color(set_volt_ch1_lbl, lv_color_hex(TEXT_COLOR), LV_PART_MAIN);
	lv_obj_set_style_text_align(set_volt_ch1_lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
	lv_obj_set_width(set_volt_ch1_lbl, 76);
	lv_obj_align(set_volt_ch1_lbl, LV_ALIGN_TOP_LEFT, 8, 160);

	lv_obj_t * set_curr_ch1_title_lbl = lv_label_create(lv_screen_active());
	lv_label_set_text(set_curr_ch1_title_lbl, "Iset");
	lv_obj_set_style_text_font(set_curr_ch1_title_lbl, &lv_font_montserrat_16, LV_PART_MAIN);
	lv_obj_set_style_text_color(set_curr_ch1_title_lbl, lv_color_hex(TEXT_COLOR), LV_PART_MAIN);
	lv_obj_set_style_text_align(set_curr_ch1_title_lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
	lv_obj_set_width(set_curr_ch1_title_lbl, 76);
	lv_obj_align(set_curr_ch1_title_lbl, LV_ALIGN_TOP_LEFT, 81, 140);

	set_curr_ch1_lbl = lv_label_create(lv_screen_active());
	lv_label_set_text(set_curr_ch1_lbl, "0.00 A");
	lv_obj_set_style_text_font(set_curr_ch1_lbl, &lv_font_montserrat_16, LV_PART_MAIN);
	lv_obj_set_style_text_color(set_curr_ch1_lbl, lv_color_hex(TEXT_COLOR), LV_PART_MAIN);
	lv_obj_set_style_text_align(set_curr_ch1_lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
	lv_obj_set_width(set_curr_ch1_lbl, 76);
	lv_obj_align(set_curr_ch1_lbl, LV_ALIGN_TOP_LEFT, 81, 160);

// channel 2 set parameters
	lv_obj_t * set_volt_ch2_title_lbl = lv_label_create(lv_screen_active());
	lv_label_set_text(set_volt_ch2_title_lbl, "Vset");
	lv_obj_set_style_text_font(set_volt_ch2_title_lbl, &lv_font_montserrat_16, LV_PART_MAIN);
	lv_obj_set_style_text_color(set_volt_ch2_title_lbl, lv_color_hex(TEXT_COLOR), LV_PART_MAIN);
	lv_obj_set_style_text_align(set_volt_ch2_title_lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
	lv_obj_set_width(set_volt_ch2_title_lbl, 76);
	lv_obj_align(set_volt_ch2_title_lbl, LV_ALIGN_TOP_LEFT, 168, 140);

	set_volt_ch2_lbl = lv_label_create(lv_screen_active());
	lv_label_set_text(set_volt_ch2_lbl, "0.00 V");
	lv_obj_set_style_text_font(set_volt_ch2_lbl, &lv_font_montserrat_16, LV_PART_MAIN);
	lv_obj_set_style_text_color(set_volt_ch2_lbl, lv_color_hex(TEXT_COLOR), LV_PART_MAIN);
	lv_obj_set_style_text_align(set_volt_ch2_lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
	lv_obj_set_width(set_volt_ch2_lbl, 76);
	lv_obj_align(set_volt_ch2_lbl, LV_ALIGN_TOP_LEFT, 168, 160);

	lv_obj_t * set_curr_ch2_title_lbl = lv_label_create(lv_screen_active());
	lv_label_set_text(set_curr_ch2_title_lbl, "Iset");
	lv_obj_set_style_text_font(set_curr_ch2_title_lbl, &lv_font_montserrat_16, LV_PART_MAIN);
	lv_obj_set_style_text_color(set_curr_ch2_title_lbl, lv_color_hex(TEXT_COLOR), LV_PART_MAIN);
	lv_obj_set_style_text_align(set_curr_ch2_title_lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
	lv_obj_set_width(set_curr_ch2_title_lbl, 76);
	lv_obj_align(set_curr_ch2_title_lbl, LV_ALIGN_TOP_LEFT, 240, 140);

	set_curr_ch2_lbl = lv_label_create(lv_screen_active());
	lv_label_set_text(set_curr_ch2_lbl, "0.00 A");
	lv_obj_set_style_text_font(set_curr_ch2_lbl, &lv_font_montserrat_16, LV_PART_MAIN);
	lv_obj_set_style_text_color(set_curr_ch2_lbl, lv_color_hex(TEXT_COLOR), LV_PART_MAIN);
	lv_obj_set_style_text_align(set_curr_ch2_lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
	lv_obj_set_width(set_curr_ch2_lbl, 76);
	lv_obj_align(set_curr_ch2_lbl, LV_ALIGN_TOP_LEFT, 240, 160);

// CV/CC mode channels parameters
	// channel 1
	lv_obj_t * cv_ch1_lbl = lv_label_create(lv_screen_active());
    lv_label_set_text(cv_ch1_lbl, "CV");
	lv_obj_set_style_text_font(cv_ch1_lbl, &lv_font_montserrat_20, LV_PART_MAIN);
	lv_obj_set_style_text_color(cv_ch1_lbl, lv_color_hex(TEXT_COLOR), LV_PART_MAIN);
	lv_obj_set_style_text_align(cv_ch1_lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
	lv_obj_align(cv_ch1_lbl, LV_ALIGN_TOP_LEFT, 15, 108);

    cv_ch1_led  = lv_led_create(lv_scr_act());
    lv_obj_align(cv_ch1_led, LV_ALIGN_TOP_LEFT, 50, 108);
    lv_led_set_color(cv_ch1_led, lv_color_hex(CH1_ITEMS_COLOR));
    lv_obj_set_size(cv_ch1_led, 20, 20);
    lv_led_off(cv_ch1_led);

	lv_obj_t * cc_ch1_lbl = lv_label_create(lv_screen_active());
    lv_label_set_text(cc_ch1_lbl, "CC");
	lv_obj_set_style_text_font(cc_ch1_lbl, &lv_font_montserrat_20, LV_PART_MAIN);
	lv_obj_set_style_text_color(cc_ch1_lbl, lv_color_hex(TEXT_COLOR), LV_PART_MAIN);
	lv_obj_set_style_text_align(cc_ch1_lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
	lv_obj_align(cc_ch1_lbl, LV_ALIGN_TOP_LEFT, 90, 108);

    cc_ch1_led  = lv_led_create(lv_scr_act());
    lv_obj_align(cc_ch1_led, LV_ALIGN_TOP_LEFT, 125, 108);
    lv_led_set_color(cc_ch1_led, lv_color_hex(CH1_ITEMS_COLOR));
    lv_obj_set_size(cc_ch1_led, 20, 20);
    lv_led_off(cc_ch1_led);

	// channel 2
	lv_obj_t * cv_ch2_lbl = lv_label_create(lv_screen_active());
    lv_label_set_text(cv_ch2_lbl, "CV");
	lv_obj_set_style_text_font(cv_ch2_lbl, &lv_font_montserrat_20, LV_PART_MAIN);
	lv_obj_set_style_text_color(cv_ch2_lbl, lv_color_hex(TEXT_COLOR), LV_PART_MAIN);
	lv_obj_set_style_text_align(cv_ch2_lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
	lv_obj_align(cv_ch2_lbl, LV_ALIGN_TOP_LEFT, 175, 108);

    cv_ch2_led  = lv_led_create(lv_scr_act());
    lv_obj_align(cv_ch2_led, LV_ALIGN_TOP_LEFT, 210, 108);
    lv_led_set_color(cv_ch2_led, lv_color_hex(CH2_ITEMS_COLOR));
    lv_obj_set_size(cv_ch2_led, 20, 20);
    lv_led_off(cv_ch2_led);

	lv_obj_t * cc_ch2_lbl = lv_label_create(lv_screen_active());
    lv_label_set_text(cc_ch2_lbl, "CC");
	lv_obj_set_style_text_font(cc_ch2_lbl, &lv_font_montserrat_20, LV_PART_MAIN);
	lv_obj_set_style_text_color(cc_ch2_lbl, lv_color_hex(TEXT_COLOR), LV_PART_MAIN);
	lv_obj_set_style_text_align(cc_ch2_lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
	lv_obj_align(cc_ch2_lbl, LV_ALIGN_TOP_LEFT, 250, 108);

    cc_ch2_led  = lv_led_create(lv_scr_act());
    lv_obj_align(cc_ch2_led, LV_ALIGN_TOP_LEFT, 285, 108);
    lv_led_set_color(cc_ch2_led, lv_color_hex(CH2_ITEMS_COLOR));
    lv_obj_set_size(cc_ch2_led, 20, 20);
    lv_led_off(cc_ch2_led);

// channels on/off display buttons
    on_off_ch1_btn = lv_button_create(lv_screen_active());
    lv_obj_align(on_off_ch1_btn, LV_ALIGN_TOP_LEFT, 52, 190);
    lv_obj_add_flag(on_off_ch1_btn, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_set_width(on_off_ch1_btn, 56);
    lv_obj_set_height(on_off_ch1_btn, 32);
	lv_obj_set_style_bg_color(on_off_ch1_btn, lv_color_hex(BUTTON_DEFAULT_COLOR), LV_PART_MAIN);
	lv_obj_set_style_bg_opa(on_off_ch1_btn, LV_OPA_COVER, LV_PART_MAIN);

    ch1_btn_label = lv_label_create(on_off_ch1_btn);
    lv_label_set_text(ch1_btn_label, "Off");
    lv_obj_center(ch1_btn_label);

    on_off_ch2_btn = lv_button_create(lv_screen_active());
    lv_obj_align(on_off_ch2_btn, LV_ALIGN_TOP_LEFT, 211, 190);
    lv_obj_add_flag(on_off_ch2_btn, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_set_width(on_off_ch2_btn, 56);
    lv_obj_set_height(on_off_ch2_btn, 32);
	lv_obj_set_style_bg_color(on_off_ch2_btn, lv_color_hex(BUTTON_DEFAULT_COLOR), LV_PART_MAIN);
	lv_obj_set_style_bg_opa(on_off_ch2_btn, LV_OPA_COVER, LV_PART_MAIN);

    ch2_btn_label = lv_label_create(on_off_ch2_btn);
    lv_label_set_text(ch2_btn_label, "Off");
    lv_obj_center(ch2_btn_label);
#else
    test_lbl = lv_label_create(lv_screen_active());
	lv_obj_set_style_text_font(test_lbl, &lv_font_montserrat_20, LV_PART_MAIN);
	lv_obj_set_style_text_color(test_lbl, lv_color_hex(TEXT_COLOR), LV_PART_MAIN);
	lv_obj_set_style_text_align(test_lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
//	lv_obj_center(test_lbl);
	lv_obj_align(test_lbl, LV_ALIGN_TOP_LEFT, 10, 10);
	lv_label_set_text_fmt(test_lbl, "%d", test_cntr);
#endif
}

/**
 * @brief Update LVGL timer
 * @param: None
 * @return: None
 */
void LCDIF_UpdateLvglTimer(void)
{
	static uint16_t lvgl_timer_div;
	if(lvgl_timer_div < LVGL_TIMER_DIV) // update timer with 10 ms period
	{
		lvgl_timer_div++;
	}
	else
	{
	  lvgl_timer_div = 0;

// UI updates shall be done in this callback to prevent LVGL invalidate and rendering tasks issues!
	  updateUI();

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


/********************************************************* UI control functions *************************************************************/
/**
 * @brief Update PSU control screen
 * @param: None
 * @return: None
 */
static void updateUI(void)
{
	if(!(channel1_ui_data.is_update_reg | channel2_ui_data.is_update_reg)) return;

#ifndef LVGL_TEST
// update PSU channel 1 measured parameters
	// measured voltage
	if(channel1_ui_data.is_update_reg & UI_UPDATE_MEAS_VOLT_MASK)
	{
		lv_label_set_text_fmt(meas_volt_ch1_lbl, "%0.2f", (float)channel1_ui_data.channel_measured_data->voltage_mv/1000.0f);
	}

	// measured current
	if(channel1_ui_data.is_update_reg & UI_UPDATE_MEAS_CURR_MASK)
	{
		if(channel1_ui_data.channel_measured_data->current_ma < 1000)
		{
			lv_label_set_text_fmt(meas_curr_ch1_lbl, "%0.3f", (float)channel1_ui_data.channel_measured_data->current_ma/1000.0f);
		}
		else
		{
			lv_label_set_text_fmt(meas_curr_ch1_lbl, "%0.2f", (float)channel1_ui_data.channel_measured_data->current_ma/1000.0f);
		}
	}

	// CV/CC mode indication
	if(channel1_ui_data.is_update_reg & UI_UPDATE_CC_CV_MASK)
	{
		if(channel1_ui_data.channel_measured_data->is_current_limit)
		{
			lv_led_off(cv_ch1_led);
			lv_led_on(cc_ch1_led);
		}
		else
		{
			lv_led_on(cv_ch1_led);
			lv_led_off(cc_ch1_led);
		}
	}

// update PSU channel 1 set parameters
	// set voltage value
	if(channel1_ui_data.is_update_reg & UI_UPDATE_SET_VOLT_MASK)
	{
		lv_label_set_text_fmt(set_volt_ch1_lbl, "%0.2f", (float)channel1_ui_data.channel_set_values->voltageSetVal/100.0f);
	}

	if(channel1_ui_data.is_update_reg & UI_UPDATE_SET_VOLT_STEP_MASK)
	{
		lv_obj_set_style_text_color(set_volt_ch1_lbl, lv_color_hex(set_parameters_lbl_text_colors[channel1_ui_data.channel_set_values->voltageSetStepIdx]), LV_PART_MAIN);
	}

	// set current limit value
	if(channel1_ui_data.is_update_reg & UI_UPDATE_SET_CURR_MASK)
	{
		lv_label_set_text_fmt(set_curr_ch1_lbl, "%0.2f", (float)channel1_ui_data.channel_set_values->currentSetVal/1000.0f);
	}

	if(channel1_ui_data.is_update_reg & UI_UPDATE_SET_CURR_STEP_MASK)
	{
		lv_obj_set_style_text_color(set_curr_ch1_lbl, lv_color_hex(set_parameters_lbl_text_colors[channel1_ui_data.channel_set_values->currentSetStepIdx]), LV_PART_MAIN);
	}

	// is channel enabled
	if(channel1_ui_data.is_update_reg & UI_UPDATE_ON_OFF_MASK)
	{
		lv_obj_set_style_bg_color(on_off_ch1_btn, lv_color_hex(channel1_ui_data.channel_set_values->is_enabled ? CH1_ITEMS_COLOR : BUTTON_DEFAULT_COLOR), LV_PART_MAIN);
		lv_label_set_text(ch1_btn_label, channel1_ui_data.channel_set_values->is_enabled ? "On" : "Off");
	}

// update PSU channel 2 measured parameters
	// measured voltage
	if(channel2_ui_data.is_update_reg & UI_UPDATE_MEAS_VOLT_MASK)
	{
		lv_label_set_text_fmt(meas_volt_ch2_lbl, "%0.2f", (float)channel2_ui_data.channel_measured_data->voltage_mv/1000.0f);
	}

	// measured current
	if(channel2_ui_data.is_update_reg & UI_UPDATE_MEAS_CURR_MASK)
	{
		if(channel2_ui_data.channel_measured_data->current_ma < 1000)
		{
			lv_label_set_text_fmt(meas_curr_ch2_lbl, "%0.3f", (float)channel2_ui_data.channel_measured_data->current_ma/1000.0f);
		}
		else
		{
			lv_label_set_text_fmt(meas_curr_ch2_lbl, "%0.2f", (float)channel2_ui_data.channel_measured_data->current_ma/1000.0f);
		}
	}

	// CV/CC mode indication
	if(channel2_ui_data.is_update_reg & UI_UPDATE_CC_CV_MASK)
	{
		if(channel2_ui_data.channel_measured_data->is_current_limit)
		{
			lv_led_off(cv_ch2_led);
			lv_led_on(cc_ch2_led);
		}
		else
		{
			lv_led_on(cv_ch2_led);
			lv_led_off(cc_ch2_led);
		}
	}

// update PSU channel 2 set parameters
	// set voltage value
	if(channel2_ui_data.is_update_reg & UI_UPDATE_SET_VOLT_MASK)
	{
		lv_label_set_text_fmt(set_volt_ch2_lbl, "%0.2f", (float)channel2_ui_data.channel_set_values->voltageSetVal/100.0f);
	}

	if(channel2_ui_data.is_update_reg & UI_UPDATE_SET_VOLT_STEP_MASK)
	{
		lv_obj_set_style_text_color(set_volt_ch2_lbl, lv_color_hex(set_parameters_lbl_text_colors[channel2_ui_data.channel_set_values->voltageSetStepIdx]), LV_PART_MAIN);
	}

	// set current limit value
	if(channel2_ui_data.is_update_reg & UI_UPDATE_SET_CURR_MASK)
	{
		lv_label_set_text_fmt(set_curr_ch2_lbl, "%0.2f", (float)channel2_ui_data.channel_set_values->currentSetVal/1000.0f);
	}

	if(channel2_ui_data.is_update_reg & UI_UPDATE_SET_CURR_STEP_MASK)
	{
		lv_obj_set_style_text_color(set_curr_ch2_lbl, lv_color_hex(set_parameters_lbl_text_colors[channel2_ui_data.channel_set_values->currentSetStepIdx]), LV_PART_MAIN);
	}

	// is channel enabled
	if(channel2_ui_data.is_update_reg & UI_UPDATE_ON_OFF_MASK)
	{
		lv_obj_set_style_bg_color(on_off_ch2_btn, lv_color_hex(channel2_ui_data.channel_set_values->is_enabled ? CH2_ITEMS_COLOR : BUTTON_DEFAULT_COLOR), LV_PART_MAIN);
		lv_label_set_text(ch2_btn_label, channel2_ui_data.channel_set_values->is_enabled ? "On" : "Off");
	}
#else
	test_cntr++;
	lv_label_set_text_fmt(test_lbl, "%d", test_cntr);
#endif

	// reset update registers
	channel1_ui_data.is_update_reg = 0;
	channel2_ui_data.is_update_reg = 0;
}

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
	uint16_t height = area->y2 - area->y1 + 1;
	uint16_t width = area->x2 - area->x1 + 1;

	ili9328_WriteGRAM(area->x1, area->y1, width, height, (uint16_t*)color_p);
	lv_display_flush_ready(disp);

}
