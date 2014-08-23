//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeSwitch.h

#ifndef  _VRMLNODESWITCH_
#define  _VRMLNODESWITCH_

#include "VrmlMFNode.h"
#include "VrmlSFInt.h"

#include "VrmlNodeChild.h"

class VrmlNodeSwitch : public VrmlNodeChild {

public:

  // Define the fields of all built in switch nodes
  static VrmlNodeType *defineType(VrmlNodeType *t = 0);
  virtual VrmlNodeType *nodeType() const;

  VrmlNodeSwitch(VrmlScene *);
  virtual ~VrmlNodeSwitch();

  virtual VrmlNode *cloneMe() const;
  void cloneChildren(VrmlNamespace *);

  virtual VrmlNodeSwitch* toSwitch() const; //LarryD

  virtual bool isModified() const;

  virtual void clearFlags();

  virtual void addToScene( VrmlScene *s, const char *relUrl );

  virtual void copyRoutes( VrmlNamespace *ns ) const;

  virtual ostream& printFields(ostream& os, int indent);

  virtual void render(Viewer *);

  virtual void setField(const char *fieldName, const VrmlField &fieldValue);

  VrmlMFNode *getChoiceNodes()  { return &d_choice;} 
  virtual int getWhichChoice() { return d_whichChoice.get(); }  


protected:

  VrmlMFNode d_choice;
  VrmlSFInt d_whichChoice;
  
};

#endif // _VRMLNODESWITCH_

