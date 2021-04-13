#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#include "Core/Globals.h"

#include "Core/Platform/Window.h"
#include "Core/Platform/ImGuiLayer.h"


// --- Main Declaration, Defined on Main ---
int main(int argc, char** argv);



// --- Memory Usage ---
class MemoryMetrics
{
public:

	uint GetAllocations()			const { return m_TotalAllocated; }
	uint GetDeallocations()			const { return m_TotalFreed; }
	uint GetCurrentMemoryUsage()	const { return m_TotalAllocated - m_TotalFreed; }

	void AddAllocation(uint size)	const { m_TotalAllocated += size; }
	void AddDeallocation(uint size)	const { m_TotalFreed += size; }

private:

	mutable uint m_TotalAllocated = 0;
	mutable uint m_TotalFreed = 0;
};



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

	inline static const MemoryMetrics& GetMemoryMetrics() { return s_MemoryMetrics; }

private:

	void Update();

private:

	// --- Memory ---
	static MemoryMetrics s_MemoryMetrics;

	// --- App Singleton & Classes ---
	static Application* s_ApplicationInstance;
	friend int ::main(int argc, char** argv);

	UniquePtr<Window> m_AppWindow = nullptr;
	ImGuiLayer* m_ImGuiLayer = nullptr;

	// --- App Properties ---
	bool m_Running = true;
	float m_LastFrameTime = 0.0f, m_DeltaTime = 0.0f;
};

#endif //_APPLICATION_H_