@echo OFF
.\builds\win32\genAWGpattern.exe
if %errorlevel% neq 0 pause && exit /b %errorlevel%
@echo ON
"C:\Program Files\PuTTY\plink.exe" -load AWG < testFile_points
