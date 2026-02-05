#version 330 core
layout (location = 0) in vec2 inPos;
layout (location = 1) in vec2 inUV;

out vec2 chUV;

uniform mat4 uP;

void main()
{
    chUV = inUV;
    gl_Position = uP * vec4(inPos, 0.0, 1.0);
}
