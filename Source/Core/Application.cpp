#include "Application.h"
#include "Input.h"
#include "Renderer/Renderer.h"

Application* Application::s_ApplicationInstance = nullptr;

// TODO: TEMP
#include "Renderer/Texture.h"
#include "Renderer/Buffers.h"
#include "Renderer/Shader.h"
#include "Renderer/Utils/RenderCommand.h"
Ref<VertexBuffer> m_VBuffer;
Ref<IndexBuffer> m_IBuffer;
Ref<VertexArray> m_VArray;
Ref<Texture> m_TestTexture;
Ref<Shader> m_TextureShader;


Application::Application(const std::string& name, uint window_width, uint window_height, float framerate)
{
	ASSERT(!s_ApplicationInstance, "An Instance of the Application alrady exists!");
	s_ApplicationInstance = this;

	ENGINE_LOG("--- Initializing Application Window ---");
	m_AppWindow = CreateUnique<Window>(window_width, window_height, name);
	m_AppWindow->Init();

	ENGINE_LOG("--- Initializing Application Renderer ---");
	//m_AppRenderer = CreateUnique<Renderer>();
	Renderer::Init();

	ENGINE_LOG("--- Initializing ImGui Layer ---");
    m_ImGuiLayer = new ImGuiLayer();
	m_ImGuiLayer->Init();


	m_DeltaTime = 1.0f / framerate;
	m_LastFrameTime = m_AppWindow->GetGLFWTime();


	// TODO: TEMP
    // -- Buffers Test --
    uint indices[6] = { 0, 1, 2, 2, 3, 0 };
    float vertices[5 * 4] = {
        -0.5f,	-0.5f,	0.0f, 0.0f, 0.0f,		// For negative X positions, UV should be 0, for positive, 1
         0.5f,	-0.5f,	0.0f, 1.0f, 0.0f,		// If you render, on a square, the texCoords (as color = vec4(tC, 0, 1)), the colors of the square in its corners are
         0.5f,	 0.5f,	0.0f, 1.0f, 1.0f,		// (0,0,0,1) - Black, (1,0,0,1) - Red, (1,1,0,0) - Yellow, (0,1,0,1) - Green
        -0.5f,	 0.5f,	0.0f, 0.0f, 1.0f
    };

    BufferLayout layout = { { SHADER_DATA::FLOAT3, "a_Position" }, { SHADER_DATA::FLOAT2, "a_TexCoord" } };
    
    m_VArray = CreateRef<VertexArray>();
    m_VBuffer = CreateRef<VertexBuffer>(vertices, sizeof(vertices));
    m_IBuffer = CreateRef<IndexBuffer>(indices, sizeof(indices) / sizeof(uint));
    
    m_VBuffer->SetLayout(layout);
    m_VArray->AddVertexBuffer(m_VBuffer);
    m_VArray->SetIndexBuffer(m_IBuffer);
    
    m_VArray->Unbind();
    m_VBuffer->Unbind();
    m_IBuffer->Unbind();
    
    // -- Texture Test --
    m_TestTexture = CreateRef<Texture>("Resources/textures/dice.png");
    
    // -- Shader Test --
    m_TextureShader = CreateRef<Shader>("Resources/shaders/TexturedShader.glsl");
}

Application::~Application()
{
    Renderer::Shutdown();
    delete m_ImGuiLayer;
}

void Application::OnWindowResize(uint width, uint height)
{
    if (width == 0 || height == 0)
        return;

    RenderCommand::SetViewport(0, 0, width, height);
}

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

        // -- Shader Hot Reload --
        m_TextureShader->CheckLastModification();

        // -- Render --
        Renderer::BeginScene(glm::mat4(1.0f));
        Render();
        Renderer::EndScene();

        // -- UI Rendering --
        m_ImGuiLayer->Begin();
        RenderUI();
        m_ImGuiLayer->Render();

        // -- Input & Window Update --
        Input::Update();
        m_AppWindow->Update();

        // -- Arena Global Variable Reset --
        GlobalFrameArenaHead = 0;
    }
}

void Application::Render()
{
    RenderCommand::SetClearColor(glm::vec3(0.15f));
    RenderCommand::Clear();
    RenderCommand::SetViewport(0, 0, m_AppWindow->GetWidth(), m_AppWindow->GetHeight());
    
    m_TextureShader->Bind();
    m_TestTexture->Bind();
    
    m_TextureShader->SetUniformInt("u_Texture", 0);
    //m_TextureShader->SetUniformVec4("u_Color", { 0.6f, 0.2f, 0.2f, 1.0f });
    
    Renderer::Submit(m_TextureShader, m_VArray);
    
    m_TestTexture->Unbind();
    m_TextureShader->Unbind();
    m_VArray->Unbind();
}


#include <imgui.h>
void Application::RenderUI()
{
    ImGui::Begin("Info");
    ImGui::Text("FPS: %f", 1.0f/m_DeltaTime);
    ImGui::End();

    ImGui::Begin("Renderer Statistics");
    RendererStatistics stats = Renderer::GetStatistics();
    ImGui::Text("- Graphics by %s -", stats.GLVendor.c_str());
    ImGui::Text("Graphics Card:     %s", stats.GraphicsCard.c_str());
    ImGui::Text("OpenGL Version:    %i.%i (%s)", stats.OGL_MajorVersion, stats.OGL_MinorVersion, stats.GLVersion.c_str());
    ImGui::Text("Shading Version:   GLSL %s", stats.GLShadingVersion.c_str());
    ImGui::End();
}