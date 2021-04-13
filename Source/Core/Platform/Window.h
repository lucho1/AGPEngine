#ifndef _WINDOW_H_
#define _WINDOW_H_

#include "Core/Globals.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>


class Window
{
public:

	// --- Public Class Methods ---
	Window(uint width, uint height, std::string name) : m_Width(width), m_Height(height), m_Name(name) {}
	~Window();

	void Init();
	void Update();
	void ResizeWindow(uint width, uint height);
	void CloseWindow();

	// --- Getters/Setters ---
	void SetVSYNC(bool enabled);

	inline void* GetNativeWindow()	const { return m_Window; }
	inline uint GetWidth()			const { return m_Width; }
	inline uint GetHeight()			const { return m_Height; }

	float GetGLFWTime()		const;

private:

	// --- Private Class Methods ---
	void SetGLFWEventCallbacks() const;

private:

	// --- Variables ---
	GLFWwindow* m_Window = nullptr;
	uint m_Width = 960, m_Height = 540;
	std::string m_Name = "Unnamed Window";
	bool m_VSYNC = true;
};

#endif //_WINDOW_H_