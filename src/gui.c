#include "gui.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

int DrawGUI(int activeModelIndex, bool *useCelShading, bool *orbitalLight, bool *highlightEnabled)
{
    if (IsKeyPressed(KEY_ONE)) *useCelShading = false;
    if (IsKeyPressed(KEY_TWO)) *useCelShading = true;
    
    // Inverte o estado da variável ao carregar
    if (IsKeyPressed(KEY_NINE)) *orbitalLight = !(*orbitalLight);
    if (IsKeyPressed(KEY_ZERO)) *highlightEnabled = !(*highlightEnabled);

    DrawText("1 - Lambert | 2 - Cel Shading", 10, 10, 20, DARKGRAY);
    DrawText(*useCelShading ? "Mode: CEL SHADING" : "Mode: DEFAULT", 10, 40, 20, MAROON);
    
    DrawText("9 - Toggle Orbital Light", 10, 70, 20, DARKGRAY);
    DrawText(*orbitalLight ? "Light: ORBITAL" : "Light: STATIC", 10, 100, 20, MAROON);
    
    DrawText("0 - Toggle Highlight", 10, 130, 20, DARKGRAY);
    DrawText(*highlightEnabled ? "Highlight: ON" : "Highlight: OFF", 10, 160, 20, MAROON);

    if (IsKeyPressed(KEY_TAB)) {
            if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) {
                // CTRL + TAB: Volta para o modelo anterior
                activeModelIndex = (activeModelIndex - 1 + 4) % 4; 
            }
            else {
                // TAB: Avança para o próximo modelo
                activeModelIndex = (activeModelIndex + 1) % 4;
            }
        }

    GuiToggleGroup(
        (Rectangle){ 10, 200, 120, 30 }, // Descemos um pouco o menu para caber os textos
        "Esfera;Arvore;Link Ocarina", 
        &activeModelIndex
    );

    return activeModelIndex;
}