/**************************************************
 * VRML 2.0 Parser
 * Copyright (C) 1996 Silicon Graphics, Inc.
 *
 * Author(s)	: Gavin Bell
 *                Daniel Woods (first port)
 **************************************************
 */
%{

#include "config.h"
#include <string.h>

#include "VrmlNode.h"

#include "VrmlMFColor.h"
#include "VrmlMFFloat.h"
#include "VrmlMFInt.h"
#include "VrmlMFRotation.h"
#include "VrmlMFString.h"
#include "VrmlMFVec2f.h"
#include "VrmlMFVec3f.h"

#include "VrmlSFBool.h"
#include "VrmlSFColor.h"
#include "VrmlSFFloat.h"
#include "VrmlSFImage.h"
#include "VrmlSFInt.h"
#include "VrmlSFRotation.h"
#include "VrmlSFString.h"
#include "VrmlSFTime.h"
#include "VrmlSFVec2f.h"
#include "VrmlSFVec3f.h"

#include "parser.h"

#define YY_NO_UNPUT 1	/* Not using yyunput/yyless */


char *yyinStr = 0;			/* For input from strings */
int (*yyinFunc)(char *, int) = 0;	/* For input from functions */

#if HAVE_LIBPNG || HAVE_ZLIB
#include <zlib.h>

gzFile yygz = 0;			/* For input from gzipped files */

#define YY_INPUT(buf,result,max_size) \
	if (yyinStr) { \
		for (result=0; result<max_size && *yyinStr; ) \
		    buf[result++] = *yyinStr++; \
	} else if (yyinFunc) { \
		if ((result = (*yyinFunc)( buf, max_size )) == -1) \
		    YY_FATAL_ERROR( "cb input in flex scanner failed" ); \
	} else if (yygz) { \
		if ((result = gzread( yygz, buf, max_size )) == -1) \
		    YY_FATAL_ERROR( "gz input in flex scanner failed" ); \
	} else if ( ((result = fread( buf, 1, max_size, yyin )) == 0) \
		  && ferror( yyin ) ) \
		YY_FATAL_ERROR( "yyin input in flex scanner failed" );
#else
#define YY_INPUT(buf,result,max_size) \
	if (yyinStr) { \
		for (result=0; result<max_size && *yyinStr; ) \
		    buf[result++] = *yyinStr++; \
	} else if ( ((result = fread( buf, 1, max_size, yyin )) == 0) \
		  && ferror( yyin ) ) \
		YY_FATAL_ERROR( "input in flex scanner failed" );
#endif

	/* Current line number */
int currentLineNumber = 1;

extern void yyerror(const char *);

	/* The YACC parser sets this to a token to direct the lexer */
	/* in cases where just syntax isn't enough: */
int expectToken = 0;
int expectCoordIndex = 0;

	/* True when parsing a multiple-valued field: */
static int parsing_mf = 0;

	/* These are used when parsing SFImage fields: */
static int sfImageIntsParsed = 0;
static int sfImageIntsExpected = 0;
static int sfImageNC = 0;
static unsigned char *sfImagePixels = 0;
static int sfImageMask[] = { 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000 };

	/* These are used when parsing SFString fields: */
#define STRING_SIZE_INCR 1000
static int sfStringMax = 0;
static int sfStringN = 0;
static char *sfStringChars = 0;

static void checkStringSize(int nmore)
{
  if (sfStringN+nmore >= sfStringMax)
    {
      int incr = STRING_SIZE_INCR > nmore ? STRING_SIZE_INCR : (nmore+1);
      char *p = new char[sfStringMax += incr];
      if (sfStringChars)
	{
	  strcpy(p, sfStringChars);
	  delete [] sfStringChars;
	}
      sfStringChars = p;
    }
}

static void initString()
{
  checkStringSize(0);
  sfStringN = 0;
  sfStringChars[0] = 0;
}

	/* These are used when parsing MF* fields */

static vector<int> mfInts;
static vector<float> mfFloats;
static vector<char*> mfStrs;

#ifdef __cplusplus
extern "C"
#endif
int yywrap(void) { yyinStr = 0; yyinFunc = 0; BEGIN INITIAL; return 1; }



static char *skip_ws(char *s)
{
  while (*s == ' ' ||
	 *s == '\f' ||
	 *s == '\n' ||
	 *s == '\r' ||
	 *s == '\t' ||
	 *s == '\v' ||
	 *s == ',' ||
	 *s == '#')
    {
      if (*s == '#')
	{
	  while (*s && *s != '\n') ++s;
	}
      else
	{
	  if (*s++ == '\n') ++currentLineNumber;
	}
    }
  return s;
}


%}

	/* Normal state:  parsing nodes.  The initial start state is used */
	/* only to recognize the VRML header. */
%x NODE

	/* Start tokens for all of the field types, */
	/* except for MFNode and SFNode, which are almost completely handled */
	/* by the parser: */
%x SFB SFC SFF SFIMG SFI SFR SFS SFT SFV2 SFV3
%x MFC MFF MFI MFR MFS MFV2 MFV3
%x IN_SFS IN_MFS IN_SFIMG

	/* Big hairy expression for floating point numbers: */
float ([-+]?(([0-9]+\.?)|([0-9]*\.?[0-9]+)([eE][+\-]?[0-9]+)?)) 


	/* Ints are decimal or hex (0x##): */
int ([-+]?([0-9]+)|(0[xX][0-9a-fA-F]*))

	/* Whitespace.  Using this pattern can screw up currentLineNumber, */
	/* so it is only used wherever it is really convenient and it is */
	/* extremely unlikely that the user will put in a carriage return */
	/* (example: between the floats in an SFVec3f) */
ws ([ \t\r\n,]|(#.*))+
	/* And the same pattern without the newline */
wsnnl ([ \t\r,]|(#.*))

	/* Legal characters to start an identifier */
idStartChar ([^\x30-\x39\x00-\x20\x22\x23\x27\x2b-\x2e\x5b-\x5d\x7b\x7d])
	/* Legal other characters in an identifier */
idRestChar      ([^\x00-\x20\x22\x23\x27\x2c\x2e\x5b-\x5d\x7b\x7d])

%%

%{
	/* Switch into a new start state if the parser */
	/* just told us that we've read a field name */
	/* and should expect a field value (or IS) */
  if (expectToken != 0) {
#if DEBUG
    extern int yy_flex_debug;
    if (yy_flex_debug)
      fprintf(stderr,"LEX--> Start State %d\n", expectToken);
#endif
      
    /*
     * Annoying.  This big switch is necessary because
     * LEX wants to assign particular numbers to start
     * tokens, and YACC wants to define all the tokens
     * used, too.  Sigh.
     */
    switch(expectToken) {
    case SF_BOOL: BEGIN SFB; break;
    case SF_COLOR: BEGIN SFC; break;
    case SF_FLOAT: BEGIN SFF; break;
    case SF_IMAGE: BEGIN SFIMG; break;
    case SF_INT32: BEGIN SFI; break;
    case SF_ROTATION: BEGIN SFR; break;
    case SF_STRING: BEGIN SFS; break;
    case SF_TIME: BEGIN SFT; break;
    case SF_VEC2F: BEGIN SFV2; break;
    case SF_VEC3F: BEGIN SFV3; break;
    case MF_COLOR: BEGIN MFC; break;
    case MF_FLOAT: BEGIN MFF; break;
    case MF_INT32: BEGIN MFI; break;
    case MF_ROTATION: BEGIN MFR; break;
    case MF_STRING: BEGIN MFS; break;
    case MF_VEC2F: BEGIN MFV2; break;
    case MF_VEC3F: BEGIN MFV3; break;

      /* SFNode and MFNode are special.  Here the lexer just returns */
      /* "marker tokens" so the parser knows what type of field is */
      /* being parsed; unlike the other fields, parsing of SFNode/MFNode */
      /* field happens in the parser. */
    case MF_NODE: expectToken = 0; return MF_NODE;
    case SF_NODE: expectToken = 0; return SF_NODE;
        
    default: yyerror("ACK: Bad expectToken"); break;
    }
  }
%}

	/* This is more complicated than they really need to be because */
	/* I was ambitious and made the whitespace-matching rule aggressive */
<INITIAL>"#VRML V2.0 utf8".*\n{wsnnl}*	{ BEGIN NODE; currentLineNumber = 2; }

	/* The lexer is in the NODE state when parsing nodes, either */
	/* top-level nodes in the .wrl file, in a prototype implementation, */
	/* or when parsing the contents of SFNode or MFNode fields. */
<NODE>PROTO			{ return PROTO; }
<NODE>EXTERNPROTO		{ return EXTERNPROTO; }
<NODE>DEF			{ return DEF; }
<NODE>USE			{ return USE; }
<NODE>TO			{ return TO; }
<NODE>IS			{ return IS; }
<NODE>ROUTE			{ return ROUTE; }
<NODE>NULL			{ return SFN_NULL; }
<NODE>eventIn			{ return EVENTIN; }
<NODE>eventOut			{ return EVENTOUT; }
<NODE>field			{ return FIELD; }
<NODE>exposedField		{ return EXPOSEDFIELD; }

	/* Legal identifiers. */
<NODE>{idStartChar}{idRestChar}*	{ yylval.string = strdup(yytext);
					  return IDENTIFIER; }

	/* All fields may have an IS declaration: */
<SFB,SFC,SFF,SFIMG,SFI,SFR,SFS,SFT,SFV2,SFV3>IS		{ BEGIN NODE;
					  expectToken = 0;
					  yyless(0);
					}
<MFC,MFF,MFI,MFR,MFS,MFV2,MFV3>IS     { BEGIN NODE;
					expectToken = 0;
                                        yyless(0); /* put back the IS */
                                      }

    /* All MF field types other than MFNode are completely parsed here */
    /* in the lexer, and one token is returned to the parser.  They all */
    /* share the same rules for open and closing brackets: */
<MFC,MFF,MFI,MFR,MFS,MFV2,MFV3>\[ 	{ if (parsing_mf) yyerror("Double [");
					  parsing_mf = 1;
					  mfInts.erase(mfInts.begin(), mfInts.end());
					  mfFloats.erase(mfFloats.begin(), mfFloats.end());
					  /* parse errors can leak memory */
					  mfStrs.erase(mfStrs.begin(), mfStrs.end());
					}

<MFC,MFF,MFI,MFR,MFS,MFV2,MFV3>\]	{
					  if (! parsing_mf) yyerror("Unmatched ]");
					  int fieldType = expectToken;

					  switch (fieldType) {
					  case MF_COLOR:
					    yylval.field = new VrmlMFColor(mfFloats.size() / 3, &mfFloats[0]);
					    break;
					  case MF_FLOAT:
					    yylval.field = new VrmlMFFloat(mfFloats.size(), &mfFloats[0]);
					    break;
					  case MF_INT32:
					    if (expectCoordIndex &&
						mfInts.size() > 0 &&
						-1 != mfInts[mfInts.size()-1])
					      mfInts.push_back(-1);
					    yylval.field = new VrmlMFInt(mfInts.size(), &mfInts[0]);
					    break;
					  case MF_ROTATION:
					    yylval.field = new VrmlMFRotation(mfFloats.size() / 4, &mfFloats[0]);
					    break;
					  case MF_STRING:
					    yylval.field = new VrmlMFString(mfStrs.size(), &mfStrs[0]);
                        {
					      vector<char*>::iterator mfs;
					      for (mfs = mfStrs.begin();
						   mfs != mfStrs.end(); ++mfs)
					        delete [] (*mfs);
                        }
					    mfStrs.erase(mfStrs.begin(), mfStrs.end());
					    break;
					  case MF_VEC2F:
					    yylval.field = new VrmlMFVec2f(mfFloats.size() / 2, &mfFloats[0]);
					    break;
					  case MF_VEC3F:
					    yylval.field = new VrmlMFVec3f(mfFloats.size() / 3, &mfFloats[0]);
					    break;
					  }
					  BEGIN NODE;
					  parsing_mf = 0;
					  expectToken = 0;

					  mfFloats.erase(mfFloats.begin(), mfFloats.end());
					  mfInts.erase(mfInts.begin(), mfInts.end());
					  return fieldType;
					}


<SFB>FALSE		{ BEGIN NODE; expectToken = 0;
			  yylval.field = new VrmlSFBool(0); return SF_BOOL; }
<SFB>TRUE		{ BEGIN NODE; expectToken = 0;
			  yylval.field = new VrmlSFBool(1); return SF_BOOL; }

<SFI>{int}		{ BEGIN NODE; expectToken = 0;
			  yylval.field = new VrmlSFInt(strtol(yytext,0,0));
			  return SF_INT32;
			}

<MFI>{int}		{
			  int i = strtol(yytext,0,0);
			  if (parsing_mf)
			    mfInts.push_back(i);
			  else {    /* No open bracket means a single value: */
			    yylval.field = new VrmlMFInt(i);
			    BEGIN NODE; expectToken = 0;
			    return MF_INT32;
			  }
			}

	/* All the floating-point types are pretty similar: */
<SFF>{float}		{
			  yylval.field = new VrmlSFFloat(atof(yytext));
			  BEGIN NODE; expectToken = 0;
			  return SF_FLOAT;
			}

<MFF>{float}		{
			  float f = atof(yytext);
			  if (parsing_mf)
			    mfFloats.push_back(f);
                  	  else {    /* No open bracket means a single value: */
			    yylval.field = new VrmlMFFloat(f);
			    BEGIN NODE; expectToken = 0;
			    return MF_FLOAT;
			  }
			}

<SFV2>{float}{ws}{float}	{
				  float x = 0.0, y = 0.0;
				  int n = 0;
				  char *s = &yytext[0];

				  sscanf(s,"%g%n", &x, &n);
				  s = skip_ws(s+n);
				  sscanf(s,"%g", &y);
				  yylval.field = new VrmlSFVec2f(x,y);
				  BEGIN NODE; expectToken = 0;
				  return SF_VEC2F; 
				}

<MFV2>{float}{ws}{float}	{
				  float x = 0.0, y = 0.0;
				  int n = 0;
				  char *s = &yytext[0];

				  sscanf(s,"%g%n", &x, &n);
				  s = skip_ws(s+n);
				  sscanf(s,"%g", &y);
				  if (parsing_mf) {
				    mfFloats.push_back(x);
				    mfFloats.push_back(y);
				  } else {
				    yylval.field = new VrmlMFVec2f(x,y);
				    BEGIN NODE; expectToken = 0;
				    return MF_VEC2F;
				  }
				}

<SFV3>({float}{ws}){2}{float}	{
				  float x = 0.0, y = 0.0, z = 0.0;
				  int n = 0;
				  char *s = &yytext[0];

				  sscanf(s,"%g%n", &x, &n);
				  s = skip_ws(s+n);
				  sscanf(s,"%g%n", &y, &n);
				  s = skip_ws(s+n);
				  sscanf(s,"%g", &z);

				  yylval.field = new VrmlSFVec3f(x,y,z);

				  BEGIN NODE; expectToken = 0; 
				  return SF_VEC3F;
				}

<MFV3>({float}{ws}){2}{float}	{
				  float x = 0.0, y = 0.0, z = 0.0;
				  int n = 0;
				  char *s = &yytext[0];

				  sscanf(s,"%g%n", &x, &n);
				  s = skip_ws(s+n);
				  sscanf(s,"%g%n", &y, &n);
				  s = skip_ws(s+n);
				  sscanf(s,"%g", &z);

				  if (parsing_mf) {
				    mfFloats.push_back(x);
				    mfFloats.push_back(y);
				    mfFloats.push_back(z);
				  } else {
				    yylval.field = new VrmlMFVec3f(x,y,z);
                                    BEGIN NODE; expectToken = 0;
				    return MF_VEC3F;
                                  }
                                }

<SFR>({float}{ws}){3}{float}    {
				  float x = 0.0, y = 0.0, z = 0.0, r = 0.0;
				  int n = 0;
				  char *s = &yytext[0];
				  sscanf(s,"%g%n", &x, &n);
				  s = skip_ws(s+n);
				  sscanf(s,"%g%n", &y, &n);
				  s = skip_ws(s+n);
				  sscanf(s,"%g%n", &z, &n);
				  s = skip_ws(s+n);
				  sscanf(s,"%g", &r);
				  yylval.field = new VrmlSFRotation(x,y,z,r);
				  BEGIN NODE; expectToken = 0; 
				  return SF_ROTATION;
				}

<MFR>({float}{ws}){3}{float}	{
				  float x = 0.0, y = 0.0, z = 0.0, r = 0.0;
				  int n = 0;
				  char *s = &yytext[0];
				  sscanf(s,"%g%n", &x, &n);
				  s = skip_ws(s+n);
				  sscanf(s,"%g%n", &y, &n);
				  s = skip_ws(s+n);
				  sscanf(s,"%g%n", &z, &n);
				  s = skip_ws(s+n);
				  sscanf(s,"%g", &r);

				  if (parsing_mf) {
				    mfFloats.push_back(x);
				    mfFloats.push_back(y);
				    mfFloats.push_back(z);
				    mfFloats.push_back(r);
				  } else {
				    yylval.field = new VrmlMFRotation(x,y,z,r);
                                    BEGIN NODE; expectToken = 0;
				    return MF_ROTATION;
                                  }
                                }

<SFC>({float}{ws}){2}{float}    {
				  float r = 0.0, g = 0.0, b = 0.0;
				  int n = 0;
				  char *s = &yytext[0];
				  sscanf(s,"%g%n", &r, &n);
				  s = skip_ws(s+n);
				  sscanf(s,"%g%n", &g, &n);
				  s = skip_ws(s+n);
				  sscanf(s,"%g", &b);

				  yylval.field = new VrmlSFColor(r,g,b);
				  BEGIN NODE; expectToken = 0; 
				  return SF_COLOR;
				}

<MFC>({float}{ws}){2}{float}	{
				  float r = 0.0, g = 0.0, b = 0.0;
				  int n = 0;
				  char *s = &yytext[0];
				  sscanf(s,"%g%n", &r, &n);
				  s = skip_ws(s+n);
				  sscanf(s,"%g%n", &g, &n);
				  s = skip_ws(s+n);
				  sscanf(s,"%g", &b);

				  if (parsing_mf) {
				    mfFloats.push_back(r);
				    mfFloats.push_back(g);
				    mfFloats.push_back(b);
				  } else {
				    yylval.field = new VrmlMFColor(r,g,b);
                                    BEGIN NODE; expectToken = 0;
				    return MF_COLOR;
                                  }
                                }

<SFT>{float}	    		{
				  yylval.field = new VrmlSFTime(atof(yytext));
				  BEGIN NODE; expectToken = 0;
				  return SF_TIME;
				}

    /* SFString/MFString */
<SFS>\"				{ BEGIN IN_SFS; initString(); }
<MFS>\"				{ BEGIN IN_MFS; initString(); }

	/* Anything besides open-quote (or whitespace) is an error: */
<SFS>[^ \"\t\r\,\n]+	{ yyerror("SFString missing open-quote");
			  yylval.field = 0;
			  BEGIN NODE; expectToken = 0;
			  return SF_STRING;
                        }

    /* Expect open-quote, open-bracket, or whitespace: */
<MFS>[^ \[\]\"\t\r\,\n]+	{ yyerror("MFString missing open-quote");
				  yylval.field = 0;
				  BEGIN NODE; expectToken = 0;
				  return MF_STRING;
				}

    /* Backslashed-quotes and backslashed-backslashes are OK: */
<IN_SFS,IN_MFS>\\\"	   	{ checkStringSize(1);
                                  strcpy(sfStringChars+sfStringN++,"\""); }
<IN_SFS,IN_MFS>\\\\	   	{ checkStringSize(1);
                                  strcpy(sfStringChars+sfStringN++,"\\"); }
<IN_SFS,IN_MFS>\\n	   	{ checkStringSize(2);
                                  strcpy(sfStringChars+sfStringN, "\\n");
				  sfStringN += 2;
				}

    /* Newline characters are OK: */
<IN_SFS,IN_MFS>\n	   	{ checkStringSize(1);
                                  strcpy(sfStringChars+sfStringN++,"\n");
				  ++currentLineNumber;
				}

    /* Eat anything besides quotes, backslashed (escaped) chars and newlines. */
<IN_SFS,IN_MFS>[^\"\n\\]+	{ checkStringSize(yyleng);
				  strcpy(sfStringChars+sfStringN,yytext);
				  sfStringN += yyleng;
				}

	/* Quote ends the string: */
<IN_SFS>\"			{
				  yylval.field = new VrmlSFString(sfStringChars);
				  BEGIN NODE; expectToken = 0;
				  return SF_STRING;
				}

<IN_MFS>\"			{
				  if (parsing_mf) {
				    char *s = new char[strlen(sfStringChars)+1];
				    strcpy(s,sfStringChars);
				    mfStrs.push_back(s);
				    BEGIN MFS;
				  } else {
				    yylval.field = new VrmlMFString(sfStringChars);
				    BEGIN NODE; expectToken = 0;
				    return MF_STRING;
				  }
				}

    /* SFImage: width height numComponents then width*height integers: */
<SFIMG>{int}{ws}{int}{ws}{int}	{ int w = 0, h = 0, nc = 0, n = 0;
				  unsigned char *pixels = 0;
				  char *s = &yytext[0];

				  sscanf(s,"%d%n", &w, &n);
				  s = skip_ws(s+n);
				  sscanf(s,"%d%n", &h, &n);
				  s = skip_ws(s+n);
				  sscanf(s,"%d", &nc);

				  sfImageIntsExpected = w*h;
				  sfImageIntsParsed = 0;
				  sfImageNC = nc;

				  if (sfImageIntsExpected > 0)
				    pixels = new unsigned char[nc*w*h];

				  sfImagePixels = pixels;
				  memset(pixels,0,nc*w*h);

				  yylval.field = new VrmlSFImage(w,h,nc,pixels);

				  if (sfImageIntsExpected > 0) {
				    BEGIN IN_SFIMG;
				  } else {
				    BEGIN NODE; expectToken = 0;
				    return SF_IMAGE;
				  }
				}

<IN_SFIMG>{int}         {
			  unsigned long pixval = strtol(yytext, 0, 0);

			  int i, j = sfImageNC * sfImageIntsParsed++;
			  for (i=0; i<sfImageNC; ++i)
			    sfImagePixels[i+j] = (sfImageMask[i] & pixval) >> (8*i);
			  if (sfImageIntsParsed == sfImageIntsExpected) {
			    BEGIN NODE; expectToken = 0;
			    return SF_IMAGE;
                          }
                        }


	/* Whitespace rules apply to all start states except inside strings: */

<INITIAL,NODE,SFB,SFC,SFF,SFIMG,SFI,SFR,SFS,SFT,SFV2,SFV3,MFC,MFF,MFI,MFR,MFS,MFV2,MFV3,IN_SFIMG>{
  {wsnnl}+		;

	/* This is also whitespace, but we'll keep track of line number */
	/* to report in errors: */
  {wsnnl}*\n{wsnnl}*	{ ++currentLineNumber; }
}

	/* This catch-all rule catches anything not covered by any of */
	/* the above: */
<*>. 			{ return yytext[0]; }

%%

/* Set up to read from string s. Reading from strings skips the header */

void yystring(char *s) 
{
  yyin = 0;
#if HAVE_LIBPNG || HAVE_ZLIB
  yygz = 0;
#endif
  yyinStr = s;
  yyinFunc = 0;
  BEGIN NODE;
  expectToken = 0;
  parsing_mf = 0;
  currentLineNumber = 1;
}

/* Set up to read from function f. */

void yyfunction( int (*f)(char *, int) )
{
  yyin = 0;
#if HAVE_LIBPNG || HAVE_ZLIB
  yygz = 0;
#endif
  yyinStr = 0;
  yyinFunc = f;
  BEGIN INITIAL;
  expectToken = 0;
  parsing_mf = 0;
  currentLineNumber = 1;
}

