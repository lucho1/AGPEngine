#type VERTEX_SHADER
#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;
layout(location = 2) in vec3 a_Normal;
layout(location = 3) in vec3 a_Tangent;
layout(location = 4) in vec3 a_Bitangent;

// --- Interface Block ---
out IBlock
{
	vec2 TexCoord;
	vec3 Normal;
	vec3 FragPos;
	vec3 CamPos;
} v_VertexData;


// --- Camera UBO ---
layout(std140, binding = 0) uniform ub_CameraData
{
	mat4 ViewProjection;
	vec3 CamPosition;
};


// --- Uniforms ---
uniform mat4 u_ViewProjection = mat4(1.0);
uniform mat4 u_Model = mat4(1.0);

// --- MAIN ---
void main()
{
	v_VertexData.TexCoord = a_TexCoord;
	v_VertexData.CamPos = CamPosition;
	v_VertexData.Normal = mat3(transpose(inverse(u_Model))) * a_Normal;
	v_VertexData.FragPos = vec3(u_Model * vec4(a_Position, 1.0));
	
	gl_Position = ViewProjection * u_Model * vec4(a_Position, 1.0);
}


// ------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------
#type FRAGMENT_SHADER
#version 460 core

layout(location = 0) out vec4 color;

// --- Interface Block ---
in IBlock
{
	vec2 TexCoord;
	vec3 Normal;
	vec3 FragPos;
	vec3 CamPos;
} v_VertexData;


// --- Light Struct ---
struct PointLight
{
	vec4 Pos, Color;
	float Intensity, AttK, AttL, AttQ; // Attenuation: K constant, L linear, Q quadratic
};


// --- Light SSBO ---
layout(std430, binding = 0) buffer ssb_Lights
{
	int CurrentLights;
	PointLight PLightsVec[];
};

// --- Uniforms ---
uniform sampler2D u_Texture;
uniform vec4 u_Color = vec4(1.0);

// ------------------------------------------ LIGHT CALCULATION ------------------------------------------
vec4 CalculateLighting(PointLight light, vec3 normal, vec3 view)
{
	// Direction & Distance
	vec3 pos_to_frag = light.Pos.xyz - v_VertexData.FragPos;
	float dist = length(pos_to_frag);
	vec3 dir = normalize(pos_to_frag);
	vec3 halfway_dir = normalize(dir + view);

	// Diffuse & Specular
	float diff_impact = max(dot(normal, dir), 0.0);
	float spec_impact = pow(max(dot(normal, halfway_dir), 0.0), 32.0); //MATERIAL SHININESS!

	// Final Impact
	float light_att = 1.0/(light.AttK + light.AttL * dist + light.AttQ * dist * dist);
	vec4 light_impact = light.Color * light.Intensity * light_att * (diff_impact + spec_impact);
	
	return vec4(light_impact.rgb, 1.0);
}


// ------------------------------------------------ MAIN -------------------------------------------------
void main()
{
	vec3 normal_vec = normalize(v_VertexData.Normal);
	vec3 view_dir = normalize(v_VertexData.CamPos - v_VertexData.FragPos);

	vec4 light_impact = vec4(0.0);
	for(int i = 0; i < CurrentLights; ++i)
	{
		light_impact += CalculateLighting(PLightsVec[i], normal_vec, view_dir);
	}

	color = texture(u_Texture, v_VertexData.TexCoord) * u_Color + light_impact;
	//color = vec4(v_VertexData.Normal, 1.0);
}