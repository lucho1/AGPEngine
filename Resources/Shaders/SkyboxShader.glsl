#type VERTEX_SHADER
#version 460 core

layout(location = 0) in vec3 a_Position;
out vec3 v_TexCoord;

uniform mat4 u_View = mat4(1.0);
uniform mat4 u_Proj = mat4(1.0);
uniform mat4 u_Model = mat4(1.0);

void main()
{
    v_TexCoord = a_Position;
    vec4 pos = u_Proj * u_View * u_Model * vec4(a_Position, 1.0);
    gl_Position = pos.xyww;
}


// ------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------
#type FRAGMENT_SHADER
#version 460 core

out vec4 color;
in vec3 v_TexCoord;
uniform samplerCube u_SkyboxTexture;
uniform vec3 u_TintColor = vec3(1.0);

void main()
{
    color = texture(u_SkyboxTexture, v_TexCoord) * vec4(u_TintColor, 1.0);
}