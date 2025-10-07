#pragma once
#include "godot_cpp/classes/control.hpp"
#include "godot_cpp/classes/h_slider.hpp"
#include "godot_cpp/classes/label.hpp"
#include "godot_cpp/classes/packed_scene.hpp"
#include "godot_cpp/classes/popup_panel.hpp"
#include "recorder.hpp"
#include "godot_cpp/classes/resource_loader.hpp"

class Recorder_Controller : public godot::Node {
	// Make class usable in godot with gdscript
	GDCLASS(Recorder_Controller, Node)

protected:
	// Bind c plus plus methods to gdscript class
	static void _bind_methods();

private:
    void Measure_node_allocation(godot::Node *node);

	Recorder *recorder;
    godot::PopupPanel *controls_popup_panel;
	godot::PopupPanel *input_popup_panel;
	godot::PopupPanel *debug_popup_panel;

	godot::Control *input_lable_parent;
    godot::HSlider *time_line_slider;

	godot::Label *frame_counter_ui;
	godot::Label *fps_label;
	godot::Label *memory_label;

	godot::String label_string_static_part;
    bool is_replaying = false;
	bool in_debug = false;

	godot::Input *input_singleton = godot::Input::get_singleton(); //Input interface
	godot::InputMap *input_map_singleton = godot::InputMap::get_singleton(); //List of possible inputs

	godot::Ref<godot::PackedScene> label_scene = godot::ResourceLoader::get_singleton()->load("res://addons/replay_qol/input_label.tscn");

public:

    void update();

    void exit_replay();

    void force_pause_replay()
    {
        recorder->force_pause_replay();
    }
	void replay_trigger()
	{
        recorder->replay_pause_trigger();
	}
	void set_recorder(Recorder *new_recorder)
	{
		recorder = new_recorder;
	}
	Recorder *get_recorder()
	{
		return recorder;
	}
		void set_time_line_slider(godot::HSlider *new_slider)
	{
		time_line_slider = new_slider;
	}

	void set_frame(int frame)
	{
		recorder->force_set_replay_frame(frame);
	}

	bool get_replay_pause()
	{
		return recorder->get_replay_pause();
	}

	void set_frame_counter_ui(godot::Label *new_label)
	{
		frame_counter_ui = new_label;
	}

	void set_fps_label(godot::Label *new_label)
	{
		fps_label = new_label;
	}

	void set_memory_label(godot::Label *new_label)
	{
		memory_label = new_label;
	}

    void set_controls_popup(godot::PopupPanel*panel);

	void set_input_popup(godot::PopupPanel*panel);

	void set_debug_popup(godot::PopupPanel*panel);

	void set_input_lable_parent(godot::Control*control);


	void enable_debug_window()
	{
		in_debug = !in_debug;
	}
};