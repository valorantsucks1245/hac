#pragma once
#include "headers.h"
#include "mainloop.h"
#include "weapon_images.h"


class MainLoop;

cv::Mat load_image_from_memory(const std::vector<uint8_t>& buffer) {
    std::vector<uint8_t> decoded;
    return cv::imdecode(decoded, cv::IMREAD_GRAYSCALE);
}

class WeaponDetection {
private:
    static std::vector<cv::Mat> processed_templates;
    static bool templates_initialized;
    static std::mutex initialization_mutex;

    MainLoop* ml;

    int height;
    int width;
    RECT weapon_rect;
    const double confidence_threshold = 0.61;
    VALO_GUNS current_gun = MELEE;
    ScreenCapturer* capturer;
    uint8_t* pixels = nullptr;

    static bool initialize_templates() {
        std::lock_guard<std::mutex> lock(initialization_mutex);

        if (templates_initialized) return true;

        const std::vector<const std::vector<uint8_t>*> ordered_weapon_data = {
            &weapon_images::Melee_data,
            &weapon_images::Classic_data,
            &weapon_images::Shorty_data,
            &weapon_images::Frenzy_data,
            &weapon_images::Ghost_data,
            &weapon_images::Sheriff_data,
            &weapon_images::Stinger_data,
            &weapon_images::Spectre_data,
            &weapon_images::Bucky_data,
            &weapon_images::Judge_data,
            &weapon_images::Bulldog_data,
            &weapon_images::Guardian_data,
            &weapon_images::Phantom_data,
            &weapon_images::Vandal_data,
            &weapon_images::Marshal_data,
            &weapon_images::Operator_data,
            &weapon_images::Ares_data,
            &weapon_images::Odin_data,
            &weapon_images::Outlaw_data
        };

        for (const auto& data_ptr : ordered_weapon_data) {
            cv::Mat templ = cv::imdecode(*data_ptr, cv::IMREAD_GRAYSCALE);
            if (templ.empty()) return false;

            cv::Mat processed;
            cv::threshold(templ, processed, 160, 255, cv::THRESH_BINARY);
            processed_templates.push_back(processed);

        }

        templates_initialized = true;
        return true;
    }

public:

    WeaponDetection(MainLoop* main_loop) {
        ml = main_loop;
        if (!templates_initialized) {
            initialize_templates();
        }

        int screen_width = GetSystemMetrics(SM_CXSCREEN);
        int screen_height = GetSystemMetrics(SM_CYSCREEN);

        weapon_rect = {
            5 * screen_width / 6,
            15 * screen_height / 24,
            (5 * screen_width / 6) + screen_width / 8,
            (15 * screen_height / 24) + screen_height / 4
        };

        height = weapon_rect.bottom - weapon_rect.top;
        width = weapon_rect.right - weapon_rect.left;

        capturer = new ScreenCapturer();
        capturer->init(weapon_rect);
        pixels = new uint8_t[width * height * 4]();
    }

    ~WeaponDetection() {
        delete[] pixels;
        delete capturer;
    }

    VALO_GUNS detect_current_weapon() {
        if (!templates_initialized) return UNKNOWN;

        capturer->capture_pixels(pixels);
        if (pixels == nullptr) return current_gun;

        cv::Mat screen = cv::Mat::zeros(height, width, CV_8UC3);
        if (height <= 0 || width <= 0) return current_gun;

        bool hasValidPixels = false;
        for (int y = 0; y < height; y++) {
            auto row = y * width * 4;
            for (int x = 0; x < width; x++) {
                auto pixel = row + x * 4;
                screen.at<cv::Vec3b>(y, x) = cv::Vec3b(
                    pixels[pixel],
                    pixels[pixel + 1],
                    pixels[pixel + 2]
                );
                if (pixels[pixel] != 0 || pixels[pixel + 1] != 0 || pixels[pixel + 2] != 0) {
                    hasValidPixels = true;
                }
            }
        }

        if (!hasValidPixels || screen.empty() || screen.channels() != 3) return current_gun;

        try {
            cv::Mat processed_screen(screen.rows, screen.cols, CV_8UC1);
            for (int i = 0; i < screen.rows; i++) {
                for (int j = 0; j < screen.cols; j++) {
                    cv::Vec3b pixel = screen.at<cv::Vec3b>(i, j);
                    processed_screen.at<uchar>(i, j) = static_cast<uchar>(
                        (pixel[0] * 0.114 + pixel[1] * 0.587 + pixel[2] * 0.299));
                }
            }

            if (processed_screen.empty()) return current_gun;

            cv::Mat thresholded = cv::Mat::zeros(processed_screen.size(), CV_8UC1);
            for (int i = 0; i < processed_screen.rows; i++) {
                for (int j = 0; j < processed_screen.cols; j++) {
                    thresholded.at<uchar>(i, j) = (processed_screen.at<uchar>(i, j) > 160) ? 255 : 0;
                }
            }
            processed_screen = thresholded;

            std::vector<double> confidences;
            confidences.reserve(processed_templates.size());

            for (size_t i = 0; i < processed_templates.size(); i++) {
                const auto& template_img = processed_templates[i];
                if (template_img.empty()) continue;

                double best_confidence = 0.0;
                for (double scale : {0.8, 0.9, 1.0, 1.1, 1.2}) {
                    cv::Mat scaled_template;
                    cv::Mat result;
                    cv::resize(template_img, scaled_template, cv::Size(), scale, scale, cv::INTER_LINEAR);

                    if (scaled_template.rows <= processed_screen.rows &&
                        scaled_template.cols <= processed_screen.cols) {
                        cv::matchTemplate(processed_screen, scaled_template, result, cv::TM_CCOEFF_NORMED);
                        double min_val, max_val;
                        cv::Point min_loc, max_loc;
                        cv::minMaxLoc(result, &min_val, &max_val, &min_loc, &max_loc);
                        best_confidence = std::max<double>(best_confidence, max_val);
                    }
                }
                confidences.push_back(best_confidence);
            }

            auto max_it = std::max_element(confidences.begin(), confidences.end());
            if (max_it != confidences.end() && *max_it > confidence_threshold) {
                auto best_match_idx = std::distance(confidences.begin(), max_it);
                current_gun = static_cast<VALO_GUNS>(best_match_idx);
            }
        }
        catch (...) {
            printf("%s\n", "error in weapon_detection");
        }

        printf("%s\n", GUN_NAMES[current_gun]);

        return current_gun;
    }

    void weapon_detection_loop() {
        while (1) {
            auto start_time = std::chrono::high_resolution_clock::now();
            ml->current_gun = detect_current_weapon();
            auto end_time = std::chrono::high_resolution_clock::now();

            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
            // printf("Weapon detection took %lld ms\n", duration.count());

            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }
};
std::vector<cv::Mat> WeaponDetection::processed_templates;
bool WeaponDetection::templates_initialized = false;
std::mutex WeaponDetection::initialization_mutex;
