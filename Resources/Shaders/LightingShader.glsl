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
	vec3 Normal;
	vec3 FragPos;
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

	v_VertexData.Tg_CamPos = TBN * CamPosition;
	v_VertexData.Tg_FragPos = TBN * v_VertexData.FragPos;
	
	gl_Position = ViewProjection * u_Model * vec4(a_Position, 1.0);
}


// ------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------
#type FRAGMENT_SHADER
#version 460 core

layout(location = 0) out vec4 color;
layout(location = 1) out vec4 brightness;

// --- Interface Block ---
in IBlock
{
	vec2 TexCoord;
	vec3 Normal;
	vec3 FragPos;
	vec3 CamPos;
	mat3 TBN;
	vec3 Tg_CamPos;
	vec3 Tg_FragPos;
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
	float Smoothness, Bumpiness, Heighscale, ParallaxLayers;
	vec4 AlbedoColor;
};

uniform Material u_Material = Material(1.0, 1.0, 0.1, 32.0, vec4(1.0));
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
	//vec3 pos_to_frag = v_VertexData.TBN * light.Pos.xyz - v_VertexData.Tg_FragPos;
	vec3 pos_to_frag = light.Pos.xyz  - v_VertexData.FragPos;
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



// ------------------------------------------------ MAIN -------------------------------------------------
void main()
{
	//vec3 normal_vec = normalize(v_VertexData.Normal);
	vec3 view_dir = normalize(v_VertexData.Tg_CamPos - v_VertexData.Tg_FragPos);
	vec2 tex_coords = CalculateParallaxMapping(v_VertexData.TexCoord, view_dir);

	vec3 normal_vec = texture(u_Normal, tex_coords).rgb;
	normal_vec = normal_vec * 2.0 - 1.0;
	normal_vec.z *= u_Material.Bumpiness;
	normal_vec = normalize(v_VertexData.TBN * normal_vec);	

	vec4 light_impact = CalculateDirectionalLight(normal_vec, view_dir);
	//vec4 light_impact = vec4(0.0);
	for(int i = 0; i < CurrentLights; ++i)
	{
		light_impact += CalculateLighting(PLightsVec[i], normal_vec, view_dir);
	}

	color = texture(u_Albedo, tex_coords) * u_Material.AlbedoColor + light_impact;
	//color = vec4(normal_vec, 1.0);

	float bright = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
	if(bright > 1.0)
		brightness = color;
	else
		brightness = vec4(0.0, 0.0, 0.0, 1.0);
}