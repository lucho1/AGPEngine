#ifndef _RENDERERPRIMITIVES_H_
#define _RENDERERPRIMITIVES_H_

#include "Core/Globals.h"
#include "Renderer/Texture.h"

namespace RendererPrimitives
{
	struct DefaultTextures
	{
		static ScopePtr<Texture> WhiteTexture;
		static ScopePtr<Texture> BlackTexture;
		static ScopePtr<Texture> MagentaTexture;
		static ScopePtr<Texture> TempNormalTexture;
	};
}

#endif //_RENDERERPRIMITIVES_H_