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

- Using Maya shader state

Maya specific shader state items are accessible through 
the function mayabase_stateitem_get(). The function takes
variable size arguments, and is terminated with MBSI_NULL.
The function returns pointers to the specified items, and
allows to access the values directly. For example,

miInteger *val1, *val2, *val3;
mayabase_stateitem_get(state,
	MBSI_LIGHTAMBIENT, &val1,
	MBSI_LIGHTDIFFUSE, &val2,
	MBSI_LIGHTSPECULAR, &val3,
	MBSI_NULL
	);

val1, val2, val3 points to the shader state items
storing emit ambient, emit diffuse, emit specular 
information, respectively. val1, val2, val3 can be
used either to set or to read the shader state values.

The structure MbStateItem in mayaapi.h shows 
the full list of shader state items available.


- emit diffuse, emit specular

A light shader may set MBSI_LIGHTAMBIENT, MBSI_LIGHTDIFFUSE, and
MBSI_LIGHTSPECULAR item in the shader state to inform other shaders
that this light emits ambient, diffuse and specular lights. Other
shaders, material shader for instance, may read these values to
compute the final color.

See examples in this directory.

maya_light_point modifies mib_light_point to set MBSI_LIGHTDIFFUSE
and MBSI_LIGHTSPECULAR. maya_illum_phong modifies mib_illum_phong
to read MBSI_LIGHTDIFFUSE and MBSI_LIGHTSPECULAR. Diffuse component
is added only if MBSI_LIGHTDIFFUSE is set.

Note that maya_light_point got two extra parameters.

AETemplate files are provided.

