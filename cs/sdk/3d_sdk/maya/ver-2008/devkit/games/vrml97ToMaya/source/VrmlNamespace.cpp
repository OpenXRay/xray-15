//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//  Copyrights on some portions of the code are held by others as documented
//  in the code. Permission to use this code for any purpose is granted as 
//  long as all other copyrights in the code are respected and this copyright
//  statement is retained in the code and accompanying documentation both 
//  online and printed.
//
/**************************************************
 * VRML 2.0 Parser
 * Copyright (C) 1996 Silicon Graphics, Inc.
 *
 * Author(s)    : Gavin Bell
 *                Daniel Woods (first port)
 **************************************************
 */

#include "config.h"
#include "VrmlNamespace.h"
#include "VrmlNodeType.h"
#include "VrmlNode.h"
#include "System.h"

#include <string.h>

// This should at least be a sorted vector...
list< VrmlNodeType* > VrmlNamespace::builtInList;

VrmlNamespace::VrmlNamespace( VrmlNamespace *parent ) :
  d_parent(parent)
{
  // Initialize typeList with built in nodes
  if (builtInList.size() == 0) defineBuiltIns();
}

VrmlNamespace::~VrmlNamespace()
{
  // Free nameList
  list<VrmlNode*>::iterator n;
  for (n = d_nameList.begin(); n != d_nameList.end(); ++n)
    (*n)->dereference();
  
  // Free typeList
  list<VrmlNodeType*>::iterator i;
  for (i = d_typeList.begin(); i != d_typeList.end(); ++i)
    (*i)->dereference();
}


//
//  Built in nodes.
//  This code replaces the reading of the "standardNodes.wrl" file
//  of empty PROTOs so I don't need to carry that file around.
//

void
VrmlNamespace::addBuiltIn( VrmlNodeType *type)
{
  builtInList.push_front( type->reference() );
}


#include "VrmlNodeAnchor.h"
#include "VrmlNodeAppearance.h"
#include "VrmlNodeAudioClip.h"
#include "VrmlNodeBackground.h"
#include "VrmlNodeBillboard.h"
#include "VrmlNodeBox.h"
#include "VrmlNodeCollision.h"
#include "VrmlNodeColor.h"
#include "VrmlNodeColorInt.h"
#include "VrmlNodeCone.h"
#include "VrmlNodeCoordinate.h"
#include "VrmlNodeCoordinateInt.h"
#include "VrmlNodeCylinder.h"
#include "VrmlNodeCylinderSensor.h"
#include "VrmlNodeDirLight.h"
#include "VrmlNodeElevationGrid.h"
#include "VrmlNodeExtrusion.h"
#include "VrmlNodeFog.h"
#include "VrmlNodeFontStyle.h"
#include "VrmlNodeGroup.h"
#include "VrmlNodeIFaceSet.h"
#include "VrmlNodeILineSet.h"
#include "VrmlNodeImageTexture.h"
#include "VrmlNodeInline.h"
#include "VrmlNodeLOD.h"
#include "VrmlNodeMaterial.h"
#include "VrmlNodeMovieTexture.h"
#include "VrmlNodeNavigationInfo.h"
#include "VrmlNodeNormal.h"
#include "VrmlNodeNormalInt.h"
#include "VrmlNodeOrientationInt.h"
#include "VrmlNodePixelTexture.h"
#include "VrmlNodePlaneSensor.h"
#include "VrmlNodePointLight.h"
#include "VrmlNodePointSet.h"
#include "VrmlNodePositionInt.h"
#include "VrmlNodeProximitySensor.h"
#include "VrmlNodeScalarInt.h"
#include "VrmlNodeScript.h"
#include "VrmlNodeShape.h"
#include "VrmlNodeSound.h"
#include "VrmlNodeSphere.h"
#include "VrmlNodeSphereSensor.h"
#include "VrmlNodeSpotLight.h"
#include "VrmlNodeSwitch.h"
#include "VrmlNodeText.h"
#include "VrmlNodeTextureCoordinate.h"
#include "VrmlNodeTextureTransform.h"
#include "VrmlNodeTimeSensor.h"
#include "VrmlNodeTouchSensor.h"
#include "VrmlNodeTransform.h"
#include "VrmlNodeViewpoint.h"
#include "VrmlNodeVisibilitySensor.h"
#include "VrmlNodeWorldInfo.h"


void VrmlNamespace::defineBuiltIns()
{
  addBuiltIn( VrmlNodeAnchor::defineType() );
  addBuiltIn( VrmlNodeAppearance::defineType() );
  addBuiltIn( VrmlNodeAudioClip::defineType() );
  addBuiltIn( VrmlNodeBackground::defineType() );
  addBuiltIn( VrmlNodeBillboard::defineType() );
  addBuiltIn( VrmlNodeBox::defineType() );
  addBuiltIn( VrmlNodeCollision::defineType() );
  addBuiltIn( VrmlNodeColor::defineType() );
  addBuiltIn( VrmlNodeColorInt::defineType() );
  addBuiltIn( VrmlNodeCone::defineType() );
  addBuiltIn( VrmlNodeCoordinate::defineType() );
  addBuiltIn( VrmlNodeCoordinateInt::defineType() );
  addBuiltIn( VrmlNodeCylinder::defineType() );
  addBuiltIn( VrmlNodeCylinderSensor::defineType() );
  addBuiltIn( VrmlNodeDirLight::defineType() );
  addBuiltIn( VrmlNodeElevationGrid::defineType() );
  addBuiltIn( VrmlNodeExtrusion::defineType() );
  addBuiltIn( VrmlNodeFog::defineType() );
  addBuiltIn( VrmlNodeFontStyle::defineType() );
  addBuiltIn( VrmlNodeGroup::defineType() );
  addBuiltIn( VrmlNodeIFaceSet::defineType() );
  addBuiltIn( VrmlNodeILineSet::defineType() );
  addBuiltIn( VrmlNodeImageTexture::defineType() );
  addBuiltIn( VrmlNodeInline::defineType() );
  addBuiltIn( VrmlNodeLOD::defineType() );
  addBuiltIn( VrmlNodeMaterial::defineType() );
  addBuiltIn( VrmlNodeMovieTexture::defineType() );
  addBuiltIn( VrmlNodeNavigationInfo::defineType() );
  addBuiltIn( VrmlNodeNormal::defineType() );
  addBuiltIn( VrmlNodeNormalInt::defineType() );
  addBuiltIn( VrmlNodeOrientationInt::defineType() );
  addBuiltIn( VrmlNodePixelTexture::defineType() );
  addBuiltIn( VrmlNodePlaneSensor::defineType() );
  addBuiltIn( VrmlNodePointLight::defineType() );
  addBuiltIn( VrmlNodePointSet::defineType() );
  addBuiltIn( VrmlNodePositionInt::defineType() );
  addBuiltIn( VrmlNodeProximitySensor::defineType() );
  addBuiltIn( VrmlNodeScalarInt::defineType() );
  addBuiltIn( VrmlNodeScript::defineType() );
  addBuiltIn( VrmlNodeShape::defineType() );
  addBuiltIn( VrmlNodeSound::defineType() );
  addBuiltIn( VrmlNodeSphere::defineType() );
  addBuiltIn( VrmlNodeSphereSensor::defineType() );
  addBuiltIn( VrmlNodeSpotLight::defineType() );
  addBuiltIn( VrmlNodeSwitch::defineType() );
  addBuiltIn( VrmlNodeText::defineType() );
  addBuiltIn( VrmlNodeTextureCoordinate::defineType() );
  addBuiltIn( VrmlNodeTextureTransform::defineType() );
  addBuiltIn( VrmlNodeTimeSensor::defineType() );
  addBuiltIn( VrmlNodeTouchSensor::defineType() );
  addBuiltIn( VrmlNodeTransform::defineType() );
  addBuiltIn( VrmlNodeViewpoint::defineType() );
  addBuiltIn( VrmlNodeVisibilitySensor::defineType() );
  addBuiltIn( VrmlNodeWorldInfo::defineType() );
}


// A safer version for reading PROTOs from files.

void
VrmlNamespace::addNodeType( VrmlNodeType *type )
{
  if ( findType( type->getName() ) != NULL)
    theSystem->warn("PROTO %s already defined\n",
		    type->getName() );
  else
    d_typeList.push_front( type->reference() );
}


const VrmlNodeType *
VrmlNamespace::findType( const char *name )
{
  // Look through the PROTO stack:
  const VrmlNodeType *nt = findPROTO(name);
  if (nt) return nt;

  // Look in parent scope for the type
  if (d_parent)
    return d_parent->findType( name );

  // Look through the built ins
  list<VrmlNodeType*>::iterator i;
  for (i = builtInList.begin(); i != builtInList.end(); ++i)
    {
      nt = *i;
      if (nt != NULL && strcmp(nt->getName(),name) == 0)
	return nt;
    }

  return NULL;
}

const VrmlNodeType *    // LarryD
VrmlNamespace::findPROTO(const char *name)
{
  // Look through the PROTO list ONLY:
  list<VrmlNodeType*>::iterator i;
  for (i = d_typeList.begin(); i != d_typeList.end(); ++i)
    {
      const VrmlNodeType *nt = *i;
      if (nt != NULL && strcmp(nt->getName(),name) == 0)
	return nt;
    }
  return NULL;
}


const VrmlNodeType *
VrmlNamespace::firstType()
{
  // Top of the PROTO stack (should make sure it has an implementation...)
  if (d_typeList.size() > 0)
    return d_typeList.front()->reference();
  return NULL;
}

void
VrmlNamespace::addNodeName( VrmlNode *namedNode )
{
  // We could remove any existing node with this name, but
  // since we are just pushing this one onto the front of
  // the list, the other name won't be found. If we do
  // something smart with this list (like sorting), this
  // will need to change.
  d_nameList.push_front( namedNode->reference() );
}

void
VrmlNamespace::removeNodeName( VrmlNode *namedNode )
{
  d_nameList.remove( namedNode );
  namedNode->dereference();
}


VrmlNode* VrmlNamespace::findNode( const char *name )
{
  list<VrmlNode*>::iterator n;
  for (n = d_nameList.begin(); n != d_nameList.end(); ++n)
    if (strcmp((*n)->name(), name) == 0)
      return *n;

  return 0;
}
