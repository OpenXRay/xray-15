//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
#ifndef  _VRMLSFNODE_
#define  _VRMLSFNODE_


#if defined(OSMac_)
#include <ostream.h>
#endif // OSMac_

#include "VrmlField.h"

class VrmlNode;

class VrmlSFNode : public VrmlSField {
public:

  VrmlSFNode(VrmlNode *value = 0);
  VrmlSFNode(const VrmlSFNode &n);
  ~VrmlSFNode();

  // Assignment.
  VrmlSFNode& operator=(const VrmlSFNode& rhs);

  virtual ostream& print(ostream& os) const;

  virtual VrmlField *clone() const;

  virtual VrmlFieldType fieldType() const;
  virtual const VrmlSFNode* toSFNode() const;
  virtual VrmlSFNode* toSFNode();

  VrmlNode *get(void) const		{ return d_value; }

  void set(VrmlNode *value);

private:
  VrmlNode *d_value;

};

#endif // _VRMLSFNODE_
