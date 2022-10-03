#version 330 core

struct UniformBufferObject{
    mat4 model;
    mat4 view;
    mat4 proj;
};

uniform UniformBufferObject ubo;

in vec3 inPosition;
in vec3 inColor;
in vec2 inTexPos;
in vec3 inNormal;

out vec3 fragColor;
out vec3 fragNormal;
out vec3 fragPos;

void main() {    
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
    fragColor = inColor;
    fragNormal = inNormal;
    fragPos = vec3(ubo.model * vec4(inPosition, 1.0));
}