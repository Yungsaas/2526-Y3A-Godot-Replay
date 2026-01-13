extends ColorRect

signal marker_clicked(bookmark_index: int)

func _ready():
	gui_input.connect(_on_gui_input)
	mouse_entered.connect(_on_mouse_entered)
	mouse_exited.connect(_on_mouse_exited)

func _on_gui_input(event: InputEvent):
	if event is InputEventMouseButton:
		if event.button_index == MOUSE_BUTTON_LEFT and event.pressed:
			var bookmark_index = get_meta("bookmark_index", -1)
			if bookmark_index >= 0:
				var controller = get_tree().get_first_node_in_group("recorder_controller")
				if controller:
					controller.on_bookmark_marker_clicked(bookmark_index)

func _on_mouse_entered():
	scale = Vector2(1.2, 1.2)
	
	var bookmark_index = get_meta("bookmark_index", -1)
	if bookmark_index >= 0:
		var controller = get_tree().get_first_node_in_group("recorder_controller")
		if controller:
			var popup_position = global_position + Vector2(0, -20)
			controller.show_bookmark_info(bookmark_index, popup_position)       

func _on_mouse_exited():
	scale = Vector2(1.0, 1.0)
	var controller = get_tree().get_first_node_in_group("recorder_controller")
	if controller:
		controller.hide_bookmark_info()
