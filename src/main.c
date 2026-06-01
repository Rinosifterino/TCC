#include "raylib.h"
#include "gui.h"
#include "celShader.h"
#include <math.h>

Shader LoadLambertShader(void);

int main(void)
{
    InitWindow(1800, 920, "Cel Shading Teste");
    SetTargetFPS(60);

    Camera3D camera = {0};
    camera.position = (Vector3){4.0f, 4.0f, 4.0f};
    camera.target = (Vector3){0.0f, 0.0f, 0.0f};
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    Model models[4];
    models[0] = LoadModelFromMesh(GenMeshSphere(1.0f, 32, 32));
    models[1] = LoadModel("assets/bee.obj");
    models[2] = LoadModel("assets/link.glb");
    int activeModelIndex = 0;

    Texture2D texture = LoadTexture("assets/wood.png");
    Shader celShader = LoadCelShader();
    Shader lambertShader = LoadLambertShader();
    Shader outlineShader = LoadOutlineShader();

    for (int m = 0; m < 2; m++)
    {
        models[m].materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
    }

    for (int m = 0; m < 4; m++)
    {
        for (int i = 0; i < models[m].materialCount; i++)
        {
            models[m].materials[i].shader = lambertShader;
        }
    }

    Vector3 lightDir = {1.0f, 0.4f, 0.2f};
    SetShaderValue(lambertShader, GetShaderLocation(lambertShader, "lightDirection"), &lightDir, SHADER_UNIFORM_VEC3);

    bool useCelShading = true;
    bool orbitalLight = true;
    bool highlightEnabled = true;
    Vector3 staticLightDir = {-1.0f, 1.0f, -0.2f};

    while (!WindowShouldClose())
    {
        UpdateCamera(&camera, CAMERA_FREE);

        Vector3 lightDir;
        if (orbitalLight) {
            float time = GetTime();
            lightDir = (Vector3){ cosf(time), 1.0f, sinf(time) };
        } else {
            lightDir = staticLightDir;
        }

        SetShaderValue(
            lambertShader, 
            GetShaderLocation(lambertShader, "lightDirection"), 
            &lightDir, 
            SHADER_UNIFORM_VEC3
        );

        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode3D(camera);

        if (useCelShading)
        {
            DrawModelCelShaded(
                models[activeModelIndex], 
                (Vector3){0.0f, 0.0f, 0.0f}, 
                1.0f,                        
                celShader,                  
                outlineShader,
                lightDir,    
                camera.position,                
                BLACK,                       
                0.05f,
                highlightEnabled
            );
        }
        else
        {
            DrawModel(models[activeModelIndex], (Vector3){0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
        }

        DrawGrid(20, 1.0f);
        EndMode3D();

        activeModelIndex = DrawGUI(activeModelIndex, &useCelShading, &orbitalLight, &highlightEnabled);

        EndDrawing();
    }

    UnloadTexture(texture);
    UnloadShader(celShader);
    UnloadShader(lambertShader);
    UnloadShader(outlineShader); 
    for (int m = 0; m < 2; m++) UnloadModel(models[m]);

    CloseWindow();
    return 0;
}