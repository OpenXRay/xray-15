//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeGroup.h

#ifndef  _VRMLNODEGROUP_
#define  _VRMLNODEGROUP_

#include "VrmlMFNode.h"
#include "VrmlSFString.h"
#include "VrmlSFVec3f.h"

#include "VrmlNodeChild.h"
#include "Viewer.h"


class VrmlNodeGroup : public VrmlNodeChild {

public:

  // Define the fields of all built in group nodes
  static VrmlNodeType *defineType(VrmlNodeType *t = 0);
  virtual VrmlNodeType *nodeType() const;

  VrmlNodeGroup(VrmlScene *s = 0);
  virtual ~VrmlNodeGroup();

  virtual VrmlNode *cloneMe() const;
  virtual void cloneChildren(VrmlNamespace *);

  virtual VrmlNodeGroup* toGroup() const;

  virtual bool isModified() const;
  virtual void clearFlags();

  virtual void addToScene( VrmlScene *s, const char *relativeUrl );

  virtual void copyRoutes(VrmlNamespace *ns) const;

  virtual ostream& printFields(ostream& os, int indent);

  virtual void render(Viewer *);

  virtual void accumulateTransform(VrmlNode*);

  void activate( double timeStamp, bool isOver, bool isActive, double *p );

  void addChildren( const VrmlMFNode &children );
  void removeChildren( const VrmlMFNode &children );
  void removeChildren();

  virtual void eventIn(double timeStamp,
		       const char *eventName,
		       const VrmlField *fieldValue);

  virtual void setField(const char *fieldName, const VrmlField &fieldValue);

  int size();
  VrmlNode *child(int index);

  virtual VrmlNode* getParentTransform();

  // LarryD Feb 11/99
  VrmlMFNode *getNodes()  { return &d_children;}

protected:

  VrmlSFVec3f d_bboxCenter;
  VrmlSFVec3f d_bboxSize;
  VrmlMFNode d_children;

  VrmlSFString d_relative;
  VrmlNode *d_parentTransform;
  Viewer::Object d_viewerObject;
};

#endif // _VRMLNODEGROUP_

