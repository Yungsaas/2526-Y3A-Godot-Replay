class_name Coin
extends Area2D

var taken := false

const WALK_ACCEL = 1000.0

func _on_body_enter(body: Node2D) -> void:
	if not taken and body is Player:
		($AnimationPlayer as AnimationPlayer).play("taken")
		

var speed := 10   # pixels per second

func _process(delta):
	position.x += speed * delta
	
