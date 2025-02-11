draw_set_font(FontMono);
draw_set_color(c_white);
for(var i = 0; i < ds_list_size(m_txt_lines); i++) {
	 draw_text(8, 8 + 18 * i, m_txt_lines[|i]);
}
