#include "Renderer.h"
#include "Utils/RendererUtils.h"
#include "Utils/RenderCommand.h"
#include "Texture.h"
#include "Utils/RendererPrimitives.h"

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
	// -- Check OGL Version --
	int v_maj, v_min;
	glGetIntegerv(GL_MAJOR_VERSION, &v_maj);
	glGetIntegerv(GL_MINOR_VERSION, &v_min);
	ASSERT((v_maj == 4 && v_min <= 6), "Wrong OpenGL version!");

	m_RendererStatistics.OGL_MinorVersion = v_min;
	m_RendererStatistics.OGL_MajorVersion = v_maj;
	m_RendererStatistics.GLVersion = std::string((const char*)glGetString(GL_VERSION));
	m_RendererStatistics.GraphicsCard = std::string((const char*)glGetString(GL_RENDERER));
	m_RendererStatistics.GLVendor = std::string((const char*)glGetString(GL_VENDOR));
	m_RendererStatistics.GLShadingVersion = std::string((const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));

	// -- Enable OGL Debugging --
	#ifdef _DEBUG
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(RendererUtils::OpenGLMessageCallback, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
	#endif	

	// -- Set Default Textures --
	// First 2 Fs are for alpha, rest for bgr (in that order!)
	uint white_data = 0xffffffff; // Full Fs (1) for every channel (2x4 channels - rgba -), same logic for the next ones
	uint black_data = 0xff000000;
	uint magenta_data = 0xffff00ff;
	uint normal_data = 0xffff8080;

	RendererPrimitives::DefaultTextures::WhiteTexture = CreateScopePtr<Texture>(1, 1);
	RendererPrimitives::DefaultTextures::BlackTexture = CreateScopePtr<Texture>(1, 1);
	RendererPrimitives::DefaultTextures::MagentaTexture = CreateScopePtr<Texture>(1, 1);
	RendererPrimitives::DefaultTextures::TempNormalTexture = CreateScopePtr<Texture>(1, 1);

	RendererPrimitives::DefaultTextures::WhiteTexture->SetData(&white_data, sizeof(white_data)); // or sizeof(uint)
	RendererPrimitives::DefaultTextures::BlackTexture->SetData(&black_data, sizeof(black_data));
	RendererPrimitives::DefaultTextures::MagentaTexture->SetData(&magenta_data, sizeof(magenta_data));
	RendererPrimitives::DefaultTextures::TempNormalTexture->SetData(&normal_data, sizeof(normal_data));
	

	// -- Set OGL Init Stuff --
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