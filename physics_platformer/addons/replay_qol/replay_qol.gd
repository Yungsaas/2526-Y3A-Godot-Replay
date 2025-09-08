@tool
extends EditorPlugin

var panel

const TOOL_PANEL = preload("res://addons/replay_qol/Record_Panel.tscn")

func _enter_tree() -> void:
	panel = TOOL_PANEL.instantiate()
	add_control_to_dock(EditorPlugin.DOCK_SLOT_LEFT_BL, panel)
	


func _exit_tree() -> void:
	remove_control_from_docks(panel)
	
	panel.queue_free()
	
