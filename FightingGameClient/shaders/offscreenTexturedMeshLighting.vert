#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(binding = 1) uniform GlobalOffscreenData {
    mat4 pointLightSpaceMatrix;
    mat4 projection;
    vec3 lightPos;
} globalOffscreenData;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexPos;
layout(location = 3) in vec3 inNormal;

layout(location = 0) out vec4 fragPos;

void main() {
    gl_Position = globalOffscreenData.projection * globalOffscreenData.pointLightSpaceMatrix * ubo.model * vec4(inPosition, 1.0);
    fragPos = ubo.model * vec4(inPosition, 1.0);
}