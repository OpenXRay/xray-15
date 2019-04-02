md res\
md res\bin\
md res\gamedata\

cd resources\
copy * ..\res\gamedata\
cd ..\

if %CONFIGURATION%==Debug if %PLATFORM%==x86 goto :DX86
if %CONFIGURATION%==Debug if %PLATFORM%==x64 goto :DX64
if %CONFIGURATION%==Mixed if %PLATFORM%==x86 goto :MX86
if %CONFIGURATION%==Mixed if %PLATFORM%==x64 goto :MX64
if %CONFIGURATION%==Release if %PLATFORM%==x86 goto :RX86
if %CONFIGURATION%==Release if %PLATFORM%==x64 goto :RX64

echo ! Unknown configuration and/or platform
goto :EOF

:DX86
cd ..\bin\Win32\Debug
call :COPY_ENGINE
7z a OpenXRay.ClearSky.Dx86.7z .\*

cd ..\bin\Win32\Debug
call :COPY_SYMBOLS
7z a OpenXRay.ClearSky.Symbols.Dx86.7z .\*
goto :EOF

:DX64
cd ..\bin\Win64\Debug
call :COPY_ENGINE
7z a OpenXRay.ClearSky.Dx64.7z .\*

cd ..\bin\Win64\Debug
call :COPY_SYMBOLS
7z a OpenXRay.ClearSky.Symbols.Dx64.7z .\*
goto :EOF

:MX86
cd ..\bin\Win32\Mixed
call :COPY_ENGINE
7z a OpenXRay.ClearSky.Mx86.7z .\*

cd ..\bin\Win32\Mixed
call :COPY_SYMBOLS
7z a OpenXRay.ClearSky.Symbols.Mx86.7z .\*
goto :EOF

:MX64
cd ..\bin\Win64\Mixed
call :COPY_ENGINE
7z a OpenXRay.ClearSky.Mx64.7z .\*

cd ..\bin\Win64\Mixed
call :COPY_SYMBOLS
7z a OpenXRay.ClearSky.Symbols.Mx64.7z .\*
goto :EOF

:RX86
cd ..\bin\Win32\Release
call :COPY_ENGINE
7z a OpenXRay.ClearSky.Rx86.7z .\*

cd ..\bin\Win32\Release
call :COPY_SYMBOLS
7z a OpenXRay.ClearSky.Symbols.Rx86.7z .\*
goto :EOF

:RX64
cd ..\bin\Win64\Release
call :COPY_ENGINE
7z a OpenXRay.ClearSky.Rx64.7z .\*

cd ..\bin\Win64\Release
call :COPY_SYMBOLS
7z a OpenXRay.ClearSky.Symbols.Rx64.7z .\*
goto :EOF

:COPY_ENGINE
copy *.dll ..\..\..\cs\res\bin\
copy *.exe ..\..\..\cs\res\bin\
cd ..\..\..\

copy Licence.txt .\cs\res\
copy README.md .\cs\res\
cd res\
goto :EOF

:COPY_SYMBOLS
copy *.pdb ..\..\..\res\bin\
cd ..\..\..\
copy Licence.txt .\res\
copy README.md .\res\
cd res\
goto :EOF
