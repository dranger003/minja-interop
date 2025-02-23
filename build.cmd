@echo off
if "%~1"=="" (
    echo Usage: build.cmd [debug^|release]
    exit /b 1
)

if /i "%1"=="debug" (
    cmake -S . -B build && cmake --build build --config Debug
) else if /i "%1"=="release" (
    cmake -S . -B build && cmake --build build --config Release
) else (
    echo Invalid build configuration. Use "debug" or "release".
    exit /b 1
)
