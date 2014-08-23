//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeShape.h

#ifndef  _VRMLNODESHAPE_
#define  _VRMLNODESHAPE_

#include "VrmlNodeChild.h"
#include "VrmlSFNode.h"

#include "Viewer.h"

class VrmlNodeShape : public VrmlNodeChild {

public:

  // Define the fields of Shape nodes
  static VrmlNodeType *defineType(VrmlNodeType *t = 0);
  virtual VrmlNodeType *nodeType() const;

  VrmlNodeShape(VrmlScene *);
  virtual ~VrmlNodeShape();

  virtual VrmlNode *cloneMe() const;
  virtual void cloneChildren(VrmlNamespace*);

  virtual bool isModified() const;

  virtual void clearFlags();

  virtual VrmlNodeShape* toShape()	const;

  virtual void addToScene( VrmlScene *s, const char *relUrl );

  virtual void copyRoutes(VrmlNamespace *ns) const;

  virtual ostream& printFields(ostream& os, int indent);

  virtual void render(Viewer *);

  virtual void setField(const char *fieldName, const VrmlField &fieldValue);

  virtual VrmlNode* getAppearance()  { return d_appearance.get(); }
  virtual VrmlNode* getGeometry()    { return d_geometry.get(); }

private:

  VrmlSFNode d_appearance;
  VrmlSFNode d_geometry;

  Viewer::Object d_viewerObject; // move to VrmlNode.h ? ...
  
};

#endif // _VRMLNODESHAPE_

