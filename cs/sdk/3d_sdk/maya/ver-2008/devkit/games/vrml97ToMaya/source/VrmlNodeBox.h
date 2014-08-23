//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeBox.h

#ifndef  _VRMLNODEBOX_
#define  _VRMLNODEBOX_

#include "VrmlNodeGeometry.h"
#include "VrmlSFVec3f.h"

class VrmlScene;

class VrmlNodeBox : public VrmlNodeGeometry {

public:

  // Define the fields of box nodes
  static VrmlNodeType *defineType(VrmlNodeType *t = 0);
  virtual VrmlNodeType *nodeType() const;

  VrmlNodeBox(VrmlScene *);
  virtual ~VrmlNodeBox();

  virtual VrmlNode *cloneMe() const;

  virtual ostream& printFields(ostream& os, int indent);

  virtual Viewer::Object insertGeometry(Viewer *);

  virtual void setField(const char *fieldName, const VrmlField &fieldValue);

  virtual VrmlNodeBox* toBox() const; //LarryD Mar 08/99
  virtual const VrmlSFVec3f& getSize() const;  //LarryD Mar 08/99

protected:

  VrmlSFVec3f d_size;

};

#endif // _VRMLNODEBOX_
