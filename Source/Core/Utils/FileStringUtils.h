#ifndef _FILESTRINGUTILS_H_
#define _FILESTRINGUTILS_H_

#include "../Globals.h"
#include <GLFW/glfw3.h>

namespace FileUtils
{
	// --- File Standard Functions ---
	// Reads file & returns string with its contents
	std::string ReadTextFile(const char* filepath);
	std::string MakePath(const std::string& dir, const std::string& filename);

	// Last time file was modified. Check for file modifications for hot reloads.
	uint64 GetFileLastWriteTimestamp(const char* filepath);
	std::string GetDirectoryPart(const std::string& path);


	// --- Files Dialogues ---
	class FileDialogs
	{
	public:

		// If cancelled, returns empty string
		static std::string OpenFile(const char* filter, GLFWwindow* window);

		// If cancelled, returns empty string
		static std::string SaveFile(const char* filter, const char* filename, GLFWwindow* window);
	};
}

namespace StringUtils
{
	std::string MakeString(const char* cstr);
}

#endif //_FILESTRINGUTILS_H_