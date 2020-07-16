@echo off

call "C:\Program Files\qemu\qemu-system-x86_64.exe" -drive file=image.fat,index=0,media=disk,format=raw -bios OVMF-pure-efi.fd -net none