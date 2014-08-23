 /**********************************************************************
 
	FILE:           INodeBakeProperties.h

	DESCRIPTION:    Public interface for setting and getting a node's
                    texture baking properties.

	CREATED BY:		Kells Elmquist

	HISTORY:		created 15 december 2001

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/


#ifndef _INodeBakeProperties__H_
#define _INodeBakeProperties__H_

#include "ifnpub.h"

//#include "renderElements.h"
class MaxBakeElement;

// The interface ID for class INodeBakeProperties
#define NODE_BAKE_PROPERTIES_INTERFACE Interface_ID(0x3e5d0f38, 0x37e500ac)

// by default use this channel for all baking ops
#define DEFAULT_BAKE_CHANNEL	3
#define DEFAULT_N_DILATIONS		2

//==============================================================================
// class INodeBakeProperties
//  
//
// This class defines an interface for accessing a node's texture
//  baking properties.
//
// An instance of this interface can be retrieved using the following line of
// code (assuming 'node' is of type INode*):
//
//	   static_cast<INodeBakeProperties*>(INode*->GetInterface(NODE_BAKE_PROPERTIES_INTERFACE))
//
//
// Description of the node's texture baking properties:
//
//
// GENERAL PROPERTIES:
//
//	  Enable:
//		Texture baking is enabled for this object
//
//    Baking Channel:
//      Flattening & baking use this uv mapping channel for this object
//
//	  List of Baking Render Elements:
//		each object has a list of render elements for output
//
//==============================================================================


// now the baking properties themselves
class INodeBakeProperties : public FPMixinInterface {

public:

	// General properties
	virtual BOOL GetBakeEnabled() const = 0;
	virtual void SetBakeEnabled( BOOL isExcluded ) = 0;

	// mapping channel to use for baking
	virtual int  GetBakeMapChannel() const = 0;
	virtual void SetBakeMapChannel( int mapChannel ) = 0;

	// number of dilations after rendering, affects seaming
	virtual int  GetNDilations() const = 0;
	virtual void SetNDilations( int nDilations ) = 0;

	virtual int  GetBakeFlags() const = 0;
	virtual void SetBakeFlags( int flags ) = 0;

	virtual float GetSurfaceArea() const = 0;
	virtual void  SetSurfaceArea( float area ) = 0;

	virtual float GetAreaScale() const = 0;
	virtual void  SetAreaScale( float scale ) = 0;

	// bake render elements
	virtual int GetNBakeElements() const = 0;
	virtual MaxBakeElement* GetBakeElement( int nElement ) = 0;
	virtual Tab<MaxBakeElement*> GetBakeElementArray() = 0;

	virtual BOOL AddBakeElement( MaxBakeElement* pEle ) = 0;
	virtual BOOL RemoveBakeElement( MaxBakeElement* pEle ) = 0;
	virtual BOOL RemoveBakeElementByName( char * name ) = 0;
	virtual BOOL RemoveBakeElementByIndex( int index ) = 0;
	virtual void RemoveAllBakeElements() = 0;

	// reset params to default, toss render elements
	virtual void ResetBakeProps() = 0;

	// enabled & has some elements & sz not 0
	virtual BOOL GetEffectiveEnable() = 0;

	// largest size of enabled baking elements
	virtual IPoint2 GetRenderSize() = 0;

	virtual FBox2 GetActiveRegion() =0;
	virtual void SetActiveRegion(FBox2 region) = 0;
};


#endif