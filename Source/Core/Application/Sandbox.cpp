#include "Sandbox.h"

#include "Core/Platform/Input.h"
#include "Core/Resources/Resources.h"
#include "Core/Utils/FileStringUtils.h"

#include "Renderer/Renderer.h"
#include "Renderer/Utils/RenderCommand.h"
#include "Renderer/Utils/RendererPrimitives.h"

#include "EditorUI.h"

#include <imgui.h>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Renderer/Entities/Lights.h"



// ------------------------------------------------------------------------------
void Sandbox::Init()
{
    // -- Buffers Test --
    uint indices[6] = { 0, 1, 2, 2, 3, 0 };
    float vertices[5 * 4] = {
        -1.0f,	-1.0f,	0.0f, 0.0f, 0.0f,		// For negative X positions, UV should be 0, for positive, 1
         1.0f,	-1.0f,	0.0f, 1.0f, 0.0f,		// If you render, on a square, the texCoords (as color = vec4(tC, 0, 1)), the colors of the square in its corners are
         1.0f,	 1.0f,	0.0f, 1.0f, 1.0f,		// (0,0,0,1) - Black, (1,0,0,1) - Red, (1,1,0,0) - Yellow, (0,1,0,1) - Green
        -1.0f,	 1.0f,	0.0f, 0.0f, 1.0f
    };
    
    BufferLayout layout = { { SHADER_DATA::FLOAT3, "a_Position" }, { SHADER_DATA::FLOAT2, "a_TexCoord" } };
    Ref<VertexBuffer> vbo = CreateRef<VertexBuffer>(vertices, sizeof(vertices));
    Ref<IndexBuffer> ibo = CreateRef<IndexBuffer>(indices, sizeof(indices) / sizeof(uint));
    
    m_QuadArray = CreateRef<VertexArray>();

    vbo->SetLayout(layout);
    m_QuadArray->AddVertexBuffer(vbo);
    m_QuadArray->SetIndexBuffer(ibo);
    m_QuadArray->Unbind(); vbo->Unbind(); ibo->Unbind();


    // --- Skybox VArray & Texture ---
    BufferLayout skyb_layout = { {SHADER_DATA::FLOAT3, "a_Position"} };
    Ref<VertexBuffer> skyb_vbo = CreateRef<VertexBuffer>(RendererPrimitives::SkyboxVertices, sizeof(RendererPrimitives::SkyboxVertices));
    skyb_vbo->SetLayout(skyb_layout);

    m_SkyboxVArray = CreateRef<VertexArray>();
    m_SkyboxVArray->AddVertexBuffer(skyb_vbo);
    m_SkyboxVArray->Unbind(); skyb_vbo->Unbind();

    m_SkyboxTexture = CreateRef<CubemapTexture>();


    // -- Engine Camera Startup --glm::vec3(14.0f, 15.5f, 18.8f)
    m_EngineCamera.ZoomLevel = 30.0f;
    m_EngineCamera.SetPosition(glm::vec3(10.0f, 12.0f, -5.0f) - m_EngineCamera.GetForwardVector() * m_EngineCamera.ZoomLevel);
    m_EngineCamera.SetOrientation(0.24f, -0.41f);


    // -- Models Setup --
    Ref<Model> bandit_model = Resources::CreateModel("Resources/Models/TWTODBandit/Bandit_ToTest.obj");
    bandit_model->GetTransformation().Scale = glm::vec3(0.1f);

    Ref<Model> patrick_model = Resources::CreateModel("Resources/Models/Patrick/Patrick.obj");
    patrick_model->GetTransformation().Translation = glm::vec3(-3.5f, 3.5f, 3.5f);
    patrick_model->GetTransformation().Scale = glm::vec3(1.0f);
    
    Ref<Model> patrick_model2 = Resources::CreateModel(patrick_model, "Patrick2");
    patrick_model2->GetTransformation().Translation = glm::vec3(3.5f);

    Ref<Model> plane_model = Resources::CreateModel("Resources/Models/Plane/Plane_Ground.obj");
    plane_model->GetTransformation().Scale = glm::vec3(0.1f);

    m_SceneModels.push_back(plane_model);
    m_SceneModels.push_back(bandit_model);
    m_SceneModels.push_back(patrick_model);
    m_SceneModels.push_back(patrick_model2);

    // -- Shaders --
    m_SkyboxShader = CreateRef<Shader>("Resources/Shaders/SkyboxShader.glsl");
    m_TextureShader = CreateRef<Shader>("Resources/Shaders/TexturedShader.glsl");
    m_LightingShader = CreateRef<Shader>("Resources/Shaders/LightingShader.glsl");
    m_DeferredLightingShader = CreateRef<Shader>("Resources/Shaders/DeferredLightingShader.glsl");
    m_BlurShader = CreateRef<Shader>("Resources/Shaders/BlurShader.glsl");
    m_FinalBloomShader = CreateRef<Shader>("Resources/Shaders/BloomEffectShader.glsl");

    // -- Framebuffer --
    m_EditorFramebuffer = CreateRef<Framebuffer>(new Framebuffer(WINDOW_WIDTH, WINDOW_HEIGHT,
                                                    {   RendererUtils::FBO_TEXTURE_FORMAT::RGBA8,           // Color Attachment
                                                        RendererUtils::FBO_TEXTURE_FORMAT::RGBA32,          // Normal Attachment
                                                        RendererUtils::FBO_TEXTURE_FORMAT::RGBA32,          // Position Attachment
                                                        RendererUtils::FBO_TEXTURE_FORMAT::RGBA32,          // Smoothness Attachment (LOL, pretty much ugly)
                                                        RendererUtils::FBO_TEXTURE_FORMAT::RGBA32,          // Depth Attachment (ugly too)
                                                        RendererUtils::FBO_TEXTURE_FORMAT::DEPTH }));       // Depth

    m_DeferredFramebuffer = CreateRef<Framebuffer>(new Framebuffer(WINDOW_WIDTH, WINDOW_HEIGHT, { RendererUtils::FBO_TEXTURE_FORMAT::RGBA8, RendererUtils::FBO_TEXTURE_FORMAT::RGBA32 }));
    m_BlurPingPongFramebuffer[0] = CreateRef<Framebuffer>(new Framebuffer(WINDOW_WIDTH, WINDOW_HEIGHT, { RendererUtils::FBO_TEXTURE_FORMAT::RGBA16 }));
    m_BlurPingPongFramebuffer[1] = CreateRef<Framebuffer>(new Framebuffer(WINDOW_WIDTH, WINDOW_HEIGHT, { RendererUtils::FBO_TEXTURE_FORMAT::RGBA16 }));
    m_BlurFinalFramebuffer = CreateRef<Framebuffer>(new Framebuffer(WINDOW_WIDTH, WINDOW_HEIGHT, { RendererUtils::FBO_TEXTURE_FORMAT::RGBA16 }));

    // -- Resources Print --
    Resources::PrintResourcesReferences();

    // -- Measurement Timer --
    m_MeasureTime.Start();
}


void Sandbox::OnMouseScrollEvent(float scroll)
{
    m_EngineCamera.OnMouseScroll(scroll, (m_ViewportFocused || m_ViewportHovered));
}

void Sandbox::OnWindowResizeEvent(uint width, uint height)
{
    m_EngineCamera.SetCameraViewport(width, height);
}



// ------------------------------------------------------------------------------
void Sandbox::OnUpdate(float dt)
{
    // -- Shader Hot Reload --
    //m_LightingShader->CheckLastModification();

    bool rendering_measure = false;
    if (m_MeasureTime.GetSeconds() > 1.0f)
    {
        rendering_measure = true;
        m_MeasureTime.Start();
    }

    // -- Viewport Resize --
    if (m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f)
    {
        if (m_EditorFramebuffer->GetWidth() != (uint)m_ViewportSize.x || m_EditorFramebuffer->GetHeight() != (uint)m_ViewportSize.y)
        {
            m_EditorFramebuffer->Resize((uint)m_ViewportSize.x, (uint)m_ViewportSize.y);
            m_EngineCamera.SetCameraViewport(m_ViewportSize.x, m_ViewportSize.y);
        }
    }

    // -- Camera Update --
    m_EngineCamera.OnUpdate(dt, (m_ViewportFocused || m_ViewportHovered));

    // -- Measure Rendering --
    if (rendering_measure)
    {
        if (m_DeferredRendering)
        {
            m_FwRendTimer.Stop();
            m_DefRendTimer.Start();
        }
        else
        {
            m_DefRendTimer.Stop();
            m_FwRendTimer.Start();
        }
    }

    // -- Render Geometry --
    m_EditorFramebuffer->Bind();
    Renderer::ClearRenderer();
    Renderer::SetSceneData(m_EngineCamera.GetCamera().GetViewProjection(), m_EngineCamera.GetPosition());

    Ref<Shader> shader = m_TextureShader;
    bool set_directionals = false;

    if (!m_DeferredRendering)
    {
        shader = m_LightingShader;
        set_directionals = true;
    }

    // Skybox
    if (m_RenderSkybox && !m_DeferredRendering)
        RenderSkybox();

    Renderer::BeginScene(shader, set_directionals);
    
    // Draw Calls
    for (auto& model : m_SceneModels)
        Renderer::SubmitModel(shader, model);

    // Draw Lights Spheres
    if (m_DrawLightsSpheres)
        Renderer::DrawLightsSpheres(shader);

    // End Scene
    Renderer::EndScene(shader);    

    m_EditorFramebuffer->Unbind();

    if (m_DeferredRendering)
    {
        // -- Render Lighting --
        m_DeferredFramebuffer->Bind();
        Renderer::ClearRenderer();

        // Skybox
        if (m_RenderSkybox)
            RenderSkybox();

        Renderer::BeginScene(m_DeferredLightingShader, true);

        // Attach & Send GBuffer Textures
        RenderCommand::AttachDeferredTexture(m_EditorFramebuffer->GetFBOTextureID(0), 0);
        RenderCommand::AttachDeferredTexture(m_EditorFramebuffer->GetFBOTextureID(1), 1);
        RenderCommand::AttachDeferredTexture(m_EditorFramebuffer->GetFBOTextureID(2), 2);
        RenderCommand::AttachDeferredTexture(m_EditorFramebuffer->GetFBOTextureID(3), 3);

        m_DeferredLightingShader->SetUniformInt("u_gColor", 0);
        m_DeferredLightingShader->SetUniformInt("u_gNormal", 1);
        m_DeferredLightingShader->SetUniformInt("u_gPosition", 2);
        m_DeferredLightingShader->SetUniformInt("u_gSmoothness", 3);

        // Draw Deferred Quad
        Renderer::Submit(m_DeferredLightingShader, m_QuadArray);

        // Detach GBuffer Textures
        RenderCommand::DettachDeferredTexture();
        RenderCommand::DettachDeferredTexture();
        RenderCommand::DettachDeferredTexture();
        RenderCommand::DettachDeferredTexture();

        // End Scene
        Renderer::EndScene(m_DeferredLightingShader);
        m_DeferredFramebuffer->Unbind();
    }

    if (rendering_measure)
    {
        m_DefRendTimer.Stop();
        m_FwRendTimer.Stop();
        rendering_measure = false;
    }

    // Blur
    if (m_BloomActive)
    {
        bool horizontal = true, first_iteration = true;
        uint texture_to_use = m_DeferredRendering ? m_DeferredFramebuffer->GetFBOTextureID(1) : m_EditorFramebuffer->GetFBOTextureID(1);

        m_BlurShader->Bind();
        m_BlurShader->SetUniformInt("u_BlurAmount", m_BloomBlurAmount);

        for (uint i = 0; i < m_BloomBlurIterations; ++i)
        {
            m_BlurPingPongFramebuffer[horizontal]->Bind();
            m_BlurShader->SetUniformInt("u_HorizontalPass", horizontal);

            glBindTexture(GL_TEXTURE_2D, first_iteration ? texture_to_use : m_BlurPingPongFramebuffer[!horizontal]->GetFBOTextureID());

            Renderer::Submit(m_BlurShader, m_QuadArray);
            horizontal = !horizontal;
            if (first_iteration)
                first_iteration = false;
        }

        //glBindTexture(GL_TEXTURE_2D, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        //m_BlurPingPongFramebuffer->Unbind();
        //m_BlurShader->Unbind();

        Renderer::ClearRenderer();
        m_BlurFinalFramebuffer->Bind();
        m_FinalBloomShader->Bind();

        texture_to_use = m_DeferredRendering ? m_DeferredFramebuffer->GetFBOTextureID() : m_EditorFramebuffer->GetFBOTextureID();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture_to_use);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_BlurPingPongFramebuffer[!horizontal]->GetFBOTextureID());
        
        m_FinalBloomShader->SetUniformFloat("u_BloomExposure", m_BloomExposure);
        m_FinalBloomShader->SetUniformFloat("u_HDRGamma", m_BloomHDRGamma);
        m_FinalBloomShader->SetUniformInt("u_GammaCorrection", m_GammaCorrection);
        m_FinalBloomShader->SetUniformInt("u_ToneMapping", m_ToneMapping);
        Renderer::Submit(m_FinalBloomShader, m_QuadArray);

        m_FinalBloomShader->Unbind();
        m_BlurFinalFramebuffer->Unbind();
    }
}


void Sandbox::RenderSkybox()
{
    RenderCommand::SetCubemapSeamless(true);
    glDepthFunc(GL_LEQUAL);

    m_SkyboxShader->Bind();
    glm::mat4 view = glm::mat4(glm::mat3(m_EngineCamera.GetCamera().GetView()));
    glm::mat4 proj = m_EngineCamera.GetCamera().GetProjection();
    glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    m_SkyboxShader->SetUniformMat4("u_View", view);
    m_SkyboxShader->SetUniformMat4("u_Proj", proj);
    m_SkyboxShader->SetUniformMat4("u_Model", model);
    m_SkyboxShader->SetUniformVec3("u_TintColor", m_SkyboxTint);

    glBindTexture(GL_TEXTURE_CUBE_MAP, m_SkyboxTexture->GetTextureID());
    Renderer::DrawSkyboxCubemap(m_SkyboxVArray, 36);

    m_SkyboxShader->Unbind();
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    glDepthFunc(GL_LESS);
    RenderCommand::SetCubemapSeamless(false);
}



// ------------------------------------------------------------------------------
void Sandbox::OnUIRender(float dt)
{
    // -- Docking Space --
    EditorUI::SetDocking();    

    // -- GBuffer Framebuffer --
    ImGui::Begin("GBuffer View", (bool*)true, ImGuiWindowFlags_NoScrollbar);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));

    // Get viewport size & draw fbo texture
    ImVec2 viewportpanel_size = ImGui::GetContentRegionAvail();
    static uint gbtexture_index = 0;
    
    if (m_DeferredRendering)
        ImGui::Image((ImTextureID)(m_EditorFramebuffer->GetFBOTextureID(gbtexture_index)), viewportpanel_size, ImVec2(0, 1), ImVec2(1, 0));
    else
    {
        std::string text = "Deferred Rendering Needs to be Active to Display the GBuffer!";
        float font_size = ImGui::GetFontSize() * text.size() / 2;

        ImGui::NewLine(); ImGui::NewLine(); ImGui::NewLine(); ImGui::NewLine(); ImGui::NewLine();
        ImGui::SameLine(ImGui::GetWindowSize().x / 2 - font_size + (font_size / 2));
        ImGui::TextColored(ImVec4(1.0f, 0.25f, 0.25f, 1.0f), text.c_str());
    }

    ImGui::PopStyleVar();
    ImGui::End();

    // --- Scene Framebuffer ---
    ImGui::Begin("Scene", (bool*)true, ImGuiWindowFlags_NoScrollbar);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));

    if (ImGui::IsWindowHovered() && (ImGui::IsMouseClicked(ImGuiMouseButton_Right) || ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Middle)))
        ImGui::SetWindowFocus();

    m_ViewportFocused = ImGui::IsWindowFocused();
    m_ViewportHovered = ImGui::IsWindowHovered();

    // Get viewport size & draw fbo texture
    viewportpanel_size = ImGui::GetContentRegionAvail();
    m_ViewportSize = glm::vec2(viewportpanel_size.x, viewportpanel_size.y);
    
    //ImGui::Image((ImTextureID)(m_EditorFramebuffer->GetFBOTextureID(texture_index)), viewportpanel_size, ImVec2(0, 1), ImVec2(1, 0));
    static uint displaytexture_index = 0;
    static bool render_bloomblurrtexture = false;
    if(render_bloomblurrtexture)
        ImGui::Image((ImTextureID)(m_BlurPingPongFramebuffer[1]->GetFBOTextureID()), viewportpanel_size, ImVec2(0, 1), ImVec2(1, 0));
    else
    {
        if (m_BloomActive)
            ImGui::Image((ImTextureID)(m_BlurFinalFramebuffer->GetFBOTextureID()), viewportpanel_size, ImVec2(0, 1), ImVec2(1, 0));
        else
        {
            if (m_DeferredRendering)
                ImGui::Image((ImTextureID)(m_DeferredFramebuffer->GetFBOTextureID(displaytexture_index)), viewportpanel_size, ImVec2(0, 1), ImVec2(1, 0));
            else
                ImGui::Image((ImTextureID)(m_EditorFramebuffer->GetFBOTextureID(displaytexture_index)), viewportpanel_size, ImVec2(0, 1), ImVec2(1, 0));
        }
    }

    ImGui::PopStyleVar();
    ImGui::End();

    // --- Performance Info Display ---
    ImGui::Begin("Info");
    ImGui::Text("FPS: %.0f", 1.0f / dt);
    DrawPerformancePanel();
    ImGui::End();

    // --- Resources Info Display ---
    ImGui::Begin("Resources");
    std::vector<std::string> text = Resources::GetResourcesReferences();
    for (std::string str : text)
        ImGui::Text(str.c_str());
    ImGui::End();

    // --- Entities Display ---
    ImGui::Begin("Entities");
    DrawEntitiesPanel();
    ImGui::End();

    // --- Camera Settings ---
    ImGui::Begin("Camera");
    DrawCameraPanel();
    ImGui::End();

    ImGui::Begin("Lights");
    DrawLightsPanel();
    ImGui::End();


    // --- Renderer Statistics Display ---
    ImGui::Begin("Renderer");
    RendererStatistics stats = Renderer::GetStatistics();

    ImGui::PushTextWrapPos(ImGui::GetContentRegionAvailWidth());
    ImGui::Text("- Graphics by %s -", stats.GLVendor.c_str()); ImGui::NewLine();
    ImGui::Text("Graphics Card:     %s", stats.GraphicsCard.c_str()); ImGui::NewLine();
    ImGui::Text("OpenGL Version:    %i.%i (%s)", stats.OGL_MajorVersion, stats.OGL_MinorVersion, stats.GLVersion.c_str()); ImGui::NewLine();
    ImGui::Text("Shading Version:   GLSL %s", stats.GLShadingVersion.c_str()); ImGui::NewLine();
    ImGui::PopTextWrapPos();
    
    ImGui::Separator();
    ImGui::NewLine();
    ImGui::Checkbox("Draw Light Spheres", &m_DrawLightsSpheres);
    //ImGui::NewLine();
    //ImGui::Text("Last Measured Deferred Rendering: %.2f ms", m_DefRendTimer.GetMilliseconds());
    //ImGui::Text("Last Measured Forward Rendering: %.2f ms", m_FwRendTimer.GetMilliseconds());

    // Rendering Type Dropdown
    const char* rendering_options[] = { "Forward", "Deferred" };
    const char* current_rend_option = rendering_options[(int)m_DeferredRendering];

    ImGui::NewLine();
    if (ImGui::BeginCombo("Rendering Type", current_rend_option))
    {
        for (uint i = 0; i < 2; ++i)
        {
            bool selected = current_rend_option == rendering_options[i];
            if (ImGui::Selectable(rendering_options[i], selected))
            {
                current_rend_option = rendering_options[i];
                m_DeferredRendering = (bool)i;
            }

            if (selected)
                ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
    }


    // GBuffer Renderer Dropdown
    uint current_gbtexture = gbtexture_index;
    const char* gbt_options[] = { "Colors", "Normals", "Positions", "Mat. Smoothness/Specular", "Depth"  };
    const char* current_gbt_option = gbt_options[current_gbtexture];

    if (ImGui::BeginCombo("GBuffer Texture Display", current_gbt_option))
    {
        for (uint i = 0; i < 5; ++i)
        {
            bool selected = current_gbt_option == gbt_options[i];
            if (ImGui::Selectable(gbt_options[i], selected))
            {
                current_gbt_option = gbt_options[i];
                gbtexture_index = i;
            }

            if (selected)
                ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
    }

    // Bloom Settings
    ImGui::NewLine(); ImGui::NewLine(); ImGui::Separator();
    ImGui::Text(" - BLOOM EFFECT -");
    ImGui::Checkbox("Bloom Active", &m_BloomActive);

    static bool displaytexture_index_bool = false;
    ImGui::Checkbox("Display Brightness Texture (Disable Bloom!)", &displaytexture_index_bool);
    displaytexture_index = (uint)displaytexture_index_bool;

    if (m_BloomActive)
    {
        ImGui::Checkbox("Display Brightness Blurred Texture", &render_bloomblurrtexture);

        //EditorUI::DrawDragFloat("Bloom Exposure", "###bloom_exp", &m_BloomExposure, 20.0f, ImGui::GetContentRegionAvailWidth() / 3.0f, 0.01f, 0.1f, 3.0f);
        //EditorUI::DrawDragFloat("Bloom HDR Gamma", "###bloom_hdrgamma", &m_BloomHDRGamma, 20.0f, ImGui::GetContentRegionAvailWidth() / 3.0f, 0.01f, 0.1f, 3.0f);

        ImGui::Checkbox("Bloom Gamma Correction", &m_GammaCorrection);
        if (m_GammaCorrection)
        {
            ImGui::SameLine();
            ImGui::DragFloat("###bloomgamma", &m_BloomHDRGamma, 0.01f, 0.1f, 3.0f, "%.2f");
        }

        ImGui::Checkbox("Bloom Tone Mapping", &m_ToneMapping);
        if (m_ToneMapping)
        {
            ImGui::SameLine();
            ImGui::DragFloat("###bloomexposure", &m_BloomExposure, 0.01f, 0.1f, 3.0f, "%.2f");
        }

        ImGui::DragInt("Bloom Blur Iterations", &m_BloomBlurIterations, 1.0f, 0, 500);
        ImGui::DragInt("Bloom Blur Amount", &m_BloomBlurAmount, 1.0f, 0, 500);
    }

    // Skybox Settings
    ImGui::NewLine(); ImGui::NewLine(); ImGui::Separator();
    ImGui::Text(" - SKYBOX -");
    ImGui::Checkbox("Skybox Active", &m_RenderSkybox);
    ImGui::ColorEdit3("Skybox Tint", glm::value_ptr(m_SkyboxTint), ImGuiColorEditFlags_NoInputs);

    // End UI Draw
    ImGui::End();
}




// ------------------------------------------------------------------------------
void Sandbox::SetMemoryMetrics()
{
    m_MemoryMetrics = Application::Get().GetMemoryMetrics();

    if (m_AllocationsIndex == ALLOCATIONS_SAMPLES)
        m_AllocationsIndex = 0;

    m_MemoryAllocations[m_AllocationsIndex] = m_MemoryMetrics.GetCurrentMemoryUsage();
    ++m_AllocationsIndex;
}



// ------------------------------------------------------------------------------
void Sandbox::DrawCameraPanel()
{
    // -- Transform Values --
    ImGui::Text("SPEED");
    float slider_indent = ImGui::GetContentRegionAvailWidth() / 3.0f;
    float indent = ImGui::GetContentRegionAvailWidth() / 10.0f;
    
    EditorUI::DrawSlider("Mov.", "##EdcamMovSp", &m_EngineCamera.MoveSpeed, indent, slider_indent, 1.0f, 10.0f);
    EditorUI::DrawSlider("Rot.", "##EdcamRotSp", &m_EngineCamera.RotationSpeed, indent, slider_indent, 0.1f, 5.0f);
    EditorUI::DrawSlider("Pan", "##EdcamPanSp", &m_EngineCamera.PanSpeed, indent, slider_indent, 1.0f, 20.0f);
    EditorUI::DrawSlider("Max Zoom", "##EdcamZoomSp", &m_EngineCamera.MaxZoomSpeed, indent, slider_indent, 1.0f, 300.0f);

    ImGui::NewLine(); ImGui::SameLine(indent);
    ImGui::Text("Zoom: %.1f", m_EngineCamera.ZoomLevel);

    // -- Camera --
    ImGui::NewLine(); ImGui::Separator(); ImGui::NewLine();

    // -- Transform Values --
    ImGui::Text("CAMERA");

    float fov = m_EngineCamera.GetCamera().GetFOV();
    float nplane = m_EngineCamera.GetCamera().GetNearPlane();
    float fplane = m_EngineCamera.GetCamera().GetFarPlane();
    
    // FOV
    if (EditorUI::DrawSlider("FOV", "##EdcamFOV", &fov, indent, slider_indent, 15.0f, 180.0f))
        m_EngineCamera.GetCamera().SetFOV(fov);
    
    // Cam Planes
    ImGui::NewLine(); ImGui::SameLine(indent);
    ImGui::Text("Planes"); ImGui::SameLine(slider_indent);
    float width = ImGui::GetContentRegionAvailWidth() / 1.3f;

    EditorUI::SetItemWidth(width/2.1f);
    if (ImGui::DragFloat("##EdcamNPl", &nplane, 0.01f, 0.01f, fplane - 0.01f, "%.3f"))
        m_EngineCamera.GetCamera().SetNearPlane(nplane);

    ImGui::SameLine(); EditorUI::SetItemWidth(width/2.1f);
    if (ImGui::DragFloat("##EdcamFPl", &fplane, 1.0f, nplane + 0.1f, INFINITY, "%.3f"))
        m_EngineCamera.GetCamera().SetFarPlane(fplane);

    // Resolution/Viewport
    glm::ivec2 view_size = m_EngineCamera.GetCamera().GetViewportSize();
    ImGui::NewLine(); ImGui::SameLine(indent);
    ImGui::Text("Viewport"); ImGui::SameLine(slider_indent);
    ImGui::Text("%ix%i (AR: %.2f)", view_size.x, view_size.y, m_EngineCamera.GetCamera().GetAspectRato());

    // Pos & Rot
    glm::vec2 rot = { m_EngineCamera.GetPitch(), m_EngineCamera.GetYaw() };
    glm::vec3 pos = m_EngineCamera.GetPosition();

    ImGui::NewLine(); ImGui::Separator(); ImGui::NewLine();
    ImGui::Text("PLACEMENT");

    ImGui::NewLine(); ImGui::SameLine(indent);
    ImGui::Text("Rot"); ImGui::SameLine(slider_indent);

    EditorUI::SetItemWidth(width);
    if (ImGui::DragFloat2("###EdcamRot", &rot[0], 0.01f))
        m_EngineCamera.SetOrientation(rot.x, rot.y);

    ImGui::NewLine(); ImGui::SameLine(indent);
    ImGui::Text("Pos"); ImGui::SameLine(slider_indent);
    EditorUI::SetItemWidth(width);
    if (ImGui::DragFloat3("###EdcamPos", &pos[0], 0.05f))
        m_EngineCamera.SetPosition(pos);
}


void Sandbox::DrawLightsPanel()
{
    // -- Directional Light --
    Light& dir_light = Renderer::GetDirectionalLight();
    ImGui::NewLine();
    ImGui::Text("Directional Light");
    ImGui::SameLine(150.0f);

    ImGui::ColorEdit3("##LightColor", glm::value_ptr(dir_light.Color), ImGuiColorEditFlags_NoInputs);
    EditorUI::DrawSlider("Int.", "##DirLightInts", &dir_light.Intensity, 52.0f, ImGui::GetContentRegionAvailWidth() / 3.0f, 0.1f, 5.0f);
    EditorUI::DrawVec3Control("Dir.", "##DLightPos", 52.0f, dir_light.Direction, glm::vec3(1.0f), ImGui::GetContentRegionAvailWidth() / 3.0f, 56.0f);

    ImGui::NewLine(); ImGui::NewLine(); ImGui::Separator(); ImGui::Separator();

    // -- Lighting Info --
    ImGui::NewLine();
    float btn_width = 88.5f;
    float half_avail_width = ImGui::GetContentRegionAvailWidth() / 2.0f;
    float text_size = ImGui::GetFontSize() * 14.0f/2.0f;

    ImGui::SameLine(half_avail_width - btn_width / 2.0f);
    if (ImGui::Button("Add Light", { btn_width, 20.0f }))
        Renderer::AddLight();

    ImGui::NewLine();
    ImGui::SameLine(half_avail_width - text_size + (text_size / 2.0f));
    ImGui::Text("Max Lights: %i", RendererUtils::s_MaxLights);
    ImGui::NewLine(); ImGui::Separator(); ImGui::NewLine();

    // -- Lights --
    uint i = 0;
    for (PointLight& light : Renderer::GetLights())
    {
        static char popup_id[16];
        sprintf_s(popup_id, 16, "LightTag_%i", i);
        ImGui::PushID(popup_id);

        // -- Light Num., Active & Color --
        ImGui::Text("%i.  ", i); ImGui::SameLine();
        ImGui::Checkbox("##LightActive", &light.Active);
                
        ImGui::SameLine(100.0f); ImGui::Text("Color"); ImGui::SameLine();
        ImGui::ColorEdit3("##LightColor", glm::value_ptr(light.Color), ImGuiColorEditFlags_NoInputs);

        float btn_width = 44.5f;
        float avail_width = ImGui::GetContentRegionAvailWidth() - 2.0f * btn_width;
        ImGui::SameLine(avail_width);
        
        // -- Light Remove --
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.1f, 0.1f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.2f, 0.2f, 1.0f));
        
        if (ImGui::Button("X", { btn_width , 20.0f }))
            Renderer::RemoveLight(light.GetID());

        ImGui::PopStyleColor(2);
        

        // -- Light Position --
        float indent = 52.0f, slider_indent = ImGui::GetContentRegionAvailWidth() / 3.0f;
        ImGui::NewLine();
        EditorUI::DrawVec3Control("Pos", "##LightPos", indent, light.Position, glm::vec3(0.0f, 1.0f, 2.0f), slider_indent, 56.0f);

        // -- Light Values --
        slider_indent = ImGui::GetContentRegionAvailWidth() / 3.0f;
        EditorUI::DrawSlider("Int.", "##LightInts", &light.Intensity, indent, slider_indent, 0.1f, 5.0f);
                
        ImGui::NewLine(); ImGui::SameLine(indent);
        ImGui::Text("Att. KLQ"); ImGui::SameLine(slider_indent);

        float width = (ImGui::GetContentRegionAvailWidth() / 1.3f) / 3.25f;
        EditorUI::SetItemWidth(width);
        ImGui::DragFloat("##LightAttK", &light.AttenuationK, 0.01f, 0.01f, 10.0f, "%.2f");

        ImGui::SameLine(); EditorUI::SetItemWidth(width);
        ImGui::DragFloat("##LightAttQ", &light.AttenuationQ, 0.001f, 0.001f, 4.0f, "%.3f");
        
        ImGui::SameLine(); EditorUI::SetItemWidth(width);
        ImGui::DragFloat("##LightAttL", &light.AttenuationL, 0.0001f, 0.001f, 4.0f, "%.3f");

        // -- Pop & Spacing --
        ImGui::PopID();
        ImGui::NewLine(); ImGui::Separator(); ImGui::NewLine();
        ++i;
    }
}


void Sandbox::DrawMeshMaterials(const Mesh* mesh, std::vector<uint>& materials_shown, uint meshindex_uitexturebtn)
{
    for (uint i = 0; i < mesh->GetSubmeshes()->size(); ++i)
        DrawMeshMaterials(mesh->GetSubmeshes()->at(i).get(), materials_shown, meshindex_uitexturebtn + i + 1);

    std::vector<uint>::iterator it = std::find(materials_shown.begin(), materials_shown.end(), mesh->GetMaterialIndex());
    if (it == materials_shown.end())
    {
        // -- Material Name --
        materials_shown.push_back(mesh->GetMaterialIndex());
        Ref<Material> mat = Resources::GetMaterial(mesh->GetMaterialIndex());
        ImGui::NewLine(); ImGui::Text("MATERIAL %i: '%s'", mat->GetID(), mat->GetName().c_str());
        ImGui::SameLine(); ImGui::Checkbox("Two Sided", &mat->IsTwoSided); ImGui::NewLine();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 20.0f);

        // -- Albedo Color --
        ImGui::PushID(meshindex_uitexturebtn);
        ImGui::Text("Color"); ImGui::SameLine();
        ImGui::ColorEdit4("##MatAlbColor", glm::value_ptr(mat->AlbedoColor), ImGuiColorEditFlags_NoInputs);

        // -- Albedo Texture --
        ImVec2 btn_size = ImVec2(20.0f, 20.0f);
        EditorUI::DrawTextureButton(mat->Albedo, "Albedo", btn_size, meshindex_uitexturebtn, 0, 150.0f); ImGui::NewLine();
        EditorUI::DrawTextureButton(mat->Normal, "Normal", btn_size, meshindex_uitexturebtn, 1);
        EditorUI::DrawTextureButton(mat->Bump, "Bump", btn_size, meshindex_uitexturebtn, 2, 164.0f);

        // -- Sliders --
        std::string sm_str = std::string("###smoothness" + std::to_string(meshindex_uitexturebtn));
        EditorUI::DrawSlider("Smoothness", sm_str.c_str(), &mat->Smoothness, 20.0f, ImGui::GetContentRegionAvailWidth() / 3.0f, 0.1f, 1.0f);
        std::string bmp_str = std::string("###bumpscale" + std::to_string(meshindex_uitexturebtn));
        EditorUI::DrawDragFloat("Bumpiness", bmp_str.c_str(), &mat->Bumpiness, 20.0f, ImGui::GetContentRegionAvailWidth() / 3.0f, 0.01f, 0.1f, 5.0f);
        std::string heightsc_str = std::string("###heightscale" + std::to_string(meshindex_uitexturebtn));
        EditorUI::DrawSlider("Bump Height", heightsc_str.c_str(), &mat->Heightscale, 20.0f, ImGui::GetContentRegionAvailWidth() / 3.0f, 0.1f, 5.0f);
        std::string pxlayers_str = std::string("###parxlayers" + std::to_string(meshindex_uitexturebtn));
        EditorUI::DrawDragFloat("Bump Layers", pxlayers_str.c_str(), &mat->ParallaxLayers, 20.0f, ImGui::GetContentRegionAvailWidth() / 3.0f, 1.0f, 10.0f, 1000.0f);

        ImGui::PopID();
    }
}


void Sandbox::DrawEntitiesPanel()
{
    uint i = 0;
    for (auto& entity : m_SceneModels)
    {
        // -- New ImGui ID --
        static char popup_id[16];
        sprintf_s(popup_id, 16, "EntityTag_%i", i);
        ImGui::PushID(popup_id);

        // -- Entity Active --
        ImGui::Checkbox("##EntActive", &entity->GetTransformation().EntityActive);
        
        // -- Entity Name --
        ImGui::SameLine();
        EditorUI::SetItemWidth(ImGui::GetContentRegionAvailWidth() / 1.5f);

        char buffer[256];
        memset(buffer, 0, sizeof(buffer));
        strncpy_s(buffer, entity->GetName().c_str(), sizeof(buffer));

        if (ImGui::InputText("##EntName", buffer, sizeof(buffer), ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_CharsNoBlank))
            entity->SetName(std::string(buffer));

        // -- Entity Transform --
        float indent = ImGui::GetContentRegionAvailWidth() / 5.0f - 10.0f;
        EditorUI::DrawVec3Control("Pos", "##Translation", indent, entity->GetTransformation().Translation);
        EditorUI::DrawVec3Control("Rot", "##Rotation", indent, entity->GetTransformation().Rotation, glm::vec3(0.0f, 180.0f, 0.0f));
        EditorUI::DrawVec3Control("Sca", "##Scale", indent, entity->GetTransformation().Scale, glm::vec3(0.25f));

        // -- Entity Materials --
        std::vector<uint> mats_shown_vec;
        DrawMeshMaterials(entity->GetRootMesh(), mats_shown_vec, i);

        // -- Pop & Spacing --
        ImGui::PopID();
        ImGui::NewLine(); ImGui::NewLine(); ImGui::NewLine(); ImGui::Separator(); ImGui::NewLine();
        ++i;
    }
}


void Sandbox::DrawPerformancePanel()
{
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
    EditorUI::DrawPlot(float_mem_allocs, plot_scale, true, "Memory Used: %.0f %s", current_usage, unit);

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
}