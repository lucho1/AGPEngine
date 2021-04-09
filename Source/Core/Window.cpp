#include "Window.h"
#include "Platform.h"
#include "Input.h"


static void ErrorCallback(int error_code, const char* error_msg)
{
    ENGINE_LOG("GLFW Error: %s (%i)", error_msg, error_code);
}



// ------------------------------------------------------------------------------
Window::~Window()
{
    glfwDestroyWindow(m_Window);
    ENGINE_LOG("Terminating GLFW");
    glfwTerminate();
}

void Window::Init(App& app)
{
    // -- GLFW Initialization --
    glfwSetErrorCallback(ErrorCallback);
    if (!glfwInit())
    {
        ENGINE_LOG("GLFW Initialization Failed\n");
        return;
    }
    
    // -- GLFW Hints --
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    #ifdef _DEBUG
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
    #endif

    // -- Window Creation --
    ILOG("Creating Window '%s' of %ix%i\n", m_Name.c_str(), m_Width, m_Height);
    m_Window = glfwCreateWindow(m_Width, m_Height, m_Name.c_str(), NULL, NULL);

    if (!m_Window)
    {
        ENGINE_LOG("glfwCreateWindow() failed\n");
        return;
    }
    
    // -- Graphics Context Creation --
    glfwMakeContextCurrent(m_Window);

    // -- GLFW Window User ptr & VSYNC --
    glfwSetWindowUserPointer(m_Window, (void*)&app);
    glfwSwapInterval(1); //TODO: Set VSYNC

    // -- Set GLFW Callbacks --
    SetGLFWEventCallbacks();

    // Load all OpenGL functions using the glfw loader function
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        ENGINE_LOG("Failed to initialize OpenGL context\n");
        return;
    }
}



// ------------------------------------------------------------------------------
void Window::SetGLFWEventCallbacks() const
{
    // Window/Application Events
    glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int w, int h)
        {
            ((Window*)GetApplicationWindow())->m_Width = w;
            ((Window*)GetApplicationWindow())->m_Height = h;
            ((App*)glfwGetWindowUserPointer(window))->displaySize = vec2(w, h);
        });

    glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
        {
            ((App*)glfwGetWindowUserPointer(window))->isRunning = false;
        });

    // Mouse Events
    glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods)
        {
            Input::SetMouseCallback(button, action);
        });

    glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOff, double yOff)
        {
            Input::SetScrollCallback(xOff, yOff);
        });

    glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos)
        {
            Input::SetCursorCallback(xPos, yPos);
        });


    // Key Events
    glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
        {             
            Input::SetKeyCallback(key, action);
        });

    //glfwSetCharCallback(m_Window, [](GLFWwindow* window, uint keycode)
    //    {
    //        WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
    //        KeyTypedEvent event(keycode);
    //        data.EventCallback(event);
    //    });
}