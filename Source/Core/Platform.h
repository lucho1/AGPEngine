#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#include "Globals.h"
#include "Engine.h"
//#include "Window.h"
//#include <GLFW/glfw3.h> //TODO: Delete this line

// Platform.h : This file contains basic platform types and tools. Also, it exposes
// the necessary functions for the Engine to communicate with the Platform layer.

std::string MakeString(const char *cstr);

std::string MakePath(const std::string& dir, const std::string& filename);

std::string GetDirectoryPart(const std::string& path);

/**
 * Reads a whole file and returns a string with its contents. The returned string
 * is temporary and should be copied if it needs to persist for several frames.
 */
std::string ReadTextFile(const char *filepath);

/**
 * It retrieves a timestamp indicating the last time the file was modified.
 * Can be useful in order to check for file modifications to implement hot reloads.
 */
uint64 GetFileLastWriteTimestamp(const char *filepath);

void* GetApplicationWindow();
void* GetImGuiLayer();


#define ILOG(...)                 \
{                                 \
char logBuffer[1024] = {};        \
sprintf(logBuffer, __VA_ARGS__);  \
}
//LogString(logBuffer);             \
}

#define ELOG(...) ILOG(__VA_ARGS__)

#define ARRAY_COUNT(array) (sizeof(array)/sizeof(array[0]))

#define ASSERT(condition, message) assert((condition) && message)



#endif //_PLATFORM_H_