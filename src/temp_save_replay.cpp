#pragma once

#include "temp_save_replay.hpp"
#include "godot_cpp/classes/node.hpp"
#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/core/print_string.hpp"
#include "godot_cpp/variant/string.hpp"

bool Temp_save_replay::add_node(godot::Node* node)
{
    if(tracked_nodes.has(node)) return false;

    tracked_nodes.append(node);
    return true;
}

void Temp_save_replay::debug_print_array()
{
    for (auto nodeIn : tracked_nodes) {
    godot::print_line(nodeIn);
    }
}

void Temp_save_replay::_bind_methods()
{
    godot::ClassDB::bind_method(godot::D_METHOD("debug_print_array"), &Temp_save_replay::debug_print_array);
    godot::ClassDB::bind_method(godot::D_METHOD("add_node", "node"), &Temp_save_replay::add_node);
}