#ifndef _LIGHT_H_
#define _LIGHT_H_

#include "Core/Globals.h"
#include <glm/glm.hpp>

class PointLight
{
	friend class Renderer;
private:

	PointLight(int id) : m_ID(id) {}

public:
	
	~PointLight() = default;

	inline uint GetID()		const { return m_ID; }
	inline operator uint()	const { return m_ID; }

	void SetLightData(const Ref<Shader>& shader, const std::string& lightstruct_uniform_name)
	{
		std::string unif_name = lightstruct_uniform_name + ".";

		shader->SetUniformVec4(unif_name + "Pos", glm::vec4(Position, 0.0f));
		shader->SetUniformVec4(unif_name + "Color", glm::vec4(Color, 1.0f));
		shader->SetUniformFloat(unif_name + "Intensity", Intensity);
		shader->SetUniformFloat(unif_name + "AttK", AttenuationK);
		shader->SetUniformFloat(unif_name + "AttL", AttenuationL);
		shader->SetUniformFloat(unif_name + "AttQ", AttenuationQ);
	}

public:

	glm::vec3 Position = glm::vec3(0.0f, 1.0f, 2.0f);
	glm::vec3 Color = glm::vec3(0.8f);

	float Intensity = 1.0f;
	float AttenuationK = 1.0f, AttenuationL = 0.09f, AttenuationQ = 0.032f;

	bool Active = true;

private:

	int m_ID = -1;
};

#endif //_LIGHT_H_