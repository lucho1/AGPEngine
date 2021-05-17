#ifndef _LIGHT_H_
#define _LIGHT_H_

#include "Core/Globals.h"
#include "Renderer/Resources/Buffers.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// --- Base Light ---
class Light
{
public:

	Light() = default;

	glm::vec3 Direction = glm::vec3(1.0f);
	glm::vec3 Color = glm::vec3(0.8f);
	float Intensity = 1.0f;
};


// --- Point Light ---
class PointLight : public Light
{
	friend class Renderer;
private:

	PointLight(int id) : m_ID(id) {}

public:
	
	~PointLight() = default;

	inline uint GetID()		const { return m_ID; }
	inline operator uint()	const { return m_ID; }

	float GetLightRadius(float max_distance)
	{
		return 1.0f / (AttenuationK + AttenuationL * max_distance + AttenuationQ * max_distance * max_distance);
	}

	void SetLightData(ShaderStorageBuffer* ssbo, const std::string& lightunif_name)
	{
		ssbo->SetData(lightunif_name + "Pos", glm::value_ptr(glm::vec4(Position, 0.0f)));
		ssbo->SetData(lightunif_name + "Color", glm::value_ptr(glm::vec4(Color, 1.0f)));
		ssbo->SetData(lightunif_name + "Intensity", &Intensity);
		ssbo->SetData(lightunif_name + "AttK", &AttenuationK);
		ssbo->SetData(lightunif_name + "AttL", &AttenuationL);
		ssbo->SetData(lightunif_name + "AttQ", &AttenuationQ);
	}

public:

	glm::vec3 Position = glm::vec3(0.0f, 1.0f, 2.0f);
	float AttenuationK = 1.0f, AttenuationL = 0.09f, AttenuationQ = 0.032f;
	bool Active = true;

private:

	int m_ID = -1;
};

#endif //_LIGHT_H_