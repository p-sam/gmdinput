# gmdinput

[![Build](https://github.com/p-sam/gmdinput/actions/workflows/build.yml/badge.svg?branch=develop)](https://github.com/p-sam/gmdinput/actions/workflows/build.yml)

GameMaker extension that allows using DirectInput8 API to fetch gamepad states

This allows using more 4 gamepads on Windows (max 16 customizable at build time) , and should be as simple as replacing `gamepad_*` functions by their `gmdinput_gamepad_*` counterparts.

DLL embeds the community sourced database of game controller mappings [SDL_GameControllerDB](https://github.com/mdqinc/SDL_GameControllerDB), which is used to map raw axes/hat/buttons data to the common gamepad layout.

# Usage

```gml
// add the extension to your project

// now all gamepad functions should work
gmdinput_gamepad_is_supported();
gmdinput_gamepad_get_device_count();
gmdinput_gamepad_button_check(0, gp_face1);

// it should replace the original functions
// if using the unprefixed version of the extension
if(gamepad_is_supported() == 42) {
    show_debug_message("using gmdinput unprefixed");
}
```

# API

```c
double gmdinput_gamepad_is_supported();
double gmdinput_gamepad_is_connected(double device);
const char* gmdinput_gamepad_get_guid(double device);
double gmdinput_gamepad_get_device_count(void);
const char* gmdinput_gamepad_get_description(double device);
double gmdinput_gamepad_get_button_threshold(double device);
double gmdinput_gamepad_get_axis_deadzone(double device);
double gmdinput_gamepad_get_option(double device, const char* option);
double gmdinput_gamepad_set_button_threshold(double device, double threshold);
double gmdinput_gamepad_set_axis_deadzone(double device, double deadzone);
double gmdinput_gamepad_axis_count(double device);
double gmdinput_gamepad_axis_value(double device, double axis);
double gmdinput_gamepad_button_check(double device, double button);
double gmdinput_gamepad_button_check_pressed(double device, double button);
double gmdinput_gamepad_button_check_released(double device, double button);
double gmdinput_gamepad_button_count(double device);
double gmdinput_gamepad_button_value(double device, double button);
double gmdinput_gamepad_hat_count(double device);
double gmdinput_gamepad_hat_value(double device, double hat);

// no-op functions
double gmdinput_gamepad_set_vibration(double device, double left, double right);
double gmdinput_gamepad_set_colour(double device, double color);
double gmdinput_gamepad_set_option(double device, const char* option, double value);
const char* gmdinput_gamepad_get_mapping(double device);
double gmdinput_gamepad_test_mapping(double device, const char* mapping);
double gmdinput_gamepad_remove_mapping(double device);

// advanced (these functions are automatically called for you on init/shutdown of the game)
double gmdinput_init(void* hwnd, void* video_d3d11_swapchain);
double gmdinput_shutdown();
double gmdinput_poll(double refresh);
```
