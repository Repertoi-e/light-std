REM @Platform

@echo off

call ..\third_party\premake\premake5.exe vs2019

cscript PatchProjectFiles.vbs
