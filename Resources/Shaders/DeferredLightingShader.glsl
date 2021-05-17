#type VERTEX_SHADER
#version 460 core


// --- Vertex Attributes ---
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

// --- Output Values ---
out vec2 TexCoord;
out vec3 CamPos;

// --- Camera UBO ---
layout(std140, binding = 0) uniform ub_CameraData
{
	mat4 ViewProjection;
	vec3 CamPosition;
};

// --- Uniforms ---
uniform mat4 u_Model = mat4(1.0);

// --- MAIN ---
void main()
{
	TexCoord = a_TexCoord;
	CamPos = CamPosition;
	
	gl_Position = u_Model * vec4(a_Position, 1.0);
}


// ------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------
#type FRAGMENT_SHADER
#version 460 core

layout(location = 0) out vec4 color;

uniform sampler2D u_gColor;
uniform sampler2D u_gNormal;
uniform sampler2D u_gPosition;


// --- Output Values ---
in vec2 TexCoord;
in vec3 CamPos;


// --- Light Structs ---
struct DirectionalLight
{
	vec3 Color, Direction;
	float Intensity;	
};

struct PointLight // Pos & Color are vec4 to remember that they are aligned!
{
	vec4 Pos, Color;					// Pos & Color are vec4 due to alignment (to keep it in mind!)
	float Intensity, AttK, AttL, AttQ;	// Attenuation: K constant, L linear, Q quadratic
};


// --- Lights Uniforms ---
uniform DirectionalLight u_DirLight = DirectionalLight(vec3(1.0), vec3(1.0), 1.0);

layout(std430, binding = 0) buffer ssb_Lights // PLights SSBO
{
	int CurrentLights;
	PointLight PLightsVec[];
};


// ------------------------------------------ LIGHT CALCULATION ------------------------------------------
//vec4 CalculateDirectionalLight(vec3 normal, vec3 view)
//{
//	// Direction & Distance
//	vec3 dir = normalize(u_DirLight.Direction);
//	vec3 halfway_dir = normalize(dir + view);
//
//	// Diffuse & Specular
//	float diff_impact = max(dot(normal, dir), 0.0);
//	float spec_impact = pow(max(dot(normal, halfway_dir), 0.0), u_Material.Smoothness * 256.0);
//
//	// Final Impact
//	vec3 light_impact = u_DirLight.Color * u_DirLight.Intensity * (diff_impact + spec_impact);
//	return vec4(light_impact, 1.0);
//}
//
//
//vec4 CalculateLighting(PointLight light, vec3 normal, vec3 view)
//{
//	// Direction & Distance
//	vec3 pos_to_frag = light.Pos.xyz - v_VertexData.FragPos;
//	float dist = length(pos_to_frag);
//	vec3 dir = normalize(pos_to_frag);
//	vec3 halfway_dir = normalize(dir + view);
//
//	// Diffuse & Specular
//	float diff_impact = max(dot(normal, dir), 0.0);
//	float spec_impact = pow(max(dot(normal, halfway_dir), 0.0), u_Material.Smoothness * 256.0); //MATERIAL SHININESS!
//
//	// Final Impact
//	float light_att = 1.0/(light.AttK + light.AttL * dist + light.AttQ * dist * dist);
//	vec4 light_impact = light.Color * light.Intensity * light_att * (diff_impact + spec_impact);
//	
//	return vec4(light_impact.rgb, 1.0);
//}


// ------------------------------------------------ MAIN -------------------------------------------------
void main()
{
	vec3 color_vec = texture(u_gColor, TexCoord).rgb;
	vec3 normal_vec = texture(u_gNormal, TexCoord).rgb;
	vec3 frag_pos = texture(u_gPosition, TexCoord).rgb;
	//float specular = texture(u_gPosition, TexCoord).a;




	//vec3 normal_vec = normalize(v_VertexData.Normal);
	vec3 view_dir = normalize(CamPos - frag_pos);
//
	//vec3 normal_vec = texture(u_Normal, v_VertexData.TexCoord).rgb;
	//normal_vec = normal_vec * 2.0 - 1.0;
	//normal_vec = normalize(v_VertexData.TBN * normal_vec);

	//vec4 light_impact = CalculateDirectionalLight(normal_vec, view_dir);
	vec3 light_impact = vec3(0.0);
	for(int i = 0; i < CurrentLights; ++i)
	{
		vec3 light_dir = normalize(PLightsVec[i].Pos.xyz - frag_pos);
		vec3 diffuse = max(dot(normal_vec, light_dir), 0.0) * color_vec * PLightsVec[i].Color.rgb;
		light_impact += diffuse;

		//light_impact += CalculateLighting(PLightsVec[i], normal_vec, view_dir);
	}

	color = vec4(light_impact, 1.0);
	//color = texture(u_Albedo, v_VertexData.TexCoord) * u_Material.AlbedoColor + light_impact;
	//color = vec4(normal_vec, 1.0);
}