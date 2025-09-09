@tool
extends EditorPlugin

@onready var add_button: Button = $Button
@onready var tree: Tree = $Tree

var nodes: Array[NodePath] = []  # store node paths

func _ready():
	tree.create_item() # root item

func _on_button_pressed() -> void:
	var editor = get_editor_interface()
	var selected = editor.get_selection().get_selected_nodes()
	if selected.size() > 0:
		var node = selected[0]
		var path = node.get_path()
		if not nodes.has(path):
			print("Node added in list")
		else:
			print("Node already in list")
			
func _add_to_tree(node: Node):
	var root_item = tree.get_root()
	var item = tree.create_item(root_item)
	item.set_text(0, node.name)
	item.set_metadata(0, node.get_path())
