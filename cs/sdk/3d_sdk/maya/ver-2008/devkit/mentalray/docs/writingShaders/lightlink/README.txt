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

Using Maya light linker inside mental ray shader

- How light linking relationship is represented

	The mental ray for Maya plugin accesses light linking
	relationship using Maya API (MLightLinks) and
	generates a global light-object table.

	Previously, shaders had to access this table (by using 
	mayabase_lightlink_get and mayabase_lightlink_check) to
	query light linking information.

	Now, the Maya light linking relationships are translated
	into the native mental ray light linking format, and
	so the mental ray core handles light linking automatically.
	Shaders no longer have to handle this.

The source file maya_illum_lambert.cpp modifies the
existing shader mib_illum_lambert shader so that
Maya light linking is respected. This shader has now been
updated to comment out the sections that are no longer
necessary now that native mental ray light linking is
supported.

The following information is no longer relevant now that
there is support for native mental ray light linking, however
it is maintained here for historical purposes.

##################################################################

The shader maya_illum_lambert has both
	- the explicit light array parameter and
	- the lightLink parameter for lightlink shader connection.
The shader also has a boolean parameter "miLightLink", see below.

- Automatic light linking

	mental ray for Maya 6.0 and later versions automatically
	onnect lights to the light array of a shader based on
	the light-object relationship on rendering time.
	( You can check this in the exported .mi file. )

	Thus, stricly speaking, it is not necessary to access
	Maya light linking information on the shader level, if
	the shader has a light array parameter as many standard
	shaders like maya_illum_lambert have. On the other hand,
	the plug-in does not resolve cases where the same shader
	is attached to several objects which have different light
	link relations (which would require to copy the shader
	onto every object with different light array settings).
	This is where light linking in the shader is beneficial.

	Note : If the shader has a light list array with explicit
	connections of lights to it, then the automatic light linking
	is not performed.

To perform light linking on the	shader level, there is no
need to the light array parameter any more. However, to
support both types of light linking side by side a few more
extra steps should be done besides modifying the shader code.

1. Disable automatic light linking in the plug-in by adding
   a dynamic attribute to your shader node within Maya.
   For example,

   addAttr -ln "miLightLink" -at bool maya_illum_lambert1;

   If the value of this attribute is 0, mental ray for Maya
   will not connect lights to the light array for this shader.
   The example shader maya_illum_lambert has this attribute
   as static, and its default value is 0, so that the shader
   uses Maya style light linking by default.

2. Set light mode to 2
   so that all the lights in the scene are initially considered
   for lighting, and filtered by light linking to the actual set
   of linked lights.
   For example, 
   setAttr "maya_illum_lambert1.mode" 2;

3. Connect lightLinker node(s) to the shader.
   For example, 
   connectAttr lightLinker1.message maya_illum_lambert1.lightLink;
   This is performed by the example AETemplate file automatically.

The AETemplate file for maya_illum_lambert is provided:
Enabling "Use Automatic Light Link" check box sets "miLightLink" to 1, 
and enables automatic light linking. It also sets "mode" to 1, and
disconnects the lightLinker node, so that automatic light linking
works properly.
Disabling "Use Automatic Light Link" check box sets "miLightLink" to 0,
and disables automatic light linking. The "mode" is set to 2, so that
light linking relationship is used properly. The light linker node
will be connected automatically.

