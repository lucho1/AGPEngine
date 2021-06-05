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

uniform sampler2D u_Texture;
uniform bool u_HorizontalPass;

float Weights[5] = float[] (0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162);

void main()
{
    vec2 offset = 1.0 / textureSize(u_Texture, 0);
    vec3 res = texture(u_Texture, v_TexCoord).rgb * Weights[0];

    if(u_HorizontalPass)
    {
        for(int i = 1; i < 5; ++i)
        {
            res += texture(u_Texture, v_TexCoord + vec2(offset.x * i, 0.0)).rgb * Weights[i];
            res += texture(u_Texture, v_TexCoord - vec2(offset.x * i, 0.0)).rgb * Weights[i];
        }
    }
    else
    {
        for(int i = 1; i < 5; ++i)
         {
             res += texture(u_Texture, v_TexCoord + vec2(0.0, offset.y * i)).rgb * Weights[i];
             res += texture(u_Texture, v_TexCoord - vec2(0.0, offset.y * i)).rgb * Weights[i];
         }
    }

    color = vec4(res, 1.0);
}