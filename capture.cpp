#include "capture.h"

ScreenCapturer::ScreenCapturer() {}

void ScreenCapturer::init(const RECT& rect) {
    width = rect.right - rect.left;
    height = rect.bottom - rect.top;
    captureRect = rect;

    hScreenDC = GetDC(NULL);
    hMemoryDC = CreateCompatibleDC(hScreenDC);

    BITMAPINFO bmi = { 0 };
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height; // Negative height to store the image top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    
    hBitmap = CreateDIBSection(hMemoryDC, &bmi, DIB_RGB_COLORS, &pBits, NULL, 0);
    if (!hBitmap || !pBits) {
        printf("Failed to create DIB section or allocate pixel memory.");
        return;
    }
    SelectObject(hMemoryDC, hBitmap);
}

ScreenCapturer::~ScreenCapturer() {
    DeleteObject(hBitmap);
    DeleteDC(hMemoryDC);
    ReleaseDC(NULL, hScreenDC);
}

void ScreenCapturer::capture_pixels(uint8_t*& pixels) {
    if (!BitBlt(hMemoryDC, 0, 0, width, height, hScreenDC, captureRect.left, captureRect.top, SRCCOPY)) {
        printf("BitBlt failed.");
    }
    pixels = static_cast<uint8_t*>(pBits);
}
