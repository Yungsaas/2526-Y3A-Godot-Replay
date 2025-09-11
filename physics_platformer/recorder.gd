extends Temp_save_replay

var is_recording: bool = false

func _ready() -> void:
	#add_node($"../Player")
	pass

func _physics_process(_delta: float) -> void:
	
	#get input
	var start_recording_input := Input.is_action_pressed(&"start_recording")
	var stop_recording_input := Input.is_action_pressed(&"stop_recording")
	
	if start_recording_input and not is_recording:
		print("Start Recording")
		start_recording()
		is_recording = true
		
	if stop_recording_input and is_recording:
		print("Stop Recording")
		stop_recording()
		is_recording = false
	
	update()
