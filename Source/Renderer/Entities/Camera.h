#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "Core/Globals.h"
#include <glm/glm.hpp>

class Camera
{
	friend class CameraController;
public:

	// --- Public Class Methods ---
	Camera();
	Camera(float FOV, float ar = 1.778f, float nclip = 0.01f, float fclip = 10000.0f);
	virtual ~Camera() = default;

	// Sets the camera's Viewport
	void SetCameraParameters(float FOV = 45.0f, float nclip = 0.1f, float fclip = 10000.0f);

public:

	// --- Getters ---
	inline float GetNearPlane()						const { return m_NearPlane; }
	inline float GetFarPlane()						const { return m_FarPlane; }
	inline float GetFOV()							const { return m_FOV; }
	inline float GetAspectRato()					const { return m_AR; }
	inline glm::ivec2 GetViewportSize()				const { return m_ViewportSize; }

	inline glm::mat4 GetViewProjection()			const { return m_Projection * m_View; }		// PxV since OGL is column-major, though in DX should be VxP (careful! TODO!)

public:

	// --- Setters ---
	void SetViewport(uint width, uint height);
	inline void SetFOV(float FOV)					{ m_FOV = FOV;			CalculateProjectionMatrix(); }
	inline void SetNearPlane(float nclip)			{ m_NearPlane = nclip;	CalculateProjectionMatrix(); }
	inline void SetFarPlane(float fclip)			{ m_FarPlane = fclip;	CalculateProjectionMatrix(); }

private:

	// --- Private Camera Methods ---
	void CalculateViewMatrix(const glm::vec3& position, const glm::quat& orientation);
	void CalculateProjectionMatrix();

private:

	glm::mat4 m_Projection = glm::mat4(1.0f);
	glm::mat4 m_View = glm::mat4(1.0f);

	float m_AR = 1.778f, m_FOV = 45.0f, m_NearPlane = 0.1f, m_FarPlane = 10000.0f;
	glm::ivec2 m_ViewportSize = glm::ivec2(1280, 720);
};

#endif //_CAMERA_H_