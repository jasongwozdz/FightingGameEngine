#version 330 core

uniform sampler2D texSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
    //outColor = vec4(vec3(255.0, 0, 0), 1.0);
    outColor = texture(texSampler, fragTexCoord);
}