# ==========================================
# DEFINA A VERSÃO AQUI
# ==========================================
$version = "2.8.0" 

# O nome do arquivo mudou para .exe nas versões recentes
$exeFile = "w64devkit-x64-$version.7z.exe"
$url = "https://github.com/skeeto/w64devkit/releases/download/v$version/$exeFile"

if (-not (Test-Path "w64devkit")) {
    Write-Host "Baixando a versão fixa ($version)..."
    
    try {
        Invoke-WebRequest -Uri $url -OutFile $exeFile
        Write-Host "Download concluído. Extraindo silenciosamente..."
        
        # O parâmetro -y responde "sim" para tudo e o -o define o diretório de destino
        Start-Process -FilePath ".\$exeFile" -ArgumentList "-y", "-o`"$PWD`"" -Wait -NoNewWindow
        
        # Apaga o executável de instalação para manter a pasta limpa
        Remove-Item $exeFile
    } catch {
        Write-Host "ERRO: Não foi possível baixar ou extrair a versão $version." -ForegroundColor Red
        if (Test-Path $exeFile) { Remove-Item $exeFile }
        exit
    }
} else {
    Write-Host "A pasta w64devkit já existe. Pulando o download."
}

# Configura o terminal atual para reconhecer os comandos
$env:PATH = "$PWD\w64devkit\bin;" + $env:PATH
Write-Host "Ambiente 100% pronto! Já pode rodar o 'make'."