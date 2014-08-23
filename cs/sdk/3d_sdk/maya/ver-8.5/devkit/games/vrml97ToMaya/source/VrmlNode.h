//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNode.h

#ifndef  _VRMLNODE_
#define  _VRMLNODE_

#include "config.h"
#include "System.h"		// error

#if defined AW_NEW_IOSTREAMS
#  include <iostream> // for PRINT_FIELD. What a mess...
#  include <list>
#else
#  include <list.h>
#endif

#include <string.h>

class Route;
class Viewer;

class VrmlNamespace;
class VrmlNodeType;
class VrmlField;
class VrmlScene;

// For the safe downcasts
class VrmlNodeAnchor;
class VrmlNodeAppearance;
class VrmlNodeAudioClip;
class VrmlNodeBackground;
class VrmlNodeBox;   //LarryD Mar 08/99
class VrmlNodeChild;
class VrmlNodeColor;
class VrmlNodeCone;   //LarryD Mar 08/99
class VrmlNodeCoordinate;
class VrmlNodeCylinder; //LarryD Mar 08/99
class VrmlNodeDirLight; //LarryD Mar 08/99
class VrmlNodeElevationGrid; //LarryD Mar 09/99
class VrmlNodeExtrusion; //LarryD Mar 09/99
class VrmlNodeFog;
class VrmlNodeFontStyle;
class VrmlNodeGeometry;
class VrmlNodeGroup;
class VrmlNodeIFaceSet;
class VrmlNodeInline;
class VrmlNodeLight;
class VrmlNodeMaterial;
class VrmlNodeMovieTexture;
class VrmlNodeNavigationInfo;
class VrmlNodeNormal;
class VrmlNodePlaneSensor;
class VrmlNodePointLight;
class VrmlNodeScript;
class VrmlNodeShape;
class VrmlNodeSphere; //LarryD Mar 08/99
class VrmlNodeSound;
class VrmlNodeSpotLight;
class VrmlNodeSwitch;
class VrmlNodeTexture;
class VrmlNodeTextureCoordinate;
class VrmlNodeTextureTransform;
class VrmlNodeTimeSensor;
class VrmlNodeTouchSensor;
class VrmlNodeTransform;
class VrmlNodeViewpoint;
class VrmlNodeImageTexture;
class VrmlNodePixelTexture;
class VrmlNodeLOD;
class VrmlNodeScalarInt;
class VrmlNodeOrientationInt;
class VrmlNodePositionInt;

class VrmlNodeProto;

class VrmlNode {
  friend ostream& operator<<(ostream& os, const VrmlNode& f);

public:

  // Define the fields of all built in VrmlNodeTypes
  static VrmlNodeType *defineType(VrmlNodeType *t);
  virtual VrmlNodeType *nodeType() const;

  // VrmlNodes are reference counted, optionally named objects
  // The reference counting is manual (that is, each user of a
  // VrmlNode, such as the VrmlMFNode class, calls reference()
  // and dereference() explicitly). Should make it internal...

  VrmlNode ( VrmlScene *s );
  VrmlNode(  const VrmlNode& );
  virtual ~VrmlNode() = 0;

  // Copy the node, defining its name in the specified scope.
  // Uses the flag to determine whether the node is a USEd node.
  VrmlNode *clone( VrmlNamespace* );
  virtual VrmlNode *cloneMe() const = 0;
  virtual void cloneChildren( VrmlNamespace* );

  // Copy the ROUTEs
  virtual void copyRoutes(VrmlNamespace *ns) const;

  // Add/remove references to a VrmlNode. This is silly, as it
  // requires the users of VrmlNode to do the reference/derefs...
  VrmlNode *reference();
  void dereference();

  // Safe node downcasts. These avoid the dangerous casts of VrmlNode* (esp in
  // presence of protos), but are ugly in that this class must know about all
  // the subclasses. These return 0 if the typecast is invalid.
  // Remember to also add new ones to VrmlNodeProto. Protos should
  // return their first implementation node (except toProto()).
  virtual VrmlNodeAnchor*	toAnchor() const;
  virtual VrmlNodeAppearance*	toAppearance() const;
  virtual VrmlNodeAudioClip*	toAudioClip() const;
  virtual VrmlNodeBackground*	toBackground() const;
  virtual VrmlNodeBox*		toBox() const; //LarryD Mar 08/99
  virtual VrmlNodeChild*	toChild() const;
  virtual VrmlNodeColor*	toColor() const;
  virtual VrmlNodeCone* toCone() const; //LarryD Mar 08/99
  virtual VrmlNodeCoordinate*	toCoordinate() const;
  virtual VrmlNodeCylinder* toCylinder() const; //LarryD Mar 08/99
  virtual VrmlNodeDirLight* toDirLight() const; //LarryD Mar 08/99
  virtual VrmlNodeElevationGrid* toElevationGrid() const; //LarryD Mar 09/99
  virtual VrmlNodeExtrusion*    toExtrusion() const; //LarryD Mar 09/99
  virtual VrmlNodeFog*		toFog() const;
  virtual VrmlNodeFontStyle*	toFontStyle() const;
  virtual VrmlNodeGeometry*	toGeometry() const;
  virtual VrmlNodeGroup*	toGroup() const;
  virtual VrmlNodeIFaceSet*	toIFaceSet() const;
  virtual VrmlNodeImageTexture* toImageTexture() const;
  virtual VrmlNodePixelTexture* toPixelTexture() const;
   virtual VrmlNodeInline*	toInline() const;
  virtual VrmlNodeLight*	toLight() const;
  virtual VrmlNodeMaterial*	toMaterial() const;
  virtual VrmlNodeMovieTexture*	toMovieTexture() const;
  virtual VrmlNodeNavigationInfo*	toNavigationInfo() const;
  virtual VrmlNodeNormal*	toNormal() const;
  virtual VrmlNodePlaneSensor*	toPlaneSensor() const;
  virtual VrmlNodePointLight*	toPointLight() const;
  virtual VrmlNodeScript*	toScript() const;
  virtual VrmlNodeShape*	toShape() const;
  virtual VrmlNodeSphere* toSphere() const; //LarryD Mar 08/99
  virtual VrmlNodeSound*	toSound() const;
  virtual VrmlNodeSpotLight*	toSpotLight() const;
  virtual VrmlNodeSwitch* toSwitch() const; //LarryD Mar 08/99
  virtual VrmlNodeTexture*	toTexture() const;
  virtual VrmlNodeTextureCoordinate*	toTextureCoordinate() const;
  virtual VrmlNodeTextureTransform* toTextureTransform() const;
  virtual VrmlNodeTimeSensor*	toTimeSensor() const;
  virtual VrmlNodeTouchSensor*	toTouchSensor() const;
  virtual VrmlNodeTransform* toTransform() const;     //LarryD Feb 24/99
  virtual VrmlNodeViewpoint*	toViewpoint() const;

  virtual VrmlNodeLOD* toLOD() const;
  virtual VrmlNodeScalarInt* toScalarInt() const;
  virtual VrmlNodeOrientationInt* toOrientationInt() const;
  virtual VrmlNodePositionInt* toPositionInt() const;

  virtual VrmlNodeProto*	toProto() const;

  // Node DEF/USE/ROUTE name
  void setName(const char *nodeName, VrmlNamespace *ns = 0 );
  const char *name() const;

  // Add to a scene. A node can belong to at most one scene for now.
  // If it doesn't belong to a scene, it can't be rendered.
  virtual void addToScene( VrmlScene *, const char *relativeUrl );

  // Write self
  ostream& print(ostream& os, int indent) const;
  virtual ostream& printFields(ostream& os, int indent);
  static  ostream& printField(ostream&, int, const char*, const VrmlField&);

  // Indicate that the node state has changed, need to re-render
  void setModified();
  void clearModified() { d_modified = false; }
  virtual bool isModified() const;

  // A generic flag (typically used to find USEd nodes).
  void setFlag() { d_flag = true; }
  virtual void clearFlags();	// Clear childrens flags too.
  bool isFlagSet() { return d_flag; }

  // Add a ROUTE from a field in this node
  void addRoute(const char *fromField, VrmlNode *toNode, const char *toField);

  // Delete a ROUTE from a field in this node
  void deleteRoute(const char *fromField, VrmlNode *toNode, const char *toField);

  // Pass a named event to this node. This method needs to be overridden
  // to support any node-specific eventIns behaviors, but exposedFields
  // (should be) handled here...
  virtual void eventIn(double timeStamp,
		       const char *eventName,
		       const VrmlField *fieldValue);

  // Set a field by name (used by the parser, not for external consumption).
  virtual void setField(const char *fieldName,
			const VrmlField &fieldValue);

  // Get a field or eventOut by name.
  virtual const VrmlField *getField(const char *fieldName) const;
  
  // Return an eventOut/exposedField value. Used by the script node
  // to access the node fields.
  const VrmlField *getEventOut(const char *fieldName) const;

  // Do nothing. Renderable nodes need to redefine this.
  virtual void render(Viewer *);

  // Do nothing. Grouping nodes need to redefine this.
  virtual void accumulateTransform(VrmlNode*);

  virtual VrmlNode* getParentTransform();

  // Compute an inverse transform (either render it or construct the matrix)
  virtual void inverseTransform(Viewer *);
  virtual void inverseTransform(double [4][4]);

  VrmlScene *scene() { return d_scene; }


protected:

  enum { INDENT_INCREMENT = 4 };

  // Send a named event from this node.
  void eventOut(double timeStamp,
		const char *eventName,
		const VrmlField &fieldValue);

  // Scene this node belongs to
  VrmlScene *d_scene;

  // True if a field changed since last render
  bool d_modified;
  bool d_flag;

  // Routes from this node (clean this up, add RouteList ...)
  Route *d_routes;


private:

  int d_refCount;		// Number of active references
  char *d_name;

};


// Routes
class Route {
public:
  Route(const char *fromEventOut, VrmlNode *toNode, const char *toEventIn);
  Route(const Route&);
  ~Route();

  char *fromEventOut() { return d_fromEventOut; }
  char *toEventIn() { return d_toEventIn; }
  VrmlNode *toNode() { return d_toNode; }

  Route *prev() { return d_prev; }
  Route *next() { return d_next; }
  void setPrev(Route* r) { d_prev = r; }
  void setNext(Route* r) { d_next = r; }
  
private:
  char *d_fromEventOut;
  VrmlNode *d_toNode;
  char *d_toEventIn;
  Route *d_prev, *d_next;
};


// Ugly macro used in printFields
#define PRINT_FIELD(_f) printField(os,indent+INDENT_INCREMENT,#_f,d_##_f)

// Ugly macros used in setField

#define TRY_FIELD(_f,_t) \
(strcmp(fieldName, #_f) == 0) {\
    if ( fieldValue.to##_t() )\
      d_##_f = (Vrml##_t &)fieldValue;\
    else \
      theSystem->error("Invalid type (%s) for %s field of %s node (expected %s).\n",\
	    fieldValue.fieldTypeName(), #_f, nodeType()->getName(), #_t);\
  }

// For SFNode fields. Allow un-fetched EXTERNPROTOs to succeed...
#define TRY_SFNODE_FIELD(_f,_n) \
(strcmp(fieldName, #_f) == 0) { \
    VrmlSFNode *x=(VrmlSFNode*)&fieldValue; \
    if (fieldValue.toSFNode() && \
	( (!x->get()) || x->get()->to##_n() || x->get()->toProto() )) \
      d_##_f = (VrmlSFNode &)fieldValue; \
    else \
      theSystem->error("Invalid type (%s) for %s field of %s node (expected %s).\n",\
	    fieldValue.fieldTypeName(), #_f, nodeType()->getName(), #_n);\
  }

#define TRY_SFNODE_FIELD2(_f,_n1,_n2) \
(strcmp(fieldName, #_f) == 0) { \
    VrmlSFNode *x=(VrmlSFNode*)&fieldValue; \
    if (fieldValue.toSFNode() && \
	((!x->get()) || x->get()->to##_n1() || x->get()->to##_n2() || \
	 x->get()->toProto() )) \
      d_##_f = (VrmlSFNode &)fieldValue; \
    else \
      theSystem->error("Invalid type (%s) for %s field of %s node (expected %s or %s).\n",\
	    fieldValue.fieldTypeName(), #_f, nodeType()->getName(), #_n1, #_n2);\
  }


#endif

