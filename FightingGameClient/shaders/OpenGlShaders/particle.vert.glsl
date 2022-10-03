#version 330

struct UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
};

uniform UniformBufferObject ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexPos;
layout(location = 3) in vec3 inNormal;

out vec3 fragColor;
out vec2 fragTexCoord;
out vec3 fragNormal;
out vec3 fragPos;

void main() {

    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);

    fragColor = inColor;
    fragTexCoord = inTexPos;
    fragPos = vec3(ubo.model * vec4(inPosition, 1.0));
    fragNormal = mat3(transpose(inverse(ubo.model))) * mat3(transpose(inverse(boneTransforms))) * inNormal
}