#pragma once

#include "godot_cpp/classes/node.hpp"
#include "godot_cpp/classes/wrapped.hpp"
#include "godot_cpp/variant/string.hpp"
#include "godot_cpp/variant/transform3d.hpp"
#include <tuple>
#include <unordered_map>
#include <vector>

class Selection_Panel:public godot::Node{

    // Make class usable in godot with gdscript
    GDCLASS(Selection_Panel, Node)
    
    protected:
    // Bind c plus plus methods to gdscript class
    static void _bind_methods();

    private:
    godot::Node* position_screen;
    public:

    void print_positions();
    
    void update();
};