cmake_minimum_required(VERSION 3.14)
project(MeuTCC C)

# Define o padrão C (C99 é recomendado para raylib)
set(CMAKE_C_STANDARD 99)

# Baixa o raylib diretamente do GitHub
include(FetchContent)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG master # Pode trocar por uma versão específica, ex: 5.0
)
FetchContent_MakeAvailable(raylib)

# Adiciona seus arquivos fonte
add_executable(${PROJECT_NAME} src/main.c)

# Linka o raylib ao seu projeto
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)