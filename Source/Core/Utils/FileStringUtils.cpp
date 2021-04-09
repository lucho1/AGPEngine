#include "FileStringUtils.h"


// ------------------------------------------------------------------------------
static uint Strlen(const char* string)
{
    uint len = 0;
    while (*string++) ++len;
    return len;
}

static void* PushSize(uint byteCount)
{
    ASSERT(GlobalFrameArenaHead + byteCount <= GLOBAL_FRAME_ARENA_SIZE, "Trying to allocate more temp memory than available");
    unsigned char* curPtr = GlobalFrameArenaMemory + GlobalFrameArenaHead;
    GlobalFrameArenaHead += byteCount;
    return curPtr;
}

static void* PushBytes(const void* bytes, uint byteCount)
{
    ASSERT(GlobalFrameArenaHead + byteCount <= GLOBAL_FRAME_ARENA_SIZE, "Trying to allocate more temp memory than available");
    unsigned char* srcPtr = (unsigned char*)bytes;
    unsigned char* curPtr = GlobalFrameArenaMemory + GlobalFrameArenaHead;
    unsigned char* dstPtr = GlobalFrameArenaMemory + GlobalFrameArenaHead;

    GlobalFrameArenaHead += byteCount;
    while (byteCount--) *dstPtr++ = *srcPtr++;
    return curPtr;
}

static unsigned char* PushChar(unsigned char c)
{
    ASSERT(GlobalFrameArenaHead + 1 <= GLOBAL_FRAME_ARENA_SIZE, "Trying to allocate more temp memory than available");
    unsigned char* ptr = GlobalFrameArenaMemory + GlobalFrameArenaHead;
    GlobalFrameArenaHead++;
    *ptr = c;
    return ptr;
}



// ------------------------------------------------------------------------------
namespace FileUtils
{
    std::string FileUtils::ReadTextFile(const char* filepath)
    {
        FILE* file = fopen(filepath, "rb");
         std::string ret;
        if (file)
        {
            fseek(file, 0, SEEK_END);
            uint length = ftell(file);
            fseek(file, 0, SEEK_SET);

            char* str = (char*)PushSize(length + 1);
            fread(str, sizeof(char), length, file);
            str[length] = '\0';

            fclose(file);
            ret = str;
        }
        else
            ENGINE_LOG("fopen() failed reading file %s", filepath);

        return ret;
    }

    std::string FileUtils::MakePath(const std::string& dir, const std::string& filename)
    {
        char* str = (char*)PushBytes(dir.c_str(), dir.size());
        PushChar('/');
        PushBytes(filename.c_str(), filename.size());
        PushChar(0);
        return str;
    }

    uint64 FileUtils::GetFileLastWriteTimestamp(const char* filepath)
    {
        #ifdef _WIN32
                union Filetime2u64
                {
                    FILETIME filetime;
                    unsigned long long int u64time;
                } conversor;
        
                WIN32_FILE_ATTRIBUTE_DATA Data;
                if (GetFileAttributesExA(filepath, GetFileExInfoStandard, &Data))
                {
                    conversor.filetime = Data.ftLastWriteTime;
                    return(conversor.u64time);
                }
        #else
                // NOTE: This has not been tested in unix-like systems
                struct stat attrib;
                if (stat(filepath, &attrib) == 0)
                    return attrib.st_mtime;
        #endif
        return 0;
    }

    std::string FileUtils::GetDirectoryPart(const std::string& path)
    {
        size_t length = path.size();
        while (length >= 0)
        {
            length--;
            if (path[length] == '/' || path[length] == '\\')
                break;
        }

        char* str = (char*)PushBytes(path.c_str(), length);
        PushChar(0);
        return str;
    }
}

namespace StringUtils
{
    std::string StringUtils::MakeString(const char* cstr)
    {
        char* str = (char*)PushBytes(cstr, Strlen(cstr));
        PushChar(0);
        return str;
    }
}