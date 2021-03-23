#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#define _CRT_SECURE_NO_WARNINGS

#ifdef _WIN32
    #define VC_EXTRALEAN
    #define WIN32_LEAN_AND_MEAN
    #include <Windows.h>
#else
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <unistd.h>
#endif

#include <stdio.h>

#include <assert.h>
#include <math.h>
#include <vector>
#include <string>
#include <unordered_map>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>



#pragma warning(disable : 4267) // conversion from X to Y, possible loss of data

#define KB(count) (1024*(count))
#define MB(count) (1024*KB(count))
#define GB(count) (1024*MB(count))

#define PI  3.14159265359f
#define TAU 6.28318530718f


typedef unsigned int uint;
typedef unsigned long long int uint64;
typedef unsigned short     uint16_t;

/**
 * It logs a string to whichever outputs are configured in the platform layer.
 * By default, the string is printed in the output console of VisualStudio.
 */
//void LogString(const char* str)
//{
//#ifdef _WIN32
//    OutputDebugStringA(str);
//    OutputDebugStringA("\n");
//#else
//    fprintf(stderr, "%s\n", str);
//#endif
//}




//#define GLOBAL_POOL_SIZE MB(16)
//unsigned char* GlobalPoolMemory = NULL;
//uint GlobalPoolHead = 0;


#endif //_GLOBALS_H_