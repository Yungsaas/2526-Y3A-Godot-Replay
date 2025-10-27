#pragma once
#include "ghost_player_manager.hpp"
#include "godot_cpp/classes/node.hpp"
#include "godot_cpp/classes/node2d.hpp"
#include "godot_cpp/classes/object.hpp"
#include "godot_cpp/classes/packed_scene.hpp"
#include "godot_cpp/classes/ref.hpp"
#include "godot_cpp/classes/resource.hpp"
#include "godot_cpp/classes/resource_loader.hpp"
#include "godot_cpp/classes/rigid_body2d.hpp"
#include "godot_cpp/classes/rigid_body3d.hpp"
#include "godot_cpp/classes/script.hpp"
#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/core/print_string.hpp"
#include "godot_cpp/variant/node_path.hpp"
#include "godot_cpp/variant/string.hpp"
#include "godot_cpp/variant/variant.hpp"

void Ghost_Player_Manager::set_player_node(godot::Node* new_player_node){
    player_node = new_player_node;
    player_node_path = player_node->get_path();
    player_parent_node = player_node->get_parent();
    player_parent_node_path = player_parent_node->get_path();
    player_packed_scene_path = player_node->get_scene_file_path();

}

void Ghost_Player_Manager::update()
{
    if(ghost_replaying)
    {
        replay_position();
        replay_rotation();
        ghost_replay_current_frame++;
        if(ghost_replay_current_frame > ghost_replay_max_frame)
        {
            ghost_replaying = false;
            pause_ghost_replay();
        }
    }

    if(ghost_recording)
    {
        record_ghost();
        ghost_recording_frame++;
    }
}

void Ghost_Player_Manager::start_ghost_replay()
{
    if(!player_node)
    {
        godot::print_error("Player node has not been set in ghost player manager. Aborting ghost replay.");
        return;
    }

    if(!player_parent_node_path)
    {
        godot::print_error("Player parent node was not found. Aborting ghost replay.");
        return;
    }

    ghost_replaying = true;
    ghost_replay_current_frame = 0;
    ghost_replay_max_frame = ghost_recording_frame;

    godot::print_line("Ghost replay max frame set to " + godot::String::num(ghost_replay_max_frame));

    //Create ghost player node
    godot::Ref<godot::Resource> res_player = godot::ResourceLoader::get_singleton()->load(player_packed_scene_path);
    godot::Ref<godot::PackedScene> packed_player = res_player;

    if(packed_player.is_valid())
    {
        ghost_player_node = packed_player->instantiate();
        if(ghost_player_node)
        {
            godot::print_line("Ghost player instantiated.");
            ghost_player_node->set_owner(player_parent_node->get_owner());
            player_parent_node->add_child(ghost_player_node);

            disable_physics_for_ghost();
            set_metadata();
            remove_script_from_ghost();

        } else {
            godot::print_error("Ghost player could not be instantiated.");
        }
    }else {
        godot::print_error("Player packed scene is invalid.");
    }
}

void Ghost_Player_Manager::disable_physics_for_ghost()
{
    //2D
    if(auto rb2d = godot::Object::cast_to<godot::RigidBody2D>(ghost_player_node))
    {
        rb2d->set_collision_layer(0);
        rb2d->set_collision_mask(0);
        rb2d->set_freeze_enabled(true);
    }

    //3D
    if(auto rb3d = godot::Object::cast_to<godot::RigidBody3D>(ghost_player_node))
    {
        rb3d->set_collision_layer(0);
        rb3d->set_collision_mask(0);
        rb3d->set_freeze_enabled(true);
    }

    /*
    //Disable physics for ghost children
    for (int i = 0; i < ghost_player_node->get_child_count(); ++i) 
    {
        godot::Node *child = ghost_player_node->get_child(i);
        //2D
        if(auto rb2d = godot::Object::cast_to<godot::RigidBody2D>(child))
        {
            rb2d->set_collision_layer(0);
            rb2d->set_collision_mask(0);
            rb2d->set_freeze_enabled(true);
        }

        //3D
        if(auto rb3d = godot::Object::cast_to<godot::RigidBody3D>(child))
        {
            rb3d->set_collision_layer(0);
            rb3d->set_collision_mask(0);
            rb3d->set_freeze_enabled(true);
        }
    }
    */
}

void Ghost_Player_Manager::remove_script_from_ghost()
{
    //set to empty script
    ghost_player_node->set_script(godot::Ref<godot::Script>());

        //Remove scripts for children
    for (int i = 0; i < ghost_player_node->get_child_count(); ++i) 
    {
        godot::Node *child = ghost_player_node->get_child(i);
        child->set_script(godot::Ref<godot::Script>());
    }
}

void Ghost_Player_Manager::set_metadata()
{
    ghost_player_node->set_meta("is_ghost", true);
}

void Ghost_Player_Manager::pause_ghost_replay()
{
    godot::print_line("Paused ghost replay.");
    ghost_replaying = false;
}

void Ghost_Player_Manager::continue_ghost_replay()
{
    ghost_replaying = true;
}

void Ghost_Player_Manager::stop_ghost_replay()
{
    godot::print_line("Stopped ghost player replay.");
    ghost_replaying = false;
    //remove and delete ghost player
    delete_ghost_player();
}

void Ghost_Player_Manager::delete_ghost_player()
{
    if(!ghost_player_node)
    {
        godot::print_error("Unable to delete ghost player, no ghost player found.");
        return;
    }

    if(ghost_player_node->get_parent())
    {
        ghost_player_node->get_parent()->remove_child(ghost_player_node);
    }

    ghost_player_node->queue_free();

    ghost_player_node = nullptr;

    godot::print_line("Ghost player was deleted.");
}

void Ghost_Player_Manager::replay_position()
{
    if(!ghost_player_node)
    {
        godot::print_error("Ghost player node not initiated, aborting replay.");
        ghost_replaying = false;
        return;
    }

    godot::print_line("Replaying frame: " + godot::String::num(ghost_replay_current_frame));
    //replay position:
    if(auto node2d = godot::Object::cast_to<godot::Node2D>(ghost_player_node))
    {
        auto it =temporary_data_map_2d_rep.find(ghost_replay_current_frame);
        if( it != temporary_data_map_2d_rep.end() )
        {
            node2d->set_global_position(temporary_data_map_2d_rep[ghost_replay_current_frame]);
            godot::print_line("Ghost pos2d: " + godot::String::num(node2d->get_global_position().x) + " " + godot::String::num(node2d->get_global_position().y));
        }
    }
    if(auto node3d = godot::Object::cast_to<godot::Node3D>(ghost_player_node))
    {
        auto it =temporary_data_map_3d_rep.find(ghost_replay_current_frame);
        if( it != temporary_data_map_3d_rep.end())
        {
            node3d->set_global_position(temporary_data_map_3d_rep[ghost_replay_current_frame]);
        }
    }
}

void Ghost_Player_Manager::replay_rotation()
{
    if(!ghost_player_node)
    {
        godot::print_error("Ghost player node not initiated, aborting replay.");
        ghost_replaying = false;
        return;
    }
    //replay rotation:
    if(auto node2d = godot::Object::cast_to<godot::Node2D>(ghost_player_node))
    {
        auto it =temporary_data_map_2d_rep.find(ghost_replay_current_frame);
        if( it != temporary_data_map_2d_rep.end() )
        {
            node2d->set_global_rotation(temporary_data_map_2d_rotation_rep[ghost_replay_current_frame]);
        }
    }
    if(auto node3d = godot::Object::cast_to<godot::Node3D>(ghost_player_node))
    {
        auto it =temporary_data_map_3d_rep.find(ghost_replay_current_frame);
        if( it != temporary_data_map_3d_rep.end())
        {
            node3d->set_global_rotation(temporary_data_map_3d_rotation_rep[ghost_replay_current_frame]);
        }
    }
}

godot::Node* Ghost_Player_Manager::get_player_node()
{
    return player_node;
}

void Ghost_Player_Manager::set_replay_path(godot::String new_path)
{
    json_replay_path = new_path;
}

godot::String Ghost_Player_Manager::get_replay_path()
{
    return json_replay_path;
}

void Ghost_Player_Manager::start_ghost_recording()
{
    ghost_recording = true;
    ghost_recording_frame = 0;

    //make sure all recording maps are cleared out
    temporary_data_map_2d_rec.clear();
    temporary_data_map_2d_rotation_rec.clear();
    temporary_data_map_3d_rec.clear();
    temporary_data_map_3d_rotation_rec.clear();
}

void Ghost_Player_Manager::record_ghost()
{
    if(!ghost_recording)
    {
        return;
    }
    
    if(!player_node)
    {
        godot::print_error("No player node set in ghost player manager!");
        return;
    }

    if(auto p2dn = godot::Object::cast_to<godot::Node2D>(player_node))
    {
        if(prev_pos2!=p2dn->get_global_position()){
        temporary_data_map_2d_rec[ghost_recording_frame] = p2dn->get_global_position();
        prev_pos2 = temporary_data_map_2d_rec[ghost_recording_frame];}

        if(prev_rot2!=p2dn->get_global_rotation())
        {
            temporary_data_map_2d_rotation_rec[ghost_recording_frame] = p2dn->get_global_rotation();
            prev_rot2 = temporary_data_map_2d_rotation_rec[ghost_recording_frame];
        }
    }
    if(auto p3dn = godot::Object::cast_to<godot::Node3D>(player_node))
    {
        if(prev_pos3!=p3dn->get_global_position()){
        temporary_data_map_3d_rec[ghost_recording_frame] = p3dn->get_global_position();
        prev_pos3 = temporary_data_map_3d_rec[ghost_recording_frame];}

        if(prev_rot3!=p3dn->get_global_rotation())
        {
            temporary_data_map_3d_rotation_rec[ghost_recording_frame] = p3dn->get_global_rotation();
            prev_rot3 = temporary_data_map_3d_rec[ghost_recording_frame];
        }
    }
}

void Ghost_Player_Manager::stop_ghost_recording()
{
    ghost_recording = false;
    godot::print_line("Stopped ghost recording. Frames: " + godot::String::num(ghost_recording_frame));
}

void Ghost_Player_Manager::load_ghost_recording_from_memory()
{
    temporary_data_map_2d_rep = temporary_data_map_2d_rec;
    temporary_data_map_3d_rep = temporary_data_map_3d_rec;
    temporary_data_map_2d_rotation_rep = temporary_data_map_2d_rotation_rec;
    temporary_data_map_3d_rotation_rep = temporary_data_map_3d_rotation_rec;
}



void Ghost_Player_Manager::_bind_methods()
{
    
    //General functions    
	godot::ClassDB::bind_method(godot::D_METHOD("update"), &Ghost_Player_Manager::update);
    godot::ClassDB::bind_method(godot::D_METHOD("delete_ghost_player"), &Ghost_Player_Manager::delete_ghost_player);

    //Ghost replay functionality
	godot::ClassDB::bind_method(godot::D_METHOD("start_ghost_replay"), &Ghost_Player_Manager::start_ghost_replay);
	godot::ClassDB::bind_method(godot::D_METHOD("pause_ghost_replay"), &Ghost_Player_Manager::pause_ghost_replay);
	godot::ClassDB::bind_method(godot::D_METHOD("continue_ghost_replay"), &Ghost_Player_Manager::continue_ghost_replay);
	godot::ClassDB::bind_method(godot::D_METHOD("stop_ghost_replay"), &Ghost_Player_Manager::stop_ghost_replay);

    //Ghost recording functionality
    godot::ClassDB::bind_method(godot::D_METHOD("start_ghost_recording"), &Ghost_Player_Manager::start_ghost_recording);
    godot::ClassDB::bind_method(godot::D_METHOD("stop_ghost_recording"), &Ghost_Player_Manager::stop_ghost_recording);

    //Recording/replay data
    godot::ClassDB::bind_method(godot::D_METHOD("load_from_memory"), &Ghost_Player_Manager::load_ghost_recording_from_memory);

    //Add json replay path property
	godot::ClassDB::bind_method(godot::D_METHOD("set_replay_path", "json_path"), &Ghost_Player_Manager::set_replay_path);
	godot::ClassDB::bind_method(godot::D_METHOD("get_replay_path"), &Ghost_Player_Manager::get_replay_path);
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::STRING, "json_replay_path"), "set_replay_path", "get_replay_path");

    //Player node
	godot::ClassDB::bind_method(godot::D_METHOD("set_player_node", "node"), &Ghost_Player_Manager::set_player_node);
	godot::ClassDB::bind_method(godot::D_METHOD("get_player_node"), &Ghost_Player_Manager::get_player_node);


	godot::ClassDB::bind_method(godot::D_METHOD("set_player_node_raw", "node"), &Ghost_Player_Manager::set_player_node_raw);

	godot::ClassDB::bind_method(godot::D_METHOD("set_ghost_player_node", "node"), &Ghost_Player_Manager::set_ghost_player_node);
	godot::ClassDB::bind_method(godot::D_METHOD("get_ghost_player_node"), &Ghost_Player_Manager::get_ghost_player_node);
    
}