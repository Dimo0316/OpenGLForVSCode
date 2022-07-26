#version 330 core

out vec4 FracColor;

in vec2 outTexCoord;

uniform sampler2D texture1;
uniform sampler2D texture2;

void main(){
    FracColor = mix(texture(texture1, outTexCoord), texture(texture2, outTexCoord), 0.2);
}
