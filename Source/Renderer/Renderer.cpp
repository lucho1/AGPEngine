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

Light Renderer::m_DirectionalLight = {};
ShaderStorageBuffer* Renderer::m_LightsSSBuffer = nullptr;
std::vector<PointLight> Renderer::m_Lights = {};
uint Renderer::m_LightsIndex = 0;
// ------------------------------------------------------------------------------



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
	RenderCommand::SetScissorTest(true);

	// -- Load Default Textures --
	LoadDefaultTextures();

	// -- Create the Uniform Buffer for the Camera --
	BufferLayout camera_ubo_layout = { { SHADER_DATA::MAT4, "ViewProjection" }, { SHADER_DATA::FLOAT4, "Position" } }; //Vec3 "are like" Vec4 in this case for GPU alignment
	m_CameraUniformBuffer = new UniformBuffer(camera_ubo_layout, 0);

	// -- Create the Shader Storage Buffer for Lights --
	// First, add the int (it's a int4 due to gpu mem alignment), then the lights
	BufferLayout lights_ssbo_layout = { { SHADER_DATA::INT4, "CurrentLights" } };
	for (uint i = 0; i < RendererUtils::s_MaxLights; ++i)
	{
		std::string uniform_name = "PLightsVec[" + std::to_string(i) + "].";

		// A possible improvement in terms of memory, would be to pass "Pos" and "Color" as 3 floats each. That would save up 2 float to be passed per light (8bxLight)
		// But in addition, would save the 12 bytes that the ssbo integer takes due to alignment. With 10 lights that would mean 8x10+12 = 92b.
		// We are not doing it because it's more comfortable to do it like this, but is a possible memory improvement
		lights_ssbo_layout.AddElements({{ SHADER_DATA::FLOAT4, uniform_name + "Pos" },		{ SHADER_DATA::FLOAT4, uniform_name + "Color" },
										{ SHADER_DATA::FLOAT, uniform_name + "Intensity" },	{ SHADER_DATA::FLOAT, uniform_name + "AttK" },
										{ SHADER_DATA::FLOAT, uniform_name + "AttL" },		{ SHADER_DATA::FLOAT, uniform_name + "AttQ" }
									});
	}

	m_LightsSSBuffer = new ShaderStorageBuffer(lights_ssbo_layout, 0);
}

void Renderer::Shutdown()
{
	RendererPrimitives::DefaultTextures::CleanUp();
	delete m_CameraUniformBuffer;
	delete m_LightsSSBuffer;
	m_Lights.clear();
}

void Renderer::OnWindowResized(uint width, uint height)
{
	RenderCommand::SetViewport(0, 0, width, height);
}



// ------------------------------------------------------------------------------
void Renderer::AddLight()
{
	if (m_Lights.size() < RendererUtils::s_MaxLights)
	{
		int id = m_LightsIndex;
		++m_LightsIndex;
		m_Lights.push_back(PointLight(id));
	}
	else
		ENGINE_LOG("Cannot add more lights! Max Lights (%i) reached!", RendererUtils::s_MaxLights);
}

void Renderer::RemoveLight(uint light_id)
{
	std::vector<PointLight>::iterator it = m_Lights.begin();
	for (; it != m_Lights.end(); ++it)
	{
		if ((*it) == light_id)
		{
			m_Lights.erase(it);
			return;
		}
	}
}



// ------------------------------------------------------------------------------
void Renderer::ClearRenderer()
{
	RenderCommand::SetClearColor(glm::vec3(0.15f));
	RenderCommand::Clear();
}

void Renderer::BeginScene(const glm::mat4& viewproj_mat, const glm::vec3& view_position)
{
	// -- Set Camera UBO --
	m_CameraUniformBuffer->Bind();
	m_CameraUniformBuffer->SetData("ViewProjection", glm::value_ptr(viewproj_mat));
	m_CameraUniformBuffer->SetData("CamPosition", glm::value_ptr(glm::vec4(view_position, 0.0f)));	
	m_CameraUniformBuffer->Unbind();

	// -- Set PLighs SSBO --
	int curr_lights = 0;
	m_LightsSSBuffer->Bind();
	for (uint i = 0; i < m_Lights.size(); ++i) // TODO: move this from here (should be on Begin())
	{
		if (m_Lights[i].Active)
		{
			char uniform_name[16];
			sprintf_s(uniform_name, 16, "PLightsVec[%i].", i);
			m_Lights[i].SetLightData(m_LightsSSBuffer, uniform_name);
			++curr_lights;
		}
	}

	m_LightsSSBuffer->SetData("CurrentLights", glm::value_ptr(glm::ivec4(curr_lights, 0, 0, 0)));
	m_LightsSSBuffer->Unbind();
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
	Ref<Material> mesh_mat = Resources::GetMaterial(mesh->GetMaterialIndex());
	Resources::TexturesIndex alb_binding = Resources::TexturesIndex::MAGENTA, norm_binding = Resources::TexturesIndex::TESTNORMAL,
		bump_binding = Resources::TexturesIndex::BLACK;
	
	Texture* albedo = nullptr;
	Texture* normal = nullptr;
	Texture* bump = nullptr;

	if (mesh_mat)
	{
		if (mesh_mat->Albedo)
		{
			albedo = mesh_mat->Albedo.get();
			alb_binding = Resources::TexturesIndex::ALBEDO;
		}
		if (mesh_mat->Normal)
		{
			normal = mesh_mat->Normal.get();
			norm_binding = Resources::TexturesIndex::NORMAL;
		}
		if (mesh_mat->Bump)
		{
			bump = mesh_mat->Bump.get();
			bump_binding = Resources::TexturesIndex::BUMP;
		}
	}

	// -- Shader Bindings --
	Renderer::BindTexture(alb_binding, albedo);
	Renderer::BindTexture(norm_binding, normal);
	Renderer::BindTexture(bump_binding, bump);

	shader->SetUniformMat4("u_Model", transform);
	shader->SetUniformInt("u_Albedo", (int)alb_binding);
	shader->SetUniformInt("u_Normal", (int)norm_binding);
	shader->SetUniformInt("u_Bump", (int)bump_binding);
	shader->SetUniformVec4("u_Material.AlbedoColor", mesh_mat->AlbedoColor);
	shader->SetUniformFloat("u_Material.Smoothness", mesh_mat->Smoothness);

	// -- Draw Call & Unbinds --
	mesh->m_VertexArray->Bind();
	RenderCommand::DrawIndexed(mesh->m_VertexArray);

	mesh->m_VertexArray->Unbind();
	Renderer::UnbindTexture(bump_binding, bump);
	Renderer::UnbindTexture(norm_binding, normal);
	Renderer::UnbindTexture(alb_binding, albedo);
}


void Renderer::SubmitModel(const Ref<Shader>& shader, const Ref<Model>& model)
{
	if (!model->GetTransformation().EntityActive)
		return;

	shader->Bind();
	shader->SetUniformVec3("u_DirLight.Direction", m_DirectionalLight.Direction);
	shader->SetUniformVec3("u_DirLight.Color", m_DirectionalLight.Color);
	shader->SetUniformFloat("u_DirLight.Intensity", m_DirectionalLight.Intensity);

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