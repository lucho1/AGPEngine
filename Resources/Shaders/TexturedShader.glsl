#type VERTEX_SHADER
#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;
layout(location = 2) in vec3 a_Normal;
layout(location = 3) in vec3 a_Tangent;
layout(location = 4) in vec3 a_Bitangent;

uniform mat4 u_ViewProjection = mat4(1.0);
uniform mat4 u_Model = mat4(1.0);

layout(std140, binding = 0) uniform ub_CameraData
{
	mat4 ViewProjection;
	vec3 Position;
};

out IBlock
{
	vec2 TexCoord;
	vec3 Normal;
	vec3 Tangent;
	vec3 Bitangent;
} v_VertexData;



void main()
{
	v_VertexData.TexCoord = a_TexCoord;
	v_VertexData.Normal = a_Normal;
	v_VertexData.Tangent = a_Tangent;
	v_VertexData.Bitangent = a_Bitangent;
	gl_Position = ViewProjection * u_Model * vec4(a_Position, 1.0);
}



#type FRAGMENT_SHADER
#version 460 core

layout(location = 0) out vec4 color;

in IBlock
{
	vec2 TexCoord;
	vec3 Normal;
	vec3 Tangent;
	vec3 Bitangent;
} v_VertexData;

uniform sampler2D u_Texture;
uniform vec4 u_Color = vec4(1.0);

void main()
{
	color = texture(u_Texture, v_VertexData.TexCoord) * u_Color;
	//color = vec4(v_VertexData.Normal, 1.0);
}