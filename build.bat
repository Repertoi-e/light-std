@echo off

start /b /wait "" "C:\Program Files (x86)\Microsoft Visual Studio\2019\Preview\Msbuild\Current\Bin\MSBuild.exe" "./light-std.sln" /v:m /nologo /p:Configuration=Debug /p:Platform=x64 /m
