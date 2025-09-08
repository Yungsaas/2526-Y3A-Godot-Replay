#pragma once

#include "temp_save_replay.hpp"
#include "godot_cpp/classes/node.hpp"
#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/core/print_string.hpp"
#include "godot_cpp/variant/string.hpp"
#include "godot_cpp/variant/vector3.hpp"
#include <tuple>

bool Temp_save_replay::add_node(godot::Node* node)
{
    if(tracked_nodes.has(node)) return false;

    tracked_nodes.append(node);
    return true;
}

void Temp_save_replay::debug_print_node_array()
{
    for (auto nodeIn : tracked_nodes) {
    godot::print_line(nodeIn);
    }
}

void Temp_save_replay::debug_print_position_data()
{
    for(auto entry : temporary_saved_data)
    {
        godot::Node* node = std::get<0>(entry.first);
        int frame = std::get<1>(entry.first);

        godot::Vector3 position = entry.second;

        godot::String output = "Node: " + node->get_name() + ", Frame: " + godot::String::num(frame) + ", Position: " + position;

        godot::print_line(output);
    }
}

void Temp_save_replay::start_recording()
{
    temporary_saved_data.clear();
    recordingFrame = 0;
    recording = true;
}

void Temp_save_replay::stop_recording()
{
    recordingFrame = 0;
    recording = false;
}

void Temp_save_replay::update()
{
    if(!recording) return;

    for (auto nodeIn : tracked_nodes)
    {
        temporary_saved_data.emplace(std::make_tuple(nodeIn,recordingFrame), nodeIn.TRANSFORM3D);
    }

    recordingFrame++;
}

void Temp_save_replay::_bind_methods()
{
    godot::ClassDB::bind_method(godot::D_METHOD("debug_print_node_array"), &Temp_save_replay::debug_print_node_array);
    godot::ClassDB::bind_method(godot::D_METHOD("debug_print_position_data"), &Temp_save_replay::debug_print_position_data);
    godot::ClassDB::bind_method(godot::D_METHOD("add_node", "node"), &Temp_save_replay::add_node);
    godot::ClassDB::bind_method(godot::D_METHOD("start_recording"), &Temp_save_replay::start_recording);
    godot::ClassDB::bind_method(godot::D_METHOD("stop_recording"), &Temp_save_replay::stop_recording);
    godot::ClassDB::bind_method(godot::D_METHOD("update"), &Temp_save_replay::update);
}