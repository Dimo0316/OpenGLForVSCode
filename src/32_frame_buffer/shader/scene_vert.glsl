#version 330 core
layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec2 TexCoords;

out vec2 outTexCoord;
out vec3 outNormal;
out vec3 outFragPos; // 世界坐标系 片元（顶点）位置

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float uvScale;

void main(){
    gl_Position = projection * view * model *  vec4(Position, 1.0f);
    // gl_PointSize = 10.0f;

    outFragPos = vec3(model * vec4(Position, 1.0));
    outTexCoord = TexCoords * uvScale;

    // 解决不等比缩放，对法向量产生的影响
    outNormal = mat3(transpose(inverse(model))) * Normal;
    
}