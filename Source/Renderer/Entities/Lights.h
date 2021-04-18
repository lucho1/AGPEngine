#ifndef _LIGHT_H_
#define _LIGHT_H_

#include "Core/Globals.h"
#include <glm/glm.hpp>

class PointLight
{
public:

	// --- Des/Constructor ---
	PointLight() = default;
	~PointLight() = default;

public:

	glm::vec3 Position = glm::vec3(0.0f, 1.0f, 2.0f);
	glm::vec3 Color = glm::vec3(0.8f);

	float Intensity = 1.0f;
	float AttenuationK = 1.0f, AttenuationL = 0.09f, AttenuationQ = 0.032f;
};

#endif //_LIGHT_H_