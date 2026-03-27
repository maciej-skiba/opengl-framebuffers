#version 330 core

#define NUM_OF_DIR_LIGHTS 1
#define NUM_OF_POINT_LIGHTS 2
#define NUM_OF_SPOT_LIGHTS 1

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct DirLight {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular; 
    
    bool on;
};

in vec3 normal;
in vec3 fragPos;
in vec2 texCoord;

out vec4 FragColor;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform vec3 cameraPos;

uniform DirLight dirLight[NUM_OF_DIR_LIGHTS];
uniform PointLight pointLight[NUM_OF_POINT_LIGHTS];
uniform SpotLight spotLight[NUM_OF_SPOT_LIGHTS];

float shininess = 32.0f;

vec3 CalcDirLight(DirLight dirLight, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight pointLight, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight spotLight, vec3 normal, vec3 fragPos, vec3 viewDir);
float GetAttenuation(float constant, float linear, float quadratic, float distance);

void main()
{    
    vec3 norm = normalize(normal);
    vec3 cameraDir = normalize(cameraPos - fragPos);
    vec3 result = vec3(0);

    for (int dirLightIndex = 0; dirLightIndex < NUM_OF_DIR_LIGHTS; dirLightIndex++)
    {
        result += CalcDirLight(dirLight[dirLightIndex], norm, cameraDir);
    }

    FragColor = vec4(result, 1.0f);
}

vec3 CalcDirLight(DirLight dirLight, vec3 normal, vec3 viewDir)
{
    vec3 ambient = dirLight.ambient * vec3(texture(texture_diffuse1, texCoord));

    vec3 lightDirection = normalize(dirLight.position - fragPos);
    float diffuseStrength = max(dot(normal, lightDirection), 0.0f);
    vec3 diffuse = dirLight.diffuse * diffuseStrength * vec3(texture(texture_diffuse1, texCoord));

    vec3 reflectedLight = reflect(-lightDirection, normal);
    float spec = pow(max(dot(reflectedLight, viewDir), 0.0f), shininess);
    vec3 specular = dirLight.specular * vec3(texture(texture_specular1, texCoord)) * spec;

    return ambient + diffuse + specular;
}

vec3 CalcPointLight(PointLight pointLight, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    float distance = length(fragPos - pointLight.position);
    float attenuation = GetAttenuation(pointLight.constant, pointLight.linear, pointLight.quadratic, distance);
    vec3 ambient = pointLight.ambient * vec3(texture(texture_diffuse1, texCoord));

    vec3 lightDirection = normalize(pointLight.position - fragPos);
    float diffuseStrength = max(dot(normal, lightDirection), 0.0f);
    vec3 diffuse = pointLight.diffuse * diffuseStrength * vec3(texture(texture_diffuse1, texCoord));

    vec3 reflectedLight = reflect(-lightDirection, normal);
    float spec = pow(max(dot(reflectedLight, viewDir), 0.0f), shininess);
    vec3 specular = pointLight.specular * vec3(texture(texture_specular1, texCoord)) * spec;

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return ambient + diffuse + specular;
}

vec3 CalcSpotLight(SpotLight spotLight, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    if (!spotLight.on) return vec3(0.0f);
    
    vec3 lightToFragDir = normalize(fragPos - spotLight.position);
    float thetaDotProduct = dot(lightToFragDir, normalize(spotLight.direction));

    if (thetaDotProduct < spotLight.outerCutOff)
        return vec3(0.0f);

    float epsilon = spotLight.cutOff - spotLight.outerCutOff;
    float intensity = clamp((thetaDotProduct - spotLight.outerCutOff) / epsilon, 0.0, 1.0);

    float distance = length(fragPos - spotLight.position);
    float attenuation = GetAttenuation(spotLight.constant, spotLight.linear, spotLight.quadratic, distance);

    vec3 ambient = spotLight.ambient * vec3(texture(texture_diffuse1, texCoord));
    float diffuseStrength = max(dot(normal, -lightToFragDir), 0.0f);
    vec3 diffuse = spotLight.diffuse * diffuseStrength * vec3(texture(texture_diffuse1, texCoord));
    vec3 reflectedLight = reflect(lightToFragDir, normal);
    float spec = pow(max(dot(reflectedLight, viewDir), 0.0f), shininess);
    vec3 specular = spotLight.specular * vec3(texture(texture_specular1, texCoord)) * spec;

    ambient *= attenuation;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;

    return ambient + diffuse + specular;
}

float GetAttenuation(float constant, float linear, float quadratic, float distance)
{
    return 1.0 / (constant + linear * distance + quadratic * (distance * distance));
}