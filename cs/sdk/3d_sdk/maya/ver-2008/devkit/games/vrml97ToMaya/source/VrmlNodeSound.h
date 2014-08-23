//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeSound.h
//    contributed by Kumaran Santhanam

#ifndef  _VRMLNODESOUND_
#define  _VRMLNODESOUND_

#include "VrmlNodeChild.h"

#include "VrmlSFBool.h"
#include "VrmlSFFloat.h"
#include "VrmlSFNode.h"
#include "VrmlSFVec3f.h"

class VrmlScene;


class VrmlNodeSound : public VrmlNodeChild {

public:

  // Define the fields of Sound nodes
  static VrmlNodeType *defineType(VrmlNodeType *t = 0);
  virtual VrmlNodeType *nodeType() const;

  VrmlNodeSound( VrmlScene *scene = 0 );
  virtual ~VrmlNodeSound();

  virtual VrmlNode *cloneMe() const;
  virtual void cloneChildren(VrmlNamespace*);

  virtual void clearFlags();

  virtual void addToScene(VrmlScene *s, const char *);

  virtual void copyRoutes(VrmlNamespace *ns) const;

  virtual void render(Viewer *);

  virtual VrmlNodeSound* toSound() const;

  virtual ostream& printFields(ostream& os, int indent);

  virtual void setField(const char *fieldName, const VrmlField &fieldValue);

private:

  // Fields
  VrmlSFVec3f d_direction;
  VrmlSFFloat d_intensity;
  VrmlSFVec3f d_location;
  VrmlSFFloat d_maxBack;
  VrmlSFFloat d_maxFront;
  VrmlSFFloat d_minBack;
  VrmlSFFloat d_minFront;
  VrmlSFFloat d_priority;
  VrmlSFNode d_source;
  VrmlSFBool d_spatialize;
};

#endif // _VRMLNODESOUND_

