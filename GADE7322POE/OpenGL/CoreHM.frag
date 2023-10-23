#version 330 core

in vec2 TexCoord;
in float Height;

out vec4 color;

uniform sampler2D ourTexture1;

void main()
{
    color = texture(ourTexture1, TexCoord);
};
