//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//  %W% %G%
//  VrmlNodeElevationGrid.h

#ifndef  _VRMLNODEELEVATIONGRID_
#define  _VRMLNODEELEVATIONGRID_

#include "VrmlNodeGeometry.h"

#include "VrmlSFBool.h"
#include "VrmlSFFloat.h"
#include "VrmlSFInt.h"
#include "VrmlSFNode.h"

#include "VrmlMFFloat.h"


class VrmlNodeElevationGrid : public VrmlNodeGeometry {

public:

  // Define the fields of elevationGrid nodes
  static VrmlNodeType *defineType(VrmlNodeType *t = 0);
  virtual VrmlNodeType *nodeType() const;

  VrmlNodeElevationGrid(VrmlScene *);
  virtual ~VrmlNodeElevationGrid();

  virtual VrmlNode *cloneMe() const;
  virtual void cloneChildren(VrmlNamespace *);

  virtual bool isModified() const;
  virtual void clearFlags();

  virtual void addToScene( VrmlScene *s, const char *relUrl );

  virtual void copyRoutes(VrmlNamespace *ns) const;

  virtual ostream& printFields(ostream& os, int indent);

  virtual Viewer::Object insertGeometry(Viewer *);

  virtual void setField(const char *fieldName, const VrmlField &fieldValue);

  virtual VrmlNodeColor *color();

  virtual VrmlNodeElevationGrid* toElevationGrid() const; //LarryD Mar 09/99

  virtual VrmlNode* getNormal();  //LarryD Mar 09/99
  virtual VrmlNode* getTexCoord();  //LarryD Mar 09/99
  virtual bool getCcw(){ return d_ccw.get(); }   //LarryD Mar 09/99
  virtual bool getColorPerVertex(){ return d_colorPerVertex.get(); }  //LarryD Mar 09/99
  virtual float getCreaseAngle(){ return d_creaseAngle.get();}  //LarryD Mar 09/99
  virtual const VrmlMFFloat& getHeight() const;   //LarryD Mar 09/99
  virtual bool getNormalPerVertex(){ return d_normalPerVertex.get();}  //LarryD Mar 09/99
  virtual bool getSolid(){ return d_solid.get();}  //LarryD Mar 09/99
  virtual int getXDimension() { return d_xDimension.get(); }  //LarryD Mar 09/99
  virtual float getXSpacing() { return d_xSpacing.get(); }  //LarryD Mar 09/99
  virtual int getZDimension() { return d_zDimension.get(); }  //LarryD Mar 09/99
  virtual float getZSpacing() { return d_zSpacing.get(); }  //LarryD Mar 09/99

protected:

  VrmlSFNode d_color;
  VrmlSFNode d_normal;
  VrmlSFNode d_texCoord;

  VrmlSFBool d_ccw;
  VrmlSFBool d_colorPerVertex;
  VrmlSFFloat d_creaseAngle;
  VrmlMFFloat d_height;
  VrmlSFBool d_normalPerVertex;
  VrmlSFBool d_solid;
  VrmlSFInt d_xDimension;
  VrmlSFFloat d_xSpacing;
  VrmlSFInt d_zDimension;
  VrmlSFFloat d_zSpacing;

};

#endif // _VRMLNODEELEVATIONGRID_
