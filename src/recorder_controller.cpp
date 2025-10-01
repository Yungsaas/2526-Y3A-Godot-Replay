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
			if(recorder->get_replay_pause())//replay is paused
			{

			}else //replay is playing
			{
				time_line_slider->set_value(recorder->get_replay_frame());
			}

			if(!controls_popup_panel->is_visible())
			{
				recorder->stop_replay();
				recorder->set_controlled_replay(false);
				is_replaying = false;
			}
		}else //recorder controller needs to be initialized
		{
			int recordingMin = 0; //If clipping and/or instant replay become a thing this should be set to their starting frame
			int recordingLength = recorder->get_recording_frame();
			time_line_slider->set_min(recordingMin);
			time_line_slider->set_max(recordingLength);
			current_recording_frame = 0;
			is_replaying=true;
			controls_popup_panel->set_visible(true);
			recorder->set_controlled_replay(true);
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

	godot::ClassDB::bind_method(godot::D_METHOD("set_time_line_slider", "slider"), &Recorder_Controller::set_time_line_slider);

	godot::ClassDB::bind_method(godot::D_METHOD("set_controls_popup_panel", "panel"), &Recorder_Controller::set_controls_popup);

	//Functionality
	godot::ClassDB::bind_method(godot::D_METHOD("replay_trigger"), &Recorder_Controller::replay_trigger);
	
	godot::ClassDB::bind_method(godot::D_METHOD("force_pause_replay"), &Recorder_Controller::force_pause_replay);
	
	godot::ClassDB::bind_method(godot::D_METHOD("update"), &Recorder_Controller::update);

	godot::ClassDB::bind_method(godot::D_METHOD("get_replay_paused"), &Recorder_Controller::get_replay_pause);
	
	godot::ClassDB::bind_method(godot::D_METHOD("set_frame", "frame"), &Recorder_Controller::set_frame);
}