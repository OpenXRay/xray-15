//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
#ifndef  _VRMLMFINT_
#define  _VRMLMFINT_

#include "VrmlField.h"

//
// It would be nice to somehow incorporate the reference counting
// into a base class (VrmlMField) or make a VrmlMField template...
// There is no support for copy-on-write, so if you modify an element
// of the data vector, all objects that share that data will see the
// change.
//

class VrmlMFInt : public VrmlMField {
private:

  class IData {			// reference counted int data
  public:
    IData(int n=0) : d_refs(1), d_n(n), d_v(n > 0 ? new int[n] : 0) {}
    ~IData() { delete [] d_v; }

    IData *ref() { ++d_refs; return this; }
    void deref() { if (--d_refs == 0) delete this; }

    int d_refs;			// number of MFInt objects using this data
    int d_n;			// size (in ints) of d_v
    int *d_v;			// data vector
  };

  IData *d_data;

public:

  VrmlMFInt();
  VrmlMFInt(int value);
  VrmlMFInt(int n, int *v);
  VrmlMFInt(const VrmlMFInt &src);

  ~VrmlMFInt();

  virtual ostream& print(ostream& os) const;

  void set(int n, int *v);
  VrmlMFInt& operator=(const VrmlMFInt& rhs);

  virtual VrmlField *clone() const;

  virtual VrmlFieldType fieldType() const;
  virtual const VrmlMFInt* toMFInt() const;
  virtual VrmlMFInt* toMFInt();

  int size() const			{ return d_data->d_n; }
  int *get() const			{ return d_data->d_v; }
  int &operator[](int i) const		{ return d_data->d_v[i]; }

};

#endif // _VRMLMFINT_
