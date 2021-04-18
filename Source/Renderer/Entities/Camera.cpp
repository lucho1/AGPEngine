#include "Camera.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>


// ------------------------------------------------------------------------------
Camera::Camera()
{
	CalculateProjectionMatrix();
	CalculateViewMatrix(glm::vec3(0.0f, 0.0f, 2.0f), glm::identity<glm::quat>());
}

Camera::Camera(float FOV, float ar, float nclip, float fclip)
{
	CalculateProjectionMatrix();
	CalculateViewMatrix(glm::vec3(0.0f, 0.0f, 2.0f), glm::identity<glm::quat>());
}



// ------------------------------------------------------------------------------
void Camera::SetCameraParameters(float FOV, float nclip, float fclip)
{
	m_FOV = FOV; m_NearPlane = nclip; m_FarPlane = fclip;
	CalculateProjectionMatrix();
}

void Camera::SetViewport(uint width, uint height)
{
	if (height == 0)
	{
		ENGINE_LOG("Tried to set Camera Viewport to 0, aborting!");
		return;
	}

	m_ViewportSize = glm::ivec2(width, height);
	m_AR = (float)width / (float)height;
	CalculateProjectionMatrix();
}



// ------------------------------------------------------------------------------
void Camera::CalculateViewMatrix(const glm::vec3& position, const glm::quat& orientation)
{
	glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::toMat4(orientation);
	m_View = glm::inverse(transform);
}

void Camera::CalculateProjectionMatrix()
{
	m_Projection = glm::perspective(glm::radians(m_FOV), m_AR, m_NearPlane, m_FarPlane);
}
