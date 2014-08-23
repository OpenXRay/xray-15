/**********************************************************************
*<
FILE: IDxMaterial.h

DESCRIPTION:	Interface for accessing the bitmaps used by effects in the DxMaterial

CREATED BY:		Neil Hazzard

HISTORY:		Created:  07/08/03


*>	Copyright (c) 2003, All Rights Reserved.
**********************************************************************/
#include "iFnPub.h"

#ifndef	__IDXMATERIAL_H__
#define __IDXMATERIAL_H__

#define IDXMATERIAL_INTERFACE Interface_ID(0x55b3201d, 0x29ab7fc3)

//!An interface class to manage access to various parameters used by DirectX effects.
/*!
 The Effect file has many internal presentations and as such the DxMaterial does not always store a 
 one to one look up.  Many parameters are stored in the ParamBlock and can be accessed that way.  The 
 paramblock index for the Effect is located at 0 (the material maintains 5 ParamBlocks).  However things like Lights
 and Bitmaps do not get stored in the paramblock in an ideal way for data access.  This interface simplifies this data
 access by providing direct access to these elements.
 */
class IDxMaterial : public FPMixinInterface
{

public:

	typedef enum LightSemantics {
		LIGHT_COLOR,
		LIGHT_DIRECTION,
		LIGHT_POSITION,
	}LightSemantics;

	//!Reloads the current active effect
	virtual void ReloadDXEffect()=0;

	//!Returns the number of bitmaps used by the currently loaded effect
	/*!
	\return The number of bitmaps
	*/
	virtual int GetNumberOfEffectBitmaps()=0;

	//!Gets the bitmap used by the effect 
	/*!
	\param index The index of the bitmap to retrieve
	\return A PBBitmap pointer for the bitmap used
	*/
	virtual PBBitmap * GetEffectBitmap(int index)=0;

	//!Set the bitmap used by the effect
	/*!
	\param index The index of the bitmap to set
	\param * bmap A PBBitmap pointer for the bitmap to set
	*/
	virtual void SetEffectBitmap(int index,PBBitmap * bmap)=0;

	//!Get the Dx Effect file
	/*!This can also be accessed via the paramblock, but it is simply provided for completeness
	\return The effect file in use
	*/
	virtual TCHAR*  GetEffectFilename()=0;

	//!Set the Dx Effect file
	/*!This can also be accessed via the paramblock, but it is simply provided for completeness
	\param filename The effect file to set
	*/
	virtual void SetEffectFilename(TCHAR * filename)=0;

	//! Get the bitmap used for the software rendering overrride
	/*! This can also be set by the Paramblock, it is just used for completeness
	return The Bitmap used
	*/
	virtual PBBitmap * GetSoftwareRenderBitmap() = 0;

	//!Set the bitmap to be used by the Renderer.
	/*! This can also be set by the Paramblock, it is just used for completeness
	\param *bmap A PBBitmap specifiying the bitmap to use
	*/
	virtual void SetSoftwareRenderBitmap(PBBitmap * bmap) = 0;

	//************************************************************************************
	// The following method are not exposed through function publishing
	//************************************************************************************

	//!Get the number of light based parameters
	/*!This will return the number of parameters that are light based, even Light Color.  Care needs to be taken with Light Color
	as it could also have a Color Swatch associated with it, so could already have been exported as part of the ParamBlock.  
	\return The number of light based parameters
	*/
	virtual int GetNumberOfLightParams()=0;

	//!The actual node used by the parameter
	/*!This represent the light node used by the parameter.  Care needs to taken as this could be a NULL pointer.  There are two reason for this.
	The first is that the LightColor Semantic is stored internally as a LightElement, but the writer of the Effect file may not have specified 
	a parentID 	for the light, this will result in a NULL.  Secondly if the user has not actually assigned a light via the UI drop down list,
	then again this will again result in a NULL pointer.
	\param index The index of the light to return.
	\return The INode for the active light.
	*/
	virtual INode * GetLightNode(int index)=0;

	//!The name of the parameter in the Effect file
	/*
	 \param index The index of the light to retrieve
	 \return A TCHAR* containing the name
	 */
	virtual TCHAR * GetLightParameterName(int index)=0;

	//!The light semantic as defined in the effect file
	/*
	 \param index THe index of the light to retrieve	
	 \return The semantic represented as a LightSemantics
	 */
	virtual LightSemantics GetLightSemantic(int index)=0;

};

#endif