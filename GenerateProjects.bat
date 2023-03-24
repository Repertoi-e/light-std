REM @Platform

@echo off

call ThirdParty\bin\premake\premake5.exe vs2022

cscript PatchProjectFiles.vbs
