m_replaced = _gamepad_is_replaced();
m_use_gmdinput = !m_replaced;
m_device_index = 0;

m_txt_lines = ds_list_create();

m_btn_layout = [];
array_push(m_btn_layout, {str: "face1     ", cr: 1, idx: gp_face1});
array_push(m_btn_layout, {str: "face2     ", cr: 0, idx: gp_face2});
array_push(m_btn_layout, {str: "face3     ", cr: 0, idx: gp_face3});
array_push(m_btn_layout, {str: "face4     ", cr: 0, idx: gp_face4});
array_push(m_btn_layout, {str: "shoulderl ", cr: 1, idx: gp_shoulderl});
array_push(m_btn_layout, {str: "shoulderr ", cr: 0, idx: gp_shoulderr});
array_push(m_btn_layout, {str: "shoulderlb", cr: 0, idx: gp_shoulderlb});
array_push(m_btn_layout, {str: "shoulderrb", cr: 0, idx: gp_shoulderrb});
array_push(m_btn_layout, {str: "select    ", cr: 1, idx: gp_select});
array_push(m_btn_layout, {str: "start     ", cr: 0, idx: gp_start});
array_push(m_btn_layout, {str: "stickl    ", cr: 0, idx: gp_stickl});
array_push(m_btn_layout, {str: "stickr    ", cr: 0, idx: gp_stickr});
array_push(m_btn_layout, {str: "padu      ", cr: 1, idx: gp_padu});
array_push(m_btn_layout, {str: "padd      ", cr: 0, idx: gp_padd});
array_push(m_btn_layout, {str: "padl      ", cr: 0, idx: gp_padl});
array_push(m_btn_layout, {str: "padr      ", cr: 0, idx: gp_padr});

m_axis_layout = [];
array_push(m_axis_layout, {str: "axislh", cr: 1, idx: gp_axislh});
array_push(m_axis_layout, {str: "axislv", cr: 0, idx: gp_axislv});
array_push(m_axis_layout, {str: "axisrh", cr: 0, idx: gp_axisrh});
array_push(m_axis_layout, {str: "axisrv", cr: 0, idx: gp_axisrv});
array_push(m_axis_layout, {str: "accelx", cr: 1, idx: gp_axis_acceleration_x});
array_push(m_axis_layout, {str: "accely", cr: 0, idx: gp_axis_acceleration_y});
array_push(m_axis_layout, {str: "accelz", cr: 0, idx: gp_axis_acceleration_z});
array_push(m_axis_layout, {str: "gyrox ", cr: 1, idx: gp_axis_angular_velocity_x});
array_push(m_axis_layout, {str: "gyroy ", cr: 0, idx: gp_axis_angular_velocity_y});
array_push(m_axis_layout, {str: "gyroz ", cr: 0, idx: gp_axis_angular_velocity_z});
