#version 330 core

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

in vec3 fragColor;

out vec4 outColor;

void main() {
    outColor = vec4(fragColor.x, fragColor.y, fragColor.z, 1.0);
}