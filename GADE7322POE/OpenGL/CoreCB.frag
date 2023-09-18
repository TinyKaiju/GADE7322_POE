#version 330 core

in vec2 TexCoord;

out vec4 color;

uniform sampler2D faceTexture;

void main()
{
    color = texture(faceTexture, TexCoord);
};