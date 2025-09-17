@tool
extends EditorPlugin

var record_panel
const RECORD_PANEL = preload("res://addons/replay_qol/Record_Panel.tscn")

var selection_panel
const SELECTION_PANEL = preload("res://addons/replay_qol/Selection_Panel.tscn")

var data_panel
const DATA_PANEL = preload("res://addons/replay_qol/Data_Panel.tscn")

var data_panels:Data_Panels = Data_Panels.new()


func _enter_tree() -> void:
	record_panel = RECORD_PANEL.instantiate()
	add_control_to_bottom_panel(record_panel, "Record_Panel")
	
	selection_panel = SELECTION_PANEL.instantiate()
	selection_panel.editor_interface = get_editor_interface() # Pass editor interface to panel
	add_control_to_dock(DOCK_SLOT_RIGHT_UL, selection_panel)
	
	data_panel = DATA_PANEL.instantiate()
	add_control_to_dock(DOCK_SLOT_LEFT_UR, data_panel)
	
	data_panel.data_panels = data_panels
	data_panel.data_panels.add_position_screen(data_panel.position_text)
	data_panel.data_panels.add_input_screen(data_panel.input_text)
	
	var replay_node = get_tree().get_root().find_child("Recorder", true, false) as Temp_save_replay
	replay_node.set_input_screen(data_panel.input_text)
	print(replay_node.get_input_screen())
	ResourceSaver.save(replay_node.get_scene(), replay_node.get_scene().scene_file_path)
	
	
#	var edited_scene = get_editor_interface().get_edited_scene_root() #try to use this to instantiate a recorder instead of making one manualy
#	if edited_scene:
#		var recorder_new := edited_scene.get_node("Recorder") as Temp_save_replay
#		recorder_new.set_input_screen(data_panel.input_text)
	
	var groupKey := "global_group/recording"
	if not ProjectSettings.has_setting(groupKey):
		ProjectSettings.set_setting(groupKey, true)
		ProjectSettings.add_property_info({"name": groupKey, "type":TYPE_BOOL, "usage": PROPERTY_USAGE_GROUP})

func _exit_tree() -> void:
	remove_control_from_bottom_panel(record_panel)
	record_panel.queue_free()
	
	remove_control_from_docks(selection_panel)
	selection_panel.queue_free()
	
	remove_control_from_docks(data_panel)
	data_panel.queue_free()
	
	ProjectSettings.clear("global_group/recording")
