extends Recorder_Controller


@onready var marker_scene = preload("res://addons/replay_qol/bookmark_marker.tscn")
@onready var marker_container = $PopupPanel/Control/TimeLineSlider/Control

@onready var bookmark_info_popup = $BookmarkInfoPopup
@onready var bookmark_info_label = $BookmarkInfoPopup/BookmarkInfoLabel

func _ready() -> void:
	add_to_group("recorder_controller")
	
	set_bookmark_marker_scene(marker_scene)
	set_bookmark_marker_container(marker_container)
	
	set_bookmark_info_popup(bookmark_info_popup)
	set_bookmark_info_label(bookmark_info_label)
	
	if(not $"../Recorder"):
		if(not $"../InstantReplayRecorder"):
			push_error("Recorder Controller could not find Recorder node!\nMake sure to add a Recorder node to your scene!")
		else:
			print("Instant Replay Recorder has been added")
			set_recorder($"../InstantReplayRecorder")
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
func _on_test_button_pressed() -> void:
	if get_bookmark_count() > 0:
		jump_to_bookmark(0)  # Jump to first bookmark
	
		
