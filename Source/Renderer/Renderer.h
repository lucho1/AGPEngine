#ifndef _RENDERER_H_
#define _RENDERER_H_

#include "Core/Globals.h"
#include "Core/Resources/Resources.h"

#include "Resources/Buffers.h"
#include "Resources/Shader.h"
#include "Entities/Lights.h"

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


	// --- Lighting Stuff ---
	static void AddLight();
	static void RemoveLight(uint light_id);
	static std::vector<PointLight>& GetLights() { return m_Lights; }


	// --- Rendering Stuff ---
	static void ClearRenderer();

	static void BeginScene(const glm::mat4& viewproj_mat, const glm::vec3& view_position);
	static void EndScene();
	static void Submit(const Ref<Shader>& shader, const Ref<VertexArray>& vertex_array, const glm::mat4& transform = glm::mat4(1.0f));
	static void SubmitModel(const Ref<Shader>& shader, const Ref<Model>& model);


	// --- Resources Stuff ---
	// If a default texture is to be bound, just pass its TexturesIndex and a nullptr, otherwise pass the desired index (albedo, specular...) and a pointer to the texture
	static void BindTexture(Resources::TexturesIndex texture_type, Texture* texture = nullptr);
	static void UnbindTexture(Resources::TexturesIndex texture_type, Texture* texture = nullptr);

	// --- Events ---
	static void OnWindowResized(uint width, uint height);

public:

	// --- Getters ---
	static const RendererStatistics& GetStatistics() { return m_RendererStatistics; }

private:

	// --- Private Rendering Stuff ---
	static void RenderMesh(const Ref<Shader>& shader, const Mesh* mesh, const glm::mat4& transform = glm::mat4(1.0f));

	// --- Private Class Methods ---
	static void SetRendererStatistics(int ogl_major_version, int ogl_min_version);
	static void LoadDefaultTextures();

private:

	static RendererStatistics m_RendererStatistics;
	static UniformBuffer* m_CameraUniformBuffer;
	static std::vector<PointLight> m_Lights;
	static uint m_LightsIndex;
};

#endif //_RENDERER_H_