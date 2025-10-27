#pragma once
#include "godot_cpp/classes/json.hpp"
#include "godot_cpp/classes/node.hpp"
#include "godot_cpp/classes/packed_scene.hpp"
#include "godot_cpp/classes/ref.hpp"
#include "godot_cpp/variant/node_path.hpp"

class Ghost_Player_Manager : public godot::Node
{
	// Make class usable in godot with gdscript
	GDCLASS(Ghost_Player_Manager, Node)

protected:
	// Bind c plus plus methods to gdscript class
	static void _bind_methods();

private:

godot::Node* player_node;
godot::Node* player_parent_node;
godot::NodePath player_node_path;
godot::NodePath player_parent_node_path;
godot::PackedScene player_packed_scene;
godot::String player_packed_scene_path;

//Temporary data maps for ghost player replay
std::unordered_map<int, godot::Vector3> temporary_data_map_3d_rep;
std::unordered_map<int, godot::Vector2> temporary_data_map_2d_rep;

//Temporary data maps for ghost player recording
std::unordered_map<int, godot::Vector3> temporary_data_map_3d_rec;
std::unordered_map<int, godot::Vector2> temporary_data_map_2d_rec;

void load_json_file();

bool ghost_replaying = false;
bool ghost_recording = false;
int ghost_replay_current_frame = 0;
int ghost_replay_max_frame;
int ghost_recording_frame = 0;

godot::Node* ghost_player_node;
godot::NodePath ghost_player_node_path;

godot::String json_replay_path;
godot::Ref<godot::JSON> json_replay_path_ref;


void disable_physics_for_ghost();
void remove_script_from_ghost();
void delete_ghost_player();
void replay_position();
void record_ghost();

public:

void set_player_node(godot::Node* new_player_node);
godot::Node* get_player_node();

void set_replay_path(godot::String new_path);
godot::String get_replay_path();

void start_ghost_replay();
void pause_ghost_replay();
void continue_ghost_replay();
void stop_ghost_replay();

void start_ghost_recording();
void stop_ghost_recording();

void load_ghost_recording_from_memory();

void save_ghost_recording_to_json(); //TODO
void load_ghost_recording_from_json(); //TODO

void update();
};