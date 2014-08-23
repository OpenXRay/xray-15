//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
#ifndef  _VRMLMFROTATION_
#define  _VRMLMFROTATION_

#include "VrmlField.h"

//
// It would be nice to somehow incorporate the reference counting
// into a base class (VrmlMField) or make a VrmlMField template...
// There is no support for copy-on-write, so if you modify an element
// of the data vector, all objects that share that data will see the
// change.
//

class VrmlMFRotation : public VrmlMField {
private:

  class FData {			// reference counted float data
  public:
    FData(int n=0) : d_refs(1), d_n(n), d_v(n > 0 ? new float[n] : 0) {}
    ~FData() { delete [] d_v; }

    FData *ref() { ++d_refs; return this; }
    void deref() { if (--d_refs == 0) delete this; }

    int d_refs;			// number of MF* objects using this data
    int d_n;			// size (in floats) of d_v
    float *d_v;			// data vector
  };

  FData *d_data;

public:

  VrmlMFRotation();
  VrmlMFRotation(float x, float y, float z, float r);
  VrmlMFRotation(int n, float *v);
  VrmlMFRotation(const VrmlMFRotation &src);

  ~VrmlMFRotation();

  virtual ostream& print(ostream& os) const;

  // Assignment.
  void set(int n, float *v);
  VrmlMFRotation& operator=(const VrmlMFRotation& rhs);

  virtual VrmlField *clone() const;

  virtual VrmlFieldType fieldType() const;
  virtual const VrmlMFRotation* toMFRotation() const;
  virtual VrmlMFRotation* toMFRotation();

  int size() const		{ return d_data->d_n/4; } // # of rotations
  float *get() const		{ return d_data->d_v; }
  float *operator[](int index)	{ return &d_data->d_v[4*index]; }

};

#endif // _VRMLMFROTATION_
