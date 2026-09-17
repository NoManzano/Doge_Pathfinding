param([string]$Toolchain = 'C:\msys64\ucrt64')
$ErrorActionPreference = 'Stop'
Push-Location $PSScriptRoot
try {
    $env:PATH = "$Toolchain\bin;" + $env:PATH
    $compiler = Join-Path $Toolchain 'bin\g++.exe'
    if (!(Test-Path -LiteralPath $compiler)) { throw "No se encontro g++ en $Toolchain" }
    New-Item -ItemType Directory -Force -Path 'bin','dist\DoomPathfinding' | Out-Null
    & $compiler -std=c++17 -O2 -Wall -Wextra -Iinclude src/main.cpp src/Juego.cpp src/Mapa.cpp src/Personaje.cpp -o bin/juego.exe -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio
    if ($LASTEXITCODE -ne 0) { throw 'La compilacion fallo' }
    & .\bin\juego.exe --self-test
    if ($LASTEXITCODE -ne 0) { throw 'Las pruebas fallaron' }
    Copy-Item -LiteralPath 'bin\juego.exe' -Destination 'dist\DoomPathfinding\juego.exe' -Force
    $pending = [System.Collections.Generic.Queue[string]]::new()
    $seen = [System.Collections.Generic.HashSet[string]]::new([StringComparer]::OrdinalIgnoreCase)
    $pending.Enqueue((Join-Path $PSScriptRoot 'dist\DoomPathfinding\juego.exe'))
    while ($pending.Count -gt 0) {
        $binary = $pending.Dequeue()
        $headers = & "$Toolchain\bin\objdump.exe" -p $binary
        if ($LASTEXITCODE -ne 0) { throw "No se pudieron leer dependencias: $binary" }
        foreach ($line in $headers) {
            if ($line -match 'DLL Name:\s*(\S+)') {
                $dll = $Matches[1]
                $source = Join-Path "$Toolchain\bin" $dll
                if ((Test-Path -LiteralPath $source) -and $seen.Add($dll)) {
                    Copy-Item -LiteralPath $source -Destination "dist\DoomPathfinding\$dll" -Force
                    $pending.Enqueue($source)
                }
            }
        }
    }
    Copy-Item -LiteralPath 'README.md' -Destination 'dist\DoomPathfinding\LEEME.md' -Force
    # Preserve the available runtime license notices with the portable build.
    Copy-Item -LiteralPath "$Toolchain\share\licenses" -Destination 'dist\DoomPathfinding' -Recurse -Force
    Compress-Archive -Path 'dist\DoomPathfinding\*' -DestinationPath 'dist\DoomPathfinding-Windows.zip' -Force
    Write-Host 'Listo: dist\DoomPathfinding\juego.exe'
} finally { Pop-Location }

