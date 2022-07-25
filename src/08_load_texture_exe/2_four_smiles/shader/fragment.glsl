#version 330 core

out vec4 FracColor;

in vec3 ourColor;
in vec2 TexCoord;

uniform sampler2D texture1;
uniform sampler2D texture2;
uniform float factor;

void main(){
    FracColor = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), abs(sin(factor)));
    // FracColor = vec4(ourColor, 1.0f);
}