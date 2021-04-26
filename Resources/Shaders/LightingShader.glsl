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
	mat3 TBN;
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

	vec3 T = normalize(vec3(u_Model * vec4(a_Tangent, 0.0)));
	vec3 N = normalize(vec3(u_Model * vec4(a_Normal, 0.0)));

	T = normalize(T - dot(T, N)*N); // Re-orthogonalize
	vec3 B = cross(N, T);
	
	
	//vec3 B = normalize(vec3(u_Model * vec4(a_Bitangent, 0.0)));

	mat3 TBN = mat3(T, B, N);
	v_VertexData.TBN = TBN;
	
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
	mat3 TBN;
} v_VertexData;


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


// --- Material Struct & Uniform ---
struct Material
{
	float Smoothness;
	vec4 AlbedoColor;
};

uniform Material u_Material = Material(1.0, vec4(1.0));
uniform sampler2D u_Albedo, u_Normal, u_Bump;



// ------------------------------------------ LIGHT CALCULATION ------------------------------------------
vec4 CalculateDirectionalLight(vec3 normal, vec3 view)
{
	// Direction & Distance
	vec3 dir = normalize(u_DirLight.Direction);
	vec3 halfway_dir = normalize(dir + view);

	// Diffuse & Specular
	float diff_impact = max(dot(normal, dir), 0.0);
	float spec_impact = pow(max(dot(normal, halfway_dir), 0.0), u_Material.Smoothness * 256.0);

	// Final Impact
	vec3 light_impact = u_DirLight.Color * u_DirLight.Intensity * (diff_impact + spec_impact);
	return vec4(light_impact, 1.0);
}


vec4 CalculateLighting(PointLight light, vec3 normal, vec3 view)
{
	// Direction & Distance
	vec3 pos_to_frag = light.Pos.xyz - v_VertexData.FragPos;
	float dist = length(pos_to_frag);
	vec3 dir = normalize(pos_to_frag);
	vec3 halfway_dir = normalize(dir + view);

	// Diffuse & Specular
	float diff_impact = max(dot(normal, dir), 0.0);
	float spec_impact = pow(max(dot(normal, halfway_dir), 0.0), u_Material.Smoothness * 256.0); //MATERIAL SHININESS!

	// Final Impact
	float light_att = 1.0/(light.AttK + light.AttL * dist + light.AttQ * dist * dist);
	vec4 light_impact = light.Color * light.Intensity * light_att * (diff_impact + spec_impact);
	
	return vec4(light_impact.rgb, 1.0);
}


// ------------------------------------------------ MAIN -------------------------------------------------
void main()
{
	//vec3 normal_vec = normalize(v_VertexData.Normal);
	vec3 view_dir = normalize(v_VertexData.CamPos - v_VertexData.FragPos);

	vec3 normal_vec = texture(u_Normal, v_VertexData.TexCoord).rgb;
	normal_vec = normal_vec * 2.0 - 1.0;
	normal_vec = normalize(v_VertexData.TBN * normal_vec);

	vec4 light_impact = CalculateDirectionalLight(normal_vec, view_dir);
	//vec4 light_impact = vec4(0.0);
	for(int i = 0; i < CurrentLights; ++i)
	{
		light_impact += CalculateLighting(PLightsVec[i], normal_vec, view_dir);
	}

	color = texture(u_Albedo, v_VertexData.TexCoord) * u_Material.AlbedoColor + light_impact;
	//color = vec4(normal_vec, 1.0);
}