#include "Framebuffer.h"


// ------------------------------------------------------------------------------
Framebuffer::Framebuffer(uint width, uint height, std::initializer_list<RendererUtils::FBO_TEXTURE_FORMAT> texture_formats)
{
	for (RendererUtils::FBO_TEXTURE_FORMAT texture : texture_formats)
	{
		if (RendererUtils::IsDepthFormatTexture(texture))
			m_DepthAttachment = texture;
		else
			m_ColorAttachments.push_back(texture);
	}

	Resize(width, height);
}

void Framebuffer::Bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_ID);
	glViewport(0, 0, m_Width, m_Height);
}

void Framebuffer::Unbind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}



// ------------------------------------------------------------------------------
void Framebuffer::Resize(uint width, uint height)
{
	if (width == 0 || height == 0 || width > RendererUtils::s_MaxFBOSize || height > RendererUtils::s_MaxFBOSize)
	{
		ENGINE_LOG("Warning: Tried to resize FBO to %ix%i, aborting operation (too big or 0)", width, height);
		return;
	}
	
	if (m_ID != 0)
		DeleteTexturesAndFBO();

	// -- Set FBO values ---
	m_Width = width;
	m_Height = height;

	// -- Create FBO --
	GLenum FBOsampling = m_Samples > 1 ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
	glCreateFramebuffers(1, &m_ID);
	glBindFramebuffer(GL_FRAMEBUFFER, m_ID);

	// -- Process Color Attachments --
	if (m_ColorAttachments.size() > 0)
		ResetColorTextures(FBOsampling);

	// -- FBO Depth Attachment --
	if (m_DepthAttachment != RendererUtils::FBO_TEXTURE_FORMAT::NONE)
		ResetDepthTexture(FBOsampling);

	// -- Draw Buffers --
	if (m_ColorTextures.size() > 1)
	{
		ASSERT(m_ColorTextures.size() <= 4, "(FBO): More than 4 color attachments is not allowed!");

		GLenum color_buffers[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
		glDrawBuffers((GLsizei)m_ColorTextures.size(), color_buffers);
	}
	else if (m_ColorTextures.empty()) // Depth pass
		glDrawBuffer(GL_NONE);

	// -- Unbind FBO --
	bool fbo_status = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
	ASSERT(fbo_status, "Framebuffer Incompleted!");
	Unbind();
}

//void Framebuffer::ClearFBOTexture(uint index, int value)
//{
//	ASSERT(index < m_ColorTextures.size(), "FBO: Index out of bounds");
//	glClearTexImage(m_ColorTextures[index], 0, RendererUtils::GLTextureFormat(m_ColorAttachments[index]), GL_INT, &value);
//}

uint Framebuffer::GetFBOTextureID(uint index) const
{
	ASSERT(index < m_ColorTextures.size(), "FBO - Index is outside bounds");
	return m_ColorTextures[index];
}



// ------------------------------------------------------------------------------
void Framebuffer::SetTexture(bool depth_texture, GLenum internal_format, GLenum format, uint width, uint height, GLenum data_type, uint samples)
{
	if (samples > 1)
	{
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, internal_format, width, height, GL_FALSE);
		return;
	}

	if (!depth_texture)
		glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, data_type, nullptr);
	else
		glTexStorage2D(GL_TEXTURE_2D, 1, internal_format, width, height);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);		// Set MIN/MAG Filter for texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);	// Set Wraps for texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}


void Framebuffer::DeleteTexturesAndFBO()
{
	glDeleteFramebuffers(1, &m_ID);

	//if(m_ColorTextures.size() > 0)
		glDeleteTextures(m_ColorTextures.size(), m_ColorTextures.data());

	//if(m_DepthTexture != 0)
		glDeleteTextures(1, &m_DepthTexture);

	m_ColorTextures.clear();
	m_DepthTexture = 0;
}


void Framebuffer::ResetColorTextures(GLenum FBOsampling)
{
	m_ColorTextures.resize(m_ColorAttachments.size());
	glCreateTextures(FBOsampling, m_ColorTextures.size(), m_ColorTextures.data());

	for (size_t i = 0; i < m_ColorTextures.size(); ++i)
	{
		glBindTexture(FBOsampling, m_ColorTextures[i]);
		switch (m_ColorAttachments[i])
		{
			case RendererUtils::FBO_TEXTURE_FORMAT::RGBA8:
				SetTexture(false, GL_RGBA8, GL_RGBA, m_Width, m_Height, GL_UNSIGNED_BYTE, m_Samples);
				break;
			case RendererUtils::FBO_TEXTURE_FORMAT::RGBA16:
				SetTexture(false, GL_RGBA16F, GL_RGBA, m_Width, m_Height, GL_FLOAT, m_Samples);
				break;
		}

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, FBOsampling, m_ColorTextures[i], 0);
	}
}


void Framebuffer::ResetDepthTexture(GLenum FBOsampling)
{
	glCreateTextures(FBOsampling, 1, &m_DepthTexture);
	glBindTexture(FBOsampling, m_DepthTexture);
	switch (m_DepthAttachment)
	{
		case RendererUtils::FBO_TEXTURE_FORMAT::DEPTH24STENCIL8:
			SetTexture(true, GL_DEPTH24_STENCIL8, GL_NONE, m_Width, m_Height, 0, m_Samples);
			break;
	}

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, FBOsampling, m_DepthTexture, 0);
}
