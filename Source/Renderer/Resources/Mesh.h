#ifndef _MESH_H_
#define _MESH_H_

#include "Core/Globals.h"
#include <filesystem>


// ------------------------------------------------------------------------------
class Mesh;

class Model
{
	friend class Resources;
private:

	// --- Des/Constructor ---
	Model(const std::string& filepath, Mesh* root_mesh = nullptr) : m_Path(filepath), m_RootMesh(root_mesh)
	{
		std::filesystem::path path = filepath;
		m_Name = path.stem().string();
	}

public:

	~Model() = default;
	
	void SetName(const std::string& name)		{ m_Name = name; }
	const std::string& GetName()		const	{ return m_Name; }

	void SetRootMesh(Mesh* mesh)				{ m_RootMesh = mesh; }
	const Mesh* GetRootMesh()			const	{ return m_RootMesh; }

private:

	std::string m_Path = "unpathed";
	std::string m_Name = "unnamed";
	Mesh* m_RootMesh = nullptr;
};



// ------------------------------------------------------------------------------
class Mesh
{
	friend class Resources;
private:

	// --- Constructor ---
	Mesh(const Ref<VertexArray>& vertex_array, int id, uint material_index = 0, Mesh* parent = nullptr)
		: m_VertexArray(vertex_array), m_ID(id), m_MaterialIndex(material_index), m_ParentMesh(parent) {}

public:
	
	// --- Destructor ---
	~Mesh() { DeleteMesh(); }

	// --- Getters ---
	inline uint GetID()								const	{ return m_ID; }
	inline const Ref<VertexArray>& GetVertexArray()	const	{ return m_VertexArray; }
	inline const Mesh* GetParent()					const	{ return m_ParentMesh; }
	inline const Ref<Mesh>* GetSubmesh(uint index)			{ if (index < m_Submeshes.size()) return &m_Submeshes[index]; return nullptr; }

	uint GetMaterialIndex()							const	{ return m_MaterialIndex; }

	bool operator==(const Mesh& mesh)			const { return m_ID == mesh.m_ID; }


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
	uint m_MaterialIndex = 0;	// Index of res. material for this mesh
	std::vector<Ref<Mesh>> m_Submeshes;
	
	Ref<VertexArray> m_VertexArray = nullptr;
	Mesh* m_ParentMesh = nullptr;
};

#endif //_MESH_H_