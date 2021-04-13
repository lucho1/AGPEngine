#ifndef _IMGUILAYER_H_
#define _IMGUILAYER_H_

#include "Engine.h"
#include "Window.h"

class ImGuiLayer
{
public:

	ImGuiLayer() = default;
	~ImGuiLayer();

	void Init();
	void Begin();
	void Render();

	bool CapturingKeyboard() const;
	bool CapturingMouse() const;
};

#endif //_IMGUILAYER_H_