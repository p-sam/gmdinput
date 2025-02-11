#pragma once

#include <stdint.h>

#define SDLMAPPING_GUID_SIZE 32
#define SDLMAPPING_NAME_MAX 63
#define SDLMAPPING_PLATFORM_MAX 15

#define SDLMAPPING_AXISVAL_MIN INT16_MIN
#define SDLMAPPING_AXISVAL_MAX INT16_MAX

enum {
	SDLMAPPING_BIND_NONE = 0,
	SDLMAPPING_BIND_BUTTON,
	SDLMAPPING_BIND_AXIS,
	SDLMAPPING_BIND_HAT,
	SDLMAPPING_BIND_ENUMCOUNT,
};

enum {
	SDLMAPPING_BUTTON_A = 0,
	SDLMAPPING_BUTTON_B,
	SDLMAPPING_BUTTON_X,
	SDLMAPPING_BUTTON_Y,
	SDLMAPPING_BUTTON_BACK,
	SDLMAPPING_BUTTON_GUIDE,
	SDLMAPPING_BUTTON_START,
	SDLMAPPING_BUTTON_LEFT_STICK,
	SDLMAPPING_BUTTON_RIGHT_STICK,
	SDLMAPPING_BUTTON_LEFT_SHOULDER,
	SDLMAPPING_BUTTON_RIGHT_SHOULDER,
	SDLMAPPING_BUTTON_DPAD_UP,
	SDLMAPPING_BUTTON_DPAD_RIGHT,
	SDLMAPPING_BUTTON_DPAD_DOWN,
	SDLMAPPING_BUTTON_DPAD_LEFT,
	SDLMAPPING_BUTTON_MISC1,
	SDLMAPPING_BUTTON_MISC2,
	SDLMAPPING_BUTTON_PADDLE1,
	SDLMAPPING_BUTTON_PADDLE2,
	SDLMAPPING_BUTTON_PADDLE3,
	SDLMAPPING_BUTTON_PADDLE4,
	SDLMAPPING_BUTTON_TOUCHPAD,
	SDLMAPPING_BUTTON_ENUMCOUNT,
};

enum {
	SDLMAPPING_AXIS_LEFT_X = 0,
	SDLMAPPING_AXIS_LEFT_Y,
	SDLMAPPING_AXIS_RIGHT_X,
	SDLMAPPING_AXIS_RIGHT_Y,
	SDLMAPPING_AXIS_LEFT_TRIGGER,
	SDLMAPPING_AXIS_RIGHT_TRIGGER,
	SDLMAPPING_AXIS_ENUMCOUNT,
};

enum {
	SDLMAPPING_RANGE_FULL = 0,
	SDLMAPPING_RANGE_POSITIVE = 1,
	SDLMAPPING_RANGE_NEGATIVE = 2,
	SDLMAPPING_RANGE_ENUMCOUNT,
};

enum {
	SDLMAPPING_HAT_UP = 0,
	SDLMAPPING_HAT_RIGHT,
	SDLMAPPING_HAT_DOWN,
	SDLMAPPING_HAT_LEFT,
	SDLMAPPING_HAT_ENUMCOUNT,
};

enum {
	SDLMAPPING_ERROR_OK = 0,
	SDLMAPPING_ERROR_SYNTAX,
	SDLMAPPING_ERROR_NUMBER,
	SDLMAPPING_ERROR_BAD_GUID,
	SDLMAPPING_ERROR_BAD_NAME,
	SDLMAPPING_ERROR_BAD_HAT,
	SDLMAPPING_ERROR_UNKNOWN_BIND_KEY,
	SDLMAPPING_ERROR_UNKNOWN_BIND_TYPE,
	SDLMAPPING_ERROR_ENUMCOUNT,
};

typedef struct {
	int8_t min;
	int8_t max;
	uint8_t invert;
} SDLMappingRange;

typedef struct {
	uint8_t type;
	uint8_t index;
	union {
		uint8_t hat;
		struct {
			uint8_t range;
			uint8_t invert;
		} axis;
	};
} SDLMappingBind;

typedef struct {
	SDLMappingBind r[SDLMAPPING_RANGE_ENUMCOUNT];
} SDLMappingAxisBind;

typedef struct {
	SDLMappingBind buttons[SDLMAPPING_BUTTON_ENUMCOUNT];
	SDLMappingAxisBind axes[SDLMAPPING_AXIS_ENUMCOUNT];
} SDLMappingBinds;

typedef struct {
	char guid[SDLMAPPING_GUID_SIZE + 2];
	char name[SDLMAPPING_NAME_MAX + 1];
	char platform[SDLMAPPING_PLATFORM_MAX + 1];
	SDLMappingBinds binds;
} SDLMapping;

typedef struct {
    int16_t (*axis)(void* userdata, uint8_t index);
    uint8_t (*button)(void* userdata, uint8_t index);
	uint8_t (*hat)(void* userdata, uint8_t index);
} SDLMappingDriver;

uint8_t sdlmapping_parse(const char* str, SDLMapping* out_mapping, size_t* out_pos);
const char* sdlmapping_button_name(uint8_t index);
const char* sdlmapping_axis_name(uint8_t index);

uint8_t sdlmapping_button_get(const SDLMappingDriver* driver, const SDLMappingBind* bind, void* userdata);
int16_t sdlmapping_axis_get(const SDLMappingDriver* driver, const SDLMappingAxisBind* axisBind, void* userdata);
