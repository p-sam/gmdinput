function _gamepad_change_value(use_gmdinput, k, deviceIndex, offset) {
	if(offset == 0.0) {
		return;
	}
	var v = _gamepad_fn(use_gmdinput, "get_"+k, deviceIndex);
	v += offset;
	return _gamepad_fn(use_gmdinput, "set_"+k, deviceIndex, v);
}