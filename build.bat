@echo off

start /b /wait "" "C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/MSBuild/Current/Bin/MSBuild.exe" "./lstd/lstd.vcxproj" /v:m /nologo /p:Configuration=Debug /p:Platform=x64

start /b /wait "" "C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/MSBuild/Current/Bin/MSBuild.exe" "./lstd-graphics/lstd-graphics.vcxproj" /v:m /nologo /p:Configuration=Debug /p:Platform=x64

start /b /wait "" "C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/MSBuild/Current/Bin/MSBuild.exe" "./test-suite/test-suite.vcxproj" /v:m /nologo /p:Configuration=Debug /p:Platform=x64
