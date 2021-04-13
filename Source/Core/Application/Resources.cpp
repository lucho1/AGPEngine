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
