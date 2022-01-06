#version 450
#extension GL_ARB_separate_shader_objects : enable

//layout (binding = 1) uniform samplerCube samplerCubeMapArray;

layout(location = 0) in vec3 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
    //outColor = texture(samplerCubeMapArray, fragTexCoord);
    vec3 color = vec3(0, 0, 255);
    outColor = vec4(color, 1.0);
}