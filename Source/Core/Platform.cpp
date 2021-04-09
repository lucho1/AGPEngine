//
// Platform.cpp : This file contains the 'main' function. Program execution begins and ends there.
// The platform layer is in charge to create the environment necessary so the engine disposes of what
// it needs in order to create the application (e.g. window, graphics context, I/O, allocators, etc).
//

#include "Platform.h"
#include "Window.h"
#include "Input.h"
#include "ImGuiLayer.h"




static Window* m_Window = nullptr;
void* GetApplicationWindow() { return (void*)m_Window; }

static ImGuiLayer* m_ImGuiLayer = nullptr;
void* GetImGuiLayer() { return (void*)m_ImGuiLayer; }



int main()
{
    App app         = {};
    app.deltaTime   = 1.0f/60.0f;
    app.displaySize = ivec2(960, 540);
    app.isRunning   = true;

    ENGINE_LOG("-- WINDOW INIT --");
    m_Window = new Window(960, 540, "AGPEngine by Lucho Suaya");
    m_Window->Init(app);

    ENGINE_LOG("-- IMGUI INIT --");
    m_ImGuiLayer = new ImGuiLayer();
    m_ImGuiLayer->Init(m_Window);

    double lastFrameTime = glfwGetTime();
    GlobalFrameArenaMemory = (unsigned char*)malloc(GLOBAL_FRAME_ARENA_SIZE);

    ENGINE_LOG("-- APP INIT --");
    Init(&app);

    while (app.isRunning)
    {
        // Tell GLFW to call platform callbacks
        glfwPollEvents();
        m_ImGuiLayer->Update(app);

        // Clear input state if required by ImGui
        Input::ResetInput();

        // Update
        Update(&app);

        // Transition input key/button states
        Input::Update();

        // Render
        Render(&app);

        // ImGui Render
        m_ImGuiLayer->Render();

        // Present image on screen
        glfwSwapBuffers(&m_Window->GetWindow());

        // Frame time
        double currentFrameTime = glfwGetTime();
        app.deltaTime = (float)(currentFrameTime - lastFrameTime);
        lastFrameTime = currentFrameTime;

        // Reset frame allocator
        GlobalFrameArenaHead = 0;
    }

    free(GlobalFrameArenaMemory);
    delete m_Window;
    delete m_ImGuiLayer;
    return 0;
}