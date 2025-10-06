#pragma once
#include "recorder_controller.hpp"
#include "godot_cpp/classes/popup_panel.hpp"
#include "godot_cpp/core/print_string.hpp"
#include "recorder.hpp"
#include "instant_replay_recorder.hpp"

void Recorder_Controller::set_controls_popup(godot::PopupPanel*panel)
{
	controls_popup_panel = panel;
}
void Recorder_Controller::update()
{
	if(!recorder)
	{
		godot::print_error("Recorder has not been set, recorder controller will not work.");
		return;
	}

	if(recorder->get_general_replay_state())//is replaying
	{
		if(!time_line_slider)
		{
			godot::print_error("Timeline slider has not been set, recorder controller will not work.");
			return;
		}
		if(!frame_counter_ui)
		{
			godot::print_error("Frame counter ui has not been set, recorder controller will not work.");
			return;
		}
		if(is_replaying)//recorder controller has been initialized for this replay and is currently replaying
		{
			if(recorder->get_replay_pause())//replay is paused
			{
				frame_counter_ui->set_text( godot::String::num_int64(time_line_slider->get_value()) + label_string_static_part);
			}else //replay is playing
			{
				int replayFrame = recorder->get_replay_frame();
				time_line_slider->set_value(replayFrame);
				frame_counter_ui->set_text(godot::String::num_int64(time_line_slider->get_value()) + label_string_static_part);
			}

			if(!controls_popup_panel->is_visible())
			{
				recorder->stop_replay();
				recorder->set_controlled_replay(false);
				is_replaying = false;
			}
		}else //recorder controller needs to be initialized
		{
			int recordingMin = recorder->get_min_record_frame(); //If clipping and/or instant replay become a thing this should be set to their starting frame
			int recordingLength = recorder->get_recording_frame();
			time_line_slider->set_min(recordingMin);
			time_line_slider->set_max(recordingLength);
			int tempInt = recordingLength;
			is_replaying=true;
			label_string_static_part = "/" + godot::String::num_int64(recordingLength);
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
	
	godot::ClassDB::bind_method(godot::D_METHOD("get_replay_paused"), &Recorder_Controller::get_replay_pause);
	
	godot::ClassDB::bind_method(godot::D_METHOD("set_frame", "frame"), &Recorder_Controller::set_frame);
	
	godot::ClassDB::bind_method(godot::D_METHOD("set_frame_counter_label", "label"), &Recorder_Controller::set_frame_counter_ui);

	//Functionality
	godot::ClassDB::bind_method(godot::D_METHOD("replay_trigger"), &Recorder_Controller::replay_trigger);
	
	godot::ClassDB::bind_method(godot::D_METHOD("force_pause_replay"), &Recorder_Controller::force_pause_replay);
	
	godot::ClassDB::bind_method(godot::D_METHOD("update"), &Recorder_Controller::update);

}