@tool
extends EditorPlugin

var record_panel
const RECORD_PANEL = preload("res://addons/replay_qol/Record_Panel.tscn")

var selection_panel
const SELECTION_PANEL = preload("res://addons/replay_qol/Selection_Panel.tscn")

var data_panel
const DATA_PANEL = preload("res://addons/replay_qol/Data_Panel.tscn")

var recorder:Temp_save_replay = Temp_save_replay.new()
var selection_panels:Selection_Panels = Selection_Panels.new()


func _enter_tree() -> void:
	record_panel = RECORD_PANEL.instantiate()
	add_control_to_bottom_panel(record_panel, "Record_Panel")
	
	selection_panel = SELECTION_PANEL.instantiate()
	selection_panel.editor_interface = get_editor_interface() # Pass editor interface to panel
	add_control_to_dock(DOCK_SLOT_RIGHT_UL, selection_panel)
	
	data_panel = DATA_PANEL.instantiate()
	add_control_to_dock(DOCK_SLOT_LEFT_UR, data_panel)
	
	selection_panels.set_replay_ptr(recorder)
	
	data_panel.selection_panels = selection_panels
	data_panel.selection_panels.add_position_screen(data_panel.position_text) #naming is incorrect, need to change it to data_panel.data_panels or smthing like that
	data_panel.selection_panels.add_input_screen(data_panel.input_text)
	
	var panel_Button = record_panel.get_node("Button") as Button
	panel_Button.pressed.connect(self._on_panel_button_pressed)

#create a button variable in recorder.gd and make it equal to the one in the recorder_panel

func _on_panel_button_pressed() -> void:
	var edited_scene = get_editor_interface().get_edited_scene_root() #try to use this to instantiate a recorder instead of making one manualy
	if edited_scene:
		var recorder := edited_scene.get_node("Recorder") as Temp_save_replay
		recorder.is_recording = true

func _physics_process(delta: float) -> void:
	recorder.update()

func _get_recorder()-> Temp_save_replay:
	
	return recorder

func _exit_tree() -> void:
	remove_control_from_bottom_panel(record_panel)
	record_panel.queue_free()
	
	remove_control_from_docks(selection_panel)
	selection_panel.queue_free()
	
	remove_control_from_docks(data_panel)
	data_panel.queue_free()
