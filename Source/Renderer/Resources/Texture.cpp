#include "Texture.h"
#include "Core/Resources/Resources.h"

#include <stb_image.h>
#include <stb_image_write.h>


// ------------------------------------------------------------------------------
Texture::Texture(uint width, uint height) : m_Width(width), m_Height(height)
{
	// -- Set Parameters --
	m_InternalFormat = GL_RGBA8;
	m_DataFormat = GL_RGBA;

	// -- Create Texture ---
	glCreateTextures(GL_TEXTURE_2D, 1, &m_ID);
	//glBindTexture(GL_TEXTURE_2D, m_ID);
	glTextureStorage2D(m_ID, 1, GL_RGBA8, m_Width, m_Height);
	
	// -- Set Texture Parameters --
	glTextureParameteri(m_ID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(m_ID, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // this gives error: GL_LINEAR_MIPMAP_LINEAR
	glTextureParameteri(m_ID, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_ID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_ID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

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
	//ASSERT(texture_data, "Failed to load texture data from path: %s", path.c_str());
	if (!texture_data)
	{
		ENGINE_LOG("Failed to load texture data from path: %s\nAborting...", path.c_str());
		stbi_image_free(texture_data);
		return;
	}

	// -- Set Parameters --
	m_Width = w; m_Height = h;
	m_Path = path;

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
	glCreateTextures(GL_TEXTURE_2D, 1, &m_ID);
	//glBindTexture(GL_TEXTURE_2D, m_ID);
	//glTexStorage2D(GL_TEXTURE_2D, 1, m_InternalFormat, m_Width, m_Height);
	glTextureStorage2D(m_ID, 1, m_InternalFormat, m_Width, m_Height);

	glTextureParameteri(m_ID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(m_ID, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // this gives error: GL_LINEAR_MIPMAP_LINEAR
	glTextureParameteri(m_ID, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_ID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_ID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// -- Set Subimage, Mipmap & Unbind --
	glTextureSubImage2D(m_ID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, texture_data);
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
	//glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);
	glTextureSubImage2D(m_ID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);
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



// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
CubemapTexture::CubemapTexture()
{
	m_TexturePaths =
	{
		std::string("Resources/Textures/Skybox/right.jpg"),
		std::string("Resources/Textures/Skybox/left.jpg"),
		std::string("Resources/Textures/Skybox/bottom.jpg"),
		std::string("Resources/Textures/Skybox/top.jpg"),
		std::string("Resources/Textures/Skybox/front.jpg"),
		std::string("Resources/Textures/Skybox/back.jpg")
	};

	LoadTextures();
}

void CubemapTexture::SetTexture(CUBEMAP_TEXTURE cubemap_texture_type, const std::string& filepath)
{
	if (std::filesystem::exists(filepath))
	{
		int w, h, channels;
		stbi_uc* data = stbi_load(filepath.c_str(), &w, &h, &channels, 0);

		if (!data)
		{
			ENGINE_LOG("Failed to load texture data from path: %s\nAborting...", filepath.c_str());
			stbi_image_free(data);
			return;
		}

		m_TexturePaths[(int)cubemap_texture_type] = filepath;
		LoadTextures();
	}
}


void CubemapTexture::LoadTextures()
{
	// -- Set Variables --
	int w, h, channels;
	std::vector<stbi_uc*> texture_data;
	stbi_set_flip_vertically_on_load(1);

	// -- Create Cubemap Texture --
	glGenTextures(1, &m_ID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_ID);

	// -- Load Cubemap Textures --
	for (uint i = 0; i < 6; ++i)
	{
		texture_data.push_back(stbi_load(m_TexturePaths[i].c_str(), &w, &h, &channels, 0));
		if (!texture_data[i])
		{
			ENGINE_LOG("Failed to load texture data from path: %s\nAborting...", m_TexturePaths[i].c_str());

			for (uint j = 0; j <= i; ++j)
				stbi_image_free(texture_data[j]);

			return;
		}
	}

	// -- Set Cubemap Textures --
	m_Width = w; m_Height = h;
	for (uint i = 0; i < 6; ++i)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, texture_data[i]);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	// -- Unbind & Free --
	glBindTexture(GL_TEXTURE_2D, 0);
	for (uint i = 0; i < texture_data.size(); ++i)
		stbi_image_free(texture_data[i]);
}
