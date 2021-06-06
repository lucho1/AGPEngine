#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#include "Core/Globals.h"
#include <glad/glad.h>


class CubemapTexture;

class Texture
{
	friend class Resources;
	friend class Renderer;
	friend class CubemapTexture;
private:

	// --- Des/Construction ---
	Texture(const std::string& path);
	Texture() = default; // Only for cubemaps use!

	// --- Class Private Methods ---
	// Slot 0 should be left for internal stuff, 1 for white. Use from there.
	void Bind(uint slot = 0) const;
	void Unbind() const;
	void SetData(void* data, uint size);

public:

	// --- Des/Construction ---
	Texture(uint width, uint height);
	~Texture();

	// --- Getters ---
	uint GetWidth()		const { return m_Width; }
	uint GetHeight()	const { return m_Height; }
	uint GetTextureID()	const { return m_ID; }

	// --- Operators ---
	bool operator==(const Texture& texture) const { return m_ID == texture.m_ID; }

private:

	// --- Variables ---
	std::string m_Path = "unpathed"; // Debug
	uint m_Width = 0, m_Height = 0;
	uint m_ID = 0;

	GLenum m_InternalFormat = 0, m_DataFormat = 0;
};


// ----- Cubemap -----
class CubemapTexture
{
public:

	enum class CUBEMAP_TEXTURE { RIGHT, LEFT, BOTTOM, TOP, FRONT, BACK };

	CubemapTexture();
	void SetTexture(CUBEMAP_TEXTURE cubemap_texture_type, const std::string& filepath);
	void LoadTextures();

public:

	uint GetTextureID() const { return m_ID; }

private:

	uint m_ID = 0;
	uint m_Width = 0, m_Height = 0;

	std::vector<std::string> m_TexturePaths;
};

#endif //_TEXTURE_H_