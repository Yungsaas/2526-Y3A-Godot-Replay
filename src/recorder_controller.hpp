#pragma once
#include "godot_cpp/classes/h_slider.hpp"
#include "godot_cpp/classes/popup_panel.hpp"
#include "recorder.hpp"

class Recorder_Controller : public godot::Node {
	// Make class usable in godot with gdscript
	GDCLASS(Recorder_Controller, Node)

protected:
	// Bind c plus plus methods to gdscript class
	static void _bind_methods();

private:
	Recorder *recorder;
    godot::PopupPanel *controls_popup_panel;
    godot::HSlider *time_line_slider;
    int current_recording_frame = 0;
    bool is_replaying = false;

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
    void set_controls_popup(godot::PopupPanel*panel);
};