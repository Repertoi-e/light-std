REM @Platform

@echo off

call ..\third_party\premake\premake5.exe vs2022

cscript PatchProjectFiles.vbs
