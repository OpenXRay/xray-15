//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
#ifndef  _VRMLMFNODE_
#define  _VRMLMFNODE_

#include "VrmlField.h"

class VrmlNode;

class VrmlMFNode : public VrmlMField {
public:

  VrmlMFNode();
  VrmlMFNode(VrmlNode *value);
  VrmlMFNode(int n, VrmlNode **v);
  VrmlMFNode(const VrmlMFNode&);

  ~VrmlMFNode();

  virtual ostream& print(ostream& os) const;

  // Assignment. Since the nodes themselves are ref counted,
  // I don't bother trying to share the NodeLists.
  VrmlMFNode& operator=(const VrmlMFNode& rhs);

  virtual VrmlField *clone() const;

  virtual VrmlFieldType fieldType() const;
  virtual const VrmlMFNode* toMFNode() const;
  virtual VrmlMFNode* toMFNode();

  int size() const			{ return d_size; }
  VrmlNode **get()			{ return d_v; }
  VrmlNode* get(int index)		{ return d_v[index]; }

  // can't use this as lhs for now.
  VrmlNode* operator[](int index) const	{ return d_v[index]; }

  bool exists(VrmlNode *n);

  void addNode(VrmlNode *n);
  void removeNode(VrmlNode *n);


private:

  VrmlNode **d_v;
  int d_allocated;
  int d_size;

};

#endif //  _VRMLMFNODE_
