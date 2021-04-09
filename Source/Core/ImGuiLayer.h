#ifndef _IMGUILAYER_H_
#define _IMGUILAYER_H_

#include "Engine.h"
#include "Window.h"

class ImGuiLayer
{
public:

	ImGuiLayer() = default;
	~ImGuiLayer();

	void Init(const Window* window);
	void Begin(const App& app);
	void PrepareRender(const App& app);
	void Render();

	bool CapturingKeyboard() const;
	bool CapturingMouse() const;
};

#endif //_IMGUILAYER_H_