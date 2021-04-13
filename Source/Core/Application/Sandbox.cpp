#include "Sandbox.h"

#include "Application.h"
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



// ------------------------------------------------------------------------------
void Sandbox::OnUpdate(float dt)
{
    // -- Shader Hot Reload --
    m_TextureShader->CheckLastModification();

    // -- Render Stuff --
    m_TextureShader->Bind();
    m_TestTexture->Bind();

    m_TextureShader->SetUniformInt("u_Texture", 0);
    //m_TextureShader->SetUniformVec4("u_Color", { 0.6f, 0.2f, 0.2f, 1.0f });
    Renderer::Submit(m_TextureShader, m_VArray);

    m_TestTexture->Unbind();
    m_TextureShader->Unbind();
    m_VArray->Unbind();
}



// ------------------------------------------------------------------------------
void Sandbox::OnUIRender(float dt)
{
    ImGui::Begin("Info");
    ImGui::Text("FPS: %f", 1.0f/dt);
    ImGui::End();

    ImGui::Begin("Renderer Statistics");
    RendererStatistics stats = Renderer::GetStatistics();
    ImGui::Text("- Graphics by %s -", stats.GLVendor.c_str());
    ImGui::Text("Graphics Card:     %s", stats.GraphicsCard.c_str());
    ImGui::Text("OpenGL Version:    %i.%i (%s)", stats.OGL_MajorVersion, stats.OGL_MinorVersion, stats.GLVersion.c_str());
    ImGui::Text("Shading Version:   GLSL %s", stats.GLShadingVersion.c_str());
    ImGui::End();
}