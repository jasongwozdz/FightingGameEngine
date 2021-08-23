#version 450 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;
layout(push_constant) uniform uPushConstant { mat4 mvp; } pc;
out gl_PerVertex { vec4 gl_Position; };
layout(location = 0) out struct { vec4 Color; } Out;
void main()
{
    Out.Color = vec4(aColor, 1.0);
    gl_Position = pc.mvp * vec4(aPos, 1.0);
}
