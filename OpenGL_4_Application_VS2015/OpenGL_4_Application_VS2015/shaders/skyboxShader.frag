#version 410 core

in vec3 textureCoordinates;
out vec4 color;

uniform samplerCube skybox;

void main()
{
    color = 0.5*texture(skybox, textureCoordinates);
}
