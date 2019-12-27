#version 410 core

in vec3 textureCoordinates;
out vec4 color;

uniform samplerCube skybox;
uniform float lightColor2;

void main()
{
	vec4 fogColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);
    color =  lightColor2*texture(skybox, textureCoordinates) + 0.3*fogColor;
}
