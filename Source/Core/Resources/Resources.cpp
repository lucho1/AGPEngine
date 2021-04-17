#include "Resources.h"
#include "MeshImporter.h"


// ------------------------------------------------------------------------------
void Resources::PrintResourcesReferences()
{
	ENGINE_LOG("\n\n----- RESOURCES REFERENCES -----");
	ENGINE_LOG("\t- Materials (%i)", m_Materials.size());
	for (auto& mat : m_Materials)
		ENGINE_LOG("\t\tMat %i '%s' -> Refs: %i", mat.first, mat.second->m_Name.c_str(), mat.second.use_count());

	ENGINE_LOG("\t- Meshes (%i)", m_Meshes.size());
	for (auto& mesh : m_Meshes)
		ENGINE_LOG("\t\tMesh %i -> Refs: %i", mesh.first, mesh.second.use_count());

	ENGINE_LOG("\t- Models (%i)", m_Models.size());
	for(auto& model : m_Models)
		ENGINE_LOG("\t\tModel '%s' -> Refs: %i", model->m_Name.c_str(), model.use_count());

	ENGINE_LOG("\t- Textures (%i)\n\t\tFirst 5 are Default ones", m_Textures.size());
	for(auto& tex : m_Textures)
		ENGINE_LOG("\t\tTexture %i -> Refs: %i", tex->m_ID, tex.use_count());

	ENGINE_LOG("\n");
}



// ------------------------------------------------------------------------------
// --- Textures ---
std::vector<Ref<Texture>> Resources::m_Textures = {};

Ref<Texture> Resources::CreateTexture(const std::string& filepath)
{
	// --- Check if resource already exists ---
	for (uint i = 0; i < m_Textures.size(); ++i)
	{
		if (m_Textures[i]->m_Path != "unpathed" && m_Textures[i]->m_Path == filepath)
			return m_Textures[i];
	}

	// --- Create Resource ---
	Ref<Texture> texture = CreateRef<Texture>(new Texture(filepath));
	m_Textures.push_back(texture);
	return texture;
}



// ------------------------------------------------------------------------------
// --- Models ---
std::vector<Ref<Model>> Resources::m_Models = {};

Ref<Model> Resources::CreateModel(const std::string& filepath, Mesh* root_mesh)
{
	// --- Check if resource already exists ---
	for (uint i = 0; i < m_Models.size(); ++i)
	{
		if (m_Models[i]->m_Path != "unpathed" && m_Models[i]->m_Path == filepath)
			return m_Models[i];
	}

	// --- Create Resource ---
	Ref<Model> model = MeshImporter::LoadModel(filepath);
	if (model == nullptr)
		return nullptr;

	//Ref<Model> model = CreateRef<Model>(new Model(filepath, root_mesh));
	m_Models.push_back(model);
	return model;
}



// ------------------------------------------------------------------------------
// --- Meshes & Materials ---
std::unordered_map<int, Ref<Mesh>> Resources::m_Meshes = {};
std::unordered_map<int, Ref<Material>> Resources::m_Materials = {};

Ref<Mesh>* Resources::CreateMesh(const Ref<VertexArray>& vertex_array, uint material_index, Mesh* parent)
{
	int id = m_Meshes.size();
	Ref<Mesh> mesh = CreateRef<Mesh>(new Mesh(vertex_array, id, material_index, parent));
	m_Meshes.insert({ id,  mesh });
	return &m_Meshes[id];
}


const Ref<Material>* Resources::CreateMaterial(const std::string& name)
{
	int id = m_Materials.size(); // TODO: There should be some default materials (white, ...) which are indexes 0-4!!!
	std::string mat_name = name;
	if (name.find_last_of('.') != name.npos)
		mat_name = name.substr((size_t)0, name.find_last_of('.'));

	Ref<Material> mat = CreateRef<Material>(new Material(id, mat_name));
	m_Materials.insert({ id, mat });
	return &m_Materials[id];
}


void Resources::DeleteAllMeshReferences(uint meshID_to_delete)
{
	if (m_Meshes.find(meshID_to_delete) == m_Meshes.end())
		return;

	// -- Delete from Parent --
	Mesh* parent = m_Meshes[meshID_to_delete]->m_ParentMesh;
	if (parent)
	{
		std::vector<Ref<Mesh>>::iterator it = parent->m_Submeshes.begin();
		for (; it != parent->m_Submeshes.end(); it++)
		{
			if ((*it)->GetID() == meshID_to_delete)
			{
				parent->m_Submeshes.erase(it);
				break;
			}
		}
	}

	// -- Delete from Ref Ptrs List --
	if (m_Meshes.find(meshID_to_delete) != m_Meshes.end())
		m_Meshes.erase(meshID_to_delete);

	// -- Delete All References --
	// This will call mesh destructor, which calls mesh->DeleteMesh(), so all good :)
	m_Meshes[meshID_to_delete].reset();
}


void Resources::DeleteAllMaterialReferences(uint materialID_to_delete)
{
	if (m_Materials.find(materialID_to_delete) == m_Materials.end())
		return;

	// -- Change the Material on all Meshes --
	for (auto& mesh : m_Meshes)
		if(mesh.second->m_MaterialIndex == materialID_to_delete) // TODO: There should be some default materials (white, ...) which are indexes 0-4!!!
			mesh.second->m_MaterialIndex = 0;

	// -- Delete All References & Erase from List --
	// This will call mesh destructor, which calls mesh->DeleteMesh(), so all good :)
	m_Materials[materialID_to_delete].reset();
	m_Materials.erase(materialID_to_delete);
}


void Resources::SetMeshMaterial(uint meshID, uint materialID)
{
	if (m_Materials.find(materialID) != m_Materials.end())
		m_Meshes[meshID]->m_MaterialIndex = materialID;
}