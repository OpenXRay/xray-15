//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
#ifndef  _VRMLSFINT_
#define  _VRMLSFINT_

#include "VrmlField.h"


class VrmlSFInt : public VrmlSField {
public:

  VrmlSFInt(int value = 0);

  virtual ostream& print(ostream& os) const;

  virtual VrmlField *clone() const;

  virtual VrmlFieldType fieldType() const;
  virtual const VrmlSFInt* toSFInt() const;
  virtual VrmlSFInt* toSFInt();

  int  get(void) const		{ return d_value; }
  void set(int value)		{ d_value = value; }

private:
  int d_value;

};

#endif // _VRMLSFINT_
