#include "FileStringUtils.h"

// --- To get usage of windows file dialogs ---
#include <commdlg.h>
// --- To attach file dialogs to the engine's window ---
#define GLFW_EXPOSE_NATIVE_WIN32 // If defined, we can get Win32 functionalities we need
#include <GLFW/glfw3native.h>


// ------------------------------------------------------------------------------
namespace FileUtils
{
    // ----- Files Standard Functions -----
    std::string FileUtils::MakePath(const std::string& dir, const std::string& filename)
    {
        return dir + "/" + filename;
    }

    std::string FileUtils::GetDirectory(const std::string& path)
    {
        size_t last_slash = path.find_last_of("/\\");
        if (last_slash != path.npos)
            return path.substr((size_t)0, path.find_last_of("/\\"));
        else
            return "INVALID PATH!";
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