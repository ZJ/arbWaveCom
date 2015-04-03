gcc.exe -Wall -o .\builds\win32\genAWGpattern.exe genBinary.c driver.c
@echo OFF
if %errorlevel% neq 0 pause && exit /b %errorlevel%
