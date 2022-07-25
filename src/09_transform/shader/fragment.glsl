#version 330 core

out vec4 FracColor;

in vec3 ourColor;
in vec2 TexCoord;
in vec3 ourPos;

uniform sampler2D texture1;
uniform sampler2D texture2;

void main(){
    float xy = length(ourPos.xy);
    FracColor = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), 0.2);
    // FracColor = vec4(ourColor, 1.0f);
}