#include "mainloop.h"

#include "aimer.h"
#include "weapon_detection.h"

MainLoop::MainLoop() :
    trigger_toggle(false),
    prev_state(false),
    flick_toggle(false),
    flick_prev_state(false),
    in_cross(false),
    current_gun((VALO_GUNS)0),
    pixels(nullptr)
{
    last_fire_time = std::chrono::high_resolution_clock::now();
}

MainLoop::~MainLoop() {
    delete[] pixels;
}

void MainLoop::shoot() {
    if (config->trigger_config.mode != 0 || config->flickbot_config.mode != 0) {
        bool flick_pressed = GetAsyncKeyState(config->flickbot_config.key);

        if (config->trigger_config.mode == 1) {
            if (flick_pressed && !flick_prev_state)
                flick_toggle = !trigger_toggle;
        }
        else flick_toggle = flick_pressed;

        flick_prev_state = flick_pressed;

        bool trigger_pressed = GetAsyncKeyState(config->trigger_config.key) & 0x8000;

        if (config->trigger_config.mode == 1) {
            if (trigger_pressed && !prev_state)
                trigger_toggle = !trigger_toggle;
        }
        else {
            trigger_toggle = trigger_pressed;
        }
        prev_state = trigger_pressed;

        bool flick = ((config->flickbot_config.mode == 1 && flick_toggle) || (config->flickbot_config.mode == 2 && flick_pressed));
        bool trigger = ((config->trigger_config.mode == 1 && trigger_toggle) || (config->trigger_config.mode == 2 && trigger_pressed));

        if (flick || trigger) {
            in_cross = true;
            auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::high_resolution_clock::now() - last_fire_time).count();
            if (elapsed_time >= config->weapon_config.GUNS_SHOT_DELAYS[current_gun]) {
                pipe_client.left_click();
                //std::cout << current_gun << " : " << config->weapon_config.GUNS_SHOT_DELAYS[current_gun] << std::endl;
                last_fire_time = std::chrono::high_resolution_clock::now();
                Sleep(1);
            }
        }
    }
}

void MainLoop::start(LauncherPointers* launcher_pointers) {
    screen_width = GetSystemMetrics(SM_CXSCREEN);
    screen_height = GetSystemMetrics(SM_CYSCREEN);
    config = (Config*)launcher_pointers->config;
    roi_size = 200;
    height = roi_size;
    width = roi_size;
    center_x = width / 2;
    center_y = height / 2;
    roi_half = roi_size / 2;

    rect = {
        screen_width / 2 - roi_half, screen_height / 2 - roi_half,
        screen_width / 2 + roi_half, screen_height / 2 + roi_half
    };

    pixels = new uint8_t[width * height * 4]();

    ScreenCapturer screen_capturer = ScreenCapturer();
    screen_capturer.init(rect);

    pipe_client = PipeClient();
    pipe_client.connect("pipe187420");

    last_fire_time = std::chrono::high_resolution_clock::now();
    screen_capturer.capture_pixels(pixels);

    pixel_data = PixelData();
    pixel_data.init(pixels, width, height);

    Aimeling aimer = Aimeling(this);
    std::thread aimer_loop(&Aimeling::aimeling_loop, &aimer);
    aimer_loop.detach();

    WeaponDetection weapon_detector = WeaponDetection(this);
    std::thread weapon_detect_loop(&WeaponDetection::weapon_detection_loop, &weapon_detector);
    weapon_detect_loop.detach();
    int frames = 0;
    int fps = 0;
    auto last_time = std::chrono::high_resolution_clock::now();

    while (true) {
        screen_capturer.capture_pixels(pixels);
        pixel_data.create_bitmap();

        opencv_funcs::connect_nearby_pixels(pixel_data.bitmap, width, height, 3);
        opencv_funcs::connect_close_contours(pixel_data.bitmap, width, height, 3);

        pixel_data.check_center_pixels();
        if (pixel_data.is_in_cross() && GetAsyncKeyState(config->trigger_config.key)) {
            shoot();
        }

        frames++;
        auto current_time = std::chrono::high_resolution_clock::now();
        auto time_diff = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - last_time).count();

        if (time_diff >= 1000) {
            fps = frames;
            frames = 0;
            last_time = current_time;

            //on a decent pc, this loop should run at around what the monitor framerate is, on mine it ran at 165fps/runs per second
            //bitblt fetches monitor pixels
            //directx would create its own monitor which it reads from in a faster refresh rate.
            std::cout << "main runs per second: " << fps << std::endl;
        }

        // 0,1ms we better not do this (windows makes this wait around 10ms)
        // std::this_thread::sleep_for(std::chrono::microseconds(100));

        pixel_data.reset();
    }
}