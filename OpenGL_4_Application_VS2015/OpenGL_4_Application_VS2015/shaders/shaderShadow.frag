#version 410 core
out vec4 fColor;
void main()
{
fColor = vec4(gl_FragCoord.z, gl_FragCoord.z, gl_FragCoord.z, 1.0f);
}