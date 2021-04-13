#ifndef _SANDBOX_H_
#define _SANDBOX_H_

#include "Core/Globals.h"

#include "Renderer/Resources/Buffers.h"
#include "Renderer/Resources/Texture.h"
#include "Renderer/Resources/Shader.h"


class Sandbox
{
public:

	Sandbox() = default;
	~Sandbox() = default;

	void Init();
	void OnUpdate(float dt);
	void OnUIRender(float dt);

private:

	Ref<VertexBuffer> m_VBuffer;
	Ref<IndexBuffer> m_IBuffer;
	Ref<VertexArray> m_VArray;
	Ref<Texture> m_TestTexture;
	Ref<Shader> m_TextureShader;
};

#endif //_SANDBOX_H_