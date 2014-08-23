//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  VrmlField.h
//  Each field type should support clone, get, and set methods. (Most of these
//  field types could be handled by SF and MF templates ...)
//

#ifndef  _VRMLFIELD_
#define  _VRMLFIELD_

#include "config.h"

#if defined AW_NEW_IOSTREAMS
#  include <iosfwd>
#else
#  include <iostream.h> // IRIX only
#endif

class VrmlSFBool;
class VrmlSFColor;
class VrmlSFFloat;
class VrmlSFImage;
class VrmlSFInt;
class VrmlSFNode;
class VrmlSFRotation;
class VrmlSFString;
class VrmlSFTime;
class VrmlSFVec2f;
class VrmlSFVec3f;

class VrmlMFColor;
class VrmlMFFloat;
class VrmlMFInt;
class VrmlMFNode;
class VrmlMFRotation;
class VrmlMFString;
class VrmlMFVec2f;
class VrmlMFVec3f;


// Abstract base class for field values

class VrmlField {
  friend ostream& operator<<(ostream& os, const VrmlField& f);

public:

  // Field type identifiers
  typedef enum {
    NO_FIELD,
    SFBOOL,
    SFCOLOR,
    SFFLOAT,
    SFINT32,
    SFROTATION,
    SFTIME,
    SFVEC2F,
    SFVEC3F,
    SFIMAGE,
    SFSTRING,
    MFCOLOR,
    MFFLOAT,
    MFINT32,
    MFROTATION,
    MFSTRING,
    MFVEC2F,
    MFVEC3F,
    SFNODE,
    MFNODE
  } VrmlFieldType;

  // Constructors/destructor
  VrmlField();
  virtual ~VrmlField() = 0;

  // Copy self
  virtual VrmlField *clone() const = 0;

  // Write self
  virtual ostream& print(ostream& os) const = 0;

  // Field type
  virtual VrmlFieldType fieldType() const;

  // Field type name
  const char *fieldTypeName() const;

  // Name to field type
  static VrmlFieldType fieldType(const char *fieldTypeName);

  // safe downcasts, const and non-const versions.
  // These avoid casts of VrmlField* but are ugly in that this class
  // must know of the existence of all of its subclasses...

  virtual const VrmlSFBool *toSFBool() const;
  virtual const VrmlSFColor *toSFColor() const;
  virtual const VrmlSFFloat *toSFFloat() const;
  virtual const VrmlSFImage *toSFImage() const;
  virtual const VrmlSFInt *toSFInt() const;
  virtual const VrmlSFNode *toSFNode() const;
  virtual const VrmlSFRotation *toSFRotation() const;
  virtual const VrmlSFString *toSFString() const;
  virtual const VrmlSFTime *toSFTime() const;
  virtual const VrmlSFVec2f *toSFVec2f() const;
  virtual const VrmlSFVec3f *toSFVec3f() const;
  
  virtual const VrmlMFColor *toMFColor() const;
  virtual const VrmlMFFloat *toMFFloat() const;
  virtual const VrmlMFInt *toMFInt() const;
  virtual const VrmlMFNode *toMFNode() const;
  virtual const VrmlMFRotation *toMFRotation() const;
  virtual const VrmlMFString *toMFString() const;
  virtual const VrmlMFVec2f *toMFVec2f() const;
  virtual const VrmlMFVec3f *toMFVec3f() const;

  virtual VrmlSFBool *toSFBool();
  virtual VrmlSFColor *toSFColor();
  virtual VrmlSFFloat *toSFFloat();
  virtual VrmlSFImage *toSFImage();
  virtual VrmlSFInt *toSFInt();
  virtual VrmlSFNode *toSFNode();
  virtual VrmlSFRotation *toSFRotation();
  virtual VrmlSFString *toSFString();
  virtual VrmlSFTime *toSFTime();
  virtual VrmlSFVec2f *toSFVec2f();
  virtual VrmlSFVec3f *toSFVec3f();
  
  virtual VrmlMFColor *toMFColor();
  virtual VrmlMFFloat *toMFFloat();
  virtual VrmlMFInt *toMFInt();
  virtual VrmlMFNode *toMFNode();
  virtual VrmlMFRotation *toMFRotation();
  virtual VrmlMFString *toMFString();
  virtual VrmlMFVec2f *toMFVec2f();
  virtual VrmlMFVec3f *toMFVec3f();

};

// Abstract base classes for single-valued & multi-valued fields
// So far they don't do anything, so they don't really exist yet,
// but I would like to make VrmlMFields be ref counted (I think)...

#define VrmlSField VrmlField
#define VrmlMField VrmlField


#endif
