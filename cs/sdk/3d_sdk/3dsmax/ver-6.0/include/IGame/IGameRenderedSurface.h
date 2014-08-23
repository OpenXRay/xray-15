/**********************************************************************
 *<
	FILE: IGameRenderedSurface.h

	DESCRIPTION: IGameRenderSurface interfaces for IGame.  This interface
	provide access to the lighting and color of an object at the face level.
	Any IGame Object can provide access to the data calculated here.  It uses
	the same basic approach as the Vertex Color Utility

	CREATED BY: Neil Hazzard, Discreet 

	HISTORY: created 12/11/02

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/

#ifndef __IGAMERENDERSURFACE__H
#define __IGAMERENDERSURFACE__H

#pragma once

/*!\file IGameRenderedSurface.h
\brief Access to the main render routines to calculate surface data such as Lighting and color.

Lighting and color can be calculated on a per face/vertex level.  This data is the same technique that
the Apply Vertex color utility uses.  The difference here is that you are provided the color independently of the
vertex color channel.  THis leaves that channel for other data and provides more options at export time.
*/


#define EVAL_MAPS		(1<<0)
#define EVAL_SHADOWS	(1<<1)

//!class IGameRenderedSurface
/*! Access to the main render surface interface.  Methods are available to setup the system and define the lighting models
used.  Actual access to the data is provided through the IGameMesh class
*/
class IGameRenderedSurface
{
public:
	

	enum LightingModel{
		SHADED,
		LIGHTING_ONLY,
		SHADED_ONLY
	};

	//! Setup the database for rendering.
	virtual bool InitialiseRenderedSurfaceData() = 0;

	//! Clean it all up
	virtual void ReleaseRenderedSurfaceData() = 0 ;

	//! Define the type of model be used in the render
	virtual void SetLightingModel(LightingModel lm) = 0;

	//! Defines the usage of Maps and shadows
	virtual void SetFlag(DWORD mask) = 0;

	//! defines the usgae of Maps and shadows
	virtual void ClearFlag(DWORD mask) = 0;
};

//! Get the main interface pointer.
IGAMEEEXPORT IGameRenderedSurface *GetIGameRenderedSurfaceInterface();

#endif
