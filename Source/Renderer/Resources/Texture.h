#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#include "Core/Globals.h"
#include <glad/glad.h>

class Texture
{
	friend class Resources;
	friend class Renderer;
private:

	// --- Des/Construction ---
	Texture(const std::string& path);

public:
	
	Texture(uint width, uint height);
	~Texture();

	// --- Class Methods ---
	// Slot 0 should be left for internal stuff, 1 for white. Use from there.
	void Bind(uint slot = 0) const;
	void Unbind() const;
	void SetData(void* data, uint size);

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

#endif //_TEXTURE_H_