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
	vec3 Tg_CamPos;
	vec3 Tg_FragPos;
} v_VertexData;

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
	
	v_VertexData.Tg_CamPos = TBN * CamPosition;
	v_VertexData.Tg_FragPos = TBN * v_VertexData.FragPos;

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
layout(location = 3) out vec4 gBuff_Smoothness;
layout(location = 4) out vec4 gBuff_Depth;

// --- Interface Block ---
in IBlock
{
	vec2 TexCoord;
	vec3 FragPos;
	vec3 Normal;
	vec3 CamPos;
	mat3 TBN;
	vec3 Tg_CamPos;
	vec3 Tg_FragPos;
} v_VertexData;

// --- Uniforms ---
struct Material
{
	float Smoothness, Bumpiness, Heighscale, ParallaxLayers;
	vec4 AlbedoColor;
};

uniform Material u_Material = Material(1.0, 1.0, 0.1, 32.0, vec4(1.0));
uniform sampler2D u_Albedo, u_Normal, u_Bump;



// ------------------------------------------ RELIEF MAP CALCULATION -------------------------------------
vec2 CalculateParallaxMapping(vec2 tcoords, vec3 view)
{
	// 8 & 32 values are like the max & min depth layers for parallax, change it as you see fit
	float layers_num = mix(u_Material.ParallaxLayers, 8.0, max(dot(vec3(0.0, 0.0, 1.0), view), 0.0));

    // Layers Depth & TCoords Shift
    float layer_depth = 1.0 / layers_num;
    float current_layer_depth = 0.0;
    vec2 P = view.xy * u_Material.Heighscale; // TODO: HEIGHT_SCALE!
    vec2 tcoords_shift = P / layers_num;

	// Perform Parallax Mapping
	vec2  current_tcoords = tcoords;
	float current_depth = texture(u_Bump, current_tcoords).r;

	while(current_layer_depth < current_depth)
	{
	    // Move coordinates along P
	    current_tcoords -= tcoords_shift;
	    current_depth = texture(u_Bump, current_tcoords).r;
	    current_layer_depth += layer_depth;  
	}

	// TCoords & Depth Before/After Collision (to interpolate)
	vec2 prev_tcoords = current_tcoords + tcoords_shift;
	float after_depth  = current_depth - current_layer_depth;
	float before_depth = texture(u_Bump, prev_tcoords).r - current_layer_depth + layer_depth;
	
	// Interpolate TCoords
	float weight = after_depth / (after_depth - before_depth);
	vec2 ret = prev_tcoords * weight + current_tcoords * (1.0 - weight);
	return ret;
}

// --- MAIN ---
void main()
{
	vec3 view_dir = normalize(v_VertexData.Tg_CamPos - v_VertexData.Tg_FragPos);
	vec2 tex_coords = CalculateParallaxMapping(v_VertexData.TexCoord, view_dir);

	vec3 normal_vec = texture(u_Normal, tex_coords).rgb;
	normal_vec = normal_vec * 2.0 - 1.0;
	normal_vec.z *= u_Material.Bumpiness;
	normal_vec = normalize(v_VertexData.TBN * normal_vec);

	gBuff_Color = texture(u_Albedo, tex_coords) * u_Material.AlbedoColor;
	gBuff_Normal = vec4(normal_vec, 1.0);
	gBuff_Position = vec4(v_VertexData.FragPos, 1.0);
	gBuff_Smoothness = vec4(vec3(u_Material.Smoothness), 1.0);
	gBuff_Depth = vec4(vec3(gl_FragCoord.z), 1.0);
}