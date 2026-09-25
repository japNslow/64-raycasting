@echo off
setlocal

rem Check if cl65 is in PATH, otherwise try C:\cc65\bin\cl65.exe
where cl65 >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    set CC65=cl65
) else if exist C:\cc65\bin\cl65.exe (
    set CC65=C:\cc65\bin\cl65.exe
) else (
    echo Error: cl65.exe not found! Please add cc65\bin to PATH or install to C:\cc65.
    exit /b 1
)

echo [1/2] Generating trigonometry and projection tables...
python generate_tables.py

echo [2/2] Compiling raycast.prg for Commodore 64...
%CC65% -t c64 -O -o raycast.prg src\tables.c src\map.c src\raycast.c src\render.c src\main.c

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ======================================================
    echo Build successful! Created raycast.prg
    echo Run with VICE emulator: x64sc.exe raycast.prg
    echo ======================================================
) else (
    echo.
    echo Build failed!
    exit /b 1
)
