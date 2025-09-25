#pragma once

#include "godot_cpp/classes/input.hpp"
#include "godot_cpp/classes/input_map.hpp"
#include "godot_cpp/classes/json.hpp"
#include "godot_cpp/classes/node.hpp"
#include "godot_cpp/classes/wrapped.hpp"
#include "godot_cpp/variant/array.hpp"
#include "godot_cpp/variant/node_path.hpp"
#include "godot_cpp/variant/string.hpp"
#include "godot_cpp/variant/string_name.hpp"
#include "godot_cpp/variant/vector3.hpp"
#include <godot_cpp/classes/text_edit.hpp>
#include <tuple>
#include <unordered_map>

class Recorder : public godot::Node {
	// Make class usable in godot with gdscript
	GDCLASS(Recorder, Node)

protected:
	// Bind c plus plus methods to gdscript class
	static void _bind_methods();

private:
	godot::Array tracked_nodes;
	godot::Array snapshot_nodes;
	std::unordered_multimap<int, std::tuple<godot::Node *, godot::Vector3>> temporary_data_map_3d_pos;
	std::unordered_multimap<int, std::tuple<godot::Node *, godot::Vector2>> temporary_data_map_2d_pos;
    std::unordered_multimap<int, std::tuple<godot::StringName, bool>> temporary_data_map_input;
	std::unordered_map<godot::Node *, godot::Vector3> last_recorded_3d_pos;
	std::unordered_map<godot::Node *, godot::Vector2> last_recorded_2d_pos;

    godot::Input *input_singleton = godot::Input::get_singleton();
    godot::InputMap *input_map_singleton = godot::InputMap::get_singleton();
	bool is_recording = false;
	bool is_replaying = false;
	bool input_active = true;
    bool position_active = true;
	int recording_frame = 0;
	int replay_frame = 0;

	godot::NodePath main_scene_path;

	godot::Ref<godot::JSON> json_path;
	godot::Ref<godot::JSON> input_json_path;

	void handle_recording();
	void handle_replaying();

    void record_input();
    void replay_input();

    void record_position();
    void replay_position();

	void save_2dpos_to_json();
	void save_input_to_json();
	void load_json_file_to_game();

	void add_nodes_from_group();

public:
	void set_tracked_nodes(godot::Array new_tracked_nodes);
	godot::Array get_tracked_nodes();

	void set_json_path(const godot::Ref<godot::JSON> &p_path);
	void set_input_json_path(const godot::Ref<godot::JSON> &p_path);

	void get_missing_nodes();
	void instantiate_from_snapshot(godot::Node *node);
	void cleanup_tracked_nodes();

	void set_snapshot();
	godot::Array get_snapshot();

	bool add_node(godot::Node *node);
	bool remove_node(godot::Node *node);

	void debug_print_array();
	void debug_print_positions();

	void debug_print_temp();

	void start_recording();
	void stop_recording();

	void start_replay();
	void stop_replay();

	void get_main_scene(godot::NodePath new_main_path)
	{
		main_scene_path = new_main_path;
	}

    void set_input_recording_state(bool state)
    {
        input_active = state;
    }
    bool get_input_recording_state()
    {
        return input_active;
    }

    void set_position_recording_state(bool state)
    {
        position_active = state;
    }
    bool get_position_recording_state()
    {
        return position_active;
    }

	void check_input();

	bool set_input_screen(godot::TextEdit *new_screen);
	godot::TextEdit *get_input_screen();

	void update();
};