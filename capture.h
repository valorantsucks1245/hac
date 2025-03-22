#pragma once
#include "headers.h"


class ScreenCapturer {
private:
    HDC hScreenDC;
    HDC hMemoryDC;
    HBITMAP hBitmap;
    void* pBits;
    int width;
    int height;
    RECT captureRect;
    uint8_t* pixels;

public:
    ScreenCapturer();
    ~ScreenCapturer();
    void init(const RECT& rect);
    void capture_pixels(uint8_t*& pixels);
};
