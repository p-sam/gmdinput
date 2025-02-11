if(keyboard_check_pressed(vk_space) && !m_replaced) {
	m_use_gmdinput = !m_use_gmdinput;
}
if(keyboard_check_pressed(vk_left)) {
	m_device_index--;
}
if(keyboard_check_pressed(vk_right)) {
	m_device_index++;
}
_gamepad_change_value(m_use_gmdinput, "axis_deadzone", m_device_index, (-keyboard_check_pressed(ord("J"))+keyboard_check_pressed(ord("K")))*0.05);
_gamepad_change_value(m_use_gmdinput, "button_threshold", m_device_index, (-keyboard_check_pressed(ord("U"))+keyboard_check_pressed(ord("I")))*0.05);

if(_gamepad_fn(m_use_gmdinput, "button_check_pressed", m_device_index, gp_stickr)) {
	show_message("stick r pressed");	
}

ds_list_clear(m_txt_lines);

if(m_replaced) {
	ds_list_add(m_txt_lines, "UNPREFIXED MODE")
	ds_list_add(m_txt_lines, "");
} else {
	ds_list_add(m_txt_lines, "use_gmdinput: "+string(m_use_gmdinput));
	ds_list_add(m_txt_lines, "left/right = device | j/k = deadzone | u/i = threshold");
}
	
ds_list_add(m_txt_lines, "");

var deviceCount = _gamepad_fn(m_use_gmdinput, "get_device_count");
var connected =  _gamepad_fn(m_use_gmdinput, "is_connected", m_device_index);

ds_list_add(m_txt_lines, "index: "+string(m_device_index)+" | count: "+string(deviceCount)+" | connected: "+string(connected));
ds_list_add(m_txt_lines, "----------------------------------------------------");

var guid = _gamepad_fn(m_use_gmdinput, "get_guid", m_device_index);
ds_list_add(m_txt_lines, "guid: "+string(guid));
var description = _gamepad_fn(m_use_gmdinput, "get_description", m_device_index);
ds_list_add(m_txt_lines, "description: "+string(description));

var deadzone = _gamepad_fn(m_use_gmdinput, "get_axis_deadzone", m_device_index);
var threshold = _gamepad_fn(m_use_gmdinput, "get_button_threshold", m_device_index);
ds_list_add(m_txt_lines, "deadzone:"+string_format(deadzone, 2, 2)+" | threshold:"+string_format(threshold, 2, 2));
ds_list_add(m_txt_lines, "");

ds_list_add(m_txt_lines, "axes:");

var line = "";
for(var i = 0; i < array_length(m_axis_layout); i++) {
	if(m_axis_layout[i].cr) {
		ds_list_add(m_txt_lines, line);
		line = "";
	}
	var axisValue = _gamepad_fn(m_use_gmdinput, "axis_value", m_device_index, m_axis_layout[i].idx);
	line += "@";
	line += m_axis_layout[i].str;
	line += ":";
	line += string_format(axisValue, 2, 2);
	line += "   ";
}
ds_list_add(m_txt_lines, line);
ds_list_add(m_txt_lines, "");

ds_list_add(m_txt_lines, "buttons:");

line = "";
for(var i = 0; i < array_length(m_btn_layout); i++) {
	if(m_btn_layout[i].cr) {
		ds_list_add(m_txt_lines, line);
		line = "";
	}
	var btnCheck = _gamepad_fn(m_use_gmdinput, "button_check", m_device_index, m_btn_layout[i].idx);
	var btnPressed = _gamepad_fn(m_use_gmdinput, "button_check_pressed", m_device_index, m_btn_layout[i].idx);
	var btnReleased = _gamepad_fn(m_use_gmdinput, "button_check_released", m_device_index, m_btn_layout[i].idx);
	var btnValue = _gamepad_fn(m_use_gmdinput, "button_value", m_device_index, m_btn_layout[i].idx);
	line += "@";
	line += m_btn_layout[i].str;
	line += ": ";
	line += btnCheck ? "C" : "-";
	line += btnPressed ? "P" : "-";
	line += btnReleased ? "R" : "-";
	line += string_format(btnValue, 1, 2);
	line += "   ";
}
ds_list_add(m_txt_lines, line);
ds_list_add(m_txt_lines, "");

var rawAxesCount = _gamepad_fn(m_use_gmdinput, "axis_count", m_device_index);
ds_list_add(m_txt_lines, "raw axes: "+string(rawAxesCount));

line = "";
for(var i = 0; i < rawAxesCount; i++) {
	if((i % 8) == 0) {
		ds_list_add(m_txt_lines, line);
		line = "";
	}
	var axisValue = _gamepad_fn(m_use_gmdinput, "axis_value", m_device_index, i);
	line += "#";
	line += string_replace(string_format(i, 2, 0), " ", "0");
	line += ":";
	line += string_format(axisValue, 2, 2);
	line += "   ";
}
ds_list_add(m_txt_lines, line);
ds_list_add(m_txt_lines, "");

var rawButtonCount = _gamepad_fn(m_use_gmdinput, "button_count", m_device_index);
ds_list_add(m_txt_lines, "raw buttons: "+string(rawButtonCount));

line = "";
for(var i = 0; i < rawButtonCount; i++) {
	if((i % 16) == 0) {
		ds_list_add(m_txt_lines, line);
		line = "";
	}
	var buttonValue = _gamepad_fn(m_use_gmdinput, "button_value", m_device_index, i);
	line += string_format(buttonValue, 2, 2);
}
ds_list_add(m_txt_lines, line);
ds_list_add(m_txt_lines, "");

var rawHatCount = _gamepad_fn(m_use_gmdinput, "hat_count", m_device_index);
ds_list_add(m_txt_lines, "raw hats: "+string(rawHatCount));
ds_list_add(m_txt_lines, "");

line = "";
for(var i = 0; i < rawHatCount; i++) {
	var hatValue = _gamepad_fn(m_use_gmdinput, "hat_value", m_device_index, i);
	line += "#";
	line += string(i);
	line += ": ";
	for(var n = 0; n < 4; n++) {
		line += string((hatValue & (1 << n)) > 0);
	}
	line += "   ";
}
ds_list_add(m_txt_lines, line)