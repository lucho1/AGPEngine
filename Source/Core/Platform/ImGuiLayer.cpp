#include "ImGuiLayer.h"

#include "Core/Application/Application.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>



// ------------------------------------------------------------------------------
ImGuiLayer::~ImGuiLayer()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}



// ------------------------------------------------------------------------------
void ImGuiLayer::Init()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
    //io.ConfigViewportsNoAutoMerge = true;
    //io.ConfigViewportsNoTaskBarIcon = true;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    if (!ImGui_ImplGlfw_InitForOpenGL(static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow()), true))
    {
        ENGINE_LOG("ImGui_ImplGlfw_InitForOpenGL() failed\n");
        return;
    }

    if (!ImGui_ImplOpenGL3_Init("#version 460"))
    {
        ENGINE_LOG("Failed to initialize ImGui OpenGL wrapper\n");
        return;
    }
}


void ImGuiLayer::Begin()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}


void ImGuiLayer::Render()
{
    ImGuiIO& io = ImGui::GetIO();
    Window& window = Application::Get().GetWindow();
    io.DisplaySize = ImVec2((float)window.GetWidth(), (float)window.GetHeight());

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }
}



// ------------------------------------------------------------------------------
bool ImGuiLayer::CapturingKeyboard() const
{
    return ImGui::GetIO().WantCaptureKeyboard;
}

bool ImGuiLayer::CapturingMouse() const
{
    return ImGui::GetIO().WantCaptureMouse;
}