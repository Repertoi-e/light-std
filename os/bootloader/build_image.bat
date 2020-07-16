REM @Platform

@echo off

del image.fat

REM We use the Dist configuration to build the image
call uefi-run.exe --size 100 ..\..\bin\Dist-windows-x86_64\os\x64_boot.efi