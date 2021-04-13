//
// Engine.cpp : Put all your graphics stuff in this file. This is kind of the graphics module.
// In here, you should type all your OpenGL commands, and you can also type code to handle
// input platform events (e.g to move the camera or react to certain shortcuts), writing some
// graphics related GUI options, and so on.
//

#include "Engine.h"
#include "Globals.h"
#include "Application.h"

#define APPLICATION_NAME "AGPEngine by Lucho Suaya"
#define WINDOW_WIDTH 960
#define WINDOW_HEIGHT 540
#define FRAMERATE 60.0f

int main(int argc, char** argv)
{
    Application* application = new Application(APPLICATION_NAME, WINDOW_WIDTH, WINDOW_HEIGHT, FRAMERATE);

    App app = {};
    app.isRunning = true;
    GlobalFrameArenaMemory = (unsigned char*)malloc(GLOBAL_FRAME_ARENA_SIZE);
    
    while (app.isRunning)
    {
        application->Update();

        // Reset frame allocator
        GlobalFrameArenaHead = 0;
    }

    free(GlobalFrameArenaMemory);
    delete application;
    return 0;
}