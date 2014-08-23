/******************************************************************************
 * Copyright (C) 1986-2005 mental images GmbH, Fasanenstr. 81, D-10623 Berlin,
 * Germany. All rights reserved.
 * Portions Copyright (C) 1997-2005 Alias Systems Corp.
 ******************************************************************************
/*
// Copyright (C) Alias Systems, a division of Silicon Graphics Limited and/or
// its licensors ("Alias").  All rights reserved.  These coded instructions,
// statements, computer programs, and/or related material (collectively, the
// "Material") contain unpublished information proprietary to Alias, which is
// protected by Canadian and US federal copyright law and by international
// treaties.  This Material may not be disclosed to third parties, or be
// copied or duplicated, in whole or in part, without the prior written
// consent of Alias.  ALIAS HEREBY DISCLAIMS ALL WARRANTIES RELATING TO THE
// MATERIAL, INCLUDING, WITHOUT LIMITATION, ANY AND ALL EXPRESS OR IMPLIED
// WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.  IN NO EVENT SHALL ALIAS BE LIABLE FOR ANY DAMAGES
// WHATSOEVER, WHETHER DIRECT, INDIRECT, SPECIAL, OR PUNITIVE, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, OR IN EQUITY,
// ARISING OUT OF OR RELATED TO THE ACCESS TO, USE OF, OR RELIANCE UPON THE
// MATERIAL.
*/

Using maya light linking inside mental ray shader

- How light linking relationship is represented
mental ray for maya plugin access light linking relationship
using Maya API (MLightLinks)
and generates a global light-object table.
Shaders can access this table by using 
mayabase_lightlink_get and mayabase_lightlink_check.

The source file mayaLightlink_baselambert.c
modifies the existing shader mib_illum_lambert
so that maya light linking is respected.

The shader declaration file mayashaderExample.mi
has shader declaration of :
	mayaLightlink_mib_illum_lambert
	
mayaLightlink_mib_illum_lambert
has explicit light array parameter
as well as lightLink parameter.
The shader also has a boolean parameter "miLightLink",
which is explained below.

- automatic light linking
mentalray for maya 6.0 and later versions 
automatically connect lights to the light array of a shader
based on the light-object relationship on rendering time.
( You can check this in The exported .mi file. )

Thus, it is not necessary to access 
maya light linking information in the shader level,
if the shader has a light array parameter
as mayaLightlink_mib_illum_lambert do.

Note : If the shader has light list array,
and user makes light list connection explicitly,
the automatic light linking is disabled.


However, if you want to handle the light linking in the shader level,
and the shader has light array parameter,
a few more extra steps should be done
besides modifying the shader code.

1. Disable automatic light linking by adding a dynamic attribute to your shader.
   For example,

   addAttr -ln "miLightLink" -at bool mayaLightlink_mib_illum_lambert1;

   If the value of this attribute is 0, 
   mentalray for maya will not connect lights to the light array for this shader.
   mayaLightlink_mib_illum_lambert has this attribute as static,
   and default value is 0,
   so that the shader uses maya style light linking by default.


2. Set light mode to 2
   so that all the lights in the scene are checked against the light linking.
   For example, 
   setAttr "mayaLightlink_mib_illum_lambert1.mode" 2;

3. Connect lightLinker node(s) to the shader.
   For example, 
   connectAttr lightLinker1.message mayaLightlink_mib_illum_lambert1.lightLink;
   
AETemplate file for mayaLightlink_mib_illum_lambert is provided.
Note that "mode" and "lights" are hidden in the UI.
Enabling "Use Automatic Light Link" check box 
sets "miLightLink" to 1, 
and enables automatic light linking.
It also sets "mode" to 1,
and disconnect lightLinker,
so that automatic light linking works properly.
Disabling "Use Automatic Light Link" check box
sets "miLightLink" to 1, 
and disables automatic light linking.
"mode" is set to 2,
so that light linking relationship is used properly.
User has to connect light linker node manually.

Toggling "miLightLink" should not have any difference,
provided that lightLinker node is properly connected,
and the shader is not shared among multiple objects with different light linking.

The automatic light linking does not handle
multiple instances because it could be expensive.

If the shader is assigned to multiple objects with different light linking,
setting "miLightLink" to 0 and connecting the lightLinker node
will produce the correct result.



