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
	vec3 CamPosition;
};

out IBlock
{
	vec2 TexCoord;
	vec3 Normal;
	vec3 FragPos;
	vec3 CamPos;
} v_VertexData;



void main()
{
	v_VertexData.TexCoord = a_TexCoord;
	v_VertexData.CamPos = CamPosition;
	v_VertexData.Normal = mat3(transpose(inverse(u_Model))) * a_Normal;
	v_VertexData.FragPos = vec3(u_Model * vec4(a_Position, 1.0));
	
	gl_Position = ViewProjection * u_Model * vec4(a_Position, 1.0);
}



#type FRAGMENT_SHADER
#version 460 core

layout(location = 0) out vec4 color;

in IBlock
{
	vec2 TexCoord;
	vec3 Normal;
	vec3 FragPos;
	vec3 CamPos;
} v_VertexData;

uniform sampler2D u_Texture;
uniform vec4 u_Color = vec4(1.0);

struct PointLight
{
	vec4 Pos, Color;
	float Intensity, AttK, AttL, AttQ; // Attenuation: K constant, L linear, Q quadratic
};

uniform PointLight p_light;


void main()
{
	vec3 normal_vec = normalize(v_VertexData.Normal);
	vec3 view_dir = normalize(v_VertexData.CamPos - v_VertexData.FragPos);

	// - Point Light Calc -
	vec3 dir = normalize(p_light.Pos.xyz - v_VertexData.FragPos);
	vec3 halfway_dir = normalize(dir + view_dir);
	
	float dist = length(p_light.Pos.xyz - v_VertexData.FragPos);
	float light_att = 1.0/(p_light.AttK + p_light.AttL * dist + p_light.AttQ * dist * dist);

	float diff_impact = max(dot(normal_vec, dir), 0.0);
	float spec_impact = pow(max(dot(normal_vec, halfway_dir), 0.0), 32.0); //MATERIAL SHININESS!

	vec4 light_impact = p_light.Color * (diff_impact + spec_impact) * p_light.Intensity * light_att;
	light_impact.a = 1.0;

	color = texture(u_Texture, v_VertexData.TexCoord) * u_Color * light_impact;
	//color = vec4(v_VertexData.Normal, 1.0);
}