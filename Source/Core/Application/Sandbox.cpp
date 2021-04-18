#include "Sandbox.h"

#include "Core/Platform/Input.h"
#include "Core/Resources/Resources.h"

#include "Renderer/Renderer.h"
#include "Renderer/Utils/RenderCommand.h"
#include "Renderer/Utils/RendererPrimitives.h"

#include <imgui.h>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>



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
    Ref<VertexBuffer> vbo = CreateRef<VertexBuffer>(vertices, sizeof(vertices));
    Ref<IndexBuffer> ibo = CreateRef<IndexBuffer>(indices, sizeof(indices) / sizeof(uint));
    Ref<VertexArray> vao = CreateRef<VertexArray>();

    vbo->SetLayout(layout);
    vao->AddVertexBuffer(vbo);
    vao->SetIndexBuffer(ibo);
    vao->Unbind(); vbo->Unbind(); ibo->Unbind();

    // -- Models Setup --
    Ref<Model> patrick_model = Resources::CreateModel("Resources/Models/Patrick/Patrick.obj");
    patrick_model->GetTransformation().Scale = glm::vec3(0.25f);
    patrick_model->GetTransformation().Rotation = glm::vec3(0.0f, 180.0f, 0.0f);
    
    Ref<Model> patrick_model2 = Resources::CreateModel(patrick_model, "Patrick2");
    patrick_model2->GetTransformation().Translation = glm::vec3(1.0f, 0.0f, 0.0f);

    m_SceneModels.push_back(patrick_model);
    m_SceneModels.push_back(patrick_model2);


    // -- Shader --
    m_TextureShader = CreateRef<Shader>("Resources/Shaders/TexturedShader.glsl");

    // -- Resources Print --
    Resources::PrintResourcesReferences();
}



// ------------------------------------------------------------------------------
void Sandbox::OnUpdate(float dt)
{
    // -- Shader Hot Reload --
    m_TextureShader->CheckLastModification();

    // Draw Call
    for(auto& model : m_SceneModels)
        Renderer::SubmitModel(m_TextureShader, model);
}



// ------------------------------------------------------------------------------
void Sandbox::OnUIRender(float dt)
{
    // --- Performance Info Display ---
    ImGui::Begin("Info");
    ImGui::Text("FPS: %.0f", 1.0f / dt);
    DrawPerformancePanel();
    ImGui::End();

    // --- Resources Info Display ---
    ImGui::Begin("Resources");
    DrawResourcesPanel();
    ImGui::End();

    // --- Entities Display ---
    ImGui::Begin("Entities");
    DrawEntitiesPanel();
    ImGui::End();


    // --- Renderer Statistics Display ---
    ImGui::Begin("Renderer Statistics");
    RendererStatistics stats = Renderer::GetStatistics();

    ImGui::PushTextWrapPos(ImGui::GetContentRegionAvailWidth());
    ImGui::Text("- Graphics by %s -", stats.GLVendor.c_str()); ImGui::NewLine();
    ImGui::Text("Graphics Card:     %s", stats.GraphicsCard.c_str()); ImGui::NewLine();
    ImGui::Text("OpenGL Version:    %i.%i (%s)", stats.OGL_MajorVersion, stats.OGL_MinorVersion, stats.GLVersion.c_str()); ImGui::NewLine();
    ImGui::Text("Shading Version:   GLSL %s", stats.GLShadingVersion.c_str()); ImGui::NewLine();
    ImGui::PopTextWrapPos();
    ImGui::End();
}



// ------------------------------------------------------------------------------
void Sandbox::SetItemWidth(float width)
{
    if (!glm::epsilonEqual(width, 0.0f, glm::epsilon<float>()))
        ImGui::SetNextItemWidth(width);
}

void Sandbox::SetItemSpacing(float width, float indent, bool set_indent)
{
    if(set_indent)
        ImGui::NewLine(); ImGui::SameLine(indent);

    SetItemWidth(width);
}

void Sandbox::DrawVec3Control(const char* name, const char* label, float indent, glm::vec3& value, glm::vec3 reset_val)
{
    ImVec4 im_active_color = ImVec4(0.8f, 0.1f, 0.15f, 1.0f);
    ImVec4 im_hover_color = ImVec4(0.9f, 0.2f, 0.25f, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_Button, im_active_color);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, im_active_color);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, im_hover_color);

    ImGui::NewLine(); ImGui::SameLine(indent);
    ImGui::Text(name); ImGui::SameLine();
    if (ImGui::Button(label, { 17.5f, 17.5f }))
        value = reset_val;

    ImGui::SameLine();
    float width = ImGui::GetContentRegionAvailWidth() - 5.0f;

    SetItemSpacing(width);
    ImGui::DragFloat3(label, glm::value_ptr(value), 0.05f, 0.0f, 0.0f, "%.1f");
    ImGui::PopStyleColor(3);
}

void Sandbox::DrawEntitiesPanel()
{
    uint i = 0;
    for (auto& entity : m_SceneModels)
    {
        // -- New ImGui ID --
        static char popup_id[64];
        sprintf_s(popup_id, 64, "EntityTag_%i", i);
        ImGui::PushID(popup_id);

        // -- Entity Active --
        ImGui::Checkbox("##EntActive", &entity->GetTransformation().EntityActive);
        
        // -- Entity Name --
        ImGui::SameLine();
        SetItemWidth(ImGui::GetContentRegionAvailWidth() / 1.5f);

        char buffer[256];
        memset(buffer, 0, sizeof(buffer));
        std::strncpy(buffer, entity->GetName().c_str(), sizeof(buffer));

        if (ImGui::InputText("##EntName", buffer, sizeof(buffer), ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_CharsNoBlank))
            entity->SetName(std::string(buffer));


        // -- Entity Transform --
        float indent = ImGui::GetContentRegionAvailWidth() / 5.0f - 10.0f;
        DrawVec3Control("Pos", "##Translation", indent, entity->GetTransformation().Translation);
        DrawVec3Control("Rot", "##Rotation", indent, entity->GetTransformation().Rotation, glm::vec3(0.0f, 180.0f, 0.0f));
        DrawVec3Control("Sca", "##Scale", indent, entity->GetTransformation().Scale, glm::vec3(0.25f));


        // -- Pop & Spacing --
        ImGui::PopID();
        ImGui::NewLine(); ImGui::Separator(); ImGui::NewLine();
        ++i;
    }
}


void Sandbox::DrawResourcesPanel()
{
    //Resources::PrintResourcesReferences();
}


void Sandbox::SetMemoryMetrics()
{
    m_MemoryMetrics = Application::Get().GetMemoryMetrics();

    if (m_AllocationsIndex == ALLOCATIONS_SAMPLES)
        m_AllocationsIndex = 0;

    m_MemoryAllocations[m_AllocationsIndex] = m_MemoryMetrics.GetCurrentMemoryUsage();
    ++m_AllocationsIndex;
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
}