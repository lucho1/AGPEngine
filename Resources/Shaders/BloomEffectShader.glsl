#type VERTEX_SHADER
#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;
out vec2 v_TexCoord;

uniform mat4 u_Model = mat4(1.0);

void main()
{
    v_TexCoord = a_TexCoord;
    gl_Position = u_Model * vec4(a_Position, 1.0);
}


// ------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------
#type FRAGMENT_SHADER
#version 460 core

layout(location = 0) out vec4 color;
in vec2 v_TexCoord;

uniform sampler2D u_SceneTexture;
uniform sampler2D u_BlurredTexture;
uniform float u_BloomExposure = 1.0, u_HDRGamma = 2.2;

void main()
{
    vec4 Scene_Color = texture(u_SceneTexture, v_TexCoord);
    if(Scene_Color.a < 0.1)
	{
		color = Scene_Color;
		return;
	}

    vec3 HDR_Color = Scene_Color.rgb;
    vec3 Bloom_Color = texture(u_BlurredTexture, v_TexCoord).rgb;
    HDR_Color += Bloom_Color;

    vec3 res = vec3(1.0) - exp(-HDR_Color * u_BloomExposure);
    res = pow(res, vec3(1.0/u_HDRGamma));

    color = vec4(res, 1.0);
}