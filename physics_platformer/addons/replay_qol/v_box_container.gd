@tool
extends Control

var editor_interface  # Remove the type hint


@onready var add_button: Button = $VBoxContainer/Button
@onready var tree: Tree = $VBoxContainer/Tree
var nodes: Array[NodePath] = []

func _ready():
	tree.create_item()

func _on_button_pressed() -> void:
	if editor_interface == null:
		push_error("EditorInterface is null! Make sure plugin passes it correctly.")
		return

	var selection = editor_interface.get_selection().get_selected_nodes()
	if selection.size() > 0:
		var node = selection[0]
		var path = node.get_path()
		if not nodes.has(path):
			nodes.append(path)
			_add_to_tree(node)
			print("Node added in list")
		else:
			print("Node already in list")


func _add_to_tree(node: Node):
	var root_item = tree.get_root()
	var item = tree.create_item(root_item)
	item.set_text(0, node.name)
	item.set_metadata(0, node.get_path())
