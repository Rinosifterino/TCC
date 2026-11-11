#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include "gui.h"
#include "celShader.h"
#include <math.h>

#define SHADOWMAP_RESOLUTION 2048

Shader LoadLambertShader(void);

// Cria um Framebuffer (FBO) apenas com textura de profundidade para o Shadow Mapping
static RenderTexture2D LoadShadowmapRenderTexture(int width, int height)
{
    RenderTexture2D target = { 0 };
    target.id = rlLoadFramebuffer();
    target.texture.width = width;
    target.texture.height = height;

    if (target.id > 0)
    {
        rlEnableFramebuffer(target.id);

        target.depth.id = rlLoadTextureDepth(width, height, false);
        target.depth.width = width;
        target.depth.height = height;
        target.depth.format = 19; // DEPTH_COMPONENT_24BIT
        target.depth.mipmaps = 1;

        rlFramebufferAttach(target.id, target.depth.id, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_TEXTURE2D, 0);

        if (rlFramebufferComplete(target.id))
            TRACELOG(LOG_INFO, "FBO: [ID %i] Shadowmap Framebuffer criado com sucesso (%ix%i)", target.id, width, height);

        rlDisableFramebuffer();
    }
    else
    {
        TRACELOG(LOG_WARNING, "FBO: Nao foi possivel criar o Framebuffer de Shadowmap");
    }

    return target;
}

static void UnloadShadowmapRenderTexture(RenderTexture2D target)
{
    if (target.id > 0)
    {
        rlUnloadFramebuffer(target.id);
    }
}

int main(void)
{
    InitWindow(1800, 920, "Cel Shading & Shadow Mapping - TCC");
    SetTargetFPS(60);

    Camera3D camera = {0};
    camera.position = (Vector3){4.0f, 4.0f, 4.0f};
    camera.target = (Vector3){0.0f, 0.0f, 0.0f};
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    Model models[3];
    models[0] = LoadModelFromMesh(GenMeshSphere(1.0f, 32, 32));
    models[1] = LoadModel("assets/tree.glb");
    models[2] = LoadModel("assets/link.glb");
    int activeModelIndex = 0;

    // Calcula a BoundingBox de cada modelo para posicionar o chão receptor de sombra exatamente na base
    BoundingBox modelBoxes[3];
    Vector3 modelCenters[3];
    float modelMaxDims[3];
    for (int m = 0; m < 3; m++)
    {
        modelBoxes[m] = GetModelBoundingBox(models[m]);
        modelCenters[m] = (Vector3){
            (modelBoxes[m].min.x + modelBoxes[m].max.x) * 0.5f,
            (modelBoxes[m].min.y + modelBoxes[m].max.y) * 0.5f,
            (modelBoxes[m].min.z + modelBoxes[m].max.z) * 0.5f
        };
        float sx = modelBoxes[m].max.x - modelBoxes[m].min.x;
        float sy = modelBoxes[m].max.y - modelBoxes[m].min.y;
        float sz = modelBoxes[m].max.z - modelBoxes[m].min.z;
        float maxD = fmaxf(sx, fmaxf(sy, sz));
        modelMaxDims[m] = (maxD < 2.0f) ? 2.0f : maxD;
    }

    // Cria um plano de chão (1x1 base, escalado dinamicamente conforme o modelo) para receber as sombras projetadas
    Model floorModel = LoadModelFromMesh(GenMeshPlane(1.0f, 1.0f, 4, 4));
    floorModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = (Color){ 225, 225, 225, 255 };

    Texture2D texture = LoadTexture("assets/wood.png");
    Shader celShader = LoadCelShader();
    Shader lambertShader = LoadLambertShader();
    Shader outlineShader = LoadOutlineShader();
    RenderTexture2D shadowMap = LoadShadowmapRenderTexture(SHADOWMAP_RESOLUTION, SHADOWMAP_RESOLUTION);

    for (int m = 0; m < 2; m++)
    {
        models[m].materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
    }

    for (int m = 0; m < 3; m++)
    {
        for (int i = 0; i < models[m].materialCount; i++)
        {
            models[m].materials[i].shader = lambertShader;
        }
    }
    floorModel.materials[0].shader = lambertShader;

    Vector3 lightDir = {1.0f, 0.4f, 0.2f};
    SetShaderValue(lambertShader, GetShaderLocation(lambertShader, "lightDirection"), &lightDir, SHADER_UNIFORM_VEC3);

    bool useCelShading = true;
    bool orbitalLight = true;
    bool highlightEnabled = true;
    bool shadowsEnabled = true;
    Vector3 staticLightDir = {1.0f, 0.3f, 0.6f};
    float lightAngle = atan2f(staticLightDir.z, staticLightDir.x);

    while (!WindowShouldClose())
    {
        UpdateCamera(&camera, CAMERA_FREE);
        DisableCursor();

        float dt = GetFrameTime();
        // Controle manual da altura da luz (I = sobe, K = desce) tanto no modo Orbital quanto Estático
        if (IsKeyDown(KEY_I)) staticLightDir.y += 1.0f * dt;
        if (IsKeyDown(KEY_K)) staticLightDir.y -= 1.0f * dt;
        if (staticLightDir.y < 0.05f) staticLightDir.y = 0.05f;

        // Controle manual da rotação horizontal da luz (J / L)
        if (IsKeyDown(KEY_J)) { orbitalLight = false; lightAngle -= 1.5f * dt; }
        if (IsKeyDown(KEY_L)) { orbitalLight = false; lightAngle += 1.5f * dt; }

        if (orbitalLight) {
            lightAngle += dt;
        }

        staticLightDir.x = cosf(lightAngle);
        staticLightDir.z = sinf(lightAngle);
        Vector3 lightDir = staticLightDir;

        SetShaderValue(
            lambertShader, 
            GetShaderLocation(lambertShader, "lightDirection"), 
            &lightDir, 
            SHADER_UNIFORM_VEC3
        );

        Vector3 center = modelCenters[activeModelIndex];
        float maxDim = modelMaxDims[activeModelIndex];
        Vector3 floorPos = { center.x, modelBoxes[activeModelIndex].min.y, center.z };
        float floorScale = maxDim * 5.0f;

        // Configura a câmera ortográfica da luz direcional para o Shadow Map
        Vector3 normLightDir = Vector3Normalize(lightDir);
        Camera3D lightCam = { 0 };
        lightCam.position = Vector3Add(center, Vector3Scale(normLightDir, maxDim * 3.5f));
        lightCam.target = center;
        lightCam.up = (Vector3){ 0.0f, 1.0f, 0.0f };
        lightCam.fovy = maxDim * 3.5f;
        lightCam.projection = CAMERA_ORTHOGRAPHIC;

        // =====================================================
        // PASSO 1: RENDERIZAÇÃO DO SHADOW MAP (DEPTH PASS)
        // =====================================================
        Matrix lightView = { 0 };
        Matrix lightProj = { 0 };

        // Evita Framebuffer Feedback Loop desvinculando a textura de sombra antes de escrever nela
        int zeroShadows = 0;
        SetShaderValue(lambertShader, GetShaderLocation(lambertShader, "useShadows"), &zeroShadows, SHADER_UNIFORM_INT);
        rlActiveTextureSlot(10);
        rlDisableTexture();
        rlActiveTextureSlot(0);

        // Ajusta os planos Near/Far para concentrar a precisão do Depth Buffer na região do modelo
        rlSetClipPlanes(0.1, maxDim * 8.0);

        BeginTextureMode(shadowMap);
        ClearBackground(WHITE);
        BeginMode3D(lightCam);
            lightView = rlGetMatrixModelview();
            lightProj = rlGetMatrixProjection();
            if (shadowsEnabled)
            {
                rlDisableBackfaceCulling();
                DrawModel(models[activeModelIndex], (Vector3){0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
                rlEnableBackfaceCulling();
            }
        EndMode3D();
        EndTextureMode();

        // Restaura os planos Near/Far padrão para a câmera principal
        rlSetClipPlanes(0.05, 4000.0);

        Matrix lightVP = MatrixMultiply(lightView, lightProj);

        // =====================================================
        // PASSO 2: RENDERIZAÇÃO PRINCIPAL DA CENA
        // =====================================================
        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode3D(camera);

        // Atualiza os uniforms de Shadow Mapping em ambos os shaders
        SetShaderShadowMap(lambertShader, lightVP, shadowMap.depth.id, SHADOWMAP_RESOLUTION, shadowsEnabled);
        SetShaderShadowMap(celShader, lightVP, shadowMap.depth.id, SHADOWMAP_RESOLUTION, shadowsEnabled);

        if (useCelShading)
        {
            // Desenha o chão receptor de sombra com Cel Shader (sem outline)
            DrawModelCelShaded(
                floorModel,
                floorPos,
                floorScale,
                celShader,
                outlineShader,
                lightDir,
                camera.position,
                BLACK,
                0.0f,
                false
            );

            // Desenha o modelo ativo com Cel Shader + Outline
            DrawModelCelShaded(
                models[activeModelIndex], 
                (Vector3){0.0f, 0.0f, 0.0f}, 
                1.0f,                        
                celShader,                  
                outlineShader,
                lightDir,    
                camera.position,                
                BLACK,                       
                0.03f,
                highlightEnabled
            );
        }
        else
        {
            DrawModel(floorModel, floorPos, floorScale, WHITE);
            DrawModel(models[activeModelIndex], (Vector3){0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
        }

        // Indicador visual (Gizmo) da posição e direção da fonte de luz na cena 3D
        Vector3 lightGizmoPos = Vector3Add(center, Vector3Scale(normLightDir, maxDim * 1.35f));
        Vector3 lightRayEnd = Vector3Add(center, Vector3Scale(normLightDir, maxDim * 0.75f));
        DrawSphere(lightGizmoPos, maxDim * 0.05f, GOLD);
        DrawLine3D(lightGizmoPos, lightRayEnd, ORANGE);

        DrawGrid(20, 1.0f);
        EndMode3D();

        activeModelIndex = DrawGUI(activeModelIndex, &useCelShading, &orbitalLight, &highlightEnabled, &shadowsEnabled);

        EndDrawing();
    }

    UnloadShadowmapRenderTexture(shadowMap);
    UnloadModel(floorModel);
    UnloadTexture(texture);
    UnloadShader(celShader);
    UnloadShader(lambertShader);
    UnloadShader(outlineShader); 
    for (int m = 0; m < 3; m++) UnloadModel(models[m]);
    EnableCursor();

    CloseWindow();
    return 0;
}
