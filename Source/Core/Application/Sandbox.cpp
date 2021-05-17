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

    // -- Shader --
    m_TextureShader = CreateRef<Shader>("Resources/Shaders/TexturedShader.glsl");
    m_LightingShader = CreateRef<Shader>("Resources/Shaders/LightingShader.glsl");    
    m_DeferredLightingShader = CreateRef<Shader>("Resources/Shaders/DeferredLightingShader.glsl");

    // -- Framebuffer --
    m_EditorFramebuffer = CreateRef<Framebuffer>(new Framebuffer(WINDOW_WIDTH, WINDOW_HEIGHT,
                                                    {   RendererUtils::FBO_TEXTURE_FORMAT::RGBA8,           // Color Attachment
                                                        RendererUtils::FBO_TEXTURE_FORMAT::RGBA32,          // Normal Attachment
                                                        RendererUtils::FBO_TEXTURE_FORMAT::RGBA32,          // Position Attachment
                                                        RendererUtils::FBO_TEXTURE_FORMAT::RGBA32,          // Smoothness Attachment (LOL, pretty much ugly)
                                                        RendererUtils::FBO_TEXTURE_FORMAT::RGBA32,          // Depth Attachment (ugly too)
                                                        RendererUtils::FBO_TEXTURE_FORMAT::DEPTH }));       // Depth

    m_DeferredFramebuffer = CreateRef<Framebuffer>(new Framebuffer(WINDOW_WIDTH, WINDOW_HEIGHT, { RendererUtils::FBO_TEXTURE_FORMAT::RGBA8 }));

    // -- Resources Print --
    Resources::PrintResourcesReferences();
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

    // -- Render Geometry --
    m_EditorFramebuffer->Bind();
    Renderer::ClearRenderer();
    Renderer::SetSceneData(m_EngineCamera.GetCamera().GetViewProjection(), m_EngineCamera.GetPosition());
    Renderer::BeginScene(m_TextureShader, false);
    
    // Draw Calls
    for (auto& model : m_SceneModels)
        Renderer::SubmitModel(m_TextureShader, model);

    // Draw Lights Spheres
    if (m_DrawLightsSpheres)
        Renderer::DrawLightsSpheres(m_TextureShader);

    // End Scene
    Renderer::EndScene(m_TextureShader);
    m_EditorFramebuffer->Unbind();

    // -- Render Lighting --
    m_DeferredFramebuffer->Bind();
    Renderer::ClearRenderer();
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



// ------------------------------------------------------------------------------
void Sandbox::OnUIRender(float dt)
{
    // -- Docking Space --
    EditorUI::SetDocking();    

    // -- GBuffer Framebuffer --
    ImGui::Begin("GBuffer View", (bool*)true, ImGuiWindowFlags_NoScrollbar);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));

    // Get viewport size & draw fbo texture
    static uint gbtexture_index = 0;
    ImVec2 viewportpanel_size = ImGui::GetContentRegionAvail();
    ImGui::Image((ImTextureID)(m_EditorFramebuffer->GetFBOTextureID(gbtexture_index)), viewportpanel_size, ImVec2(0, 1), ImVec2(1, 0));

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
    ImGui::Image((ImTextureID)(m_DeferredFramebuffer->GetFBOTextureID()), viewportpanel_size, ImVec2(0, 1), ImVec2(1, 0));

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
    
    ImGui::Checkbox("Draw Light Spheres", &m_DrawLightsSpheres);

    // GBuffer Renderer Dropdown
    uint current_gbtexture = gbtexture_index;
    const char* gbuffer_options[] = { "Colors", "Normals", "Positions", "Mat. Smoothness/Specular", "Depth"  };
    const char* current_option = gbuffer_options[current_gbtexture];

    if (ImGui::BeginCombo("GBuffer Texture Display", current_option))
    {
        for (uint i = 0; i < 5; ++i)
        {
            bool selected = current_option == gbuffer_options[i];
            if (ImGui::Selectable(gbuffer_options[i], selected))
            {
                current_option = gbuffer_options[i];
                gbtexture_index = i;
            }

            if (selected)
                ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
    }

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

        // -- Smoothness Slider --
        std::string sm_str = std::string("###smoothness" + std::to_string(meshindex_uitexturebtn));
        EditorUI::DrawSlider("Smoothness", sm_str.c_str(), &mat->Smoothness, 20.0f, ImGui::GetContentRegionAvailWidth() / 3.0f, 0.1f, 1.0f);
        std::string bmp_str = std::string("###bumpscale" + std::to_string(meshindex_uitexturebtn));
        EditorUI::DrawSlider("Bumpscale", bmp_str.c_str(), &mat->Bumpscale, 20.0f, ImGui::GetContentRegionAvailWidth() / 3.0f, 0.1f, 1.0f);
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
        std::strncpy(buffer, entity->GetName().c_str(), sizeof(buffer));

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