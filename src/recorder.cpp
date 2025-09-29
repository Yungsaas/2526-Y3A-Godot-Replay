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
#include "godot_cpp/classes/scene_tree.hpp"
#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/core/print_string.hpp"
#include "godot_cpp/variant/array.hpp"
#include "godot_cpp/variant/dictionary.hpp"
#include "godot_cpp/variant/string.hpp"
#include "godot_cpp/variant/string_name.hpp"
#include "godot_cpp/variant/variant.hpp"
#include "godot_cpp/variant/vector2.hpp"
#include "recorder.hpp"
#include <cstddef>
#include <godot_cpp/classes/input_event.hpp>
#include <godot_cpp/classes/input_event_key.hpp>
#include <tuple>
#include <unordered_map>
#include <utility>

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
	;
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

void Recorder::start_recording()
{
	is_recording = true;
	recording_frame = 0;
	temporary_data_map_3d_pos.clear();
	temporary_data_map_2d_pos.clear();
	temporary_data_map_input.clear();
	last_recorded_3d_pos.clear();
	last_recorded_2d_pos.clear();

	add_nodes_from_groups();
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

	save_2dpos_to_json();
	save_input_to_json();
}

void Recorder::start_replay()
{
	is_replaying = true;
	replay_frame = 0;
}

void Recorder::stop_replay()
{
	is_replaying = false;
}

void Recorder::replay_position()
{
	if (!position_active)
		return;

	godot::print_line("Replaying Positions");
	auto range2d = temporary_data_map_2d_pos.equal_range(replay_frame);
	auto range3d = temporary_data_map_3d_pos.equal_range(replay_frame);

	//Set positions
	for (auto data = range2d.first; data != range2d.second; data++) {
		godot::Node *node = std::get<0>(data->second);
		godot::Vector2 pos = std::get<1>(data->second);

		if (auto node2d = godot::Object::cast_to<godot::Node2D>(node)) {
			node2d->set_global_position(pos);
		}
	}

	for (auto data = range3d.first; data != range3d.second; data++) {
		godot::Node *node = std::get<0>(data->second);
		godot::Vector3 pos = std::get<1>(data->second);

		if (auto node3d = godot::Object::cast_to<godot::Node3D>(node)) {
			node3d->set_global_position(pos);
		}
	}
}

void Recorder::record_position()
{
	if (!position_active)
		return;

	godot::print_line("Recording Position");
	for (auto nodeVariant : tracked_nodes) {
		if (nodeVariant.booleanize()) {
			auto node = godot::Object::cast_to<godot::Node>(nodeVariant);
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
			godot::print_error("Node in recording list was deleted");
		}
	}
}

void Recorder::handle_recording()
{
	godot::print_line("Recording Frame: " + godot::String::num_int64(recording_frame));
	record_input();

	record_position();

	record_custom_data();

	recording_frame++;
}

void Recorder::handle_replaying()
{
	if (recording_frame == 0 || temporary_data_map_2d_pos.empty() && temporary_data_map_3d_pos.empty() && temporary_data_map_input.empty()) {
		godot::print_line("No recording in memory.");
		return;
	}
	godot::print_line("Replay Frame: " + godot::String::num_int64(replay_frame));

	replay_input();

	replay_position();

	replay_custom_data();

	replay_frame++;

	if (replay_frame > recording_frame) {
		is_replaying = false;
	}
}

void Recorder::save_2dpos_to_json()
{
	godot::Dictionary node_entries;

	for (int currentFrame = 0; currentFrame < recording_frame; currentFrame++) {
		auto range2d = temporary_data_map_2d_pos.equal_range(currentFrame);

		for (auto iterator = range2d.first; iterator != range2d.second; iterator++) {
			auto node = std::get<0>(iterator->second);
			auto position = std::get<1>(iterator->second);

			godot::Dictionary entry;
			entry["frame"] = currentFrame;
			entry["pos"] = position;

			godot::String node_name = node->get_path();

			// If node key doesn’t exist, create an array
			if (!node_entries.has(node_name)) {
				node_entries[node_name] = godot::Array();
			}

			// Push entry into the node’s array
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
		filename = "res://addons/replay_qol/json/test_" + godot::String::num(recording_index) + ".json";

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

void Recorder::load_json_file_to_game() {
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

	if (input_json_path != NULL) {
		auto json_data = input_json_path->get_data(); // JSON file -> Variant

		// Root is now a Dictionary
		godot::Dictionary root = json_data;

		// Extract recording_frame once
		recording_frame = root["recording_frame"];

		// Extract actions array
		godot::Array actions = root["actions"];

		for (int i = 0; i < actions.size(); i++) {
			godot::Dictionary entry = actions[i];

			int frame = entry["frame"];
			godot::StringName name = entry["name"];
			bool pressed = entry["pressed"];

			temporary_data_map_input.insert({ frame, std::make_tuple(name, pressed) });
		}
	}
}

void Recorder::set_json_path(const godot::Ref<godot::JSON> &p_path)
{
	json_path = p_path;
}

void Recorder::set_input_json_path(const godot::Ref<godot::JSON> &p_path) {
	input_json_path = p_path;
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
	godot::print_line("Recording Input");
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
	godot::print_line("Replaying Input");
	godot::Array actions = input_map_singleton->get_actions();
	auto range_input = temporary_data_map_input.equal_range(replay_frame);

	for (auto data = range_input.first; data != range_input.second; data++) {
		godot::StringName action_name = std::get<0>(data->second);
		bool press_bool = std::get<1>(data->second);
		if (press_bool) {
			input_singleton->action_press(action_name);
		} else {
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
	if (tracked_nodes.has(node)) {
		tracked_custom_data.emplace(node, customDataName);
		godot::print_line("Data: " + customDataName + " from node: " + node->get_name() + " will be recorded.");
	} else {
		godot::print_error("Node: " + node->get_name() + " is not in the recording list.");
	}
}

void Recorder::record_custom_data()
{
	if (!custom_data_active)
		return;

	for (auto node_data_pair : tracked_custom_data) {
		//Using variables here for readability
		auto node = node_data_pair.first;
		auto data_name = node_data_pair.second;
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
		data_entry.node->set_meta(data_entry.variableName, data_entry.variableData);
	}
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
	godot::ClassDB::bind_method(godot::D_METHOD("set_json_path", "json_file"), &Recorder::set_json_path);
	godot::ClassDB::bind_method(godot::D_METHOD("load_json_file"), &Recorder::load_json_file_to_game);
	godot::ClassDB::bind_method(godot::D_METHOD("set_input_json_path", "json_file"), &Recorder::set_input_json_path);

	godot::ClassDB::bind_method(godot::D_METHOD("check_input"), &Recorder::check_input);
	godot::ClassDB::bind_method(godot::D_METHOD("set_input_recording_state", "state"), &Recorder::set_input_recording_state);
	godot::ClassDB::bind_method(godot::D_METHOD("get_input_recording_state"), &Recorder::get_input_recording_state);
	godot::ClassDB::bind_method(godot::D_METHOD("set_position_recording_state", "state"), &Recorder::set_position_recording_state);
	godot::ClassDB::bind_method(godot::D_METHOD("get_position_recording_state"), &Recorder::get_position_recording_state);
	godot::ClassDB::bind_method(godot::D_METHOD("set_custom_data_recording_state", "state"), &Recorder::set_custom_data_recording_state);
	godot::ClassDB::bind_method(godot::D_METHOD("get_custom_data_recording_state"), &Recorder::get_custom_data_recording_state);

	godot::ClassDB::bind_method(godot::D_METHOD("add_recording_group", "group_name"), &Recorder::add_recording_group);

	godot::ClassDB::bind_method(godot::D_METHOD("add_custom_data", "node", "custom_data"), &Recorder::add_custom_data);
}