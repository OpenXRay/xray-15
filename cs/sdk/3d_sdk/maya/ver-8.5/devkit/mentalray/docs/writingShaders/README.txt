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
 
 This directory contains mental ray shader examples
 that uses maya specific information.
 
 - mayaShaderExample.mi
 This file contains declaration of shaders included in the examples.
 
 - light link relationship
mental ray for maya plugin access light linking relationship
using Maya API (MLightLinks)
and generates a global light-object table.
Shaders can access this table by using 
mayabase_lightlink_get and mayabase_lightlink_check.
See the example shader in "lightlink" directory.

- shader state
Some maya specific information is stored in the shader state.
Shaders can access this information using
mayabase_stateitem_get.
The full list of shader state items is in mayaapi.h.
"emitLight" contains one example of using mayabase_stateitem_get
to get "emit diffuse" and "emit specular" values of the light.
