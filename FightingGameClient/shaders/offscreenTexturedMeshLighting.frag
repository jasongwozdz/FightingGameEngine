#version 450

layout(location = 0) in vec4 fragPos;

layout(location = 0) out vec4 outColor;

void main() {
//    float far_plane = 100.0f;
//    vec3 lightVec = fragPos.xyz - fragLightPos;
//    outColor = vec4(length(lightVec)/far_plane, 1.0f, 1.0f, 1.0f);
	outColor = vec4(1.0, 0.0, 0.0, 1.0);
}