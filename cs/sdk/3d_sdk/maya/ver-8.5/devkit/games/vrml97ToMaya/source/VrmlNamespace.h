//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
#ifndef _VRMLNAMESPACE_
#define _VRMLNAMESPACE_

//
// The VrmlNamespace class keeps track of defined nodes and prototypes.
//

#include "config.h"

#if defined AW_NEW_IOSTREAMS
#  include <list>
#else
#  include <list.h>
#endif

#ifdef _WIN32
#include "VrmlNodeType.h"
#include "VrmlNode.h"
#else
class VrmlNodeType;
class VrmlNode;
#endif

class VrmlNamespace {
public:

  VrmlNamespace( VrmlNamespace *parent = 0 );
  ~VrmlNamespace();

  // PROTO definitions add node types to the namespace.
  // PROTO implementations are a separate node type namespace,
  // and require that any nested PROTOs NOT be available outside
  // the PROTO implementation. PROTOs defined outside the current
  // namespace are available.
  
  // addNodeType will print an error if the given type
  // is already defined (spec says behavior is undefined).
  void addNodeType(VrmlNodeType *);

  // DEFd nodes add node names to the namespace.
  // Node names are only defined in the current name space. They
  // are not available outside of the PROTO they are defined in,
  // nor are they available inside of nested PROTOs.

  void addNodeName(VrmlNode *);
  void removeNodeName(VrmlNode *);

  // Find a node type, given a type name. Returns NULL if type is not defined.
  const VrmlNodeType *findType(const char *nm);
  
  // Larry Feb 10_99  Find a nodeType, given a PROTO name 
  const VrmlNodeType *findPROTO(const char *nm);
 
  // Return the first node type in scope (default EXTERNPROTO implementation)
  const VrmlNodeType *firstType();
  
  // Find a node by name.
  VrmlNode *findNode(const char *name);
  
private:

  void addBuiltIn(VrmlNodeType *);
  void defineBuiltIns();

  // Builtin node types are stored (once) in this data structure:
  static list< VrmlNodeType* > builtInList;

  // Defined node types (PROTOs) for this namespace
  list< VrmlNodeType* > d_typeList;

  // Defined node names for this namespace
  list< VrmlNode* > d_nameList;

  // Parent namespace
  VrmlNamespace *d_parent;

};

#endif // _VRMLNAMESPACE_
