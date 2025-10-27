extends Ghost_Player_Manager

func _ready() -> void:
	set_player_node($"../Player")
	pass
	
func _physics_process(_delta: float) -> void:
	
	#get input
	var start_recording_input := Input.is_action_just_pressed(&"start_recording")
	var stop_recording_input := Input.is_action_just_pressed(&"stop_recording")
	
	
	var start_replay_input := Input.is_action_just_pressed(&"start_replay")
	var stop_replay_input := Input.is_action_just_pressed(&"stop_replay")
	
	if start_recording_input:
		print("start rec")
		start_ghost_recording()
	
	if stop_recording_input:
		stop_ghost_recording()

	if start_replay_input:
		print("start rep")
		load_from_memory()
		start_ghost_replay()

	if stop_replay_input:
		stop_ghost_replay()
		
	update()
	pass
