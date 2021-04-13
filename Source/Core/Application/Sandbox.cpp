#include "Sandbox.h"

#include "Core/Platform/Input.h"
#include "Core/Application/Resources.h"

#include "Renderer/Renderer.h"
#include "Renderer/Utils/RenderCommand.h"
#include "Renderer/Utils/RendererPrimitives.h"

#include <imgui.h>



// ------------------------------------------------------------------------------
void Sandbox::Init()
{
    // -- Buffers Test --
    uint indices[6] = { 0, 1, 2, 2, 3, 0 };
    float vertices[5 * 4] = {
        -0.5f,	-0.5f,	0.0f, 0.0f, 0.0f,		// For negative X positions, UV should be 0, for positive, 1
         0.5f,	-0.5f,	0.0f, 1.0f, 0.0f,		// If you render, on a square, the texCoords (as color = vec4(tC, 0, 1)), the colors of the square in its corners are
         0.5f,	 0.5f,	0.0f, 1.0f, 1.0f,		// (0,0,0,1) - Black, (1,0,0,1) - Red, (1,1,0,0) - Yellow, (0,1,0,1) - Green
        -0.5f,	 0.5f,	0.0f, 0.0f, 1.0f
    };

    BufferLayout layout = { { SHADER_DATA::FLOAT3, "a_Position" }, { SHADER_DATA::FLOAT2, "a_TexCoord" } };
    Ref<VertexBuffer> vb = CreateRef<VertexBuffer>(vertices, sizeof(vertices));
    Ref<IndexBuffer> ib = CreateRef<IndexBuffer>(indices, sizeof(indices) / sizeof(uint));

    m_SquareVArray = CreateRef<VertexArray>();
    vb->SetLayout(layout);

    m_SquareVArray->AddVertexBuffer(vb);
    m_SquareVArray->SetIndexBuffer(ib);

    m_SquareVArray->Unbind();
    vb->Unbind(); ib->Unbind();

    // -- Texture Test --
    m_TestTexture = Resources::CreateTexture("Resources/textures/dice.png");

    // -- Shader Test --
    m_TextureShader = CreateRef<Shader>("Resources/shaders/TexturedShader.glsl");
}



// ------------------------------------------------------------------------------
void Sandbox::OnUpdate(float dt)
{
    // -- Shader Hot Reload --
    m_TextureShader->CheckLastModification();

    // -- Render Stuff --
    // Texture Bind
    Renderer::BindTexture(Resources::TexturesIndex::ALBEDO, m_TestTexture);
    
    // Shader Bind & Uniforms
    m_TextureShader->Bind();
    m_TextureShader->SetUniformInt("u_Texture", (int)Resources::TexturesIndex::ALBEDO);
    //m_TextureShader->SetUniformVec4("u_Color", { 0.6f, 0.2f, 0.2f, 1.0f });

    // Draw Call
    Renderer::Submit(m_TextureShader, m_SquareVArray);
    
    // Unbinds
    Renderer::UnbindTexture(Resources::TexturesIndex::ALBEDO, m_TestTexture);
    m_TextureShader->Unbind();
    m_SquareVArray->Unbind();
}



// ------------------------------------------------------------------------------
void Sandbox::OnUIRender(float dt)
{
    // --- Performance Info Display ---
    ImGui::Begin("Info");
    ImGui::Text("FPS: %f", 1.0f / dt);
    
    if (Input::IsKeyPressed(KEY::A))
    {
        for(int i = 0; i < 512; ++i)
        float* a = new float(2.0f);
    }

    // --- Memory Metrics Gathering ---
    float float_mem_allocs[ALLOCATIONS_SAMPLES];
    for (uint i = 0; i < ALLOCATIONS_SAMPLES; ++i)
        float_mem_allocs[i] = BYTETOKB((float)m_MemoryAllocations[i]);

    // --- Memory Metrics Display ---
    char unit[3] = "KB";
    float plot_scale = 100.0f;
    float current_usage = BYTETOKB((float)m_MemoryMetrics.GetCurrentMemoryUsage());

    // Frame the plot according to the memory usage (up until 10MB, which is a LOT)
    if (current_usage > 100.0f)
        plot_scale = 500.0f;

    if (current_usage > 500.0f)
        plot_scale = 1000.0f;

    if (current_usage > 1024.0f)
    {
        unit[0] = 'M';
        plot_scale = 5120.0f;
        current_usage = BYTETOKB(current_usage); // Another conversion makes the value to be at MB
    }

    if (current_usage > 5120.0f)
        plot_scale = 10240.0f;

    // Print Plot
    char overlay[50];
    sprintf(overlay, "Memory Used: %.0f %s", current_usage, unit);
    float plots_width = ImGui::GetContentRegionAvailWidth();
    ImGui::PlotLines("###MemoryUsage", float_mem_allocs, IM_ARRAYSIZE(float_mem_allocs), 0, overlay, 0.0f, plot_scale, ImVec2(plots_width, 100.0f));

    // Stop Memory Metrics Gathering
    static bool stop = false;
    ImGui::Checkbox("Stop Metrics Count", &stop);
    if (!stop)
        SetMemoryMetrics();

    // Print other useful info.
    float text_separation = ImGui::GetContentRegionAvailWidth() / 3.0f + 25.0f;
    ImGui::NewLine();

    ImGui::Text("Allocations"); ImGui::SameLine(text_separation);
    ImGui::Text("%.0f KB", BYTETOKB(m_MemoryMetrics.GetAllocations()));

    ImGui::Text("Deallocations"); ImGui::SameLine(text_separation);
    ImGui::Text("%.0f KB", BYTETOKB(m_MemoryMetrics.GetDeallocations()));

    ImGui::End();



    // --- Renderer Statistics Display ---
    ImGui::Begin("Renderer Statistics");
    RendererStatistics stats = Renderer::GetStatistics();
    ImGui::Text("- Graphics by %s -", stats.GLVendor.c_str());
    ImGui::Text("Graphics Card:     %s", stats.GraphicsCard.c_str());
    ImGui::Text("OpenGL Version:    %i.%i (%s)", stats.OGL_MajorVersion, stats.OGL_MinorVersion, stats.GLVersion.c_str());
    ImGui::Text("Shading Version:   GLSL %s", stats.GLShadingVersion.c_str());
    ImGui::End();
}


void Sandbox::SetMemoryMetrics()
{
    m_MemoryMetrics = Application::Get().GetMemoryMetrics();

    if (m_AllocationsIndex == ALLOCATIONS_SAMPLES)
        m_AllocationsIndex = 0;

    m_MemoryAllocations[m_AllocationsIndex] = m_MemoryMetrics.GetCurrentMemoryUsage();
    ++m_AllocationsIndex;
}