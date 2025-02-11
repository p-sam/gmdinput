function _gamepad_fn(use_gmdinput, fn) {
	var i = 2;

	if(use_gmdinput) {
		switch(fn) {
		case "is_supported":
			return gmdinput_gamepad_is_supported();
		case "is_connected":
			return gmdinput_gamepad_is_connected(argument[i]);
		case "get_guid":
			return gmdinput_gamepad_get_guid(argument[i]);
		case "get_device_count":
			return gmdinput_gamepad_get_device_count();
		case "get_description":
			return gmdinput_gamepad_get_description(argument[i]);
		case "get_button_threshold":
			return gmdinput_gamepad_get_button_threshold(argument[i]);
		case "get_axis_deadzone":
			return gmdinput_gamepad_get_axis_deadzone(argument[i]);
		case "set_button_threshold":
			return gmdinput_gamepad_set_button_threshold(argument[i], argument[i+1]);
		case "set_axis_deadzone":
			return gmdinput_gamepad_set_axis_deadzone(argument[i], argument[i+1]);
		case "set_vibration":
			return gmdinput_gamepad_set_vibration(argument[i], argument[i+1], argument[i+2]);
		case "set_colour":
			return gmdinput_gamepad_set_colour(argument[i], argument[i+1]);
		case "axis_count":
			return gmdinput_gamepad_axis_count(argument[i]);
		case "axis_value":
			return gmdinput_gamepad_axis_value(argument[i], argument[i+1]);
		case "button_check":
			return gmdinput_gamepad_button_check(argument[i], argument[i+1]);
		case "button_check_pressed":
			return gmdinput_gamepad_button_check_pressed(argument[i], argument[i+1]);
		case "button_check_released":
			return gmdinput_gamepad_button_check_released(argument[i], argument[i+1]);
		case "button_count":
			return gmdinput_gamepad_button_count(argument[i]);
		case "button_value":
			return gmdinput_gamepad_button_value(argument[i], argument[i+1]);
		case "hat_count":
			return gmdinput_gamepad_hat_count(argument[i]);
		case "hat_value":
			return gmdinput_gamepad_hat_value(argument[i], argument[i+1]);
		default:
			return undefined;
		}	
	}

	switch(fn) {
	case "is_supported":
	    return gamepad_is_supported();
	case "is_connected":
	    return gamepad_is_connected(argument[i]);
	case "get_guid":
	    return gamepad_get_guid(argument[i]);
	case "get_device_count":
	    return gamepad_get_device_count();
	case "get_description":
	    return gamepad_get_description(argument[i]);
	case "get_button_threshold":
	    return gamepad_get_button_threshold(argument[i]);
	case "get_axis_deadzone":
	    return gamepad_get_axis_deadzone(argument[i]);
	case "set_button_threshold":
	    return gamepad_set_button_threshold(argument[i], argument[i+1]);
	case "set_axis_deadzone":
	    return gamepad_set_axis_deadzone(argument[i], argument[i+1]);
	case "set_vibration":
	    return gamepad_set_vibration(argument[i], argument[i+1], argument[i+2]);
	case "set_colour":
	    return gamepad_set_colour(argument[i], argument[i+1]);
	case "axis_count":
	    return gamepad_axis_count(argument[i]);
	case "axis_value":
	    return gamepad_axis_value(argument[i], argument[i+1]);
	case "button_check":
	    return gamepad_button_check(argument[i], argument[i+1]);
	case "button_check_pressed":
	    return gamepad_button_check_pressed(argument[i], argument[i+1]);
	case "button_check_released":
	    return gamepad_button_check_released(argument[i], argument[i+1]);
	case "button_count":
	    return gamepad_button_count(argument[i]);
	case "button_value":
	    return gamepad_button_value(argument[i], argument[i+1]);
	case "hat_count":
	    return gamepad_hat_count(argument[i]);
	case "hat_value":
	    return gamepad_hat_value(argument[i], argument[i+1]);
	default:
	    return undefined;
	}
}