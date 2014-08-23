/******************************************************************************
 * Copyright (C) 1986-2007 mental images GmbH, Fasanenstr. 81, D-10623 Berlin,
 * Germany. All rights reserved.
 * Portions Copyright (C) 1997-2007 Alias Systems Corp.
 ******************************************************************************
// Copyright (C) 1997-2006 Autodesk, Inc., and/or its licensors.
// All rights reserved.
//
// The coded instructions, statements, computer programs, and/or related
// material (collectively the "Data") in these files contain unpublished
// information proprietary to Autodesk, Inc. ("Autodesk") and/or its licensors,
// which is protected by U.S. and Canadian federal copyright law and by
// international treaties.
//
// The Data is provided for use exclusively by You. You have the right to use,
// modify, and incorporate this Data into other products for purposes authorized
// by the Autodesk software license agreement, without fee.
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND. AUTODESK
// DOES NOT MAKE AND HEREBY DISCLAIMS ANY EXPRESS OR IMPLIED WARRANTIES
// INCLUDING, BUT NOT LIMITED TO, THE WARRANTIES OF NON-INFRINGEMENT,
// MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, OR ARISING FROM A COURSE
// OF DEALING, USAGE, OR TRADE PRACTICE. IN NO EVENT WILL AUTODESK AND/OR ITS
// LICENSORS BE LIABLE FOR ANY LOST REVENUES, DATA, OR PROFITS, OR SPECIAL,
// DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES, EVEN IF AUTODESK AND/OR ITS
// LICENSORS HAS BEEN ADVISED OF THE POSSIBILITY OR PROBABILITY OF SUCH DAMAGES.
*/
 
 This directory contains mental ray shader examples
 which utilize Maya specific information provided by mayabase.

Directory Layout:

	mayaShaderExample.mi
		This file contains declarations of all shaders
		included in the examples.

	mayaShaderExample_32bit.sln or mayaShaderExample_64bit.sln
	mayaShaderExample_32bit.vcproj or mayaShaderExample_64bit.vcproj
		The Visual Studio 2005 solution files
		to build the examples on Windows
		32 or 64-bit (depending on platform).

	mayaShaderExample.xcodeproj
		The XCode 2 project file
		to build the examples on Mac OS X.

	GNUmakefile
		The makefile for GNU make (gmake)
		to build the examples on any Linux.

	emitLight/
	lightlink/
		Directories with shader source code
		and Maya UI scripts for example shaders.

Example Features:

- Utilize Maya light linking in custom shaders

    mental ray for maya plugin access light linking relationship
    using Maya API (MLightLinks) and generates a global light-object
    table. This table is translated into the mental ray light linking
    format, and so light linking is handled external to shaders.
    There is no longer any need to query the light linking relationships
    through a special shader as in previous versions (new in Maya 2008).
    See example in "lightlink" directory.

- Utilize Maya shader state in custom shaders

    Maya specific information is stored in the regular shader state
    appended as user data. Shaders can access this information using
    the mayapi function mayabase_stateitem_get(). The full list of
    available items is listed in the header file mayaapi.h.
    The directory "emitLight" contains one example of using
    mayabase_stateitem_get() to set and retrieve "emit diffuse" and
    "emit specular" parameters on light shaders. They are typically
    provided by all Maya light shaders. As an example, a standard
    mental ray light shader has been extended to provide the same
    parameters.


Building The Shaders:

    There are project files for Windows and MacOSX development tools
    and a Linux GNUmakefile to build the final shader binaries for the
    target platform (mayaShaderExample.dll/.so) almost automatically.
    The projects were setup to compile the shaders as 'release' versions.
    A 'debug' build is prepared in the project files as well, it builds
    a debuggable version of the shaders that will successfully run with
    the release version of Maya within a debugger, to investigate
    the shader execution at runtime. Check the configuration settings in
    Visual Studio / XCode before building to select the configuration
    needed. For the XCode project, the shaders will be compiled as 'release'.
    versions by default. For Visual Studio, it might not be the case. It
    depends on last selected configuration which is saved in the *.suo 
    files (Solution User Options).

    The XCode project was set to target platform 'Current Mac OS'.
    For building universal binaries, a simple switch in the 'project
    inspector' general page needs to be changed to retarget the 
    'Cross-Develop Using Target SDK' to 'Mac OS X 10.4 (Universal)'. 

    Please note the special complications on Windows systems when
    introducing interdependencies between .dll libraries (in this
    case mayaShaderExample.dll is calling functions from mayabase.dll).
    This will require additional build and runtime steps as following:
        Upon building, the example shader library need to link the
    library mayabase.lib (in addition to shader.lib for the mental ray
    core functions) to resolve the function references in the linking
    phase (the VC project perform this automatically).
        Furthermore, upon loading the example shader library into Maya,
    the path to the mayabase.dll library needs to be known to the system,
    typically achieved by adding the directory to the PATH environment
    variable. It is recommended to do this in the Maya.env file of the
    user's local preferences directory, by adding an entry like this:

        PATH = $MAYA_LOCATION/mentalray/lib;$PATH

    It prefaces the PATH with the directory where mayabase.dll is
    installed (note the Unix-style syntax which is supported by Maya
    on all platforms including Windows). In case of problems the
    example shaders won't load into Maya, and the output window
    will print an error about
    'can't load...library, The specified module could not be found'.


Loading Shaders in Maya:

1. Register the UI scripts

    The shaders come with predefined UI scripts for Maya. These script
    files should be copied to any of the known Maya script directories
    (user local or Maya global).
    As an alternative, the environment variable MAYA_SCRIPT_PATH can
    be set to list the additional script directories prior to starting
    Maya. The recommended and most portable way is to add an entry to
    Maya.env like this:

        MAYA_SCRIPT_PATH = <path to local directory>


2. Load the shader declaration

    Within Maya, open Windows/Rendering Editors/mental ray/Shader Manager
    and use 'browse' to load the mayaShaderExample.mi declaration file
    (NOT the .dll or .so). The .dll or .so binary should be stored in
    the same directory so that the Shader Manager is able to find and
    load it automatically.
    A new entry should appear in the 'manually loaded' section of the
    Shader Manager window. The 'info' box will list a valid 'library'
    entry if the binary was indeed loaded successfully.


3. Prepare a scene and render

    The new shading nodes should appear in various menu sets of Maya,
    like Lighting/Shading/Assign New Material, as well as in Hypershade.
    These nodes can be assigned as usual, for example by drag'n'drop
    icons or establishing attribute connections in the attribute editor.


4. Prepare for auto-loading

    If the shader development phase had been finished and final versions
    of shaders will be used on a regular basis, they can be added to the
    default set of auto-loaded packages of the Shader Manager. This can
    be achieved either
    - by copying the .mi and .dll/.so files to the global repository
      for mental ray for Maya standard shaders (to enable the shaders
      for all users at once), or
    - by setting the environment variable MI_CUSTOM_SHADER_PATH to
      point to the files or directories, preferably in Maya.env
      (see Maya documentation for details).


5. Troubleshooting

    Scenes containing custom nodes should be saved in Maya ASCII format,
    unless custom node IDs have been assigned (see Maya docs for details).
    These ID numbers are utilized in binary Maya scene files to identify
    node types.
    If rendering with custom shaders didn't succeed, the script editor or
    console/output window should be checked for error or warning messages,
    which should help to identify problems. Check environment variables
    from within Maya via 'getenv' MEL command, to verify correct settings
    performed in Maya.env or in system scripts.

