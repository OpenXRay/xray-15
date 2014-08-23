/**********************************************************************
 *<
	FILE: IGameMaterial.h

	DESCRIPTION: Material interfaces for IGame

	CREATED BY: Neil Hazzard, Discreet

	HISTORY: created 02/02/02

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/
/*!\file IGameMaterial.h
\brief IGame material and texture Interfaces.
*/
#ifndef __IGAMEMATERIAL__H
#define __IGAMEMATERIAL__H
#pragma once

#include "max.h"
#include "stdmat.h"
#include "IGameProperty.h"

// copied here from stdmat.h -- this is just to allow IGame to be self contained
#define ID_AM 0   // ambient
#define ID_DI 1   // diffuse
#define ID_SP 2   // specular
#define ID_SH 3   // shininesNs
#define ID_SS 4   // shininess strength
#define ID_SI 5   // self-illumination
#define ID_OP 6   // opacity
#define ID_FI 7   // filter color
#define ID_BU 8   // bump 
#define ID_RL 9   // reflection
#define ID_RR 10  // refraction 
#define ID_DP 11  // displacement

class IGameBitmapTex;
class IGameTextureMap;
class IGameUVGen;

//!Simple wrapper for max materials
/*! IGameMaterial An IGame Wrapper around a basic Material.  It provides access to the basic material properties and 
Bitmap Textures used by the material.  Any material will be wrapped in this class, but only Standard Material is directly supported
with API access to the properties.  If the material is not directly supported then the data can be access via the IPropertyContainer
interface.
\sa IGameProperty, IGameScene
*/
class IGameMaterial : public IExportEntity
{
public:
	//! Is the material a Multi Material type - This could be for a Blend or Mix material
	/*!
	\return TRUE is Multi material
	*/
	virtual bool IsMultiType()=0;

	//! Is the material a SubObject style Multi Material  This could be for Max's Multi Subobject material
	/*!
	\return TRUE is a Subobject material
	*/
	virtual bool IsSubObjType()=0;

	//! The material name as sence in the Material Editor
	/*!
	\return the name of the material
	*/
	virtual TCHAR * GetMaterialName()=0;

	//! The number of sub materials this material maintains
	/*! The value is used by IGameScene::GetSubMaterial
	\return The number of Sub material
	\sa GetSubMaterial
	*/
	virtual int GetSubMaterialCount()=0;

	//! Access to any sub material.
	/*! The sub material is any material used by a multi material For example, a Top/Bottom material the sub materials
	would be the top and bottom
	\param index Index into the submaterial
	\return A Pointer to a IGameMaterial
	\sa IGameMaterial, IGameScene::GetRootMaterial
	*/
	virtual IGameMaterial * GetSubMaterial(int index)  =0;

	//! For subobject materials get the MatID for the actual material.  
	/*! This value represents the MatID used on objects to define what faces receive this material
	\param subIndex The index of the submaterial to retrieve 
	\return The MatID of the material
	*/
	virtual int GetMaterialID(int subIndex)=0;

	//! Get the Ambient Data
	/*!
	\return A pointer to IGameProperty
	*/
	virtual IGameProperty *  GetAmbientData()=0;

	//! Get the Diffuse Data
	/*!
	\return A pointer to IGameProperty
	*/
	virtual IGameProperty *  GetDiffuseData()=0;

	//! Get the Emissive Data
	/*!
	\return A pointer to IGameProperty
	*/
	virtual IGameProperty *  GetEmissiveData()=0;

	//! Get the Specular Data
	/*!
	\return A pointer to IGameProperty
	*/
	virtual IGameProperty *  GetSpecularData()=0;

	//! Get the Opacity Data
	/*!
	\return A pointer to IGameProperty
	*/
	virtual IGameProperty *  GetOpacityData()=0;

	//! Get the Glossiness Data
	/*!
	\return A pointer to IGameProperty
	*/
	virtual IGameProperty *  GetGlossinessData()=0;

	//! Get the Specular Level Data
	/*!
	\return A pointer to IGameProperty
	*/
	virtual IGameProperty *  GetSpecularLevelData()=0;

	//! Get the Emissive Amount Data
	/*!
	\return A pointer to IGameProperty
	*/
	virtual IGameProperty *  GetEmissiveAmtData()=0;

	//! Get the number of Textures used by the material
	/*!
	\return The texture count.
	*/
	virtual int GetNumberOfTextureMaps()=0;

	//!Access to the actual Texture Map
	/*!
	\param index The index to the  Texture Map to retrieve
	\return A pointer to a IGameTextureMap
	*/
	virtual IGameTextureMap * GetIGameTextureMap(int index)  =0;

	//! Access to the actual Max material definition
	/*! This allows developer access to the complete max object if further data access is needed
	\return A pointer to a standard max Mtl class
	\sa IGameBitmapTex
	*/
	virtual Mtl * GetMaxMaterial()=0;


};


//!Simple wrapper for max textures
/*! A genric class that wraps all the max texture maps.  This class directly supports the Bitmap Texture.  This can be
tested for by calling IsObjectSupported.  If it is not supported then access to the paramblocks, if defined can be obtained
by using the properties interface. The usual texture map properties including coordinate rollout access are provide here
*/

class IGameTextureMap : public IExportEntity
{
public:

	/*! The name of the TextureMap as seen in the material editor/material browser.
	\returns The name of the texture map
	*/
	virtual TCHAR * GetTextureName() = 0;


	//!Provide access to the actual max definition
	/*! This allows the developer to get hold of extra data such as Texture Transforms, specified from the 
	Coordinates rollout.  You can use the max method of GetUVGen or GetXYZGen for more advanced access
	\return A pointer to a max class Texamp
	*/
	virtual Texmap * GetMaxTexmap() = 0;
	
	//!Access to the Coordinate Rollout
	/*!If the developer needs access to the transforms applied to the texture, then this can be accessed here.
	\returns A pointer to IGameUVGen
	*/
	virtual IGameUVGen * GetIGameUVGen()=0;

	//! This returns the slot that the bitmap was found in.  
	/*! It uses the standard Max convention ID_BU  for bump etc..  If this is -1 then it means either the hosting material 
	was  not a standard material and the channel conversion could not be performed based on the active shader.
	\return the Slot definition.  This can be -1 signifying an unsupported material.
	*/
	virtual int GetStdMapSlot()=0;

	//! The filename of the bitmap used by the Bitmap Texture
	/*!
	\return The name of bitmap file
	*/
	virtual TCHAR * GetBitmapFileName()=0;

	//! Get the Clip U Data from a the Bitmap Texture
	/*!
	\return A pointer to IGameProperty
	*/
	virtual IGameProperty * GetClipUData()=0;

	//! Get the Clip V Data from a the Bitmap Texture
	/*!
	\return A pointer to IGameProperty
	*/
	virtual IGameProperty * GetClipVData()=0;

	//! Get the Clip H Data from a the Bitmap Texture
	/*!
	\return A pointer to IGameProperty
	*/
	virtual IGameProperty * GetClipHData()=0;

	//! Get the Clip W Data from a the Bitmap Texture
	/*!
	\return A pointer to IGameProperty
	*/
	virtual IGameProperty * GetClipWData()=0;


};

//! class IGameBitmapTex
/*! An IGame wrapper class for basic Bitmap Texture access. Properties such as tiling are also provided
\sa IGameMaterial, IGameProperty
*/
/*
class IGameBitmapTex : public IExportEntity
{
public:

	virtual BitmapTex* GetMaxBitmapTex()=0;



};
*/
//!simple wrapper for UVGen type data
/*!This is basically a helper class to access some data from the Coordinate Rollout panel.  All data is extracted
from the paramblock, and access is provided by support methods that handle the Property Container Access.  This data
is used to extract the actual MAtrix used.  However it can be animated, so using this data the matrix can be recontructed.
\sa IGameBitmapTex, IGameProperty
*/
class IGameUVGen : public IExportEntity
{
public:

	//! Get the U Offset Data
	/*!
	\return A pointer to IGameProperty
	*/
	virtual IGameProperty * GetUOffsetData() = 0;

	//! Get the V Offset Data
	/*!
	\return A pointer to IGameProperty
	*/
	virtual IGameProperty * GetVOffsetData() = 0;

	//! Get the U Tiling Data
	/*!
	\return A pointer to IGameProperty
	*/
	virtual IGameProperty * GetUTilingData() = 0;

	//! Get the V Tiling Data
	/*!
	\return A pointer to IGameProperty
	*/
	virtual IGameProperty * GetVTilingData() = 0;

	//! Get the U Angle Data
	/*!
	\return A pointer to IGameProperty
	*/
	virtual IGameProperty * GetUAngleData() = 0;
	
	//! Get the V Angle Data
	/*!
	\return A pointer to IGameProperty
	*/
	virtual IGameProperty * GetVAngleData() = 0;

	//! Get the W Angle Data
	/*!
	\return A pointer to IGameProperty
	*/
	virtual IGameProperty * GetWAngleData() = 0;

	//! Get the actual UV transform.
	/*! The UV transform that is the result of the Coordinate data.  
	\returns A GMatrix representation of the matrix
	*/
	virtual GMatrix GetUVTransform() = 0;

};
#endif 