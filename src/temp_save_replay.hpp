#pragma once

#include "godot_cpp/classes/json.hpp"
#include "godot_cpp/classes/node.hpp"
#include "godot_cpp/classes/wrapped.hpp"
#include "godot_cpp/variant/string.hpp"
#include "godot_cpp/variant/vector3.hpp"
#include <cstddef>
#include <tuple>
#include <unordered_map>

class Temp_save_replay:public godot::Node{

    // Make class usable in godot with gdscript
    GDCLASS(Temp_save_replay, Node)
    
    protected:
    // Bind c plus plus methods to gdscript class
    static void _bind_methods();

    private:
    godot::Array tracked_nodes;
    std::unordered_multimap<int, std::tuple<godot::Node*,godot::Vector3>> temporary_data_map_3d_pos;
    std::unordered_multimap<int, std::tuple<godot::Node*,godot::Vector2>> temporary_data_map_2d_pos;
    std::unordered_map<godot::Node*, godot::Vector3> last_recorded_3d_pos;
    std::unordered_map<godot::Node*, godot::Vector2> last_recorded_2d_pos;

    bool is_recording = false;
    bool is_replaying = false;
    int recording_frame = 0;
    int replay_frame = 0;

    godot::Ref<godot::JSON> json_path;

    void handle_recording();
    void handle_replaying();

    void save_2dpos_to_json();
    void load_json_file_to_game();

    void add_nodes_from_group();
    
    public:
    void set_tracked_nodes(godot::Array new_tracked_nodes);
    godot::Array get_tracked_nodes();

    void set_json_path(const godot::Ref<godot::JSON> &p_path);

    bool add_node(godot::Node* node);
    bool remove_node(godot::Node* node);
    
    void debug_print_array();
    void debug_print_positions();

    void start_recording();
    void stop_recording();

    void start_replay();
    void stop_replay();

    void update();
};