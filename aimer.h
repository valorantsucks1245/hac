#pragma once
#include "headers.h"
#include "mainloop.h"

class MainLoop;

class Aimeling {
private:
    MainLoop* ml;
    void move_screen(const int x, int y, float min_speed, float max_speed, float distance_threshold);
public:
    Aimeling(MainLoop* main_loop);
    void aimeling_loop();
};