@echo off

if "%1"=="" (
    set configuration=Debug
) else (
    set configuration=%1
)

if "%2"=="" (
    set target=vs2022
) else (
    set target=%2
)

echo  %~dp0..\build\%target%\light-std.sln 

msbuild %~dp0..\build\%target%\light-std.sln -verbosity:minimal -nologo -property:Configuration=%configuration%