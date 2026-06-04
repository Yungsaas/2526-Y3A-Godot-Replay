#pragma once
#include "recorder_controller.hpp"
#include "godot_cpp/classes/canvas_item.hpp"
#include "godot_cpp/classes/check_box.hpp"
#include "godot_cpp/classes/file_access.hpp"
#include "godot_cpp/classes/control.hpp"
#include "godot_cpp/classes/label.hpp"
#include "godot_cpp/classes/node.hpp"
#include "godot_cpp/classes/object.hpp"
#include "godot_cpp/classes/packed_scene.hpp"
#include "godot_cpp/classes/popup_panel.hpp"
#include "godot_cpp/classes/scene_tree.hpp"
#include "godot_cpp/core/print_string.hpp"
#include "godot_cpp/variant/string.hpp"
#include "godot_cpp/variant/variant.hpp"
#include "godot_cpp/variant/vector2.hpp"
#include "instant_replay_recorder.hpp"
#include "recorder.hpp"
#include "godot_cpp/templates/hash_set.hpp"

void Recorder_Controller::set_controls_popup(godot::PopupPanel *panel)
{
	controls_popup_panel = panel;
}

void Recorder_Controller::set_input_popup(godot::PopupPanel *panel)
{
	input_popup_panel = panel;
}

void Recorder_Controller::set_filter_popup(godot::PopupPanel * panel)
{
    filter_popup_panel = panel;
}

void Recorder_Controller::set_input_lable_parent(godot::Control *control)
{
	input_lable_parent = control;
}

void Recorder_Controller::set_filter_lable_parent(godot::Control *control)
{
	filter_lable_parent = control;
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

	// While an MP4 export is running, the recorder drives the replay itself, so
	// skip the normal replay UI handling and just refresh the status label.
	if (recorder->is_export_active()) {
		if (export_status_label) {
			export_status_label->set_text(recorder->get_export_status());
		}
		was_exporting = true;
		return;
	} else if (was_exporting) {
		// Export just finished: show the final result line once.
		if (export_status_label) {
			export_status_label->set_text(recorder->get_export_status());
		}
		was_exporting = false;
	}

	check_tracked_objects();
	
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
			filter_popup_panel->set_visible(true);
			recorder->set_controlled_replay(true);
			recorder->force_pause_replay();

            tracked_nodes = get_recorder()->get_tracked_nodes();
			//filer types
			auto collectedNodeTypes = CollectNodeTypes(tracked_nodes);

			long long typesSize = collectedNodeTypes.size();
			godot::print_line(typesSize);

			for (int i = 0; i < collectedNodeTypes.size(); i++) {
				Node *filter_instance = typeCheck_scene->instantiate();

				godot::Label *label = godot::Object::cast_to<godot::Label>(filter_instance);

				label->set_text(collectedNodeTypes[i]);
                godot::Vector2 pos = label->get_position();
				pos.y += i * 30;
				label->set_position(pos);

				filter_lable_parent->add_child(filter_instance);
			}
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


        for (auto node : tracked_nodes) 
        {
            Object *obj = node;
            godot::String class_name = obj->get_class();

            for (int i = 0; i < filter_lable_parent->get_child_count(); i++) 
            {
				godot::Node *child = filter_lable_parent->get_child(i);
                godot::Label *label = godot::Object::cast_to<godot::Label>(child);

				godot::Node *checkBoxNode = child->get_child(0);
				godot::CheckBox *checkBox = godot::Object::cast_to<godot::CheckBox>(checkBoxNode);

                if (class_name == label->get_text()) 
                {
                    godot::CanvasItem *canvas = godot::Object::cast_to<godot::CanvasItem>(node);
                    bool isVisible = !checkBox->is_pressed();
                    canvas->set_visible(isVisible);
                
                }
			}
        
        }
	}
}
void Recorder_Controller::exit_replay()
{
	recorder->stop_replay();
	is_replaying = false;
}

// ===== MP4 export ===========================================================
void Recorder_Controller::trigger_mp4_export()
{
	if (!recorder) {
		godot::print_error("Cannot export: recorder not set");
		return;
	}

	if (recorder->is_export_active()) {
		godot::print_line("Export already in progress, ignoring request");
		return;
	}

	// If we are currently reviewing a replay, hand control over to the export.
	// We deliberately do NOT call stop_replay() here: that would clear the
	// destroyed-node snapshots, and the export wants them so destroyed objects
	// still show up in the video. start_mp4_export() resets the replay frame to 0
	// and drives playback itself; finish_export() clears the snapshots afterwards.
	if (recorder->get_general_replay_state()) {
		recorder->set_controlled_replay(false);
		is_replaying = false;
	}

	// Hide the replay overlays so they do not end up in the captured frames.
	// (If you capture a dedicated SubViewport via set_capture_viewport(), the UI
	// is never captured anyway, but hiding them keeps the default window capture
	// clean too.)
	if (controls_popup_panel) {
		controls_popup_panel->set_visible(false);
	}
	if (input_popup_panel) {
		input_popup_panel->set_visible(false);
	}
	if (filter_popup_panel) {
		filter_popup_panel->set_visible(false);
	}

	if (export_status_label) {
		export_status_label->set_text("Starting export...");
	}

	recorder->start_mp4_export(export_target_fps, export_output_path);
}

//bookmark
void Recorder_Controller::add_bookmark(godot::String event_type, godot::String event_data, int frame)
{
	if (!recorder) {
		godot::print_error("Cannot add bookmark: Recorder not set");
		return;
	}

	// If frame is -1, use current replay frame
	int bookmark_frame;
    if (frame == -1) {
        if (event_type == "destroy") {
            bookmark_frame = recorder->get_recording_frame();
        } else {
            bookmark_frame = recorder->get_replay_frame();
        }
    } else {
        bookmark_frame = frame;
    }

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

void Recorder_Controller::track_object_for_deletion()
{
    if (!recorder) {
        godot::print_error("Recorder not set");
        return;
    }
    
    godot::Array tracked_nodes = recorder->get_tracked_nodes();

    if (tracked_nodes.is_empty()) 
    {
        godot::print_line("No nodes to track");
        return;
    }

    godot::print_line("Starting to track " + godot::String::num_int64(tracked_nodes.size()) + " nodes");

    for (int i = 0; i < tracked_nodes.size(); i++) 
    {
        godot::Variant node_variant = tracked_nodes[i];
        
        // Check if it's actually an object
        if (node_variant.get_type() != godot::Variant::OBJECT) {
            godot::print_error("Tracked item at index " + godot::String::num_int64(i) + " is not an object");
            continue;
        }
        
        godot::Object *obj = node_variant;
        godot::Node *node = godot::Object::cast_to<godot::Node>(obj);
        
        if (!node) {
            godot::print_error("Could not cast to Node at index " + godot::String::num_int64(i));
            continue;
        }
        
        // Create a fresh dictionary for THIS node
        godot::Dictionary tracked_data;
        tracked_data["node"] = node;
        tracked_data["name"] = node->get_name();
        tracked_data["id"] = node->get_instance_id();

        tracked_objects.push_back(tracked_data);

        godot::print_line("Now tracking: " + node->get_name());
    }
    
    godot::print_line("Finished tracking setup. Total tracked: " + godot::String::num_int64(tracked_objects.size()));
}

void Recorder_Controller::check_tracked_objects()
{
	if (recorder->get_tracked_nodes().is_empty()) 
	{
		godot::print_error("Cannot track null node");
        return;
	}
    for (int i = tracked_objects.size() - 1; i >= 0; i--) 
	{
        godot::Dictionary tracked_data = tracked_objects[i];
        godot::Object *obj = tracked_data["node"];
        
        // Check if object is null or queued for deletion
        if (!obj || obj->is_queued_for_deletion()) {
            godot::String object_name = tracked_data["name"];
            
            // Add bookmark for destruction
            add_bookmark("destroy", object_name);
            
            // Remove from tracking list
            tracked_objects.remove_at(i);
            
            godot::print_line("Detected destruction: " + object_name);
        }
    }
}

void Recorder_Controller::initialize_spawn_tracking(godot::Array objects)
{
    tracked_spawn_objects.clear();
    
    // Store all initial object IDs
    for (int i = 0; i < objects.size(); i++) {
        godot::Object *obj = objects[i];
        godot::Node *node = godot::Object::cast_to<godot::Node>(obj);
        
        if (node) {
            tracked_spawn_objects.push_back(node->get_instance_id());
        }
    }
    
    godot::print_line("Spawn tracking initialized with " + godot::String::num_int64(tracked_spawn_objects.size()) + " objects");
}

void Recorder_Controller::check_for_spawns(godot::Array current_objects)
{
    // Check each object in current array
    for (int i = 0; i < current_objects.size(); i++) {
        godot::Object *obj = current_objects[i];
        godot::Node *node = godot::Object::cast_to<godot::Node>(obj);
        
        if (!node) continue;
        
        int64_t node_id = node->get_instance_id();
        
        // Check if this ID exists in our tracked array
        bool is_new = true;
        for (int j = 0; j < tracked_spawn_objects.size(); j++) {
            int64_t tracked_id = tracked_spawn_objects[j];
            if (tracked_id == node_id) {
                is_new = false;
                break;
            }
        }
        
        // If it's new, add bookmark and add to tracked array
        if (is_new) {
            godot::String node_name = node->get_name();
            add_bookmark("spawn", node_name);
            tracked_spawn_objects.push_back(node_id);
            
            godot::print_line("New spawn detected: " + node_name);
        }
    }
}

godot::Array Recorder_Controller::CollectNodeTypes(const godot::Array &variantsArray)
{
    godot::HashSet<godot::String> seen;
    godot::Array result;

    for (int i = 0; i < variantsArray.size(); i++) {
        godot::Variant v = variantsArray[i];
        if (v.get_type() != godot::Variant::OBJECT) continue;

        Object *obj = v;
        if (!obj) continue;

        godot::String class_name = obj->get_class();
        if (!result.has(class_name)) {
            result.append(class_name);
        }
    }


    return result;
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

	godot::ClassDB::bind_method(godot::D_METHOD("track_object_for_deletion"), &Recorder_Controller::track_object_for_deletion);
	
	godot::ClassDB::bind_method(godot::D_METHOD("check_tracked_objects"), &Recorder_Controller::check_tracked_objects);

	godot::ClassDB::bind_method(godot::D_METHOD("initialize_spawn_tracking", "objects"), &Recorder_Controller::initialize_spawn_tracking);

	godot::ClassDB::bind_method(godot::D_METHOD("check_for_spawns", "current_objects"), &Recorder_Controller::check_for_spawns);

    godot::ClassDB::bind_method(godot::D_METHOD("set_filter_popup", "popup"), &Recorder_Controller::set_filter_popup);
    
    godot::ClassDB::bind_method(godot::D_METHOD("set_filter_parent", "panel"), &Recorder_Controller::set_filter_lable_parent);

	// MP4 export
	godot::ClassDB::bind_method(godot::D_METHOD("trigger_mp4_export"), &Recorder_Controller::trigger_mp4_export);
	godot::ClassDB::bind_method(godot::D_METHOD("set_export_status_label", "label"), &Recorder_Controller::set_export_status_label);
	godot::ClassDB::bind_method(godot::D_METHOD("set_export_fps", "fps"), &Recorder_Controller::set_export_fps);
	godot::ClassDB::bind_method(godot::D_METHOD("set_export_output_path", "path"), &Recorder_Controller::set_export_output_path);


}