#ifndef _RENDERERPRIMITIVES_H_
#define _RENDERERPRIMITIVES_H_

#include "Core/Globals.h"
#include "Renderer/Resources/Texture.h"
#include "Renderer/Resources/Material.h"

namespace RendererPrimitives
{
	struct DefaultTextures
	{
		static UniquePtr<Texture> WhiteTexture;
		static UniquePtr<Texture> BlackTexture;
		static UniquePtr<Texture> MagentaTexture;
		static UniquePtr<Texture> TempAlbedoTexture;
		static UniquePtr<Texture> TempNormalTexture;

		static Texture* GetTextureFromIndex(uint index)
		{
			switch (index)
			{
				case 0:		return WhiteTexture.get();
				case 1:		return BlackTexture.get();
				case 2:		return MagentaTexture.get();
				case 3:		return TempNormalTexture.get();
				case 4:		return TempAlbedoTexture.get();
				default:	return nullptr;
			}
		}
	};
}

#endif //_RENDERERPRIMITIVES_H_