#pragma once
#include "recorder_controller.hpp"
#include "godot_cpp/classes/popup_panel.hpp"
#include "recorder.hpp"

void Recorder_Controller::set_controls_popup(godot::PopupPanel*panel)
{
	controls_popup_panel = panel;
}
void Recorder_Controller::update()
{
	if(recorder->get_general_replay_state())//is replaying
	{
		if(is_replaying)//recorder controller has been initialized for this replay and is currently replaying
		{
		}else //recorder controller needs to be initialized
		{
			int recordingLength = recorder->get_recording_frame();
			time_line_slider->set_max(recordingLength);
			current_recording_frame = 0;
			is_replaying=true;
			recorder->force_pause_replay();
		}
	}
	
}
void Recorder_Controller::exit_replay()
{
	recorder->stop_replay();
	is_replaying = false;
}

void Recorder_Controller::_bind_methods()
{
    //Recorder setting and getting
	godot::ClassDB::bind_method(godot::D_METHOD("set_recorder", "recorder"), &Recorder_Controller::set_recorder);
	godot::ClassDB::bind_method(godot::D_METHOD("get_recorder"), &Recorder_Controller::get_recorder);

	godot::ClassDB::bind_method(godot::D_METHOD("set_controls_popup_panel", "panel"), &Recorder_Controller::set_controls_popup);

	//Functionality
	godot::ClassDB::bind_method(godot::D_METHOD("replay_trigger"), &Recorder_Controller::replay_trigger);
	
	godot::ClassDB::bind_method(godot::D_METHOD("force_pause_replay"), &Recorder_Controller::force_pause_replay);
}