#ifndef _TRANSFORMCOMP_H_
#define _TRANSFORMCOMP_H_

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>


struct TransformComponent
{
	bool EntityActive = true;

	glm::vec3 Translation = glm::vec3(0.0f);
	glm::vec3 Rotation = glm::vec3(0.0f); // In Degrees!
	glm::vec3 Scale = glm::vec3(1.0f);

	TransformComponent() = default;
	TransformComponent(const TransformComponent&) = default;

	const glm::mat4 GetTransform() const
	{
		glm::mat4 rotation = glm::toMat4(glm::quat(glm::radians(Rotation)));
		return glm::translate(glm::mat4(1.0f), Translation) * rotation * glm::scale(glm::mat4(1.0f), Scale);
	}

	const glm::vec3 GetUpVector()				const { return glm::rotate(glm::quat(Rotation), glm::vec3(0.0f, 1.0f, 0.0f)); }
	const glm::vec3 GetRightVector()			const { return glm::rotate(glm::quat(Rotation), glm::vec3(1.0f, 0.0f, 0.0f)); }
	const glm::vec3 GetForwardVector()			const { return glm::rotate(glm::quat(Rotation), glm::vec3(0.0f, 0.0f, -1.0f)); }
};

#endif //_TRANSFORMCOMP_H_