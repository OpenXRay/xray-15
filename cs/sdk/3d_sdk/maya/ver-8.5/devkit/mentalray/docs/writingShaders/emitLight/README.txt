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

- Using maya shader state

maya specific shader state items are accessible through 
mayabase_stateitem_get.
mayabase_stateitem_get takes varialbe size arguments,
which should terminate with MBSI_NULL.
The function returns pointers to the specified items.
For example,

miInteger *val1, *val2, *val3;
mayabase_stateitem_get(state,
	MBSI_LIGHTAMBIENT, &val1,
	MBSI_LIGHTDIFFUSE, &val2,
	MBSI_LIGHTSPECULAR, &val3,
	MBSI_NULL
	);
	
val1, val2, val3 points to the shader state items
storing emit ambient, emit diffuse, emit specular 
information respectively.

val1, val2, val3 can be used either to set or to read 
the shader state values.

MbStateItem in mayaapi.h shows 
the full list of shader state items available.

- emit diffuse, emit specular

A light shader may set 
MBSI_LIGHTAMBIENT, MBSI_LIGHTDIFFUSE, MBSI_LIGHTSPECULAR
item in the shader state
to indicate other shaders that
this light emits ambient, diffuse and specular lights.
Other shaders, material shader for instance,
may read these values to compute the final color.

See examples in this directory.

mayaEmitLight_mib_light_point modifies mib_light_point
to set MBSI_LIGHTDIFFUSE and MBSI_LIGHTSPECULAR.

mayaEmitLight_mib_illum_phong modifies mib_illum_phong
to read MBSI_LIGHTDIFFUSE and MBSI_LIGHTSPECULAR.
Diffuse component is added only if MBSI_LIGHTDIFFUSE is set.

mayaShaderState_base.mi
contains declaration for these shaders.
Note that mayaEmitLight_mib_light_point has two extra parameters.

AETemplate files are provided.





