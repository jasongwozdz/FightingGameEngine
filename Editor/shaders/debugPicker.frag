#version 450 core

#define DEPTH_ARRAY_SCALE 32

layout(location = 0) out vec4 fColor;
layout(location = 0) in struct { vec3 Color; } In;
layout(binding = 0) buffer writeonly MouseHoverData
{
    uint data[DEPTH_ARRAY_SCALE];
} mouseHoverData;

 layout(push_constant) uniform PushConstantFrag
 {
     layout(offset = 64)uint uniqueId;
     vec2 mousePos;
 } pc;

void main()
{
    uint index = uint(gl_FragCoord.z * DEPTH_ARRAY_SCALE);

    if(length(pc.mousePos - gl_FragCoord.xy) < 1)
    {
        mouseHoverData.data[index] = pc.uniqueId;
    }

    fColor = vec4(In.Color, 0.5f);
}