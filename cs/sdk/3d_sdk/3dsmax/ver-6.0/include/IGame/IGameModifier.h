/**********************************************************************
 *<
	FILE: IGameModifier.h

	DESCRIPTION: Modifier interfaces for IGame

	CREATED BY: Neil Hazzard, Discreet

	HISTORY: created 02/02/02

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/
/*!\file IGameModifier.h
\brief IGame Modifier Interfaces including skin and physique.

High level access to MAX's modifier, with specific exposure for Skin modifiers
*/
#ifndef __IGAMEMODIFIER__H
#define __IGAMEMODIFIER__H

#pragma once

#include "max.h"
#include "IGameProperty.h"
#include "IConversionManager.h"


class IGameProperty;
class IGameNode;


//!Simple wrapper for max modifiers
/*! This is an IGame wrapper for the basic max modifer class.  This is provided so that the developer does not need
to walk the modifier stack and look for derived objects.  An instance of this class is obtained from IGameObject class
\sa IGameObject
*/
class IGameModifier : public IExportEntity
{
private:
	Modifier * gameMod;
	INode * gameNode;
	TSTR intName;
public:

	//! An enum of Modifier Types
	/*! These are the modifier known to IGame
	*/
	enum ModType{
		IGAME_SKINNING,	/*!<A skinning Modifier*/
		IGAME_GENERAL,	/*!<A generic Max modifier*/
	};

	IGameModifier(Modifier * mod, INode * node);


	/*! Return the Type of Modifier IGameModifier represents
	\return The modifier type This is value from the ModType Enum
	*/
	virtual ModType GetModifierType() =0;
	
	//!Get the modifier Name
	/*!The name as viewed in StackView
	\return The name
	*/
	virtual TCHAR * GetUIName() ;

	//!Get the modifier Name
	/*!The internal name of the modifier
	\return The name
	*/
	virtual TCHAR * GetInternalName() ;


	//! Access to the max modifier
	/*! This is provided so the developer can get to any LocalModData that may have been added to the modifier
	\return The pointer a standard max modifier. 
	*/
	virtual Modifier * GetMaxModifier();

	//! Access to the nodes this modifier is applied to,
	/*! This enumerates all the nodes that are effected by this modifier. 
	\param &nodeList The tab to receive the node list.  This will always be at least 1 in size, as it will contain the 
	original node.
	*/
	virtual void EffectedNodes(Tab<INode*> &nodeList);

	//! Defines whether the modifier is a skinning modifier
	/*!
	\return TRUE if the modifier is a skinning modifier
	*/
	virtual bool IsSkin();

	//! Defines whether the modifier is the  morpher modifier
	/*!
	\return TRUE if the modifier is the morpher modifier
	*/
	virtual bool IsMorpher();

	virtual ~IGameModifier();

};

//! A skin wrapper Class
/*! This class provides an unified interface to the various skin options present in Max.  This include Physique and Skin
All the data from skin and physique are stored in the same way, but options exist to find out what skinning option was used
The vertex indexes used here are the same as those for the actual mesh, so this provides a one to one corelation.
\n
The version of the Character Studio that is used for IGame is 3.2.1 - Anything earlier will cause problems
\n
NB: The bones need to be parsed by IGame before this interface can be used.
\sa IGameModifier

*/

class IGameSkin : public IGameModifier
{
public:

	//! An enum of Skin Modifier Types
	/*! These are the Skin modifiers known to IGame
	*/
	enum SkinType{
		IGAME_PHYSIQUE,	/*!<A Physique Modifier*/
		IGAME_SKIN,	/*!<A Max skin Modifier*/
	};
	//! An enum of Vertex types
	/*! These are the types used by the modifiers 
	*/
	enum VertexType{
		IGAME_RIGID,			/*!<A RIGID vertex*/
		IGAME_RIGID_BLENDED,	/*!<A BLENED vertex*/
		IGAME_UNKNOWN			/*!<Error or unsupported vertex*/
	};


	//! the numbers of vertices effected by this instance of the modifier.  
	/*! If the modifier is attached to more than one node, then this will be the count of vertices on the current node
	\return the number of vertices
	*/
	virtual int GetNumOfSkinnedVerts()=0;
	
	//! the numbers of bones effecting the vertex
	/*!
	\param vertexIndex The index of the vertex
	\return the number of bones
	*/
	virtual int GetNumberOfBones(int vertexIndex)= 0;
	
	//! Get the weight for the bone and vertex index passed in
	/*!
	\param vertexIndex The index of the vertex
	\param boneIndex The bone index 
	\return The weight
	*/
	virtual float GetWeight(int vertexIndex,int boneIndex) = 0;

	//! Get the max bone effecting the vertex
	/*!
	\param vertexIndex The index of the vertex
	\param boneIndex The bone index 
	\return A pointer to a max INode for the bone
	*/
	virtual INode * GetBone(int vertexIndex,int boneIndex)= 0;
	
	//! Get the IGameNode equivelant of the bone effecting the vertex
	/*!
	\param vertexIndex The index of the vertex
	\param boneIndex The bone index 
	\return A pointer to a IGameNode for the bone
	*/
	virtual IGameNode * GetIGameBone(int vertexIndex,int boneIndex)= 0;

	//! Get the IGameNode ID equivelant of the bone effecting the vertex
	/*! The IGameNode ID can be used if you pass out the nodes first
	// and use this value as an index when you import
	\param vertexIndex The index of the vertex
	\param boneIndex The bone index 
	\return A Node ID
	*/
	virtual int GetBoneID(int vertexIndex, int boneIndex) =0;
	
	//! Specifies whether the Vertex is either Rigid or blended
	/*!
	\param vertexIndex The vertex to query
	\return The vertex type.  It can be one of the following\n
	IGAME_RIGID\n				
	IGAME_RIGID_BLENDED\n
	*/
	virtual VertexType GetVertexType(int vertexIndex)=0;
	//! What skinning technique is used
	/*! This can be used to find out whether Max's skin or Physique was used
	\return The skinning type.  It can be one of the following\n
	IGAME_PHYSIQUE\n	
	IGAME_SKIN\n
	*/
	virtual SkinType GetSkinType()=0;

	//! The bone TM when skin was added
	/*! This provides access to the intial Bone TM when the skin modifier was applied.
	\param boneNode THe IGameNode bone whose matrix is needed
	\param &intMat  THe matrix to receive the intial TM
	\return True if the bone was found
	*/
	virtual bool GetInitBoneTM(IGameNode * boneNode, GMatrix &intMat)=0;

	//! The bone TM when skin was added
	/*! This provides access to the intial Bone TM when the skin modifier was applied.
	\param boneNode The Max INode bone whose matrix is needed
	\param &intMat  The matrix to receive the intial TM
	\return True if the bone was found
	*/
	virtual bool GetInitBoneTM(INode * boneNode, GMatrix &intMat)=0;

	//! The original TM for the node with skin. 
	/*! This provides access to the intial node TM when the skin modifier was applied.
	\param &intMat  The matrix to receive the intial TM
	*/
	virtual void GetInitSkinTM(GMatrix & intMat) = 0;

	IGameSkin(Modifier * mod, INode * node):IGameModifier(mod, node){};



};

//! A generic Modifier class
/*! Any modifier that is not known to IGame will be implemented as a "Generic" modifier, so that basic access can be provided
*/
class IGameGenMod : public IGameModifier
{
public:
	
	IGameGenMod(Modifier * mod, INode * node):IGameModifier(mod, node){};

};
#endif