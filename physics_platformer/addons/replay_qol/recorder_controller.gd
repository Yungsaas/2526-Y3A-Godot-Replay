extends Recorder_Controller

func _ready() -> void:
	
	if(not $"../Recorder"):
		push_error("Recorder Controller could not find Recorder node!\nMake sure to add a Recorder node to your scene!")
	else:
		set_recorder($"../Recorder")
		print("Recorder in Recorder Controller has been set.")
		force_pause_replay()
		
	set_controls_popup_panel($PopupPanel)
	set_input_popup_panel($PopupPanel2)
	set_label_parent($PopupPanel2/Control2)
	set_time_line_slider($PopupPanel/Control/TimeLineSlider)
	set_frame_counter_label($PopupPanel/Control/ReplayFrameCounter)

func _physics_process(delta: float) -> void:
	update()

func _on_play_stop_button_pressed() -> void:
	replay_trigger()

func _on_time_line_slider_drag_started() -> void:
	force_pause_replay()

func _on_time_line_slider_value_changed(value: float) -> void:
	if(get_replay_paused()):
		set_frame(value)
#aw
