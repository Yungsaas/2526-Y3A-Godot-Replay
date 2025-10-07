extends InstantReplayRecorder

# CTRL+K for start recording SHIFT+K for start replay
# CTRL+L for stop recording SHIFT+K for stop replay

var is_recording: bool = true
var is_replaying: bool = false

@export var nodes_array: Array[Node] = []
@export var json_path: JSON
@export var input_json_path: JSON

func _ready() -> void:
	set_tracked_nodes(nodes_array)
	
	set_json_path(json_path)
	set_input_json_path(input_json_path)
	
	load_json_file()
	start_recording()
	pass

func _on_btn_pressed() -> void:
	print(nodes_array[0])
	pass

func _physics_process(_delta: float) -> void:
	
	#get input
	var start_recording_input := Input.is_action_just_pressed(&"start_recording")
	var stop_recording_input := Input.is_action_just_pressed(&"stop_recording")
	
	
	var start_replay_input := Input.is_action_just_pressed(&"start_replay")
	var stop_replay_input := Input.is_action_just_pressed(&"stop_replay")
	

	if start_replay_input:
		stop_recording()
		print("Start replaying")
		start_replay()

	if stop_replay_input:
		print("Stop replaying manually")
		stop_replay()
		start_recording()
	
	#check_input()
	
	update()
