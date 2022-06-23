#version 450
#extension GL_ARB_separate_shader_objects : enable

#define MAX_BONES 64

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    mat4 bones[MAX_BONES];
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
layout(location = 4) in vec4 inBoneWeights;
layout(location = 5) in ivec4 inBoneIds;

layout(location = 0) out vec4 fragPos;
layout(location = 1) out vec3 fragLightPos;

void main() {
    mat4 boneTransforms = ubo.bones[inBoneIds[0]] * inBoneWeights[0];
    boneTransforms     += ubo.bones[inBoneIds[1]] * inBoneWeights[1];
    boneTransforms     += ubo.bones[inBoneIds[2]] * inBoneWeights[2];
    boneTransforms     += ubo.bones[inBoneIds[3]] * inBoneWeights[3];

    gl_Position = globalOffscreenData.projection * globalOffscreenData.pointLightSpaceMatrix * ubo.model * boneTransforms * vec4(inPosition, 1.0);

    fragPos = ubo.model * boneTransforms * vec4(inPosition, 1.0);
    fragLightPos = globalOffscreenData.lightPos;
}