#include "gmdinput.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Dbt.h>

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <dxgi.h>

#include <MinHook.h>

#include <stdio.h>
#include <sdlmapping.h>
#include "sdlmappings.inc.h"

#ifndef NDEBUG
#define DEBUG_LOG(fmt, ...) printf("[gmextdinput][%s:%d] " fmt "\n", __func__, __LINE__, ##__VA_ARGS__)
#else
#define DEBUG_LOG(fmt, ...)
#endif

#define ERROR_LOG(fmt, ...) printf("[gmextdinput][!] " fmt "\n", ##__VA_ARGS__)

#define NORM_DOUBLE(range_min, range_max, value) (((double)(value) - (double)(range_min)) / ((double)(range_max) - (double)(range_min)))
#define DENORM(value, range_min, range_max) ((value) * ((range_max) - (range_min)) + (range_min))

#define GUID_FMT_STR "%08x%04x%04x%02x%02x%02x%02x%02x%02x%02x%02x"
#define GUID_FMT_ARG(guid) (guid).Data1, (guid).Data2, (guid).Data3, (guid).Data4[0], (guid).Data4[1], (guid).Data4[2], (guid).Data4[3], (guid).Data4[4], (guid).Data4[5], (guid).Data4[6], (guid).Data4[7]

typedef HRESULT (WINAPI *IDXGISwapChain_Present_fn)(IDXGISwapChain *This, UINT sync_interval, UINT flags);

typedef struct {
    LPDIRECTINPUTDEVICE8 device;
    DIJOYSTATE prev;
    DIJOYSTATE current;
    SDLGenMapping* mapping;
    double deadzone;
    double threshold;
    int povCount;
    int axisCount;
    int buttonCount;
} Joystick;

struct {
    BOOL trigger;
    uint8_t index;
} g_gm_buttons[] = {
    /* GM_gp_face1 */ {FALSE, SDLMAPPING_BUTTON_A},
    /* GM_gp_face2 */ {FALSE, SDLMAPPING_BUTTON_B},
    /* GM_gp_face3 */ {FALSE, SDLMAPPING_BUTTON_X},
        /* GM_gp_face4 */{FALSE, SDLMAPPING_BUTTON_Y},
        /* GM_gp_shoulderl */{FALSE, SDLMAPPING_BUTTON_LEFT_SHOULDER},
        /* GM_gp_shoulderr */{FALSE, SDLMAPPING_BUTTON_RIGHT_SHOULDER},
        /* GM_gp_shoulderlb */{TRUE, SDLMAPPING_AXIS_LEFT_TRIGGER},
        /* GM_gp_shoulderrb */{TRUE, SDLMAPPING_AXIS_RIGHT_TRIGGER},
        /* GM_gp_select */{FALSE, SDLMAPPING_BUTTON_BACK},
        /* GM_gp_start */{FALSE, SDLMAPPING_BUTTON_START},
        /* GM_gp_stickl */{FALSE, SDLMAPPING_BUTTON_LEFT_STICK},
        /* GM_gp_stickr */{FALSE, SDLMAPPING_BUTTON_RIGHT_STICK},
        /* GM_gp_padu */{FALSE, SDLMAPPING_BUTTON_DPAD_UP},
        /* GM_gp_padd */{FALSE, SDLMAPPING_BUTTON_DPAD_DOWN},
        /* GM_gp_padl */{FALSE, SDLMAPPING_BUTTON_DPAD_LEFT},
        /* GM_gp_padr */{FALSE, SDLMAPPING_BUTTON_DPAD_RIGHT},
 };
 _Static_assert(sizeof(g_gm_buttons) / sizeof(g_gm_buttons[0]) == (GM_BUTTONS_END - GM_BUTTONS_START) + 1, "gm buttons");

 struct {
    BOOL invert;
    uint8_t index;
 } g_gm_axes[] = {
     /* GM_gp_axislh */ {FALSE, SDLMAPPING_AXIS_LEFT_X},
     /* GM_gp_axislv */ {TRUE, SDLMAPPING_AXIS_LEFT_Y},
     /* GM_gp_axisrh */ {FALSE, SDLMAPPING_AXIS_RIGHT_X},
     /* GM_gp_axisrv */ {TRUE, SDLMAPPING_AXIS_RIGHT_Y},
 };
 _Static_assert(sizeof(g_gm_axes) / sizeof(g_gm_axes[0]) == (GM_AXES_END - GM_AXES_START) + 1, "gm axes");

 HINSTANCE g_hInstance = NULL;
 HWND g_hWnd = NULL;
 WNDPROC g_wndProc = NULL;
 HDEVNOTIFY g_hDevNotify = NULL;
 LPDIRECTINPUT8 g_di8 = NULL;
 IDXGISwapChain_Present_fn g_swapchainPresentFn = NULL;

 Joystick g_joysticks[MAX_GAMEPADS] = { 0 };

 int g_joystickCount = 0;
 BOOL g_joystickRefresh = TRUE;

static inline double _clamp(double v, double min, double max) {
    if(v > max) {
        return max;
    }
    if(v < min) {
        return min;
    }
    return v;
}

static inline double _apply_deadzone(double deadzone, double v) {
    if(v > deadzone || v < -deadzone) {
        return v;
    }

    return 0.0;
}

 static inline int16_t _driver_axis(const DIJOYSTATE* state, uint8_t axisIndex) {
     switch (axisIndex) {
     case 0:
         return (int16_t)state->lX;
     case 1:
         return (int16_t)state->lY;
     case 2:
         return (int16_t)state->lZ;
     case 3:
         return (int16_t)state->lRx;
     case 4:
         return (int16_t)state->lRy;
     case 5:
         return (int16_t)state->lRz;
     case 6:
         return (int16_t)state->rglSlider[0];
     case 7:
         return (int16_t)state->rglSlider[1];
     default:
         return 0;
     }
 }

 static inline uint8_t _driver_button(const DIJOYSTATE* state, uint8_t btnIndex) {
     return btnIndex < 32 && state->rgbButtons[btnIndex];
 }

 static inline uint8_t _driver_hat(const DIJOYSTATE* state, uint8_t hatIndex) {
     if (hatIndex >= 4) {
         return 0;
     }

     switch (state->rgdwPOV[hatIndex]) {
     case 0:
         return 0b0001;
     case 4500:
         return 0b0011;
     case 9000:
         return 0b0010;
     case 13500:
         return 0b0110;
     case 18000:
         return 0b0100;
     case 22500:
         return 0b1100;
     case 27000:
         return 0b1000;
     case 31500:
         return 0b1001;
     default:
         return 0;
     }
 }

const SDLMappingDriver g_sdlmapping_driver = {
    .axis = _driver_axis,
    .button = _driver_button,
    .hat = _driver_hat,
};

static inline double _map_axis_value(const Joystick* joy, int val, BOOL trigger) {
    return _apply_deadzone(joy->deadzone, DENORM(NORM_DOUBLE(SDLMAPPING_AXISVAL_MIN, SDLMAPPING_AXISVAL_MAX, val), trigger ? 0.0 : -1.0, 1.0));
}

static inline double _gm_button(const Joystick* joy, const DIJOYSTATE* state, int btnIndex) {
    if (btnIndex < GM_BUTTONS_START || btnIndex > GM_BUTTONS_END) {
        if(btnIndex < 0 || btnIndex > UINT8_MAX) {
            return 0.0;
        }
        return _driver_button(state, (uint8_t)btnIndex);
    }

    btnIndex -= GM_BUTTONS_START;
    if (!g_gm_buttons[btnIndex].trigger) {
        return (double)sdlmapping_button_get(&g_sdlmapping_driver, &joy->mapping->binds.buttons[g_gm_buttons[btnIndex].index], (void*)state);
    }

    int val = sdlmapping_axis_get(&g_sdlmapping_driver, &joy->mapping->binds.axes[g_gm_buttons[btnIndex].index], (void*)state);
    return _map_axis_value(joy, val, TRUE);
}

static inline double _gm_button_check(const Joystick* joy, const DIJOYSTATE* state, int btnIndex) {
    return _gm_button(joy, state, btnIndex) >= joy->threshold;
}

static inline double _gm_axis(const DIJOYSTATE* state, const Joystick* joy, int axisIndex) {
    if (axisIndex < GM_AXES_START || axisIndex > GM_AXES_END) {
        if(axisIndex < 0 || axisIndex > UINT8_MAX) {
            return 0.0;
        }
        return _map_axis_value(joy, _driver_axis(state, (uint8_t)axisIndex), FALSE);
    }

    axisIndex -= GM_AXES_START;
    int val = sdlmapping_axis_get(&g_sdlmapping_driver, &joy->mapping->binds.axes[g_gm_axes[axisIndex].index], (void*)state);

    if(g_gm_axes[axisIndex].invert) {
        val = -val;
    }

    return _map_axis_value(joy, val, FALSE);
}

static LRESULT WINAPI _window_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    DEBUG_LOG("hwnd: %p; msg: %u; wParam: %p; lParam: %p", hwnd, msg, wParam, lParam);

    if (msg == WM_DEVICECHANGE) {
        switch (wParam) {
        case DBT_DEVNODES_CHANGED:
        case DBT_DEVICEARRIVAL:
        case DBT_DEVICEREMOVECOMPLETE:
            if (lParam && ((DEV_BROADCAST_HDR*)(lParam))->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE) {
                g_joystickRefresh = TRUE;
            }
            break;
        }
    }

    if(g_wndProc) {
	    return CallWindowProc(g_wndProc, hwnd, msg, wParam, lParam);
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

static BOOL CALLBACK _enum_objects_callback(const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* pContext) {
    Joystick* joystick = (Joystick*)pContext;
    if (pdidoi->dwType & DIDFT_AXIS) {
        DIPROPRANGE diprg = { 0 };

        diprg.diph.dwSize = sizeof(DIPROPRANGE);
        diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER);
        diprg.diph.dwHow = DIPH_BYID;
        diprg.diph.dwObj = pdidoi->dwType;
        diprg.lMin = SDLMAPPING_AXISVAL_MIN;
        diprg.lMax = SDLMAPPING_AXISVAL_MAX;

        HRESULT hr = IDirectInputDevice_SetProperty(joystick->device, DIPROP_RANGE, &diprg.diph);
        if (hr < 0) {
            ERROR_LOG("IDirectInputDevice_SetProperty DIPROP_RANGE [%i]", hr);
            return DIENUM_STOP;
        }
        
        DIPROPDWORD dipd;
        dipd.diph.dwSize = sizeof(DIPROPDWORD);
        dipd.diph.dwHeaderSize = sizeof(DIPROPHEADER);
        dipd.diph.dwObj = pdidoi->dwType;
        dipd.diph.dwHow = DIPH_BYID;
        dipd.dwData = 0;

        hr = IDirectInputDevice_SetProperty(joystick->device, DIPROP_DEADZONE, &dipd.diph);
        if (hr < 0) {
            ERROR_LOG("IDirectInputDevice_SetProperty DIPROP_DEADZONE [%i]", hr);
            return DIENUM_STOP;
        }

        joystick->axisCount++;
    }

    if (pdidoi->dwType & DIDFT_POV) {
        joystick->povCount++;
    }

    if (pdidoi->dwType & DIDFT_BUTTON) {
        joystick->buttonCount++;
    }

    return DIENUM_CONTINUE;
}

static BOOL _init_joystick(Joystick* joystick) {
    HRESULT hr = IDirectInputDevice8_SetDataFormat(joystick->device, &c_dfDIJoystick);
    if (hr < 0) {
        ERROR_LOG("IDirectInputDevice8_SetDataFormat [%i]", hr);
        return FALSE;
    }

    hr = IDirectInputDevice8_SetCooperativeLevel(joystick->device, g_hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
    if (hr < 0) {
        ERROR_LOG("IDirectInputDevice8_SetCooperativeLevel [%i]", hr);
        return FALSE;
    }

    DIPROPDWORD dipd = {0};
    dipd.diph.dwSize = sizeof(DIPROPDWORD);
    dipd.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    dipd.diph.dwHow = DIPH_DEVICE;
    dipd.dwData = DIPROPAXISMODE_ABS;

    hr = IDirectInputDevice_SetProperty(joystick->device, DIPROP_AXISMODE, &dipd.diph);
    if (hr < 0) {
        ERROR_LOG("IDirectInputDevice_SetProperty DIPROP_AXISMODE [%i]", hr);
        return FALSE;
    }

    hr = IDirectInputDevice8_EnumObjects(joystick->device, _enum_objects_callback, joystick, DIDFT_ALL);
    if (hr < 0) {
        ERROR_LOG("IDirectInputDevice8_EnumObjects [%i]", hr);
        return FALSE;
    }

    DEBUG_LOG("povs: %d; axes: %d", joystick->povCount, joystick->axisCount);
    if(joystick->axisCount < 1 && joystick->povCount < 1) {
        return FALSE;
    }

    joystick->deadzone = 0.15;
    joystick->threshold = 0.5;
    return TRUE;
}

static BOOL CALLBACK _enum_joysticks_callback(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext) {
    unsigned idx = g_joystickCount;
    if (!g_di8 || idx >= MAX_GAMEPADS) {
        return DIENUM_STOP;
    }

    DEBUG_LOG("#%u", idx);
    DEBUG_LOG("instance: " GUID_FMT_STR, GUID_FMT_ARG(pdidInstance->guidProduct));
    DEBUG_LOG("product: " GUID_FMT_STR, GUID_FMT_ARG(pdidInstance->guidInstance));

    if (memcmp(&pdidInstance->guidProduct.Data4[2], "PIDVID", 6) != 0) {
        ERROR_LOG("guidProduct not recognized [" GUID_FMT_STR "]", GUID_FMT_ARG(pdidInstance->guidProduct));
        return DIENUM_CONTINUE;
    }

    DEBUG_LOG("pidvid: 0x%08x", pdidInstance->guidProduct.Data1);

    Joystick* joystick = &g_joysticks[idx];
    ZeroMemory(joystick, sizeof(Joystick));

    for (size_t i = 0; i < g_mappings_count; i++) {
        if (pdidInstance->guidProduct.Data1 == g_mappings[i].pidvid) {
            joystick->mapping = &g_mappings[i];
            break;
        }
    }

    if (joystick->mapping == NULL) {
        ERROR_LOG("no mapping found for pidvid: [0x%08x]", pdidInstance->guidProduct.Data1);
        return DIENUM_CONTINUE;
    }

    DEBUG_LOG("mapping: '%s'", joystick->mapping->name);

    HRESULT hr = IDirectInput8_CreateDevice(g_di8, &pdidInstance->guidInstance, &joystick->device, NULL);
    if (hr < 0) {
        ERROR_LOG("IDirectInput8_CreateDevice [%i]", hr);
        return DIENUM_CONTINUE;
    }

    if (!_init_joystick(joystick)) {
        IDirectInputDevice8_Release(joystick->device);
        joystick->device = NULL;
        return DIENUM_CONTINUE;
    }

    DEBUG_LOG("OK");
    g_joystickCount++;
    return DIENUM_CONTINUE;
}

static void _cleanup_joysticks(void) {
    for (int i = 0; i < g_joystickCount; i++) {
        if (g_joysticks[i].device) {
            IDirectInputDevice8_Release(g_joysticks[i].device);
            g_joysticks[i].device = NULL;
        }
    }

    g_joystickCount = 0;
}

static BOOL _refresh_joysticks(void) {
    DEBUG_LOG("");
    _cleanup_joysticks();

    if (g_di8) {
        HRESULT hr = IDirectInput8_EnumDevices(g_di8, DI8DEVCLASS_GAMECTRL, _enum_joysticks_callback, NULL, DIEDFL_ATTACHEDONLY);

        if (hr < 0) {
            ERROR_LOG("IDirectInput8_EnumDevices [%i]", hr);
            return FALSE;
        }
    }

    return TRUE;
}

static void _poll_joystick(Joystick* joystick) {
    if (!joystick->device) {
        return;
    }

    HRESULT hr = IDirectInputDevice8_Poll(joystick->device);
    if (hr == DIERR_NOTACQUIRED || hr == DIERR_INPUTLOST) {
        IDirectInputDevice8_Acquire(joystick->device);
        hr = IDirectInputDevice8_Poll(joystick->device);
    }

    if(hr < 0) {
        DEBUG_LOG("IDirectInputDevice8_Poll [%i]", hr);
        return;
    }

    memcpy(&joystick->prev, &joystick->current, sizeof(DIJOYSTATE));

    hr = IDirectInputDevice8_GetDeviceState(joystick->device, sizeof(DIJOYSTATE), &joystick->current);
    if (hr < 0) {
        ERROR_LOG("IDirectInputDevice8_GetDeviceState [%i]", hr);
        return;
    }
}

static void _init_window(HWND hwnd) {
    DEBUG_LOG("hwnd: %p", hwnd);

    if (!hwnd) {
        ERROR_LOG("hwnd not provided");
        return;
    }

    g_hWnd = hwnd;
    g_wndProc = (WNDPROC)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)_window_proc);

    if (!g_wndProc) {
        ERROR_LOG("SetWindowLongPtr [%i]", GetLastError());
        return;
    }

    DEV_BROADCAST_DEVICEINTERFACE notificationFilter;
    ZeroMemory(&notificationFilter, sizeof(notificationFilter));

    notificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    notificationFilter.dbcc_size = sizeof(notificationFilter);

    g_hDevNotify = RegisterDeviceNotification(g_hWnd, &notificationFilter, DEVICE_NOTIFY_WINDOW_HANDLE | DEVICE_NOTIFY_ALL_INTERFACE_CLASSES);

    if (!g_hDevNotify) {
        ERROR_LOG("RegisterDeviceNotification [%i]", GetLastError());
        return;
    }
}

static void _shutdown_window(void) {
    DEBUG_LOG("");

    if (g_hDevNotify) {
        UnregisterDeviceNotification(g_hDevNotify);
        g_hDevNotify = NULL;
    }
}

static void _init_dinput8(void) {
    DEBUG_LOG("");

    HRESULT hr;
    if (!g_di8) {
        hr = DirectInput8Create(g_hInstance, DIRECTINPUT_VERSION, &IID_IDirectInput8, (VOID**)&g_di8, NULL);
        if (hr < 0) {
            g_di8 = NULL;
            ERROR_LOG("DirectInput8Create [%ld]", hr);
            return;
        }

        g_joystickRefresh = TRUE;
    }
}

static void _shutdown_dinput8(void) {
    _cleanup_joysticks();

    if (g_di8) {
        IDirectInput8_Release(g_di8);
        g_di8 = NULL;
    }
}

static HRESULT WINAPI _swapchain_present_callback(IDXGISwapChain *This, UINT sync_interval, UINT flags) {
    if(!g_swapchainPresentFn) {
        return DXGI_ERROR_DEVICE_REMOVED;
    }

    HRESULT hr = g_swapchainPresentFn(This, sync_interval, flags);
    if(hr == S_OK) {
        gmdinput_poll(0.0);
    }

    return hr;
}

static void _init_dxgi_swapchain_hook(IDXGISwapChain* swapchain) {
    if (!swapchain) {
        ERROR_LOG("swapchain not provided");
        return;
    }

    DEBUG_LOG("swapchain: %p", swapchain);
    DEBUG_LOG("target: %p", swapchain->lpVtbl->Present);

    MH_STATUS rc = MH_Initialize();
	if (rc != MH_OK && rc != MH_ERROR_ALREADY_INITIALIZED) {
        ERROR_LOG("MH_Initialize [%d]\n", rc);
        return;
    }

    if(g_swapchainPresentFn != NULL) {
        return;
    }

    rc = MH_CreateHook((DWORD_PTR*)swapchain->lpVtbl->Present, _swapchain_present_callback, (void**)(&g_swapchainPresentFn));
    if (rc != MH_OK) {
        g_swapchainPresentFn = NULL;
        ERROR_LOG("MH_CreateHook [%d]\n", rc);
        return;
    }

    rc = MH_EnableHook((DWORD_PTR*)swapchain->lpVtbl->Present);
    if (rc != MH_OK) {
        ERROR_LOG("MH_EnableHook [%d]\n", rc);
        return;
    }
}

static void _cleanup_dxgi_swapchain_hook() {
	if (MH_DisableHook(MH_ALL_HOOKS) != MH_OK) {
        return;
    };

	if (MH_Uninitialize() != MH_OK) {
        return;
    }
}

double gmdinput_init(void* hwnd, void* video_d3d11_swapchain) {
    DEBUG_LOG("mappings: %lld\n", g_mappings_count);
    g_hInstance = GetModuleHandle(NULL);

    _init_window(hwnd);
    _init_dinput8();
    _init_dxgi_swapchain_hook(video_d3d11_swapchain);

    return 1.0;
}

double gmdinput_shutdown(void) {
    DEBUG_LOG("mappings: %lld\n", g_mappings_count);
    DEBUG_LOG("mappings: %lld\n", g_mappings_count);
    DEBUG_LOG("mappings: %lld\n", g_mappings_count);
    DEBUG_LOG("mappings: %lld\n", g_mappings_count);
    DEBUG_LOG("mappings: %lld\n", g_mappings_count);
    DEBUG_LOG("mappings: %lld\n", g_mappings_count);
    DEBUG_LOG("mappings: %lld\n", g_mappings_count);
    DEBUG_LOG("mappings: %lld\n", g_mappings_count);
    DEBUG_LOG("mappings: %lld\n", g_mappings_count);
    DEBUG_LOG("mappings: %lld\n", g_mappings_count);
    DEBUG_LOG("mappings: %lld\n", g_mappings_count);
    DEBUG_LOG("mappings: %lld\n", g_mappings_count);
    DEBUG_LOG("mappings: %lld\n", g_mappings_count);
    DEBUG_LOG("mappings: %lld\n", g_mappings_count);
    DEBUG_LOG("mappings: %lld\n", g_mappings_count);
    DEBUG_LOG("mappings: %lld\n", g_mappings_count);
    DEBUG_LOG("mappings: %lld\n", g_mappings_count);
    DEBUG_LOG("mappings: %lld\n", g_mappings_count);
    DEBUG_LOG("mappings: %lld\n", g_mappings_count);
    DEBUG_LOG("mappings: %lld\n", g_mappings_count);
    DEBUG_LOG("mappings: %lld\n", g_mappings_count);
    DEBUG_LOG("mappings: %lld\n", g_mappings_count);
    DEBUG_LOG("mappings: %lld\n", g_mappings_count);
    DEBUG_LOG("mappings: %lld\n", g_mappings_count);
    DEBUG_LOG("mappings: %lld\n", g_mappings_count);
    DEBUG_LOG("mappings: %lld\n", g_mappings_count);
    DEBUG_LOG("mappings: %lld\n", g_mappings_count);
    DEBUG_LOG("mappings: %lld\n", g_mappings_count);
    DEBUG_LOG("mappings: %lld\n", g_mappings_count);
    DEBUG_LOG("mappings: %lld\n", g_mappings_count);
    DEBUG_LOG("mappings: %lld\n", g_mappings_count);
    DEBUG_LOG("mappings: %lld\n", g_mappings_count);
    DEBUG_LOG("mappings: %lld\n", g_mappings_count);
    DEBUG_LOG("mappings: %lld\n", g_mappings_count);
    DEBUG_LOG("mappings: %lld\n", g_mappings_count);
    DEBUG_LOG("mappings: %lld\n", g_mappings_count);
    DEBUG_LOG("mappings: %lld\n", g_mappings_count);
    DEBUG_LOG("mappings: %lld\n", g_mappings_count);
    DEBUG_LOG("mappings: %lld\n", g_mappings_count);
    DEBUG_LOG("mappings: %lld\n", g_mappings_count);
    DEBUG_LOG("mappings: %lld\n", g_mappings_count);
    DEBUG_LOG("mappings: %lld\n", g_mappings_count);
    DEBUG_LOG("mappings: %lld\n", g_mappings_count);
    DEBUG_LOG("mappings: %lld\n", g_mappings_count);
    DEBUG_LOG("mappings: %lld\n", g_mappings_count);
    DEBUG_LOG("mappings: %lld\n", g_mappings_count);
    DEBUG_LOG("mappings: %lld\n", g_mappings_count);
    DEBUG_LOG("mappings: %lld\n", g_mappings_count);
    DEBUG_LOG("mappings: %lld\n", g_mappings_count);
    DEBUG_LOG("mappings: %lld\n", g_mappings_count);
    DEBUG_LOG("mappings: %lld\n", g_mappings_count);
    DEBUG_LOG("mappings: %lld\n", g_mappings_count);
    DEBUG_LOG("mappings: %lld\n", g_mappings_count);
    DEBUG_LOG("mappings: %lld\n", g_mappings_count);
    DEBUG_LOG("mappings: %lld\n", g_mappings_count);
    DEBUG_LOG("mappings: %lld\n", g_mappings_count);
    DEBUG_LOG("mappings: %lld\n", g_mappings_count);
    DEBUG_LOG("mappings: %lld\n", g_mappings_count);
    DEBUG_LOG("mappings: %lld\n", g_mappings_count);
    DEBUG_LOG("mappings: %lld\n", g_mappings_count);
    DEBUG_LOG("mappings: %lld\n", g_mappings_count);
    _cleanup_dxgi_swapchain_hook();
    _shutdown_dinput8();
    _shutdown_window();

    return 1.0;
}

double gmdinput_poll(double refresh) {
    if ((g_joystickRefresh || refresh)) {
        g_joystickRefresh = FALSE;
        _refresh_joysticks();
    }

    for (int i = 0; i < g_joystickCount; i++) {
        _poll_joystick(&g_joysticks[i]);
    }

    return 1.0;
}

double gmdinput_gamepad_is_supported(void) {
    return 42.0;
}

double gmdinput_gamepad_is_connected(double device) {
    int d = (int)device;

    return (d >= 0 && d < g_joystickCount);
}

const char* gmdinput_gamepad_get_guid(double device) {
    int d = (int)device;

    if (d < 0 || d >= g_joystickCount) {
        return "";
    }

    return g_joysticks[d].mapping->guid;
}

double gmdinput_gamepad_get_device_count(void) {
    return g_joystickCount;
}

const char* gmdinput_gamepad_get_description(double device) {
    int d = (int)device;

    if (d < 0 || d >= g_joystickCount) {
        return "";
    }

    return g_joysticks[d].mapping->name;
}

double gmdinput_gamepad_get_button_threshold(double device) {
    int d = (int)device;

    if (d < 0 || d >= g_joystickCount) {
        return 0.0;
    }

    return g_joysticks[d].threshold;
}

double gmdinput_gamepad_get_axis_deadzone(double device) {
    int d = (int)device;

    if (d < 0 || d >= g_joystickCount) {
        return 0.0;
    }

    return g_joysticks[d].deadzone;
}

double gmdinput_gamepad_get_option(double device, const char* option) {
    return 0.0;
}

double gmdinput_gamepad_set_button_threshold(double device, double threshold) {
    int d = (int)device;

    if (d < 0 || d >= g_joystickCount) {
        return 0.0;
    }

    return g_joysticks[d].threshold = _clamp(threshold, 0.0, 1.0);
}

double gmdinput_gamepad_set_axis_deadzone(double device, double deadzone) {
    int d = (int)device;

    if (d < 0 || d >= g_joystickCount) {
        return 0.0;
    }

    return g_joysticks[d].deadzone = _clamp(deadzone, 0.0, 1.0);
}

double gmdinput_gamepad_axis_count(double device) {
    int d = (int)device;

    if (d < 0 || d >= g_joystickCount) {
        return 0.0;
    }

    return g_joysticks[d].axisCount;
}

double gmdinput_gamepad_axis_value(double device, double axisIndex) {
    int d = (int)device;
    int i = (int)axisIndex;

    if (d < 0 || d >= g_joystickCount) {
        return 0.0;
    }

    const Joystick* joy = &g_joysticks[d];
    return _gm_axis(&joy->current, joy, i);
}

double gmdinput_gamepad_button_check(double device, double buttonIndex) {
    int d = (int)device;
    int i = (int)buttonIndex;

    if (d < 0 || d >= g_joystickCount) {
        return 0.0;
    }

    const Joystick* joy = &g_joysticks[d];
    return _gm_button_check(joy, &joy->current, i);
}

double gmdinput_gamepad_button_check_pressed(double device, double buttonIndex) {
    int d = (int)device;
    int i = (int)buttonIndex;

    if (d < 0 || d >= g_joystickCount) {
        return 0.0;
    }

    const Joystick* joy = &g_joysticks[d];
    return _gm_button_check(joy, &joy->current, i) && !_gm_button_check(joy, &joy->prev, i);
}

double gmdinput_gamepad_button_check_released(double device, double buttonIndex) {
    int d = (int)device;
    int i = (int)buttonIndex;

    if (d < 0 || d >= g_joystickCount) {
        return 0.0;
    }

    const Joystick* joy = &g_joysticks[d];
    return !_gm_button_check(joy, &joy->current, i) && _gm_button_check(joy, &joy->prev, i);
}

double gmdinput_gamepad_button_count(double device) {
    int d = (int)device;

    if (d < 0 || d >= g_joystickCount) {
        return 0.0;
    }

    return g_joysticks[d].buttonCount;
}

double gmdinput_gamepad_button_value(double device, double buttonIndex) {
    int d = (int)device;
    int i = (int)buttonIndex;

    if (d < 0 || d >= g_joystickCount) {
        return 0.0;
    }

    const Joystick* joy = &g_joysticks[d];
    return _gm_button(joy, &joy->current, i);
}

double gmdinput_gamepad_hat_count(double device) {
    int d = (int)device;

    if (d < 0 || d >= g_joystickCount) {
        return 0.0;
    }

    return g_joysticks[d].povCount;
}

double gmdinput_gamepad_hat_value(double device, double hatIndex) {
    int d = (int)device;
    int i = (int)hatIndex;

    if (d < 0 || d >= g_joystickCount || i < 0 || i > UINT8_MAX) {
        return 0.0;
    }

    return _driver_hat(&g_joysticks[d].current, (uint8_t)i);
}

double gmdinput_gamepad_set_vibration(double device, double left, double right) {
    return 0.0;
}

double gmdinput_gamepad_set_colour(double device, double color) {
    return 0.0;
}

double gmdinput_gamepad_set_option(double device, const char* option, double value) {
    return 0.0;
}

const char* gmdinput_gamepad_get_mapping(double device) {
    return "";
}

double gmdinput_gamepad_test_mapping(double device, const char* mapping) {
    return 0.0;
}

double gmdinput_gamepad_remove_mapping(double device) {
    return 0.0;
}
