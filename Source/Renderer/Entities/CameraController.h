#ifndef _CAMERACONTROLLER_H_
#define _CAMERACONTROLLER_H_

#include "Core/Globals.h"
#include "Camera.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>


class CameraController
{
public:

	// --- Public Class/Event Methods ---
	CameraController();
	CameraController(Camera& camera);
	~CameraController() = default;

	void OnUpdate(float dt);
	void OnMouseScroll(float scroll);

public:

	// --- Getters ---
	inline const Camera& GetCamera()	const { return m_Camera; }

	inline glm::vec3 GetPosition()		const { return m_Position; }
	inline glm::quat GetOrientation()	const { return glm::quat(glm::vec3(-m_Pitch, -m_Yaw, 0.0f)); }

	glm::vec3 GetUpVector()				const { return glm::rotate(GetOrientation(), glm::vec3(0.0f, 1.0f, 0.0f)); }
	glm::vec3 GetRightVector()			const { return glm::rotate(GetOrientation(), glm::vec3(1.0f, 0.0f, 0.0f)); }
	glm::vec3 GetForwardVector()		const { return glm::rotate(GetOrientation(), glm::vec3(0.0f, 0.0f, -1.0f)); }

	// --- Setters ---
	void SetOrientation(float x_angle, float y_angle);
	void SetPosition(const glm::vec3& position);

	inline void SetZoomLevel(float zoom_level)				{ m_ZoomLevel = zoom_level; }
	inline void SetMoveSpeed(float speed)					{ m_MoveSpeed = speed; }
	inline void SetRotationSpeed(float speed)				{ m_RotationSpeed = speed; }
	inline void SetCameraViewport(uint width, uint height)	{ m_Camera.SetViewport(width, height); }

private:

	// --- Private Camera Methods ---
	void CalculateRotation(const glm::vec2 rotation);
	void RotateCamera(const glm::vec2& rotation);
	void OrbitCamera(const glm::vec2& rotation);
	void PanCamera(const glm::vec2& panning);
	void ZoomCamera(float zoom);

	void RecalculateView();

private:

	// --- Camera ---
	Camera m_Camera;

	// --- Camera Transform Parameters ---
	glm::vec3 m_Position = glm::vec3(0.0f, 0.0f, 2.0f);
	float m_Pitch = 0.0f, m_Yaw = 0.0f;

	glm::vec3 m_FocalPoint = glm::vec3(0.0f);
	glm::vec2 m_InitialMousePosition = glm::vec2(0.0f);

	float m_ZoomLevel = 5.0f, m_MoveSpeed = 2.0f, m_SpeedMultiplier = 1.0f, m_RotationSpeed = 0.8f, m_PanSpeed = 2.4f;
};


#endif //_CAMERACONTROLLER_H_