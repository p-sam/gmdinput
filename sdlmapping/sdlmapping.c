#include "sdlmapping.h"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define NORM_DOUBLE(range_min, range_max, value) (((double)(value) - (double)(range_min)) / ((double)(range_max) - (double)(range_min)))
#define DENORM(value, range_min, range_max) ((value) * ((range_max) - (range_min)) + (range_min))

static const char* g_button_names[] = {
	"a",
	"b",
	"x",
	"y",
	"back",
	"guide",
	"start",
	"leftstick",
	"rightstick",
	"leftshoulder",
	"rightshoulder",
	"dpup",
	"dpright",
	"dpdown",
	"dpleft",
	"misc1",
	"misc2",
	"paddle1",
	"paddle2",
	"paddle3",
	"paddle4",
	"touchpad"
};
_Static_assert((sizeof(g_button_names) / sizeof(g_button_names[0])) == SDLMAPPING_BUTTON_ENUMCOUNT, "g_button_names size");

static const char* g_axis_names[] = {
	"leftx",
	"lefty",
	"rightx",
	"righty",
	"lefttrigger",
	"righttrigger"
};
_Static_assert((sizeof(g_axis_names) / sizeof(g_axis_names[0])) == SDLMAPPING_AXIS_ENUMCOUNT, "g_axis_names size");

struct {
    int16_t min;
    int16_t max;
    int16_t threshold;
} g_axis_ranges[] = {
    {SDLMAPPING_AXISVAL_MIN, SDLMAPPING_AXISVAL_MAX, 0},
    {0, SDLMAPPING_AXISVAL_MAX, SDLMAPPING_AXISVAL_MAX/2},
    {0, SDLMAPPING_AXISVAL_MIN, SDLMAPPING_AXISVAL_MIN/2},
};
_Static_assert((sizeof(g_axis_ranges) / sizeof(g_axis_ranges[0])) == SDLMAPPING_RANGE_ENUMCOUNT, "g_axis_ranges size");

static inline bool _sdlmapping_range_char(const char c, uint8_t* out_range) {
	switch (c) {
	case '+':
		*out_range = SDLMAPPING_RANGE_POSITIVE;
		return true;
	case '-':
		*out_range = SDLMAPPING_RANGE_NEGATIVE;
		return true;
	default:
		*out_range = SDLMAPPING_RANGE_FULL;
		return false;
	}
}

static inline SDLMappingBind* _sdlmapping_find_bind(SDLMappingBinds* binds, const char* p, size_t len) {
	for (uint8_t i = 0; i < SDLMAPPING_BUTTON_ENUMCOUNT; i++) {
		if (strlen(g_button_names[i]) != len) {
			continue;
		}

		if(memcmp(g_button_names[i], p, len) != 0) {
			continue;
		}

		return &binds->buttons[i];
	}


	uint8_t range;
	if (_sdlmapping_range_char(*p, &range)) {
		p++;
		len--;
	}

	for (uint8_t i = 0; i < SDLMAPPING_AXIS_ENUMCOUNT; i++) {
		if (strlen(g_axis_names[i]) != len) {
			continue;
		}

		if (memcmp(g_axis_names[i], p, len) != 0) {
			continue;
		}

		return &binds->axes[i].r[range];
	}

	return NULL;
}

static inline bool _sdlmapping_find_hat(uint8_t mask, uint8_t* out) {
	for (uint8_t i = 0; i < SDLMAPPING_HAT_ENUMCOUNT; i++) {
		if (mask == (1 << i)) {
			*out = i;
			return true;
		}
	}
	return false;
}

uint8_t sdlmapping_parse(const char* str, SDLMapping* out_mapping, size_t* out_pos) {
#define _POS_SET(x) if(out_pos != NULL) { *out_pos = (x); }
#define _ERROR_RET(errcode) do { _POS_SET(p - str); return errcode; } while(0)
#define _ERROR_IF(cond, errcode) if(cond) { _ERROR_RET(errcode); }

	memset(out_mapping, 0, sizeof(SDLMapping));
	const char* p = str;

	size_t len = strspn(p, "0123456789abcdefABCDEF");

	memcpy(out_mapping->guid, p, len);
	p += len;
	_ERROR_IF(len != SDLMAPPING_GUID_SIZE, SDLMAPPING_ERROR_BAD_GUID);
	_ERROR_IF(*p != ',', SDLMAPPING_ERROR_SYNTAX);
	p++;

	len = strcspn(p, ":,");
	memcpy(out_mapping->name, p, len > SDLMAPPING_NAME_MAX ? SDLMAPPING_NAME_MAX : len);
	p += len;
	_ERROR_IF(*p != ',', SDLMAPPING_ERROR_BAD_NAME);
	p++;

	while (*p) {
		len = strcspn(p, ":,");
		_ERROR_IF(p[len] != ':', SDLMAPPING_ERROR_SYNTAX);

		if (len == 8 && _strnicmp("platform", p, len) == 0) {
			p += 9;

			len = strcspn(p, ":,");
			memcpy(out_mapping->platform, p, len > SDLMAPPING_PLATFORM_MAX ? SDLMAPPING_PLATFORM_MAX : len);
			p += len;

			_ERROR_IF(*p != ',', SDLMAPPING_ERROR_SYNTAX);
			p++;
			continue;
		}

		SDLMappingBind* bind = _sdlmapping_find_bind(&out_mapping->binds, p, len);
		_ERROR_IF(bind == NULL, SDLMAPPING_ERROR_UNKNOWN_BIND_KEY);
		p += len + 1;

		uint8_t axisRange;
		bool hasAxisRange = _sdlmapping_range_char(*p, &axisRange);
		if (hasAxisRange) {
			p++;
		}

		switch (*p) {
		case 'b':
			bind->type = SDLMAPPING_BIND_BUTTON;
			break;
		case 'a':
			bind->type = SDLMAPPING_BIND_AXIS;
			break;
		case 'h':
			bind->type = SDLMAPPING_BIND_HAT;
			break;
		default:
			_ERROR_RET(SDLMAPPING_ERROR_UNKNOWN_BIND_TYPE);
		}

		_ERROR_IF(bind->type != SDLMAPPING_BIND_AXIS && hasAxisRange, SDLMAPPING_ERROR_UNKNOWN_BIND_TYPE);
		p++;

		unsigned long n;
		n = strtoul(p, (char**)&p, 10);
		_ERROR_IF(n >= UINT8_MAX, SDLMAPPING_ERROR_NUMBER);

		bind->index = (uint8_t)n;

		if (bind->type == SDLMAPPING_BIND_HAT) {
			_ERROR_IF(*p != '.', SDLMAPPING_ERROR_SYNTAX);
			p++;

			n = strtoul(p, (char**)&p, 10);
			_ERROR_IF(n >= UINT8_MAX, SDLMAPPING_ERROR_NUMBER);
			_ERROR_IF(!_sdlmapping_find_hat((uint8_t)n, &bind->hat), SDLMAPPING_ERROR_BAD_HAT);
		}

		if (bind->type == SDLMAPPING_BIND_AXIS) {
			bind->axis.range = axisRange;

			if (*p == '~') {
				bind->axis.invert = 1;
				p++;
			}
		}

		_ERROR_IF(*p != ',', SDLMAPPING_ERROR_SYNTAX);
		p++;
	}

	_ERROR_RET(SDLMAPPING_ERROR_OK);

#undef _ERROR_IF
#undef _ERROR_RET
#undef _POS_SET
}

const char* sdlmapping_button_name(uint8_t index) {
	return index >= 0 && index < SDLMAPPING_BUTTON_ENUMCOUNT ? g_button_names[index] : NULL;
}

const char* sdlmapping_axis_name(uint8_t index) {
	return index >= 0 && index < SDLMAPPING_AXIS_ENUMCOUNT ? g_axis_names[index] : NULL;
}

uint8_t sdlmapping_button_get(const SDLMappingDriver* driver, const SDLMappingBind* bind, void* userdata) {
	uint8_t v;

	switch (bind->type) {
		case SDLMAPPING_BIND_BUTTON:
			return driver->button(userdata, bind->index) ? 1 : 0;
		case SDLMAPPING_BIND_HAT:
			v = 1 << bind->hat;
			return (driver->hat(userdata, bind->index) & v) == v;
		case SDLMAPPING_BIND_AXIS:
			if(bind->axis.range >= SDLMAPPING_RANGE_ENUMCOUNT) {
				return 0;
			}
		
			v = g_axis_ranges[bind->axis.range].min > g_axis_ranges[bind->axis.range].max;

			if(bind->axis.invert) {
				v = !v;
			}
		
			if(v) {
				return driver->axis(userdata, bind->index) <= g_axis_ranges[bind->axis.range].threshold;
			}

			return driver->axis(userdata, bind->index) >= g_axis_ranges[bind->axis.range].threshold;
		default:
			return 0;
	}
}

static inline int16_t _sdlmapping_axis_range_get(const SDLMappingDriver* driver, const SDLMappingBind* bind, uint8_t range, void* userdata) {
    if (bind->type == SDLMAPPING_BIND_BUTTON || bind->type == SDLMAPPING_BIND_HAT) {
        return sdlmapping_button_get(driver, bind, userdata) ? g_axis_ranges[range].max : g_axis_ranges[range].min;
    }

    if (bind->type != SDLMAPPING_BIND_AXIS || bind->axis.range > SDLMAPPING_RANGE_ENUMCOUNT) {
        return 0;
    }

    int16_t min = g_axis_ranges[bind->axis.range].min;
    int16_t max = g_axis_ranges[bind->axis.range].max;
	if(bind->axis.invert) {
		int16_t tmp = min;
		min = max;
		max = tmp;
	}

    int16_t val = driver->axis(userdata, bind->index);
	bool inrange = (min > max) ? (val >= max && val <= min) : (val >= min && val <= max);
	if(!inrange) {
		return 0;
	}

    if (g_axis_ranges[range].min != min || g_axis_ranges[range].max != max) {
		double normalized = NORM_DOUBLE(min, max, val);
		val = (int16_t)(DENORM(normalized, g_axis_ranges[range].min, g_axis_ranges[range].max));
    }

	return val;
}

int16_t sdlmapping_axis_get(const SDLMappingDriver* driver, const SDLMappingAxisBind* axisBind, void* userdata) {
	int16_t result = 0;

	for(uint8_t i = 0; i < SDLMAPPING_RANGE_ENUMCOUNT; i++) {
		result += _sdlmapping_axis_range_get(driver, &axisBind->r[i], i, userdata);
	}

	return result;
}
