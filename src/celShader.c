#include "celShader.h"
#include "rlgl.h"

// =========================================
// SHADERS DO CEL SHADING (COM SHADOW MAPPING)
// =========================================
const char *celVertexShader =
    "#version 330\n"
    "in vec3 vertexPosition;\n"
    "in vec2 vertexTexCoord;\n"
    "in vec3 vertexNormal;\n"
    "in vec4 vertexColor;\n"
    "uniform mat4 mvp;\n"
    "uniform mat4 matModel;\n"  // Matriz do modelo
    "uniform mat4 matNormal;\n" // Matriz de transformação das normais
    "out vec2 fragTexCoord;\n"
    "out vec4 fragColor;\n"
    "out vec3 fragNormal;\n"
    "out vec3 fragPosition;\n" // Posição real 3D no mundo
    "void main()\n"
    "{\n"
    "    fragTexCoord = vertexTexCoord;\n"
    "    fragColor = vertexColor;\n"
    "    fragNormal = normalize(vec3(matNormal * vec4(vertexNormal, 0.0)));\n"
    "    fragPosition = vec3(matModel * vec4(vertexPosition, 1.0));\n"
    "    gl_Position = mvp * vec4(vertexPosition, 1.0);\n"
    "}\n";

const char *celFragmentShader =
    "#version 330\n"
    "in vec2 fragTexCoord;\n"
    "in vec4 fragColor;\n"
    "in vec3 fragNormal;\n"
    "in vec3 fragPosition;\n"
    "uniform vec3 lightDirection;\n"
    "uniform vec3 viewPos;\n"
    "uniform vec4 colDiffuse;\n"
    "uniform sampler2D texture0;\n"
    "uniform int useHighlight;\n"
    "uniform mat4 lightVP;\n"
    "uniform sampler2D shadowMap;\n"
    "uniform int useShadows;\n"
    "uniform int shadowMapResolution;\n"
    "out vec4 finalColor;\n"
    "\n"
    "float CalculateShadow(vec3 fragPosWorld, vec3 normal, vec3 lightDir)\n"
    "{\n"
    "    vec4 fragPosLightSpace = lightVP * vec4(fragPosWorld, 1.0);\n"
    "    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;\n"
    "    projCoords = projCoords * 0.5 + 0.5;\n"
    "    if (projCoords.z > 1.0 || projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0)\n"
    "        return 0.0;\n"
    "    float bias = max(0.0025 * (1.0 - dot(normal, lightDir)), 0.0005);\n"
    "    float currentDepth = projCoords.z;\n"
    "    vec2 texelSize = 1.0 / vec2(float(shadowMapResolution));\n"
    "    int shadowCounter = 0;\n"
    "    for (int x = -1; x <= 1; x++)\n"
    "    {\n"
    "        for (int y = -1; y <= 1; y++)\n"
    "        {\n"
    "            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;\n"
    "            if (currentDepth - bias > pcfDepth) shadowCounter++;\n"
    "        }\n"
    "    }\n"
    "    return float(shadowCounter) / 9.0;\n"
    "}\n"
    "\n"
    "void main()\n"
    "{\n"
    "    vec3 normal = normalize(fragNormal);\n"
    "    vec3 lightDir = normalize(lightDirection);\n"
    "    float NdotL = max(dot(normal, lightDir), 0.0);\n"
    "\n"
    "    float shadow = (useShadows == 1) ? CalculateShadow(fragPosition, normal, lightDir) : 0.0;\n"
    "    float effectiveLight = (shadow > 0.5) ? 0.0 : NdotL;\n"
    "\n"
    "    // Cel Shading de 2 tons: Tom Claro (1.0) e Tom de Sombra (0.5)\n"
    "    float diffuse = (effectiveLight > 0.2) ? 0.5 : 0.0;\n"
    "    float ambient = 0.5;\n"
    "\n"
    "    // CÁLCULO DO GLOSSY DINÂMICO (apenas em faces iluminadas e fora da sombra)\n"
    "    float specular = 0.0;\n"
    "    if (useHighlight == 1 && effectiveLight > 0.0) {\n"
    "        vec3 viewDir = normalize(viewPos - fragPosition);\n"
    "        vec3 reflectDir = reflect(-lightDir, normal);\n"
    "        specular = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);\n"
    "        if (specular > 0.45) specular = 1.0;\n"
    "        else specular = 0.0;\n"
    "    }\n"
    "\n"
    "    vec4 albedo = texture(texture0, fragTexCoord) * fragColor * colDiffuse;\n"
    "    vec3 color = albedo.rgb * (ambient + diffuse);\n"
    "    color = min(color + vec3(specular), vec3(1.0));\n"
    "    finalColor = vec4(color, albedo.a);\n"
    "}\n";

// =========================================
// SHADERS DO OUTLINE (Crescimento pela Normal)
// =========================================
const char *outlineVertexShader =
    "#version 330\n"
    "in vec3 vertexPosition;\n"
    "in vec3 vertexNormal;\n"
    "uniform mat4 mvp;\n"
    "uniform float outlineThickness;\n"
    "void main()\n"
    "{\n"
    "    // Empurra os vértices para fora seguindo a Normal\n"
    "    vec3 pos = vertexPosition + (vertexNormal * outlineThickness);\n"
    "    gl_Position = mvp * vec4(pos, 1.0);\n"
    "}\n";

const char *outlineFragmentShader =
    "#version 330\n"
    "uniform vec4 outlineColor;\n"
    "out vec4 finalColor;\n"
    "void main()\n"
    "{\n"
    "    finalColor = outlineColor;\n"
    "}\n";

// =========================================
// IMPLEMENTAÇÃO DA API
// =========================================

Shader LoadCelShader(void)
{
    return LoadShaderFromMemory(celVertexShader, celFragmentShader);
}

Shader LoadOutlineShader(void)
{
    return LoadShaderFromMemory(outlineVertexShader, outlineFragmentShader);
}

void SetShaderShadowMap(Shader shader, Matrix lightVP, unsigned int shadowMapTexId, int shadowMapResolution, bool shadowsEnabled)
{
    int lightVPLoc = GetShaderLocation(shader, "lightVP");
    int shadowMapLoc = GetShaderLocation(shader, "shadowMap");
    int useShadowsLoc = GetShaderLocation(shader, "useShadows");
    int resLoc = GetShaderLocation(shader, "shadowMapResolution");

    int useShadowsVal = shadowsEnabled ? 1 : 0;
    SetShaderValueMatrix(shader, lightVPLoc, lightVP);
    SetShaderValue(shader, useShadowsLoc, &useShadowsVal, SHADER_UNIFORM_INT);
    SetShaderValue(shader, resLoc, &shadowMapResolution, SHADER_UNIFORM_INT);

    int textureSlot = 10;
    rlEnableShader(shader.id);
    rlActiveTextureSlot(textureSlot);
    rlEnableTexture(shadowMapTexId);
    rlSetUniform(shadowMapLoc, &textureSlot, RL_SHADER_UNIFORM_SAMPLER2D, 1);
    rlActiveTextureSlot(0);
}

void DrawModelCelShaded(Model model, Vector3 position, float scale, Shader celShader, Shader outlineShader, Vector3 lightDirection, Vector3 viewPosition, Color outlineColor, float outlineThickness, bool highlightEnabled)
{
    // Passa a luz
    int lightDirLoc = GetShaderLocation(celShader, "lightDirection");
    float lightDir[3] = { lightDirection.x, lightDirection.y, lightDirection.z };
    SetShaderValue(celShader, lightDirLoc, lightDir, SHADER_UNIFORM_VEC3);

    // PASSA A POSIÇÃO DA CÂMARA PARA O SHADER
    int viewPosLoc = GetShaderLocation(celShader, "viewPos");
    float vPos[3] = { viewPosition.x, viewPosition.y, viewPosition.z };
    SetShaderValue(celShader, viewPosLoc, vPos, SHADER_UNIFORM_VEC3);

    int highlightLoc = GetShaderLocation(celShader, "useHighlight");
    int hlValue = highlightEnabled ? 1 : 0;
    SetShaderValue(celShader, highlightLoc, &hlValue, SHADER_UNIFORM_INT);

    // Salva estado original
    Shader originalShaders[256];
    for (int i = 0; i < model.materialCount; i++) originalShaders[i] = model.materials[i].shader;

    // ==================================
    // PASSO 1: OUTLINE (VIA SHADER)
    // ==================================
    if (outlineThickness > 0.0f)
    {
        rlEnableBackfaceCulling();
        rlSetCullFace(RL_CULL_FACE_FRONT);

        // Passa a espessura e a cor para o Shader de Outline
        int thickLoc = GetShaderLocation(outlineShader, "outlineThickness");
        SetShaderValue(outlineShader, thickLoc, &outlineThickness, SHADER_UNIFORM_FLOAT);

        int colorLoc = GetShaderLocation(outlineShader, "outlineColor");
        float c[4] = { outlineColor.r/255.0f, outlineColor.g/255.0f, outlineColor.b/255.0f, outlineColor.a/255.0f };
        SetShaderValue(outlineShader, colorLoc, c, SHADER_UNIFORM_VEC4);

        for (int i = 0; i < model.materialCount; i++) model.materials[i].shader = outlineShader;

        // O desenho volta a ser normal, sem gambiarras de escala! O Shader faz o trabalho.
        DrawModel(model, position, scale, WHITE);

        rlSetCullFace(RL_CULL_FACE_BACK);
    }

    // ==================================
    // PASSO 2: DESENHO CEL SHADING
    // ==================================
    for (int i = 0; i < model.materialCount; i++) model.materials[i].shader = celShader;
    DrawModel(model, position, scale, WHITE);

    // ==================================
    // PASSO 3: RESTAURAR O MODELO
    // ==================================
    for (int i = 0; i < model.materialCount; i++) model.materials[i].shader = originalShaders[i];
}
