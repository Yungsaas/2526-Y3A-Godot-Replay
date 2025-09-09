@tool
extends EditorPlugin

var record_panel
const RECORD_PANEL = preload("res://addons/replay_qol/Record_Panel.tscn")

var selection_panel
const SELECTION_PANEL = preload("res://addons/replay_qol/Selection_Panel.tscn")

var data_panel
const DATA_PANEL = preload("res://addons/replay_qol/Data_Panel.tscn")

var recorder:Temp_save_replay = Temp_save_replay.new()

func _enter_tree() -> void:
	record_panel = RECORD_PANEL.instantiate()
	add_control_to_bottom_panel(record_panel, "Record_Panel")
	
	selection_panel = SELECTION_PANEL.instantiate()
	selection_panel.editor_interface = get_editor_interface() # Pass editor interface to panel
	add_control_to_dock(DOCK_SLOT_RIGHT_UL, selection_panel)
	
	data_panel = DATA_PANEL.instantiate()
	add_control_to_dock(DOCK_SLOT_LEFT_UR, data_panel)
	
	record_panel.set_recorder(recorder)
	selection_panel.set_recorder(recorder)


func _exit_tree() -> void:
	remove_control_from_bottom_panel(record_panel)
	record_panel.queue_free()
	
	remove_control_from_docks(selection_panel)
	selection_panel.queue_free()
	
	remove_control_from_docks(data_panel)
	data_panel.queue_free()
	
