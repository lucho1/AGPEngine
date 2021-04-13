#ifndef _GLOBALS_H_
#define _GLOBALS_H_


// --- Standard Definitions & Includes ---
#pragma warning(disable : 4267) // conversions, possible loss of data
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
#include <iostream>

#include <assert.h>
#include <math.h>
#include <vector>
#include <string>
#include <unordered_map>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>


// --- Engine Definitions ---
#define ASSERT(condition, message) assert((condition) && message)
#define ARRAY_COUNT(array) (sizeof(array)/sizeof(array[0]))

// Logs string to outputs configured in platform layer & engine console. By default, string is printed in the output console of VisualStudio
static void LogString(const char* str)
{
	std::cout << str << std::endl;
    #ifdef _WIN32
        OutputDebugStringA(str);
        OutputDebugStringA("\n");
    #else
        fprintf(stderr, "%s\n", str);
    #endif
}

#define ILOG(...) { char logBuffer[1024] = {}; sprintf_s(logBuffer, __VA_ARGS__); LogString(logBuffer); }
#define ENGINE_LOG(...) ILOG(__VA_ARGS__)


// --- Conversions ---
#define KBTOBYTE(val) (1024*(val))
#define MBTOBYTE(val) (1024*KBTOBYTE(val))

#define PI  3.14159265359f
#define TAU 6.28318530718f


// --- Typedefs ---
typedef unsigned int            uint;
typedef unsigned long long int  uint64;
typedef unsigned short          uint16_t;


// --- Smart Ptrs ----
template<typename T>
using UniquePtr = std::unique_ptr<T>;
template<typename T, typename ... Args>
constexpr UniquePtr<T> CreateUnique(Args&& ... args) { return std::make_unique<T>(std::forward<Args>(args)...); }

template<typename T>
using Ref = std::shared_ptr<T>;
template<typename T, typename ... Args>
constexpr Ref<T> CreateRef(Args&& ... args) { return std::make_shared<T>(std::forward<Args>(args)...); }


// --- Memory Pool ---
#define GLOBAL_FRAME_ARENA_SIZE MBTOBYTE(16)
static unsigned char* GlobalFrameArenaMemory = NULL;
static uint GlobalFrameArenaHead = 0;

#endif //_GLOBALS_H_