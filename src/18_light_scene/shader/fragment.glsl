#version 330 core

out vec4 FracColor;
uniform vec3 lightColor;
uniform vec3 objectColor;

void main(){
    FracColor = vec4(objectColor * lightColor, 1.0f);
}
