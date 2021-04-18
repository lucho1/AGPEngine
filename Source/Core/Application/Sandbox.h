#ifndef _SANDBOX_H_
#define _SANDBOX_H_

#include "Core/Globals.h"
#include "Application.h"

#include "Renderer/Entities/CameraController.h"

#include "Renderer/Resources/Buffers.h"
#include "Renderer/Resources/Texture.h"
#include "Renderer/Resources/Shader.h"
#include "Renderer/Resources/Mesh.h"

#define ALLOCATIONS_SAMPLES 90


class Sandbox
{
public:

	Sandbox() = default;
	~Sandbox() = default;

	void Init();
	void OnUpdate(float dt);
	void OnUIRender(float dt);

	void OnMouseScrollEvent(float scroll);
	void OnWindowResizeEvent(uint width, uint height);

private:

	void SetMemoryMetrics();
	void SetItemWidth(float width);
	void SetItemSpacing(float width, float indent = 0.0f, bool set_indent = false);
	void DrawVec3Control(const char* name, const char* label, float indent, glm::vec3& value, glm::vec3 reset_val = glm::vec3(0.0f));
	bool DrawSlider(const char* text, const char* label, float* value, float text_indent, float slider_indent, float min = 0.0f, float max = 0.0f);

	void DrawResourcesPanel();
	void DrawCameraPanel();
	void DrawEntitiesPanel();
	void DrawPerformancePanel();

private:

	std::vector<Ref<Model>> m_SceneModels;
	Ref<Shader> m_TextureShader;
	
	uint m_MemoryAllocations[ALLOCATIONS_SAMPLES] = { 0 };
	uint m_AllocationsIndex = 0;
	MemoryMetrics m_MemoryMetrics = {};

	CameraController m_EngineCamera = {};
};

#endif //_SANDBOX_H_