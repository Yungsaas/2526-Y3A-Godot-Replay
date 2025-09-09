#pragma once

#include "godot_cpp/classes/node.hpp"
#include "godot_cpp/classes/wrapped.hpp"
#include "godot_cpp/variant/string.hpp"
#include "godot_cpp/variant/transform3d.hpp"
#include <tuple>
#include <unordered_map>
#include <vector>

class Temp_save_replay:public godot::Node{

    // Make class usable in godot with gdscript
    GDCLASS(Temp_save_replay, Node)
    
    protected:
    // Bind c plus plus methods to gdscript class
    static void _bind_methods();

    private:
    std::vector<godot::Node*> tracked_nodes;
    std::unordered_multimap<int, std::tuple<godot::Node*,godot::Transform3D>> temporary_data_map_3d_pos;
    std::unordered_multimap<int, std::tuple<godot::Node*,godot::Transform2D>> temporary_data_map_2d_pos;
    bool is_recording = false;
    int recording_frame = 0;

    public:
    bool add_node(godot::Node* node);

    void debug_print_array();
    void debug_print_positions();

    void start_recording();
    void stop_recording();
    
    void update();
};