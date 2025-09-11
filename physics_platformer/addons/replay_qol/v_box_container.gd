@tool
extends Control

var editor_interface

var recorder: Temp_save_replay

@onready var add_button: Button = $VBoxContainer/AddButton
@onready var removeButton: Button = $VBoxContainer/Button2
@onready var tree: Tree = $VBoxContainer/Tree
var nodes: Array[NodePath] = []

func _ready():
	tree.create_item()

func _on_add_button_pressed() -> void:
	if editor_interface == null:
		push_error("EditorInterface is null! Make sure plugin passes it correctly.")
		return

	var selection = editor_interface.get_selection().get_selected_nodes()
	if selection.size() > 0:
		var node = selection[0]
		recorder.add_node(node)
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
	

func _on_button_2_pressed() -> void:
	
	var selection = editor_interface.get_selection().get_selected_nodes()
	if selection.size() > 0:
		var node = selection[0]
		recorder.remove_node(node)
	
	var selected_item = tree.get_selected()
	if selected_item:
		var path = selected_item.get_metadata(0)
		if path in nodes:
			nodes.erase(path)  # Remove from your array
			
			print("Node removed from list")
		else:
			print("Node not in list")
		
		# Remove from tree
		var parent = selected_item.get_parent()
		if parent:
			parent.remove_child(selected_item)
		else:
			tree.set_root(null)  # In case it’s the root
	else:
		print("No item selected")
