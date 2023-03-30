@echo off

if "%1"=="" (
    set target="vs2022"
) else (
    set target=%1
)

echo  %~dp0..\build\%target%\light-std.sln 

msbuild %~dp0..\build\%target%\light-std.sln -verbosity:minimal -nologo