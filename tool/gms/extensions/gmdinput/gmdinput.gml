#define gmdinput_init_auto
var osinfo = os_get_info();
gmdinput_init(window_handle(), osinfo[? "video_d3d11_swapchain"]);

#define gmdinput_shutdown_auto
gmdinput_shutdown();
