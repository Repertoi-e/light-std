REM @Platform

@echo off

REM In order to call this script inside Visual Studio, you need to launch devenv from vcvarsall.bat console...
REM Otherwise Visual Studio can't find msbuild.exe.

del bin\Debug-windows-x86_64\game\*.pdb >NUL

REM We don't check for the dll write time unless the buildlock file exists, 
REM because in rare cases the handle used for checking for the write file
REM makes it so the compiler can't write to the dll.
type nul>bin\Debug-windows-x86_64\game\buildlock
call msbuild.exe /NOLOGO /VERBOSITY:minimal light-std.sln /t:%1 /p:Configuration="Debug" /p:Platform="x64" /p:BuildProjectReferences=true
del bin\Debug-windows-x86_64\game\buildlock >NUL

echo Done compiling game.
