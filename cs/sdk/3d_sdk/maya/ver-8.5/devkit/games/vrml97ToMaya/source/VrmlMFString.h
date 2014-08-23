//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
#ifndef  _VRMLMFSTRING_
#define  _VRMLMFSTRING_

#include "VrmlField.h"


class VrmlMFString : public VrmlMField {
public:

  VrmlMFString();
  VrmlMFString(char* s);
  VrmlMFString(int n, char** values = 0);
  VrmlMFString(const VrmlMFString&);

  ~VrmlMFString();

  // Assignment. Just reallocate for now...
  void set(int n, char *v[]);
  VrmlMFString& operator=(const VrmlMFString& rhs);

  virtual VrmlField *clone() const;

  virtual VrmlFieldType fieldType() const;
  virtual const VrmlMFString* toMFString() const;
  virtual VrmlMFString* toMFString();

  virtual ostream& print(ostream& os) const;


  int size() const			{ return d_size; }
  char** get()				{ return &d_v[0]; }
  char* get(int index)			{ return d_v[index]; }
  char* &operator[](int index)		{ return d_v[index]; }

private:

  char **d_v;
  int d_allocated;
  int d_size;

};

#endif //  _VRMLMFSTRING_
