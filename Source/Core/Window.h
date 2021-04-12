#ifndef _WINDOW_H_
#define _WINDOW_H_

#include "Globals.h"
#include "Engine.h"

#include <GLFW/glfw3.h>

class Window
{
public:

	Window(uint width, uint height, std::string name) : m_Width(width), m_Height(height), m_Name(name) {}
	~Window();

	void Init(App& app);
	inline GLFWwindow& GetWindow()	const { return *m_Window; }

	inline uint GetWidth()			const { return m_Width; }
	inline uint GetHeight()			const { return m_Height; }

private:

	void SetGLFWEventCallbacks() const;

private:

	GLFWwindow* m_Window = nullptr;
	uint m_Width = 960, m_Height = 540;
	std::string m_Name = "Unnamed Window";
};

#endif //_WINDOW_H_