#include "EditorUI.h"


// ------------------------------------------------------------------------------
void EditorUI::SetDocking()
{
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNav;

    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        window_flags |= ImGuiWindowFlags_NoBackground;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("Dockspace", (bool*)true, window_flags);
    ImGui::PopStyleVar();
    ImGui::PopStyleVar(2);

    // -- Set Dockspace & its minimum size --
    ImGuiIO& io = ImGui::GetIO();
    ImGuiStyle& style = ImGui::GetStyle();
    float original_min_size = style.WindowMinSize.x;
    style.WindowMinSize.x = 370.0f;

    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        ImGuiID dock_id = ImGui::GetID("MyDockspace");
        ImGui::DockSpace(dock_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    }

    style.WindowMinSize.x = original_min_size;
    ImGui::End();
}



// ------------------------------------------------------------------------------
void EditorUI::DrawVec3Control(const char* name, const char* label, float indent, glm::vec3& value, glm::vec3 reset_val, float vec_indent, float slider_width)
{
    ImVec4 im_active_color = ImVec4(0.8f, 0.1f, 0.15f, 1.0f);
    ImVec4 im_hover_color = ImVec4(0.9f, 0.2f, 0.25f, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_Button, im_active_color);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, im_active_color);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, im_hover_color);

    ImGui::NewLine(); ImGui::SameLine(indent);
    ImGui::Text(name);
    
    if (!glm::epsilonEqual(vec_indent, 0.0f, glm::epsilon<float>()))
        ImGui::SameLine(vec_indent);
    else
        ImGui::SameLine();
    
    if (ImGui::Button(label, { 17.5f, 17.5f }))
        value = reset_val;

    ImGui::SameLine();
    float width = ImGui::GetContentRegionAvailWidth() - slider_width;

    SetItemSpacing(width);
    ImGui::DragFloat3(label, glm::value_ptr(value), 0.05f, 0.0f, 0.0f, "%.1f");
    ImGui::PopStyleColor(3);
}
