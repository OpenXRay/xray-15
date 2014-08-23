//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
// %W% %G%
//

#ifndef  _VRMLNODEAPPEARANCE_
#define  _VRMLNODEAPPEARANCE_

#include "VrmlNodeChild.h"
#include "VrmlSFNode.h"

class VrmlNodeType;
class VrmlScene;

class VrmlNodeAppearance : public VrmlNodeChild {

public:

  // Define the built in VrmlNodeType:: "Appearance"
  static VrmlNodeType *defineType(VrmlNodeType *t = 0);
  virtual VrmlNodeType *nodeType() const;

  VrmlNodeAppearance(VrmlScene *);
  virtual ~VrmlNodeAppearance();

  // Copy the node.
  virtual VrmlNode *cloneMe() const;
  virtual void cloneChildren( VrmlNamespace* );

  virtual VrmlNodeAppearance* toAppearance() const;

  virtual bool isModified() const;
  virtual void clearFlags();	// Clear childrens flags too.

  virtual void addToScene( VrmlScene *s, const char *relativeUrl );

  virtual void copyRoutes(VrmlNamespace *ns) const;

  virtual ostream& printFields(ostream& os, int indent);

  virtual void render(Viewer *);

  virtual void setField(const char *fieldName,
			const VrmlField &fieldValue);

  VrmlNode *material()	{ return (VrmlNode *) d_material.get(); }
  VrmlNode *texture()	{ return (VrmlNode *) d_texture.get(); }
  VrmlNode *textureTransform()
    { return (VrmlNode *) d_textureTransform.get(); }

protected:

  VrmlSFNode d_material;
  VrmlSFNode d_texture;
  VrmlSFNode d_textureTransform;

};

#endif // _VRMLNODEAPPEARANCE_

