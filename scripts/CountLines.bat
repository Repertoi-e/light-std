@echo off
call %~dp0..\ThirdParty\bin-x86\cloc\cloc-1.64.exe src -force-lang="C++",cppm %*

