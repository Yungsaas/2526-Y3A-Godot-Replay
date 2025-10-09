#pragma once

#include "godot_cpp/classes/input.hpp"
#include "godot_cpp/classes/input_map.hpp"
#include "godot_cpp/classes/json.hpp"
#include "godot_cpp/classes/node.hpp"
#include "godot_cpp/classes/wrapped.hpp"
#include "godot_cpp/variant/string.hpp"
#include "godot_cpp/variant/string_name.hpp"
#include "godot_cpp/variant/variant.hpp"
#include "godot_cpp/variant/vector3.hpp"
#include <functional>
#include <godot_cpp/classes/text_edit.hpp>
#include <tuple>
#include <unordered_map>

//Custom key for last recorded custom data
struct CustomDataKey {
	godot::Node *node_ptr;
	godot::StringName data_name;
	bool operator==(CustomDataKey const &o) const
	{
		return node_ptr == o.node_ptr && data_name == o.data_name;
	}
};

struct CustomDataKeyHash {
	size_t operator()(CustomDataKey const &k) const noexcept
	{
		//Hashing the custom data key
		size_t h1 = std::hash<godot::Node *>()(k.node_ptr);
		size_t h2 = static_cast<size_t>(k.data_name.hash()); //Found the solution to the memory intensive utf8 conversion (was .hash() not .get_hash())
		return h1 ^ (h2 << 1);
	}
};

struct CustomDataEntry {
	godot::Node *node;
	godot::StringName variableName;
	godot::Variant variableData;

	//Constructor
	CustomDataEntry(godot::Node *n, godot::StringName &nm, const godot::Variant &v) :
			node(n), variableName(nm), variableData(v) {}
};

class Recorder : public godot::Node {
	// Make class usable in godot with gdscript
	GDCLASS(Recorder, Node)

protected:
	// Bind c plus plus methods to gdscript class
	static void _bind_methods();

	virtual void handle_recording();
	void handle_replaying();

	void record_input();
	void replay_input();

	void record_position();
	void replay_position();

	void record_custom_data();
	void replay_custom_data();

	void save_2dpos_to_json();
	void save_input_to_json();

	void save_all_to_json()
	{
		save_2dpos_to_json();
		save_input_to_json();
	}

	void clear_all_temp_maps();

	void load_json_file_to_game();

	void add_nodes_from_groups();

	
	

	//node lists for data tracking
	godot::Array tracked_nodes; //List of tracked nodes
	std::unordered_multimap<godot::Node *, godot::StringName> tracked_custom_data; //List of tracked data of specific nodes (other than position)

	//temporary data maps
	std::unordered_multimap<int, std::tuple<godot::Node *, godot::Vector3>> temporary_data_map_3d_pos; //Recorded data for 3D positions
	std::unordered_multimap<int, std::tuple<godot::Node *, godot::Vector2>> temporary_data_map_2d_pos; //Recorded data for 2D positions
	std::unordered_multimap<int, std::tuple<godot::StringName, bool>> temporary_data_map_input; //Recorded input data
	std::unordered_multimap<int, CustomDataEntry> temporary_data_map_custom_data; //Recorded data for other data that is serializable in godot

	//last recorded data to avoid redundant data in memory
	std::unordered_map<CustomDataKey, godot::Variant, CustomDataKeyHash> last_recorded_custom_data; //Used for cheking custom data changes
	std::unordered_map<godot::Node *, godot::Vector3> last_recorded_3d_pos; //Used for checking for position changes
	std::unordered_map<godot::Node *, godot::Vector2> last_recorded_2d_pos; //Used for checking for position changes

	std::vector<godot::StringName> recording_groups; //Vector of groups that are supposed to get recorded

	godot::Input *input_singleton = godot::Input::get_singleton(); //Input interface
	godot::InputMap *input_map_singleton = godot::InputMap::get_singleton(); //List of possible inputs
	
	bool is_recording = false;
	bool is_replaying = false;
	bool input_active = true;
	bool position_active = true;
	bool custom_data_active = true;
	bool json_enabled = true;

	bool replay_paused = false;

	bool controlled_replay = false;

	int replay_frame = 0;
	int recording_frame = 0;

	godot::Ref<godot::JSON> json_path;
	godot::Ref<godot::JSON> input_json_path;

private:

public:
	void set_tracked_nodes(godot::Array new_tracked_nodes);
	godot::Array get_tracked_nodes();

	void set_json_path(const godot::Ref<godot::JSON> &p_path);
	void set_input_json_path(const godot::Ref<godot::JSON> &p_path);

	bool add_node(godot::Node *node);
	bool remove_node(godot::Node *node);

	void debug_print_array();
	void debug_print_positions();

	virtual void start_recording();
	virtual void stop_recording();

	virtual void start_replay();
	virtual void stop_replay();

	virtual int get_min_record_frame()
	{
		return 0;
	}
	
	void replay_pause_trigger()
	{
		if(replay_paused)
		{
			replay_paused = false;
		} else {
			replay_paused = true;
		}
	}

	void force_pause_replay()
	{
		replay_paused = true;
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

	void set_custom_data_recording_state(bool state)
	{
		custom_data_active = state;
	}
	bool get_custom_data_recording_state()
	{
		return custom_data_active;
	}

	void set_json_saving(bool state)
	{
		json_enabled = state;
	}
	bool get_json_saving()
	{
		return json_enabled;
	}

	int get_replay_frame()
	{
		return replay_frame;
	}

	int get_recording_frame()
	{
		return recording_frame;
	}

	bool get_general_recording_state()
	{
		return is_recording;
	}

	bool get_general_replay_state()
	{
		return is_replaying;
	}

	void set_replay_frame(int f)
	{
		replay_frame = f;
	}

	bool get_replay_pause()
	{
		return replay_paused;
	}

	void set_controlled_replay(bool state)
	{
		controlled_replay = state;
	}

	void force_set_replay_frame(int forced_frame)
	{
		replay_frame = forced_frame;
	}

	void add_recording_group(godot::StringName group_to_add);

	void add_custom_data(godot::Node *node, godot::StringName customDataName);

	void check_input();

	bool set_input_screen(godot::TextEdit *new_screen);
	godot::TextEdit *get_input_screen();

	void update();

	virtual int get_max_recording_length()
	{
		return 0; //no max recording length
	}
	
	virtual ~Recorder() = default;
};