//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
#ifndef  _VRMLMFVEC3F_
#define  _VRMLMFVEC3F_

#include "VrmlField.h"

//
// It would be nice to somehow incorporate the reference counting
// into a base class (VrmlMField) or make a VrmlMField template...
// There is no support for copy-on-write, so if you modify an element
// of the data vector, all objects that share that data will see the
// change.
//

class VrmlMFVec3f : public VrmlMField {
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

  FData *d_data;		// Vec3f data

public:

  VrmlMFVec3f(int n = 0);
  VrmlMFVec3f(float x, float y, float z);
  VrmlMFVec3f(int n, float *v);
  VrmlMFVec3f(const VrmlMFVec3f &source);

  ~VrmlMFVec3f();

  // Assignment.
  void set(int n, float *v);
  VrmlMFVec3f& operator=(const VrmlMFVec3f& rhs);

  virtual VrmlField *clone() const;

  virtual VrmlFieldType fieldType() const;
  virtual const VrmlMFVec3f* toMFVec3f() const;
  virtual VrmlMFVec3f* toMFVec3f();

  virtual ostream& print(ostream& os) const;

  int size() const		{ return d_data->d_n/3; } // # of vec3fs
  float *get() const		{ return d_data->d_v; }
  float *operator[](int index)	{ return &d_data->d_v[3*index]; }

};

#endif // _VRMLMFVEC3F_
