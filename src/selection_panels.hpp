#pragma once

#include "godot_cpp/classes/node.hpp"
#include "godot_cpp/classes/wrapped.hpp"
#include "godot_cpp/variant/string.hpp"
#include "godot_cpp/variant/transform3d.hpp"
#include <godot_cpp/classes/text_edit.hpp>
#include <tuple>
#include <unordered_map>
#include <vector>
#include "temp_save_replay.hpp"

class Selection_Panels:public godot::Node
{

    // Make class usable in godot with gdscript
    GDCLASS(Selection_Panels, Node)
    
    protected:
    // Bind c plus plus methods to gdscript class
    static void _bind_methods();

    private:
    Temp_save_replay* replay_ptr;

    godot::TextEdit* input_screen;
    godot::TextEdit* position_screen = nullptr;
    godot::Node* event_screen;

    public:
    //Selection_Panels(Temp_save_replay* existing_replay) : replay_ptr(existing_replay) {}

    void set_replay_ptr(Temp_save_replay* recorder);

    bool add_input_screen(godot::TextEdit* node);
    bool add_position_screen(godot::TextEdit* node);
    bool add_event_screen(godot::Node* node);
    
    void print_input();
    void print_positions();
    void print_event();
    
    void update();
};