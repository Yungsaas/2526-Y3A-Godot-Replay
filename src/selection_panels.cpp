#pragma once

#include "selection_panels.hpp"
#include "godot_cpp/classes/node.hpp"
#include "godot_cpp/classes/node2d.hpp"
#include "godot_cpp/classes/node3d.hpp"
#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/core/print_string.hpp"
#include "godot_cpp/variant/string.hpp"
#include <godot_cpp/core/object.hpp>
#include <algorithm>
#include <tuple>


bool Selection_Panels::add_input_screen(godot::TextEdit* node)
{
    input_screen = node;
    return true;
}

bool Selection_Panels::add_position_screen(godot::TextEdit* node)
{
    position_screen = node;
    return true;
}

void Selection_Panels::print_input() {

    input_screen->set_text("Input text!");

}

void Selection_Panels::print_positions() {
    position_screen->set_text("Position text!");

}

void Selection_Panels::set_replay_ptr(Temp_save_replay* recorder)
{
    replay_ptr = recorder;

}

void Selection_Panels::_bind_methods()
{
    godot::ClassDB::bind_method(godot::D_METHOD("add_input_screen", "node"), &Selection_Panels::add_input_screen);
    godot::ClassDB::bind_method(godot::D_METHOD("add_position_screen", "node"), &Selection_Panels::add_position_screen);
    godot::ClassDB::bind_method(godot::D_METHOD("print_input"), &Selection_Panels::print_input);
    godot::ClassDB::bind_method(godot::D_METHOD("print_positions"), &Selection_Panels::print_positions);

    godot::ClassDB::bind_method(godot::D_METHOD("set_replay_ptr", "recorder"), &Selection_Panels::set_replay_ptr);

}