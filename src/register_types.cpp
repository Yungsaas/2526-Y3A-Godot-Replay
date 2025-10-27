// Include your classes, that you want to expose to Godot

#include "auto_clipping_recorder.hpp"
#include "ghost_player_manager.hpp"
#include "recorder.hpp"
#include "recorder_controller.hpp"
#include "selection_panels.hpp"
#include "instant_replay_recorder.hpp"
#include <gdextension_interface.h>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

using namespace godot;

void initialize_gdextension_types(ModuleInitializationLevel p_level)
{
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	// Register your classes here, so they are available in the Godot editor and engine
	GDREGISTER_CLASS(Recorder)
	GDREGISTER_CLASS(Data_Panels)
	GDREGISTER_CLASS(Recorder_Controller)
	GDREGISTER_CLASS(InstantReplayRecorder)
	GDREGISTER_CLASS(AutoClippingRecorder)
	GDREGISTER_CLASS(Ghost_Player_Manager)
}

void uninitialize_gdextension_types(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
}

extern "C"
{
	// Initialization
	GDExtensionBool GDE_EXPORT replay_qol_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization)
	{
		GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);
		init_obj.register_initializer(initialize_gdextension_types);
		init_obj.register_terminator(uninitialize_gdextension_types);
		init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);
		
		return init_obj.init();
	}
}