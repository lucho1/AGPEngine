#ifndef _RENDERERUTILS_H_
#define _RENDERERUTILS_H_

#include "Core/Globals.h"
#include <glad/glad.h>

namespace RendererUtils
{
	// ------------------------------------------------------------------------------
	// ----- Error Callback -----
	static void APIENTRY OpenGLMessageCallback(GLenum msg_source, GLenum msg_type, GLuint msg_id, GLenum msg_severity, GLsizei msg_length, const GLchar* msg, const void* user_param)
	{
		// Take into account that "ShaderRecompilation" message (#131218) warns you that there is a shader already bound on a shader-bind call
		std::string error_message = " --- OpenGL Error (#" + std::to_string(msg_id) + "): ";
		switch (msg_type)
		{
			case GL_DEBUG_TYPE_ERROR:				error_message += "GL Error";			break;
			case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:	error_message += "Deprecated Behavior";	break;
			case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:	error_message += "Undefined Behavior";	break;
			case GL_DEBUG_TYPE_PORTABILITY:			error_message += "Portability Error";	break;
			case GL_DEBUG_TYPE_PERFORMANCE:			error_message += "Performance Error";	break;
			case GL_DEBUG_TYPE_OTHER:				error_message += "Other Error";			break;
			default:								error_message += "Unknown Error";		break;
		}

		switch (msg_severity)
		{
			case GL_DEBUG_SEVERITY_HIGH:			ENGINE_LOG((error_message + " - High Sev. -" + msg).c_str());		return;
			case GL_DEBUG_SEVERITY_MEDIUM:			ENGINE_LOG((error_message + " - Mid Sev. -" + msg).c_str());		return;
			case GL_DEBUG_SEVERITY_LOW:				ENGINE_LOG((error_message + " - Low Sev. -" + msg).c_str());		return;
			case GL_DEBUG_SEVERITY_NOTIFICATION:	ENGINE_LOG((error_message + " - Notification -" + msg).c_str());	return;
		}

		ENGINE_LOG((error_message + " - UNKNOWN SEVERITY -" + msg).c_str());
		ASSERT(false, "Error of Unknown Severity Level!");
	}



	// ------------------------------------------------------------------------------
	// ----- Shader Data Type Stuff -----
	enum class SHADER_DATA { NONE = 0, FLOAT, FLOAT2, FLOAT3, FLOAT4, MAT3, MAT4, INT, INT2, INT3, INT4, BOOL };

	static uint ShaderDataTypeSize(SHADER_DATA type)
	{
		switch (type)
		{
			case SHADER_DATA::FLOAT:	return 4;			// sizeof(float)
			case SHADER_DATA::FLOAT2:	return 4 * 2;		// sizeof(float) * 2 ... and so on
			case SHADER_DATA::FLOAT3:	return 4 * 3;
			case SHADER_DATA::FLOAT4:	return 4 * 4;
			case SHADER_DATA::MAT3:		return 4 * 3 * 3;	// same here sizeof(float) * 3 * 3 --> Matrix of 3x3
			case SHADER_DATA::MAT4:		return 4 * 4 * 4;
			case SHADER_DATA::INT:		return 4;
			case SHADER_DATA::INT2:		return 4 * 2;
			case SHADER_DATA::INT3:		return 4 * 3;
			case SHADER_DATA::INT4:		return 4 * 4;
			case SHADER_DATA::BOOL:		return 1;			// sizeof(bool)
		}

		ASSERT(false, "Unknown Shader Data Type passed");
		return 0;
	}

	static uint ShaderDataTypeCount(SHADER_DATA type)
	{
		switch (type)
		{
			case SHADER_DATA::FLOAT:	return 1;
			case SHADER_DATA::FLOAT2:	return 2;
			case SHADER_DATA::FLOAT3:	return 3;
			case SHADER_DATA::FLOAT4:	return 4;
			case SHADER_DATA::MAT3:		return 3 * 3;
			case SHADER_DATA::MAT4:		return 4 * 4;
			case SHADER_DATA::INT:		return 1;
			case SHADER_DATA::INT2:		return 2;
			case SHADER_DATA::INT3:		return 3;
			case SHADER_DATA::INT4:		return 4;
			case SHADER_DATA::BOOL:		return 1;
		}

		ASSERT(false, "Element has unknown Shader data type!");
		return 0;
	}



	// ------------------------------------------------------------------------------
	// ----- Shader Type Stuff -----
	static const uint s_MaxLights = 10;

	static GLenum ShaderTypeFromString(const std::string& shader_type_str)
	{
		if (shader_type_str == "VERTEX_SHADER")
			return GL_VERTEX_SHADER;
		if (shader_type_str == "FRAGMENT_SHADER" || shader_type_str == "PIXEL_SHADER")
			return GL_FRAGMENT_SHADER;

		ASSERT(false, "Unknown Shader Type '%s'", shader_type_str.c_str());
		return 0;
	}

	static const char* StringFromShaderType(const GLenum& shader_type)
	{
		if (shader_type == GL_VERTEX_SHADER)
			return "Vertex";
		if (shader_type == GL_FRAGMENT_SHADER)
			return "Fragment/Pixel";

		ASSERT(false, "Unknown Shader Type '%i'", (int)shader_type);
		return 0;
	}



	// ------------------------------------------------------------------------------
	// ----- Conversions -----
	static GLenum ShaderDataTypeToOpenGLType(SHADER_DATA type)
	{
		switch (type)
		{
			case SHADER_DATA::FLOAT:		return GL_FLOAT;
			case SHADER_DATA::FLOAT2:		return GL_FLOAT;
			case SHADER_DATA::FLOAT3:		return GL_FLOAT;
			case SHADER_DATA::FLOAT4:		return GL_FLOAT;
			case SHADER_DATA::MAT3:			return GL_FLOAT;
			case SHADER_DATA::MAT4:			return GL_FLOAT;
			case SHADER_DATA::INT:			return GL_INT;
			case SHADER_DATA::INT2:			return GL_INT;
			case SHADER_DATA::INT3:			return GL_INT;
			case SHADER_DATA::INT4:			return GL_INT;
			case SHADER_DATA::BOOL:			return GL_BOOL;
		}

		ASSERT(false, "ShaderData passed Unknown or Incorrect!");
		return (GLenum)0;
	}


	// ------------------------------------------------------------------------------
	// ----- Framebuffer Stuff -----
	static const uint s_MaxFBOSize = 8192; // Hardcoded because we don't have a Renderer-Capabilities system

	enum class FBO_TEXTURE_FORMAT
	{
		NONE = 0,
		RGBA8,				// Color
		RGBA16,
		DEPTH24STENCIL8,	// Depth & Stencil

		// Defaults
		DEPTH = DEPTH24STENCIL8
	};

	static GLenum GLTextureFormat(FBO_TEXTURE_FORMAT format)
	{
		switch (format)
		{
			case FBO_TEXTURE_FORMAT::DEPTH24STENCIL8:	return GL_DEPTH24_STENCIL8;
			case FBO_TEXTURE_FORMAT::RGBA8:				return GL_RGBA8;
			case FBO_TEXTURE_FORMAT::RGBA16:			return GL_RGBA16F;
		}

		ASSERT(false, "Invalid Format Passed to GLTextureFormat!");
		return GL_NONE;
	}

	static bool IsDepthFormatTexture(FBO_TEXTURE_FORMAT format)
	{
		switch (format)
		{
			case FBO_TEXTURE_FORMAT::DEPTH24STENCIL8: return true;
		}

		return false;
	}
}

#endif //_RENDERERUTILS_H_