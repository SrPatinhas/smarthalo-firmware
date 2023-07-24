
@echo off
cls
setlocal EnableDelayedExpansion
set "cmd=findstr /R /N "^^" *.h | find /C ":""

for /f %%a in ('!cmd!') do set number=%%a

echo %number%

set "cmd=findstr /R /N "^^" *.c | find /C ":""

for /f %%a in ('!cmd!') do set number=%%a

echo %number%
pause