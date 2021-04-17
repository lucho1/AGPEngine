#type VERTEX_SHADER
#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;
layout(location = 2) in vec3 a_Normal;
layout(location = 3) in vec3 a_Tangent;
layout(location = 4) in vec3 a_Bitangent;

uniform mat4 u_ViewProjection = mat4(1.0);
uniform mat4 u_Model = mat4(1.0);

out vec2 v_TexCoord;
out vec3 v_Normal;
out vec3 v_Tangent;
out vec3 v_Bitangent;

void main()
{
	v_TexCoord = a_TexCoord;
	v_Normal = a_Normal;
	v_Tangent = a_Tangent;
	v_Bitangent = a_Bitangent;
	gl_Position = u_ViewProjection * u_Model * vec4(a_Position, 1.0);
}



#type FRAGMENT_SHADER
#version 460 core

layout(location = 0) out vec4 color;

in vec2 v_TexCoord;
in vec3 v_Normal;
in vec3 v_Tangent;
in vec3 v_Bitangent;

uniform sampler2D u_Texture;
uniform vec4 u_Color = vec4(1.0);

void main()
{
	color = texture(u_Texture, v_TexCoord) * u_Color;
	//color = vec4(v_Normal, 1.0);
}