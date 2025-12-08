#version 330 core

layout(location = 0) in vec2 aPos;  
layout(location = 1) in vec2 inTex;

out vec2 vUV;

uniform vec2 uCamCenter;
uniform float uZoom;

void main()
{
    // zoom area around camera
    //fromula UV' = (UV - P) * Zoom + P
    vec2 zoomedUV = (inTex - uCamCenter) * uZoom + uCamCenter;
    vUV = zoomedUV;
    gl_Position = vec4(aPos, 0.0, 1.0);
}
