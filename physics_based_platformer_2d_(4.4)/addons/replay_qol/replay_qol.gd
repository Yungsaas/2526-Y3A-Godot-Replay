@tool
extends EditorPlugin

func _enter_tree() -> void:
	# Add recording key mappings
	var startReplayKey := "input/start_replay"
	if not ProjectSettings.has_setting(startReplayKey):
		ProjectSettings.set_setting(startReplayKey, true)
	
	var stopReplayKey := "input/stop_replay"
	if not ProjectSettings.has_setting(stopReplayKey):
		ProjectSettings.set_setting(startReplayKey, true)
		
	var startRecordKey := "input/start_recording"
	if not ProjectSettings.has_setting(startRecordKey):
		ProjectSettings.set_setting(startReplayKey, true)
		
	var stopRecordKey := "input/stop_recording"
	if not ProjectSettings.has_setting(stopRecordKey):
		ProjectSettings.set_setting(startReplayKey, true)
		
	var groupKey := "global_group/recording"
	if not ProjectSettings.has_setting(groupKey):
		ProjectSettings.set_setting(groupKey, true)
		ProjectSettings.add_property_info({"name": groupKey, "type":TYPE_BOOL, "usage": PROPERTY_USAGE_GROUP})

func _exit_tree() -> void:
	
	ProjectSettings.clear("global_group/recording")
