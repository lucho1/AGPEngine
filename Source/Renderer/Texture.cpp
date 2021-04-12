#include "Texture.h"

#include <stb_image.h>
#include <stb_image_write.h>


// ------------------------------------------------------------------------------
Texture::Texture(uint width, uint height) : m_Width(width), m_Height(height)
{
	// -- Set Parameters --
	m_InternalFormat = GL_RGBA8;
	m_DataFormat = GL_RGBA;

	// -- Create Texture ---
	glGenTextures(1, &m_ID);
	glBindTexture(GL_TEXTURE_2D, m_ID);
	glTexStorage2D(GL_TEXTURE_2D, 1, m_InternalFormat, m_Width, m_Height);
	
	// -- Set Texture Parameters --
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // this gives error: GL_LINEAR_MIPMAP_LINEAR
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// -- Mipmap & Unbind --
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
}

Texture::Texture(const std::string& path)
{
	int w, h, channels;
	stbi_set_flip_vertically_on_load(1);
	stbi_uc* texture_data = stbi_load(path.c_str(), &w, &h, &channels, 0);

	// -- Check for Failure --
	ASSERT(texture_data, "Failed to load texture data from path: %s", path.c_str());
	if (!texture_data)
	{
		ENGINE_LOG("Failed to load texture data from path: %s\nAborting...", path.c_str());
		return;
	}

	// -- Set Parameters --
	m_Width = w; m_Height = h;
	if (channels == 4)
	{
		m_InternalFormat = GL_RGBA8;
		m_DataFormat = GL_RGBA;
	}
	else if (channels == 3)
	{
		m_InternalFormat = GL_RGB8;
		m_DataFormat = GL_RGB;
	}

	ASSERT(m_InternalFormat & m_DataFormat, "Image Format not Supported!"); // False (0) if either is 0
	
	// -- Set Texture Parameters --
	glGenTextures(1, &m_ID);
	glBindTexture(GL_TEXTURE_2D, m_ID);
	glTexStorage2D(GL_TEXTURE_2D, 1, m_InternalFormat, m_Width, m_Height);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // this gives error: GL_LINEAR_MIPMAP_LINEAR
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// -- Set Subimage, Mipmap & Unbind --
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, texture_data);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	// -- Free STBI Image --
	stbi_image_free(texture_data);
}

Texture::~Texture()
{
	glDeleteTextures(1, &m_ID);
}



// ------------------------------------------------------------------------------
void Texture::SetData(void* data, uint size)
{
	uint bpp = m_DataFormat == GL_RGBA ? 4 : 3; // Bytes per pixel
	ASSERT(size == m_Width * m_Height * bpp, "Data passed must be the same size than the entire texture size");

	Bind();
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);
}


void Texture::Bind(uint slot) const
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, m_ID);
}

void Texture::Unbind() const
{
	glBindTexture(GL_TEXTURE_2D, 0);
}