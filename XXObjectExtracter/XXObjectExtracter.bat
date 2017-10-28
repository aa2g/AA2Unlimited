::@echo off
set aaunlimited_directory=%~dp0
set filename_with_extension=%1
set filename=%filename_with_extension:~0, -3%

set /p object=Enter object name:
::call XXObjectExtracter.exe %1 %object% %filename%-%object%.xxo
pause