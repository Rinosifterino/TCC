#include "celShader.h"
#include "rlgl.h"

// =========================================
// SHADERS DO CEL SHADING
// =========================================
const char *celVertexShader =
    "#version 330\n"
    "in vec3 vertexPosition;\n"
    "in vec2 vertexTexCoord;\n"
    "in vec3 vertexNormal;\n"
    "in vec4 vertexColor;\n"
    "uniform mat4 mvp;\n"
    "uniform mat4 matModel;\n" // Matriz do modelo
    "out vec2 fragTexCoord;\n"
    "out vec4 fragColor;\n"
    "out vec3 fragNormal;\n"
    "out vec3 fragPosition;\n" // Posição real 3D
    "void main()\n"
    "{\n"
    "    fragTexCoord = vertexTexCoord;\n"
    "    fragColor = vertexColor;\n"
    "    fragNormal = normalize(vertexNormal);\n"
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
    "uniform int useHighlight;\n" // <-- NOVA VARIÁVEL GLSL
    "out vec4 finalColor;\n"
    "void main()\n"
    "{\n"
    "    vec3 normal = normalize(fragNormal);\n"
    "    vec3 lightDir = normalize(lightDirection);\n"
    "    float diffuse = max(dot(normal, lightDir), 0.0);\n"
    "    if (diffuse > 0.75) diffuse = 1.0;\n"
    "    else diffuse = 0.50;\n"
    //"    else diffuse = 0.20;\n"
    "    float ambient = 0.15;\n"
    
    // CÁLCULO DO GLOSSY DINÂMICO
    "    float specular = 0.0;\n"
    "    if (useHighlight == 1) {\n"
    "        vec3 viewDir = normalize(viewPos - fragPosition);\n"
    "        vec3 reflectDir = reflect(-lightDir, normal);\n"
    "        specular = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);\n"
    "        if (specular > 0.45) specular = 1.0;\n"
    "        else specular = 0.0;\n"
    "    }\n"
    
    "    vec4 albedo = texture(texture0, fragTexCoord) * colDiffuse;\n"
    "    vec3 color = albedo.rgb * (ambient + diffuse);\n"
    "    color += vec3(specular);\n"
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