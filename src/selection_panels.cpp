#pragma once

#include "selection_panel.hpp"
#include "godot_cpp/classes/node.hpp"
#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/core/print_string.hpp"
#include "godot_cpp/variant/string.hpp"
#include "godot_cpp/classes/text_edit.hpp"
#include "godot_cpp/variant/utility_functions.hpp"
#include <algorithm>
#include <tuple>

bool Selection_Panel::add_position_screen(godot::Node* node)
{
    position_screen = node;
    return true;
}

void Selection_Panel::print_positions() {

    godot::print_line("Node: " + position_screen->get_name());
}

void Selection_Panel::_bind_methods()
{
    godot::ClassDB::bind_method(godot::D_METHOD("print_positions"), &Selection_Panel::print_positions);

}