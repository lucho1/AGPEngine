//
// Engine.cpp : Put all your graphics stuff in this file. This is kind of the graphics module.
// In here, you should type all your OpenGL commands, and you can also type code to handle
// input platform events (e.g to move the camera or react to certain shortcuts), writing some
// graphics related GUI options, and so on.
//

#include "Engine.h"
#include "Globals.h"
#include "Utils/FileStringUtils.h"
#include <imgui.h>
#include <stb_image.h>
#include <stb_image_write.h>


#include "Window.h"
#include "Input.h"
#include "ImGuiLayer.h"
#include "Renderer/Renderer.h"
#include "Renderer/Buffers.h"
#include "Renderer/Utils/RendererPrimitives.h"
#include "Renderer/Texture.h"
#include "Renderer/Shader.h"
#include "Renderer/Utils/RenderCommand.h"

static Window* m_Window = nullptr;
void* GetApplicationWindow() { return (void*)m_Window; }

static ImGuiLayer* m_ImGuiLayer = nullptr;
void* GetImGuiLayer() { return (void*)m_ImGuiLayer; }

static Renderer* m_Renderer = nullptr;
void* GetRenderer() { return (void*)m_Renderer; }

Ref<VertexBuffer> m_VBuffer;
Ref<IndexBuffer> m_IBuffer;
Ref<VertexArray> m_VArray;
Ref<Texture> m_TestTexture;
Ref<Shader> m_TextureShader;

int main()
{
    App app = {};
    app.deltaTime = 1.0f / 60.0f;
    app.displaySize = ivec2(960, 540);
    app.isRunning = true;

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

    ENGINE_LOG("-- RENDERER INIT --");
    m_Renderer = new Renderer();
    m_Renderer->Init();
    
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
    m_IBuffer = CreateRef<IndexBuffer>(indices, sizeof(indices)/sizeof(uint));

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

    while (app.isRunning)
    {
        // Tell GLFW to call platform callbacks
        glfwPollEvents();

        // ImGui Begin
        m_ImGuiLayer->Begin(app);

        // ImGui Prepare Render
        m_ImGuiLayer->PrepareRender(app);

        // Clear input state if required by ImGui
        Input::ResetInput();

        // Update
        Update(&app);

        // Transition input key/button states
        Input::Update();

        // Render
        m_Renderer->BeginScene(glm::mat4(1.0f));
        Render(&app);
        m_Renderer->EndScene();

        // ImGui Render, after Input Update
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
    delete m_Renderer;
    delete m_Window;
    delete m_ImGuiLayer;
    return 0;
}



void Init(App* app)
{
    app->mode = Mode_TexturedQuad;
}

void Gui(App* app)
{
}

void Update(App* app)
{
    // You can handle app->input keyboard/mouse here
}

void Render(App* app)
{
    RenderCommand::SetClearColor(glm::vec3(0.15f));
    RenderCommand::Clear();
    RenderCommand::SetViewport(0, 0, app->displaySize.x, app->displaySize.y);

    m_TextureShader->Bind();
    m_TestTexture->Bind();
    
    m_TextureShader->SetUniformInt("u_Texture", 0);
    //m_TextureShader->SetUniformVec4("u_Color", { 0.6f, 0.2f, 0.2f, 1.0f });

    m_Renderer->Submit(m_TextureShader, m_VArray);

    m_TestTexture->Unbind();
    m_TextureShader->Unbind();
    m_VArray->Unbind();
}

