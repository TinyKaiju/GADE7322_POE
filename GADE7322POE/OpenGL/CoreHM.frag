#version 330 core

in vec2 TexCoord;
in float Height;

out vec4 color;

uniform sampler2D ourHM_texture;

void main()
{
    color = texture(ourHM_texture, TexCoord);
};
