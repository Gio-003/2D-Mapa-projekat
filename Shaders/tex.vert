#version 330 core

layout(location = 0) in vec2 aPos;   
layout(location = 1) in vec2 aUV;

out vec2 vUV;

uniform vec2 uPos;    // NDC centar pozicije
uniform vec2 uSize;   // Sirina i visina u NDC
// Dodato za Sprite Sheet (seckanje teksture)
uniform vec2 uUVScale;  // Koliki deo teksture prikazujemo (npr 0.1 za jednu cifru)
uniform vec2 uUVOffset; // Odakle pocinjemo (npr 0.3 za broj 3)

void main()
{
    vec2 scaled = aPos * uSize;
    vec2 finalPos = scaled + uPos;

    gl_Position = vec4(finalPos, 0.0, 1.0);
    
    // Transformacija UV koordinata za isecanje
    vUV = aUV * uUVScale + uUVOffset;
}