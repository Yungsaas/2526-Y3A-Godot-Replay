#pragma once

#include "godot_cpp/classes/node.hpp"
#include "godot_cpp/classes/wrapped.hpp"
#include "godot_cpp/variant/string.hpp"
#include "godot_cpp/variant/transform3d.hpp"
#include <tuple>
#include <unordered_map>
#include <vector>

class Selection_Panels:public godot::Node
{

    // Make class usable in godot with gdscript
    GDCLASS(Selection_Panels, Node)
    
    protected:
    // Bind c plus plus methods to gdscript class
    static void _bind_methods();

    private:
    godot::Node* input_screen;
    godot::Node* position_screen;
    godot::Node* event_screen;

    public:
    bool add_input_screen(godot::Node* node);
    bool add_position_screen(godot::Node* node);
    bool add_event_screen(godot::Node* node);
    
    void print_input();
    void print_positions();
    void print_event();
    
    void update();
};