#version 330 core
#define NR_POINT_LIGHTS 4

in vec3 outNormal;
in vec3 outFragPos;
in vec2 outTexCoord;

out vec4 FragColor;

uniform vec3 lightPos; 
uniform vec3 viewPos;

struct Material {
    sampler2D diffuse; // 漫反射贴图
    sampler2D specular; // 镜面光贴图
    float shininess; // 高光指数
};

struct DirLight {
    vec3 direction;
    
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
};

uniform Material material;
uniform SpotLight spotLight;
uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{   
    vec3 norm = normalize(outNormal);
    vec3 viewDir = normalize(viewPos - outFragPos);
    
    // 第一阶段 定向光
    vec3 result = CalcDirLight(dirLight, norm, viewDir);
    // 第二阶段：点光源
    for(int i = 0; i < NR_POINT_LIGHTS; i++)
        result += CalcPointLight(pointLights[i], norm, outFragPos, viewDir);    
    // 第三阶段：聚光
    result += CalcSpotLight(spotLight, norm, outFragPos, viewDir);    

    FragColor = vec4(result, 1.0);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir){
    vec3 lightDir = normalize(-light.direction);
    // 漫反射着色
    float diff = max(dot(normal, lightDir), 0.0);
    // 镜面光着色
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // 合并结果
    vec3 ambient  = light.ambient  * vec3(texture(material.diffuse, outTexCoord));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.diffuse, outTexCoord));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, outTexCoord));
    return ambient + diffuse + specular;
}

// 点光源
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // 漫反射着色
    float diff = max(dot(normal, lightDir), 0.0);
    // 镜面光着色
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // 衰减
    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
                 light.quadratic * (distance * distance));    
    // 合并结果
    vec3 ambient  = light.ambient  * vec3(texture(material.diffuse, outTexCoord));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.diffuse, outTexCoord));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, outTexCoord));
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    return ambient + diffuse + specular;
}

// 聚光灯
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir){

    vec3 lightDir = normalize(light.position - fragPos);
    float theta = dot(lightDir, normalize(-light.direction));

    // 聚光灯衰减系数
    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
                 light.quadratic * (distance * distance)); 

    float epsilon = light.cutOff - light.outerCutOff;
    // clamp 约束 数值在[0.0, 1.0]
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    vec4 objectColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
    vec3 diffuseMap = vec3(texture(material.diffuse, outTexCoord));
    vec3 specularMap = vec3(texture(material.specular, outTexCoord));
    // 全局光照，环境光
    vec3 ambient = light.ambient * diffuseMap;

    // 漫反射
    float diff = max(dot(normal, lightDir), 0.0); // 漫反射分量
    vec3 diffuse = light.diffuse * diff * diffuseMap; // 漫反射光 

    // 镜面反射
    vec3 reflectDir = reflect(-lightDir, normal); // 根据法向量和光照向量计算反射后光线向量

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * specularMap;

    // 边缘软化
    ambient *= intensity * attenuation;
    diffuse *= intensity * attenuation;
    specular *= intensity * attenuation;

    return ambient + diffuse + specular;
}