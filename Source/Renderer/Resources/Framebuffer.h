#ifndef _FRAMEBUFFER_H_
#define _FRAMEBUFFER_H_

#include "Core/Globals.h"
#include "Renderer/Utils/RendererUtils.h"
#include <glad/glad.h>


// ----- Framebuffer Class -----
class Framebuffer
{
public:

	// --- Des/Construction ---
	Framebuffer(uint width, uint height, std::initializer_list<RendererUtils::FBO_TEXTURE_FORMAT> texture_formats);
	~Framebuffer() { DeleteTexturesAndFBO(); }

	// --- Public FBO Methods ---
	void Bind();
	void Unbind();

	void Resize(uint width, uint height);
	//void ClearFBOTexture(uint index, int value);

	// --- Getters ---
	uint GetFBOTextureID(uint index = 0) const;
	uint GetWidth() const { return m_Width; }
	uint GetHeight() const { return m_Height; }

private:

	// --- Private FBO Methods ---
	void SetTexture(bool depth_texture, GLenum internal_format, GLenum format, uint width, uint height, GLenum data_type, uint samples);
	void DeleteTexturesAndFBO();
	void ResetColorTextures(GLenum FBOsampling);
	void ResetDepthTexture(GLenum FBOsampling);

private:

	uint m_ID = 0;
	uint m_Width = 0, m_Height = 0, m_Samples = 1;

	std::vector<RendererUtils::FBO_TEXTURE_FORMAT> m_ColorAttachments;
	RendererUtils::FBO_TEXTURE_FORMAT m_DepthAttachment;

	std::vector<uint> m_ColorTextures;
	uint m_DepthTexture = 0;
};

#endif //_FRAMEBUFFER_H_