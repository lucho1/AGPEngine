#ifndef _FILESTRINGUTILS_H_
#define _FILESTRINGUTILS_H_

#include "../Globals.h"

namespace FileUtils
{
	// Reads file & returns string with its contents
	static std::string ReadTextFile(const char* filepath);
	static std::string MakePath(const std::string& dir, const std::string& filename);

	// Last time file was modified. Check for file modifications for hot reloads.
	static uint64 GetFileLastWriteTimestamp(const char* filepath);
	static std::string GetDirectoryPart(const std::string& path);
}

namespace StringUtils
{
	static std::string MakeString(const char* cstr);
}

#endif //_FILESTRINGUTILS_H_