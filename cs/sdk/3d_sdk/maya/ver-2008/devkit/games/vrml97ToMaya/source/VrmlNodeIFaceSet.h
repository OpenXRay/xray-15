//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeIFaceSet.h

#ifndef  _VRMLNODEIFACESET_
#define  _VRMLNODEIFACESET_

#include "VrmlNodeIndexedSet.h"
#include "VrmlSFBool.h"
#include "VrmlSFFloat.h"
#include "VrmlSFNode.h"
#include "VrmlMFInt.h"

class VrmlNodeIFaceSet : public VrmlNodeIndexedSet {

public:

  // Define the fields of indexed face set nodes
  static VrmlNodeType *defineType(VrmlNodeType *t = 0);
  virtual VrmlNodeType *nodeType() const;

  VrmlNodeIFaceSet(VrmlScene *);
  virtual ~VrmlNodeIFaceSet();

  virtual VrmlNode *cloneMe() const;
  virtual void cloneChildren(VrmlNamespace *);

  virtual bool isModified() const;

  virtual void clearFlags();

  virtual void addToScene( VrmlScene *s, const char *relUrl );

  virtual void copyRoutes( VrmlNamespace *ns ) const;

  virtual ostream& printFields(ostream& os, int indent);

  virtual Viewer::Object insertGeometry(Viewer *v);

  virtual void setField(const char *fieldName, const VrmlField &fieldValue);

  virtual VrmlNodeIFaceSet* toIFaceSet() const;

  virtual VrmlNode* getNormal();
  virtual const VrmlMFInt& getNormalIndex() const;

  virtual VrmlNode* getTexCoord();
  virtual const VrmlMFInt& getTexCoordIndex() const;

  virtual bool getCcw(){ return d_ccw.get(); }  // LarryD  Feb18/99
  virtual bool getConvex(){ return d_convex.get(); }   // LarryD Feb18/99
  virtual float getCreaseAngle(){ return d_creaseAngle.get();} // LarryD  Feb18/99
  virtual bool getNormalPerVertex(){ return d_normalPerVertex.get();} // LarryD  Feb18/99
  virtual bool getSolid(){ return d_solid.get();} // LarryD  Feb18/99

protected:

  VrmlSFBool d_ccw;
  VrmlSFBool d_convex;
  VrmlSFFloat d_creaseAngle;
  VrmlSFNode d_normal;
  VrmlMFInt d_normalIndex;
  VrmlSFBool d_normalPerVertex;
  VrmlSFBool d_solid;
  VrmlSFNode d_texCoord;
  VrmlMFInt d_texCoordIndex;
  
};

#endif // _VRMLNODEIFACESET_

