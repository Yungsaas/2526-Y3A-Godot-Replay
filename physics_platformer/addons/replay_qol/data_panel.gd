@tool
extends Control

var data_panels: Data_Panels

@onready var input_text: TextEdit
@onready var position_text: TextEdit

func register_ui(ui_node: Control) -> void:
	input_text = ui_node.get_node("Input Text")
	position_text = ui_node.get_node("Position Text")
	

func _ready() -> void:
	data_panels = Data_Panels.new()
	register_ui(self)

func _physics_process(delta: float) -> void:
	if not Engine.is_editor_hint():
		data_panels.check_input()

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
