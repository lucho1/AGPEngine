#ifndef _RENDERCOMMANDS_H_
#define _RENDERCOMMANDS_H_

#include "Core/Globals.h"
#include "Renderer/Buffers.h"
#include <glad/glad.h>

class RenderCommand
{
public:

	// --- Clearing ---
	inline static void Clear()										{ glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); }
	
	inline static void SetClearColor(const glm::vec4& color)		{ glClearColor(color.r, color.g, color.b, color.a); }
	inline static void SetClearColor(const glm::vec3& color)		{ glClearColor(color.r, color.g, color.b, 1.0f); }

	// --- Blending & Depth ---
	inline static void SetBlendingFunc(int s_val, int f_val)		{ glBlendFunc(s_val, f_val); }

	inline static void SetBlending(bool enable)						{ m_BlendEnabled = enable;  enable ? glEnable(GL_BLEND) : glDisable(GL_BLEND); }
	inline static bool IsBlendingEnabled()							{ return m_BlendEnabled; }

	inline static void SetDepthTest(bool enable)					{ m_DepthTestEnabled = enable;  enable ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST); }
	inline static bool IsDepthTestEnabled()							{ return m_DepthTestEnabled; }

	// --- Viewport ---
	inline static void SetViewport(uint x, uint y, uint w, uint h)	{ glViewport(x, y, w, h); }

	// --- Drawing ---
	inline static void DrawIndexed(const Ref<VertexArray>& vertex_array, uint index_count = 0)
	{
		uint count = index_count ? index_count : vertex_array->GetIndexBuffer()->GetCount();
		glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
		//glBindTexture(GL_TEXTURE_2D, 0);
	};

private:

	static bool m_BlendEnabled, m_DepthTestEnabled;
};


#endif //_RENDERCOMMANDS_H_