#ifndef _EDITORUI_H_
#define _EDITORUI_H_

#include "Core/Globals.h"
#include "Core/Utils/FileStringUtils.h"
#include "Core/Resources/Resources.h"

#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>


namespace EditorUI
{
	void SetDocking();
	void DrawVec3Control(const char* name, const char* label, float indent, glm::vec3& value, glm::vec3 reset_val = glm::vec3(0.0f), float vec_indent = 0.0f, float slider_width = 5.0f);


	// --- Header Defined ---
	static void SetItemWidth(float width)
	{
		if (!glm::epsilonEqual(width, 0.0f, glm::epsilon<float>()))
			ImGui::SetNextItemWidth(width);
	}

	static bool DrawSlider(const char* text, const char* label, float* value, float text_indent, float slider_indent, float min = 0.0f, float max = 0.0f)
	{
		ImGui::NewLine(); ImGui::SameLine(text_indent);
		ImGui::Text(text); ImGui::SameLine(slider_indent);
		float width = ImGui::GetContentRegionAvailWidth() / 1.3f;
		SetItemWidth(width);
		return ImGui::SliderFloat(label, value, min, max, "%.1f", 1.0f);
	}
	
	static void SetItemSpacing(float width, float indent = 0.0f, bool set_indent = false)
	{
		if (set_indent)
			ImGui::NewLine(); ImGui::SameLine(indent);

		SetItemWidth(width);
	}

	static void DrawPlot(float* values, float plot_scale, bool print_overlay = false,  const char* overlay_label = "Example %.1f%s", float label_value = 2.0f, const char* label_unit = "GB")
	{
		char overlay[50] = "";
		if (print_overlay)
			sprintf(overlay, overlay_label, label_value, label_unit);

		std::string label = "###" + std::string(overlay_label);
		ImGui::PlotLines(label.c_str(), values, IM_ARRAYSIZE(values), 0, overlay, 0.0f, plot_scale, ImVec2(ImGui::GetContentRegionAvailWidth(), 100.0f));
	}


	static void DrawTextureButton(Ref<Texture>& texture, const char* texture_name, ImVec2 btn_size, uint meshindex_uitexturebtn, uint texturebtn_number)
	{
		std::string label = "###texture_" + std::string(texture_name) + "_btn";		
		uint id = texture == nullptr ? 0 : texture->GetTextureID();
		bool set_mat_texture = false;

		char btnid[24];
		sprintf_s(btnid, 24, "MatTextureBtn_%i%i", meshindex_uitexturebtn, texturebtn_number);
		ImGui::PushID(btnid);

		ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 20.0f);
		ImGui::Text(texture_name); ImGui::SameLine();
		
		if (id == 0)
			set_mat_texture = ImGui::Button(label.c_str(), btn_size);
		else
			set_mat_texture = ImGui::ImageButton((ImTextureID)id, btn_size, { 0.0f, 1.0f }, { 1.0f, 0.0f }, 0, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));

		if (set_mat_texture)
		{
			std::string texture_file = FileUtils::FileDialogs::OpenFile("Texture (*.png)\0*.png\0");
			if (!texture_file.empty())
				texture = Resources::CreateTexture(texture_file);
		}
		
		ImGui::SameLine();
		if (ImGui::Button("X", btn_size))
			texture = nullptr;

		ImGui::PopID();
	}
	

}

#endif //_EDITORUI_H_