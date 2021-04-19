#ifndef _SANDBOX_H_
#define _SANDBOX_H_

#include "Core/Globals.h"
#include "Application.h"

#include "Renderer/Entities/CameraController.h"

#include "Renderer/Resources/Framebuffer.h"
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

	void DrawLightsPanel();
	void DrawCameraPanel();
	void DrawEntitiesPanel();
	void DrawPerformancePanel();

private:

	// Scene
	CameraController m_EngineCamera = {};

	std::vector<Ref<Model>> m_SceneModels;
	Ref<Shader> m_TextureShader, m_LightingShader;

	// Viewport
	Ref<Framebuffer> m_EditorFramebuffer;
	glm::vec2 m_ViewportSize = glm::vec2(0.0f);
	bool m_ViewportFocused = false, m_ViewportHovered = false;
	
	// Performance Panel
	uint m_MemoryAllocations[ALLOCATIONS_SAMPLES] = { 0 };
	uint m_AllocationsIndex = 0;
	MemoryMetrics m_MemoryMetrics = {};
};

#endif //_SANDBOX_H_