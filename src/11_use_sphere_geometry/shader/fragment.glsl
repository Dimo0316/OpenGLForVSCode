#version 330 core

out vec4 FracColor;

in vec2 outTexCoord;

uniform sampler2D texture1;
uniform sampler2D texture2;

void main(){
    // float xy = length(ourPos.xy);
    FracColor = mix(texture(texture1, outTexCoord), texture(texture2, outTexCoord), 0.2);
    // vec2 coord = gl_PointCoord - vec2(0.5);

    // float r = float(length(coord) > 0.5);
    // FracColor = vec4(0.0, 0.91, 0.9, (1 - r) * 0.1);
}
