#ifndef _SANDBOX_H_
#define _SANDBOX_H_

#include "Core/Utils/Timer.h"

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

	void RenderSkybox();

	void SetMemoryMetrics();

	void DrawLightsPanel();
	void DrawCameraPanel();
	void DrawMeshMaterials(const Mesh* mesh, std::vector<uint>& materials_shown, uint meshindex_uitexturebtn);
	void DrawEntitiesPanel();
	void DrawPerformancePanel();

private:

	// Scene
	CameraController m_EngineCamera = {};
	std::vector<Ref<Model>> m_SceneModels;
	Ref<Shader> m_TextureShader, m_LightingShader;

	// Deferred Rendering
	Ref<VertexArray> m_QuadArray;
	Ref<Shader> m_DeferredLightingShader;
	Ref<Framebuffer> m_DeferredFramebuffer;

	// Bloom
	Ref<Framebuffer> m_BlurPingPongFramebuffer[2], m_BlurFinalFramebuffer;
	Ref<Shader> m_BlurShader, m_FinalBloomShader;
	float m_BloomExposure = 1.0f, m_BloomHDRGamma = 2.2f;
	int m_BloomBlurAmount = 5, m_BloomBlurIterations = 10;
	bool m_GammaCorrection = false, m_ToneMapping = false;

	// Skybox
	Ref<VertexArray> m_SkyboxVArray;
	Ref<CubemapTexture> m_SkyboxTexture;
	Ref<Shader> m_SkyboxShader;
	glm::vec3 m_SkyboxTint = glm::vec4(1.0f);

	// Timers
	Timer m_FwRendTimer, m_DefRendTimer, m_MeasureTime;

	// Viewport
	Ref<Framebuffer> m_EditorFramebuffer;
	glm::vec2 m_ViewportSize = glm::vec2(0.0f);
	bool m_ViewportFocused = false, m_ViewportHovered = false;
	
	// Performance Panel
	uint m_MemoryAllocations[ALLOCATIONS_SAMPLES] = { 0 };
	uint m_AllocationsIndex = 0;
	MemoryMetrics m_MemoryMetrics = {};

	// Rendering Options
	bool m_DrawLightsSpheres = true;
	bool m_DeferredRendering = true;
	bool m_BloomActive = false;
	bool m_RenderSkybox = true;
};

#endif //_SANDBOX_H_