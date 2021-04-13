#include "Shader.h"

#include "Renderer/Utils/RendererUtils.h"
#include "Core/Utils/FileStringUtils.h"

#include <filesystem>
#include <fstream>


// ------------------------------------------------------------------------------
Shader::Shader(const std::string& name, const std::string& vertex_src, const std::string& fragment_src)
{
	// -- Set source for Shader --
	std::unordered_map<GLenum, std::string> sources;
	sources[GL_VERTEX_SHADER] = vertex_src;
	sources[GL_FRAGMENT_SHADER] = fragment_src;

	// -- Compile Shader --
	CompileShader(sources);
}

Shader::Shader(const std::string& filepath)
{
	// -- Compile Shader --
	CompileShader(PreProcessShader(ReadShaderFile(filepath)));

	// -- Shader name from filepath --
	//size_t lastSlash = filepath.find_last_of("/\\");
	//size_t lastDot = filepath.rfind('.');
	//
	//lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
	//lastDot = (lastDot == std::string::npos ? filepath.size() : lastDot) - lastSlash;
	//m_Name = filepath.substr(lastSlash, lastDot);	

	// This is better:
	std::filesystem::path path = filepath;
	m_Name = path.stem().string(); // Returns the file's name stripped of the extension
	
	// -- File Last Modification Time --
	m_LastModificationTimestamp = FileUtils::GetFileLastWriteTimestamp(filepath.c_str()); // If problems, try: std::filesystem::last_write_time(path);
	m_Path = filepath;
}

Shader::~Shader()
{
	glDeleteProgram(m_ID);
}

void Shader::Bind() const
{
	glUseProgram(m_ID);
}

void Shader::Unbind() const
{
	glUseProgram(0);
}

void Shader::CheckLastModification()
{
	uint64 last_time = FileUtils::GetFileLastWriteTimestamp(m_Path.c_str());
	if (last_time > m_LastModificationTimestamp)
	{
		CompileShader(PreProcessShader(ReadShaderFile(m_Path)));
		m_LastModificationTimestamp = last_time; // If problems, try: std::filesystem::last_write_time(path);
	}
}



// ------------------------------------------------------------------------------
int Shader::GetUniformLocation(const std::string& uniform_name) const
{
	if (m_UniformLocationCache.find(uniform_name) != m_UniformLocationCache.end())
		return m_UniformLocationCache[uniform_name];

	int loc = glGetUniformLocation(m_ID, uniform_name.c_str());
	if (loc == -1)
		ENGINE_LOG("Warning! Uniform '%s' doesn't exist! (loc == -1)", uniform_name.c_str());

	m_UniformLocationCache[uniform_name] = loc;
	return loc;
}


void Shader::SetUniformInt(const std::string& uniform_name, int value)
{
	glUniform1i(GetUniformLocation(uniform_name), value);
}

void Shader::SetUniformVec4(const std::string& uniform_name, const glm::vec4& color)
{
	glUniform4f(GetUniformLocation(uniform_name), color.r, color.g, color.b, color.a);
}

void Shader::SetUniformMat4(const std::string& uniform_name, const glm::mat4& matrix)
{
	glUniformMatrix4fv(GetUniformLocation(uniform_name), 1, GL_FALSE, glm::value_ptr(matrix));
}



// ------------------------------------------------------------------------------
void Shader::CompileShader(const std::unordered_map<GLenum, std::string>& shader_sources)
{
	// -- Create Program --
	GLuint program = glCreateProgram();
	std::vector<GLenum> glShaderIDs;
	glShaderIDs.reserve(shader_sources.size());

	// Instead, you could create an array, which is much better than a vector:
	//std::array<GLenum, 2> glShaderIDs;
	// int glShaderIDIndex = 0; // In this case, the glShaderIDs.push_back() - downwards -, should be substituted by a glShaderIDs[glShaderIDIndex++] = shader;

	for (auto&& [key, value] : shader_sources)
	{
		GLenum type = key;
		const std::string& source = value;

		// -- Create empty Shader handle --
		GLuint shader = glCreateShader(type);

		// -- Send Shader to OGL --
		const GLchar* GLShaderSource = source.c_str();
		glShaderSource(shader, 1, &GLShaderSource, 0);

		// -- Compile Shader --
		glCompileShader(shader);

		// -- Check for Compilation Errors --
		GLint isCompiled = 0;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
		if (isCompiled == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<GLchar> infoLog(maxLength);
			glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);
			glDeleteShader(shader);

			ENGINE_LOG("%s Shader Compilation Error: %s", RendererUtils::StringFromShaderType(type), infoLog.data());
			ASSERT(false, "Shader Compilation Failure!");
			break;
		}

		// Attach our shaders to our program
		glAttachShader(program, shader);
		glShaderIDs.push_back(shader);
	}

	// -- Link --
	m_ID = program;
	glLinkProgram(program);

	// -- Check for Link Errors --
	GLint isLinked = 0;
	glGetProgramiv(program, GL_LINK_STATUS, (int*)&isLinked);
	if (isLinked == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

		std::vector<GLchar> infoLog(maxLength);
		glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);
		glDeleteProgram(program);

		for (auto id : glShaderIDs)
			glDeleteShader(id);

		ENGINE_LOG("Shader Linking Error: %s", infoLog.data());
		ASSERT(false, "Shader Program Link Failure!");
		return;
	}

	// -- Detach Shaders after successful link --
	for (auto id : glShaderIDs)
	{
		glDetachShader(program, id);
		glDeleteShader(id);
	}
}


const std::unordered_map<GLenum, std::string> Shader::PreProcessShader(const std::string& source)
{
	std::unordered_map<GLenum, std::string> ret;

	// -- Token #type for Shader Type --
	const char* typeToken = "#type";
	size_t typeTokenLength = strlen(typeToken);
	size_t pos = source.find(typeToken, 0);

	// -- Iterate all the string --
	while (pos != std::string::npos)
	{
		size_t eol = source.find_first_of("\r\n", pos);					// End of token line
		size_t begin = pos + typeTokenLength + 1;						// Start of Shader Type Name (after '#type' token)
		std::string shader_type = source.substr(begin, eol - begin);

		ASSERT(RendererUtils::ShaderTypeFromString(shader_type), "Invalid ShaderType specification or not supported");

		size_t next_line_pos = source.find_first_not_of("\r\n", eol);		// Start of shader code after token
		pos = source.find(typeToken, next_line_pos);						// Start of next shader token
		ret[RendererUtils::ShaderTypeFromString(shader_type)] = (pos == std::string::npos) ? source.substr(next_line_pos) : source.substr(next_line_pos, pos - next_line_pos);
	}

	return ret;
}


const std::string Shader::ReadShaderFile(const std::string& filepath)
{
	std::ifstream file(filepath, std::ios::in | std::ios::binary);
	std::string ret;

	if (file)
	{
		// -- File Size --
		file.seekg(0, std::ios::end);
		size_t file_size = file.tellg();
		
		if (file_size != -1)
		{
			// -- String as big as file size & Load file into string --			
			ret.resize(file_size);
			file.seekg(0, std::ios::beg); // Cursor at file beginning
			file.read(&ret[0], file_size);
			return ret;
		}
		else
			ENGINE_LOG("Couldn't read Shader file at path '%s'", filepath.c_str());
	}
	else
		ENGINE_LOG("Couldn't open Shader file at path '%s'", filepath.c_str());

	return ret;
}