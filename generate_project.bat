REM @Platform

@echo off

call vendor\Windows\premake5.exe vs2019


for /R %%f in (.\*.vcxproj) do (
  set /p val=<%%f
  call :MakeReplace "stdcpp17"  "stdcpplatest" %%f
  echo "Replacing stdcpp17 with stdcpplatest in file: %%f"
)

exit /b 

:FindReplace <findstr> <replstr> <file>
set tmp="%temp%\tmp.txt"
If not exist %temp%\_.vbs call :MakeReplace
for /f "tokens=*" %%a in ('dir "%3" /s /b /a-d /on') do (
  for /f "usebackq" %%b in (`Findstr /mic:"%~1" "%%a"`) do (
    echo(&Echo Replacing "%~1" with "%~2" in file %%~nxa
    <%%a cscript //nologo %temp%\_.vbs "%~1" "%~2">%tmp%
    if exist %tmp% move /Y %tmp% "%%~dpnxa">nul
  )
)
del %temp%\_.vbs
exit /b

:MakeReplace
echo ^(Get-Content "%3"^) ^| ForEach-Object { $_ -replace %1, %2 } ^| Set-Content %3>Rep.ps1
Powershell.exe -executionpolicy remotesigned -File Rep.ps1
if exist Rep.ps1 del Rep.ps1
pause
