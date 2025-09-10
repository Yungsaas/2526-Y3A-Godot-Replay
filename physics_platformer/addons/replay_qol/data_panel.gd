@tool
extends Control

var selection_panels: Selection_Panels

@onready var input_text: TextEdit = $"Input Text"
@onready var position_text: TextEdit = $"Position Text"

func _ready() -> void:
	selection_panels = Selection_Panels.new()
	
	selection_panels.add_input_screen(input_text)
	selection_panels.add_position_screen(position_text)


func _on_input_pressed() -> void:
	$"Input Text".visible = true
	$"Position Text".visible = false
	$"Events Text".visible = false
	$"Console Text".visible = false
	
	selection_panels.print_input()


func _on_positions_pressed() -> void:
	$"Input Text".visible = false
	$"Position Text".visible = true
	$"Events Text".visible = false
	$"Console Text".visible = false
	
	selection_panels.print_positions()


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
