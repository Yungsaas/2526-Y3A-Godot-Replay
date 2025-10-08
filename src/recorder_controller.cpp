#pragma once
#include "recorder_controller.hpp"
#include "godot_cpp/classes/control.hpp"
#include "godot_cpp/classes/label.hpp"
#include "godot_cpp/classes/node.hpp"
#include "godot_cpp/classes/object.hpp"
#include "godot_cpp/classes/packed_scene.hpp"
#include "godot_cpp/classes/popup_panel.hpp"
#include "godot_cpp/classes/resource_loader.hpp"
#include "godot_cpp/core/print_string.hpp"
#include "godot_cpp/variant/string.hpp"
#include "godot_cpp/variant/vector2.hpp"
#include "godot_cpp/classes/performance.hpp"
#include "recorder.hpp"
#include "instant_replay_recorder.hpp"
#include <godot_cpp/classes/os.hpp>

void Recorder_Controller::set_controls_popup(godot::PopupPanel*panel)
{
	controls_popup_panel = panel;
}

void Recorder_Controller::set_input_popup(godot::PopupPanel*panel)
{
	input_popup_panel = panel;
}

void Recorder_Controller::set_debug_popup(godot::PopupPanel*panel)
{
	debug_popup_panel = panel;
}

void Recorder_Controller::set_input_lable_parent(godot::Control*control)
{
	input_lable_parent = control;
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
				frame_counter_ui->set_text(godot::String::num_int64(time_line_slider->get_value() - recorder->get_max_recording_length()) + label_string_static_part);
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
			if(recordingMin>0)
			{
				label_string_static_part = "/" + godot::String::num_int64(recordingLength - recorder->get_max_recording_length());
			}else{
			label_string_static_part = "/" + godot::String::num_int64(recordingLength);}
			controls_popup_panel->set_visible(true);
			input_popup_panel->set_visible(true);
			recorder->set_controlled_replay(true);
			recorder->force_pause_replay();
		}

		godot::Array actions = input_map_singleton->get_actions();

		for (int i = 0; i < actions.size(); i++) {
			godot::StringName action_name = actions[i];
			if (action_name == godot::StringName("start_recording") || action_name == godot::StringName("stop_recording") || action_name == godot::StringName("start_replay") || action_name == godot::StringName("stop_replay")) {
				continue;
			}

			if (input_singleton->is_action_just_pressed(action_name)) {
				if (label_scene.is_null()) {
					godot::print_line("Failed to load MyLabel.tscn!");
					return;
				}

				for (int i = 0; i < input_lable_parent->get_child_count(); i++) {
					godot::Node *child = input_lable_parent->get_child(i);
					godot::Label *label = godot::Object::cast_to<godot::Label>(child);

					godot::Vector2 pos = label->get_position();
					pos.y += 30;
					label->set_position(pos);

					int times_moved = label->get_meta("times_moved", 0);
					times_moved++;
					label->set_meta("times_moved", times_moved);

					if (times_moved >= 10) {
						label->queue_free();
					}
				}
				Node *label_instance = label_scene->instantiate();

				godot::Label *label = godot::Object::cast_to<godot::Label>(label_instance);
				label->set_text(action_name);
				label->set_meta("times_moved", 0);

				input_lable_parent->add_child(label_instance);
			}
		}
	}

	if (in_debug) 
	{
		debug_popup_panel->set_visible(true);

		double fps = godot::Performance::get_singleton()->get_monitor(godot::Performance::TIME_FPS);
		double process_time_ms = godot::Performance::get_singleton()->get_monitor(godot::Performance::TIME_PROCESS) * 1000.0;

		godot::String text = "FPS: " + godot::String::num(fps, 1) + " (Process: " + godot::String::num(process_time_ms, 2) + " ms)";
		fps_label->set_text(text);

		double memory_bytes = godot::Performance::get_singleton()->get_monitor(godot::Performance::MEMORY_STATIC);
		double memory_mb = memory_bytes / (1024.0 * 1024.0);
		godot::String memory_text = "Memory: " + godot::String::num(memory_mb, 1) + " MB";
		memory_label->set_text(memory_text);
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
	
	godot::ClassDB::bind_method(godot::D_METHOD("set_input_popup_panel", "panel"), &Recorder_Controller::set_input_popup);
	
	godot::ClassDB::bind_method(godot::D_METHOD("set_debug_popup_panel", "panel"), &Recorder_Controller::set_debug_popup);

	godot::ClassDB::bind_method(godot::D_METHOD("set_label_parent", "panel"), &Recorder_Controller::set_input_lable_parent);
	
	godot::ClassDB::bind_method(godot::D_METHOD("set_fps_label", "label"), &Recorder_Controller::set_fps_label);
	
	godot::ClassDB::bind_method(godot::D_METHOD("set_memory_label", "label"), &Recorder_Controller::set_memory_label);
	
	godot::ClassDB::bind_method(godot::D_METHOD("get_replay_paused"), &Recorder_Controller::get_replay_pause);
	
	godot::ClassDB::bind_method(godot::D_METHOD("set_frame", "frame"), &Recorder_Controller::set_frame);
	
	godot::ClassDB::bind_method(godot::D_METHOD("set_frame_counter_label", "label"), &Recorder_Controller::set_frame_counter_ui);

	godot::ClassDB::bind_method(godot::D_METHOD("enable_debug_window"), &Recorder_Controller::enable_debug_window);

	//Functionality
	godot::ClassDB::bind_method(godot::D_METHOD("replay_trigger"), &Recorder_Controller::replay_trigger);
	
	godot::ClassDB::bind_method(godot::D_METHOD("force_pause_replay"), &Recorder_Controller::force_pause_replay);
	
	godot::ClassDB::bind_method(godot::D_METHOD("update"), &Recorder_Controller::update);

}