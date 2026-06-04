#pragma once

#include "godot_cpp/classes/file_access.hpp"
#include "godot_cpp/classes/input.hpp"
#include "godot_cpp/classes/input_map.hpp"
#include "godot_cpp/classes/json.hpp"
#include "godot_cpp/classes/node.hpp"
#include "godot_cpp/classes/node2d.hpp"
#include "godot_cpp/classes/node3d.hpp"
#include "godot_cpp/classes/object.hpp"
#include "godot_cpp/classes/ref.hpp"
#include "godot_cpp/classes/rigid_body2d.hpp"
#include "godot_cpp/classes/rigid_body3d.hpp"
#include "godot_cpp/classes/scene_tree.hpp"
#include "godot_cpp/classes/window.hpp"
#include "godot_cpp/classes/os.hpp"                 // MP4 export: OS::execute
#include "godot_cpp/classes/project_settings.hpp"   // MP4 export: globalize_path
#include "godot_cpp/classes/rendering_server.hpp"   // MP4 export: frame_post_draw
#include "godot_cpp/classes/viewport_texture.hpp"   // MP4 export: viewport readback
#include "godot_cpp/classes/texture2d.hpp"          // MP4 export: get_image
#include "godot_cpp/classes/dir_access.hpp"         // MP4 export: temp dir handling
#include "godot_cpp/classes/engine.hpp"             // MP4 export: max fps override
#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/core/print_string.hpp"
#include "godot_cpp/variant/array.hpp"
#include "godot_cpp/variant/dictionary.hpp"
#include "godot_cpp/variant/string.hpp"
#include "godot_cpp/variant/string_name.hpp"
#include "godot_cpp/variant/variant.hpp"
#include "godot_cpp/variant/vector2.hpp"
#include "godot_cpp/variant/packed_string_array.hpp" // MP4 export: ffmpeg args
#include "godot_cpp/variant/callable.hpp"            // MP4 export: signal connect
#include "recorder.hpp"
#include <cstddef>
#include <godot_cpp/classes/input_event.hpp>
#include <godot_cpp/classes/input_event_key.hpp>
#include <tuple>
#include <unordered_map>
#include <utility>

bool Recorder::is_node_valid(godot::Node *node)
{
	if (!node) {
		return false;
	}
	// Check if node is in the tree - safest check in Godot
	return node->is_inside_tree();
}

bool Recorder::add_node(godot::Node *node)
{
	if (tracked_nodes.has(node)) {
		godot::print_line("Node " + node->get_name() + " is already in the list");
		return false;
	}
	godot::print_line("Node " + node->get_name() + " has been added to the recording list");
	tracked_nodes.append(node);
	return true;
}

bool Recorder::remove_node(godot::Node *node)
{
	if (tracked_nodes.has(node)) {
		tracked_nodes.erase(node);
		godot::print_line("Node " + node->get_name() + " has been removed from recording list");
		return true;
	}
	godot::print_line("Node " + node->get_name() + " was not found in the recording list");
	return false;
}

void Recorder::debug_print_array()
{
	for (auto nodeVariant : tracked_nodes) {
		auto node = godot::Object::cast_to<godot::Node>(nodeVariant);
		godot::print_line("Node: " + node->get_name());
	}
}

void Recorder::debug_print_positions()
{
	godot::print_line("Printing recorded positions: ");
	for (int currentFrame = 0; currentFrame < recording_frame; currentFrame++) {
		// Print all data for each frame
		auto range2d = temporary_data_map_2d_pos.equal_range(currentFrame);
		auto range3d = temporary_data_map_3d_pos.equal_range(currentFrame);

		godot::print_line("\nRecorded Frame: " + godot::String::num(currentFrame));
		godot::print_line("\n2D positions:");
		for (auto iterator = range2d.first; iterator != range2d.second; iterator++) {
			auto node = std::get<0>(iterator->second);
			auto position = std::get<1>(iterator->second);
			godot::print_line("Node: " + node->get_name() + " Position: \nX: " + godot::String::num(position.x) + "\nY: " + godot::String::num(position.y));
		}

		godot::print_line("\n3D positions:");
		for (auto it = range3d.first; it != range3d.second; it++) {
			auto node = std::get<0>(it->second);
			auto position = std::get<1>(it->second);
			godot::print_line("Node: " + node->get_name() + " Position: \nX: " + godot::String::num(position.x) + "\nY: " + godot::String::num(position.y) + "\nZ: " + godot::String::num(position.z));
		}
	}
}

void Recorder::create_node_snapshots()
{
	godot::print_line("Creating node snapshots for replay...");
	
	// Clear any existing snapshots
	clear_snapshots();
	
	// Create snapshots of all tracked nodes
	for (auto nodeVariant : tracked_nodes) {
		if (!nodeVariant.booleanize()) {
			continue; // Skip invalid nodes
		}
		
		auto node = godot::Object::cast_to<godot::Node>(nodeVariant);
		if (!node || !node->is_inside_tree()) {
			continue;
		}
		
		// Duplicate the node (deep copy with flags: DUPLICATE_SIGNALS | DUPLICATE_GROUPS | DUPLICATE_SCRIPTS)
		godot::Node *snapshot = godot::Object::cast_to<godot::Node>(node->duplicate(7));
		if (!snapshot) {
			godot::print_error("Failed to create snapshot for node: " + node->get_name());
			continue;
		}
		
		// Store parent information for later restoration
		if (node->get_parent()) {
			snapshot->set_meta("original_parent_path", node->get_parent()->get_path());
		}
		
		// Store the snapshot (but don't add it to the scene tree yet)
		node_snapshots[node] = snapshot;
		snapshot_to_original[snapshot] = node;
		
		godot::print_line("Created snapshot for node: " + node->get_name());
	}
	
	godot::print_line("Snapshot creation complete. Total snapshots: " + godot::String::num_int64(node_snapshots.size()));
}

void Recorder::restore_destroyed_nodes()
{
	godot::print_line("=== Restore Destroyed Nodes Start ===");
	godot::print_line("Total snapshots: " + godot::String::num_int64(node_snapshots.size()));
	
	// Get the scene tree
	godot::SceneTree *tree = get_tree();
	if (!tree) {
		godot::print_error("No scene tree available");
		return;
	}
	
	// Check each tracked node and restore if needed
	for (auto &snapshot_pair : node_snapshots) {
		godot::Node *original = snapshot_pair.first;
		godot::Node *snapshot = snapshot_pair.second;
		
		if (!snapshot) {
			continue;
		}
		
		// Check if original node is still valid
		bool node_is_valid = is_node_valid(original);
		
		if (!node_is_valid) {
			godot::print_line("Original node destroyed, restoring snapshot to tree");
			
			// Get the parent path if it was stored
			if (snapshot->has_meta("original_parent_path")) {
				godot::NodePath parent_path = snapshot->get_meta("original_parent_path");
				godot::Node *parent = tree->get_root()->get_node_or_null(parent_path);
				
				if (parent && snapshot->get_parent() == nullptr) {
					parent->add_child(snapshot);
					godot::print_line("Snapshot added to original parent: " + parent->get_name());
				}
			} else if (snapshot->get_parent() == nullptr) {
				// Fallback: add to root
				tree->get_root()->add_child(snapshot);
				godot::print_line("Snapshot added to root");
			}
		}
	}
	
	godot::print_line("=== Restore Destroyed Nodes End ===");
}

void Recorder::remap_replay_data_to_snapshots()
{
	godot::print_line("Remapping replay data to snapshots...");
	
	// Remap 2D positions
	std::unordered_multimap<int, std::tuple<godot::Node *, godot::Vector2>> remapped_2d;
	for (auto &entry : temporary_data_map_2d_pos) {
		int frame = entry.first;
		godot::Node *original = std::get<0>(entry.second);
		godot::Vector2 pos = std::get<1>(entry.second);
		
		// Use snapshot if available and original is invalid, otherwise use original
		godot::Node *node_to_use = original;
		if (!is_node_valid(original) && node_snapshots.count(original) > 0) {
			node_to_use = node_snapshots[original];
			godot::print_line("Remapped 2D node to snapshot: " + original->get_name());
		}
		
		remapped_2d.emplace(frame, std::make_tuple(node_to_use, pos));
	}
	temporary_data_map_2d_pos = remapped_2d;
	
	// Remap 3D positions
	std::unordered_multimap<int, std::tuple<godot::Node *, godot::Vector3>> remapped_3d;
	for (auto &entry : temporary_data_map_3d_pos) {
		int frame = entry.first;
		godot::Node *original = std::get<0>(entry.second);
		godot::Vector3 pos = std::get<1>(entry.second);
		
		godot::Node *node_to_use = original;
		if (!is_node_valid(original) && node_snapshots.count(original) > 0) {
			node_to_use = node_snapshots[original];
			godot::print_line("Remapped 3D node to snapshot: " + original->get_name());
		}
		
		remapped_3d.emplace(frame, std::make_tuple(node_to_use, pos));
	}
	temporary_data_map_3d_pos = remapped_3d;
	
	// Remap custom data
	std::unordered_multimap<int, CustomDataEntry> remapped_custom;
	for (auto &entry : temporary_data_map_custom_data) {
		int frame = entry.first;
		CustomDataEntry data = entry.second;
		
		if (!is_node_valid(data.node) && node_snapshots.count(data.node) > 0) {
			data.node = node_snapshots[data.node];
			godot::print_line("Remapped custom data node to snapshot");
		}
		
		remapped_custom.emplace(frame, data);
	}
	temporary_data_map_custom_data = remapped_custom;
	
	godot::print_line("Remapping complete");
}

void Recorder::clear_snapshots()
{
	godot::print_line("Clearing node snapshots...");
	
	// Free all snapshot nodes
	for (auto &snapshot_pair : node_snapshots) {
		godot::Node *snapshot = snapshot_pair.second;
		if (snapshot && godot::Object::cast_to<godot::Node>(snapshot)) {
			// Remove from tree if it's in the tree
			if (snapshot->is_inside_tree()) {
				snapshot->get_parent()->remove_child(snapshot);
			}
			snapshot->queue_free();
		}
	}
	
	node_snapshots.clear();
	snapshot_to_original.clear();
	
	godot::print_line("Snapshots cleared");
}

void Recorder::start_recording()
{
	is_recording = true;
	recording_frame = 0;
	clear_all_temp_maps();
	last_recorded_3d_pos.clear();
	last_recorded_2d_pos.clear();

	add_nodes_from_groups();

	create_node_snapshots();
}

void Recorder::add_nodes_from_groups()
{
	godot::Node *self_node_ptr = this;
	godot::Node *owner = self_node_ptr->get_owner();

	if (!owner) {
		godot::print_error("Owner not found!");
		return;
	}

	//add nodes from generated recording group first
	godot::Array group_nodes = owner->get_tree()->get_nodes_in_group("recording");
	for (int i = 0; i < group_nodes.size(); i++) {
		if (godot::Node *current_node = godot::Object::cast_to<Node>(group_nodes[i])) {
			add_node(current_node);
		}
	}

	//add nodes from added groups
	for (auto group_name : recording_groups) {
		godot::Array group_nodes = owner->get_tree()->get_nodes_in_group(group_name);
		for (int i = 0; i < group_nodes.size(); i++) {
			if (godot::Node *current_node = godot::Object::cast_to<Node>(group_nodes[i])) {
				add_node(current_node);
			}
		}
	}
}

void Recorder::stop_recording()
{
	is_recording = false;
	godot::print_line("Stopping recording");
	save_2dpos_to_json();
	save_3dpos_to_json();
	save_input_to_json();
	save_custom_to_json();
}

void Recorder::clear_all_temp_maps()
{
	temporary_data_map_2d_pos.clear();
	temporary_data_map_3d_pos.clear();
	temporary_data_map_custom_data.clear();
	temporary_data_map_input.clear();

	last_recorded_2d_pos.clear();
	last_recorded_3d_pos.clear();
	last_recorded_custom_data.clear();
}

void Recorder::start_replay()
{
	is_replaying = true;
	replay_frame = 0;

	restore_destroyed_nodes();
	
	// CRITICAL: Remap all replay data to use snapshots for destroyed nodes
	remap_replay_data_to_snapshots();
}

void Recorder::stop_replay()
{
	is_replaying = false;

	clear_snapshots();
}

void Recorder::replay_position()
{
	if (!position_active)
		return;

	//godot::print_line("Replaying Positions");
	auto range2d = temporary_data_map_2d_pos.equal_range(replay_frame);
	auto range3d = temporary_data_map_3d_pos.equal_range(replay_frame);

	//Set positions
	for (auto data = range2d.first; data != range2d.second; data++) {
		godot::Node *node = std::get<0>(data->second);
		godot::Vector2 pos = std::get<1>(data->second);

		// SAFETY CHECK: Make sure node is still valid
		if (!is_node_valid(node)) {
			continue;
		}

		if (auto node2d = godot::Object::cast_to<godot::Node2D>(node)) {
			node2d->set_global_position(pos);
			if(replay_paused)
			{
				if(auto body2d = godot::Object::cast_to<godot::RigidBody2D>(node2d))
				{
					body2d->set_angular_velocity(0.0);
					body2d->set_axis_velocity(godot::Vector2(0,0));
					body2d->set_linear_velocity(godot::Vector2(0,0));
				}
			}
		}
	}

	for (auto data = range3d.first; data != range3d.second; data++) {
		godot::Node *node = std::get<0>(data->second);
		godot::Vector3 pos = std::get<1>(data->second);

		// SAFETY CHECK: Make sure node is still valid
		if (!is_node_valid(node)) {
			continue;
		}

		if (auto node3d = godot::Object::cast_to<godot::Node3D>(node)) {
			node3d->set_global_position(pos);
			if(replay_paused)
			{
				if(auto body3d = godot::Object::cast_to<godot::RigidBody3D>(node3d))
				{
					body3d->set_angular_velocity(godot::Vector3(0,0,0));
					body3d->set_axis_velocity(godot::Vector3(0,0,0));
					body3d->set_linear_velocity(godot::Vector3(0,0,0));
				}
			}
		}
	}
}

void Recorder::record_position()
{
	if (!position_active)
		return;

	//godot::print_line("Recording Position");
	for (auto nodeVariant : tracked_nodes) {
		if (nodeVariant.booleanize()) {
			auto node = godot::Object::cast_to<godot::Node>(nodeVariant);
			
			// Safety check
			if (!is_node_valid(node)) {
				continue;
			}
			
			auto node3d = godot::Object::cast_to<godot::Node3D>(node);
			auto node2d = godot::Object::cast_to<godot::Node2D>(node);
			if (node3d) {
				auto current_position = node3d->get_global_position();

				if (last_recorded_3d_pos[node] != current_position || recording_frame == 0) {
					temporary_data_map_3d_pos.emplace(recording_frame, std::make_tuple(node, current_position));
					last_recorded_3d_pos[node] = current_position;
				}
			}
			if (node2d) {
				auto current_position = node2d->get_global_position();

				if (last_recorded_2d_pos[node] != current_position || recording_frame == 0) {
					//godot::print_line("Recorded position: " + current_position);
					temporary_data_map_2d_pos.emplace(recording_frame, std::make_tuple(node, current_position));
					last_recorded_2d_pos[node] = current_position;
				}
			}
		} else {
			//godot::print_error("Node in recording list was deleted");
		}
	}
}

void Recorder::handle_recording()
{
	//godot::print_line("Recording Frame: " + godot::String::num_int64(recording_frame));
	record_input();

	record_position();

	record_custom_data();

	recording_frame++;
}

void Recorder::handle_replaying()
{
	if (recording_frame == 0 || temporary_data_map_2d_pos.empty() && temporary_data_map_3d_pos.empty() && temporary_data_map_input.empty()) {
		godot::print_error("No recording in memory.");
		return;
	}
	
	//godot::print_line("Replay Frame: " + godot::String::num_int64(replay_frame));

	replay_input();

	replay_position();

	replay_custom_data();

	if(replay_paused)
	{
		return;
	}

	replay_frame++;

	if (replay_frame > recording_frame) {
		if(controlled_replay)
		{
			replay_frame = recording_frame;
		}else {
			is_replaying = false;
		}
	}
}

void Recorder::save_2dpos_to_json()
{
	if(!json_enabled)
	{
		return;
	}

	godot::Dictionary node_entries;

	for (int currentFrame = 0; currentFrame < recording_frame; currentFrame++) {
		auto range2d = temporary_data_map_2d_pos.equal_range(currentFrame);

		for (auto iterator = range2d.first; iterator != range2d.second; iterator++) {
			auto node = std::get<0>(iterator->second);
			auto position = std::get<1>(iterator->second);
			
			godot::Dictionary entry;
			entry["frame"] = currentFrame;
			entry["pos"] = position;

			godot::String node_name; // temp fix so we can save destoryed json files
			if (!node->get_path().is_empty()) 
			{
				node_name = node->get_path();
			}
			else 
			{
				node_name = "empty";
			}

			// If node key doesn't exist, create an array
			if (!node_entries.has(node_name)) {
				node_entries[node_name] = godot::Array();
			}

			// Push entry into the node's array
			node_entries[node_name].call("push_back", entry);
		}
	}

	godot::Dictionary root;
	root["recorder_nodes"] = tracked_nodes.size();
	root["frame_count"] = recording_frame; // total number of frames
	root["entries"] = node_entries;

	auto json_string = godot::JSON::stringify(root);

	int recording_index = 0;
	godot::String filename;

	while (true) {
		filename = "res://addons/replay_qol/json/position_2D_" + godot::String::num(recording_index) + ".json";

		if (!godot::FileAccess::file_exists(filename)) {
			break; // found available filename
		}
		recording_index++;
	}

	auto file = godot::FileAccess::open(filename, godot::FileAccess::WRITE);

	if (file.is_valid()) {
		file->store_string(json_string); // Write JSON text to file
		file->close();
	}
}

void Recorder::save_3dpos_to_json()
{
	if(!json_enabled)
	{
		return;
	}

	godot::Dictionary node_entries;

	for (int currentFrame = 0; currentFrame < recording_frame; currentFrame++) {
		auto range3d = temporary_data_map_3d_pos.equal_range(currentFrame);

		for (auto iterator = range3d.first; iterator != range3d.second; iterator++) {
			auto node = std::get<0>(iterator->second);
			auto position = std::get<1>(iterator->second);
			
			godot::Dictionary entry;
			entry["frame"] = currentFrame;
			entry["pos"] = position;

			godot::String node_name = node->get_path();

			// If node key doesn't exist, create an array
			if (!node_entries.has(node_name)) {
				node_entries[node_name] = godot::Array();
			}

			// Push entry into the node's array
			node_entries[node_name].call("push_back", entry);
		}
	}

	godot::Dictionary root;
	root["recorder_nodes"] = tracked_nodes.size();
	root["frame_count"] = recording_frame; // total number of frames
	root["entries"] = node_entries;

	auto json_string = godot::JSON::stringify(root);

	int recording_index = 0;
	godot::String filename;

	while (true) {
		filename = "res://addons/replay_qol/json/position_3D_" + godot::String::num(recording_index) + ".json";

		if (!godot::FileAccess::file_exists(filename)) {
			break; // found available filename
		}
		recording_index++;
	}

	auto file = godot::FileAccess::open(filename, godot::FileAccess::WRITE);

	if (file.is_valid()) {
		file->store_string(json_string); // Write JSON text to file
		file->close();
	}
}

void Recorder::save_input_to_json() {
	if(!json_enabled)
	{
		return;
	}

	godot::Array actions;

	for (int currentFrame = 0; currentFrame < recording_frame; currentFrame++) {
		auto range_input = temporary_data_map_input.equal_range(currentFrame);
		for (auto iterator = range_input.first; iterator != range_input.second; iterator++) {
			auto string_name = std::get<0>(iterator->second);
			auto pressed = std::get<1>(iterator->second);

			godot::Dictionary entry;
			entry["frame"] = currentFrame;
			entry["name"] = string_name;
			entry["pressed"] = pressed;

			actions.push_back(entry);
		}
	}

	godot::Dictionary root;
	root["recording_frame"] = recording_frame; 
	root["actions"] = actions; 

	auto json_string = godot::JSON::stringify(root);

	int recording_index = 0;
	godot::String filename;

	while (true) {
		filename = "res://addons/replay_qol/json/input_" + godot::String::num(recording_index) + ".json";

		if (!godot::FileAccess::file_exists(filename)) {
			break; // found available filename
		}
		recording_index++;
	}

	auto file = godot::FileAccess::open(filename, godot::FileAccess::WRITE);

	if (file.is_valid()) {
		file->store_string(json_string); // Write JSON text to file
		file->close();
	}
}

void Recorder::save_custom_to_json()
{
	if(!json_enabled)
	{
		return;
	}

	godot::Array custom_data;

	for (int currentFrame = 0; currentFrame < recording_frame; currentFrame++) {
		auto range_input = temporary_data_map_custom_data.equal_range(currentFrame);
		for (auto iterator = range_input.first; iterator != range_input.second; iterator++) {
			auto custom_data_entry = (iterator->second);

			auto custom_data_node = custom_data_entry.node;
			auto custom_data_name = custom_data_entry.variableName;
			auto custom_data_variant = custom_data_entry.variableData;

			godot::Dictionary entry;
			entry["frame"] = currentFrame;
			entry["node"] = custom_data_node;
			entry["name"] = custom_data_name;
			entry["custom_variant"] = custom_data_variant;

			custom_data.push_back(entry);
		}
	}

	godot::Dictionary root;
	root["recording_frame"] = recording_frame; 
	root["custom_data"] = custom_data; 

	auto json_string = godot::JSON::stringify(root);

	int recording_index = 0;
	godot::String filename;

	while (true) {
		filename = "res://addons/replay_qol/json/custom_data_" + godot::String::num(recording_index) + ".json";

		if (!godot::FileAccess::file_exists(filename)) {
			break; // found available filename
		}
		recording_index++;
	}

	auto file = godot::FileAccess::open(filename, godot::FileAccess::WRITE);

	if (file.is_valid()) {
		file->store_string(json_string); // Write JSON text to file
		file->close();
	}
}

void Recorder::load_json_file_to_game() {
	if(!json_enabled)
	{
		godot::print_error("Cant to load json file, json saving disabled");
		return;
	}

	if (json_path != NULL) {
		auto json_data = json_path->get_data(); // JSON file -> Variant
		godot::Dictionary dict = json_data; // Variant -> Dictionary

		temporary_data_map_2d_pos.clear();

		if (dict.has("frame_count")) {
			recording_frame = (int)dict["frame_count"];
		}

		if (dict.has("entries")) {
			godot::Dictionary entries = dict["entries"];

			godot::Array node_keys = entries.keys();
			for (int k = 0; k < node_keys.size(); k++) {
				godot::String node_path_string = node_keys[k];
				godot::NodePath node_path(node_path_string);

				godot::Node *node = get_node<godot::Node>(node_path);

				godot::Array node_entries = entries[node_path_string];
				for (int i = 0; i < node_entries.size(); i++) {
					godot::Dictionary entry = node_entries[i];

					int frame = (int)entry["frame"];

					godot::Variant pos_var = entry["pos"];
					godot::Vector2 positions;

					if (pos_var.get_type() == godot::Variant::STRING) {
						godot::String pos_str = pos_var;
						pos_str = pos_str.strip_edges().replace("(", "").replace(")", ""); // used chat gpt to format the file properly, no idea what this does
						godot::Array parts = pos_str.split(",");

						if (parts.size() == 2) {
							positions.x = (float)godot::Variant(parts[0]);
							positions.y = (float)godot::Variant(parts[1]);
						} else {
							godot::print_line("Invalid position string!");
						}
					} else {
						godot::print_line("pos_var is not of type godot::Variant::STRING!");
					}

					temporary_data_map_2d_pos.insert({ frame, std::make_tuple(node, positions) });
				}
			}
		}
	}

	if (json_3d_path != NULL) {
		auto json_data = json_3d_path->get_data(); // JSON file -> Variant
		godot::Dictionary dict = json_data; // Variant -> Dictionary

		temporary_data_map_2d_pos.clear();

		if (dict.has("frame_count")) {
			recording_frame = (int)dict["frame_count"];
		}

		if (dict.has("entries")) {
			godot::Dictionary entries = dict["entries"];

			godot::Array node_keys = entries.keys();
			for (int k = 0; k < node_keys.size(); k++) {
				godot::String node_path_string = node_keys[k];
				godot::NodePath node_path(node_path_string);

				godot::Node *node = get_node<godot::Node>(node_path);

				godot::Array node_entries = entries[node_path_string];
				for (int i = 0; i < node_entries.size(); i++) {
					godot::Dictionary entry = node_entries[i];

					int frame = (int)entry["frame"];

					godot::Variant pos_var = entry["pos"];
					godot::Vector3 positions;

					if (pos_var.get_type() == godot::Variant::STRING) {
						godot::String pos_str = pos_var;
						pos_str = pos_str.strip_edges().replace("(", "").replace(")", ""); // used chat gpt to format the file properly, no idea what this does
						godot::Array parts = pos_str.split(",");

						if (parts.size() == 3) {
							positions.x = (float)godot::Variant(parts[0]);
							positions.y = (float)godot::Variant(parts[1]);
							positions.z = (float)godot::Variant(parts[2]);
						} else {
							godot::print_line("Invalid position string!");
						}
					} else {
						godot::print_line("pos_var is not of type godot::Variant::STRING!");
					}

					temporary_data_map_3d_pos.insert({ frame, std::make_tuple(node, positions) });
				}
			}
		}
	}

	if (input_json_path != NULL) {
		auto json_data = input_json_path->get_data(); // JSON file -> Variant

		godot::Dictionary root = json_data;

		recording_frame = root["recording_frame"];

		godot::Array actions = root["actions"];

		for (int i = 0; i < actions.size(); i++) {
			godot::Dictionary entry = actions[i];

			int frame = entry["frame"];
			godot::StringName name = entry["name"];
			bool pressed = entry["pressed"];

			temporary_data_map_input.insert({ frame, std::make_tuple(name, pressed) });
		}
	}

	if (custom_json_path != NULL) {
		auto json_data = custom_json_path->get_data(); // JSON file -> Variant

		temporary_data_map_custom_data.clear();

		godot::Dictionary root = json_data;

		recording_frame = root["recording_frame"];

		godot::Array custom_data = root["custom_data"];

		for (int i = 0; i < custom_data.size(); i++) {
			godot::Dictionary entry = custom_data[i];

			int frame = entry["frame"];

			godot::String node_path = entry["node"];
            godot::Node *node = get_node<godot::Node>(node_path);
			
			godot::StringName name = entry["name"];

			godot::Variant custom_data = entry["custom_variant"];

			temporary_data_map_custom_data.emplace(frame, CustomDataEntry(node, name, custom_data));
		}
	}
}

void Recorder::set_json_path(const godot::Ref<godot::JSON> &p_path)
{
	if(!json_enabled)
	{
		godot::print_error("cant set json path, json saving disabled");
		return;
	}
	json_path = p_path;
}

void Recorder::set_3d_json_path(const godot::Ref<godot::JSON> &p_path)
{
	if(!json_enabled)
	{
		godot::print_error("cant set json path, json saving disabled");
		return;
	}
	json_3d_path = p_path;
}

void Recorder::set_input_json_path(const godot::Ref<godot::JSON> &p_path) {
	if(!json_enabled)
	{
		godot::print_error("cant set input json path, json saving disabled");
		return;
	}
	input_json_path = p_path;
}

void Recorder::set_custom_json_path(const godot::Ref<godot::JSON> &p_path) {
	if(!json_enabled)
	{
		godot::print_error("cant set input json path, json saving disabled");
		return;
	}
	custom_json_path = p_path;
}

void Recorder::update() {
	if (is_recording) {
		handle_recording();
	}

	if (is_replaying) {
		handle_replaying();
	}
}

void Recorder::set_tracked_nodes(godot::Array tracked_nodes_new)
{
	for (auto new_node_variant : tracked_nodes_new) {
		auto new_node = godot::Object::cast_to<Node>(new_node_variant);
		add_node(new_node);
	}
}

godot::Array Recorder::get_tracked_nodes()
{
	return tracked_nodes;
}

void Recorder::check_input()
{
	godot::Array actions = input_map_singleton->get_actions();

	for (int i = 0; i < actions.size(); i++) {
		godot::StringName action_name = actions[i];
		if (input_singleton->is_action_pressed(action_name)) {
			godot::print_line("Action pressed: ", action_name);
		}
	}
}

void Recorder::record_input()
{
	if (!input_active)
		return;
	//godot::print_line("Recording Input");
	godot::Array actions = input_map_singleton->get_actions();
	for (int i = 0; i < actions.size(); i++) {
		godot::StringName action_name = actions[i];
		//dont record the recording related inputs
		if (action_name == godot::StringName("start_recording") || action_name == godot::StringName("stop_recording") || action_name == godot::StringName("start_replay") || action_name == godot::StringName("stop_replay")) {
			continue;
		}

		if (input_singleton->is_action_just_pressed(action_name)) {
			temporary_data_map_input.emplace(recording_frame, std::make_tuple(action_name, true));
		} else if (input_singleton->is_action_just_released(action_name)) {
			temporary_data_map_input.emplace(recording_frame, std::make_tuple(action_name, false));
		}
	}
}

void Recorder::replay_input()
{
	if (!input_active)
		return;
	//godot::print_line("Replaying Input");
	godot::Array actions = input_map_singleton->get_actions();
	auto range_input = temporary_data_map_input.equal_range(replay_frame);

	for (auto data = range_input.first; data != range_input.second; data++) {
		godot::StringName action_name = std::get<0>(data->second);
		
		//dont replay the recording related inputs
		if (action_name == godot::StringName("start_recording") || action_name == godot::StringName("stop_recording") || action_name == godot::StringName("start_replay") || action_name == godot::StringName("stop_replay")) {
			continue;
		}
		
		bool press_bool = std::get<1>(data->second);
		if (press_bool) {
			input_singleton->action_press(action_name);
		} else {
			input_singleton->action_release(action_name);
		}

		if(replay_paused)
		{
			input_singleton->action_release(action_name);
		}
	}
}

void Recorder::add_recording_group(godot::StringName group_to_add)
{
	for (const auto &existing_group : recording_groups) {
		if (existing_group == group_to_add) {
			godot::print_error("Group: " + existing_group + " already exists.");
			return;
		}
	}
	recording_groups.push_back(group_to_add);
	godot::print_line("Group: " + group_to_add + " has been added to the recording.");
}

void Recorder::add_custom_data(godot::Node *node, godot::StringName customDataName)
{
	//Save custom data name to map
	tracked_custom_data.emplace(node, customDataName);
	godot::print_line("Data: " + customDataName + " from node: " + node->get_name() + " will be recorded.");
}

void Recorder::record_custom_data()
{
	if (!custom_data_active)
		return;

	for (auto node_data_pair : tracked_custom_data) {
		//Using variables here for readability
		auto node = node_data_pair.first;
		auto data_name = node_data_pair.second;
		
		// Safety check
		if (!is_node_valid(node)) {
			continue;
		}
		
		godot::Variant data_content = node->get_meta(data_name);

		CustomDataKey key{ node, data_name };

		auto it = last_recorded_custom_data.find(key);
		if (it == last_recorded_custom_data.end() || it->second != data_content) {
			//data entry either doesnt exist in last recorded custom data or has changed
			temporary_data_map_custom_data.emplace(recording_frame, CustomDataEntry{ node, data_name, data_content });
			last_recorded_custom_data[key] = (data_content);
			godot::print_line("Custom data: " + data_name + " with data content: " + data_content.stringify() + " has been recorded");
		}
	}
}

void Recorder::replay_custom_data()
{
	if (!custom_data_active)
		return;

	//Get current frame data
	auto range_custom_data = temporary_data_map_custom_data.equal_range(replay_frame);

	//Set data
	for (auto data = range_custom_data.first; data != range_custom_data.second; data++) {
		auto data_entry = data->second;
		
		// Safety check
		if (!is_node_valid(data_entry.node)) {
			continue;
		}
		
		data_entry.node->set_meta(data_entry.variableName, data_entry.variableData);
	}
}

// ============================================================================
//  MP4 EXPORT
//
//  Connects to RenderingServer "frame_post_draw" (fires AFTER a frame is drawn,
//  which is when viewport readback is valid). Each post-draw applies one recorded
//  frame's state; that state is only visible on the NEXT rendered frame, so we
//  capture on the FOLLOWING post-draw (a "primed" one-frame pipeline). One
//  recorded frame becomes exactly one PNG, then ffmpeg encodes the sequence.
//
//  ffmpeg is an external process and cannot read res:// or user://, so every
//  path is run through ProjectSettings::globalize_path() first.
// ============================================================================

godot::String Recorder::zero_pad(int value, int width)
{
	godot::String s = godot::String::num_int64(value);
	while (s.length() < width) {
		s = "0" + s;
	}
	return s;
}

bool Recorder::prepare_export_temp_dir()
{
	// Wipe any leftover frames from a previous export, then ensure the dir exists.
	cleanup_export_temp_dir();

	if (!godot::DirAccess::dir_exists_absolute(export_temp_dir)) {
		godot::Error err = godot::DirAccess::make_dir_recursive_absolute(export_temp_dir);
		if (err != godot::OK) {
			godot::print_error("Export: failed to create temp dir: " + export_temp_dir);
			return false;
		}
	}
	return true;
}

void Recorder::cleanup_export_temp_dir()
{
	if (!godot::DirAccess::dir_exists_absolute(export_temp_dir)) {
		return;
	}

	godot::Ref<godot::DirAccess> dir = godot::DirAccess::open(export_temp_dir);
	if (dir.is_null()) {
		return;
	}

	dir->list_dir_begin();
	godot::String file_name = dir->get_next();
	while (!file_name.is_empty()) {
		if (!dir->current_is_dir()) {
			dir->remove(file_name);
		}
		file_name = dir->get_next();
	}
	dir->list_dir_end();
}

godot::Ref<godot::Image> Recorder::capture_viewport_image()
{
	// Default to the main window viewport. Point capture_viewport at a SubViewport
	// (your game-content viewport) for clean footage without the replay UI overlay.
	godot::Viewport *vp = capture_viewport ? capture_viewport : get_viewport();
	if (!vp) {
		godot::print_error("Export: no viewport available for capture");
		return godot::Ref<godot::Image>();
	}

	godot::Ref<godot::ViewportTexture> tex = vp->get_texture();
	if (tex.is_null()) {
		godot::print_error("Export: viewport texture is null");
		return godot::Ref<godot::Image>();
	}

	return tex->get_image(); // valid here because we are inside frame_post_draw
}

void Recorder::apply_export_frame(int frame)
{
	// Mirror what handle_replaying() does for a single frame, minus the auto
	// increment. replay_paused is true during export, so replay_position()
	// freezes RigidBody velocities and replay_input() neutralises presses.
	replay_frame = frame;
	replay_input();
	replay_position();
	replay_custom_data();
}

void Recorder::start_mp4_export(int fps, godot::String output_path)
{
	if (export_phase != EXPORT_IDLE) {
		godot::print_error("Export: an export is already in progress");
		return;
	}

	if (recording_frame <= 0 ||
		(temporary_data_map_2d_pos.empty() &&
		 temporary_data_map_3d_pos.empty() &&
		 temporary_data_map_custom_data.empty())) {
		export_status = "Export failed: no recording in memory";
		godot::print_error(export_status);
		return;
	}

	if (output_path.is_empty()) {
		export_status = "Export failed: output path is empty";
		godot::print_error(export_status);
		return;
	}

	if (fps <= 0) {
		fps = 60;
	}

	if (!prepare_export_temp_dir()) {
		export_status = "Export failed: could not create temp directory";
		return;
	}

	// Remember state so we can restore it when we are done.
	export_prev_is_replaying  = is_replaying;
	export_prev_replay_paused = replay_paused;
	export_prev_replay_frame  = replay_frame;
	export_prev_max_fps       = godot::Engine::get_singleton()->get_max_fps();
	export_prev_tree_paused   = get_tree()->is_paused();

	// Make sure destroyed-node snapshots are live and the replay data points at
	// them, so objects that were destroyed during recording still appear in the
	// exported video. Both calls are idempotent if a replay already set them up,
	// and finish_export() clears the snapshots again when we are done.
	restore_destroyed_nodes();
	remap_replay_data_to_snapshots();

	// Drive the replay ourselves: no normal replay loop, frozen physics, start at 0.
	is_replaying  = false;
	replay_paused = true;
	replay_frame  = 0;

	// Optional: pause all game logic for perfectly faithful capture. Off by
	// default so the export looks exactly like an in-editor replay.
	if (export_pause_tree) {
		get_tree()->set_pause(true);
	}

	// Remove the vsync cap so the capture loop runs as fast as the GPU/readback
	// allows (every frame still has to render + read back, but this speeds it up).
	godot::Engine::get_singleton()->set_max_fps(0);

	export_fps          = fps;
	export_output_path  = output_path;
	export_apply_frame  = 0;
	export_saved_count  = 0;
	export_primed       = false;
	export_encode_delay = 0;
	export_status       = "Starting export...";
	export_phase        = EXPORT_CAPTURING;

	godot::RenderingServer::get_singleton()->connect(
		"frame_post_draw", godot::Callable(this, "on_export_frame_post_draw"));

	godot::print_line("Export: started, " + godot::String::num_int64(recording_frame) +
					  " frames at " + godot::String::num_int64(fps) + " fps");
}

void Recorder::on_export_frame_post_draw()
{
	if (export_phase == EXPORT_IDLE) {
		return;
	}

	// --- Phase 1: capture ----------------------------------------------------
	if (export_phase == EXPORT_CAPTURING) {
		// A frame was applied last tick and has now been rendered: grab it.
		if (export_primed) {
			godot::Ref<godot::Image> img = capture_viewport_image();
			if (img.is_valid()) {
				godot::String path = export_temp_dir + "frame_" + zero_pad(export_saved_count, 5) + ".png";
				if (img->save_png(path) != godot::OK) {
					godot::print_error("Export: failed to save " + path);
				}
			}
			export_saved_count++;
			export_primed = false;

			export_status = "Capturing frame " + godot::String::num_int64(export_saved_count) +
							" / " + godot::String::num_int64(recording_frame);
			if (export_saved_count % 30 == 0) {
				godot::print_line("Export progress: " + export_status);
			}
		}

		// All recorded frames applied and captured?
		if (export_apply_frame >= recording_frame) {
			export_status      = "Encoding video (please wait)...";
			export_encode_delay = 3; // let the UI paint the status before the encode blocks
			export_phase       = EXPORT_DELAY;
			return;
		}

		// Apply the next recorded frame; it gets captured on the next post-draw.
		apply_export_frame(export_apply_frame);
		export_apply_frame++;
		export_primed = true;
		return;
	}

	// --- Phase 2: tiny delay so "Encoding..." shows -------------------------
	if (export_phase == EXPORT_DELAY) {
		if (export_encode_delay > 0) {
			export_encode_delay--;
			return;
		}
		export_phase = EXPORT_ENCODING;
		return;
	}

	// --- Phase 3: encode + tidy up ------------------------------------------
	if (export_phase == EXPORT_ENCODING) {
		run_ffmpeg_encode(); // blocking, but quick relative to capture
		finish_export();     // disconnect, restore state, go idle
		return;
	}
}

void Recorder::run_ffmpeg_encode()
{
	if (export_saved_count <= 0) {
		export_status = "Export failed: no frames captured";
		godot::print_error(export_status);
		cleanup_export_temp_dir();
		return;
	}

	// ffmpeg needs real OS paths, not res:// or user://.
	godot::ProjectSettings *ps = godot::ProjectSettings::get_singleton();
	godot::String frames_pattern_abs = ps->globalize_path(export_temp_dir + "frame_%05d.png");
	godot::String output_abs         = ps->globalize_path(export_output_path);

	godot::PackedStringArray args;
	args.push_back("-y");                                       // overwrite output
	args.push_back("-framerate");
	args.push_back(godot::String::num_int64(export_fps));       // input fps
	args.push_back("-i");
	args.push_back(frames_pattern_abs);                         // frame_%05d.png
	args.push_back("-vf");
	args.push_back("scale=trunc(iw/2)*2:trunc(ih/2)*2");        // force even dims (libx264 requirement)
	args.push_back("-c:v");
	args.push_back("libx264");
	args.push_back("-pix_fmt");
	args.push_back("yuv420p");                                  // broad player compatibility (VLC etc.)
	args.push_back("-crf");
	args.push_back("18");                                       // quality (lower = better/larger)
	args.push_back(output_abs);

	godot::print_line("Export: encoding " + godot::String::num_int64(export_saved_count) +
					  " frames -> " + output_abs);

	godot::Array ff_output;
	int64_t exit_code = godot::OS::get_singleton()->execute(ffmpeg_path, args, ff_output, true);

	if (exit_code == 0) {
		export_status = "Export complete: " + output_abs;
		godot::print_line(export_status);
	} else if (exit_code == -1) {
		export_status = "Export failed: ffmpeg not found (check PATH or call set_ffmpeg_path)";
		godot::print_error(export_status);
	} else {
		export_status = "Export failed: ffmpeg exit code " + godot::String::num_int64(exit_code);
		godot::print_error(export_status);
		for (int i = 0; i < ff_output.size(); i++) {
			godot::String line = ff_output[i];
			godot::print_error(line);
		}
	}

	cleanup_export_temp_dir(); // always remove the PNG scratch frames
}

void Recorder::finish_export()
{
	godot::RenderingServer *rs = godot::RenderingServer::get_singleton();
	godot::Callable cb = godot::Callable(this, "on_export_frame_post_draw");
	if (rs->is_connected("frame_post_draw", cb)) {
		rs->disconnect("frame_post_draw", cb);
	}

	// Restore the things we changed at start.
	godot::Engine::get_singleton()->set_max_fps(export_prev_max_fps);
	if (export_pause_tree) {
		get_tree()->set_pause(export_prev_tree_paused);
	}

	// Export is a terminal action: it ENDS any replay rather than resuming it.
	// (If we restored is_replaying = true, the controller would treat the replay
	// as live again and re-open / re-initialise the whole replay UI next frame.)
	// We also clear the snapshots, exactly like stop_replay() does, so the
	// destroyed-node duplicates we used for capture are removed from the scene.
	is_replaying  = false;
	replay_paused = export_prev_replay_paused;
	replay_frame  = export_prev_replay_frame;
	clear_snapshots();

	export_phase = EXPORT_IDLE;
}

void Recorder::_bind_methods()
{
	godot::ClassDB::bind_method(godot::D_METHOD("debug_print_array"), &Recorder::debug_print_array);
	godot::ClassDB::bind_method(godot::D_METHOD("debug_print_positions"), &Recorder::debug_print_positions);
	godot::ClassDB::bind_method(godot::D_METHOD("add_node", "node"), &Recorder::add_node);
	godot::ClassDB::bind_method(godot::D_METHOD("remove_node", "node"), &Recorder::remove_node);
	godot::ClassDB::bind_method(godot::D_METHOD("start_recording"), &Recorder::start_recording);
	godot::ClassDB::bind_method(godot::D_METHOD("stop_recording"), &Recorder::stop_recording);
	godot::ClassDB::bind_method(godot::D_METHOD("start_replay"), &Recorder::start_replay);
	godot::ClassDB::bind_method(godot::D_METHOD("stop_replay"), &Recorder::stop_replay);
	godot::ClassDB::bind_method(godot::D_METHOD("update"), &Recorder::update);
	godot::ClassDB::bind_method(godot::D_METHOD("set_tracked_nodes", "new_tracked_nodes"), &Recorder::set_tracked_nodes);
	godot::ClassDB::bind_method(godot::D_METHOD("set_2d_json_path", "json_file"), &Recorder::set_json_path);
	godot::ClassDB::bind_method(godot::D_METHOD("set_3d_json_path", "json_file"), &Recorder::set_3d_json_path);

	godot::ClassDB::bind_method(godot::D_METHOD("load_json_file"), &Recorder::load_json_file_to_game);
	godot::ClassDB::bind_method(godot::D_METHOD("set_input_json_path", "json_file"), &Recorder::set_input_json_path);
	godot::ClassDB::bind_method(godot::D_METHOD("set_custom_json_path", "json_file"), &Recorder::set_custom_json_path);

	godot::ClassDB::bind_method(godot::D_METHOD("get_replay_state"), &Recorder::get_general_replay_state);

	godot::ClassDB::bind_method(godot::D_METHOD("check_input"), &Recorder::check_input);

	godot::ClassDB::bind_method(godot::D_METHOD("add_recording_group", "group_name"), &Recorder::add_recording_group);

	godot::ClassDB::bind_method(godot::D_METHOD("add_custom_data", "node", "custom_data"), &Recorder::add_custom_data);
	
	//Recorder exposed properties
	//Input recording
	godot::ClassDB::bind_method(godot::D_METHOD("set_input_recording_state", "state"), &Recorder::set_input_recording_state);
	godot::ClassDB::bind_method(godot::D_METHOD("get_input_recording_state"), &Recorder::get_input_recording_state);
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::BOOL, "input_recording"), "set_input_recording_state", "get_input_recording_state");
	//Position recording
	godot::ClassDB::bind_method(godot::D_METHOD("set_position_recording_state", "state"), &Recorder::set_position_recording_state);
	godot::ClassDB::bind_method(godot::D_METHOD("get_position_recording_state"), &Recorder::get_position_recording_state);
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::BOOL, "position_recording"), "set_position_recording_state", "get_position_recording_state");
	//Custom data recording
	godot::ClassDB::bind_method(godot::D_METHOD("set_custom_data_recording_state", "state"), &Recorder::set_custom_data_recording_state);
	godot::ClassDB::bind_method(godot::D_METHOD("get_custom_data_recording_state"), &Recorder::get_custom_data_recording_state);
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::BOOL, "custom_data_recording"), "set_custom_data_recording_state", "get_custom_data_recording_state");
	//Json saving
	godot::ClassDB::bind_method(godot::D_METHOD("set_json_saving", "state"), &Recorder::set_json_saving);
	godot::ClassDB::bind_method(godot::D_METHOD("get_json_saving"), &Recorder::get_json_saving);
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::BOOL, "json_saving"), "set_json_saving", "get_json_saving");

	//MP4 export
	godot::ClassDB::bind_method(godot::D_METHOD("start_mp4_export", "fps", "output_path"), &Recorder::start_mp4_export);
	godot::ClassDB::bind_method(godot::D_METHOD("on_export_frame_post_draw"), &Recorder::on_export_frame_post_draw);
	godot::ClassDB::bind_method(godot::D_METHOD("is_export_active"), &Recorder::is_export_active);
	godot::ClassDB::bind_method(godot::D_METHOD("get_export_current_frame"), &Recorder::get_export_current_frame);
	godot::ClassDB::bind_method(godot::D_METHOD("get_export_total_frames"), &Recorder::get_export_total_frames);
	godot::ClassDB::bind_method(godot::D_METHOD("get_export_status"), &Recorder::get_export_status);
	godot::ClassDB::bind_method(godot::D_METHOD("set_ffmpeg_path", "path"), &Recorder::set_ffmpeg_path);
	godot::ClassDB::bind_method(godot::D_METHOD("set_capture_viewport", "viewport"), &Recorder::set_capture_viewport);
	godot::ClassDB::bind_method(godot::D_METHOD("set_export_pause_tree", "enabled"), &Recorder::set_export_pause_tree);
}