@echo off
setlocal
rem Build helper that works even if run from another directory.
set SCRIPT_DIR=%~dp0
set SRC_FILE=%SCRIPT_DIR%src\main.cpp
set OUTPUT=%SCRIPT_DIR%flightsim.exe

if not exist "%SRC_FILE%" (
  echo [error] Could not find %SRC_FILE%. Make sure the repository is intact.
  exit /b 1
)

echo [info] Building flightsim from %SRC_FILE%
if exist "%ProgramFiles%\Git\usr\bin\g++.exe" (
  rem Git Bash mingw path (common when using Git for Windows)
  "%ProgramFiles%\Git\usr\bin\g++" -std=c++17 "%SRC_FILE%" -o "%OUTPUT%"
) else if exist "%ProgramFiles%\mingw-w64\bin\g++.exe" (
  "%ProgramFiles%\mingw-w64\bin\g++" -std=c++17 -O2 -static "%SRC_FILE%" -o "%OUTPUT%"
) else (
  rem Fallback to cl if available in Developer Command Prompt
  cl /std:c++17 /O2 /EHsc "%SRC_FILE%" /Fe:"%OUTPUT%"
)

echo [done] Built %OUTPUT%
endlocal
