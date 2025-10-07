#include "instant_replay_recorder.hpp"

void InstantReplayRecorder::handle_recording()
{
	handle_shift();
	record_input();

	record_position();

	record_custom_data();

	recording_frame++;
}

void InstantReplayRecorder::handle_shift()
{
	int oldest_frame = recording_frame - max_recording_length;
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

int InstantReplayRecorder::get_min_record_frame()
{
	int oldest_frame = recording_frame - max_recording_length;
	if (oldest_frame < 0){
		return 0;}

	return oldest_frame;
}

void InstantReplayRecorder::_bind_methods()
{
	godot::ClassDB::bind_method(godot::D_METHOD("set_max_recording_length", "length_in_frames"), &InstantReplayRecorder::set_max_recording_length);
	godot::ClassDB::bind_method(godot::D_METHOD("get_max_recording_length"), &InstantReplayRecorder::get_max_recording_length);
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::INT, "max_recording_length"), "set_max_recording_length", "get_max_recording_length");
}