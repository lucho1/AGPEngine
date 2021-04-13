#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#include "Globals.h"

#include "Window.h"
#include "ImGuiLayer.h"
#include "Renderer/Renderer.h"


// --- Main Declaration, Defined on Main ---
int main(int argc, char** argv);



// --- Application Class ---
class Application
{
public:

	// --- Con/Destruction ---
	Application(const std::string& name, uint window_width, uint window_height, float framerate);
	~Application();

	// --- Class Methods ---
	void CloseApplication() { m_Running = false; }
	void OnWindowResize(uint width, uint height);

	// --- Getters ---
	inline Window& GetWindow()			const	{ return *m_AppWindow; }
	inline ImGuiLayer* GetImGuiLayer()	const	{ return m_ImGuiLayer; }
	inline static Application& Get()			{ return *s_ApplicationInstance; }

private:

	void Update();
	void Render();
	void RenderUI();

private:

	// --- App Singleton & Classes ---
	static Application* s_ApplicationInstance;
	friend int ::main(int argc, char** argv);

	UniquePtr<Window> m_AppWindow = nullptr;
	UniquePtr<Renderer> m_AppRenderer = nullptr;
	ImGuiLayer* m_ImGuiLayer = nullptr;

	// --- App Properties ---
	bool m_Running = true;
	float m_LastFrameTime = 0.0f, m_DeltaTime = 0.0f;
};


#endif //_APPLICATION_H_