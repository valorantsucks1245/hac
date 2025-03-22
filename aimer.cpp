#include "aimer.h"


void Aimeling::move_screen(const int x, int y, float min_speed, float max_speed, float distance_threshold) {
    //move into direction by speed (min , max , distance)
    double dx = x - ml->roi_size / 2;
    double dy = y - ml->roi_size / 2;
    double distance = std::sqrt(dx * dx + dy * dy);

    const double stop_threshold = 2.0;
    if (distance < stop_threshold) {
        return;
    }

    double angle = std::atan2(dy, dx);

    double speed_factor = std::min(distance / distance_threshold, 1.0);
    double speed = min_speed + (max_speed - min_speed) * speed_factor;

    double move_x = speed * std::cos(angle);
    double move_y = speed * std::sin(angle);

    ml->pipe_client.move_mouse(static_cast<int>(std::round(move_x)), static_cast<int>(std::round(move_y)));
}

Aimeling::Aimeling(MainLoop* main_loop) {
    ml = main_loop;
}

cv::Point findTopmost255Pixel(const cv::Mat& image) {
    CV_Assert(image.channels() == 1);

    for (int y = 0; y < image.rows; ++y) {
        const uint8_t* row = image.ptr<uint8_t>(y);
        for (int x = 0; x < image.cols; ++x) {
            if (row[x] == 255) {
                return cv::Point(x, y);
            }
        }
    }

    return cv::Point(0, 0);
}

void Aimeling::aimeling_loop() {
    int roi_size = ml->roi_size;
    cv::Point screen_center(roi_size / 2, roi_size / 2);
    bool toggle = false;
    bool prev_state = false;

    int frames = 0;
    int fps = 0;
    auto last_time = std::chrono::high_resolution_clock::now();

    uint8_t* pixel_copy = (uint8_t*)malloc((ml->width * ml->height) * 4); //todo cleanup

    while (true) {
        try {
            if (ml->pixels == nullptr) {
                std::this_thread::sleep_for(std::chrono::microseconds(250));//mhhh
                continue;
            }

            memcpy(pixel_copy, ml->pixels, (ml->width * ml->height) * 4);


            cv::Mat binary_image = cv::Mat::zeros(roi_size, roi_size, CV_8UC1);

            bool found = false;
            for (int y = 0; y < roi_size; ++y) {
                uint8_t* row = pixel_copy + y * roi_size * 4;
                for (int x = 0; x < roi_size; ++x) {
                    uint8_t* pixel = row + x * 4;

                    if (std::abs(pixel[0] - 250) <= 70 && // B
                        std::abs(pixel[1] - 100) <= 70 && // G
                        std::abs(pixel[2] - 250) <= 70)   // R
                    {
                        binary_image.at<uchar>(y, x) = 255;
                        found = true;
                    }
                }
            }

            if (!found) continue; //sleep vll

            cv::Mat dilated;
            cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(4, 4));
            cv::dilate(binary_image, dilated, kernel, cv::Point(-1, -1), 1);
            cv::Point target = findTopmost255Pixel(dilated);

            if (target.x > 0 || target.y > 0) {
                if (ml->config->aimer_config.mode != 0) {
                    bool pressed = GetAsyncKeyState(ml->config->aimer_config.key);

                    if (ml->config->aimer_config.mode == 1) {
                        if (pressed && !prev_state)
                            toggle = !toggle;
                    }
                    else {
                        toggle = pressed;
                    }
                    prev_state = pressed;

                    if ((ml->config->aimer_config.mode == 1 && toggle) ||
                        (ml->config->aimer_config.mode == 2 && pressed)) {
                        move_screen(target.x + 3, target.y + 5,  //offset to aim more towards center of target, no target resolve implemented yet
                            ml->config->aimer_config.min_speed,
                            ml->config->aimer_config.max_speed,
                            ml->config->aimer_config.speed_distance);
                    }
                }
            }
        }
        catch (const cv::Exception& e) {
            printf("OpenCV Exception: %s\n", e.what());
            printf("restarting aimeling loop\n");
            aimeling_loop();
        }
        catch (const std::exception& e) {
            printf("Standard Exception: %s\n", e.what());
            printf("restarting aimeling loop\n");
            aimeling_loop();
        }
        catch (...) {
            printf("Unknown Exception Caught\n");
            printf("restarting aimeling loop\n");
            aimeling_loop();
        }

        frames++;
        auto current_time = std::chrono::high_resolution_clock::now();
        auto time_diff = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - last_time).count();

        if (time_diff >= 1000) {
            fps = frames;
            frames = 0;
            last_time = current_time;

            std::cout << "aimer runs per second: " << fps << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::microseconds(500));
    }
}