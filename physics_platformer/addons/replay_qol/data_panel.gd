@tool
extends Control


func _on_input_pressed() -> void:
	$"Input Text".visible = true
	$"Position Text".visible = false
	$"Events Text".visible = false
	$"Console Text".visible = false


func _on_positions_pressed() -> void:
	$"Input Text".visible = false
	$"Position Text".visible = true
	$"Events Text".visible = false
	$"Console Text".visible = false


func _on_events_pressed() -> void:
	$"Input Text".visible = false
	$"Position Text".visible = false
	$"Events Text".visible = true
	$"Console Text".visible = false


func _on_console_pressed() -> void:
	$"Input Text".visible = false
	$"Position Text".visible = false
	$"Events Text".visible = false
	$"Console Text".visible = true
