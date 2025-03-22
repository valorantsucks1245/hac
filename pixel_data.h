#pragma once
#include "headers.h"

class PixelData {
private:
    uint8_t** pixels_ptr;

    int width;
    int height;
    int trigger_width;
    int trigger_height;


    int centerX;
    int centerY;
public:
    uint8_t* bitmap;
    int UL, UR, LL, LR;
    PixelData() : pixels_ptr(nullptr), bitmap(nullptr),
        width(0), height(0), trigger_width(0), trigger_height(0),
        centerX(0), centerY(0), UL(0), UR(0), LL(0), LR(0) {}

    void init(uint8_t*& pxls, int w, int h) {
        pixels_ptr = &pxls;
        width = w;
        height = h;
        bitmap = new uint8_t[width * height]();
        UL = UR = LL = LR = 0;
        centerX = width / 2;
        centerY = height / 2;
    }

    void check_center_pixels() {
        int centerX = width / 2;
        int centerY = height / 2;
        int radius = 20;
        UL = UR = LL = LR = 0;
        int startY = std::max(0, centerY - radius);
        int endY = std::min(height, centerY + radius);
        int startX = std::max(0, centerX - radius);
        int endX = std::min(width, centerX + radius);

        for (int y = startY; y < endY; y++) {
            int yOffset = y * width;
            int yDist = y - centerY;
            int yDistSq = yDist * yDist;
            for (int x = startX; x < endX; x++) {
                int xDist = x - centerX;
                if (xDist * xDist + yDistSq <= radius * radius) {
                    if (bitmap[yOffset + x] != 0) {
                        if (x < centerX) {
                            (y < centerY) ? UL++ : LL++;
                        }
                        else {
                            (y < centerY) ? UR++ : LR++;
                        }
                    }
                }
            }
        }
    }

    void create_bitmap() {
        uint8_t* pixels = *pixels_ptr;
        for (int y = 0; y < height; ++y) {
            uint8_t* row = pixels + y * width * 4;
            for (int x = 0; x < width; ++x) {
                uint8_t* pixel = row + x * 4;
                // BGR
                bool bgr = (std::abs(pixel[0] - 250) <= 70 &&
                    std::abs(pixel[1] - 100) <= 70 &&
                    std::abs(pixel[2] - 250) <= 70);
                if (bgr) {
                    bitmap[y * width + x] = 255;
                }
            }
        }
    }

    bool is_in_cross() {
        int corners3 = 0;
        int corners4 = 0;
        if (UL > 1) corners4++;
        if (UR > 1) corners4++;
        if (LL > 1) corners4++;
        if (LR > 1) corners4++;
        if (UL > 3) corners3++;
        if (UR > 3) corners3++;
        if (LL > 3) corners3++;
        if (LR > 3) corners3++;
        //printf("%d %d %d %d", UL, UR, LL, LR);
        //printf(" : %d %d \n", corners3, corners4);
        return (corners3 > 3 || corners4 > 4);
    }

    void reset() {
        UL = UR = LL = LR = 0;
        std::memset(bitmap, 0, width * height);
    }

    ~PixelData() {
        delete[] bitmap;
    }
};