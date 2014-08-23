# Microsoft Developer Studio Project File - Name="libvrml97" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libvrml97 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libvrml97.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libvrml97.mak" CFG="libvrml97 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libvrml97 - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libvrml97 - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "libvrml97 - Win32 ReleaseDebug" (based on\
 "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe

!IF  "$(CFG)" == "libvrml97 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "YY_NEVER_INTERACTIVE" /D "WRL2MA" /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD LIB32 /out:"..\lib\libvrml97.lib"

!ELSEIF  "$(CFG)" == "libvrml97 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "YY_NEVER_INTERACTIVE" /D "WRL2MA" /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD LIB32 /out:"..\lib\libvrml97.lib"

!ELSEIF  "$(CFG)" == "libvrml97 - Win32 ReleaseDebug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "vrml97___Wi"
# PROP BASE Intermediate_Dir "vrml97___Wi"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseDebug"
# PROP Intermediate_Dir "ReleaseDebug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "YY_NEVER_INTERACTIVE" /D "WRL2MA" /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD LIB32 /out:"..\lib\libvrml97.lib"

!ENDIF 

# Begin Target

# Name "libvrml97 - Win32 Release"
# Name "libvrml97 - Win32 Debug"
# Name "libvrml97 - Win32 ReleaseDebug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\source\Audio.cpp
# End Source File
# Begin Source File

SOURCE=..\source\Doc.cpp
# End Source File
# Begin Source File

SOURCE=..\source\gifread.c
# End Source File
# Begin Source File

SOURCE=..\source\HOOPSEvent.c
# End Source File
# Begin Source File

SOURCE=..\source\Image.cpp
# End Source File
# Begin Source File

SOURCE=..\source\jpgread.c
# End Source File
# Begin Source File

SOURCE=..\source\lexer.cpp
# End Source File
# Begin Source File

SOURCE=..\source\MathUtils.cpp
# End Source File
# Begin Source File

SOURCE=..\source\OpenGLEvent.c
# End Source File
# Begin Source File

SOURCE=..\source\parser.cpp
# End Source File
# Begin Source File

SOURCE=..\source\pngread.c
# End Source File
# Begin Source File

SOURCE=..\source\ScriptJS.cpp
# End Source File
# Begin Source File

SOURCE=..\source\ScriptObject.cpp
# End Source File
# Begin Source File

SOURCE=..\source\Sound.c
# End Source File
# Begin Source File

SOURCE=..\source\System.cpp
# End Source File
# Begin Source File

SOURCE=..\source\Viewer.cpp
# End Source File
# Begin Source File

SOURCE=..\source\ViewerGlut.cpp
# End Source File
# Begin Source File

SOURCE=..\source\ViewerGtk.cpp
# End Source File
# Begin Source File

SOURCE=..\source\ViewerHOOPS.cpp
# End Source File
# Begin Source File

SOURCE=..\source\ViewerOpenGL.cpp
# End Source File
# Begin Source File

SOURCE=..\source\ViewerXt.cpp
# End Source File
# Begin Source File

SOURCE=..\source\VrmlField.cpp
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNamespace.cpp
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNode.cpp
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeAnchor.cpp
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeAppearance.cpp
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeAudioClip.cpp
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeBackground.cpp
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeBillboard.cpp
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeBox.cpp
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeCollision.cpp
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeColor.cpp
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeColorInt.cpp
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeCone.cpp
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeCoordinate.cpp
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeCoordinateInt.cpp
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeCylinder.cpp
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeCylinderSensor.cpp
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeDirLight.cpp
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeElevationGrid.cpp
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeExtrusion.cpp
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeFog.cpp
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeFontStyle.cpp
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeGeometry.cpp
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeGroup.cpp
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeIFaceSet.cpp
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeILineSet.cpp
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeImageTexture.cpp
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeIndexedSet.cpp
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeInline.cpp
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeLight.cpp
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeLOD.cpp
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeMaterial.cpp
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeMovieTexture.cpp
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeNavigationInfo.cpp
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeNormal.cpp
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeNormalInt.cpp
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeOrientationInt.cpp
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodePixelTexture.cpp
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodePlaneSensor.cpp
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodePointLight.cpp
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodePointSet.cpp
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodePositionInt.cpp
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeProto.cpp
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeProximitySensor.cpp
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeScalarInt.cpp
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeScript.cpp
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeShape.cpp
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeSound.cpp
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeSphere.cpp
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeSphereSensor.cpp
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeSpotLight.cpp
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeSwitch.cpp
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeText.cpp
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeTextureCoordinate.cpp
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeTextureTransform.cpp
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeTimeSensor.cpp
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeTouchSensor.cpp
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeTransform.cpp
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeType.cpp
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeViewpoint.cpp
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeVisibilitySensor.cpp
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeWorldInfo.cpp
# End Source File
# Begin Source File

SOURCE=..\source\VrmlScene.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\source\Audio.h
# End Source File
# Begin Source File

SOURCE=..\source\config.h
# End Source File
# Begin Source File

SOURCE=..\source\Doc.h
# End Source File
# Begin Source File

SOURCE=..\source\gifread.h
# End Source File
# Begin Source File

SOURCE=..\source\HOOPSEvent.h
# End Source File
# Begin Source File

SOURCE=..\source\Image.h
# End Source File
# Begin Source File

SOURCE=..\source\jpgread.h
# End Source File
# Begin Source File

SOURCE=..\source\MathUtils.h
# End Source File
# Begin Source File

SOURCE=..\source\OpenGLEvent.h
# End Source File
# Begin Source File

SOURCE=..\source\parser.h
# End Source File
# Begin Source File

SOURCE=..\source\pngread.h
# End Source File
# Begin Source File

SOURCE=..\source\ScriptJS.h
# End Source File
# Begin Source File

SOURCE=..\source\ScriptObject.h
# End Source File
# Begin Source File

SOURCE=..\source\Sound.h
# End Source File
# Begin Source File

SOURCE=..\source\System.h
# End Source File
# Begin Source File

SOURCE=..\source\Viewer.h
# End Source File
# Begin Source File

SOURCE=..\source\ViewerGlut.h
# End Source File
# Begin Source File

SOURCE=..\source\ViewerGtk.h
# End Source File
# Begin Source File

SOURCE=..\source\ViewerHOOPS.h
# End Source File
# Begin Source File

SOURCE=..\source\ViewerOpenGL.h
# End Source File
# Begin Source File

SOURCE=..\source\ViewerX11.h
# End Source File
# Begin Source File

SOURCE=..\source\ViewerXt.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlEvent.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlField.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlMFColor.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlMFFloat.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlMFInt.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlMFNode.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlMFRotation.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlMFString.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlMFVec2f.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlMFVec3f.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNamespace.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNode.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeAnchor.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeAppearance.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeAudioClip.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeBackground.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeBillboard.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeBindable.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeBox.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeChild.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeCollision.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeColor.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeColorInt.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeCone.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeCoordinate.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeCoordinateInt.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeCylinder.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeCylinderSensor.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeDirLight.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeElevationGrid.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeExtrusion.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeFog.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeFontStyle.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeGeometry.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeGroup.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeIFaceSet.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeILineSet.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeImageTexture.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeIndexedSet.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeInline.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeLight.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeLOD.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeMaterial.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeMovieTexture.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeNavigationInfo.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeNormal.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeNormalInt.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeOrientationInt.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodePixelTexture.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodePlaneSensor.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodePointLight.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodePointSet.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodePositionInt.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeProto.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeProximitySensor.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeScalarInt.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeScript.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeShape.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeSound.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeSphere.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeSphereSensor.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeSpotLight.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeSwitch.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeText.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeTexture.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeTextureCoordinate.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeTextureTransform.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeTimeSensor.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeTouchSensor.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeTransform.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeType.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeViewpoint.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeVisibilitySensor.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlNodeWorldInfo.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlScene.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlSFBool.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlSFColor.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlSFFloat.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlSFImage.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlSFInt.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlSFNode.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlSFRotation.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlSFString.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlSFTime.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlSFVec2f.h
# End Source File
# Begin Source File

SOURCE=..\source\VrmlSFVec3f.h
# End Source File
# End Group
# End Target
# End Project
