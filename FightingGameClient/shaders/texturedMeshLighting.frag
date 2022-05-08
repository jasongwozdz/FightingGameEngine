#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D texSampler;

struct DirLight {
	vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct PointLight {
    float constant;
    float linear;
    float quadratic;
    vec3 position;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

layout (binding=2) uniform UniformData  {
    vec3 viewPos;
    DirLight dirLight;
    PointLight pointLight;
} uniformData;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragPos;

layout(location = 0) out vec4 outColor;

// calculates the color when using a directional light.
vec3 CalcDirLight(vec3 normal, vec3 viewDir){
    vec3 lightDir = normalize(-uniformData.dirLight.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 1.0f;
    //pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // combine results
    vec3 ambient = uniformData.dirLight.ambient * vec3(texture(texSampler, fragTexCoord));
    vec3 diffuse = uniformData.dirLight.diffuse * diff * vec3(texture(texSampler, fragTexCoord));
    return ((ambient + diffuse));
}

// calculates the color when using a point light.
vec3 CalcPointLight(int index, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(uniformData.pointLight.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0); //use max function because if the angle between the normal and lightdir > 90 degrees the dot product will be negative
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    // float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance    = length(uniformData.pointLight.position - fragPos);
    float attenuation = 1.0 / (uniformData.pointLight.constant + uniformData.pointLight.linear * distance + uniformData.pointLight.quadratic * (distance * distance));    
    // combine results
    vec3 ambient  = uniformData.pointLight.ambient  * vec3(texture(texSampler, fragTexCoord));
    vec3 diffuse  = uniformData.pointLight.diffuse  * diff * vec3(texture(texSampler, fragTexCoord));
    // vec3 specular = pointLight[index].specular * spec * vec3(texture(specularSampler, fragTexCoord));
    ambient  *= attenuation;
    diffuse  *= attenuation;
    // specular *= attenuation;
    return (ambient + diffuse);
} 

void main() {
    //outColor = vec4(vec3(255.0, 0, 0), 1.0);
    vec3 norm = normalize(fragNormal);
    vec3 viewDir = normalize(uniformData.viewPos - fragPos);
    vec3 result = CalcDirLight(norm, viewDir);
    result += CalcPointLight(0, norm, fragPos, viewDir);
    outColor = vec4(result, 1.0);
}