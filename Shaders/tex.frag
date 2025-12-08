#version 330 core
in vec2 vUV;
out vec4 FragColor;

uniform sampler2D uTexture;
uniform float uAlpha; 

void main() {
    vec4 texColor = texture(uTexture, vUV);
    if(texColor.a < 0.1) discard; // Izbacujemo potpuno providne piksele
    FragColor = vec4(texColor.rgb, texColor.a * uAlpha);
}