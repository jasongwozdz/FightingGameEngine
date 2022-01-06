#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexPos;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexPos;

void main() {
    // mat4 model = {
    //     {1.0f, 0.0f, 0.0f, 0.0f},
    //     {0.0f, 1.0f, 0.0f, 0.0f},
    //     {0.0f, 0.0f, 1.0f, 0.0f},
    //     {0.0f, 0.0f, 0.0f, 1.0f}
    // };

    // mat4
    // // mat4 view = {
    // //     {-0.948683262f, -0.0953462571f, 0.301511347f, 0.0f},
    // //     {0.316227764f,-0.286038756f, 0.904534042f, 0.0f},
    // //     {0.0f, 0.953462601, 0.301511347f, 0.0f},
    // //     {0.0f, -0.0000000596046448f, -3.31662464f, 0.0f}
    // // };
    // mat4 proj = {
    //     {1.60947561f, 0.0f, 0.0f, 0.0f},
    //     {0.0f, -2.41421342f, 0.0f, 0.0f},
    //     {0.0f, 0.0f, -1.00200200f, -1.00000000f},
    //     {0.0f, 0.0f, -0.200200200f, 0.0f}
    // };

    // gl_Position = proj * view * model * vec4(inPosition, 1.0);
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
    fragColor = inColor;
    fragTexPos = inTexPos;
}