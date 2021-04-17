#ifndef _SANDBOX_H_
#define _SANDBOX_H_

#include "Core/Globals.h"
#include "Application.h"

#include "Renderer/Resources/Buffers.h"
#include "Renderer/Resources/Texture.h"
#include "Renderer/Resources/Shader.h"
#include "Renderer/Resources/Mesh.h"

#define ALLOCATIONS_SAMPLES 90


class Sandbox
{
public:

	Sandbox() = default;
	~Sandbox() = default;

	void Init();
	void OnUpdate(float dt);
	void OnUIRender(float dt);

private:

	void SetMemoryMetrics();

private:

	Ref<Model> m_PatrickModel;
	Ref<Shader> m_TextureShader;
	
	uint m_MemoryAllocations[ALLOCATIONS_SAMPLES] = { 0 };
	uint m_AllocationsIndex = 0;
	MemoryMetrics m_MemoryMetrics = {};
};

#endif //_SANDBOX_H_