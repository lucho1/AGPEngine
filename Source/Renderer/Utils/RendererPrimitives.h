#ifndef _RENDERERPRIMITIVES_H_
#define _RENDERERPRIMITIVES_H_

#include "Core/Globals.h"
#include "Renderer/Resources/Texture.h"

namespace RendererPrimitives
{
	struct DefaultTextures
	{
		static UniquePtr<Texture> WhiteTexture;
		static UniquePtr<Texture> BlackTexture;
		static UniquePtr<Texture> MagentaTexture;
		static UniquePtr<Texture> TempNormalTexture;
	};
}

#endif //_RENDERERPRIMITIVES_H_