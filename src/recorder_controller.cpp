#pragma once
#include "recorder_controller.hpp"

void Recorder_Controller::_bind_methods()
{
    //Set recorder
	godot::ClassDB::bind_method(godot::D_METHOD("set_recorder", "recorder"), &Recorder_Controller::set_recorder);
	godot::ClassDB::bind_method(godot::D_METHOD("get_recorder"), &Recorder_Controller::get_recorder);
}