@echo off
set TL_PATH=d:\maibu\gcc-arm-none-eabi-4_9-2015q2-20150609-win32\bin
set PACK_PATH=D:\maibu\Pack\

mkdir build

echo ������Դ�ļ�...
%PACK_PATH%\pack.exe
copy .\build\maibu_res.h .\src\maibu_res.h
del tmp.log
del .\build\maibu_code
del OUT.maibu

echo ���ڱ���...
%TL_PATH%\arm-none-eabi-gcc -c -mthumb -mcpu=cortex-m0 .\src\*.c -I%PACK_PATH%\include\
%TL_PATH%\arm-none-eabi-ld -r .\*.o -o .\build\maibu_code
%TL_PATH%\arm-none-eabi-readelf -a .\build\maibu_code> elf.log

del *.o
echo ���ڴ��...
%PACK_PATH%\pack.exe
pause
