 /**********************************************************************
 
	FILE:           INodeGIProperties.h

	DESCRIPTION:    Public interface for setting and getting a node's
                    global illumination (radiosity) properties.

	CREATED BY:     Daniel Levesque, Discreet

	HISTORY:        created 1 June 2001

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/


#ifndef _INODEGIPROPERTIES__H_
#define _INODEGIPROPERTIES__H_

#include "ifnpub.h"

// The interface ID for class INodeGIProperties
#define NODEGIPROPERTIES_INTERFACE Interface_ID(0x3e5d0f37, 0x37e5009c)


//==============================================================================
// class INodeGIProperties
//  
//
// This class defines an interface for accessing a node's global illumination
// properties.
//
// An instance of this interface can be retrieved using the following line of
// code (assuming 'node' is of type INode*):
//
//	   static_cast<INodeGIProperties*>(node->GetInterface(NODEGIPROPERTIES_INTERFACE))
//
//
// Geometric and object objects have different properties. Accessing/setting
// geometric properties of non-geometric objects is safe,
// but will have no effect in the global illumination solution.
//
//
// Here is a description of the global illumination properties:
//
//
// GENERAL PROPERTIES (all types of objects)
//
//    Excluded:
//      Excluded objects should be ignored by the radiosity engine. The
//      should act as if these objects do not exist.
//
//    ByLayer:
//      Specifies whether the GI properties of this node's layer should be
//      used instead of the local settings.
//
//
// GEOMETRIC OBJECTS (affects only geometric objects):
//
//	   Occluder:
//       Occluding objects will block rays of light that hit their surface.
//       Non-occluding objects will not block those rays, but will still receive
//       illumination if the Receiver property is set.
//
//     Receiver:
//       Receiver objects will receive and store illumination from rays of light
//       that hit their surface.
//       Non-receiver objects will not store illumination.
//
//	   Diffuse:
//       Diffuse surfaces will reflect and trasmit light based on their diffuse
//       color and transparency value.
//
//	   Specular:
//       Specular surfaces will generate specular reflections and transparency.
//       ex.: glass is specular transparent and a mirror is specular reflective.
//
//     UseGlobalMeshSettings:
//       When subdividing the geometry for a more accurate GI solution,
//       this flag specifies whether some 'global' settings, or the node's local
//       settings should be used.
//
//     MeshingEnabled:
//       When using local settings, this specifies whether geometry subdivision
//       should occur on this node.
//
//     MeshSize:
//       The maximum size, in MAX universe units, that a face should have after
//       being subdivided.
//
//	   NbRefineSteps:
//       This is the saved number of refining steps to be performed on this node 
//       by the global illumination engine.
//
//     ExcludedFromRegather:
//       Set to 'true' to excluded an object from the 'regathering' process.
//
//     RayMult:
//       Specifies a multiplier that will increase or decrease the number of rays
//       cast for this object when regathering.
//
// LIGHT OBJECTS (affects only light sources):
//
//     StoreIllumToMesh:
//       Specifies whether the light emitted from this object should be stored
//       in the GI solution's mesh, and not be re-cast at render-time.
//
//
//==============================================================================
class INodeGIProperties : public FPMixinInterface {

public:

    // Copy properties from another interface
    virtual void CopyGIPropertiesFrom(const INodeGIProperties& source) = 0;


	// General properties
	virtual BOOL GIGetIsExcluded() const = 0;
	virtual void GISetIsExcluded(BOOL isExcluded) = 0;


	// Geometry object properties
	virtual BOOL GIGetIsOccluder() const = 0;
	virtual BOOL GIGetIsReceiver() const = 0;
	virtual BOOL GIGetIsDiffuse() const = 0;
	virtual BOOL GIGetIsSpecular() const = 0;
    virtual BOOL GIGetUseGlobalMeshSettings() const = 0;
    virtual BOOL GIGetMeshingEnabled() const = 0;
	virtual unsigned short GIGetNbRefineSteps() const = 0;
    virtual unsigned short GIGetNbRefineStepsDone() const = 0;
    virtual float GIGetMeshSize() const = 0;

	virtual void GISetIsOccluder(BOOL isOccluder) = 0;
	virtual void GISetIsReceiver(BOOL isReceiver) = 0;
	virtual void GISetIsDiffuse(BOOL isDiffuseReflective) = 0;
	virtual void GISetIsSpecular(BOOL isSpecular) = 0;
    virtual void GISetUseGlobalMeshSettings(BOOL globalMeshing) = 0;
    virtual void GISetMeshingEnabled(BOOL meshingEnabled) = 0;
	virtual void GISetNbRefineSteps(unsigned short nbRefineSteps) = 0;
    virtual void GISetNbRefineStepsDone(unsigned short nbRefineStepsDone) = 0;
    virtual void GISetMeshSize(float size) = 0;
	//JH 9.06.01
	virtual BOOL GIGetIsExcludedFromRegather() const = 0;
	virtual void GISetIsExcludedFromRegather(BOOL isExcluded) = 0;

	//JH 9.06.01
	// Light object property
	virtual BOOL GIGetStoreIllumToMesh() const = 0;
	virtual void GISetStoreIllumToMesh(BOOL storeIllum) = 0;

    // [dl | 12Nov2001] Get and Set the 'by layer' flag for radiosity properties.
    virtual BOOL GIGetByLayer() const = 0;
    virtual void GISetByLayer(BOOL byLayer) = 0;

	// > 3/9/02 - 2:36pm --MQM-- regathering ray multiplier node property
	virtual float GIGetRayMult() const = 0;
	virtual void  GISetRayMult(float rayMult) = 0;
};


#endif
