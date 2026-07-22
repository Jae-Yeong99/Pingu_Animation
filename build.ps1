$ErrorActionPreference = 'Stop'

$compiler = Get-Command g++.exe -ErrorAction Stop
New-Item -ItemType Directory -Force -Path 'build' | Out-Null

& $compiler.Source -std=c++17 -O2 -Wall -Wextra -mwindows `
    'src\main.cpp' -o 'build\DesktopPet.exe' `
    -lgdi32 -luser32 -lshell32 -lgdiplus

New-Item -ItemType Directory -Force -Path 'build\assets' | Out-Null
Copy-Item -LiteralPath 'assets\penguin_walk.png' -Destination 'build\assets\penguin_walk.png' -Force

Write-Host 'Built build\DesktopPet.exe'
