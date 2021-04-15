#ifndef _RENDERER_H_
#define _RENDERER_H_

#include "Core/Globals.h"
#include "Core/Resources/Resources.h"

#include "Resources/Buffers.h"
#include "Resources/Shader.h"

#include <glm/glm.hpp>


struct RendererStatistics
{
	std::string GLVersion, GraphicsCard, GLVendor, GLShadingVersion;

	uint OGL_MinorVersion = 0, OGL_MajorVersion = 0;
	uint DrawCalls = 0, QuadCount = 0;

	uint GetTotalVerticesCount()	const { return QuadCount * 4; }
	uint GetTotalIndicesCount()		const { return QuadCount * 6; }
	uint GetTotalTrianglesCount()	const { return QuadCount * 2; }
};


class Renderer
{
public:

	// --- Class Stuff ---
	static void Init();
	static void Shutdown();


	// --- Rendering Stuff ---
	static void ClearRenderer(uint viewport_width, uint viewport_height);

	static void BeginScene(glm::mat4 viewproj_mat);
	static void EndScene();
	static void Submit(const Ref<Shader>& shader, const Ref<VertexArray>& vertex_array, const glm::mat4& transform = glm::mat4(1.0f));

	// --- Resources Stuff ---
	// If a default texture is to be bound, just pass its TexturesIndex and a nullptr, otherwise pass the desired index (albedo, specular...) and a pointer to the texture
	static void BindTexture(Resources::TexturesIndex texture_type, const Ref<Texture>* texture = nullptr);
	static void UnbindTexture(Resources::TexturesIndex texture_type, const Ref<Texture>* texture = nullptr);

	// --- Events ---
	static void OnWindowResized(uint width, uint height);

public:

	// --- Getters ---
	static const RendererStatistics& GetStatistics() { return m_RendererStatistics; }

private:

	// --- Private Class Methods ---
	static void SetRendererStatistics(int ogl_major_version, int ogl_min_version);
	static void LoadDefaultTextures();

private:

	static glm::mat4 m_ViewProjectionMatrix;
	static RendererStatistics m_RendererStatistics;
};

#endif //_RENDERER_H_