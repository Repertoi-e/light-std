REM @Platform

@echo off

call vendor\Windows\premake5.exe vs2019 

call python generate_project_post.py

