#ifndef _FILESTRINGUTILS_H_
#define _FILESTRINGUTILS_H_

#include "../Globals.h"

namespace FileUtils
{
	// Reads file & returns string with its contents
	std::string ReadTextFile(const char* filepath);
	std::string MakePath(const std::string& dir, const std::string& filename);

	// Last time file was modified. Check for file modifications for hot reloads.
	uint64 GetFileLastWriteTimestamp(const char* filepath);
	std::string GetDirectoryPart(const std::string& path);
}

namespace StringUtils
{
	std::string MakeString(const char* cstr);
}

#endif //_FILESTRINGUTILS_H_