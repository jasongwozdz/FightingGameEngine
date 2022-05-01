#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D texSampler;

layout (binding=2) uniform DirLight  {
    vec3 viewPos;
	vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
} dirLight;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragPos;

layout(location = 0) out vec4 outColor;

vec3 CalcDirLight(vec3 normal, vec3 viewDir);

// calculates the color when using a directional light.
vec3 CalcDirLight(vec3 normal, vec3 viewDir){
    vec3 lightDir = normalize(-dirLight.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 1.0f;
    //pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // combine results
    vec3 ambient = dirLight.ambient * vec3(texture(texSampler, fragTexCoord));
    vec3 diffuse = dirLight.diffuse * diff * vec3(texture(texSampler, fragTexCoord));
    return ((ambient + diffuse));
}

void main() {
    //outColor = vec4(vec3(255.0, 0, 0), 1.0);
    vec3 norm = normalize(fragNormal);
    vec3 viewDir = normalize(dirLight.viewPos - fragPos);
    vec3 result = CalcDirLight(norm, viewDir);
    outColor = vec4(result, 1.0);
}