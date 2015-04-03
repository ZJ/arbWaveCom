gcc.exe -Wall -o .\builds\win32\genAWGpattern.exe src\genBinary\genBinary.c src\driver.c
@echo OFF
if %errorlevel% neq 0 pause && exit /b %errorlevel%
