
/*  A Bison parser, made from parser.yy
 by  GNU Bison version 1.25
  */

#define YYBISON 1  /* Identify Bison output.  */

#define	IDENTIFIER	258
#define	DEF	259
#define	USE	260
#define	PROTO	261
#define	EXTERNPROTO	262
#define	TO	263
#define	IS	264
#define	ROUTE	265
#define	SFN_NULL	266
#define	EVENTIN	267
#define	EVENTOUT	268
#define	FIELD	269
#define	EXPOSEDFIELD	270
#define	SF_BOOL	271
#define	SF_COLOR	272
#define	SF_FLOAT	273
#define	SF_INT32	274
#define	SF_ROTATION	275
#define	SF_TIME	276
#define	SF_IMAGE	277
#define	SF_STRING	278
#define	SF_VEC2F	279
#define	SF_VEC3F	280
#define	MF_COLOR	281
#define	MF_FLOAT	282
#define	MF_INT32	283
#define	MF_ROTATION	284
#define	MF_STRING	285
#define	MF_VEC2F	286
#define	MF_VEC3F	287
#define	SF_NODE	288
#define	MF_NODE	289

#line 9 "parser.yy"


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


#line 124 "parser.yy"
typedef union {
	char *string;
	VrmlField *field;
	VrmlNode *node;
        vector<VrmlNode *> *nodeList;
} YYSTYPE;
#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		133
#define	YYFLAG		-32768
#define	YYNTBASE	40

#define YYTRANSLATE(x) ((unsigned)(x) <= 289 ? yytranslate[x] : 69)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,    39,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
    35,     2,    36,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    37,     2,    38,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     1,     2,     3,     4,     5,
     6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
    16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
    26,    27,    28,    29,    30,    31,    32,    33,    34
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     2,     3,     6,     8,    10,    12,    14,    15,    20,
    23,    25,    27,    28,    38,    39,    40,    49,    50,    53,
    57,    61,    62,    68,    69,    75,    76,    79,    83,    87,
    91,    95,   104,   105,   111,   112,   115,   116,   120,   122,
   124,   128,   132,   133,   139,   140,   147,   148,   155,   157,
   159,   161,   163,   165,   167,   169,   171,   173,   175,   177,
   179,   181,   183,   185,   187,   189,   192,   195,   198,   201,
   205,   209,   213,   215,   216
};

static const short yyrhs[] = {    41,
     0,     0,    41,    42,     0,    43,     0,    45,     0,    57,
     0,    58,     0,     0,     4,     3,    44,    58,     0,     5,
     3,     0,    46,     0,    48,     0,     0,     6,     3,    47,
    35,    51,    36,    37,    41,    38,     0,     0,     0,     7,
     3,    49,    35,    55,    36,    50,    66,     0,     0,    51,
    52,     0,    12,     3,     3,     0,    13,     3,     3,     0,
     0,    14,     3,     3,    53,    66,     0,     0,    15,     3,
     3,    54,    66,     0,     0,    55,    56,     0,    12,     3,
     3,     0,    13,     3,     3,     0,    14,     3,     3,     0,
    15,     3,     3,     0,    10,     3,    39,     3,     8,     3,
    39,     3,     0,     0,     3,    59,    37,    60,    38,     0,
     0,    60,    61,     0,     0,     3,    62,    66,     0,    57,
     0,    45,     0,    12,     3,     3,     0,    13,     3,     3,
     0,     0,    14,     3,     3,    63,    66,     0,     0,    12,
     3,     3,    64,     9,     3,     0,     0,    13,     3,     3,
    65,     9,     3,     0,    26,     0,    27,     0,    28,     0,
    29,     0,    30,     0,    31,     0,    32,     0,    16,     0,
    17,     0,    18,     0,    22,     0,    19,     0,    20,     0,
    23,     0,    21,     0,    24,     0,    25,     0,    33,    43,
     0,    33,    11,     0,    34,    67,     0,     9,     3,     0,
    33,     9,     3,     0,    34,     9,     3,     0,    35,    68,
    36,     0,    43,     0,     0,    68,    43,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
   152,   155,   157,   160,   162,   163,   166,   168,   169,   170,
   173,   175,   178,   180,   184,   186,   188,   191,   193,   196,
   199,   201,   202,   204,   205,   209,   211,   214,   217,   219,
   221,   225,   231,   233,   236,   238,   241,   243,   244,   245,
   248,   250,   253,   254,   256,   257,   258,   259,   262,   264,
   265,   266,   267,   268,   269,   271,   272,   273,   274,   275,
   276,   277,   278,   279,   280,   282,   283,   284,   286,   287,
   288,   292,   294,   297,   299
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","IDENTIFIER",
"DEF","USE","PROTO","EXTERNPROTO","TO","IS","ROUTE","SFN_NULL","EVENTIN","EVENTOUT",
"FIELD","EXPOSEDFIELD","SF_BOOL","SF_COLOR","SF_FLOAT","SF_INT32","SF_ROTATION",
"SF_TIME","SF_IMAGE","SF_STRING","SF_VEC2F","SF_VEC3F","MF_COLOR","MF_FLOAT",
"MF_INT32","MF_ROTATION","MF_STRING","MF_VEC2F","MF_VEC3F","SF_NODE","MF_NODE",
"'['","']'","'{'","'}'","'.'","vrmlscene","declarations","declaration","nodeDeclaration",
"@1","protoDeclaration","proto","@2","externproto","@3","@4","interfaceDeclarations",
"interfaceDeclaration","@5","@6","externInterfaceDeclarations","externInterfaceDeclaration",
"routeDeclaration","node","@7","nodeGuts","nodeGut","@8","@9","@10","@11","fieldValue",
"mfnodeValue","nodes", NULL
};
#endif

static const short yyr1[] = {     0,
    40,    41,    41,    42,    42,    42,    43,    44,    43,    43,
    45,    45,    47,    46,    49,    50,    48,    51,    51,    52,
    52,    53,    52,    54,    52,    55,    55,    56,    56,    56,
    56,    57,    59,    58,    60,    60,    62,    61,    61,    61,
    61,    61,    63,    61,    64,    61,    65,    61,    66,    66,
    66,    66,    66,    66,    66,    66,    66,    66,    66,    66,
    66,    66,    66,    66,    66,    66,    66,    66,    66,    66,
    66,    67,    67,    68,    68
};

static const short yyr2[] = {     0,
     1,     0,     2,     1,     1,     1,     1,     0,     4,     2,
     1,     1,     0,     9,     0,     0,     8,     0,     2,     3,
     3,     0,     5,     0,     5,     0,     2,     3,     3,     3,
     3,     8,     0,     5,     0,     2,     0,     3,     1,     1,
     3,     3,     0,     5,     0,     6,     0,     6,     1,     1,
     1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
     1,     1,     1,     1,     1,     2,     2,     2,     2,     3,
     3,     3,     1,     0,     2
};

static const short yydefact[] = {     2,
     1,    33,     0,     0,     0,     0,     0,     3,     4,     5,
    11,    12,     6,     7,     0,     8,    10,    13,    15,     0,
    35,     0,     0,     0,     0,     0,     9,    18,    26,     0,
    37,     0,     0,     0,    34,    40,    39,    36,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    19,     0,     0,     0,     0,    16,    27,     0,     0,    56,
    57,    58,    60,    61,    63,    59,    62,    64,    65,    49,
    50,    51,    52,    53,    54,    55,     0,     0,    38,    41,
    42,    43,     0,     0,     0,     0,     2,     0,     0,     0,
     0,     0,     0,    69,     0,    67,    66,     0,    74,    73,
    68,     0,     0,     0,    20,    21,    22,    24,     0,    28,
    29,    30,    31,    17,    32,    70,    71,     0,     0,     0,
    44,     0,     0,    14,    72,    75,    46,    48,    23,    25,
     0,     0,     0
};

static const short yydefgoto[] = {   131,
     1,     8,     9,    22,    10,    11,    23,    12,    24,    92,
    39,    51,   122,   123,    40,    57,    13,    14,    15,    26,
    38,    42,   104,   102,   103,    79,   101,   118
};

static const short yypact[] = {-32768,
    78,-32768,     0,     1,     4,     7,    16,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,    -9,-32768,-32768,-32768,-32768,   -10,
-32768,    27,     5,    10,    33,    -1,-32768,-32768,-32768,    38,
-32768,    44,    49,    50,-32768,-32768,-32768,-32768,    20,    65,
    51,    42,    83,    84,    86,    87,    88,    89,    90,    57,
-32768,    92,    93,    94,    95,-32768,-32768,    60,    97,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,    39,    22,-32768,    96,
    98,-32768,    99,   100,   101,   103,-32768,   105,   106,   107,
   108,    42,   109,-32768,   110,-32768,-32768,   111,-32768,-32768,
-32768,   112,   113,    42,-32768,-32768,-32768,-32768,    11,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,    19,   114,   115,
-32768,    42,    42,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
   116,   119,-32768
};

static const short yypgoto[] = {-32768,
    28,-32768,   -77,-32768,   102,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,   104,   117,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,   -84,-32768,-32768
};


#define	YYLAST		139


static const short yytable[] = {    97,
   100,    31,    16,    17,     5,     6,    18,   114,     7,    19,
    32,    33,    34,     2,     3,     4,     5,     6,    20,   121,
     7,     2,     3,     4,     2,     3,     4,    21,    25,     2,
    98,    46,    47,    48,    49,    30,    35,   129,   130,    28,
   126,     2,     3,     4,    29,    41,    43,    95,   124,    96,
    59,    44,    45,    58,   125,    50,    99,    60,    61,    62,
    63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
    73,    74,    75,    76,    77,    78,    52,    53,    54,    55,
     2,     3,     4,     5,     6,    80,    81,     7,    82,    83,
    84,    85,    86,    87,    88,    89,    90,    91,    93,    94,
    56,   105,   106,   107,   -45,   108,   -47,   110,   111,   112,
   113,   115,   116,   117,   109,   132,   127,   128,   133,     0,
   119,   120,     0,     0,     0,     0,     0,    36,     0,    37,
     0,     0,     0,     0,     0,     0,     0,     0,    27
};

static const short yycheck[] = {    77,
    78,     3,     3,     3,     6,     7,     3,    92,    10,     3,
    12,    13,    14,     3,     4,     5,     6,     7,     3,   104,
    10,     3,     4,     5,     3,     4,     5,    37,    39,     3,
     9,    12,    13,    14,    15,     3,    38,   122,   123,    35,
   118,     3,     4,     5,    35,     8,     3,     9,    38,    11,
     9,     3,     3,     3,    36,    36,    35,    16,    17,    18,
    19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
    29,    30,    31,    32,    33,    34,    12,    13,    14,    15,
     3,     4,     5,     6,     7,     3,     3,    10,     3,     3,
     3,     3,     3,    37,     3,     3,     3,     3,    39,     3,
    36,     3,     3,     3,     9,     3,     9,     3,     3,     3,
     3,     3,     3,     3,    87,     0,     3,     3,     0,    -1,
     9,     9,    -1,    -1,    -1,    -1,    -1,    26,    -1,    26,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    22
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "bison.simple"

/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

#ifndef alloca
#ifdef __GNUC__
#define alloca __builtin_alloca
#else /* not GNU C.  */
#if (!defined (__STDC__) && defined (sparc)) || defined (__sparc__) || defined (__sparc) || defined (__sgi)
#include <alloca.h>
#else /* not sparc */
#if defined (MSDOS) && !defined (__TURBOC__)
#include <malloc.h>
#else /* not MSDOS, or __TURBOC__ */
#if defined(_AIX)
#include <malloc.h>
 #pragma alloca
#else /* not MSDOS, __TURBOC__, or _AIX */
#ifdef __hpux
#ifdef __cplusplus
extern "C" {
void *alloca (unsigned int);
};
#else /* not __cplusplus */
void *alloca ();
#endif /* not __cplusplus */
#endif /* __hpux */
#endif /* not _AIX */
#endif /* not MSDOS, or __TURBOC__ */
#endif /* not sparc.  */
#endif /* not GNU C.  */
#endif /* alloca not defined.  */

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

/* Note: there must be only one dollar sign in this file.
   It is replaced by the list of actions, each action
   as one case of the switch.  */

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	return(0)
#define YYABORT 	return(1)
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(token, value) \
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    { yychar = (token), yylval = (value);			\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { yyerror ("syntax error: cannot back up"); YYERROR; }	\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

#ifndef YYPURE
#define YYLEX		yylex()
#endif

#ifdef YYPURE
#ifdef YYLSP_NEEDED
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, &yylloc, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval, &yylloc)
#endif
#else /* not YYLSP_NEEDED */
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval)
#endif
#endif /* not YYLSP_NEEDED */
#endif

/* If nonreentrant, generate the variables here */

#ifndef YYPURE

int	yychar;			/*  the lookahead symbol		*/
YYSTYPE	yylval;			/*  the semantic value of the		*/
				/*  lookahead symbol			*/

#ifdef YYLSP_NEEDED
YYLTYPE yylloc;			/*  location data for the lookahead	*/
				/*  symbol				*/
#endif

int yynerrs;			/*  number of parse errors so far       */
#endif  /* not YYPURE */

#if YYDEBUG != 0
int yydebug;			/*  nonzero means print parse trace	*/
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif

/*  YYINITDEPTH indicates the initial size of the parser's stacks	*/

#ifndef	YYINITDEPTH
#define YYINITDEPTH 200
#endif

/*  YYMAXDEPTH is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#if YYMAXDEPTH == 0
#undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
int yyparse (void);
#endif

#if __GNUC__ > 1		/* GNU C and GNU C++ define this.  */
#define __yy_memcpy(TO,FROM,COUNT)	__builtin_memcpy(TO,FROM,COUNT)
#else				/* not GNU C or C++ */
#ifndef __cplusplus

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (to, from, count)
     char *to;
     char *from;
     int count;
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#else /* __cplusplus */

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (char *to, char *from, int count)
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#endif
#endif

#line 196 "bison.simple"

/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
#ifdef __cplusplus
#define YYPARSE_PARAM_ARG void *YYPARSE_PARAM
#define YYPARSE_PARAM_DECL
#else /* not __cplusplus */
#define YYPARSE_PARAM_ARG YYPARSE_PARAM
#define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
#endif /* not __cplusplus */
#else /* not YYPARSE_PARAM */
#define YYPARSE_PARAM_ARG
#define YYPARSE_PARAM_DECL
#endif /* not YYPARSE_PARAM */

int
yyparse(YYPARSE_PARAM_ARG)
     YYPARSE_PARAM_DECL
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YYSTYPE *yyvsp;
  int yyerrstatus;	/*  number of tokens to shift before error messages enabled */
  int yychar1 = 0;		/*  lookahead token as an internal (translated) token number */

  short	yyssa[YYINITDEPTH];	/*  the state stack			*/
  YYSTYPE yyvsa[YYINITDEPTH];	/*  the semantic value stack		*/

  short *yyss = yyssa;		/*  refer to the stacks thru separate pointers */
  YYSTYPE *yyvs = yyvsa;	/*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YYLSP_NEEDED
  YYLTYPE yylsa[YYINITDEPTH];	/*  the location stack			*/
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  int yystacksize = YYINITDEPTH;

#ifdef YYPURE
  int yychar;
  YYSTYPE yylval;
  int yynerrs;
#ifdef YYLSP_NEEDED
  YYLTYPE yylloc;
#endif
#endif

  YYSTYPE yyval;		/*  the variable used to return		*/
				/*  semantic values from the action	*/
				/*  routines				*/

  int yylen;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Starting parse\n");
#endif

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
#ifdef YYLSP_NEEDED
  yylsp = yyls;
#endif

/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
yynewstate:

  *++yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YYSTYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YYLSP_NEEDED
      YYLTYPE *yyls1 = yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
#ifdef YYLSP_NEEDED
      /* This used to be a conditional around just the two extra args,
	 but that might be undefined if yyoverflow is a macro.  */
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yyls1, size * sizeof (*yylsp),
		 &yystacksize);
#else
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yystacksize);
#endif

      yyss = yyss1; yyvs = yyvs1;
#ifdef YYLSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	{
	  yyerror("parser stack overflow");
	  return 2;
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
      yyss = (short *) alloca (yystacksize * sizeof (*yyssp));
      __yy_memcpy ((char *)yyss, (char *)yyss1, size * sizeof (*yyssp));
      yyvs = (YYSTYPE *) alloca (yystacksize * sizeof (*yyvsp));
      __yy_memcpy ((char *)yyvs, (char *)yyvs1, size * sizeof (*yyvsp));
#ifdef YYLSP_NEEDED
      yyls = (YYLTYPE *) alloca (yystacksize * sizeof (*yylsp));
      __yy_memcpy ((char *)yyls, (char *)yyls1, size * sizeof (*yylsp));
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YYLSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Entering state %d\n", yystate);
#endif

  goto yybackup;
 yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Reading a token: ");
#endif
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(yychar);

#if YYDEBUG != 0
      if (yydebug)
	{
	  fprintf (stderr, "Next token is %d (%s", yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
#endif
	  fprintf (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting token %d (%s), ", yychar, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (yyerrstatus) yyerrstatus--;

  yystate = yyn;
  goto yynewstate;

/* Do the default action for the current state.  */
yydefault:

  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;

/* Do a reduction.  yyn is the number of a rule to reduce with.  */
yyreduce:
  yylen = yyr2[yyn];
  if (yylen > 0)
    yyval = yyvsp[1-yylen]; /* implement default value of the action */

#if YYDEBUG != 0
  if (yydebug)
    {
      int i;

      fprintf (stderr, "Reducing via rule %d (line %d), ",
	       yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (i = yyprhs[yyn]; yyrhs[i] > 0; i++)
	fprintf (stderr, "%s ", yytname[yyrhs[i]]);
      fprintf (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif


  switch (yyn) {

case 4:
#line 161 "parser.yy"
{ addNode(yyvsp[0].node); ;
    break;}
case 8:
#line 168 "parser.yy"
{ nodeName = yyvsp[0].string; ;
    break;}
case 9:
#line 169 "parser.yy"
{ yyval.node = yyvsp[0].node; free(yyvsp[-2].string); ;
    break;}
case 10:
#line 170 "parser.yy"
{ yyval.node = lookupNode(yyvsp[0].string); free(yyvsp[0].string); ;
    break;}
case 13:
#line 179 "parser.yy"
{ beginProto(yyvsp[0].string); ;
    break;}
case 14:
#line 181 "parser.yy"
{ endProto(0); free(yyvsp[-7].string);;
    break;}
case 15:
#line 185 "parser.yy"
{ beginProto(yyvsp[0].string); ;
    break;}
case 16:
#line 187 "parser.yy"
{ expect(VrmlField::MFSTRING); ;
    break;}
case 17:
#line 188 "parser.yy"
{ endProto(yyvsp[0].field); free(yyvsp[-6].string); ;
    break;}
case 20:
#line 197 "parser.yy"
{ addEventIn(yyvsp[-1].string, yyvsp[0].string);
							  free(yyvsp[-1].string); free(yyvsp[0].string); ;
    break;}
case 21:
#line 199 "parser.yy"
{ addEventOut(yyvsp[-1].string, yyvsp[0].string);
							  free(yyvsp[-1].string); free(yyvsp[0].string); ;
    break;}
case 22:
#line 201 "parser.yy"
{ expect(addField(yyvsp[-1].string,yyvsp[0].string)); ;
    break;}
case 23:
#line 202 "parser.yy"
{ setFieldDefault(yyvsp[-2].string, yyvsp[0].field);
							  free(yyvsp[-3].string); free(yyvsp[-2].string); ;
    break;}
case 24:
#line 204 "parser.yy"
{ expect(addExposedField(yyvsp[-1].string,yyvsp[0].string)); ;
    break;}
case 25:
#line 205 "parser.yy"
{ setFieldDefault(yyvsp[-2].string, yyvsp[0].field);
							  free(yyvsp[-3].string); free(yyvsp[-2].string); ;
    break;}
case 28:
#line 215 "parser.yy"
{ addEventIn(yyvsp[-1].string, yyvsp[0].string);
						  free(yyvsp[-1].string); free(yyvsp[0].string); ;
    break;}
case 29:
#line 217 "parser.yy"
{ addEventOut(yyvsp[-1].string, yyvsp[0].string);
        					  free(yyvsp[-1].string); free(yyvsp[0].string); ;
    break;}
case 30:
#line 219 "parser.yy"
{ addField(yyvsp[-1].string, yyvsp[0].string);
        					  free(yyvsp[-1].string); free(yyvsp[0].string); ;
    break;}
case 31:
#line 221 "parser.yy"
{ addExposedField(yyvsp[-1].string, yyvsp[0].string);
        					  free(yyvsp[-1].string); free(yyvsp[0].string); ;
    break;}
case 32:
#line 227 "parser.yy"
{ addRoute(yyvsp[-6].string, yyvsp[-4].string, yyvsp[-2].string, yyvsp[0].string);
		  free(yyvsp[-6].string); free(yyvsp[-4].string); free(yyvsp[-2].string); free(yyvsp[0].string); ;
    break;}
case 33:
#line 232 "parser.yy"
{ enterNode(yyvsp[0].string); ;
    break;}
case 34:
#line 233 "parser.yy"
{ yyval.node = exitNode(); free(yyvsp[-4].string);;
    break;}
case 37:
#line 242 "parser.yy"
{ enterField(yyvsp[0].string); ;
    break;}
case 38:
#line 243 "parser.yy"
{ exitField(yyvsp[0].field); free(yyvsp[-2].string); ;
    break;}
case 41:
#line 248 "parser.yy"
{ addScriptEventIn(yyvsp[-1].string,yyvsp[0].string);
    						  free(yyvsp[-1].string); free(yyvsp[0].string); ;
    break;}
case 42:
#line 250 "parser.yy"
{ addScriptEventOut(yyvsp[-1].string, yyvsp[0].string);
    						  free(yyvsp[-1].string); free(yyvsp[0].string); ;
    break;}
case 43:
#line 253 "parser.yy"
{ enterScriptField(yyvsp[-1].string, yyvsp[0].string); ;
    break;}
case 44:
#line 254 "parser.yy"
{ exitScriptField(yyvsp[0].field);
    						  free(yyvsp[-3].string); free(yyvsp[-2].string); ;
    break;}
case 45:
#line 256 "parser.yy"
{ addScriptEventIn(yyvsp[-1].string,yyvsp[0].string); ;
    break;}
case 46:
#line 257 "parser.yy"
{ addEventIS(yyvsp[-3].string,yyvsp[0].string); free(yyvsp[-4].string); free(yyvsp[-3].string); free(yyvsp[0].string); ;
    break;}
case 47:
#line 258 "parser.yy"
{ addScriptEventOut(yyvsp[-1].string,yyvsp[0].string); ;
    break;}
case 48:
#line 259 "parser.yy"
{ addEventIS(yyvsp[-3].string,yyvsp[0].string); free(yyvsp[-4].string); free(yyvsp[-3].string); free(yyvsp[0].string); ;
    break;}
case 66:
#line 282 "parser.yy"
{ yyval.field = new VrmlSFNode(yyvsp[0].node); ;
    break;}
case 67:
#line 283 "parser.yy"
{ yyval.field = 0; ;
    break;}
case 68:
#line 284 "parser.yy"
{ yyval.field = yyvsp[0].field; ;
    break;}
case 69:
#line 286 "parser.yy"
{ yyval.field = addIS(yyvsp[0].string); free(yyvsp[0].string); ;
    break;}
case 70:
#line 287 "parser.yy"
{ yyval.field = addIS(yyvsp[0].string); free(yyvsp[0].string); ;
    break;}
case 71:
#line 288 "parser.yy"
{ yyval.field = addIS(yyvsp[0].string); free(yyvsp[0].string); ;
    break;}
case 72:
#line 293 "parser.yy"
{ yyval.field = nodeListToMFNode(yyvsp[-1].nodeList); ;
    break;}
case 73:
#line 294 "parser.yy"
{ yyval.field = new VrmlMFNode(yyvsp[0].node); ;
    break;}
case 74:
#line 298 "parser.yy"
{ yyval.nodeList = 0; ;
    break;}
case 75:
#line 299 "parser.yy"
{ yyval.nodeList = addNodeToList(yyvsp[-1].nodeList,yyvsp[0].node); ;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 498 "bison.simple"

  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YYLSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = yylloc.first_line;
      yylsp->first_column = yylloc.first_column;
      yylsp->last_line = (yylsp-1)->last_line;
      yylsp->last_column = (yylsp-1)->last_column;
      yylsp->text = 0;
    }
  else
    {
      yylsp->last_line = (yylsp+yylen-1)->last_line;
      yylsp->last_column = (yylsp+yylen-1)->last_column;
    }
#endif

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;

yyerrlab:   /* here on detecting error */

  if (! yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  int size = 0;
	  char *msg;
	  int x, count;

	  count = 0;
	  /* Start X at -yyn if nec to avoid negative indexes in yycheck.  */
	  for (x = (yyn < 0 ? -yyn : 0);
	       x < (sizeof(yytname) / sizeof(char *)); x++)
	    if (yycheck[x + yyn] == x)
	      size += strlen(yytname[x]) + 15, count++;
	  msg = (char *) malloc(size + 15);
	  if (msg != 0)
	    {
	      strcpy(msg, "parse error");

	      if (count < 5)
		{
		  count = 0;
		  for (x = (yyn < 0 ? -yyn : 0);
		       x < (sizeof(yytname) / sizeof(char *)); x++)
		    if (yycheck[x + yyn] == x)
		      {
			strcat(msg, count == 0 ? ", expecting `" : " or `");
			strcat(msg, yytname[x]);
			strcat(msg, "'");
			count++;
		      }
		}
	      yyerror(msg);
	      free(msg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror("parse error");
    }

  goto yyerrlab1;
yyerrlab1:   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Discarding token %d (%s).\n", yychar, yytname[yychar1]);
#endif

      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;

yyerrdefault:  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (yyn) goto yydefault;
#endif

yyerrpop:   /* pop the current state because it cannot handle the error token */

  if (yyssp == yyss) YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#ifdef YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

yyerrhandle:

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;
}
#line 302 "parser.yy"


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

