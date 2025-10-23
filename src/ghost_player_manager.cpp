#pragma once
#include "ghost_player_manager.hpp"
#include "godot_cpp/classes/packed_scene.hpp"
#include "godot_cpp/classes/ref.hpp"
#include "godot_cpp/classes/resource.hpp"
#include "godot_cpp/classes/resource_loader.hpp"
#include "godot_cpp/classes/rigid_body2d.hpp"
#include "godot_cpp/classes/rigid_body3d.hpp"
#include "godot_cpp/classes/script.hpp"
#include "godot_cpp/core/print_string.hpp"
#include "godot_cpp/variant/string.hpp"

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
        //replay ghost
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
    
    //Create ghost player node
    godot::Ref<godot::Resource> res_player = godot::ResourceLoader::get_singleton()->load(player_packed_scene_path);
    godot::Ref<godot::PackedScene> packed_player = res_player;

    if(packed_player.is_valid())
    {
        ghost_player_node = packed_player->instantiate();
        if(ghost_player_node)
        {
            ghost_player_node->set_owner(player_parent_node->get_owner());
            player_parent_node->add_child(ghost_player_node);
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
}

void Ghost_Player_Manager::remove_script_from_ghost()
{
    //set to empty script
    ghost_player_node->set_script(godot::Ref<godot::Script>());
}

void Ghost_Player_Manager::pause_ghost_replay()
{
    ghost_replaying = false;
}

void Ghost_Player_Manager::continue_ghost_replay()
{
    ghost_replaying = true;
}

void Ghost_Player_Manager::stop_ghost_replay()
{
    ghost_replaying = false;
}

void Ghost_Player_Manager::_bind_methods()
{

}