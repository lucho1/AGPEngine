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
layout(location = 1) out vec4 brightness;


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
uniform sampler2D u_gColor;
uniform sampler2D u_gNormal;
uniform sampler2D u_gPosition;
uniform sampler2D u_gSmoothness;

uniform DirectionalLight u_DirLight = DirectionalLight(vec3(1.0), vec3(1.0), 1.0);

layout(std430, binding = 0) buffer ssb_Lights // PLights SSBO
{
	int CurrentLights;
	PointLight PLightsVec[];
};


// ------------------------------------------ LIGHT CALCULATION ------------------------------------------
vec3 CalculateDirectionalLight(vec3 normal, vec3 view, float mat_smoothness)
{
	// Direction & Distance
	vec3 dir = normalize(u_DirLight.Direction);
	vec3 halfway_dir = normalize(dir + view);

	// Diffuse & Specular
	float diff_impact = max(dot(normal, dir), 0.0);
	float spec_impact = pow(max(dot(normal, halfway_dir), 0.0), mat_smoothness * 256.0);

	// Final Impact
	vec3 light_impact = u_DirLight.Color.rgb * u_DirLight.Intensity * (diff_impact + spec_impact);
	return light_impact;
}


vec3 CalculateLighting(PointLight light, vec3 normal, vec3 view, vec3 frag_pos, float mat_smoothness)
{
	// Direction & Distance
	vec3 pos_to_frag = light.Pos.xyz - frag_pos;
	float dist = length(pos_to_frag);
	vec3 dir = normalize(pos_to_frag);
	vec3 halfway_dir = normalize(dir + view);

	// Diffuse & Specular
	float diff_impact = max(dot(normal, dir), 0.0);
	float spec_impact = pow(max(dot(normal, halfway_dir), 0.0), mat_smoothness * 256.0); //MATERIAL SHININESS!

	// Final Impact
	float light_att = 1.0/(light.AttK + light.AttL * dist + light.AttQ * dist * dist);
	vec3 light_impact = light.Color.rgb * light.Intensity * light_att * (diff_impact + spec_impact);
	
	return light_impact;
}


// ------------------------------------------------ MAIN -------------------------------------------------
void main()
{
	vec4 albedo_color = texture(u_gColor, TexCoord);
	if(albedo_color.a < 0.1)
	{
		color = albedo_color;
		return;
	}

	vec3 color_vec = albedo_color.rgb;
	vec3 normal_vec = texture(u_gNormal, TexCoord).rgb;
	vec3 frag_pos = texture(u_gPosition, TexCoord).rgb;
	float mat_smoothness = texture(u_gSmoothness, TexCoord).r;
	
	vec3 view_dir = normalize(CamPos - frag_pos);

	vec3 light_impact = CalculateDirectionalLight(normal_vec, view_dir, mat_smoothness);
	for(int i = 0; i < CurrentLights; ++i)
	{
		light_impact += CalculateLighting(PLightsVec[i], normal_vec, view_dir, frag_pos, mat_smoothness);
	}

	color = vec4(color_vec + light_impact, 1.0);

	float bright = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
	brightness = color * smoothstep(1.0, 1.1, bright);

	//if(bright > 1.0)
	//	brightness = color;
	//else
	//	brightness = vec4(0.0, 0.0, 0.0, 1.0);
}