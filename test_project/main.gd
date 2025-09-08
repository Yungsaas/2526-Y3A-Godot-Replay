extends Node2D


func _ready() -> void:
	_custom()
	
	
func _custom() -> void:
	var my_class:Temp_save_replay = Temp_save_replay.new()
	my_class.add_node(%TestNode)
	my_class.add_node($AnimatedSprite2D)
	my_class.debug_print_array()
