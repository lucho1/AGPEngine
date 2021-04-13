#include "Input.h"
#include "ImGuiLayer.h"

#include "Core/Application/Application.h"


glm::vec2 Input::m_MousePos = glm::vec2(0.0f);
glm::vec2 Input::m_MouseDelta = glm::vec2(0.0f);

STATE::BUTTON_STATE Input::m_MouseButtons[MOUSE::MOUSE_BUTTON_COUNT] = {};
STATE::BUTTON_STATE Input::m_Keys[KEY::KEY_COUNT] = {};



// ------------------------------------------------------------------------------
void Input::ResetInput()
{
    ImGuiLayer* ui_layer = Application::Get().GetImGuiLayer();

    if (ui_layer->CapturingKeyboard())
    {
        for (uint i = 0; i < KEY::KEY_COUNT; ++i)
            m_Keys[i] = STATE::IDLE;
    }

    if (ui_layer->CapturingMouse())
    {
        for (uint i = 0; i < MOUSE::MOUSE_BUTTON_COUNT; ++i)
            m_MouseButtons[i] = STATE::IDLE;
    }
}


void Input::Update()
{
    ImGuiLayer* ui_layer = Application::Get().GetImGuiLayer();

    if (!ui_layer->CapturingKeyboard())
    {
        for (uint i = 0; i < KEY::KEY_COUNT; ++i)
        {
            if (m_Keys[i] == STATE::PRESS)
                m_Keys[i] = STATE::PRESSED;

            else if (m_Keys[i] == STATE::RELEASE)
                m_Keys[i] = STATE::IDLE;
        }
    }

    if (!ui_layer->CapturingMouse())
    {
        for (uint i = 0; i < MOUSE::MOUSE_BUTTON_COUNT; ++i)
        {
            if (m_MouseButtons[i] == STATE::PRESS)
                m_MouseButtons[i] = STATE::PRESSED;
            
            else if (m_MouseButtons[i] == STATE::RELEASE)
                m_MouseButtons[i] = STATE::IDLE;
        }
    }

    m_MouseDelta = glm::vec2(0.0f, 0.0f);
}



// ------------------------------------------------------------------------------
void Input::SetKeyCallback(int glfw_key, int action)
{
    KEY::KEY_CODE key = ConvertGLFWKeyCode(glfw_key);
    switch (action)
    {
        case GLFW_PRESS:    m_Keys[key] = STATE::PRESS; break;
        case GLFW_RELEASE:  m_Keys[key] = STATE::RELEASE; break;
    }
}

void Input::SetCursorCallback(double xpos, double ypos)
{
    m_MouseDelta.x = xpos - m_MousePos.x;
    m_MouseDelta.y = xpos - m_MousePos.y;
    m_MousePos = { xpos, ypos };
}

void Input::SetScrollCallback(double xoff, double yoff)
{
    // TODO: Zoom
    //WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
    //MouseScrolledEvent event((float)xOff, (float)yOff);
    //data.EventCallback(event);
}

void Input::SetMouseCallback(int button, int action)
{
    switch (button)
    {
        case GLFW_MOUSE_BUTTON_RIGHT:
        {
            if (action == GLFW_PRESS)
                m_MouseButtons[MOUSE::RIGHT] = STATE::PRESS;
            else if (action == GLFW_RELEASE)
                m_MouseButtons[MOUSE::RIGHT] = STATE::RELEASE;
    
            break;
        }
        case GLFW_MOUSE_BUTTON_LEFT:
        {
            if (action == GLFW_PRESS)
                m_MouseButtons[MOUSE::LEFT] = STATE::PRESS;
            else if (action == GLFW_RELEASE)
                m_MouseButtons[MOUSE::LEFT] = STATE::RELEASE;
    
            break;
        }
    }
}



// ------------------------------------------------------------------------------
bool Input::IsKeyIdle(KEY::KEY_CODE key)
{
    return m_Keys[key] == STATE::IDLE;
}

bool Input::IsKeyPressed(KEY::KEY_CODE key)
{
    return m_Keys[key] == STATE::PRESS || m_Keys[key] == STATE::PRESSED;
}

bool Input::IsKeyReleased(KEY::KEY_CODE key)
{
    return m_Keys[key] == STATE::RELEASE;
}

bool Input::IsMouseButtonIdle(MOUSE::MOUSE_CODE button)
{
    return m_MouseButtons[button] == STATE::IDLE;
}

bool Input::IsMouseButtonPressed(MOUSE::MOUSE_CODE button)
{
    return m_MouseButtons[button] == STATE::PRESSED || m_MouseButtons[button] == STATE::PRESS;
}

bool Input::IsMouseButtonReleased(MOUSE::MOUSE_CODE button)
{
    return m_MouseButtons[button] == STATE::RELEASE;
}



// ------------------------------------------------------------------------------
KEY::KEY_CODE Input::ConvertGLFWKeyCode(int glfw_key)
{
    switch (glfw_key)
    {
        case GLFW_KEY_ESCAPE:   return KEY::ESCAPE;
        case GLFW_KEY_SPACE:    return KEY::SPACE;
        case GLFW_KEY_ENTER:    return KEY::ENTER;
        case GLFW_KEY_A:        return KEY::A;
        case GLFW_KEY_B:        return KEY::B;
        case GLFW_KEY_C:        return KEY::C;
        case GLFW_KEY_D:        return KEY::D;
        case GLFW_KEY_E:        return KEY::E;
        case GLFW_KEY_F:        return KEY::F;
        case GLFW_KEY_G:        return KEY::G;
        case GLFW_KEY_H:        return KEY::H;
        case GLFW_KEY_I:        return KEY::I;
        case GLFW_KEY_J:        return KEY::J;
        case GLFW_KEY_K:        return KEY::K;
        case GLFW_KEY_L:        return KEY::L;
        case GLFW_KEY_M:        return KEY::M;
        case GLFW_KEY_N:        return KEY::N;
        case GLFW_KEY_O:        return KEY::O;
        case GLFW_KEY_P:        return KEY::P;
        case GLFW_KEY_Q:        return KEY::Q;
        case GLFW_KEY_R:        return KEY::R;
        case GLFW_KEY_S:        return KEY::S;
        case GLFW_KEY_T:        return KEY::T;
        case GLFW_KEY_U:        return KEY::U;
        case GLFW_KEY_V:        return KEY::V;
        case GLFW_KEY_W:        return KEY::W;
        case GLFW_KEY_X:        return KEY::X;
        case GLFW_KEY_Y:        return KEY::Y;
        case GLFW_KEY_Z:        return KEY::Z;
        case GLFW_KEY_0:        return KEY::K0;
        case GLFW_KEY_1:        return KEY::K1;
        case GLFW_KEY_2:        return KEY::K2;
        case GLFW_KEY_3:        return KEY::K3;
        case GLFW_KEY_4:        return KEY::K4;
        case GLFW_KEY_5:        return KEY::K5;
        case GLFW_KEY_6:        return KEY::K6;
        case GLFW_KEY_7:        return KEY::K7;
        case GLFW_KEY_8:        return KEY::K8;
        case GLFW_KEY_9:        return KEY::K9;
    }
}