#pragma once
#include "recorder_controller.hpp"
#include "godot_cpp/classes/file_access.hpp"
#include "godot_cpp/classes/control.hpp"
#include "godot_cpp/classes/label.hpp"
#include "godot_cpp/classes/node.hpp"
#include "godot_cpp/classes/packed_scene.hpp"
#include "godot_cpp/classes/popup_panel.hpp"
#include "godot_cpp/core/print_string.hpp"
#include "godot_cpp/variant/vector2.hpp"
#include "instant_replay_recorder.hpp"
#include "recorder.hpp"

void Recorder_Controller::set_controls_popup(godot::PopupPanel *panel)
{
	controls_popup_panel = panel;
}

void Recorder_Controller::set_input_popup(godot::PopupPanel *panel)
{
	input_popup_panel = panel;
}

void Recorder_Controller::set_input_lable_parent(godot::Control *control)
{
	input_lable_parent = control;
}

void Recorder_Controller::set_bookmark_marker_scene(godot::PackedScene *scene)
{
    bookmark_marker_scene = scene;
}

void Recorder_Controller::set_bookmark_marker_container(godot::Control *container)
{
    bookmark_marker_container = container;
}

void Recorder_Controller::set_bookmark_info_popup(godot::PopupPanel *popup)
{
    bookmark_info_popup = popup;
}

void Recorder_Controller::set_bookmark_info_label(godot::Label *label)
{
    bookmark_info_label = label;
}

void Recorder_Controller::update()
{
	if (!recorder) {
		godot::print_error("Recorder has not been set, recorder controller will not work.");
		return;
	}

	if (recorder->get_general_replay_state()) //is replaying
	{
		if (!time_line_slider) {
			godot::print_error("Timeline slider has not been set, recorder controller will not work.");
			return;
		}
		if (!frame_counter_ui) {
			godot::print_error("Frame counter ui has not been set, recorder controller will not work.");
			return;
		}
		if (is_replaying) //recorder controller has been initialized for this replay and is currently replaying
		{
			if (recorder->get_replay_pause()) //replay is paused
			{
				frame_counter_ui->set_text(godot::String::num_int64(time_line_slider->get_value() - time_line_slider->get_min()) + label_string_static_part);
			} else //replay is playing
			{
				int replayFrame = recorder->get_replay_frame();
				time_line_slider->set_value(replayFrame);
				frame_counter_ui->set_text(godot::String::num_int64(time_line_slider->get_value() - time_line_slider->get_min()) + label_string_static_part);
			}

			if (!controls_popup_panel->is_visible()) {
				recorder->stop_replay();
				recorder->set_controlled_replay(false);
				is_replaying = false;
			}
		} else //recorder controller needs to be initialized
		{
			int recordingMin = recorder->get_min_record_frame(); //Get the first frame of the recording
			int recordingLength = recorder->get_recording_frame(); //Get the last frame of the recording
			time_line_slider->set_min(recordingMin); //Set the minimum value of the slider
			time_line_slider->set_max(recordingLength); //Set the maximum value of the slider

			label_string_static_part = "/" + godot::String::num_int64(time_line_slider->get_max() - time_line_slider->get_min()); //Set the max frame counter value

			is_replaying = true;
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

				add_bookmark("input", action_name);

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
}
void Recorder_Controller::exit_replay()
{
	recorder->stop_replay();
	is_replaying = false;
}

//bookmark
void Recorder_Controller::add_bookmark(godot::String event_type, godot::String event_data, int frame)
{
	if (!recorder) {
		godot::print_error("Cannot add bookmark: Recorder not set");
		return;
	}

	// If frame is -1, use current replay frame
	int bookmark_frame = (frame == -1) ? recorder->get_replay_frame() : frame;

	// Choose color based on event type
	godot::Color color;
	if (event_type == "input") {
		color = godot::Color(0, 1, 0, 1); // Green
	} else if (event_type == "spawn") {
		color = godot::Color(0, 0, 1, 1); // Blue
	} else if (event_type == "destroy") {
		color = godot::Color(1, 0, 0, 1); // Red
	} else if (event_type == "animation") {
		color = godot::Color(1, 1, 0, 1); // Yellow
	} else if (event_type == "audio") {
		color = godot::Color(1, 0, 1, 1); // Magenta
	} else {
		color = godot::Color(1, 1, 1, 1); // White for unknown
	}

	Bookmark new_bookmark(bookmark_frame, event_type, event_data, color);
	bookmarks.push_back(new_bookmark);

	godot::print_line("Bookmark added: " + event_type + " at frame " + godot::String::num_int64(bookmark_frame));

	update_bookmark_markers();
}

void Recorder_Controller::remove_bookmark(int index)
{
    if (index >= 0 && index < bookmarks.size()) {
        bookmarks.remove_at(index);
        godot::print_line("Bookmark removed at index " + godot::String::num_int64(index));

		update_bookmark_markers();
    } else {
        godot::print_error("Invalid bookmark index: " + godot::String::num_int64(index));
    }
}

void Recorder_Controller::clear_bookmarks()
{
    bookmarks.clear();
    godot::print_line("All bookmarks cleared");
}

int Recorder_Controller::get_bookmark_count()
{
    return bookmarks.size();
}

Bookmark Recorder_Controller::get_bookmark(int index)
{
    if (index >= 0 && index < bookmarks.size()) {
        return bookmarks[index];
    }
    
    godot::print_error("Invalid bookmark index: " + godot::String::num_int64(index));
    return Bookmark(); // Return empty bookmark
}

void Recorder_Controller::jump_to_bookmark(int bookmark_index)
{
    if (!recorder) {
        godot::print_error("Cannot jump to bookmark: Recorder not set");
        return;
    }
    
    if (!is_replaying) {
        godot::print_error("Cannot jump to bookmark: Not currently replaying");
        return;
    }
    
    if (bookmark_index < 0 || bookmark_index >= bookmarks.size()) {
        godot::print_error("Invalid bookmark index: " + godot::String::num_int64(bookmark_index));
        return;
    }
    
    Bookmark bookmark = bookmarks[bookmark_index];
    
    // Update the timeline slider
    time_line_slider->set_value(bookmark.frame);
    
    // Update the frame counter
    frame_counter_ui->set_text(godot::String::num_int64(bookmark.frame - time_line_slider->get_min()) + label_string_static_part);
    
    // Tell the recorder to jump to this frame
    recorder->set_replay_frame(bookmark.frame);
    
    godot::print_line("Jumped to bookmark: " + bookmark.event_type + " (" + bookmark.event_data + ") at frame " + godot::String::num_int64(bookmark.frame));
}

void Recorder_Controller::update_bookmark_markers()
{
    if (!bookmark_marker_container) {
        godot::print_error("Bookmark marker container not set");
        return;
    }
    
    if (!time_line_slider) {
        godot::print_error("Timeline slider not set");
        return;
    }
    
    // Clear existing markers
    for (int i = 0; i < bookmark_marker_container->get_child_count(); i++) {
        bookmark_marker_container->get_child(i)->queue_free();
    }
    
    if (!bookmark_marker_scene) {
        godot::print_error("Bookmark marker scene not set");
        return;
    }
    
    // Get timeline dimensions
    float timeline_min = time_line_slider->get_min();
    float timeline_max = time_line_slider->get_max();
    float timeline_range = timeline_max - timeline_min;
    float timeline_width = time_line_slider->get_size().x;
    
    // Create a marker for each bookmark
    for (int i = 0; i < bookmarks.size(); i++) {
        Bookmark bookmark = bookmarks[i];
        
        // Calculate position on timeline
        float normalized_position = (bookmark.frame - timeline_min) / timeline_range;
        float x_position = normalized_position * timeline_width;
        
        // Instantiate marker
        godot::Node *marker_instance = bookmark_marker_scene->instantiate();
        godot::Control *marker_control = godot::Object::cast_to<godot::Control>(marker_instance);
        
        if (marker_control) {
            
            marker_control->set_position(godot::Vector2(x_position, 0));
            
            
            marker_control->set_modulate(bookmark.marker_color);
            
           
            marker_control->set_meta("bookmark_index", i);
            marker_control->set_meta("bookmark_frame", bookmark.frame);
            marker_control->set_meta("bookmark_type", bookmark.event_type);
            marker_control->set_meta("bookmark_data", bookmark.event_data);
            
            bookmark_marker_container->add_child(marker_control);
        }
    }
    
    godot::print_line("Updated " + godot::String::num_int64(bookmarks.size()) + " bookmark markers");
}

void Recorder_Controller::on_bookmark_marker_clicked(int bookmark_index)
{
    if (!is_replaying) {
        godot::print_error("Cannot jump to bookmark: Not currently replaying");
        return;
    }
    
    if (bookmark_index < 0 || bookmark_index >= bookmarks.size()) {
        godot::print_error("Invalid bookmark index from marker click: " + godot::String::num_int64(bookmark_index));
        return;
    }
    
    jump_to_bookmark(bookmark_index);
    
    recorder->force_pause_replay();
}

//bookmark label
void Recorder_Controller::show_bookmark_info(int bookmark_index, godot::Vector2 position)
{
    if (!bookmark_info_popup || !bookmark_info_label) {
        godot::print_error("Bookmark info popup or label not set");
        return;
    }
    
    if (bookmark_index < 0 || bookmark_index >= bookmarks.size()) {
        godot::print_error("Invalid bookmark index: " + godot::String::num_int64(bookmark_index));
        return;
    }
    
    Bookmark bookmark = bookmarks[bookmark_index];
    
    // Build info text
    godot::String info_text = "";
    info_text += "Type: " + bookmark.event_type + "\n";
    info_text += "Data: " + bookmark.event_data + "\n";
    info_text += "Frame: " + godot::String::num_int64(bookmark.frame);
    
    bookmark_info_label->set_text(info_text);
    
    // Position the popup near the marker
    bookmark_info_popup->set_position(position);
    bookmark_info_popup->set_visible(true);
}

void Recorder_Controller::hide_bookmark_info()
{
    if (bookmark_info_popup) {
        bookmark_info_popup->set_visible(false);
    }
}

//bookmark json

void Recorder_Controller::set_json_enabled(bool enabled)
{
    json_enabled = enabled;
}

void Recorder_Controller::save_bookmarks_to_json()
{
    if (!json_enabled) {
        return;
    }
    
    godot::Array bookmark_array;
    
    // Convert all bookmarks to dictionary format
    for (int i = 0; i < bookmarks.size(); i++) {
        Bookmark bookmark = bookmarks[i];
        
        godot::Dictionary entry;
        entry["frame"] = bookmark.frame;
        entry["event_type"] = bookmark.event_type;
        entry["event_data"] = bookmark.event_data;
        
        // Store color as separate RGBA values
        entry["color_r"] = bookmark.marker_color.r;
        entry["color_g"] = bookmark.marker_color.g;
        entry["color_b"] = bookmark.marker_color.b;
        entry["color_a"] = bookmark.marker_color.a;
        
        bookmark_array.push_back(entry);
    }
    
    godot::Dictionary root;
    root["bookmark_count"] = bookmarks.size();
    root["bookmarks"] = bookmark_array;
    
    auto json_string = godot::JSON::stringify(root);
    
    // Find available filename
    int recording_index = 0;
    godot::String filename;
    
    while (true) {
        filename = "res://addons/replay_qol/json/bookmarks_" + godot::String::num(recording_index) + ".json";
        
        if (!godot::FileAccess::file_exists(filename)) {
            break; // found available filename
        }
        recording_index++;
    }
    
    auto file = godot::FileAccess::open(filename, godot::FileAccess::WRITE);
    
    if (file.is_valid()) {
        file->store_string(json_string);
        file->close();
        godot::print_line("Bookmarks saved to: " + filename);
    } else {
        godot::print_error("Failed to save bookmarks to: " + filename);
    }
}

void Recorder_Controller::load_bookmarks_from_json(godot::String filename)
{
    if (!godot::FileAccess::file_exists(filename)) {
        godot::print_error("Bookmark file does not exist: " + filename);
        return;
    }
    
    auto file = godot::FileAccess::open(filename, godot::FileAccess::READ);
    
    if (!file.is_valid()) {
        godot::print_error("Failed to open bookmark file: " + filename);
        return;
    }
    
    auto json_string = file->get_as_text();
    file->close();
    
    godot::JSON json;
    auto parse_error = json.parse(json_string);
    
    if (parse_error != godot::OK) {
        godot::print_error("Failed to parse bookmark JSON: " + filename);
        return;
    }
    
    auto json_data = json.get_data();
    godot::Dictionary root = json_data;
    
    // Clear existing bookmarks
    bookmarks.clear();
    
    godot::Array bookmark_array = root["bookmarks"];
    
    for (int i = 0; i < bookmark_array.size(); i++) {
        godot::Dictionary entry = bookmark_array[i];
        
        int frame = entry["frame"];
        godot::String event_type = entry["event_type"];
        godot::String event_data = entry["event_data"];
        
        // Reconstruct color
        godot::Color color(
            entry["color_r"],
            entry["color_g"],
            entry["color_b"],
            entry["color_a"]
        );
        
        Bookmark bookmark(frame, event_type, event_data, color);
        bookmarks.push_back(bookmark);
    }
    
    godot::print_line("Loaded " + godot::String::num_int64(bookmarks.size()) + " bookmarks from: " + filename);
    
    // Update visual markers after loading
    update_bookmark_markers();
}

void Recorder_Controller::_bind_methods()
{
	//Recorder setting and getting
	godot::ClassDB::bind_method(godot::D_METHOD("set_recorder", "recorder"), &Recorder_Controller::set_recorder);
	godot::ClassDB::bind_method(godot::D_METHOD("get_recorder"), &Recorder_Controller::get_recorder);

	godot::ClassDB::bind_method(godot::D_METHOD("set_time_line_slider", "slider"), &Recorder_Controller::set_time_line_slider);

	godot::ClassDB::bind_method(godot::D_METHOD("set_controls_popup_panel", "panel"), &Recorder_Controller::set_controls_popup);

	godot::ClassDB::bind_method(godot::D_METHOD("set_input_popup_panel", "panel"), &Recorder_Controller::set_input_popup);

	godot::ClassDB::bind_method(godot::D_METHOD("set_label_parent", "panel"), &Recorder_Controller::set_input_lable_parent);

	godot::ClassDB::bind_method(godot::D_METHOD("get_replay_paused"), &Recorder_Controller::get_replay_pause);

	godot::ClassDB::bind_method(godot::D_METHOD("set_frame", "frame"), &Recorder_Controller::set_frame);

	godot::ClassDB::bind_method(godot::D_METHOD("set_frame_counter_label", "label"), &Recorder_Controller::set_frame_counter_ui);

	//Functionality
	godot::ClassDB::bind_method(godot::D_METHOD("replay_trigger"), &Recorder_Controller::replay_trigger);

	godot::ClassDB::bind_method(godot::D_METHOD("force_pause_replay"), &Recorder_Controller::force_pause_replay);

	godot::ClassDB::bind_method(godot::D_METHOD("update"), &Recorder_Controller::update);

	godot::ClassDB::bind_method(godot::D_METHOD("add_bookmark", "event_type", "event_data", "frame"), &Recorder_Controller::add_bookmark, DEFVAL(-1));

    godot::ClassDB::bind_method(godot::D_METHOD("remove_bookmark", "index"), &Recorder_Controller::remove_bookmark);

    godot::ClassDB::bind_method(godot::D_METHOD("clear_bookmarks"), &Recorder_Controller::clear_bookmarks);

    godot::ClassDB::bind_method(godot::D_METHOD("get_bookmark_count"), &Recorder_Controller::get_bookmark_count);

	godot::ClassDB::bind_method(godot::D_METHOD("jump_to_bookmark", "bookmark_index"), &Recorder_Controller::jump_to_bookmark);

	godot::ClassDB::bind_method(godot::D_METHOD("set_bookmark_marker_scene", "scene"), &Recorder_Controller::set_bookmark_marker_scene);
	
	godot::ClassDB::bind_method(godot::D_METHOD("set_bookmark_marker_container", "container"), &Recorder_Controller::set_bookmark_marker_container);
	
	godot::ClassDB::bind_method(godot::D_METHOD("update_bookmark_markers"), &Recorder_Controller::update_bookmark_markers);

	godot::ClassDB::bind_method(godot::D_METHOD("on_bookmark_marker_clicked", "bookmark_index"), &Recorder_Controller::on_bookmark_marker_clicked);

	godot::ClassDB::bind_method(godot::D_METHOD("set_bookmark_info_popup", "popup"), &Recorder_Controller::set_bookmark_info_popup);

	godot::ClassDB::bind_method(godot::D_METHOD("set_bookmark_info_label", "label"), &Recorder_Controller::set_bookmark_info_label);
	
	godot::ClassDB::bind_method(godot::D_METHOD("show_bookmark_info", "bookmark_index", "position"), &Recorder_Controller::show_bookmark_info);
	
	godot::ClassDB::bind_method(godot::D_METHOD("hide_bookmark_info"), &Recorder_Controller::hide_bookmark_info);

	godot::ClassDB::bind_method(godot::D_METHOD("save_bookmarks_to_json"), &Recorder_Controller::save_bookmarks_to_json);
	
	godot::ClassDB::bind_method(godot::D_METHOD("load_bookmarks_from_json", "filename"), &Recorder_Controller::load_bookmarks_from_json);
	
	godot::ClassDB::bind_method(godot::D_METHOD("set_json_enabled", "enabled"), &Recorder_Controller::set_json_enabled);
}