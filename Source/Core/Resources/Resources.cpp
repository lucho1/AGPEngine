#include "Resources.h"
#include "MeshImporter.h"


// ------------------------------------------------------------------------------
std::vector<std::string> Resources::GetResourcesReferences()
{
	std::vector<std::string> ret;
	ret.push_back("----- RESOURCES REFERENCES -----");
	ret.push_back("- Materials (" + std::to_string(m_Materials.size()) + ")");

	for (auto& mat : m_Materials)
	{
		std::string m_str = "\tMat " + std::to_string(mat.first) + " '" + mat.second->m_Name + "' -> Refs: " + std::to_string(mat.second.use_count());
		ret.push_back(m_str);
	}

	ret.push_back("- Meshes (" + std::to_string(m_Meshes.size()) + ")");
	for (auto& mesh : m_Meshes)
	{
		std::string m_str = "\tMesh " + std::to_string(mesh.first) + " (MatID: " + std::to_string(mesh.second->GetMaterialIndex()) + ") '";
		m_str += mesh.second->GetName() + "' -> Refs: " + std::to_string(mesh.second.use_count());
		ret.push_back(m_str);
	}

	ret.push_back("- Models (" + std::to_string(m_Models.size()) + ")");
	for (auto& model : m_Models)
	{
		std::string m_str = "\tModel '" + model->m_Name + "' -> Refs: " + std::to_string(model.use_count());
		ret.push_back(m_str);
	}

	ret.push_back("- Textures (" + std::to_string(m_Textures.size()) + ")\n\tFirst 5 are Default ones");
	for (auto& tex : m_Textures)
	{
		std::string t_str = "\tTexture " + std::to_string(tex->m_ID) + " -> Refs: " + std::to_string(tex.use_count()) + "\n";
		ret.push_back(t_str);
	}
	
	return ret;
}

void Resources::PrintResourcesReferences()
{
	ENGINE_LOG("\n\n----- RESOURCES REFERENCES -----");
	ENGINE_LOG("\t- Materials (%i)", m_Materials.size());
	for (auto& mat : m_Materials)
		ENGINE_LOG("\t\tMat %i '%s' -> Refs: %i", mat.first, mat.second->m_Name.c_str(), mat.second.use_count());

	ENGINE_LOG("\t- Meshes (%i)", m_Meshes.size());
	for (auto& mesh : m_Meshes)
		ENGINE_LOG("\t\tMesh %i (MatID: %i) '%s' -> Refs: %i", mesh.first, mesh.second->GetMaterialIndex(), mesh.second->GetName().c_str(), mesh.second.use_count());

	ENGINE_LOG("\t- Models (%i)", m_Models.size());
	for(auto& model : m_Models)
		ENGINE_LOG("\t\tModel '%s' -> Refs: %i", model->m_Name.c_str(), model.use_count());

	ENGINE_LOG("\t- Textures (%i)\n\t\tFirst 5 are Default ones", m_Textures.size());
	for(auto& tex : m_Textures)
		ENGINE_LOG("\t\tTexture %i -> Refs: %i", tex->m_ID, tex.use_count());

	ENGINE_LOG("\n");
}



// ------------------------------------------------------------------------------
void Resources::CleanUp()
{
	for (auto& mesh : m_Meshes)
		mesh.second.reset();

	for (uint i = 0; i < m_Textures.size(); ++i)
		m_Textures[i].reset();

	for (uint i = 0; i < m_Models.size(); ++i)
		m_Models[i].reset();

	for (auto& mat : m_Materials)
		mat.second.reset();

	m_Textures.clear();
	m_Models.clear();
	m_Meshes.clear();
	m_Materials.clear();
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

	m_Models.push_back(model);
	return model;
}

Ref<Model> Resources::CreateModel(const Ref<Model>& model, const std::string& new_name)
{
	// --- Create New Resource from an Existing one ---
	Ref<Model> new_model = CreateRef<Model>(new Model(*model.get()));
	if (new_model == nullptr)
		return nullptr;

	model->m_Name = new_name;
	m_Models.push_back(new_model);
	return new_model;
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

	m_Materials.insert({ id, CreateRef<Material>(new Material(id, mat_name)) });
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