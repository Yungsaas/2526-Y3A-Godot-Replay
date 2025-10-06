#include "godot_cpp/classes/wrapped.hpp"
#include "recorder.hpp"

class InstantReplayRecorder:public Recorder
{
	// Make class usable in godot with gdscript
    GDCLASS(InstantReplayRecorder, Recorder)

    protected:
    static void _bind_methods();

    bool is_recording = true;
    void handle_recording() override; 

    private:
    int max_recording_length = 600; //600 frames as standart for maximum instant recording length

    void handle_shift();
    public:

    int get_min_record_frame() override;

    void set_max_recording_length(int new_length){
        max_recording_length = new_length;
    }
    int get_max_recording_length(){
        return max_recording_length;
    }
};