#pragma once

#include "temp_save_replay.hpp"
#include "godot_cpp/classes/node.hpp"
#include "godot_cpp/classes/node2d.hpp"
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

void Temp_save_replay::debug_print_positions()
{
    godot::print_line("Printing recorded positions: ");
    for(int currentFrame = 0; currentFrame < recording_frame; currentFrame++)
    {
        // Print all data for each frame
        auto range2d = temporary_data_map_2d_pos.equal_range(currentFrame);
        auto range3d = temporary_data_map_3d_pos.equal_range(currentFrame);

                godot::print_line("\nFrame: " + godot::String::num(currentFrame) + "\n2D positions:");
        for(auto it = range2d.first; it !=range2d.second; it++)
        {
            auto node = std::get<0>(it->second);
            auto position = std::get<1>(it->second);
            godot::print_line("Node: " + node->get_name() + 
            " Position: \nX: " + godot::String::num(position.get_origin().x) 
            + "\nY: " + godot::String::num(position.get_origin().y));
        }

        godot::print_line("\n3D positions:");
        for(auto it = range3d.first; it !=range3d.second; it++)
        {
            auto node = std::get<0>(it->second);
            auto position = std::get<1>(it->second);
            godot::print_line("Node: " + node->get_name() + 
            " Position: \nX: " + godot::String::num(position.get_origin().x) 
            + "\nY: " + godot::String::num(position.get_origin().y)
            + "\nZ: " + godot::String::num(position.get_origin().z));
        }
    }
}

void Temp_save_replay::start_recording()
{
    is_recording = true;
    recording_frame = 0;
    temporary_data_map_3d_pos.clear();
    temporary_data_map_2d_pos.clear();
}

void Temp_save_replay::stop_recording()
{
    is_recording = false;
    recording_frame = 0;

    // TODO: write to json here
}



void Temp_save_replay::update()
{
    if(!is_recording) return; // return if not recording

    for(auto node : tracked_nodes)
    {
        auto node3d = godot::Object::cast_to<godot::Node3D>(node);
        auto node2d = godot::Object::cast_to<godot::Node2D>(node);
        if(node3d)
        {
            temporary_data_map_3d_pos.emplace(recording_frame, std::make_tuple(node, node3d->get_transform()));
        }
        if(node2d)
        {
            temporary_data_map_2d_pos.emplace(recording_frame, std::make_tuple(node, node2d->get_transform()));
        }
    }

    recording_frame++;
}

void Temp_save_replay::_bind_methods()
{
    godot::ClassDB::bind_method(godot::D_METHOD("debug_print_array"), &Temp_save_replay::debug_print_array);
    godot::ClassDB::bind_method(godot::D_METHOD("debug_print_positions"), &Temp_save_replay::debug_print_positions);
    godot::ClassDB::bind_method(godot::D_METHOD("add_node", "node"), &Temp_save_replay::add_node);
    godot::ClassDB::bind_method(godot::D_METHOD("start_recording"), &Temp_save_replay::start_recording);
    godot::ClassDB::bind_method(godot::D_METHOD("stop_recording"), &Temp_save_replay::stop_recording);
    godot::ClassDB::bind_method(godot::D_METHOD("update"), &Temp_save_replay::update);

}