# Microsoft Developer Studio Project File - Name="mayaClockServer" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=mayaClockServer - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "mayaClockServer.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "mayaClockServer.mak" CFG="mayaClockServer - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "mayaClockServer - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "mayaClockServer - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "mayaClockServer - Win32 ReleaseDebug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "mayaClockServer - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\..\Products\MotionCapture\Release\mayaClockServer"
# PROP Intermediate_Dir "..\..\..\Products\MotionCapture\Release\mayaClockServer"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "../include" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "DEVEL" /D _WIN32_WINNT=0x400 /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib ws2_32.lib libMocap.lib /nologo /subsystem:console /machine:I386 /out:"..\..\..\Products\buildrelease\bin\mayaClockServer.exe" /libpath:"$(AW_REPO_PRODUCTS_DIR)\lib\Release"

!ELSEIF  "$(CFG)" == "mayaClockServer - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\..\Products\MotionCapture\Debug\mayaClockServer"
# PROP Intermediate_Dir "..\..\..\Products\MotionCapture\Debug\mayaClockServer"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /I "..\include" /D "_DEBUG" /D "DEVEL" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D _WIN32_WINNT=0x400 /D "SERVER_SIDE_RECORDING" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib ws2_32.lib libMocapd.lib /nologo /subsystem:console /debug /machine:I386 /out:"..\..\..\Products\BuildDebug\bin\mayaClockServer.exe" /libpath:"$(AW_REPO_PRODUCTS_DIR)\lib\Debug"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=T:\bin\perl.exe -I T:\lib\perl     ..\..\OpenMaya\MakeNT\releaseDevkit.pl MotionCapture\clock mocap Debug
# End Special Build Tool

!ELSEIF  "$(CFG)" == "mayaClockServer - Win32 ReleaseDebug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "mayaCloc"
# PROP BASE Intermediate_Dir "mayaCloc"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\..\Products\MotionCapture\ReleaseDebug\mayaClockServer"
# PROP Intermediate_Dir "..\..\..\Products\MotionCapture\ReleaseDebug\mayaClockServer"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /I "../include" /I "../../motioncapture/include" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "DEVEL" /D _WIN32_WINNT=0x400 /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O2 /I "../include" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "DEVEL" /D _WIN32_WINNT=0x400 /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib ws2_32.lib /nologo /subsystem:console /machine:I386 /out:"..\..\..\Products\buildrelease\bin\mayaClockServer.exe"
# ADD LINK32 kernel32.lib user32.lib gdi32.lib ws2_32.lib libMocap.lib /nologo /subsystem:console /debug /machine:I386 /out:"..\..\..\Products\buildrelease\bin\mayaClockServer.exe" /libpath:"$(AW_REPO_PRODUCTS_DIR)\lib\ReleaseDebug"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=T:\bin\perl.exe -I T:\lib\perl     ..\..\OpenMaya\MakeNT\releaseDevkit.pl MotionCapture\clock mocap ReleaseDebug
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "mayaClockServer - Win32 Release"
# Name "mayaClockServer - Win32 Debug"
# Name "mayaClockServer - Win32 ReleaseDebug"
# Begin Source File

SOURCE=.\mayaClockServer.c
# End Source File
# End Target
# End Project
