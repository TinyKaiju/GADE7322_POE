#version 330 core

out vec4 color;

uniform vec3 objectcolor;
uniform vec3 lightcolor;

void main()
{
    color = texture(objectcolor * lightcolor, 1.0f);
};