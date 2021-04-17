#ifndef _MATERIAL_H_
#define _MATERIAL_H_

#include "Core/Globals.h"
#include "Renderer/Resources/Texture.h"
#include "Renderer/Resources/Shader.h"


class Material
{
	friend class Resources;
private:

	// --- Constructor ---
	Material(int id, const std::string& name = "unnamed") : m_ID(id), m_Name(name) {}

public:
	
	// --- Destructor ---
	~Material() { DeleteMaterial(); }

	// --- Getters ---
	inline uint GetID()								const	{ return m_ID; }
	inline const std::string& GetName()				const	{ return m_Name; }
	inline void SetName(const std::string& name)			{ m_Name = name; }

	bool operator==(const Material& mat)			const { return m_ID == mat.m_ID; }

	// --- Material Methods ---
	void DeleteMaterial()
	{
		Albedo.reset();
		Emissive.reset();
		Specular.reset();
		Normal.reset();
		Bump.reset();
		m_ID = -1;
	}

private:

	//Ref<Shader> m_Shader; //?
	std::string m_Name = "unnamed";
	int m_ID = -1;

public:

	float Smoothness = 1.0f;// , Metallic = 0.0f; TODO: ? -> Also a Metallic texture?
	bool IsTransparent = false, IsEmissive = false, IsTwoSided = false;

	glm::vec4 AlbedoColor = glm::vec4(1.0f);
	glm::vec4 SpecularColor = glm::vec4(0.0f);
	glm::vec4 EmissiveColor = glm::vec4(0.0f);

	Ref<Texture> Albedo = nullptr;
	Ref<Texture> Emissive = nullptr;
	Ref<Texture> Specular = nullptr;
	Ref<Texture> Normal = nullptr;
	Ref<Texture> Bump = nullptr;
};

#endif //_MATERIAL_H_