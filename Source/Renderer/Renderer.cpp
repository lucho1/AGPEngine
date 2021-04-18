#include "Renderer.h"
#include "Core/Application/Application.h"

#include "Utils/RendererUtils.h"
#include "Utils/RenderCommand.h"
#include "Utils/RendererPrimitives.h"

#include "Resources/Texture.h"

#include <glad/glad.h>

//#include <glm/glm.hpp>
//#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>


// ------------------------------------------------------------------------------
RendererStatistics Renderer::m_RendererStatistics = {};
UniformBuffer* Renderer::m_CameraUniformBuffer = nullptr;

void Renderer::Init()
{
	// -- Check OGL Version --
	int v_maj, v_min;
	glGetIntegerv(GL_MAJOR_VERSION, &v_maj);
	glGetIntegerv(GL_MINOR_VERSION, &v_min);
	ASSERT((v_maj == 4 && v_min <= 6), "Wrong OpenGL version!");
	SetRendererStatistics(v_maj, v_min);

	// -- Enable OGL Debugging --
	#ifdef _DEBUG
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(RendererUtils::OpenGLMessageCallback, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
	#endif	

	// -- Set OGL Init Stuff --
	RenderCommand::SetBlending(true);
	RenderCommand::SetBlendingFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	RenderCommand::SetDepthTest(true);

	// -- Load Default Textures --
	LoadDefaultTextures();

	// -- Create the Uniform Buffer for the Camera --
	BufferLayout camera_ubo_layout = { { SHADER_DATA::MAT4, "ViewProjection" }, { SHADER_DATA::FLOAT4, "Position" } }; //Vec3 "are like" Vec4 for GPU
	m_CameraUniformBuffer = new UniformBuffer(camera_ubo_layout, 0);
}

void Renderer::Shutdown()
{
	RendererPrimitives::DefaultTextures::CleanUp();
	delete m_CameraUniformBuffer;
}

void Renderer::OnWindowResized(uint width, uint height)
{
	RenderCommand::SetViewport(0, 0, width, height);
}



// ------------------------------------------------------------------------------
void Renderer::ClearRenderer(uint viewport_width, uint viewport_height)
{
	RenderCommand::SetClearColor(glm::vec3(0.15f));
	RenderCommand::Clear();
	RenderCommand::SetViewport(0, 0, viewport_width, viewport_height);
}

void Renderer::BeginScene(const glm::mat4& viewproj_mat, const glm::vec3& view_position)
{
	m_CameraUniformBuffer->Bind();
	m_CameraUniformBuffer->SetData("ViewProjection", glm::value_ptr(viewproj_mat));
	m_CameraUniformBuffer->SetData("Position", glm::value_ptr(view_position));
	m_CameraUniformBuffer->Unbind();
}

void Renderer::EndScene()
{
}



// ------------------------------------------------------------------------------
void Renderer::RenderMesh(const Ref<Shader>& shader, const Mesh* mesh, const glm::mat4& transform)
{
	// -- Recursive Submeshes Draw --
	for (uint i = 0; i < mesh->m_Submeshes.size(); ++i)
		RenderMesh(shader, mesh->m_Submeshes[i].get(), transform);

	// -- Material & Texture Retrieval --
	Texture* albedo = nullptr;
	Ref<Material> mesh_mat = Resources::GetMaterial(mesh->GetMaterialIndex());
	Resources::TexturesIndex texture_binding = Resources::TexturesIndex::MAGENTA;

	if (mesh_mat && mesh_mat->Albedo)
	{
		albedo = mesh_mat->Albedo.get();
		texture_binding = Resources::TexturesIndex::ALBEDO;
	}

	// -- Shader Bindings --
	Renderer::BindTexture(texture_binding, albedo);
	shader->SetUniformInt("u_Texture", (int)texture_binding);
	shader->SetUniformMat4("u_Model", transform);
	//shader->SetUniformVec4("u_Color", mesh_mat->AlbedoColor);

	// -- Draw Call & Unbinds --
	mesh->m_VertexArray->Bind();
	RenderCommand::DrawIndexed(mesh->m_VertexArray);

	mesh->m_VertexArray->Unbind();
	Renderer::UnbindTexture(texture_binding, albedo);
}


void Renderer::SubmitModel(const Ref<Shader>& shader, const Ref<Model>& model)
{
	if (!model->GetTransformation().EntityActive)
		return;

	shader->Bind();
	RenderMesh(shader, model->GetRootMesh(), model->GetTransformation().GetTransform());
	shader->Unbind();
}


void Renderer::Submit(const Ref<Shader>& shader, const Ref<VertexArray>& vertex_array, const glm::mat4& transform)
{
	shader->Bind();
	shader->SetUniformMat4("u_Model", transform);

	vertex_array->Bind();
	RenderCommand::DrawIndexed(vertex_array);
}



// ------------------------------------------------------------------------------
void Renderer::BindTexture(Resources::TexturesIndex texture_type, Texture* texture)
{
	const uint index = (uint)texture_type;
	uint materials_textures_pos = (uint)Resources::TexturesIndex::ALBEDO;

	if (index < materials_textures_pos)
		RendererPrimitives::DefaultTextures::GetTextureFromIndex(index)->Bind(index);
	else if (index >= materials_textures_pos && texture)
		texture->Bind(index);
	else
		ENGINE_LOG("Tried to Bind a nullptr texture!");
}

void Renderer::UnbindTexture(Resources::TexturesIndex texture_type, Texture* texture)
{
	const uint index = (uint)texture_type;
	uint materials_textures_pos = (uint)Resources::TexturesIndex::ALBEDO;

	if (index < materials_textures_pos)
		RendererPrimitives::DefaultTextures::GetTextureFromIndex(index)->Unbind();
	else if (index >= materials_textures_pos && texture)
		texture->Unbind();
	else
		ENGINE_LOG("Tried to Unbind a nullptr texture!");
}



// ------------------------------------------------------------------------------
void Renderer::SetRendererStatistics(int ogl_major_version, int ogl_min_version)
{
	m_RendererStatistics.OGL_MajorVersion = ogl_major_version;
	m_RendererStatistics.OGL_MinorVersion = ogl_min_version;
	m_RendererStatistics.GLVersion = std::string((const char*)glGetString(GL_VERSION));
	m_RendererStatistics.GraphicsCard = std::string((const char*)glGetString(GL_RENDERER));
	m_RendererStatistics.GLVendor = std::string((const char*)glGetString(GL_VENDOR));
	m_RendererStatistics.GLShadingVersion = std::string((const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));
}

void Renderer::LoadDefaultTextures()
{
	// Default textures with default colors, made with hex values
	// First 2 Fs are for alpha, rest for blue, green and red (in that order!)
	// Full Fs (1) for every channel (2x4 channels - rgba -) is white, same logic for the next ones
	uint white_data = 0xffffffff, black_data = 0xff000000, magenta_data = 0xffff00ff, normal_data = 0xffff8080;

	RendererPrimitives::DefaultTextures::WhiteTexture = CreateUnique<Texture>(1, 1);
	RendererPrimitives::DefaultTextures::BlackTexture = CreateUnique<Texture>(1, 1);
	RendererPrimitives::DefaultTextures::MagentaTexture = CreateUnique<Texture>(1, 1);
	RendererPrimitives::DefaultTextures::TempNormalTexture = CreateUnique<Texture>(1, 1);
	RendererPrimitives::DefaultTextures::TempAlbedoTexture = CreateUnique<Texture>(new Texture("Resources/Textures/dice.png"));
	
	RendererPrimitives::DefaultTextures::WhiteTexture->SetData(&white_data, sizeof(white_data)); // or sizeof(uint)
	RendererPrimitives::DefaultTextures::BlackTexture->SetData(&black_data, sizeof(black_data));
	RendererPrimitives::DefaultTextures::MagentaTexture->SetData(&magenta_data, sizeof(magenta_data));
	RendererPrimitives::DefaultTextures::TempNormalTexture->SetData(&normal_data, sizeof(normal_data));
}