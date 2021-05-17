#type VERTEX_SHADER
#version 460 core

// --- Vertex Attributes ---
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;
layout(location = 2) in vec3 a_Normal;
layout(location = 3) in vec3 a_Tangent;
layout(location = 4) in vec3 a_Bitangent;

// --- Interface Block ---
out IBlock
{
	vec2 TexCoord;
	vec3 FragPos;
	vec3 Normal;
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
	vec4 world_pos = u_Model * vec4(a_Position, 1.0);
	
	v_VertexData.TexCoord = a_TexCoord;
	v_VertexData.FragPos = world_pos.xyz;
	v_VertexData.Normal = transpose(inverse(mat3(u_Model))) * a_Normal;
	v_VertexData.CamPos = CamPosition;

	vec3 T = normalize(vec3(u_Model * vec4(a_Tangent, 0.0)));
	vec3 N = normalize(vec3(u_Model * vec4(a_Normal, 0.0)));

	T = normalize(T - dot(T, N)*N); // Re-orthogonalize
	vec3 B = cross(N, T);

	mat3 TBN = mat3(T, B, N);
	v_VertexData.TBN = TBN;

	gl_Position = ViewProjection * u_Model * vec4(a_Position, 1.0);
}



// ------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------
#type FRAGMENT_SHADER
#version 460 core

// --- Outputs ---
layout(location = 0) out vec4 gBuff_Color;
layout(location = 1) out vec4 gBuff_Normal;
layout(location = 2) out vec4 gBuff_Position;

// --- Interface Block ---
in IBlock
{
	vec2 TexCoord;
	vec3 FragPos;
	vec3 Normal;
	vec3 CamPos;
	mat3 TBN;
} v_VertexData;

// --- Uniforms ---
struct Material
{
	float Smoothness;
	vec4 AlbedoColor;
};

uniform Material u_Material = Material(1.0, vec4(1.0));
uniform sampler2D u_Albedo, u_Normal, u_Bump;

// --- MAIN ---
void main()
{
	//vec3 normal_vec = texture(u_Normal, v_VertexData.TexCoord).rgb;
	//normal_vec = normal_vec * 2.0 - 1.0;
	//normal_vec = normalize(v_VertexData.TBN * normal_vec);

	vec3 view_dir = normalize(v_VertexData.CamPos - v_VertexData.FragPos);

	vec3 normal_vec = texture(u_Normal, v_VertexData.TexCoord).rgb;
	normal_vec = normal_vec * 2.0 - 1.0;
	normal_vec = normalize(v_VertexData.TBN * normal_vec);



	gBuff_Color = texture(u_Albedo, v_VertexData.TexCoord) * u_Material.AlbedoColor;
	gBuff_Normal = vec4(normal_vec, 1.0);
	gBuff_Position = vec4(v_VertexData.FragPos, u_Material.Smoothness); // Store Material Smoothness in Pos.a
}