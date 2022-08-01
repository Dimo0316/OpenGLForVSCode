#version 330 core


in vec3 outNormal;
in vec3 outFragPos;

out vec4 FragColor;

uniform vec3 lightPos; 
// uniform vec3 objectColor;
uniform vec3 viewPos;


struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shiniess;
};

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Light light;
uniform Material material;
// float specularStrength = 0.5;

void main(){
    // 全局光照，环境光
    vec3 ambient = light.ambient *  material.ambient;

    // 漫反射
    vec3 norm = normalize(outNormal); // 模型 法向量单位化
    vec3 lightDir = normalize(lightPos - outFragPos); // 片元位置指向光照方向

    float diff = max(dot(norm, lightDir), 0.0); // 漫反射分量
    vec3 diffuse = light.diffuse * (diff * material.diffuse); // 漫反射光 

    // 镜面反射
    vec3 viewDir = normalize(viewPos - outFragPos); // 计算视觉方向
    vec3 reflectDir = reflect(-lightDir, norm); // 根据法向量和光照向量计算反射后光线向量

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shiniess);
    vec3 specular = light.specular * (spec * material.specular);

    vec3 result = (ambient + diffuse + specular) ; 
    FragColor = vec4(result, 1.0);  
}
