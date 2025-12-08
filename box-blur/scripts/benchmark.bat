@echo off
REM Automated Benchmark - Windows Version
REM Run this from Git Bash or WSL

echo ======================================
echo   Box Blur Performance Benchmark
echo ======================================
echo.

if "%~1"=="" (
    echo Usage: benchmark.bat photo.jpg
    echo.
    echo Example: benchmark.bat myimage.jpg
    exit /b 1
)

set INPUT_IMAGE=%~1

if not exist "%INPUT_IMAGE%" (
    echo Error: Image '%INPUT_IMAGE%' not found!
    exit /b 1
)

echo Running benchmark in WSL...
echo.

wsl bash -c "cd '%CD%' && ./benchmark.sh '%INPUT_IMAGE%'"

echo.
echo Opening results...
start results\performance_graph.png
start results\performance_summary.txt

echo.
echo Done! Check the results folder.
pause
