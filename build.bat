@echo off

start /b /wait "" "C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/MSBuild/Current/Bin/MSBuild.exe" "./light-std.sln" /v:m /nologo /p:Configuration=DebugTests /p:Platform=x64 /m
