@echo off
REM -- First make map file from Microsoft Visual C++ generated resource.h
echo // MAKEHELP.BAT generated Help Map file.  Used by SDKAPWZ.HPJ. >"hlp\SDKAPWZ.hm"
echo. >>"hlp\SDKAPWZ.hm"
echo // Dialogs (IDD_*) >>"hlp\SDKAPWZ.hm"
makehm IDD_,HIDD_,0x20000 resource.h >>"hlp\SDKAPWZ.hm"
echo. >>"hlp\SDKAPWZ.hm"

REM -- Make help for Project SDKAPWZ

start /wait hcw /C /E /M "SDKAPWZ.hpj"
if %errorlevel% == 1 goto :Error
if exist Debug\nul copy "SDKAPWZ.hlp" Debug
if exist Release\nul copy "SDKAPWZ.hlp" Release
if exist SDKAPWZ___Win32_Debug\nul copy "SDKAPWZ.hlp" SDKAPWZ___Win32_Debug
goto :done

:Error
echo SDKAPWZ.hpj(1) : error: Problem encountered creating help file

:done
echo.
