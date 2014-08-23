# Microsoft Developer Studio Project File - Name="wrl2ma" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=wrl2ma - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "wrl2ma.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "wrl2ma.mak" CFG="wrl2ma - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "wrl2ma - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "wrl2ma - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "wrl2ma - Win32 ReleaseDebug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "wrl2ma - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\..\Products\vrml97ToMayaASCII\Release"
# PROP Intermediate_Dir "..\..\..\Products\vrml97ToMayaASCII\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "." /I "../source" /I "$(AW_REPO_PRODUCTS_DIR)\buildRelease\include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ../../../Products/vrml97ToMayaASCII/Release/libvrml97.lib /nologo /subsystem:console /machine:I386 /out:"..\..\..\Products\buildRelease\bin\wrl2ma.exe"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=T:\bin\perl.exe -I T:\lib\perl      ..\..\OpenMaya\MakeNT\releaseDevkit.pl vrml97ToMayaASCII games\vrml97ToMaya Release
# End Special Build Tool

!ELSEIF  "$(CFG)" == "wrl2ma - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\..\Products\vrml97ToMayaASCII\Debug"
# PROP Intermediate_Dir "..\..\..\Products\vrml97ToMayaASCII\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "." /I "../source" /I "$(AW_REPO_PRODUCTS_DIR)\buildDebug\include" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ../../../Products/vrml97ToMayaASCII/Debug/libvrml97.lib /nologo /subsystem:console /debug /machine:I386 /out:"..\..\..\Products\buildDebug\bin\wrl2ma.exe" /pdbtype:sept
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=T:\bin\perl.exe -I T:\lib\perl     ..\..\OpenMaya\MakeNT\releaseDevkit.pl vrml97ToMayaASCII games\vrml97ToMaya Debug
# End Special Build Tool

!ELSEIF  "$(CFG)" == "wrl2ma - Win32 ReleaseDebug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ReleaseDebug"
# PROP BASE Intermediate_Dir "ReleaseDebug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\..\Products\vrml97ToMayaASCII\ReleaseDebug"
# PROP Intermediate_Dir "..\..\..\Products\vrml97ToMayaASCII\ReleaseDebug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "." /I "../source" /I "$(AW_REPO_PRODUCTS_DIR)\buildRelease\include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ../../../Products/vrml97ToMayaASCII/ReleaseDebug/libvrml97.lib /nologo /subsystem:console /machine:I386 /out:"..\..\..\Products\buildRelease\bin\wrl2ma.exe"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=T:\bin\perl.exe -I T:\lib\perl     ..\..\OpenMaya\MakeNT\releaseDevkit.pl vrml97ToMayaASCII games\vrml97ToMaya ReleaseDebug
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "wrl2ma - Win32 Release"
# Name "wrl2ma - Win32 Debug"
# Name "wrl2ma - Win32 ReleaseDebug"
# Begin Source File

SOURCE=.\AmEdge.cpp
# End Source File
# Begin Source File

SOURCE=.\AmEdge.h
# End Source File
# Begin Source File

SOURCE=.\AmEdgeList.cpp
# End Source File
# Begin Source File

SOURCE=.\AmEdgeList.h
# End Source File
# Begin Source File

SOURCE=.\wrl2ma.cpp
# End Source File
# End Target
# End Project
