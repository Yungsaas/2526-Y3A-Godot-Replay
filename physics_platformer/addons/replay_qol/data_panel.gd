@tool
extends Control

var data_panels: Data_Panels

@onready var input_text: TextEdit
@onready var position_text: TextEdit

func _on_input_pressed() -> void:
	$"Input Text".visible = true
	$"Position Text".visible = false
	$"Events Text".visible = false
	$"Console Text".visible = false
	
	data_panels.print_input()

func _on_positions_pressed() -> void:
	$"Input Text".visible = false
	$"Position Text".visible = true
	$"Events Text".visible = false
	$"Console Text".visible = false
	
	data_panels.print_positions()


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
