#version 330 core

#define MAX_BONES 64

struct UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    mat4 bones[MAX_BONES];
};

uniform UniformBufferObject ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexPos;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in vec4 inBoneWeights;
layout(location = 5) in ivec4 inBoneIds;

out vec3 fragColor;
out vec2 fragTexCoord;
out vec3 fragNormal;
out vec3 fragPos;

void main() {
    mat4 boneTransforms = ubo.bones[inBoneIds[0]] * inBoneWeights[0];
    boneTransforms     += ubo.bones[inBoneIds[1]] * inBoneWeights[1];
    boneTransforms     += ubo.bones[inBoneIds[2]] * inBoneWeights[2];
    boneTransforms     += ubo.bones[inBoneIds[3]] * inBoneWeights[3];

    gl_Position = ubo.proj * ubo.view * ubo.model * boneTransforms * vec4(inPosition, 1.0);

    fragColor = inColor;
    fragTexCoord = inTexPos;
    fragPos = vec3(ubo.model * vec4(inPosition, 1.0));
    fragNormal = mat3(transpose(inverse(ubo.model))) * mat3(transpose(inverse(boneTransforms))) * inNormal;
}