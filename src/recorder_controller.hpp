#pragma once
#include "recorder.hpp"

class Recorder_Controller:public godot::Node{
    // Make class usable in godot with gdscript
    GDCLASS(Recorder_Controller, Node)
    
    protected:
    // Bind c plus plus methods to gdscript class
    static void _bind_methods();

    private:
    Recorder* recorder;

    public:

    void set_recorder(Recorder* new_recorder){
        recorder=new_recorder;
    }
    Recorder* get_recorder(){
        return recorder;
    }

};