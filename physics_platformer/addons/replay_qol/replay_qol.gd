@tool
extends EditorPlugin

var record_panel
const RECORD_PANEL = preload("res://addons/replay_qol/Record_Panel.tscn")

var selection_panel
const SELECTION_PANEL = preload("res://addons/replay_qol/Selection_Panel.tscn")

func _enter_tree() -> void:
	record_panel = RECORD_PANEL.instantiate()
	add_control_to_bottom_panel(record_panel, "Record_Panel")
	
	selection_panel = SELECTION_PANEL.instantiate()
	add_control_to_dock(DOCK_SLOT_RIGHT_UL, selection_panel)
	


func _exit_tree() -> void:
	remove_control_from_bottom_panel(record_panel)
	record_panel.queue_free()
	
	remove_control_from_bottom_panel(selection_panel)
	selection_panel.queue_free()
	
