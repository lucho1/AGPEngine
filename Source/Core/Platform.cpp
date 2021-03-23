//
// Platform.cpp : This file contains the 'main' function. Program execution begins and ends there.
// The platform layer is in charge to create the environment necessary so the engine disposes of what
// it needs in order to create the application (e.g. window, graphics context, I/O, allocators, etc).
//

#include "Platform.h"
#include "Window.h"
#include "Input.h"
#include "ImGuiLayer.h"


#define GLOBAL_FRAME_ARENA_SIZE MB(16)
unsigned char* GlobalFrameArenaMemory = NULL;
uint GlobalFrameArenaHead = 0;

static Window* m_Window = nullptr;
void* GetApplicationWindow() { return (void*)m_Window; }

static ImGuiLayer* m_ImGuiLayer = nullptr;
void* GetImGuiLayer() { return (void*)m_ImGuiLayer; }



int main()
{
    App app         = {};
    app.deltaTime   = 1.0f/60.0f;
    app.displaySize = ivec2(960, 540);
    app.isRunning   = true;


    m_Window = new Window(960, 540, "AGPEngine by Lucho Suaya");
    m_Window->Init(app);

    m_ImGuiLayer = new ImGuiLayer();
    m_ImGuiLayer->Init(m_Window);

    double lastFrameTime = glfwGetTime();
    GlobalFrameArenaMemory = (unsigned char*)malloc(GLOBAL_FRAME_ARENA_SIZE);
    Init(&app);

    while (app.isRunning)
    {
        // Tell GLFW to call platform callbacks
        glfwPollEvents();
        m_ImGuiLayer->Update(app);

        // Clear input state if required by ImGui
        Input::ResetInput();

        // Update
        Update(&app);

        // Transition input key/button states
        Input::Update();

        // Render
        Render(&app);

        // ImGui Render
        m_ImGuiLayer->Render();

        // Present image on screen
        glfwSwapBuffers(&m_Window->GetWindow());

        // Frame time
        double currentFrameTime = glfwGetTime();
        app.deltaTime = (float)(currentFrameTime - lastFrameTime);
        lastFrameTime = currentFrameTime;

        // Reset frame allocator
        GlobalFrameArenaHead = 0;
    }

    free(GlobalFrameArenaMemory);
    delete m_Window;
    delete m_ImGuiLayer;
    return 0;
}

uint Strlen(const char* string)
{
    uint len = 0;
    while (*string++) ++len;
    return len;
}

void* PushSize(uint byteCount)
{
    ASSERT(GlobalFrameArenaHead + byteCount <= GLOBAL_FRAME_ARENA_SIZE,
           "Trying to allocate more temp memory than available");

    unsigned char* curPtr = GlobalFrameArenaMemory + GlobalFrameArenaHead;
    GlobalFrameArenaHead += byteCount;
    return curPtr;
}

void* PushBytes(const void* bytes, uint byteCount)
{
    ASSERT(GlobalFrameArenaHead + byteCount <= GLOBAL_FRAME_ARENA_SIZE,
            "Trying to allocate more temp memory than available");

    unsigned char* srcPtr = (unsigned char*)bytes;
    unsigned char* curPtr = GlobalFrameArenaMemory + GlobalFrameArenaHead;
    unsigned char* dstPtr = GlobalFrameArenaMemory + GlobalFrameArenaHead;
    GlobalFrameArenaHead += byteCount;
    while (byteCount--) *dstPtr++ = *srcPtr++;
    return curPtr;
}

unsigned char* PushChar(unsigned char c)
{
    ASSERT(GlobalFrameArenaHead + 1 <= GLOBAL_FRAME_ARENA_SIZE,
            "Trying to allocate more temp memory than available");
    unsigned char* ptr = GlobalFrameArenaMemory + GlobalFrameArenaHead;
    GlobalFrameArenaHead++;
    *ptr = c;
    return ptr;
}

std::string MakeString(const char *cstr)
{
    char* str = (char*)PushBytes(cstr, Strlen(cstr));
    PushChar(0);
    return str;
}

std::string MakePath(const std::string& dir, const std::string& filename)
{
    char* str = (char*)PushBytes(dir.c_str(), dir.size());
    PushChar('/');
    PushBytes(filename.c_str(), filename.size());
    PushChar(0);

    return str;
}

std::string GetDirectoryPart(const std::string& path)
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

std::string ReadTextFile(const char* filepath)
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
        ELOG("fopen() failed reading file %s", filepath);

    return ret;
}

uint64 GetFileLastWriteTimestamp(const char* filepath)
{
#ifdef _WIN32
    union Filetime2u64 {
        FILETIME filetime;
        unsigned long long int      u64time;
    } conversor;

    WIN32_FILE_ATTRIBUTE_DATA Data;
    if(GetFileAttributesExA(filepath, GetFileExInfoStandard, &Data)) {
        conversor.filetime = Data.ftLastWriteTime;
        return(conversor.u64time);
    }
#else
    // NOTE: This has not been tested in unix-like systems
    struct stat attrib;
    if (stat(filepath, &attrib) == 0) {
        return attrib.st_mtime;
    }
#endif

    return 0;
}