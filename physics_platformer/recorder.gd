extends Temp_save_replay

# CTRL+K for start recording SHIFT+K for start replay
# CTRL+L for stop recording SHIFT+K for stop replay

var is_recording: bool = false
var is_replaying: bool = false

@export var nodes_array: Array[Node] = []

func _ready() -> void:
	set_tracked_nodes(nodes_array)
	pass

func _physics_process(_delta: float) -> void:
	
	#get input
	var start_recording_input := Input.is_action_pressed(&"start_recording")
	var stop_recording_input := Input.is_action_pressed(&"stop_recording")
	
	
	var start_replay_input := Input.is_action_pressed(&"start_replay")
	var stop_replay_input := Input.is_action_pressed(&"stop_replay")
	
	if start_recording_input and not is_recording:
		print("Start Recording")
		start_recording()
		is_recording = true
		
	if stop_recording_input and is_recording:
		print("Stop Recording")
		stop_recording()
		is_recording = false
		
	if start_replay_input and not is_recording:
		print("Start replaying")
		start_replay()
		
	if stop_replay_input and not is_recording:
		print("Stop replaying manually")
		stop_replay()
	
	update()
