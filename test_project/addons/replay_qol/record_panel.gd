@tool
extends Control

var recorder: Recorder
var is_recording: bool = false

func set_recorder(new_recorder: Recorder) -> void:
	recorder = new_recorder

func _on_button_pressed() -> void:
	if recorder:
		if is_recording:
			recorder.stop_recording()
			print("Stopped recording.")
			is_recording = false
			$Button.text = "Record"
		else:
			recorder.start_recording()
			print("Started recording")
			recorder.debug_print_array()
			is_recording = true
			$Button.text = "Stop recording"
	pass # Replace with function body.

func _exit_tree() -> void:
	$Button.text = "Record"
	is_recording = false
