#version 330 core

layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(fragColor.x, fragColor.y, fragColor.z, 1.0);
}