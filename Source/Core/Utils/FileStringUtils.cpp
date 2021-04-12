#include "FileStringUtils.h"

// --- To get usage of windows file dialogs ---
#include <commdlg.h>
// --- To attach file dialogs to the engine's window ---
#define GLFW_EXPOSE_NATIVE_WIN32 // If defined, we can get Win32 functionalities we need
#include <GLFW/glfw3native.h>



// ------------------------------------------------------------------------------
namespace StringUtils
{
    // --- Non-Header Global Functions ---
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


    // --- Header Functions ---
    std::string StringUtils::MakeString(const char* cstr)
    {
        char* str = (char*)PushBytes(cstr, Strlen(cstr));
        PushChar(0);
        return str;
    }
}



// ------------------------------------------------------------------------------
namespace FileUtils
{
    // ----- Files Standard Functions -----
    std::string FileUtils::ReadTextFile(const char* filepath)
    {
        FILE* file = fopen(filepath, "rb");
        std::string ret;
        if (file)
        {
            fseek(file, 0, SEEK_END);
            uint length = ftell(file);
            fseek(file, 0, SEEK_SET);
            
            char* str = (char*)StringUtils::PushSize(length + 1);
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
        char* str = (char*)StringUtils::PushBytes(dir.c_str(), dir.size());
        StringUtils::PushChar('/');
        StringUtils::PushBytes(filename.c_str(), filename.size());
        StringUtils::PushChar(0);
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

        char* str = (char*)StringUtils::PushBytes(path.c_str(), length);
        StringUtils::PushChar(0);
        return str;
    }



    // ----- Files Dialogues Functions -----
    std::string FileDialogs::OpenFile(const char* filter, GLFWwindow* window)
    {
        // -- Initialize OPENFILENAME to 0 (Common Dialog Box Structure) --
        OPENFILENAMEA open_file_name;
        ZeroMemory(&open_file_name, sizeof(OPENFILENAME));

        // -- Set OPENFILENAME --
        open_file_name.lStructSize = sizeof(OPENFILENAME); // Size
        
        open_file_name.hwndOwner = glfwGetWin32Window(window); // Owner of dialog window to engine's window

        // -- Buffer File --
        CHAR file[260] = { 0 };
        open_file_name.lpstrFile = file;
        open_file_name.nMaxFile = sizeof(file);
        open_file_name.lpstrFilter = filter;
        open_file_name.nFilterIndex = 1;
        open_file_name.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR; // If this is not here, it will change the working directory

        // -- Check for current directory and set it if exists --
        CHAR directory[260] = { 0 };
        if (GetCurrentDirectoryA(256, directory))
            open_file_name.lpstrInitialDir = directory;

        // -- If file (Ascii) is open (exists), return it --
        if (GetOpenFileNameA(&open_file_name) == TRUE)
            return open_file_name.lpstrFile;

        return std::string();
    }

    std::string FileDialogs::SaveFile(const char* filter, const char* filename, GLFWwindow* window)
    {
        // -- Initialize OPENFILENAME to 0 (Common Dialog Box Structure) --
        OPENFILENAMEA open_file_name;
        ZeroMemory(&open_file_name, sizeof(OPENFILENAME));

        

        // -- Set OPENFILENAME --
        open_file_name.lStructSize = sizeof(OPENFILENAME); // Size
        open_file_name.hwndOwner = glfwGetWin32Window(window); // Owner of dialog window to engine's window		

        // -- Buffer File --
        CHAR file[260] = { 0 };
        if (filename)
        {
            std::strncpy(file, filename, sizeof(file));	// Copy filename to file[]
            file[sizeof(file) - 1] = 0;					// Add null-terminator at the end (in case filename is >= than the file[] max size)
        }

        open_file_name.lpstrFile = file;
        open_file_name.nMaxFile = sizeof(file);
        open_file_name.lpstrFilter = filter;
        open_file_name.nFilterIndex = 1;
        open_file_name.lpstrDefExt = strchr(filter, '\0') + 1;								// Default extension (gets it from filter)
        open_file_name.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;	// If this is not here, it will change the working directory

        // -- Check for current directory and set it if exists --
        CHAR directory[260] = { 0 };
        if (GetCurrentDirectoryA(256, directory))
            open_file_name.lpstrInitialDir = directory;

        // -- If file (Ascii) is open (exists), return it --
        if (GetSaveFileNameA(&open_file_name) == TRUE)
            return open_file_name.lpstrFile;

        return std::string();
    }
}