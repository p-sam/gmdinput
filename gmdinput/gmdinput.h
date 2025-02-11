#pragma once

#include "gmdinput_export.h"

#define GM_gp_face1 0x8001
#define GM_gp_face2 0x8002
#define GM_gp_face3 0x8003
#define GM_gp_face4 0x8004
#define GM_gp_shoulderl 0x8005
#define GM_gp_shoulderr 0x8006
#define GM_gp_shoulderlb 0x8007
#define GM_gp_shoulderrb 0x8008
#define GM_gp_select 0x8009
#define GM_gp_start 0x800a
#define GM_gp_stickl 0x800b
#define GM_gp_stickr 0x800c
#define GM_gp_padu 0x800d
#define GM_gp_padd 0x800e
#define GM_gp_padl 0x800f
#define GM_gp_padr 0x8010

#define GM_BUTTONS_START GM_gp_face1
#define GM_BUTTONS_END GM_gp_padr
#define GM_BUTTONS_COUNT (GM_BUTTONS_END - GM_BUTTONS_START + 1)

#define GM_gp_axislh 0x8011
#define GM_gp_axislv 0x8012
#define GM_gp_axisrh 0x8013
#define GM_gp_axisrv 0x8014

#define GM_AXES_START GM_gp_axislh
#define GM_AXES_END GM_gp_axisrv
#define GM_AXES_COUNT (GM_AXES_END - GM_AXES_START + 1)

GMDINPUT_EXPORT double gmdinput_init(void* hwnd, void* video_d3d11_swapchain);
GMDINPUT_EXPORT double gmdinput_shutdown(void);
GMDINPUT_EXPORT double gmdinput_poll(double refresh);

GMDINPUT_EXPORT double gmdinput_gamepad_is_supported(void);
GMDINPUT_EXPORT double gmdinput_gamepad_is_connected(double device);
GMDINPUT_EXPORT const char* gmdinput_gamepad_get_guid(double device);
GMDINPUT_EXPORT double gmdinput_gamepad_get_device_count(void);
GMDINPUT_EXPORT const char* gmdinput_gamepad_get_description(double device);
GMDINPUT_EXPORT double gmdinput_gamepad_get_button_threshold(double device);
GMDINPUT_EXPORT double gmdinput_gamepad_get_axis_deadzone(double device);
GMDINPUT_EXPORT double gmdinput_gamepad_get_option(double device, const char* option);
GMDINPUT_EXPORT double gmdinput_gamepad_set_button_threshold(double device, double threshold);
GMDINPUT_EXPORT double gmdinput_gamepad_set_axis_deadzone(double device, double deadzone);
GMDINPUT_EXPORT double gmdinput_gamepad_axis_count(double device);
GMDINPUT_EXPORT double gmdinput_gamepad_axis_value(double device, double axis);
GMDINPUT_EXPORT double gmdinput_gamepad_button_check(double device, double button);
GMDINPUT_EXPORT double gmdinput_gamepad_button_check_pressed(double device, double button);
GMDINPUT_EXPORT double gmdinput_gamepad_button_check_released(double device, double button);
GMDINPUT_EXPORT double gmdinput_gamepad_button_count(double device);
GMDINPUT_EXPORT double gmdinput_gamepad_button_value(double device, double button);
GMDINPUT_EXPORT double gmdinput_gamepad_hat_count(double device);
GMDINPUT_EXPORT double gmdinput_gamepad_hat_value(double device, double hat);

GMDINPUT_EXPORT double gmdinput_gamepad_set_vibration(double device, double left, double right);
GMDINPUT_EXPORT double gmdinput_gamepad_set_colour(double device, double color);
GMDINPUT_EXPORT double gmdinput_gamepad_set_option(double device, const char* option, double value);
GMDINPUT_EXPORT const char* gmdinput_gamepad_get_mapping(double device);
GMDINPUT_EXPORT double gmdinput_gamepad_test_mapping(double device, const char* mapping);
GMDINPUT_EXPORT double gmdinput_gamepad_remove_mapping(double device);
