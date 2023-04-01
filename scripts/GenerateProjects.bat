@echo off

if "%1"=="" (
    set target="vs2022"
) else (
    set target=%1
)

call %~dp0..\third-party\bin\premake\premake5.exe %target% %2
