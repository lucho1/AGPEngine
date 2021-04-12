#ifndef _RENDERER_H_
#define _RENDERER_H_

#include "Core/Globals.h"
#include "Buffers.h"
#include "Shader.h"

#include <glm/glm.hpp>

class Renderer
{
public:

	// --- Class Stuff ---
	Renderer();
	~Renderer();
	void Init();

	// --- Rendering Stuff ---
	void BeginScene(glm::mat4 viewproj_mat);
	void EndScene();
	void Submit(const Ref<Shader>& shader, const Ref<VertexArray>& vertex_array, const glm::mat4& transform = glm::mat4(1.0f));

private:

	glm::mat4 m_ViewProjectionMatrix = glm::mat4(1.0f);
};

#endif //_RENDERER_H_