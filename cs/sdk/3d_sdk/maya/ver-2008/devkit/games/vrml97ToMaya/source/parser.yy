//*******************************************************
// VRML 2.0 Parser
// Copyright (C) 1996 Silicon Graphics, Inc.
//
// Author(s)    : Gavin Bell
//                Daniel Woods (first port, minor fixes)
//*******************************************************
//
%{

#include "config.h"

#include <stdio.h>		// sprintf
#include <string.h>

// Get rid of this and calls to free() (lexer uses strdup)...
#include <malloc.h>

#include "System.h"
#include "VrmlScene.h"
#include "VrmlField.h"

#include "VrmlNode.h"
#include "VrmlNamespace.h"
#include "VrmlNodeType.h"

#include "VrmlNodeScript.h"

#include "VrmlSFNode.h"
#include "VrmlMFNode.h"

// It would be nice to remove these globals...

// The defined node types (built in and PROTOd) and DEFd nodes
VrmlNamespace *yyNodeTypes = 0;

// The parser builds a scene graph rooted at this list of nodes.
VrmlMFNode *yyParsedNodes = 0;

// Where the world is being read from (needed to resolve relative URLs)
Doc *yyDocument = 0;


// Currently-being-defined proto.  Prototypes may be nested, so a stack
// is needed. I'm using a list because the STL stack API is still in flux.

static list < VrmlNodeType* > currentProtoStack;

// This is used to keep track of which field in which type of node is being
// parsed.  Field are nested (nodes are contained inside MFNode/SFNode fields)
// so a stack of these is needed. I'm using a list because the STL stack API
// is still in flux.

typedef VrmlField::VrmlFieldType FieldType;

typedef struct {
  VrmlNode *node;
  const VrmlNodeType *nodeType;
  const char *fieldName;
  FieldType fieldType;
} FieldRec;

static list < FieldRec* > currentField;

// Name for current node being defined.

static char *nodeName = 0;

// This is used when the parser knows what kind of token it expects
// to get next-- used when parsing field values (whose types are declared
// and read by the parser) and at certain other places:
extern int expectToken;
extern int expectCoordIndex;

// Current line number (set by lexer)
extern int currentLineNumber;

// Some helper routines defined below:
static void beginProto(const char *);
static void endProto(VrmlField *url);


// PROTO interface handlers
static FieldType addField(const char *type, const char *name);
static FieldType addEventIn(const char *type, const char *name);
static FieldType addEventOut(const char *type, const char *name);
static FieldType addExposedField(const char *type, const char *name);

static void setFieldDefault(const char *fieldName, VrmlField *value);
static FieldType fieldType(const char *type);
static void enterNode(const char *name);
static VrmlNode *exitNode();

// Node fields
static void enterField(const char *name);
static void exitField(VrmlField *value);
static void expect(FieldType type);

// Script fields
static bool inScript();
static void addScriptEventIn(const char *type, const char *name);
static void addScriptEventOut(const char *type, const char *name);
static void enterScriptField(const char *type, const char *name);
static void exitScriptField( VrmlField *value );


static VrmlMFNode *nodeListToMFNode(vector<VrmlNode*> *nodeList);

static vector<VrmlNode*> *addNodeToList(vector<VrmlNode*> *nodeList, VrmlNode *node);

static void addNode(VrmlNode *);
static void addRoute(const char *, const char *, const char *, const char *);

static VrmlField *addIS(const char *);
static VrmlField *addEventIS(const char *, const char *);

static VrmlNode *lookupNode(const char *);

void yyerror(const char *);
int  yylex(void);

%}

%union {
	char *string;
	VrmlField *field;
	VrmlNode *node;
        vector<VrmlNode *> *nodeList;
};

%type <string> IDENTIFIER
%type <field>  fieldValue

%type <node>   node
%type <node>   nodeDeclaration

%type <field> mfnodeValue;
%type <nodeList> nodes;

%token IDENTIFIER DEF USE PROTO EXTERNPROTO TO IS ROUTE SFN_NULL
%token EVENTIN EVENTOUT FIELD EXPOSEDFIELD

%token <field> SF_BOOL SF_COLOR SF_FLOAT SF_INT32 SF_ROTATION SF_TIME
%token <field> SF_IMAGE SF_STRING SF_VEC2F SF_VEC3F

%token <field> MF_COLOR MF_FLOAT MF_INT32 MF_ROTATION MF_STRING MF_VEC2F MF_VEC3F

%token SF_NODE MF_NODE

%%

vrmlscene:	declarations
	;

declarations:
		/* Empty is OK */
	|	declarations declaration
	;

declaration:
		nodeDeclaration			{ addNode($1); }
	|	protoDeclaration
	|	routeDeclaration
	;

nodeDeclaration:
		node
	|	DEF IDENTIFIER		{ nodeName = $2; }
		node			{ $$ = $4; free($2); }
	|	USE IDENTIFIER		{ $$ = lookupNode($2); free($2); }
	;

protoDeclaration:
		proto
	|	externproto
	;

proto:
		PROTO IDENTIFIER			{ beginProto($2); }
	     	'[' interfaceDeclarations ']'
		'{' declarations '}'			{ endProto(0); free($2);}
	;

externproto:
		EXTERNPROTO IDENTIFIER			{ beginProto($2); }
		'[' externInterfaceDeclarations ']'
					{ expect(VrmlField::MFSTRING); }
		fieldValue		{ endProto($8); free($2); }
	;

interfaceDeclarations:
		/* Empty is OK */
	|	interfaceDeclarations interfaceDeclaration
	;

interfaceDeclaration:
		EVENTIN IDENTIFIER IDENTIFIER		{ addEventIn($2, $3);
							  free($2); free($3); }
	|	EVENTOUT IDENTIFIER IDENTIFIER		{ addEventOut($2, $3);
							  free($2); free($3); }
	|	FIELD IDENTIFIER IDENTIFIER		{ expect(addField($2,$3)); }
    		fieldValue     				{ setFieldDefault($3, $5);
							  free($2); free($3); }
	|	EXPOSEDFIELD IDENTIFIER	IDENTIFIER	{ expect(addExposedField($2,$3)); }
    		fieldValue                      	{ setFieldDefault($3, $5);
							  free($2); free($3); }
	;

externInterfaceDeclarations:
		/* Empty is OK */
     |  externInterfaceDeclarations externInterfaceDeclaration
     ;

externInterfaceDeclaration:
	EVENTIN IDENTIFIER IDENTIFIER		{ addEventIn($2, $3);
						  free($2); free($3); }
    |   EVENTOUT IDENTIFIER IDENTIFIER		{ addEventOut($2, $3);
        					  free($2); free($3); }
    |	FIELD IDENTIFIER IDENTIFIER		{ addField($2, $3);
        					  free($2); free($3); }
    |	EXPOSEDFIELD IDENTIFIER IDENTIFIER	{ addExposedField($2, $3);
        					  free($2); free($3); }
    ;

routeDeclaration:
		ROUTE IDENTIFIER '.' IDENTIFIER TO IDENTIFIER '.' IDENTIFIER
        	{ addRoute($2, $4, $6, $8);
		  free($2); free($4); free($6); free($8); }
    ; 

node:
		IDENTIFIER  			{ enterNode($1); }
        	'{' nodeGuts '}'		{ $$ = exitNode(); free($1);}
    ;

nodeGuts:
		/* Empty is OK */
    | 	nodeGuts nodeGut
    ;

nodeGut:
    IDENTIFIER 					{ enterField($1); }
        fieldValue				{ exitField($3); free($1); }
    |	routeDeclaration
    |	protoDeclaration

    	/* The following are only valid for Script nodes: */
    |   EVENTIN IDENTIFIER IDENTIFIER		{ addScriptEventIn($2,$3);
    						  free($2); free($3); }
    |   EVENTOUT IDENTIFIER IDENTIFIER		{ addScriptEventOut($2, $3);
    						  free($2); free($3); }

    |   FIELD IDENTIFIER IDENTIFIER		{ enterScriptField($2, $3); }
    	fieldValue		                { exitScriptField($5);
    						  free($2); free($3); }
    |	EVENTIN IDENTIFIER IDENTIFIER		{ addScriptEventIn($2,$3); }
	IS IDENTIFIER    			{ addEventIS($3,$6); free($2); free($3); free($6); }
    |	EVENTOUT IDENTIFIER IDENTIFIER		{ addScriptEventOut($2,$3); }
	IS IDENTIFIER    			{ addEventIS($3,$6); free($2); free($3); free($6); }
   ;

fieldValue:
    	MF_COLOR
    |	MF_FLOAT
    |	MF_INT32
    |	MF_ROTATION
    |	MF_STRING
    |	MF_VEC2F
    |	MF_VEC3F

    |	SF_BOOL
    |   SF_COLOR
    |   SF_FLOAT
    |	SF_IMAGE
    |	SF_INT32
    |	SF_ROTATION
    |	SF_STRING
    |	SF_TIME
    |	SF_VEC2F
    |	SF_VEC3F

    |	SF_NODE nodeDeclaration		{ $$ = new VrmlSFNode($2); }
    |	SF_NODE SFN_NULL		{ $$ = 0; }
    |	MF_NODE mfnodeValue		{ $$ = $2; }

    |	IS IDENTIFIER			{ $$ = addIS($2); free($2); }
    |	SF_NODE IS IDENTIFIER		{ $$ = addIS($3); free($3); }
    |	MF_NODE IS IDENTIFIER		{ $$ = addIS($3); free($3); }
   ;


mfnodeValue:
	'[' nodes ']'				{ $$ = nodeListToMFNode($2); }
     |  nodeDeclaration				{ $$ = new VrmlMFNode($1); }
     ;

nodes:
		/* Empty is OK */		{ $$ = 0; }
     |  nodes nodeDeclaration			{ $$ = addNodeToList($1,$2); } 
     ;

%%

void
yyerror(const char *msg)
{
  theSystem->error("Error near line %d: %s\n", currentLineNumber, msg);
  expect(VrmlField::NO_FIELD);
}

static VrmlNamespace *currentScope()
{
  return currentProtoStack.empty() ?
    yyNodeTypes : currentProtoStack.front()->scope();
}


static void
beginProto(const char *protoName)
{
  // Need to push node namespace as well, since node names DEF'd in the
  // implementations are not visible (USEable) from the outside and vice
  // versa.

  // Any protos in the implementation are in a local namespace:       
  VrmlNodeType *t = new VrmlNodeType( protoName );
  t->setScope( currentScope() );
  currentProtoStack.push_front(t);
}

static void
endProto(VrmlField *url)
{
  // Make any node names defined in implementation unavailable: ...

  // Add this proto definition:
  if (currentProtoStack.empty()) {
    yyerror("Error: Empty PROTO stack");
  }
  else {
    VrmlNodeType *t = currentProtoStack.front();
    currentProtoStack.pop_front();
    if (url) t->setUrl(url, yyDocument);
    currentScope()->addNodeType( t );
  }
}

static int
inProto()
{
  return ! currentProtoStack.empty();
}


// Add a field to a PROTO interface

static FieldType
addField(const char *typeString, const char *name)
{
  FieldType type = fieldType(typeString);

  if (type == VrmlField::NO_FIELD) {
    char msg[100];
    sprintf(msg,"invalid field type: %s",typeString);
    yyerror(msg);
    return VrmlField::NO_FIELD;
  }

  // Need to add support for Script nodes:
  // if (inScript) ... ???

  if (currentProtoStack.empty()) {
    yyerror("field declaration outside of prototype");
    return VrmlField::NO_FIELD;
  }
  VrmlNodeType *t = currentProtoStack.front();
  t->addField(name, type);

  return type;
}

static FieldType
addEventIn(const char *typeString, const char *name)
{
  FieldType type = fieldType(typeString);

  if (type == VrmlField::NO_FIELD) {
    char msg[100];
    sprintf(msg,"invalid eventIn type: %s",typeString);
    yyerror(msg);

    return VrmlField::NO_FIELD;
  }

  if (currentProtoStack.empty()) {
    yyerror("eventIn declaration outside of PROTO interface");
    return VrmlField::NO_FIELD;
  }
  VrmlNodeType *t = currentProtoStack.front();
  t->addEventIn(name, type);

  return type;
}

static FieldType
addEventOut(const char *typeString, const char *name)
{
  FieldType type = fieldType(typeString);

  if (type == VrmlField::NO_FIELD) {
    char msg[100];
    sprintf(msg,"invalid eventOut type: %s",typeString);
    yyerror(msg);

    return VrmlField::NO_FIELD;
  }

  if (currentProtoStack.empty()) {
    yyerror("eventOut declaration outside of PROTO interface");
    return VrmlField::NO_FIELD;
  }
  VrmlNodeType *t = currentProtoStack.front();
  t->addEventOut(name, type);

  return type;
}

static FieldType
addExposedField(const char *typeString, const char *name)
{
  FieldType type = fieldType(typeString);

  if (type == VrmlField::NO_FIELD) {
    char msg[100];
    sprintf(msg,"invalid exposedField type: %s",typeString);
    yyerror(msg);

    return VrmlField::NO_FIELD;
  }

  if (currentProtoStack.empty()) {
    yyerror("exposedField declaration outside of PROTO interface");
    return VrmlField::NO_FIELD;
  }
  VrmlNodeType *t = currentProtoStack.front();
  t->addExposedField(name, type);

  return type;
}

static void
setFieldDefault(const char *fieldName, VrmlField *value)
{
  if (currentProtoStack.empty())
    {
      yyerror("field default declaration outside of PROTO interface");
    }
  else
    {
      VrmlNodeType *t = currentProtoStack.front();
      t->setFieldDefault(fieldName, value);
      delete value;
    }
}


static FieldType
fieldType(const char *type)
{
  return VrmlField::fieldType(type);
}


static void
enterNode(const char *nodeTypeName)
{
  const VrmlNodeType *t = currentScope()->findType( nodeTypeName );

  if (t == NULL) {
    char tmp[256];
    sprintf(tmp, "Unknown node type '%s'", nodeTypeName);
    yyerror(tmp);
  }
  FieldRec *fr = new FieldRec;

  // Create a new node of type t
  fr->node = t ? t->newNode() : 0;

  // The nodeName needs to be set here before the node contents
  // are parsed because the contents can actually reference the
  // node (eg, in ROUTE statements). USEing the nodeName from
  // inside the node is probably a bad idea, and is probably
  // illegal according to the acyclic requirement, but isn't
  // checked for...
  if (nodeName)
    {
      fr->node->setName( nodeName, currentScope() );
      nodeName = 0;
    }

  fr->nodeType = t;
  fr->fieldName = NULL;
  currentField.push_front(fr);
}

static VrmlNode *
exitNode()
{
  FieldRec *fr = currentField.front();
  //assert(fr != NULL);

  VrmlNode *n = fr->node;

  currentField.pop_front();

  delete fr;

  return n;
}

static void
enterField(const char *fieldName)
{
  FieldRec *fr = currentField.front();
  //assert(fr != NULL);

  fr->fieldName = fieldName;
  if (fr->nodeType != NULL) {

    // This is wrong - it lets eventIns/eventOuts be in nodeGuts. It
    // should only allow this when followed by IS...

    // enterField is called when parsing eventIn and eventOut IS
    // declarations, in which case we don't need to do anything special--
    // the IS IDENTIFIER will be returned from the lexer normally.
    if (fr->nodeType->hasEventIn(fieldName) ||
	fr->nodeType->hasEventOut(fieldName))
      return;
    
    fr->fieldType = fr->nodeType->hasField(fieldName);

    if (fr->fieldType != 0)
      {      // Let the lexer know what field type to expect:
	expect(fr->fieldType);
	expectCoordIndex = (strcmp(fieldName,"coordIndex") == 0);
      }
    else
      {
	char msg[256];
	sprintf(msg, "%s nodes do not have %s fields/eventIns/eventOuts",
		fr->nodeType->getName(), fieldName);
	yyerror(msg);
      }
  }
  // else expect(ANY_FIELD);
}


static void
exitField(VrmlField *fieldValue)
{
  FieldRec *fr = currentField.front();
  //assert(fr != NULL);

  if (fieldValue) fr->node->setField(fr->fieldName, *fieldValue);
  delete fieldValue;  // Assumes setField is copying fieldValue...

  fr->fieldName = NULL;
  fr->fieldType = VrmlField::NO_FIELD;
}


static bool
inScript()
{
  FieldRec *fr = currentField.front();
  if (fr->nodeType == NULL ||
      strcmp(fr->nodeType->getName(), "Script") != 0) {
    yyerror("interface declaration outside of Script");
    return false;
  }
  return true;
}


static void
addScriptEventIn(const char *typeString, const char *name)
{
  if ( inScript() )
    {
      FieldType type = fieldType(typeString);

      if (type == VrmlField::NO_FIELD)
	{
	  char msg[100];
	  sprintf(msg,"invalid eventIn type: %s",typeString);
	  yyerror(msg);
	}

      ((VrmlNodeScript*)currentField.front()->node)->addEventIn(name, type);
    }
}


static void
addScriptEventOut(const char *typeString, const char *name)
{
  if ( inScript() )
    {
      FieldType type = fieldType(typeString);

      if (type == VrmlField::NO_FIELD)
	{
	  char msg[100];
	  sprintf(msg,"invalid eventOut type: %s",typeString);
	  yyerror(msg);
	}

      ((VrmlNodeScript*)currentField.front()->node)->addEventOut(name, type);
    }
}


static void
enterScriptField(const char *typeString, const char *fieldName)
{
  if ( inScript() )
    {
      FieldRec *fr = currentField.front();
      //assert(fr != NULL);

      fr->fieldName = fieldName;
      fr->fieldType = fieldType(typeString);
      if (fr->fieldType == VrmlField::NO_FIELD)
	{
	  char msg[100];
	  sprintf(msg,"invalid Script field %s type: %s",
		  fieldName, typeString);
	  yyerror(msg);
	}
      else
	expect(fr->fieldType);
    }
}


static void
exitScriptField(VrmlField *value)
{
  if ( inScript() )
    {
      FieldRec *fr = currentField.front();
      //assert(fr != NULL);

      VrmlNodeScript *s = (VrmlNodeScript*) (fr->node);
      s->addField(fr->fieldName, fr->fieldType, value);
      delete value;
      fr->fieldName = NULL;
      fr->fieldType = VrmlField::NO_FIELD;
    }
}


// Find a node by name (in the current namespace)

static VrmlNode*
lookupNode(const char *name)
{
  return currentScope()->findNode( name );
}

static VrmlMFNode *nodeListToMFNode(vector<VrmlNode*> *nodeList)
{
  VrmlMFNode *r = 0;
  if (nodeList)
    {
      r = new VrmlMFNode(nodeList->size(), &(*nodeList)[0]);
      delete nodeList;
    }
  return r;
}

static vector<VrmlNode*> *addNodeToList(vector<VrmlNode*> *nodeList,
					VrmlNode *node)
{
  if (! nodeList)
    nodeList = new vector<VrmlNode*>();
  nodeList->push_back(node);
  return nodeList;
}


static void addNode(VrmlNode *node)
{
  if (! node) return;

  if ( inProto() )
    {
      VrmlNodeType *t = currentProtoStack.front();
      t->addNode(node);		// add node to PROTO definition
    }
  else				// top level
    {				// add node to scene graph
      if (! yyParsedNodes)
	yyParsedNodes = new VrmlMFNode(node);
      else
	yyParsedNodes->addNode(node);
    }
}


static void addRoute(const char *fromNodeName,
		     const char *fromFieldName,
		     const char *toNodeName,
		     const char *toFieldName)
{
  VrmlNode *fromNode = lookupNode(fromNodeName);
  VrmlNode *toNode = lookupNode(toNodeName);

  if (! fromNode || ! toNode)
    {
      char msg[256];
      sprintf(msg, "invalid %s node name \"%s\" in ROUTE statement.",
	      fromNode ? "destination" : "source",
	      fromNode ? toNodeName : fromNodeName);
      yyerror(msg);
    }
  else
    {
      fromNode->addRoute( fromFieldName, toNode, toFieldName );
    }
}


// Store the information linking the current field and node to
// to the PROTO interface field with the PROTO definition.

static VrmlField *addIS(const char *isFieldName)
{
  if (! isFieldName)
    {
      yyerror("invalid IS field name (null)");
      return 0;
    }

  FieldRec *fr = currentField.front();
  if (! fr )
    {
      char msg[256];
      sprintf(msg,"IS statement (%s) without field declaration",
		  isFieldName);
      yyerror(msg);
    }

  if ( inProto() )
    {
      VrmlNodeType *t = currentProtoStack.front();

      if (! t )
	{
	  yyerror("invalid PROTO for IS statement");
	  return 0;
	}
      else if (! fr->fieldName)
	{
	  char msg[256];
	  sprintf(msg,"invalid IS interface name (%s) in PROTO %s",
		  isFieldName, t->getName() );
	  yyerror(msg);
	}

      else
	t->addIS(isFieldName, fr->node, fr->fieldName);
    }

  // Not in PROTO, must be a Script field
  else if (fr->nodeType &&
	   strcmp(fr->nodeType->getName(), "Script") == 0)
    {
      return new VrmlSFNode(lookupNode(isFieldName));
    }

  else
    {
      char msg[256];
      sprintf(msg,"IS statement (%s) must be in a PROTO or Script.",
		  isFieldName);
      yyerror(msg);
    }

  // Nothing is stored for IS'd fields in the PROTO implementation
  return 0;	
}


static VrmlField *addEventIS(const char *fieldName,
			     const char *isFieldName)
{
  FieldRec *fr = currentField.front();
  if (! fr )
    {
      char msg[256];
      sprintf(msg,"IS statement (%s) with no eventIn/eventOut declaration",
		  isFieldName);
      yyerror(msg);
    }
  fr->fieldName = fieldName;
  addIS( isFieldName );
  fr->fieldName = 0;
  return 0;
}

  

// This switch is necessary so the VrmlNodeType code can be independent
// of the parser tokens.

void
expect(FieldType type)
{
  switch (type)
    {
    case VrmlField::SFBOOL: expectToken = SF_BOOL; break;
    case VrmlField::SFCOLOR: expectToken = SF_COLOR; break;
    case VrmlField::SFFLOAT: expectToken = SF_FLOAT; break;
    case VrmlField::SFIMAGE: expectToken = SF_IMAGE; break;
    case VrmlField::SFINT32: expectToken = SF_INT32; break;
    case VrmlField::SFROTATION: expectToken = SF_ROTATION; break;
    case VrmlField::SFSTRING: expectToken = SF_STRING; break;
    case VrmlField::SFTIME: expectToken = SF_TIME; break;
    case VrmlField::SFVEC2F: expectToken = SF_VEC2F; break;
    case VrmlField::SFVEC3F: expectToken = SF_VEC3F; break;

    case VrmlField::MFCOLOR: expectToken = MF_COLOR; break;
    case VrmlField::MFFLOAT: expectToken = MF_FLOAT; break;
    case VrmlField::MFINT32: expectToken = MF_INT32; break;
    case VrmlField::MFROTATION: expectToken = MF_ROTATION; break;
    case VrmlField::MFSTRING: expectToken = MF_STRING; break;
    case VrmlField::MFVEC2F: expectToken = MF_VEC2F; break;
    case VrmlField::MFVEC3F: expectToken = MF_VEC3F; break;

    case VrmlField::MFNODE: expectToken = MF_NODE; break;
    case VrmlField::SFNODE: expectToken = SF_NODE; break;
    default:
       expectToken = 0; break;
    }
}

