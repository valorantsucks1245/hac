#include "headers.h"

#include "capture.h"
#include "structs.h"
#include "debug.h"
#include "pipe.h"
#include "opencv_funcs.h"
#include "aimer.h"
#include "pixel_data.h"


//this is dll stuff, no needed but i was too lazy to remove it
int start(HMODULE hModule) {

    if (hModule == 0x0000000000000000) {
        printf("error: hModule nullptr");
        return 1;
    }
    if (((LauncherPointers*)hModule)->check_pointer != 0x0000000000123456) {
        printf("error: check pointer invalid");
        return 1;
    }

    MainLoop main_loop = MainLoop();
    main_loop.start((LauncherPointers*)hModule);
}

int main()
{
    Config config_ff;
    LauncherPointers launcher_pointers_ff;
    launcher_pointers_ff.check_pointer = 0x0000000000123456;
    launcher_pointers_ff.end_time = (uintptr_t)0;
    launcher_pointers_ff.config = (uintptr_t)&config_ff;

    ConfigDebugRead configReader;
    std::thread config_get_thread(&ConfigDebugRead::run_cfg_loop, &configReader, std::ref(config_ff));
    config_get_thread.detach();

    start((HMODULE)&launcher_pointers_ff);

    return 1;
}