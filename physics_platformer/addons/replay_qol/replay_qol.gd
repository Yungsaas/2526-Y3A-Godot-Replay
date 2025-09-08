@tool
extends EditorPlugin

var button : Button

func _enter_tree() -> void:
	button = Button.new()
	button.text = "My tool button"
	
	add_control_to_dock(EditorPlugin.DOCK_SLOT_LEFT_BL, button)


func _exit_tree() -> void:
	remove_control_from_docks(button)
	
	button.queue_free()
