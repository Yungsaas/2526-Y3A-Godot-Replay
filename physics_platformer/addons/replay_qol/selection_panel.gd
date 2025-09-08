extends VBoxContainer

@onready var add_button: Button = $Button
@onready var tree: Tree = $Tree

var nodes: Array[NodePath] = []  # store node paths

func _ready():
	tree.create_item() # root item
	add_button.pressed.connect(_on_add_pressed)

func _on_add_pressed():
	var editor = get_tree().get_editor_interface()
	var selected = editor.get_selection().get_selected_nodes()
	if selected.size() > 0:
		var node = selected[0]
		var path = node.get_path()
		if not nodes.has(path):
			nodes.append(path)
			_add_to_tree(node)
		else:
			print("Node already in list")

func _add_to_tree(node: Node):
	var root_item = tree.get_root()
	var item = tree.create_item(root_item)
	item.set_text(0, node.name)
	item.set_metadata(0, node.get_path())
