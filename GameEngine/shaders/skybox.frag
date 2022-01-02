#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (binding = 1) uniform samplerCube samplerCubeMapArray;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 outUVW;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(samplerCubeMapArray, outUVW);
    //vec3 color = vec3(0, 255, 0);
    //outColor = vec4(color, 1.0);
}