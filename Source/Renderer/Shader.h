#ifndef _SHADER_H_
#define _SHADER_H_

#include "Core/Globals.h"
#include <glad/glad.h>

class Shader
{
public:

	// --- Des/Construction ---
	Shader(const std::string& name, const std::string& vertex_src, const std::string& fragment_src);
	Shader(const std::string& filepath);
	~Shader();

	// --- Class Methods ---
	void Bind() const;
	void Unbind() const;

	// --- Getters ---
	const std::string& GetName() const { return m_Name; }

public:

	// --- Uniforms Cache ---
	int GetUniformLocation(const std::string& uniform_name) const;

	// --- Uniforms Methods ---
	void SetUniformInt(const std::string& uniform_name, int value);
	void SetUniformVec4(const std::string& uniform_name, float r, float g, float b, float a);

private:

	// --- Private Methods ---
	void CompileShader(const std::unordered_map<GLenum, std::string>& shader_sources);
	const std::unordered_map<GLenum, std::string> PreProcessShader(const std::string& source);
	const std::string ReadShaderFile(const std::string& filepath);

private:

	// --- Variables ---
	uint m_ID = 0;
	std::string m_Name = "unnamed";
	uint64 m_LastModificationTimestamp = 0;

	mutable std::unordered_map<std::string, int> m_UniformLocationCache;
};

#endif //_SHADER_H_