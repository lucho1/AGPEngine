#ifndef _INPUT_H_
#define _INPUT_H_

#include <glm/glm.hpp>


// ------------------------------------------------------------------------------
namespace STATE
{
    enum BUTTON_STATE { IDLE, PRESS, PRESSED, RELEASE };
}

namespace MOUSE
{
    enum MOUSE_CODE { LEFT, RIGHT, MOUSE_BUTTON_COUNT };
}

namespace KEY
{
    enum KEY_CODE
    {
        NONE = -1, ESCAPE, SPACE, ENTER,
        A, B, C, D, E, F, G, H, I, J, K, L, M,
        N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
        K0, K1, K2, K3, K4, K5, K6, K7, K8, K9,        
        KEY_COUNT
    };
}



// ------------------------------------------------------------------------------
class Input
{
    friend class Window;
public:

    static void ResetInput();
    static void Update();

    const static glm::vec2 GetMousePos() { return m_MousePos; }
    const static glm::vec2 GetMouseMovement() { return m_MouseDelta; }

public:

    static bool IsKeyIdle(KEY::KEY_CODE key);
    static bool IsKeyPressed(KEY::KEY_CODE key);
    static bool IsKeyReleased(KEY::KEY_CODE key);

    static bool IsMouseButtonIdle(MOUSE::MOUSE_CODE button);
    static bool IsMouseButtonPressed(MOUSE::MOUSE_CODE button);
    static bool IsMouseButtonReleased(MOUSE::MOUSE_CODE button);

protected:

    static void SetKeyCallback(int glfw_key, int action);
    static void SetCursorCallback(double xpos, double ypos);
    static void SetScrollCallback(double xoff, double yoff);
    static void SetMouseCallback(int button, int action);

private:

    static KEY::KEY_CODE ConvertGLFWKeyCode(int glfw_key);

private:

    static glm::vec2 m_MousePos;
    static glm::vec2 m_MouseDelta;

    static STATE::BUTTON_STATE m_MouseButtons[MOUSE::MOUSE_BUTTON_COUNT];
    static STATE::BUTTON_STATE m_Keys[KEY::KEY_COUNT];
};

#endif //_INPUT_H_