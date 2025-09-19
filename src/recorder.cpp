#pragma once

#include "recorder.hpp"
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
#include <cstddef>
#include <godot_cpp/classes/input_event.hpp>
#include <godot_cpp/classes/input_event_key.hpp>
#include <tuple>
#include <unordered_map>

bool Temp_save_replay::add_node(godot::Node *node) {
	if (tracked_nodes.has(node)) {
		godot::print_line("Node " + node->get_name() + " is already in the list");
		return false;
	}
	godot::print_line("Node " + node->get_name() + " has been added to the recording list");
	tracked_nodes.append(node);
	return true;
}

bool Temp_save_replay::remove_node(godot::Node *node) {
	;
	if (tracked_nodes.has(node)) {
		tracked_nodes.erase(node);
		godot::print_line("Node " + node->get_name() + " has been removed from recording list");
		return true;
	}
	godot::print_line("Node " + node->get_name() + " was not found in the recording list");
	return false;
}

void Temp_save_replay::debug_print_array() {
	for (auto nodeVariant : tracked_nodes) {
		auto node = godot::Object::cast_to<godot::Node>(nodeVariant);
		godot::print_line("Node: " + node->get_name());
	}
}

void Temp_save_replay::debug_print_positions() {
	godot::print_line("Printing recorded positions: ");
	for (int currentFrame = 0; currentFrame < recording_frame; currentFrame++) {
		// Print all data for each frame
		auto range2d = temporary_data_map_2d_pos.equal_range(currentFrame);
		auto range3d = temporary_data_map_3d_pos.equal_range(currentFrame);

		godot::print_line("\nFrame: " + godot::String::num(currentFrame) + "\n2D positions:");
		for (auto iterator = range2d.first; iterator != range2d.second; iterator++) {
			auto node = std::get<0>(iterator->second);
			auto position = std::get<1>(iterator->second);
			godot::print_line("Node: " + node->get_name() +
					" Position: \nX: " + godot::String::num(position.x) + "\nY: " + godot::String::num(position.y));
		}

		godot::print_line("\n3D positions:");
		for (auto it = range3d.first; it != range3d.second; it++) {
			auto node = std::get<0>(it->second);
			auto position = std::get<1>(it->second);
			godot::print_line("Node: " + node->get_name() +
					" Position: \nX: " + godot::String::num(position.x) + "\nY: " + godot::String::num(position.y) + "\nZ: " + godot::String::num(position.z));
		}
	}
}

void Temp_save_replay::start_recording() {
	is_recording = true;
	recording_frame = 0;
	temporary_data_map_3d_pos.clear();
	temporary_data_map_2d_pos.clear();
	temporary_data_map_input.clear();
	last_recorded_3d_pos.clear();
	last_recorded_2d_pos.clear();

	add_nodes_from_group();
}

void Temp_save_replay::add_nodes_from_group() {
	godot::Node *self_node_ptr = this;
	godot::Node *owner = self_node_ptr->get_owner();

	if (!owner) {
		godot::print_error("Owner not found!");
		return;
	}

	godot::Array group_nodes = owner->get_tree()->get_nodes_in_group("recording");
	for (int i = 0; i < group_nodes.size(); i++) {
		if (godot::Node *current_node = godot::Object::cast_to<Node>(group_nodes[i])) {
			add_node(current_node);
		}
	}
}

void Temp_save_replay::stop_recording() {
	is_recording = false;

	save_2dpos_to_json();
}

void Temp_save_replay::start_replay() {
	is_replaying = true;
	replay_frame = 0;
}

void Temp_save_replay::stop_replay() {
	is_replaying = false;
}

void Temp_save_replay::replay_position() {
	if (!position_active)
		return;
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

void Temp_save_replay::record_position() {
	if (!position_active)
		return;
	for (auto nodeVariant : tracked_nodes) {
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
				godot::print_line("Recorded position: " + current_position);
				temporary_data_map_2d_pos.emplace(recording_frame, std::make_tuple(node, current_position));
				last_recorded_2d_pos[node] = current_position;
			}
		}
	}
}

void Temp_save_replay::handle_recording() {
	record_input();

	record_position();

	recording_frame++;
}

void Temp_save_replay::handle_replaying() {
	if (recording_frame == 0 || temporary_data_map_2d_pos.empty() && temporary_data_map_3d_pos.empty() && temporary_data_map_input.empty()) {
		godot::print_line("No recording in memory.");
		return;
	}

	replay_input();

	replay_position();

	replay_frame++;

	if (replay_frame > recording_frame) {
		is_replaying = false;
	}
}

void Temp_save_replay::save_2dpos_to_json() {
	godot::Array entries;

	for (int currentFrame = 0; currentFrame < recording_frame; currentFrame++) {
		auto range2d = temporary_data_map_2d_pos.equal_range(currentFrame);

		for (auto iterator = range2d.first; iterator != range2d.second; iterator++) {
			godot::Dictionary entry;

			auto node = std::get<0>(iterator->second);
			auto position = std::get<1>(iterator->second);

			entry["frame"] = currentFrame;
			entry["node"] = node->get_path();
			entry["pos"] = position;

			entries.push_back(entry);
		}
	}

	godot::Dictionary root;
	root["recorder_nodes"] = tracked_nodes.size();
	root["frame_count"] = recording_frame; // total number of frames
	root["entries"] = entries;

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

void Temp_save_replay::load_json_file_to_game() {
	if (json_path != NULL) {
		auto json_data = json_path->get_data(); //json file -> Variant

		godot::Dictionary dict = json_data; // Variant -> Dictionary

		temporary_data_map_2d_pos.clear();

		if (dict.has("frame_count")) {
			int frames = dict["frame_count"];
			recording_frame = frames;
		}

		if (dict.has("entries")) {
			godot::Array entires = dict["entries"]; // array of dictionaries holding the frames, nodes and positions
			for (int i = 0; i < entires.size(); i++) {
				godot::Dictionary entry = entires[i];

				int frame = (int)entry["frame"];

				godot::String node_path_str = entry["node"];
				godot::NodePath node_path(node_path_str);

				godot::Node *node = get_node<godot::Node>(node_path);

				godot::Variant pos_var = entry["pos"];
				godot::Vector2 positions;

				if (pos_var.get_type() == godot::Variant::STRING) {
					godot::String pos_str = pos_var;
					pos_str = pos_str.strip_edges().replace("(", "").replace(")", "");
					godot::Array parts = pos_str.split(",");
					if (parts.size() == 2) {
						positions.x = (float)godot::Variant(parts[0]);
						positions.y = (float)godot::Variant(parts[1]);
					} else {
						godot::print_line("Invalid position string!");
					}
				} else {
					godot::print_line("pos_var is not a STRING!");
				}

				temporary_data_map_2d_pos.insert({ frame, std::make_tuple(node, positions) });
				//temporary_data_map_2d_pos.emplace(frame, std::make_tuple(node, positions));
			}
		}
	}
}

void Temp_save_replay::set_json_path(const godot::Ref<godot::JSON> &p_path) {
	json_path = p_path;
}

void Temp_save_replay::update() {
	if (is_recording) {
		handle_recording();
	}

	if (is_replaying) {
		handle_replaying();
	}
}

void Temp_save_replay::set_tracked_nodes(godot::Array tracked_nodes_new) {
	tracked_nodes = tracked_nodes_new;
}

godot::Array Temp_save_replay::get_tracked_nodes() {
	return tracked_nodes;
}

void Temp_save_replay::check_input() {
	godot::Array actions = input_map_singleton->get_actions();

	for (int i = 0; i < actions.size(); i++) {
		godot::StringName action_name = actions[i];
		if (input_singleton->is_action_pressed(action_name)) {
			godot::print_line("Action pressed: ", action_name);
		}
	}
}

void Temp_save_replay::record_input() {
	if (!input_active)
		return;
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

void Temp_save_replay::replay_input() {
	if (!input_active)
		return;

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

void Temp_save_replay::_bind_methods() {
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
	godot::ClassDB::bind_method(godot::D_METHOD("set_json_path", "json_file"), &Temp_save_replay::set_json_path);
	godot::ClassDB::bind_method(godot::D_METHOD("load_json_file"), &Temp_save_replay::load_json_file_to_game);

	godot::ClassDB::bind_method(godot::D_METHOD("check_input"), &Temp_save_replay::check_input);
	godot::ClassDB::bind_method(godot::D_METHOD("set_input_recording_state", "state"), &Temp_save_replay::set_input_recording_state);
	godot::ClassDB::bind_method(godot::D_METHOD("get_input_recording_state"), &Temp_save_replay::get_input_recording_state);
	godot::ClassDB::bind_method(godot::D_METHOD("set_position_recording_state", "state"), &Temp_save_replay::set_position_recording_state);
	godot::ClassDB::bind_method(godot::D_METHOD("get_position_recording_state"), &Temp_save_replay::get_position_recording_state);
}