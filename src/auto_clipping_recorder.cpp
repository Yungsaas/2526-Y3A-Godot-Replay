#pragma once
#include "auto_clipping_recorder.hpp"

void AutoClippingRecorder::handle_recording()
{
    if(!is_clipping)    //currently not clipping
    {
        trim_recording(); //trim the recording
    }

	record_input();

	record_position();

	record_custom_data();

	recording_frame++;
}

void AutoClippingRecorder::trim_recording() //same trimming as in instant replay
{
    int oldest_frame = recording_frame - buffer_size;
	if (oldest_frame < 0)
		return;

	// 3D positions
	auto range3 = temporary_data_map_3d_pos.equal_range(oldest_frame);
	temporary_data_map_3d_pos.erase(range3.first, range3.second);

	// 2D positions
	auto range2 = temporary_data_map_2d_pos.equal_range(oldest_frame);
	temporary_data_map_2d_pos.erase(range2.first, range2.second);

	// custom data
	auto rangec = temporary_data_map_custom_data.equal_range(oldest_frame);
	temporary_data_map_custom_data.erase(rangec.first, rangec.second);

	// input
	auto rangei = temporary_data_map_input.equal_range(oldest_frame);
	temporary_data_map_input.erase(rangei.first, rangei.second);
}

void AutoClippingRecorder::_bind_methods()
{
	godot::ClassDB::bind_method(godot::D_METHOD("clip_begin"), &AutoClippingRecorder::clip_begin);
	godot::ClassDB::bind_method(godot::D_METHOD("clip_end"), &AutoClippingRecorder::clip_end);
	
	//Buffer size
	godot::ClassDB::bind_method(godot::D_METHOD("set_buffer_size", "size"), &AutoClippingRecorder::set_buffer_size);
	godot::ClassDB::bind_method(godot::D_METHOD("get_buffer_size"), &AutoClippingRecorder::get_buffer_size);
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::INT, "clip_buffer_size"), "set_buffer_size", "get_buffer_size");

}