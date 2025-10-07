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

    void save_clip()
    {
        is_clipping = false;    //Set is_clipping to false to start trimming again
        save_all_to_json();     //Save the clip
        clear_all_temp_maps();  //Clear out all temporary data maps
    }

    bool is_clipping = false;
    int buffer_size = 300; //300 frames as standart buffer for clip recording
    int buffer_counter = 0;
    

    public:

    void clip_begin()   //User needs to engage clipping manually with this function
    {
        is_clipping = true;
    }
    void clip_end()     //User needs to end clipping manually with this function
    {
        buffer_counter = buffer_size;
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