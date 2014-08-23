# Microsoft Developer Studio Project File - Name="vrml2" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=vrml2 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "vrml2.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "vrml2.mak" CFG="vrml2 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "vrml2 - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "vrml2 - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "vrml2 - Win32 ReleaseDebug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "vrml2 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\..\Products\MDt\Release"
# PROP Intermediate_Dir "..\..\..\Products\MDt\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "." /I "..\include" /I "$(AW_REPO_PRODUCTS_DIR)\buildRelease\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "NT_ENV" /D "NT_PLUGIN" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib libMDtAPI.lib Foundation.lib OpenMaya.lib OpenMayaAnim.lib Image.lib /nologo /subsystem:windows /dll /machine:I386 /out:"..\..\..\Products\buildRelease\bin\plug-ins\vrml2Export.mll" /libpath:"$(AW_REPO_PRODUCTS_DIR)\lib\Release" /export:initializePlugin /export:uninitializePlugin
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "vrml2 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\..\Products\MDt\Debug"
# PROP Intermediate_Dir "..\..\..\Products\MDt\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /I "." /I "..\include" /I "$(AW_REPO_PRODUCTS_DIR)\buildDebug\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "NT_ENV" /D "NT_PLUGIN" /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib libMDtAPId.lib Foundationd.lib OpenMayad.lib OpenMayaAnimd.lib Imaged.lib /nologo /subsystem:windows /dll /debug /machine:I386 /nodefaultlib:"LIBCMTD.lib" /out:"..\..\..\Products\buildDebug\bin\plug-ins\vrml2Export.mll" /pdbtype:sept /libpath:"$(AW_REPO_PRODUCTS_DIR)\lib\Debug" /export:initializePlugin /export:uninitializePlugin
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "vrml2 - Win32 ReleaseDebug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ReleaseDebug"
# PROP BASE Intermediate_Dir "ReleaseDebug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\..\Products\MDt\ReleaseDebug"
# PROP Intermediate_Dir "..\..\..\Products\MDt\ReleaseDebug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /I "d:\MDt\vrml2" /D "WIN32" /D "NDEBUG" /D "_BOOL" /D "_WINDOWS" /D "NT_ENV" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O2 /I "." /I "..\include" /I "$(AW_REPO_PRODUCTS_DIR)\buildRelease\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "NT_ENV" /D "NT_PLUGIN" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib libMDtAPI.lib Foundation.lib OpenMaya.lib OpenMayaAnim.lib Image.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"..\..\..\Products\buildRelease\bin\plug-ins\vrml2Export.mll" /pdbtype:sept /libpath:"$(AW_REPO_PRODUCTS_DIR)\lib\ReleaseDebug" /export:initializePlugin /export:uninitializePlugin
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "vrml2 - Win32 Release"
# Name "vrml2 - Win32 Debug"
# Name "vrml2 - Win32 ReleaseDebug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cc;cxx;c++"
# Begin Source File

SOURCE=.\vrml2Anim.cpp
# End Source File
# Begin Source File

SOURCE=.\vrml2Export.cpp
# End Source File
# Begin Source File

SOURCE=.\vrml2Translator.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\vrml2Export.h
# End Source File
# End Group
# Begin Group "Mel Scripts"

# PROP Default_Filter "mel"
# Begin Source File

SOURCE=.\vrml2Tags.mel

!IF  "$(CFG)" == "vrml2 - Win32 Release"

# Begin Custom Build
InputPath=.\vrml2Tags.mel

"..\..\..\Products\buildRelease\scripts\others\vrml2Tags.mel" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy .\vrml2Tags.mel ..\..\..\Products\buildRelease\scripts\others

# End Custom Build

!ELSEIF  "$(CFG)" == "vrml2 - Win32 Debug"

# Begin Custom Build
InputPath=.\vrml2Tags.mel

"..\..\..\Products\buildDebug\scripts\others\vrml2Tags.mel" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy .\vrml2Tags.mel ..\..\..\Products\buildDebug\scripts\others

# End Custom Build

!ELSEIF  "$(CFG)" == "vrml2 - Win32 ReleaseDebug"

# Begin Custom Build
InputPath=.\vrml2Tags.mel

"..\..\..\Products\buildRelease\scripts\others\vrml2Tags.mel" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy .\vrml2Tags.mel ..\..\..\Products\buildRelease\scripts\others

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\vrml2TranslatorOpts.mel

!IF  "$(CFG)" == "vrml2 - Win32 Release"

# Begin Custom Build
InputPath=.\vrml2TranslatorOpts.mel

"..\..\..\Products\buildRelease\scripts\others\vrml2TranslatorOpts.mel" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy .\vrml2TranslatorOpts.mel ..\..\..\Products\buildRelease\scripts\others

# End Custom Build

!ELSEIF  "$(CFG)" == "vrml2 - Win32 Debug"

# Begin Custom Build
InputPath=.\vrml2TranslatorOpts.mel

"..\..\..\Products\buildDebug\scripts\others\vrml2TranslatorOpts.mel" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy .\vrml2TranslatorOpts.mel ..\..\..\Products\buildDebug\scripts\others

# End Custom Build

!ELSEIF  "$(CFG)" == "vrml2 - Win32 ReleaseDebug"

# Begin Custom Build
InputPath=.\vrml2TranslatorOpts.mel

"..\..\..\Products\buildRelease\scripts\others\vrml2TranslatorOpts.mel" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy .\vrml2TranslatorOpts.mel ..\..\..\Products\buildRelease\scripts\others

# End Custom Build

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
