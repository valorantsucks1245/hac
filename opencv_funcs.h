#pragma once
#include "headers.h"

class opencv_funcs {
public:
    static void dilate_toward_pixels(uint8_t* bitmap, int width, int height) {
        uint8_t* temp_bitmap = new uint8_t[width * height];
        std::memcpy(temp_bitmap, bitmap, width * height);

        const int dx[8] = { 0, 1, 1, 1, 0, -1, -1, -1 };
        const int dy[8] = { -1, -1, 0, 1, 1, 1, 0, -1 };

        for (int y = 1; y < height - 1; ++y) {
            for (int x = 1; x < width - 1; ++x) {
                if (bitmap[y * width + x] != 0)
                    continue;

                int neighbor_count = 0;
                for (int dir = 0; dir < 8; ++dir) {
                    int nx = x + dx[dir];
                    int ny = y + dy[dir];

                    if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                        if (bitmap[ny * width + nx] != 0) {
                            neighbor_count++;
                        }
                    }
                }

                if (neighbor_count >= 2) {
                    temp_bitmap[y * width + x] = 255;
                }
            }
        }

        std::memcpy(bitmap, temp_bitmap, width * height);

        delete[] temp_bitmap;
    }


    static void connect_close_contours(uint8_t* bitmap, int width, int height, int max_distance = 5) {
        cv::Mat image(height, width, CV_8UC1);
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                image.at<uchar>(y, x) = bitmap[y * width + x] ? 255 : 0;
            }
        }

        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(image, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        cv::Mat result = cv::Mat::zeros(image.size(), CV_8UC1);
        for (size_t i = 0; i < contours.size(); i++) {
            double area = cv::contourArea(contours[i]);
            if (area > 20) {
                cv::drawContours(result, contours, i, cv::Scalar(255), -1);
            }
        }

        cv::Mat element = cv::getStructuringElement(cv::MORPH_ELLIPSE,
            cv::Size(2 * max_distance + 1, 2 * max_distance + 1));
        cv::morphologyEx(result, result, cv::MORPH_CLOSE, element);

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                bitmap[y * width + x] = result.at<uchar>(y, x) > 0 ? 255 : 0;
            }
        }
    }

    static void connect_nearby_pixels(uint8_t* bitmap, int width, int height, int max_distance = 3) {
        uint8_t* temp_bitmap = new uint8_t[width * height];
        std::memcpy(temp_bitmap, bitmap, width * height);

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                if (bitmap[y * width + x] == 0)
                    continue;

                for (int ny = std::max(0, y - max_distance); ny <= std::min(height - 1, y + max_distance); ny++) {
                    for (int nx = std::max(0, x - max_distance); nx <= std::min(width - 1, x + max_distance); nx++) {
                        if (bitmap[ny * width + nx] != 0)
                            continue;

                        float dist = std::sqrt(std::pow(nx - x, 2) + std::pow(ny - y, 2));

                        if (dist <= max_distance) {
                            temp_bitmap[ny * width + nx] = 255;
                        }
                    }
                }
            }
        }

        std::memcpy(bitmap, temp_bitmap, width * height);
        delete[] temp_bitmap;
    }

};