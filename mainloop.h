#pragma once
#include "headers.h"

#include "capture.h"
#include "structs.h"
#include "debug.h"
#include "pipe.h"
#include "opencv_funcs.h"
#include "pixel_data.h"


class PixelData;

class MainLoop {
public:
    int screen_width;
    int screen_height;
    Config* config;
    int roi_size;
    int height;
    int width;
    int center_x;
    int center_y;
    int roi_half;
    RECT rect;
    uint8_t* pixels;

    PixelData pixel_data;
    PipeClient pipe_client;

    bool trigger_toggle;
    bool prev_state;
    bool flick_toggle;
    bool flick_prev_state;
    bool in_cross;
    VALO_GUNS current_gun;
    std::chrono::time_point<std::chrono::high_resolution_clock> last_fire_time;

    MainLoop();
    ~MainLoop();
    void start(LauncherPointers* launcher_pointers);

private:
    void shoot();
};