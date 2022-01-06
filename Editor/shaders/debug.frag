#version 450 core
layout(location = 0) out vec4 fColor;
layout(location = 0) in struct { vec3 Color; } In;
void main()
{
    fColor = vec4(In.Color, 1.0);
}