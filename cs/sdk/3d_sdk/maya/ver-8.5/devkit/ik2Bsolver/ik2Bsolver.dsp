# Microsoft Developer Studio Project File - Name="ik2Bsolver" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=ik2Bsolver - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ik2Bsolver.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ik2Bsolver.mak" CFG="ik2Bsolver - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ik2Bsolver - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ik2Bsolver - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ik2Bsolver - Win32 ReleaseDebug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "ik2Bsolver"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ik2Bsolver - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\..\Products\SuperConductors\Release\ik2Bsolver"
# PROP Intermediate_Dir "..\..\..\Products\SuperConductors\Release\ik2Bsolver"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /Zi /O2 /I "." /I "$(AW_REPO_PRODUCTS_DIR)\buildRelease\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_BOOL" /D "STRICT" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O2 /I "." /I "$(AW_REPO_PRODUCTS_DIR)\buildRelease\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_BOOL" /D "STRICT" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib Foundation.lib OpenMaya.lib OpenMayaAnim.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../../../Products/buildRelease/bin/plug-ins/ik2Bsolver.mll" /libpath:"$(AW_REPO_PRODUCTS_DIR)\lib\releaseDebug" /export:initializePlugin /export:uninitializePlugin /export:MApiVersion
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib Foundation.lib OpenMaya.lib OpenMayaAnim.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../../../Products/buildRelease/bin/plug-ins/ik2Bsolver.mll" /libpath:"$(AW_REPO_PRODUCTS_DIR)\lib\releaseDebug" /export:initializePlugin /export:uninitializePlugin /export:MApiVersion
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "ik2Bsolver - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\..\Products\SuperConductors\Debug"
# PROP Intermediate_Dir "..\..\..\Products\SuperConductors\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "." /I "$(AW_REPO_PRODUCTS_DIR)\buildDebug\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_BOOL" /D "STRICT" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib Foundationd.lib OpenMayad.lib OpenMayaAnimd.lib /nologo /subsystem:windows /dll /pdb:"../../../Products/SuperConductors\buildDebug/ik2Bsolver.pdb" /debug /machine:I386 /out:"../../../Products/buildDebug/bin/plug-ins/ik2Bsolver.mll" /libpath:"$(AW_REPO_PRODUCTS_DIR)\lib\Debug" /export:initializePlugin /export:uninitializePlugin /export:MApiVersion
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
SOURCE=$(InputPath)
PostBuild_Cmds=copy ik2Bsolver.mel\
   $(AW_REPO_PRODUCTS_DIR)\buildDebug\scripts\others\ik2Bsolver.mel
# End Special Build Tool

!ELSEIF  "$(CFG)" == "ik2Bsolver - Win32 ReleaseDebug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ik2Bsolv"
# PROP BASE Intermediate_Dir "ik2Bsolv"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\..\Products\SuperConductors\ReleaseDebug"
# PROP Intermediate_Dir "..\..\..\Products\SuperConductors\ReleaseDebug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_BOOL" /D "STRICT" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O2 /I "." /I "$(AW_REPO_PRODUCTS_DIR)\buildRelease\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_BOOL" /D "STRICT" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib Foundation.lib OpenMaya.lib OpenMayaAnim.lib /nologo /subsystem:windows /dll /pdb:"../../../Products/SuperConductors\releaseDebug\ik2Bsolver.pdb" /machine:I386 /out:"../../../Products/buildRelease/bin/plug-ins/ik2Bsolver.fso"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib Foundation.lib OpenMaya.lib OpenMayaAnim.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../../../Products/buildRelease/bin/plug-ins/ik2Bsolver.mll" /libpath:"$(AW_REPO_PRODUCTS_DIR)\lib\releaseDebug" /export:initializePlugin /export:uninitializePlugin /export:MApiVersion
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
SOURCE=$(InputPath)
PostBuild_Cmds=copy ik2Bsolver.mel\
   $(AW_REPO_PRODUCTS_DIR)\buildRelease\scripts\others\ik2Bsolver.mel
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "ik2Bsolver - Win32 Release"
# Name "ik2Bsolver - Win32 Debug"
# Name "ik2Bsolver - Win32 ReleaseDebug"
# Begin Source File

SOURCE=.\AwMath.h
# End Source File
# Begin Source File

SOURCE=.\AwMatrix.cpp
# End Source File
# Begin Source File

SOURCE=.\AwMatrix.h
# End Source File
# Begin Source File

SOURCE=.\AwPoint.cpp
# End Source File
# Begin Source File

SOURCE=.\AwPoint.h
# End Source File
# Begin Source File

SOURCE=.\AwQuaternion.cpp
# End Source File
# Begin Source File

SOURCE=.\AwQuaternion.h
# End Source File
# Begin Source File

SOURCE=.\AwVector.cpp
# End Source File
# Begin Source File

SOURCE=.\AwVector.h
# End Source File
# Begin Source File

SOURCE=.\ik2Bsolver.cpp
# End Source File
# End Target
# End Project
