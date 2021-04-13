#include "Application.h"

#include "Core/Platform/Input.h"
#include "Renderer/Renderer.h"

// --- Testing Layer ---
#include "Sandbox.h"
static Sandbox* s_Sandbox;
// ---------------------



// ------------------------------------------------------------------------------
MemoryMetrics Application::s_MemoryMetrics = {};

void* operator new(size_t size)
{
    Application::GetMemoryMetrics().AddAllocation((uint)size);
    return malloc(size);
}

void operator delete(void* memory, size_t size)
{
    Application::GetMemoryMetrics().AddDeallocation((uint)size);
    free(memory);
}



// ------------------------------------------------------------------------------
Application* Application::s_ApplicationInstance = nullptr;

Application::Application(const std::string& name, uint window_width, uint window_height, float framerate)
{
    // -- Singleton Check --
	ASSERT(!s_ApplicationInstance, "An Instance of the Application alrady exists!");
	s_ApplicationInstance = this;

    // -- Initializations --
	ENGINE_LOG("--- Initializing Application Window ---");
	m_AppWindow = CreateUnique<Window>(window_width, window_height, name);
	m_AppWindow->Init();

	ENGINE_LOG("--- Initializing Application Renderer ---");
	Renderer::Init();

	ENGINE_LOG("--- Initializing ImGui Layer ---");
    m_ImGuiLayer = new ImGuiLayer();
	m_ImGuiLayer->Init();

    // -- Testing Area --
    s_Sandbox = new Sandbox();
    s_Sandbox->Init();

    // -- Delta Time --
	m_DeltaTime = 1.0f / framerate;
	m_LastFrameTime = m_AppWindow->GetGLFWTime();
}

Application::~Application()
{
    delete s_Sandbox;

    Renderer::Shutdown();
    Resources::CleanUp();
    delete m_ImGuiLayer;
}

void Application::OnWindowResize(uint width, uint height)
{
    if (width == 0 || height == 0)
        return;

    Renderer::OnWindowResized(width, height);
}



// ------------------------------------------------------------------------------
void Application::Update()
{
    while (m_Running)
    {
        // -- Delta Time Calculation --
        float currentFrameTime = m_AppWindow->GetGLFWTime();
        m_DeltaTime = currentFrameTime - m_LastFrameTime;
        m_LastFrameTime = currentFrameTime;

        // -- Clear Input State --
        Input::ResetInput();


        // -- Render --
        Renderer::ClearRenderer(m_AppWindow->GetWidth(), m_AppWindow->GetHeight());
        Renderer::BeginScene(glm::mat4(1.0f));
        s_Sandbox->OnUpdate(m_DeltaTime);
        Renderer::EndScene();

        // -- UI Rendering --
        m_ImGuiLayer->Begin();
        s_Sandbox->OnUIRender(m_DeltaTime);
        m_ImGuiLayer->Render();


        // -- Input & Window Update --
        Input::Update();
        m_AppWindow->Update();

        // -- Arena Global Variable Reset --
        GlobalFrameArenaHead = 0;
    }
}