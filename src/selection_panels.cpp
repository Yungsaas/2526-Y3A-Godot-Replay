#pragma once

#include "selection_panels.hpp"
#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/variant/string.hpp"
#include <godot_cpp/classes/input_event_key.hpp>
#include <godot_cpp/core/object.hpp>

bool Data_Panels::add_input_screen(godot::TextEdit *node) {
	input_screen = node;
	return true;
}

bool Data_Panels::add_position_screen(godot::TextEdit *node) {
	position_screen = node;
	return true;
}

void Data_Panels::print_input() {
	input_screen->set_text("Input text!");
}

void Data_Panels::print_positions() {
	position_screen->set_text("Position text!");
}

void Data_Panels::set_replay_ptr(Temp_save_replay *recorder) {
	replay_ptr = recorder;
}


void Data_Panels::_bind_methods() {
	godot::ClassDB::bind_method(godot::D_METHOD("add_input_screen", "node"), &Data_Panels::add_input_screen);
	godot::ClassDB::bind_method(godot::D_METHOD("add_position_screen", "node"), &Data_Panels::add_position_screen);
	godot::ClassDB::bind_method(godot::D_METHOD("print_input"), &Data_Panels::print_input);
	godot::ClassDB::bind_method(godot::D_METHOD("print_positions"), &Data_Panels::print_positions);

	godot::ClassDB::bind_method(godot::D_METHOD("set_replay_ptr", "recorder"), &Data_Panels::set_replay_ptr);
}