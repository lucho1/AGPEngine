#ifndef _RESOURCES_H_
#define _RESOURCES_H_

#include "Core/Globals.h"
#include "Renderer/Resources/Buffers.h"
#include "Renderer/Resources/Texture.h"
#include "Renderer/Resources/Mesh.h"
#include "Renderer/Resources/Material.h"

// Take into account that push_back() adds a reference into the smart ptr,
// that's what's CleanUp for, although not needed because smart ptrs clean
// themselves, it is good practice since we can have lots of resources, so we make sure to free them in-app


class Resources
{
public:

	enum class TexturesIndex { WHITE = 0, BLACK, MAGENTA, TESTNORMAL, TESTALBEDO, ALBEDO, SPECULAR, NORMAL, EMISSIVE, BUMP };

public:
	
	// --- Remove Resources References ---
	static void CleanUp();

	// --- Create Resources ---
	static Ref<Texture> CreateTexture(const std::string& filepath);
	static Ref<Model> CreateModel(const std::string& filepath, Mesh* root_mesh = nullptr);
	
	static Ref<Mesh>* CreateMesh(const Ref<VertexArray>& vertex_array, uint material_index = 0, Mesh* parent = nullptr);
	const static Ref<Material>* CreateMaterial(const std::string& name = "unnamed");

	// --- Unload Resources ---
	static void DeleteAllMeshReferences(uint meshID_to_delete);
	static void DeleteAllMaterialReferences(uint materialID_to_delete);

	// --- Other Resources Stuff --
	static void SetMeshMaterial(uint meshID, uint materialID);
	static void PrintResourcesReferences();

	// --- Getters ---
	static const Ref<Material> GetMaterial(uint index)		{ if (m_Materials.find(index) != m_Materials.end()) return m_Materials[index]; return nullptr; }

private:

	// --- Resources ---
	static std::vector<Ref<Texture>> m_Textures;
	static std::vector<Ref<Model>> m_Models;

	static std::unordered_map<int, Ref<Mesh>> m_Meshes;
	static std::unordered_map<int, Ref<Material>> m_Materials;
};

#endif //_RESOURCES_H_