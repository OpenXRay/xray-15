//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
#ifndef  _VRMLMFVEC2F_
#define  _VRMLMFVEC2F_

#include "VrmlField.h"

//
// It would be nice to somehow incorporate the reference counting
// into a base class (VrmlMField) or make a VrmlMField template...
// There is no support for copy-on-write, so if you modify an element
// of the data vector, all objects that share that data will see the
// change.
//

class VrmlMFVec2f : public VrmlMField {
private:

  class FData {			// reference counted float data
  public:
    FData(int n=0) : d_refs(1), d_n(n), d_v(n > 0 ? new float[n] : 0) {}
    ~FData() { delete [] d_v; }

    FData *ref() { ++d_refs; return this; }
    void deref() { if (--d_refs == 0) delete this; }

    int d_refs;			// number of objects using this data
    int d_n;			// size (in floats) of d_v
    float *d_v;			// data vector
  };

  FData *d_data;		// Vec2f data

public:

  VrmlMFVec2f();
  VrmlMFVec2f(float x, float y);
  VrmlMFVec2f(int n, float *v);
  VrmlMFVec2f(const VrmlMFVec2f &source);

  ~VrmlMFVec2f();

  // Assignment.
  void set(int n, float *v);
  VrmlMFVec2f& operator=(const VrmlMFVec2f& rhs);

  virtual VrmlField *clone() const;

  virtual VrmlFieldType fieldType() const;
  virtual const VrmlMFVec2f* toMFVec2f() const;
  virtual VrmlMFVec2f* toMFVec2f();

  virtual ostream& print(ostream& os) const;

  int size() const		{ return d_data->d_n/2; } // # of vec2fs
  float *get() const		{ return d_data->d_v; }
  float *operator[](int index)	{ return &d_data->d_v[2*index]; }

};

#endif // _VRMLMFVEC2F_
