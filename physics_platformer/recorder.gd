extends Temp_save_replay

var is_recording: bool = false

func _ready() -> void:
	add_node($"../Player")
	pass

func _physics_process(_delta: float) -> void:
	
	#get input
	var recorder_pressed := Input.is_action_pressed(&"recorder_button")
	if recorder_pressed:
		if is_recording:
			stop_recording()
			is_recording = false
		if not is_recording:
			start_recording()
			is_recording = true
	
	update()
