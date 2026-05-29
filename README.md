# Motor 3D - TCC

Este projeto é desenvolvido em **C** utilizando a biblioteca **raylib** e o sistema de build **CMake**. 

O gerenciamento da dependência da raylib é feito de forma automática via `FetchContent` no CMake. Isso significa que o próprio sistema de build se encarrega de baixar e compilar a versão correta da biblioteca diretamente do repositório oficial, garantindo que o projeto rode em qualquer máquina com um simples `git clone`.

---

## 📂 Estrutura do Repositório

Para que o projeto funcione corretamente, mantenha a seguinte estrutura de arquivos:

```text
├── .gitignore
├── CMakeLists.txt
├── README.md
└── src/
    └── main.c
```

---

## 🛠️ Pré-requisitos

O projeto requer um compilador C (GCC) e o CMake instalados.

### 🔹 Windows (Ambiente Portátil / Sem Admin)
Se você estiver rodando o projeto em um computador bloqueado ou sem privilégios de administrador:
1. Baixe a versão autoextraível do **w64devkit** (`w64devkit-x64-*.7z.exe`) na [página de releases do GitHub](https://github.com/skeeto/w64devkit/releases).
2. Extraia o conteúdo em uma pasta de fácil acesso (ex: `C:\Users\SeuUsuario\Documents\w64devkit`).
3. Abra o PowerShell na pasta raiz do projeto e carregue os binários do compilador temporariamente no PATH antes de compilar:
   ```powershell
   $env:PATH = "C:\Caminho\Ate\O\w64devkit\bin;" + $env:PATH
   ```

### 🔹 Linux (Debian/Ubuntu)
Instale o compilador, CMake e as dependências gráficas da raylib via terminal:
```bash
sudo apt update
sudo apt install build-essential cmake libasound2-dev libx11-dev libxrandr-dev libxi-dev libxinerama-dev libxcursor-dev libgl1-mesa-dev
```

---

## 🚀 Como Compilar e Rodar

Após clonar o repositório e abrir o terminal (ou PowerShell configurado) na pasta raiz do projeto, execute os comandos abaixo:

### 1. Gerar os arquivos de configuração do build
* **No Windows (usando MinGW do w64devkit):**
  ```bash
  cmake -G "MinGW Makefiles" -B build
  ```
* **No Linux / macOS:**
  ```bash
  cmake -B build
  ```

### 2. Compilar o código fonte
```bash
cmake --build build
```

### 3. Executar o binário gerado
* **No Windows:**
  ```powershell
  .\build\MeuTCC.exe
  ```
* **No Linux / macOS:**
  ```bash
  ./build/MeuTCC
  ```

---

## 📄 Arquivos de Configuração Base

### `CMakeLists.txt`
```cmake
cmake_minimum_required(VERSION 3.14)
project(MeuTCC C)

set(CMAKE_C_STANDARD 99)

include(FetchContent)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY [https://github.com/raysan5/raylib.git](https://github.com/raysan5/raylib.git)
    GIT_TAG master
)
FetchContent_MakeAvailable(raylib)

add_executable(${PROJECT_NAME} src/main.c)
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)
```

### `.gitignore`
```text
build/
.cache/
```