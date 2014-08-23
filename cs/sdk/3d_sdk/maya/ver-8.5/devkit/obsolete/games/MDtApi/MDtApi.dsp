# Microsoft Developer Studio Project File - Name="MDtAPI" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=MDtAPI - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "MDtAPI.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "MDtAPI.mak" CFG="MDtAPI - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "MDtAPI - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "MDtAPI - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "MDtAPI - Win32 ReleaseDebug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "MDtApi"
# PROP Scc_LocalPath ".."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "MDtAPI - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\..\Products\MDt\Release"
# PROP Intermediate_Dir "..\..\..\Products\MDt\Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "." /I "..\include" /I "$(AW_REPO_PRODUCTS_DIR)\buildRelease\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "NT_ENV" /D "NT_PLUGIN" /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\Products\lib\Release\libMDtAPI.lib"

!ELSEIF  "$(CFG)" == "MDtAPI - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\..\Products\MDt\Debug"
# PROP Intermediate_Dir "..\..\..\Products\MDt\Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /I "." /I "..\include" /I "$(AW_REPO_PRODUCTS_DIR)\buildDebug\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "NT_ENV" /D "NT_PLUGIN" /YX /FD /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\Products\lib\Debug\libMDtAPId.lib"

!ELSEIF  "$(CFG)" == "MDtAPI - Win32 ReleaseDebug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ReleaseDebug"
# PROP BASE Intermediate_Dir "ReleaseDebug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\..\Products\MDt\ReleaseDebug"
# PROP Intermediate_Dir "..\..\..\Products\MDt\ReleaseDebug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /Zi /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /ZI /Od /I "." /I "..\include" /I "$(AW_REPO_PRODUCTS_DIR)\buildRelease\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "NT_ENV" /D "NT_PLUGIN" /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\Products\lib\ReleaseDebug\libMDtAPI.lib"

!ENDIF 

# Begin Target

# Name "MDtAPI - Win32 Release"
# Name "MDtAPI - Win32 Debug"
# Name "MDtAPI - Win32 ReleaseDebug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cc;cxx;c++"
# Begin Source File

SOURCE=.\iffreader.cpp
# End Source File
# Begin Source File

SOURCE=.\iffwriter.cpp
# End Source File
# Begin Source File

SOURCE=.\MDtCamera.cpp
# End Source File
# Begin Source File

SOURCE=.\MDtCnetwork.cpp
# End Source File
# Begin Source File

SOURCE=.\MDtLayer.cpp
# End Source File
# Begin Source File

SOURCE=.\MDtLight.cpp
# End Source File
# Begin Source File

SOURCE=.\MDtMaterial.cpp
# End Source File
# Begin Source File

SOURCE=.\MDtShape.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=..\include\iffreader.h
# End Source File
# Begin Source File

SOURCE=..\include\iffwriter.h
# End Source File
# Begin Source File

SOURCE=..\include\MDt.h
# End Source File
# Begin Source File

SOURCE=.\MDtCamera.h
# End Source File
# Begin Source File

SOURCE=..\include\MDtExt.h
# End Source File
# Begin Source File

SOURCE=.\MDtLight.h
# End Source File
# End Group
# End Target
# End Project
