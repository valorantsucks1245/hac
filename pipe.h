#pragma once
#include "headers.h"

class PipeClient {
private:
    char buffer[7];
    std::string pipe_name;
    HANDLE hPipe;
    void send_data();

public:
    PipeClient();
    ~PipeClient();
    bool connect(const std::string& pipeName);
    void left_click();
    void move_mouse(int dx, int dy);
};