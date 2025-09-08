#pragma once

#include "godot_cpp/classes/node.hpp"
#include "godot_cpp/classes/wrapped.hpp"
#include "godot_cpp/variant/string.hpp"

class Temp_save_replay:public godot::Node{

    // Make class usable in godot with gdscript
    GDCLASS(Temp_save_replay, Node)
    
    protected:
    // Bind c plus plus methods to gdscript class
    static void _bind_methods();

    private:
    godot::Array tracked_nodes;
    
    public:
    bool add_node(godot::Node* node);

    void debug_print_array();
    
};