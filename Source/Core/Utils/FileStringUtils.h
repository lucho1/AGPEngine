#ifndef _FILESTRINGUTILS_H_
#define _FILESTRINGUTILS_H_

#include "../Globals.h"

namespace FileUtils
{
	// --- File Standard Functions ---
	std::string MakePath(const std::string& dir, const std::string& filename);
	std::string GetDirectory(const std::string& path);

	// Last time file was modified. Check for file modifications for hot reloads.
	uint64 GetFileLastWriteTimestamp(const char* filepath);


	// --- Files Dialogues ---
	class FileDialogs
	{
	public:

		// If cancelled, returns empty string
		static std::string OpenFile(const char* filter);

		// If cancelled, returns empty string
		static std::string SaveFile(const char* filter, const char* filename);
	};
}

#endif //_FILESTRINGUTILS_H_