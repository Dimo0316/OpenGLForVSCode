#version 330 core

out vec4 FracColor;

in vec2 outTexCoord;

uniform sampler2D texture1;
uniform sampler2D texture2;

void main(){
    // float xy = length(ourPos.xy);
    // FracColor = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), 0.2);
    FracColor = texture(texture1, outTexCoord);
}