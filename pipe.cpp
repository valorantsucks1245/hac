#include "pipe.h"

bool PipeClient::connect(const std::string& pipeName) {
    // Close existing pipe if any
    if (hPipe != INVALID_HANDLE_VALUE) {
        CloseHandle(hPipe);
        hPipe = INVALID_HANDLE_VALUE;
    }

    pipe_name = pipeName;
    std::string full_pipe_name = "\\\\.\\pipe\\" + pipe_name;
    printf("Connecting to named pipe\n");

    while (true) {
        hPipe = CreateFileA(
            full_pipe_name.c_str(),   // Pipe name
            GENERIC_WRITE,            // Write access
            0,                        // No sharing
            NULL,                     // Default security attributes
            OPEN_EXISTING,            // Opens existing pipe
            0,                        // Default attributes
            NULL);                    // No template file

        if (hPipe != INVALID_HANDLE_VALUE) {
            printf("Connected to named pipe.\n");
            return true;  // Success should return true
        }

        if (GetLastError() != ERROR_PIPE_BUSY) {
            printf("error: Could not open pipe.\n");
            std::cout << "error: " + std::to_string(GetLastError()) << std::endl;
            return false;
        }

        // If pipe is busy, wait for it
        if (!WaitNamedPipeA(full_pipe_name.c_str(), 5000)) {
            printf("error: Could not open pipe: 5-second wait timed out.\n");
            return false;
        }
        // If we get here, the pipe was busy but should be available now
        // Loop will try to connect again
    }
}

PipeClient::PipeClient() : hPipe(INVALID_HANDLE_VALUE) {
    // No pipe_name set, no connection attempt
}


PipeClient::~PipeClient() {
    if (hPipe != INVALID_HANDLE_VALUE) {
        CloseHandle(hPipe);
        hPipe = INVALID_HANDLE_VALUE;  // Good practice to clear the handle
    }
}

void PipeClient::send_data() {
    if (hPipe == INVALID_HANDLE_VALUE) {
        printf("error: Attempt to write to invalid pipe handle\n");
        return;
    }

    DWORD bytesWritten;
    BOOL result = WriteFile(
        hPipe,         // Pipe handle
        buffer,        // Buffer to write from
        7,             // Number of bytes to write
        &bytesWritten, // Number of bytes written
        NULL);         // Not overlapped I/O

    if (!result || bytesWritten != 7) {
        std::cout << "error: Failed to write to pipe" << std::endl;
        std::cout << "error: " + std::to_string(GetLastError()) << std::endl;
    }
}

//mutex adden vll noch

void PipeClient::left_click() {
    memset(buffer, 0, 7);
    buffer[0] = 0xFF;
    buffer[5] = 0x01;
    send_data();
}

void PipeClient::move_mouse(int dx, int dy) {
    memset(buffer, 0, 7);
    buffer[0] = 0xFF;
    buffer[1] = dx & 0xFF;
    buffer[2] = (dx >> 8) & 0xFF;

    buffer[3] = dy & 0xFF;
    buffer[4] = (dy >> 8) & 0xFF;

    send_data();
}