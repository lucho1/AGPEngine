#ifndef _RESOURCES_H_
#define _RESOURCES_H_

#include "Core/Globals.h"
#include "Renderer/Resources/Texture.h"


// Take into account that push_back() adds a reference into the smart ptr,
// that's what's CleanUp for, although not needed because smart ptrs clean
// themselves, it is good practice since we can have lots of resources, so we make sure to free them in-app


class Resources
{
public:

	enum class TexturesIndex { WHITE = 0, BLACK, MAGENTA, TESTNORMAL, ALBEDO, SPECULAR, NORMAL, EMISSIVE, BUMP };

public:
	
	// --- Remove Resources References ---
	static void CleanUp() { m_Textures.clear(); } //Meshes.clear(); //Materials.clear(); }

	// --- Create Resources ---
	static Ref<Texture> CreateTexture(const std::string& filepath);

private:

	// --- Resources ---
	static std::vector<Ref<Texture>> m_Textures;
	//std::vector<Mesh> Meshes;
	//std::vector<Material> Materials;
};

#endif //_RESOURCES_H_