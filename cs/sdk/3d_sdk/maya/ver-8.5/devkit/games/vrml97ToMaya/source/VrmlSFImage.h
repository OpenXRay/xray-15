//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
#ifndef  _VRMLSFIMAGE_
#define  _VRMLSFIMAGE_

#include "VrmlField.h"


class VrmlSFImage : public VrmlSField {
public:

  VrmlSFImage(int w = 0, int h = 0, int nc = 0, unsigned char *pixels = 0);
  VrmlSFImage(const VrmlSFImage&);

  ~VrmlSFImage();

  // Assignment.
  VrmlSFImage& operator=(const VrmlSFImage& rhs);

  virtual ostream& print(ostream& os) const;

  virtual VrmlField *clone() const;

  virtual VrmlFieldType fieldType() const;
  virtual const VrmlSFImage* toSFImage() const;
  virtual VrmlSFImage* toSFImage();

  int width()				{ return d_w; }
  int height()         			{ return d_h; }
  int nComponents()			{ return d_nc; }
  unsigned char *pixels()		{ return d_pixels; }

  void setSize( int w, int h )		{ d_w = w; d_h = h; }

private:

  int d_w, d_h, d_nc;
  unsigned char *d_pixels;

};

#endif // _VRMLSFIMAGE_


