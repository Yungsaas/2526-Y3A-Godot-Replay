#pragma once

#include "temp_save_replay.hpp"
#include "godot_cpp/classes/node.hpp"
#include "godot_cpp/classes/node2d.hpp"
#include "godot_cpp/classes/node3d.hpp"
#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/core/print_string.hpp"
#include "godot_cpp/variant/array.hpp"
#include "godot_cpp/variant/string.hpp"
#include "godot_cpp/variant/variant.hpp"
#include <tuple>

bool Temp_save_replay::add_node(godot::Node* node)
{
    if(tracked_nodes.has(node))
    {
        godot::print_line("Node " + node->get_name() + " is already in the list");
        return false;
    } 
    godot::print_line("Node " + node->get_name() + " has been added to the recording list");
    tracked_nodes.append(node);
    return true;
}

bool Temp_save_replay::remove_node(godot::Node* node)
{
    ;
    if(tracked_nodes.has(node))
    {
        tracked_nodes.erase(node);
    godot::print_line("Node " + node->get_name() + " has been removed from recording list");
        return true;
    }
    godot::print_line("Node " + node->get_name() + " was not found in the recording list");
    return false;
}

void Temp_save_replay::debug_print_array()
{
    for (auto nodeVariant : tracked_nodes) 
    {
        auto node = godot::Object::cast_to<godot::Node>(nodeVariant);
        godot::print_line("Node: " + node->get_name());
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
            " Position: \nX: " + godot::String::num(position.x) 
            + "\nY: " + godot::String::num(position.y));
        }

        godot::print_line("\n3D positions:");
        for(auto it = range3d.first; it !=range3d.second; it++)
        {
            auto node = std::get<0>(it->second);
            auto position = std::get<1>(it->second);
            godot::print_line("Node: " + node->get_name() + 
            " Position: \nX: " + godot::String::num(position.x) 
            + "\nY: " + godot::String::num(position.y)
            + "\nZ: " + godot::String::num(position.z));
        }
    }
}

void Temp_save_replay::start_recording()
{
    is_recording = true;
    recording_frame = 0;
    temporary_data_map_3d_pos.clear();
    temporary_data_map_2d_pos.clear();
    last_recorded_3d_pos.clear();
    last_recorded_2d_pos.clear();
}

void Temp_save_replay::stop_recording()
{
    is_recording = false;
    // TODO: write to json here
}

void Temp_save_replay::start_replay()
{
    is_replaying = true;
    replay_frame = 0;
}

void Temp_save_replay::stop_replay()
{
    is_replaying = false;
}

void Temp_save_replay::handle_recording()
{
    for(auto nodeVariant : tracked_nodes)
    {
        auto node = godot::Object::cast_to<godot::Node>(nodeVariant);
        auto node3d = godot::Object::cast_to<godot::Node3D>(node);
        auto node2d = godot::Object::cast_to<godot::Node2D>(node);
        if(node3d)
        {
            auto current_position = node3d->get_global_position();
            
            if (last_recorded_3d_pos[node] != current_position || recording_frame == 0)
            {
                temporary_data_map_3d_pos.emplace(recording_frame, std::make_tuple(node, current_position));
                last_recorded_3d_pos[node] = current_position;
            }
        }
        if(node2d)
        {   
            auto current_position = node2d->get_global_position();

            if (last_recorded_2d_pos[node] != current_position || recording_frame == 0)
            {
                godot::print_line("Recorded position: " + current_position);
                temporary_data_map_2d_pos.emplace(recording_frame, std::make_tuple(node, current_position));
                last_recorded_2d_pos[node] = current_position;
            }
        
        }
    }

    recording_frame++;
}

void Temp_save_replay::handle_replaying()
{
    if(recording_frame == 0 || temporary_data_map_2d_pos.empty() && temporary_data_map_3d_pos.empty())
    {
        godot::print_line("No recording in memory.");
        return;
    }

    auto range2d= temporary_data_map_2d_pos.equal_range(replay_frame);
    auto range3d = temporary_data_map_3d_pos.equal_range(replay_frame);
    
    //Set positions
    for (auto data = range2d.first; data != range2d.second; data++) 
    {
        godot::Node *node = std::get<0>(data->second);
        godot::Vector2 pos = std::get<1>(data->second);

        if (auto node2d = godot::Object::cast_to<godot::Node2D>(node))
        {
            node2d->set_global_position(pos);
        }
    }

    for (auto data = range3d.first; data != range3d.second; data++) 
    {
        godot::Node *node = std::get<0>(data->second);
        godot::Vector3 pos = std::get<1>(data->second);
        
        if (auto node3d = godot::Object::cast_to<godot::Node3D>(node))
        {
            node3d->set_global_position(pos);
        }
    }

    replay_frame++;

    if(replay_frame > recording_frame)
    {
        is_replaying = false;
    }
}

void Temp_save_replay::update()
{
    if(is_recording)
    {
        handle_recording();
    }

    if(is_replaying)
    {
        handle_replaying();
    }

}

void Temp_save_replay::set_tracked_nodes(godot::Array tracked_nodes_new)
{
    tracked_nodes = tracked_nodes_new;
}

godot::Array Temp_save_replay::get_tracked_nodes()
{
    return tracked_nodes;
}

void Temp_save_replay::_bind_methods()
{
    godot::ClassDB::bind_method(godot::D_METHOD("debug_print_array"), &Temp_save_replay::debug_print_array);
    godot::ClassDB::bind_method(godot::D_METHOD("debug_print_positions"), &Temp_save_replay::debug_print_positions);
    godot::ClassDB::bind_method(godot::D_METHOD("add_node", "node"), &Temp_save_replay::add_node);
    godot::ClassDB::bind_method(godot::D_METHOD("remove_node", "node"), &Temp_save_replay::remove_node);
    godot::ClassDB::bind_method(godot::D_METHOD("start_recording"), &Temp_save_replay::start_recording);
    godot::ClassDB::bind_method(godot::D_METHOD("stop_recording"), &Temp_save_replay::stop_recording);
    godot::ClassDB::bind_method(godot::D_METHOD("start_replay"), &Temp_save_replay::start_replay);
    godot::ClassDB::bind_method(godot::D_METHOD("stop_replay"), &Temp_save_replay::stop_replay);
    godot::ClassDB::bind_method(godot::D_METHOD("update"), &Temp_save_replay::update);

    godot::ClassDB::bind_method(godot::D_METHOD("set_tracked_nodes", "new_tracked_nodes"), &Temp_save_replay::set_tracked_nodes);
    godot::ClassDB::bind_method(godot::D_METHOD("get_tracked_nodes"), &Temp_save_replay::get_tracked_nodes);
    ADD_PROPERTY(godot::PropertyInfo(godot::Variant::ARRAY , "tracked_nodes"), "set_tracked_nodes", "get_tracked_nodes");
}