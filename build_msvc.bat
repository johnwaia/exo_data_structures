@echo off
setlocal enabledelayedexpansion

set "VCVARS=C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat"

if not exist "!VCVARS!" (
    echo vcvars64.bat introuvable a l'emplacement attendu :
    echo   !VCVARS!
    exit /b 1
)

call "!VCVARS!" >nul

cl /W4 /O2 /nologo benchmark.c dynamic_array.c hash_table.c linked_list.c /Fe:benchmark.exe
if errorlevel 1 (
    echo Compilation echouee.
    exit /b 1
)

echo.
echo Compilation reussie : benchmark.exe
endlocal
