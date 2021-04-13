

// ----------------------- Application ----------------------------------------------------------------
#include "Application.h"

#define APPLICATION_NAME "AGPEngine by Lucho Suaya"
#define WINDOW_WIDTH 960
#define WINDOW_HEIGHT 540
#define FRAMERATE 60.0f

int main(int argc, char** argv)
{
    // -- Initialization --
    GlobalFrameArenaMemory = (unsigned char*)malloc(GLOBAL_FRAME_ARENA_SIZE);
    Application* application = new Application(APPLICATION_NAME, WINDOW_WIDTH, WINDOW_HEIGHT, FRAMERATE);
    
    // -- App Update --
    application->Update();

    // -- App Delete --
    ENGINE_LOG("--- Closing Application ---")
    free(GlobalFrameArenaMemory);
    delete application;
    return 0;
}