#pragma once
#include "raylib.h"

Shader LoadCelShader(void);
Shader LoadOutlineShader(void);

void SetShaderShadowMap(
    Shader shader,
    Matrix lightVP,
    unsigned int shadowMapTexId,
    int shadowMapResolution,
    bool shadowsEnabled
);

void DrawModelCelShaded(
    Model model, 
    Vector3 position, 
    float scale, 
    Shader celShader, 
    Shader outlineShader, 
    Vector3 lightDirection, 
    Vector3 viewPosition,
    Color outlineColor, 
    float outlineThickness,
    bool highlightEnabled
);
