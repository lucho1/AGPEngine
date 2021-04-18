#ifndef _MESH_H_
#define _MESH_H_

#include "Core/Globals.h"
#include "Renderer/Entities/TransformComponent.h"
#include <filesystem>


// ------------------------------------------------------------------------------
class Mesh;

class Model
{
	friend class Resources;
	friend class MeshImporter;
private:

	// --- Des/Constructor ---
	Model(const std::string& filepath, Mesh* root_mesh = nullptr) : m_Path(filepath), m_RootMesh(root_mesh)
	{
		std::filesystem::path path = filepath;
		m_Name = path.stem().string();
	}

	Model(const Model&) = default;

public:

	~Model() { if (m_RootMesh) m_RootMesh = nullptr; }
	
	void SetName(const std::string& name)		{ m_Name = name; }
	const std::string& GetName()		const	{ return m_Name; }
	const Mesh* GetRootMesh()			const	{ return m_RootMesh; }

	TransformComponent& GetTransformation() { return m_Transform; }

private:

	std::string m_Path = "unpathed";
	std::string m_Name = "unnamed";
	Mesh* m_RootMesh = nullptr;

	TransformComponent m_Transform = {};
};



// ------------------------------------------------------------------------------
class Mesh
{
	friend class Resources;
	friend class MeshImporter;
	friend class Renderer;
private:

	// --- Constructor ---
	Mesh(const Ref<VertexArray>& vertex_array, int id, uint material_index = 0, Mesh* parent = nullptr)
		: m_VertexArray(vertex_array), m_ID(id), m_MaterialIndex(material_index), m_ParentMesh(parent) {}

public:
	
	// --- Destructor ---
	~Mesh() { DeleteMesh(); }

	// --- Getters/Setters ---
	inline void SetName(const std::string& name)			{ m_Name = name; }
	inline const std::string& GetName()				const	{ return m_Name; }
	inline uint GetID()								const	{ return m_ID; }
	
	inline uint GetMaterialIndex()					const	{ return m_MaterialIndex; }
	inline const Mesh* GetParent()					const	{ return m_ParentMesh; }
	//inline const Ref<VertexArray>& GetVertexArray()	const	{ return m_VertexArray; }

	//inline const Ref<Mesh>& GetSubmesh(uint index)	const	{ if (index < m_Submeshes.size()) return m_Submeshes[index]; return nullptr; }
	//inline uint GetSubmeshesSize()					const	{ return m_Submeshes.size(); }

	bool operator==(const Mesh& mesh)				const	{ return m_ID == mesh.m_ID; }


	// --- Mesh Methods ---
	void AddSubmesh(Ref<Mesh>* mesh)
	{
		if (!mesh || !mesh->get())
			return;

		m_Submeshes.push_back(*mesh);
		(*mesh)->m_ParentMesh = this;
	}


private:

	// --- Mesh Private Methods ---
	void DeleteMesh()
	{
		for (int i = m_Submeshes.size() - 1; i >= 0; i--)
			m_Submeshes[i].reset();

		m_Submeshes.clear();
		m_VertexArray.reset();
		m_ParentMesh = nullptr;
	}

private:

	// --- Variables ---
	int m_ID = -1;
	std::string m_Name = "unnamed";				// Debug
	uint m_MaterialIndex = 0;					// Index of res. material for this mesh
	std::vector<Ref<Mesh>> m_Submeshes;
	
	Ref<VertexArray> m_VertexArray = nullptr;
	Mesh* m_ParentMesh = nullptr;
};

#endif //_MESH_H_