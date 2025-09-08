#pragma once

#include "temp_save_replay.hpp"
#include "godot_cpp/classes/node.hpp"
#include "godot_cpp/classes/node3d.hpp"
#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/core/print_string.hpp"
#include "godot_cpp/variant/string.hpp"
#include <algorithm>
#include <tuple>

bool Temp_save_replay::add_node(godot::Node* node)
{
    if(std::find(tracked_nodes.begin(), tracked_nodes.end(), node) != tracked_nodes.end()) return false;

    tracked_nodes.push_back(node);
    return true;
}

void Temp_save_replay::debug_print_array()
{
    for (auto nodeIn : tracked_nodes) {
    godot::print_line(nodeIn);
    }
}

void Temp_save_replay::start_recording()
{
    is_recording = true;
    recording_frame = 0;
    temporary_data_map.clear();
}

void Temp_save_replay::stop_recording()
{
    is_recording = false;
    recording_frame = 0;

    // Maybe serialize into json here if permanent recording is wanted
}

void Temp_save_replay::update()
{
    if(!is_recording) return; // return if not recording

    for(auto node : tracked_nodes)
    {
        auto node3d = godot::Object::cast_to<godot::Node3D>(node);
        if(node3d)
        {
            temporary_data_map.emplace(recording_frame, std::make_tuple(node, node3d->get_transform()));
        }
    }

    recording_frame++;
}

void Temp_save_replay::_bind_methods()
{
    godot::ClassDB::bind_method(godot::D_METHOD("debug_print_array"), &Temp_save_replay::debug_print_array);
    godot::ClassDB::bind_method(godot::D_METHOD("add_node", "node"), &Temp_save_replay::add_node);
    godot::ClassDB::bind_method(godot::D_METHOD("start_recording"), &Temp_save_replay::start_recording);
    godot::ClassDB::bind_method(godot::D_METHOD("stop_recording"), &Temp_save_replay::stop_recording);
    godot::ClassDB::bind_method(godot::D_METHOD("update"), &Temp_save_replay::update);

}