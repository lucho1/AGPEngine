

// ----------------------- Graphics Card Usage --------------------------------------------------------
#include "Globals.h"
extern "C" {
	// http://developer.download.nvidia.com/devzone/devcenter/gamegraphics/files/OptimusRenderingPolicies.pdf
	__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;

	// http://developer.amd.com/community/blog/2015/10/02/amd-enduro-system-for-developers/
	// or (if the 1st doesn't works): https://gpuopen.com/amdpowerxpressrequesthighperformance/ or https://community.amd.com/thread/169965
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}



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