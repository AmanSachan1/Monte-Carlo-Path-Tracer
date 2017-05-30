#version 150
// ^ Change this to version 130 if you have compatibility issues

uniform mat4 u_Model;
uniform mat4 u_ViewProj;

in vec3 vs_Pos;
in vec3 vs_Col;

out vec3 fs_Col;

void main()
{
    fs_Col = vs_Col;
    vec4 modelposition = u_Model * vec4(vs_Pos, 1);

    // built-in things to pass down the pipeline
    gl_Position = u_ViewProj * modelposition;

}
