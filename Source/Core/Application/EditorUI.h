#ifndef _EDITORUI_H_
#define _EDITORUI_H_

#include "Core/Globals.h"

#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>


namespace EditorUI
{
	void SetDocking();
	void DrawVec3Control(const char* name, const char* label, float indent, glm::vec3& value, glm::vec3 reset_val = glm::vec3(0.0f));


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
}

#endif //_EDITORUI_H_