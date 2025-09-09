@tool
extends Control

var recorder: Temp_save_replay
var is_recording: bool = false

func set_recorder(new_recorder: Temp_save_replay) -> void:
	recorder = new_recorder

func _on_button_pressed() -> void:
	if recorder:
		if is_recording:
			recorder.stop_recording()
			print("Stop recording.")
			recorder.debug_print_positions()
			is_recording = false
		else:
			recorder.start_recording()
			print("Start recording")
			recorder.debug_print_array()
			is_recording = true
	pass # Replace with function body.
