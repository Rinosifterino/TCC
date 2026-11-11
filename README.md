# Motor 3D / Cel Shading - TCC

Projeto desenvolvido em **C (C99)** utilizando **raylib** e **raygui** para demonstração e comparação de técnicas de renderização 3D (**Lambert Shading** vs. **Cel Shading** com contorno e highlight especular).

A compilação é gerenciada via **Makefile** e, no Windows, o projeto já inclui os cabeçalhos (`include/`), a biblioteca pré-compilada 64-bit (`lib/`) e o script de provisionamento automático do compilador (`setup.ps1`).

---

## 📂 Estrutura do Projeto

```text
TCC/
├── .gitignore
├── Makefile              # Regras de compilação (Windows e Linux)
├── README.md             # Este guia
├── setup.ps1             # Baixa e configura o w64devkit (x64) no Windows
├── assets/               # Modelos 3D e texturas carregados em tempo de execução
│   ├── link.glb
│   ├── tree.glb
│   └── wood.png
├── include/              # Cabeçalhos da raylib e raygui
│   ├── raygui.h
│   ├── raylib.h
│   ├── raymath.h
│   └── rlgl.h
├── lib/                  # Bibliotecas pré-compiladas da raylib (Windows x86_64)
│   ├── libraylib.a
│   ├── libraylibdll.a
│   └── raylib.dll
└── src/                  # Código-fonte do projeto
    ├── main.c            # Loop principal, câmera, carregamento de modelos e shaders
    ├── gui.c / gui.h     # Interface gráfica (raygui) e atalhos de teclado
    ├── celShader.c / .h  # Shaders de Cel Shading e Outline (contorno)
    └── lambertShader.c   # Shader de iluminação difusa (Lambert)
```

---

## 🚀 Como Compilar e Rodar no Windows (Passo a Passo)

### 1. Abrir o terminal na pasta correta
Abra o **PowerShell** e navegue até a pasta `TCC` (onde estão localizados o `Makefile` e o `setup.ps1`):
```powershell
cd caminho\para\TCC
```

### 2. Configurar o compilador (`setup.ps1`)
Execute o script `setup.ps1`:
```powershell
.\setup.ps1
```
**O que esse comando faz:**
- Verifica se a pasta `w64devkit` já existe dentro do projeto.
- Caso não exista, baixa automaticamente o **w64devkit v2.8.0 (x64 / 64-bit)** do GitHub, extrai na pasta `TCC\w64devkit` e remove o instalador temporário.
- Adiciona `TCC\w64devkit\bin` **no início do `PATH`** da sessão atual do PowerShell (garantindo que o `gcc` e o `make` de 64 bits sejam usados).

> **⚠️ Caso o PowerShell bloqueie a execução do `.ps1`:**
> Rode o comando abaixo para liberar a execução apenas na janela atual do terminal e depois rode o `.\setup.ps1` novamente:
> ```powershell
> Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
> .\setup.ps1
> ```

> **💡 Observação importante:** O `setup.ps1` altera o `PATH` apenas da janela atual do PowerShell. Sempre que abrir um novo terminal para compilar, rode `.\setup.ps1` antes do `make` (como a pasta `w64devkit` já estará baixada, ele pulará o download e configurará o `PATH` instantaneamente).

### 3. Compilar o projeto (`make`)
No mesmo terminal onde rodou o `setup.ps1`, execute:
```powershell
make
```
Isso criará a pasta `build/` e gerará o executável `build/MeuTCC.exe`.

### 4. Executar a aplicação
Ainda **dentro da pasta `TCC`** (necessário para que o executável encontre os arquivos em `assets/`), execute:
```powershell
.\build\MeuTCC.exe
```

---

## 🐧 Como Compilar e Rodar no Linux (Debian/Ubuntu)

1. Instale o `gcc`, `make` e as dependências da `raylib`:
   ```bash
   sudo apt update
   sudo apt install build-essential libraylib-dev
   ```
2. Na pasta `TCC/`, compile o projeto:
   ```bash
   make
   ```
3. Execute a partir da pasta `TCC/`:
   ```bash
   ./build/MeuTCC
   ```

---

## 🎮 Controles da Aplicação

Durante a execução do `MeuTCC`:

| Tecla / Controle | Ação |
| :--- | :--- |
| **Mouse** | Rotaciona a câmera 3D (`CAMERA_FREE`) |
| **W, A, S, D** | Movimenta a câmera pelo cenário |
| **Scroll do Mouse** | Zoom in / Zoom out |
| **`1`** | Alterna para o modo **Lambert Shading** (Default) |
| **`2`** | Alterna para o modo **Cel Shading** (com contorno) |
| **`8`** | Liga / Desliga o **Shadow Mapping** (sombras projetadas) |
| **`9`** | Liga / Desliga a **Luz Orbital** (luz girando ou estática) |
| **`0`** | Liga / Desliga o **Highlight** especular (no modo Cel Shading) |
| **`I` / `K`** | Sobe / Abaixa a **altura da fonte de luz** em tempo real |
| **`J` / `L`** | Gira a **fonte de luz** horizontalmente ao redor do modelo |
| **`TAB`** | Avança para o próximo modelo 3D (Esfera → Árvore → Link) |
| **`Ctrl + TAB`** | Volta para o modelo 3D anterior |
| **`ESC`** | Fecha a aplicação |

---

## 🛠️ Solução de Problemas Comuns (Troubleshooting)

* **Erro `undefined reference to ...` ou `file format not recognized` ao rodar `make`:**
  - **Causa:** O terminal está utilizando um compilador GCC de **32 bits (`i686`)** instalado globalmente na máquina (ex.: `C:\w64devkit\bin\gcc.exe`), que é incompatível com a biblioteca `lib/libraylib.a` do projeto (compilada em **64 bits `x86_64`**).
  - **Solução:** Rode `.\setup.ps1` antes de rodar `make` na mesma janela do PowerShell. Ele colocará o compilador 64-bit local na frente do `PATH`.

* **Janela abre sem texturas ou modelos não aparecem (`Failed to open file assets/...`):**
  - **Causa:** O executável foi iniciado de fora da pasta `TCC/` (por exemplo, entrando em `cd build` e rodando `.\MeuTCC.exe`).
  - **Solução:** Execute sempre a partir da raiz do projeto (`TCC/`), usando `.\build\MeuTCC.exe`.

* **Recompilar do zero (limpar arquivos de build):**
  ```powershell
  make clean
  make
  ```
