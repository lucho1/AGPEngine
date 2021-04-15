#include "Resources.h"


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
// --- Meshes ---
void Resources::DeleteAllMeshReferences(Ref<Mesh>* mesh_to_delete)
{
	// -- Delete from Parent --
	Mesh* parent = (*mesh_to_delete)->GetParent();	
	if (parent)
	{
		std::vector<Ref<Mesh>>::iterator it = parent->m_Submeshes.begin();
		for (; it != parent->m_Submeshes.end(); it++)
		{
			if ((*it).get() == mesh_to_delete->get())
			{
				parent->m_Submeshes.erase(it);
				break;
			}
		}
	}

	// -- Delete Reference --
	// This will call mesh destructor, which calls mesh->DeleteMesh(), so all good :)
	mesh_to_delete->reset();
}


// ------------------------------------------------------------------------------
// --- Models ---
std::vector<Ref<Model>> Resources::m_Models = {};
Ref<Model> Resources::CreateModel(const std::string& filepath)
{
	// --- Check if resource already exists ---
	for (uint i = 0; i < m_Models.size(); ++i)
	{
		if (m_Models[i]->m_Path != "unpathed" && m_Models[i]->m_Path == filepath)
			return m_Models[i];
	}

	// --- Create Resource ---
	Ref<Model> model = CreateRef<Model>(new Model(filepath));
	m_Models.push_back(model);
	return model;
}