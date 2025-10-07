#pragma once

#include "godot_cpp/classes/wrapped.hpp"
#include "recorder.hpp"

class AutoClippingRecorder:public Recorder
{
	// Make class usable in godot with gdscript
    GDCLASS(AutoClippingRecorder, Recorder)

    protected:
    static void _bind_methods();
    bool is_recording = true;

    private:
    void handle_recording() override;
    void trim_recording();

    bool is_clipping = false;
    int buffer_size = 300; //300 frams as standart buffer for clip recording

    public:

    void clip_begin()
    {
        is_clipping = true;
    }
    void clip_end()
    {
        save_all_to_json();     //Save the clip
        is_clipping = false;    //Begin trimming
        clear_all_temp_maps();  //Clear out all temporary data maps
    }

    void set_buffer_size(int new_size)
    {
        buffer_size = new_size;
    }
    int get_buffer_size()
    {
        return buffer_size;
    }
};