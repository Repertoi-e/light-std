@echo off
call %~dp0..\ThirdParty\bin\cloc\cloc-1.64.exe src -force-lang="C++",cppm %*

