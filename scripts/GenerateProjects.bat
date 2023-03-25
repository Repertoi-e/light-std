@echo off

if "%1"=="" (
    set target="vs2022"
) else (
    set target=%1
)

call %~dp0..\ThirdParty\bin-x86\premake\premake5.exe target

cscript %~dp0PatchProjectFiles.vbs target
