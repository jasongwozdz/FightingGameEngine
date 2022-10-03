#version 330 core

uniform sampler2D texSampler;
uniform sampler2D dirLightShadowMap;

#define SHADOW_OPACITY 0.5

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

#define MAX_LIGHTS 64
struct GlobalData  {
    vec3 viewPos;
    DirLight dirLight;
    PointLight pointLight[MAX_LIGHTS];
    int numLights;
};

uniform GlobalData uniformData;

in vec3 fragColor;
in vec2 fragTexCoord;
in vec3 fragNormal;
in vec3 fragPos;
out vec4 outColor;

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
    vec3 lightDir = normalize(uniformData.pointLight[index].position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0); //use max function because if the angle between the normal and lightdir > 90 degrees the dot product will be negative
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    // float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance    = length(uniformData.pointLight[index].position - fragPos);
    float attenuation = 1.0 / (uniformData.pointLight[index].constant + uniformData.pointLight[index].linear * distance + uniformData.pointLight[index].quadratic * (distance * distance));    
    // combine results
    vec3 ambient  = uniformData.pointLight[index].ambient  * vec3(texture(texSampler, fragTexCoord));
    vec3 diffuse  = uniformData.pointLight[index].diffuse  * diff * vec3(texture(texSampler, fragTexCoord));
    // vec3 specular = pointLight[index].specular * spec * vec3(texture(specularSampler, fragTexCoord));
    ambient  *= attenuation;
    diffuse  *= attenuation;
    // specular *= attenuation;
    return (ambient + diffuse);
} 

void main() {
    outColor = vec4(vec3(texture(texSampler, fragTexCoord)), 1.0);
    vec3 norm = normalize(fragNormal);
    vec3 viewDir = normalize(uniformData.viewPos - fragPos);
    vec3 result = CalcDirLight(norm, viewDir);
    for(int i = 0; i < uniformData.numLights; i++)
    {
        result += CalcPointLight(i, norm, fragPos, viewDir);
    }
    outColor = vec4(result, 1.0);

//check shadows only if there is a point light
    if(uniformData.numLights > 0)
    {
        vec3 lightPos = uniformData.pointLight[0].position;
        vec3 lightVec = fragPos - lightPos;
        // float sampledDist = texture(pointLightShadowMap, lightVec).r;
        // float dist = length(lightVec);
        // float shadow = (dist <= sampledDist + 0.15) ? 1.0 : SHADOW_OPACITY;
        // outColor.rgb *= shadow;
    }
}