#pragma once
#include "godot_cpp/classes/control.hpp"
#include "godot_cpp/classes/h_slider.hpp"
#include "godot_cpp/classes/label.hpp"
#include "godot_cpp/classes/packed_scene.hpp"
#include "godot_cpp/classes/popup_panel.hpp"
#include "recorder.hpp"
#include "godot_cpp/classes/resource_loader.hpp"

struct Bookmark {
    int frame;
    godot::String event_type;  // "input", "spawn", "destroy", etc.
    godot::String event_data;  // Description or additional info
    godot::Color marker_color;
    
    Bookmark() : frame(0), marker_color(godot::Color(1, 0, 0, 1)) {}
    
    Bookmark(int f, godot::String type, godot::String data, godot::Color color = godot::Color(1, 0, 0, 1))
        : frame(f), event_type(type), event_data(data), marker_color(color) {}
};

class Recorder_Controller : public godot::Node {
	// Make class usable in godot with gdscript
	GDCLASS(Recorder_Controller, Node)

protected:
	// Bind c plus plus methods to gdscript class
	static void _bind_methods();

private:
	Recorder *recorder;
    godot::PopupPanel *controls_popup_panel;
	godot::PopupPanel *input_popup_panel;
	godot::Control *input_lable_parent;
    godot::HSlider *time_line_slider;
	godot::Label *frame_counter_ui;
	godot::String label_string_static_part;
    bool is_replaying = false;

	godot::Input *input_singleton = godot::Input::get_singleton(); //Input interface
	godot::InputMap *input_map_singleton = godot::InputMap::get_singleton(); //List of possible inputs

	godot::Ref<godot::PackedScene> label_scene = godot::ResourceLoader::get_singleton()->load("res://addons/replay_qol/input_label.tscn");

	private:
    godot::Vector<Bookmark> bookmarks;
    godot::PackedScene *bookmark_marker_scene; // Scene for individual marker
    godot::Control *bookmark_marker_container;

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

    void set_controls_popup(godot::PopupPanel*panel);

	void set_input_popup(godot::PopupPanel*panel);

	void set_input_lable_parent(godot::Control*control);

	void add_bookmark(godot::String event_type, godot::String event_data, int frame = -1);

	void remove_bookmark(int index);

	void clear_bookmarks();

	int get_bookmark_count();

	void jump_to_bookmark(int bookmark_index);

	void set_bookmark_marker_scene(godot::PackedScene *scene);

    void set_bookmark_marker_container(godot::Control *container);

    void update_bookmark_markers();

	void on_bookmark_marker_clicked(int bookmark_index);

	Bookmark get_bookmark(int index);
};