@echo off
set OLDPATH=%PATH%
set OLDINCLUDE=%INCLUDE%
set OLDLIB=%LIB%

@cd "%~dp0"

rem Detect Visual studio if no environment is set
if not "%VCINSTALLDIR%"=="" goto vc_found
if "%VS80COMNTOOLS%"=="" goto no_vs_found
call "%VS80COMNTOOLS%vsvars32.bat"
goto vc_found

:no_vs_found
echo WARNING : No Visual Studio installation found.
echo Known versions :
echo   - Visual Studio 2005 (%%VS80COMNTOOLS%%)
if not exist build goto end

echo.
echo Fallback on internal compilation engine.
set PATH=%CD%\build\drive_c\Program Files\Microsoft Visual Studio 8\VC\bin;%CD%\build\drive_c\Program Files\Microsoft Visual Studio 8\Common7\IDE;%PATH%
set INCLUDE=%CD%\build\drive_c\Program Files\Microsoft Visual Studio 8\VC\INCLUDE;%CD%\build\drive_c\Program Files\Microsoft Visual Studio 8\VC\PlatformSDK\Include
set LIB=%CD%\build\drive_c\Program Files\Microsoft Visual Studio 8\VC\lib;%CD%\build\drive_c\Program Files\Microsoft Visual Studio 8\VC\PlatformSDK\Lib
set DXSDK_DIR=%CD%\build\drive_c\Program Files\Microsoft DirectX SDK

:vc_found
if not "%DXSDK_DIR%"=="" goto dx_found
echo WARNING : No DirectX SDK found.

:dx_found
set INCLUDE=%INCLUDE%;%CD%\include;%DXSDK_DIR%\Include
set LIB=%LIB%;%DXSDK_DIR%\Lib\x86;%CD%\lib

:startcompilation
cd ..

set PATH=%CD%\win32\msys\bin;%CD%\win32\msys\mingw\bin;%PATH%
set WIN32=1
set UNICODE=1

sh
rem Reset environment
cd win32

:end
set PATH=%OLDPATH%
set LIB=%OLDLIB%
set INCLUDE=%OLDINCLUDE%
pause

echo ========================== Build process finished ==========================
COLOR 00
