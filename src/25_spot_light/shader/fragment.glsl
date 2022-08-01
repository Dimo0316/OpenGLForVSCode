#version 330 core


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

struct Light {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

uniform Light light;
uniform Material material;

void main(){

    vec3 lightDir = normalize(light.position - outFragPos);
    float theta = dot(lightDir, normalize(-light.direction));
    
    float epsilon = light.cutOff - light.outerCutOff;
    // clamp 约束 数值在[0.0, 1.0]
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    float distanceLight = length(light.position - outFragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distanceLight + 
                        light.quadratic * pow(distanceLight, 2));

    vec4 objectColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
    vec3 diffuseMap = vec3(texture(material.diffuse, outTexCoord));
    vec3 specularMap = vec3(texture(material.specular, outTexCoord));
    // 全局光照，环境光
    vec3 ambient = light.ambient * diffuseMap;

    // 漫反射
    vec3 norm = normalize(outNormal); // 模型 法向量单位化
    float diff = max(dot(norm, lightDir), 0.0); // 漫反射分量
    vec3 diffuse = light.diffuse * diff * diffuseMap; // 漫反射光 

    // 镜面反射
    vec3 viewDir = normalize(viewPos - outFragPos); // 计算视觉方向
    vec3 reflectDir = reflect(-lightDir, norm); // 根据法向量和光照向量计算反射后光线向量

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * specularMap;

    // 边缘软化
    diffuse *= intensity;
    specular *= intensity;

    // 将环境光、漫反射、镜面光分别乘以衰减距离
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    vec3 result = (ambient + diffuse + specular) * vec3(objectColor); 
    FragColor = vec4(result, 1.0);  
}
