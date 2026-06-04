extends Recorder_Controller


@onready var marker_scene = preload("res://addons/replay_qol/bookmark_marker.tscn")
@onready var marker_container = $PopupPanel/Control/TimeLineSlider/Control

@onready var bookmark_info_panel = $BookmarkInfoPanel
@onready var bookmark_info_label = $BookmarkInfoPanel/BookmarkInfoLabel

func _ready() -> void:
	add_to_group("recorder_controller")
	
	set_bookmark_marker_scene(marker_scene)
	set_bookmark_marker_container(marker_container)
	
	set_bookmark_info_popup(bookmark_info_panel)
	set_bookmark_info_label(bookmark_info_label)
	
	set_filter_popup($FilterPopup)
	set_filter_parent($FilterPopup/Control)
	
	
	
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
	
	track_object_for_deletion()
	var spawn_parent = $".."
	var initial_objects = spawn_parent.get_children() 
	initialize_spawn_tracking(initial_objects)
	
	set_export_fps(60)
	set_export_output_path("user://my_replay.mp4")

func _physics_process(delta: float) -> void:
	update()
	var spawn_parent = $".."
	var current_objects = spawn_parent.get_children()
	check_for_spawns(current_objects)
 
func _on_play_stop_button_pressed() -> void:
	replay_trigger()

func _on_time_line_slider_drag_started() -> void:
	force_pause_replay()

func _on_time_line_slider_value_changed(value: float) -> void:
	if(get_replay_paused()):
		set_frame(value)
#



func _on_export_button_pressed() -> void:
	trigger_mp4_export()
