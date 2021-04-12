#include "Renderer.h"
#include "Utils/RendererUtils.h"
#include "Utils/RenderCommand.h"

#include <glad/glad.h>


// ------------------------------------------------------------------------------
Renderer::Renderer()
{
}

Renderer::~Renderer()
{
}

void Renderer::Init()
{
	int v_maj, v_min;
	glGetIntegerv(GL_MAJOR_VERSION, &v_maj);
	glGetIntegerv(GL_MINOR_VERSION, &v_min);
	ASSERT((v_maj == 4 && v_min <= 6), "Wrong OpenGL version!");

#ifdef _DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(RendererUtils::OpenGLMessageCallback, nullptr);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
#endif

	RenderCommand::SetBlending(true);
	RenderCommand::SetBlendingFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	RenderCommand::SetDepthTest(true);
}



// ------------------------------------------------------------------------------
void Renderer::BeginScene(glm::mat4 viewproj_mat)
{
	m_ViewProjectionMatrix = viewproj_mat;
}

void Renderer::EndScene()
{
}

void Renderer::Submit(const Ref<Shader>& shader, const Ref<VertexArray>& vertex_array, const glm::mat4& transform)
{
	shader->Bind();
	shader->SetUniformMat4("u_ViewProjection", m_ViewProjectionMatrix);
	shader->SetUniformMat4("u_Model", transform);

	vertex_array->Bind();
	RenderCommand::DrawIndexed(vertex_array);
}