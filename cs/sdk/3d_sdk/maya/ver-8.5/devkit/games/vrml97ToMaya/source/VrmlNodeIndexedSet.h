//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeIndexedSet.h

#ifndef  _VRMLNODEINDEXEDSET_
#define  _VRMLNODEINDEXEDSET_

#include "VrmlNodeGeometry.h"

#include "VrmlSFBool.h"
#include "VrmlSFFloat.h"
#include "VrmlSFNode.h"
#include "VrmlMFInt.h"

class VrmlNodeIndexedSet : public VrmlNodeGeometry {

public:

  // Define the fields of indexed face set nodes
  static VrmlNodeType *defineType(VrmlNodeType *t);

  VrmlNodeIndexedSet(VrmlScene *);
  virtual ~VrmlNodeIndexedSet();

  virtual bool isModified() const;

  virtual void clearFlags();

  virtual void addToScene( VrmlScene *s, const char *relUrl );

  virtual void copyRoutes(VrmlNamespace *ns) const;

  virtual ostream& printFields(ostream& os, int indent);

  virtual void setField(const char *fieldName, const VrmlField &fieldValue);

  virtual VrmlNodeColor *color();

  virtual VrmlNode* getCoordinate();
  virtual const VrmlMFInt& getCoordIndex() const;
  virtual bool getColorPerVertex(){ return d_colorPerVertex.get(); } // LarryD  Feb18/99
  virtual const VrmlMFInt& getColorIndex() const; // LarryD  Feb18/99

protected:

  VrmlSFNode d_color;
  VrmlMFInt d_colorIndex;
  VrmlSFBool d_colorPerVertex;

  VrmlSFNode d_coord;
  VrmlMFInt d_coordIndex;

};

#endif // _VRMLNODEINDEXEDSET_
