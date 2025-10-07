#pragma once

#include "godot_cpp/classes/wrapped.hpp"
#include "recorder.hpp"

class AutoClippingRecorder:public Recorder
{
	// Make class usable in godot with gdscript
    GDCLASS(AutoClippingRecorder, Recorder)

    protected:
    static void _bind_methods();

    private:
    void handle_recording() override;
    void trim_recording();

    bool is_clipping = false;
    int buffer_size = 300; //300 frams as standart buffer for clip recording

    public:

    void save_clip();
};