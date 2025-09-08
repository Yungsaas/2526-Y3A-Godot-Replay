#pragma once

#include "godot_cpp/classes/node.hpp"
#include "godot_cpp/classes/wrapped.hpp"
#include "godot_cpp/variant/array.hpp"
#include "godot_cpp/variant/string.hpp"
#include "godot_cpp/variant/vector3.hpp"

struct nodePosTrack
{
    int frame;
    float posX;
    float posY;
    float posZ;
};

class Temp_save_replay:public godot::Node{

    // Make class usable in godot with gdscript
    GDCLASS(Temp_save_replay, Node)
    
    protected:
    // Bind c plus plus methods to gdscript class
    static void _bind_methods();

    private:
    godot::Array tracked_nodes;
    std::unordered_map<std::tuple<godot::Node*, int>, godot::Vector3> temporary_saved_data;
    bool recording = false;
    int recordingFrame = 0;
    
    public:
    bool add_node(godot::Node* node);

    void debug_print_array();

    void start_recording();
    void stop_recording();

    void update();

};