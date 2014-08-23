//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
#ifndef  _VRMLSFVEC2F_
#define  _VRMLSFVEC2F_

#include "VrmlField.h"


class VrmlSFVec2f : public VrmlSField {
public:

  VrmlSFVec2f(float x = 0.0, float y = 0.0);

  virtual ostream& print(ostream& os) const;

  virtual VrmlField *clone() const;

  virtual VrmlFieldType fieldType() const;
  virtual const VrmlSFVec2f* toSFVec2f() const;
  virtual VrmlSFVec2f* toSFVec2f();

  float x(void)			{ return d_x[0]; }
  float y(void)			{ return d_x[1]; }
  float *get()			{ return &d_x[0]; }

  void set(float x, float y)	{ d_x[0] = x; d_x[1] = y; }

  // return result
  double dot( VrmlSFVec2f * );
  double length();

  // modifiers
  void normalize();

  void add( VrmlSFVec2f * );
  void divide( float );
  void multiply( float );
  void subtract( VrmlSFVec2f * );

private:
  float d_x[2];

};

#endif
