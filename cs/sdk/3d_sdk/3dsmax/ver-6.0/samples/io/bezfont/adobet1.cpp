/**********************************************************************
 *<
	FILE: adobet1.cpp

	DESCRIPTION:  Adobe Type 1 bezier font file import module

	CREATED BY: Tom Hudson

	HISTORY: created 2 November 1995

 *>	Copyright (c) 1995, All Rights Reserved.
 **********************************************************************/

#include "Max.h"
#include <stdio.h>
#include <direct.h>
#include <commdlg.h>
#include <setjmp.h>
#include "splshape.h"
#include "bezfont.h"
#include "type1.h"
#include "at1rc.h"

// Uncomment the following for debugging prints
//#define DBGAT1

TCHAR *GetString(int id);

HINSTANCE hInstance;
char gp_buffer[256];

#define ADOBET1_LOADER_CLASS_ID 0x1810

// A table of generic pointers
typedef Tab<void *> PtrList;

// Our open font data storage class
class Type1OpenFont {
	public:
		BEZFONTHANDLE handle;	// The handle theBezFontManager has assigned this font
		PtrList ptrs;			// The allocations used by this font		
		Type1_font data;		// The actual font
		Type1OpenFont() {}
		~Type1OpenFont();
		void AddAlloc(void *ptr);
		void *Alloc(int size, int number=1);
	};

Type1OpenFont::~Type1OpenFont() {
	int count = ptrs.Count();
	for(int i = 0; i < count; ++i)
		free(ptrs[i]);
	ptrs.Delete(0, count);
	}

void Type1OpenFont::AddAlloc(void *ptr) {
	ptrs.Append(1, &ptr);
	}

void *Type1OpenFont::Alloc(int size, int number) {
	int bytes = size * number;
	void *ptr = malloc(bytes);
	if(ptr)
		AddAlloc(ptr);
	// Zero out the memory
	char *z = (char *)ptr;
	for(int i=0; i < bytes; ++i)
		*z++ = 0;
	return ptr;
	}

typedef Tab<Type1OpenFont *> Type1OpenFontTab;

/*

		Read and decrypt an Adobe Type 1 font.
		This code was written originally by John Walker
		using the _Adobe Type 1 Font Format 1.1_ published
		by Addison-Wesley ISBN 0-201-57044-0 as a guide.

		Jim Kent changed it a fair amount,  reformatting the
		indentation and renaming some of the identifiers to
		mix with the local style;  making the output go through
		functions embedded in a structure so the same interpreter 
		could be used for both sizing and drawing the font,  and
		putting in stuff to glue it into Animator Pro's font
		manager.  Also changed from line oriented parsing to token
		oriented parsing to accomodate some PD .PFB files.

		There are four main sections to this file set apart with
		long comment blocks.  One reads the font into memory.
		The second interprets the font language.  The third is concerned
		with directing the output of the interpreter into a form
		useful for PJ.  The final bit is the glue into the PJ
		virtual font system.

*/

#define DBGAT3

char t1_fullname[256];		/* Font name */
int t1_chars;			/* Chars in font */
int t1_width;			/* Current char width */
int t1_minx,t1_miny,t1_maxx,t1_maxy;

#define EOS     '\0'

#define X   0
#define Y   1


/*****************************************************************************
 *****************************************************************************
 ** The Load Section.  A PostScript Type 1 file is composed of a list of
 ** definitions.  Each definition is a keyword followed by data.  
 ** At the start of the file are a bunch of definitions that are about the
 ** font as a whole.  Then we come to the individual letters,   which are
 ** encrypted and in their own little RPN language.
 **
 ** The load section is concerned with verifying that the file is indeed
 ** a PostScript Type 1,  extracting a few things from the 
 ** pre-letter definitions,  and then decrypting
 ** the individual letters and the subroutines they use and sticking them 
 ** in a couple of arrays for later access.
 *****************************************************************************
 ****************************************************************************/

/*****************************************************************************
 * Let's handle errors (not enough memory, bad data in file, etc.) during
 * reading with a setjmp/longjmp.  The longjmp destination will be the
 * highest level read routine (read_font).
 ****************************************************************************/
static jmp_buf type1_load_errhandler; /* Jump buffer for load errors.	*/

/*static*/ void type1_load_error(char *msg)
/*
 * format & output an error message, then longjump to error handler.
 */
{
DebugPrint("AT1:%s\n",msg);
longjmp(type1_load_errhandler, Err_reported);
}


/*  Sections of the font file.  */
typedef enum { Header, FontInfo, OtherSubrs, Subrs, CharStrings } file_section;
static file_section section;

#define MAP_SIZE 256

/*  Map of PostScript character names to IBM code page 850 (multilingual) */
static char *isomap[MAP_SIZE] = {
	NULL,				 /*  00 */
	NULL,				 /*  01 */
	NULL,				 /*  02 */
	NULL,				 /*  03 */
	NULL,				 /*  04 */
	NULL,				 /*  05 */
	NULL,				 /*  06 */
	NULL,				 /*  07 */
	NULL,				 /*  08 */
	NULL,				 /*  09 */
	NULL,				 /*  10 */
	"ordmasculine",		 /*  11 */
	"ordfeminine",		 /*  12 */
	NULL,				 /*  13 */
	NULL,				 /*  14 */
	NULL,				 /*  15 */
	NULL,				 /*  16 */
	NULL,				 /*  17 */
	NULL,				 /*  18 */
	NULL,				 /*  19 */
	"paragraph",         /*  20 */
	"section",			 /*  21 */
	NULL,				 /*  22 */
	NULL,				 /*  23 */
	NULL,				 /*  24 */
	NULL,				 /*  25 */
	NULL,				 /*  26 */
	NULL,				 /*  27 */
	NULL,				 /*  28 */
	NULL,				 /*  29 */
	NULL,				 /*  30 */
	NULL,				 /*  31 */
    "space",             /*  32 */
    "exclam",            /*  33 */
    "quotedbl",          /*  34 */
    "numbersign",        /*  35 */
    "dollar",            /*  36 */
    "percent",           /*  37 */
    "ampersand",         /*  38 */
    "quotesingle",        /*  39 */		/* ?"quoteright"? */
    "parenleft",         /*  40 */
    "parenright",        /*  41 */
    "asterisk",          /*  42 */
    "plus",              /*  43 */
    "comma",             /*  44 */
    "hyphen",            /*  45 */
    "period",            /*  46 */
    "slash",             /*  47 */
    "zero",              /*  48 */
    "one",               /*  49 */
    "two",               /*  50 */
    "three",             /*  51 */
    "four",              /*  52 */
    "five",              /*  53 */
    "six",               /*  54 */
    "seven",             /*  55 */
    "eight",             /*  56 */
    "nine",              /*  57 */
    "colon",             /*  58 */
    "semicolon",         /*  59 */
    "less",              /*  60 */
    "equal",             /*  61 */
	"greater",           /*  62 */
    "question",          /*  63 */
    "at",                /*  64 */
    "A",                 /*  65 */
    "B",                 /*  66 */
    "C",                 /*  67 */
    "D",                 /*  68 */
    "E",                 /*  69 */
    "F",                 /*  70 */
    "G",                 /*  71 */
    "H",                 /*  72 */
    "I",                 /*  73 */
    "J",                 /*  74 */
    "K",                 /*  75 */
    "L",                 /*  76 */
    "M",                 /*  77 */
    "N",                 /*  78 */
    "O",                 /*  79 */
    "P",                 /*  80 */
    "Q",                 /*  81 */
    "R",                 /*  82 */
    "S",                 /*  83 */
    "T",                 /*  84 */
    "U",                 /*  85 */
    "V",                 /*  86 */
    "W",                 /*  87 */
    "X",                 /*  88 */
    "Y",                 /*  89 */
    "Z",                 /*  90 */
    "bracketleft",       /*  91 */
    "backslash",         /*  92 */
    "bracketright",      /*  93 */
    "asciicircum",       /*  94 */
    "underscore",        /*  95 */
    "grave",             /*  96 */
	"a",                 /*  97 */
    "b",                 /*  98 */
    "c",                 /*  99 */
    "d",                 /* 100 */
    "e",                 /* 101 */
    "f",                 /* 102 */
    "g",                 /* 103 */
    "h",                 /* 104 */
    "i",                 /* 105 */
    "j",                 /* 106 */
    "k",                 /* 107 */
    "l",                 /* 108 */
    "m",                 /* 109 */
    "n",                 /* 110 */
    "o",                 /* 111 */
    "p",                 /* 112 */
    "q",                 /* 113 */
    "r",                 /* 114 */
    "s",                 /* 115 */
    "t",                 /* 116 */
    "u",                 /* 117 */
    "v",                 /* 118 */
    "w",                 /* 119 */
    "x",                 /* 120 */
    "y",                 /* 121 */
    "z",                 /* 122 */
    "braceleft",         /* 123 */
    "bar",               /* 124 */
    "braceright",        /* 125 */
    "asciitilde",        /* 126 */
    NULL,            	 /* 127 */
    "Ccedilla",          /* 128 80 */
    "udieresis",         /* 129 */
    "eacute",            /* 130 */
    "acircumflex",       /* 131 */
	"adieresis",         /* 132 */
    "agrave",            /* 133 */
    "aring",             /* 134 */
    "ccedilla",          /* 135 */
    "ecircumflex",       /* 136 */
    "edieresis",         /* 137 */
    "egrave",            /* 138 */
    "idieresis",         /* 139 */
    "icircumflex",       /* 140 */
    "igrave",            /* 141 */
    "Adieresis",         /* 142 */
    "Aring",             /* 143 */
    "Eacute",            /* 144 90 */
    "ae",                /* 145 */
    "AE",                /* 146 */
    "ocircumflex",       /* 147 */
    "odieresis",         /* 148 */
    "ograve",            /* 149 */
    "ucircumflex",       /* 150 */
    "ugrave",            /* 151 */
    "ydieresis",         /* 152 */
    "Odieresis",         /* 153 */
    "Udieresis",         /* 154 */
    "oslash",            /* 155 9B */
    "sterling",          /* 156 */
    "Oslash",            /* 157 9D */
    "multiply",          /* 158 9E */	
    "florin",            /* 159 */  
    "aacute",            /* 160 A0 */
    "iacute",       	 /* 161 */
    "oacute",            /* 162 */
    "uacute",            /* 163 */
    "ntilde",            /* 164 */
    "Ntilde",            /* 165 */
    NULL,                /* 166 A6 ??? */	
	NULL,                /* 167 A7 ??? */
    "questiondown",      /* 168 */
    "registered",        /* 169 */
    "logicalnot",        /* 170 */
    "onehalf",           /* 171 */
    "onequarter",        /* 172 */
    "exclamdown",        /* 173 */
    "guillemotleft",                /* 174 AE */
    "guillemotright",                /* 175 AF */
    NULL,                /* 176 B0 ??? */
    NULL,                /* 177 B1 ??? */
    NULL,                /* 178 B2 ??? */
    NULL,                /* 179 B3 ??? */
    NULL,                /* 180 B4 ??? */
    "Aacute",            /* 181 */				
    "Acircumflex",       /* 182 */			
    "Agrave",            /* 183 */
    "copyright",         /* 184 */
    NULL,                /* 185 */				
    NULL,                /* 186 */			
    NULL,                /* 187 */		
    NULL,                /* 188 */	
    "cent",              /* 189 */
    "yen",               /* 190 */
    NULL,                /* 191 */
    NULL,                /* 192 C0 */
    "grave",             /* 193 C1 */		
    "acute",             /* 194 C2 */	
    "circumflex",        /* 195 C3 */
    "tilde",             /* 196 C4 */
    "macron",            /* 197 C5 */
    "atilde",            /* 198 C6 */				
    "Atilde",            /* 199 C7 */
    "dieresis",          /* 200 C8 */
    NULL,                /* 201 C9 */
	"ring",              /* 202 CA */
    "cedilla",           /* 203 CB */
    NULL,                /* 204 CC */
    "hungarumlaut",      /* 205 CD */
    "ogonek",                /* 206 CE */
    "currency",          /* 207 CF */
    "eth",               /* 208 DO */	
    "Eth",               /* 209 D1 */	
    "Ecircumflex",       /* 210 D2 */
    "Edieresis",         /* 211 D3 */
    "Egrave",            /* 212 D4 */
    NULL,                /* 213 D5 ??? */
    "Iacute",            /* 214 D6 */
    "Icircumflex",       /* 215 */
    "Idieresis",         /* 216 */
    NULL,                /* 217 */
    NULL,                /* 218 */
    NULL,                /* 219 */
    NULL,                /* 220 */
    "brokenbar",         /* 221 */
    "Igrave",            /* 222 */
    NULL,                /* 223 */
    "Oacute",            /* 224 E0 */
    "germandbls",        /* 225 */
    "Ocircumflex",       /* 226 */
    "Ograve",            /* 227 */
    "otilde",            /* 228 */
    "Otilde",            /* 229 */
    "mu",                /* 230 */
    "thorn",             /* 231 E7 */
    "Thorn",             /* 232 E8 */
    "Uacute",            /* 233 */
    "Ucircumflex",       /* 234 */
    "Ugrave",            /* 235 */
    "yacute",            /* 236 */
	"Yacute",            /* 237 */
    "macron",            /* 238 */
    "acute",             /* 239 EF */
    "hyphen",            /* 240 F0 */
    "plusminus",         /* 241 */
    NULL,                /* 242 */
    "threequarters",     /* 243 */
    "paragraph",         /* 244 */
    "dotlessi",          /* 245 F5 */
    "divide",            /* 246 */
    "cedilla",           /* 247 F7 */
    "degree",            /* 248 */
    "dieresis",          /* 249 F9 */
    "bullet",            /* 250 */
    "onesuperior",       /* 251 */
    "threesuperior",     /* 252 */
    "twosuperior",       /* 253 */
    NULL,                /* 254 */
    NULL                 /* 255 */
};

/*static*/ char *loader_strdup(Type1OpenFont *font, char *s)
/*  STRSAVE  --  Allocate a duplicate of a string.  */
{
	char *c = (char *)font->Alloc(strlen(s) + 1);
	if(c)
		strcpy(c, s);
	return c;
}


/*****************************************************************************
 * File input.
 ****************************************************************************/

static int (*byte_in)(FILE *fp);	/* Call indirect to read a byte.
									 * Will be at1_getc() or hex_byte_in() */

/*static*/ int hex_byte_in(FILE *fp)
/* Read a byte from file in hexadecimal format.  This skips white space,
 * and reads in two hex digits. */
{
    int xd = 0, i;
	char c;

    for (i = 0; i < 2; i++) {
		for (;;) {
			if(fread(&c,1,1,fp)!=1)
				return(EOF);
            if (isspace(c))
                continue;
            break;
        }
		if (islower(c))
			c = toupper(c);
        if (c >= '0' && c <= '9') {
            c -= '0';
		} else if (c >= 'A' && c <= 'F') {
            c = (c - 'A') + 10;
		} else {
            type1_load_error("Bad hex digit");
            return EOF;
        }
        xd = (xd << 4) | c;
    }
    return xd;
}

int at1_getc(FILE *fp)
{
unsigned char c;
if(fread(&c,1,1,fp)!=1)
	return(EOF);
return((int)c);
}

/*****************************************************************************
 *  DECRYPT  --  Perform running decryption of file.  
 ****************************************************************************/

static unsigned short int cryptR, cryptC1, cryptC2, cryptCSR;

static void crypt_init( unsigned int key)
{
	cryptR = key;
    cryptC1 = 52845;
    cryptC2 = 22719;
}

/*static*/ unsigned int decrypt(unsigned int cipher)
{
    unsigned int plain = (cipher ^ (cryptR >> 8));

    cryptR = (cipher + cryptR) * cryptC1 + cryptC2;
	return plain;
}

/*static*/ int decrypt_byte_in(FILE *fp)
{
	int ch;

	if ((ch = (*byte_in)(fp)) == EOF)
		return EOF;
	else
		return decrypt(ch);
}

/*static*/ void cstrinit(void)
{
    cryptCSR = 4330;
}

/*static*/ unsigned int decstr(unsigned int cipher)
{
    unsigned int plain = (cipher ^ (cryptCSR >> 8));

    cryptCSR = (cipher + cryptCSR) * cryptC1 + cryptC2;
    return plain;
}

/*****************************************************************************
 *  PARSER  --  Chop up file a line at a time and decide what to put where...
 ****************************************************************************/


typedef enum 
	{
	TTT_EOF,
	TTT_NAME,
	TTT_NUMBER,
	TTT_OTHER,
	TTT_TOO_LONG,
	} T1_token_type;

typedef struct
	{
	T1_token_type type;
	char string[256];
	int pushback;
	FILE *file;
	int (*source)(FILE *f);
	} Type1_token;

/*static*/ void type1_token_init(Type1_token *tok, FILE *file, int (*source)(FILE *f))
{
	memset(tok,0,sizeof(Type1_token));
	tok->file = file;
	tok->source = source;
}


/*static*/ BOOL continue_number(int ch)
/* Return true if character is a digit */
{
	return isdigit(ch);
}

/*static*/ BOOL continue_name(int ch)
/* Return true if character can be the second or further character in 
 * a name. */
{
	return ch == '_' || isalnum(ch);
}

/*static*/ void type1_get_binary(Type1_token *tok, unsigned char *buf, int size)
/* Read in X number of bytes.  Account for any pushed-back characters. 
 * Bails out if not enough bytes left. */
{
	int ch;
	int i = 0;

	if (tok->pushback != 0)
		{
		*buf++ = tok->pushback;
		i = 1;
		tok->pushback = 0;
		}
	for (; i<size; ++i)
		{
		if ((ch = tok->source(tok->file)) == EOF)
			{
			sprintf(gp_buffer,"Could only read %d of %d bytes.", i, size);
			type1_load_error(gp_buffer);
			}
		*buf++ = ch;
		}
}

/*static*/ void type1_get_token(Type1_token *token)
/* Read in a token from file and categorize it. 
 * In this case a token is a run of numbers, a letter followed by 
 * letters and numbers, or a single non-alpha-numeric character.  
 * White space serves to separate tokens but is otherwise skipped. 
 * Pass in a "source" function to get next character from file. 
 */
{
	int ch;
	int size = 0;
	BOOL (*get_next)(int ch);
	int tok_len = sizeof(token->string);
	char *string = token->string;

	/* Get pushed-back character if any. */
	if (token->pushback == 0)
		ch = token->source(token->file);
	else
		{
		ch = token->pushback;
		token->pushback = 0;
		}
	/* Skip leading spaces. */
	for (;;)
		{
		if (ch == EOF)
			{
			token->type = TTT_EOF;
			strcpy(token->string, "<EOF>");	/* For error reporting. */
			return;
			}
		if (!isspace(ch))
			break;
		ch = token->source(token->file);
		}
	if (isdigit(ch))
		{
		get_next = continue_number;
		token->type = TTT_NUMBER;
		}
	else if (ch == '_' || isalpha(ch))
		{
		get_next = continue_name;
		token->type = TTT_NAME;
		}
	else
		{
		*string++ = ch;
		*string = 0;
		token->type = TTT_OTHER;
		return;
		}
	for (;;)
		{
		if (--tok_len <= 0)
			{
			token->type = TTT_TOO_LONG;
			return;
			}
		*string++ = ch;
		ch = token->source(token->file);
		if (!(*get_next)(ch))
			{
			token->pushback = ch;
			*string = EOS;
			return;
			}
		}
}

/*static*/ Errcode type1_check_signature(FILE *fp)
/* This just verifies that the font begins with %!FontType1 or
 * %!PS-AdobeFont-1.0.  We expect this in the first 128 bytes or so. */
{
	int ch;
	int i;
	char buf[80];
	static char magic1[] = "PS-AdobeFont-1.0";
	static char magic2[] = "FontType1";

	for (i=0; i<128; ++i)
		{
		ch = at1_getc(fp);
		if (ch == EOF)
			break;
		if (ch == '%') 
			{
			ch = at1_getc(fp);
            if (ch == EOF)
				break;
            if (ch == '!')
				{
				if (fread(buf,80,1,fp)!=1)
					break;
				if (strncmp(buf, magic1, strlen(magic1)) == 0
				||	strncmp(buf, magic2, strlen(magic2)) == 0)
					return Success;
				}
	        }
		}
	return Err_bad_magic;
}

static void type1_parse_custom_encoding(Type1OpenFont *font, Type1_token *tok, int size)
/* Read in a bunch of custom encoding statements (that are used to 
 * associate letter names with ASCII values basically. */
{
	int char_ix;
	int i;

	font->data.encoding_count = size;
	font->data.encoding = (char**)font->Alloc(size * sizeof(*(font->data.encoding)));

	for (i=0; i<size; ++i)
		{
		type1_get_token(tok);
		if (tok->type == TTT_EOF)
			type1_load_error("Premature end of file in Encoding");
		if (tok->type == TTT_NAME)
			{
			if (strcmp(tok->string, "dup") == 0)
				{
				/* This should be the start of a sequence formmatted:
				 * 		dup NN /name put
				 */
				type1_get_token(tok);			/* get NN into char_ix */
				if (tok->type != TTT_NUMBER)
					goto SYNTAX_ERROR;
				char_ix = atoi(tok->string);
				if (char_ix < 0 || char_ix >= size)
					{
					sprintf(gp_buffer,"Character index %d out of range.",char_ix);
					type1_load_error(gp_buffer);
					}
				type1_get_token(tok);			/* Skip over /  */
				if (tok->type != TTT_OTHER && tok->string[0] != '/')
					goto SYNTAX_ERROR;
				type1_get_token(tok);			/* Get name into encoding. */
				if (tok->type == TTT_NAME)
					{
					font->data.encoding[char_ix] = loader_strdup(font,tok->string);
					}
				type1_get_token(tok);			/* Skip over put */
				}
			else if (strcmp(tok->string, "def") == 0)
				return;
			}
		}
	return;
SYNTAX_ERROR:
	type1_load_error("Syntax error in Encoding");
}

static void type1_parse_encoding(Type1OpenFont *font, Type1_token *tok)
/* Parse the encoding statement - which determines how names of letters
 * match up with ascii values. This will either just be StandardEncoding,
 * or it will be an array of name/number pairs. */
{
	long encode_size;

	type1_get_token(tok);
	if (tok->type == TTT_NAME 
	&& ((strcmp(tok->string, "StandardEncoding") == 0)
	|| (strcmp(tok->string, "ISOLatin1Encoding") == 0)))
		{
		font->data.encoding = isomap;
		font->data.encoding_count = 256;
		return;
		}
	if (tok->type == TTT_NUMBER)
		{
		encode_size = atoi(tok->string);
		type1_get_token(tok);
		if (tok->type == TTT_NAME 
		&& strcmp(tok->string, "array") == 0)
			{
			type1_parse_custom_encoding(font, tok, encode_size);
			return;
			}
		}
	type1_load_error("Strange /Encoding");
}

/*static*/ void type1_parse_fullname(Type1OpenFont *font, Type1_token *tok)
/* Gets the font name out of the file */
{
	char name[256]="";

	type1_get_token(tok);
	if(strcmp(tok->string,"(")==0)
		{
		loop:
		type1_get_token(tok);
		if(strcmp(tok->string,")")!=0)
			{
			if((strlen(name)+strlen(tok->string)+1)<255)
				{
				strcat(name,tok->string);
				strcat(name," ");
				goto loop;
				}
			}
		if(strlen(name))
			name[strlen(name)-1]=0;
		strcpy(font->data.fullname,name);
		return;
		}
	type1_load_error("Strange /FullName");
}

int get_type1_number(Type1_token *tok,int *rval)
{
	int neg=0,val;

	type1_get_token(tok);
	if(strcmp(tok->string,"-")==0)
		{
		type1_get_token(tok);
		neg=1;
		}
	if(tok->type==TTT_NUMBER)
		{
		sscanf(tok->string,"%d",&val);
		if(neg)
			val= -val;
		*rval=val;
		return(1);
		}
	return(0);
}

/*static*/ void type1_parse_fontbbox(Type1OpenFont *font, Type1_token *tok)
/* Gets the font bounding box out of the file */
{
	int b1,b2,b3,b4;

	type1_get_token(tok);
	if(strcmp(tok->string,"{")==0 || strcmp(tok->string,"[")==0)
		{
		if(get_type1_number(tok,&b1)==0)
			goto error;
		if(get_type1_number(tok,&b2)==0)
			goto error;
		if(get_type1_number(tok,&b3)==0)
			goto error;
		if(get_type1_number(tok,&b4)==0)
			goto error;
/*
sprintf(gp_buffer,"BBOX: %d %d %d %d",b1,b2,b3,b4);
DebugPrint(gp_buffer);
*/
		font->data.minx=0;
		font->data.miny=0;
		font->data.maxx=b3-b1;
		font->data.maxy=b4-b2;
		return;
		}
	error:
	type1_load_error("Invalid font bounding box");
}

static void type1_parse_public_definition(Type1OpenFont *font, Type1_token *tok)
/* Cope with /XXXX definitions during the public (unencrypted) part of
 * the font.  Right now we ignore everything except /Encoding. */
{
	type1_get_token(tok);

	if (tok->type != TTT_NAME)
		return;
	if (strcmp(tok->string, "Encoding") == 0)
		type1_parse_encoding(font, tok);
	else
	if (strcmp(tok->string, "FullName") == 0)
		type1_parse_fullname(font, tok);
	else
	if (strcmp(tok->string, "FontBBox") == 0)
		type1_parse_fontbbox(font, tok);
}

static void type1_parse_to_eexec(Type1OpenFont *font, Type1_token *tok)
/* Read through unencrypted part of the file.  Stop at "eexec" 
 * statement. */
{
	/* First, read to the first '/' encountered */
	for(;;)
		{
		int c;
		c=at1_getc(tok->file);
		if(c==EOF)
			return;
		if(c=='/')
			{
			tok->pushback=c;
			break;
			}
		}
	for (;;)
		{
		type1_get_token(tok);
		switch (tok->type)
		  {
		  case TTT_EOF:
		    type1_load_error("No eexec in Type1 font file.");
			break;
		  case TTT_NAME:
		    if (strcmp(tok->string, "eexec") == 0)
				return;
			break;
		  case TTT_OTHER:
			if (tok->string[0] == '/')
			    type1_parse_public_definition(font, tok);
			break;
		  default:
		  	break;
		  }
		}
}

static void type1_find_mode(Type1OpenFont *font, FILE *fp)
/* 
   (John Walker's comment on how to tell hex from binary.)
   "Adobe Type 1 Font Format Version 1.1", ISBN 0-201-57044-0 states
   on page 64 that one distinguishes an ASCII from a Hexadecimal
   font file by two tests:

		* The first ciphertext byte must not be an ASCII white space
			  character (blank, tab, carriage return or line feed).
		* At least one of the first 4 ciphertext bytes must not be one
		  of the ASCII hexadecimal character codes (a code for 0-9,
		  A-F, or a-f).  These restrictions can be satisfied by adjusting
		  the random plaintext bytes as necessary.

   Well, notwithstanding this statement, Adobe's own Helvetica Bold
   Narrow Oblique file furnished with Adobe Type Manager for Windows
   has a carriage return as the first byte after the eexec invocation.
   Consequently, I turned off recognition of a hex file by the
   presence of a carriage return. */
{
	char cs[4];
	long encrypt_start;
	int i;

	encrypt_start = ftell(fp);
	cs[0] = at1_getc(fp);

	if (cs[0] == ' ' || cs[0] == '\t' ||
		/* cs[0] == '\r' || */
		cs[0] == '\r' ||
		cs[0] == '\n') 
		{
		byte_in = hex_byte_in;
        } 
	else 
		{
		for (i = 1; i < 4; i++) 
			{
			cs[i] = at1_getc(fp);
            }
		byte_in = hex_byte_in;
		for (i = 0; i < 4; i++) 
			{
			if (!((cs[i] >= '0' && cs[0] <= '0') ||
				  (cs[i] >= 'A' && cs[0] <= 'F') ||
				  (cs[i] >= 'a' && cs[0] <= 'f'))) 
				{
				byte_in = at1_getc;
				break;
                }
            }
        }

	fseek(fp, encrypt_start, SEEK_SET);	/* Reread encrypted random bytes as
										 * the decrypter depends on everything
										 * from encrypt_start on going through
										 * byte_in(). */
}

static int type1_get_number(Type1_token *tok)
/* Get next token,  make sure it's a number,  and return the
 * atoi'd value of number. */
{
	type1_get_token(tok);
	if (tok->type != TTT_NUMBER)
		{
		sprintf(gp_buffer,"Expecting number got %s", tok->string);
		type1_load_error(gp_buffer);
		}
	return atoi(tok->string);
}

static void type1_force_symbol(Type1_token *tok, char *symbol)
/* Get next token.  Verify that it matches symbol. */
{
	type1_get_token(tok);
	if (tok->type != TTT_NAME || strcmp(tok->string, symbol) != 0)
		{
		sprintf(gp_buffer,"Expecting %s got %s", symbol, tok->string);
		type1_load_error(gp_buffer);
		}
}

static void type1_skip_to(Type1_token *tok, char *symbol)
/* Skip tokens until come to one that matches symbol. */
{
	for (;;)
		{
		type1_get_token(tok);
		if (tok->type == TTT_EOF)
			{
			sprintf(gp_buffer,"End of file looking for %s\n", symbol);
			type1_load_error(gp_buffer);
			}
		if (strcmp(tok->string, symbol) == 0)
			return;
		}
}


static void type1_force_RD(Type1_token *tok)
/* Make sure that the next bit in the input is either "RD" or "-|" 
 * Also skip the following white space.  */
{
	unsigned char buf[4];

	type1_get_token(tok);
	if (strcmp(tok->string, "RD") == 0)
		{
		type1_get_binary(tok, buf, 1);	/* Skip white space. */
		return;
		}
	if (tok->string[0] == '-')
		{
		type1_get_binary(tok, buf, 2);
		if (buf[0] == '|')
			return;
		}
	type1_load_error("Expecting RD or -|");
}

static void type1_read_encoded_buffer(Type1_token *token, unsigned char *str, int size)
/* Read in bytes from file and (doubly) decrypt.  
 * You may wonder what happens to any pushed-back characters.
 * Well, this is only called in contexts where there will be no
 * pushbacks. */
{
	int i;

	cstrinit();	/* Initialize string decryption. */
	/* We'll throw out the first 4 characters,  only using them
	 * to cycle the decryptor.  */
	for (i=0; i<4; ++i)	/* Decrypt next three. */
		decstr(token->source(token->file));
	/* Now read in and decrypt (again) the string. */
	while (--size >= 0)
		*str++ = decstr(token->source(token->file));
}

static unsigned char *alloc_and_read_RD_string(Type1OpenFont *font,	Type1_token *tok, int binary_size)
{
	unsigned char *buf;

	type1_force_RD(tok);
	buf = (unsigned char *)font->Alloc(binary_size);
	type1_read_encoded_buffer(tok, buf, binary_size);
	return buf;
}

static void type1_get_subrs(Type1OpenFont *font, Type1_token *tok)
{
/* The Subrs format should be of the form:
 *	NN array 
 *		dup NN NN RD xxxxxxx NP
 *				...
 *		dup NN NN RD xxxxxxx NP
 */
	int sub_count;
	int sub_ix;
	int binary_size;
	unsigned char **subrs;
	int i;

	font->data.sub_count = sub_count = type1_get_number(tok);
	font->data.subrs = subrs = (unsigned char **)font->Alloc(sub_count * sizeof(*subrs));
	type1_force_symbol(tok, "array");
	for (i=0; i<sub_count; ++i)
		{
		type1_skip_to(tok, "dup");
		sub_ix = type1_get_number(tok);
		if (sub_ix < 0 || sub_ix >= sub_count)
			{
			sprintf(gp_buffer,"Subr %d out of range (0-%d).", sub_ix, sub_count);
			type1_load_error(gp_buffer);
			}
		binary_size = type1_get_number(tok) - 4;
		subrs[sub_ix] = alloc_and_read_RD_string(font, tok, binary_size);
		}
}

static void type1_get_char_strings(Type1OpenFont *font, Type1_token *tok)
{
/* The CharStrings format should be of the form:
 *	NN dict dup begin 
 *		/name NN RD xxxxxxx ND
 *				...
 *		/name NN RD xxxxxxx ND
 */
	int letter_count;
	char **letter_names;
	unsigned char **letter_defs;
	int letter_ix = 0;
	int binary_size;

	font->data.letter_count = letter_count = type1_get_number(tok);
	font->data.letter_names = letter_names 
		= (char **)font->Alloc(letter_count * sizeof(*letter_names));
	font->data.letter_defs = letter_defs
		= (unsigned char **)font->Alloc(letter_count * sizeof(*letter_defs));
	for (;;)
		{
		type1_get_token(tok);
		if (tok->type == TTT_EOF)
			return;		/* Oh heck, probably have most of the font by now. */
		else if (tok->type == TTT_NAME && strcmp(tok->string, "end") == 0)
			return;
		else if (tok->type == TTT_OTHER && tok->string[0] == '/')
			{
			type1_get_token(tok);
			if (tok->string[0] == '.')	
				{
				/* Here hopefully all we are doing is converting
				 * ".notdef" to "notdef" */
				type1_get_token(tok);
				}
			if (tok->type == TTT_NUMBER)
				{	/* bocklin.pfb has a character with a missing name.
					 * Well, I geuss we kludge around it here... */
				letter_names[letter_ix] = loader_strdup(font, "");
				binary_size = atoi(tok->string);
				}
			else
				{
				letter_names[letter_ix] = loader_strdup(font, tok->string);
				binary_size = type1_get_number(tok);
				}
			letter_defs[letter_ix] = alloc_and_read_RD_string(font, tok, binary_size);
			if (++letter_ix >= letter_count)
				return;
			}
		}
}

/*****************************************************************************
 *****************************************************************************
 ** The Interpreter Section.  This section deals with interpreting the
 ** little reverse-polish-notation language that describes the letters in
 ** the font.  
 *****************************************************************************
 ****************************************************************************/

enum cscommand {
/*  Charstring command op-codes.  */
	Unused_0,
	Hstem,
	Unused_2,
	Vstem,
	Vmoveto,
	Rlineto,
	Hlineto,
	Vlineto,
	Rrcurveto,
	Closepath,
    Callsubr,
    Return,
    Esc/*ape*/,		// Need to do this to avoid collision
    Hsbw,
    Endchar,
    Unused_15,
    Unused_16,
    Unused_17,
    Unused_18,
    Unused_19,
    Unused_20,
    Rmoveto,
	Hmoveto,
    Unused_23,
    Unused_24,
    Unused_25,
    Unused_26,
    Unused_27,
    Unused_28,
    Unused_29,
    Vhcurveto,
    Hvcurveto,

    /* 12 x commands */

    Dotsection,
    Vstem3,
    Hstem3,
    Unused_12_3,
    Unused_12_4,
    Unused_12_5,
    Seac,
    Sbw,
    Unused_12_8,
    Unused_12_9,
    Unused_12_10,
    Unused_12_11,
    Div,
    Unused_12_13,
    Unused_12_14,
    Unused_12_15,
    Callothersubr,
    Pop,
    Unused_12_18,
    Unused_12_19,
    Unused_12_20,
    Unused_12_21,
	Unused_12_22,
    Unused_12_23,
    Unused_12_24,
    Unused_12_25,
    Unused_12_26,
    Unused_12_27,
    Unused_12_28,
    Unused_12_29,
    Unused_12_30,
    Unused_12_31,
    Unused_12_32,
    Setcurrentpoint
};

#define StackLimit  25
#define OtherLimit  10                /* Maximum othersubr return values */

#define Sl(n) if (sp < (n)) {fflush(stdout); DebugPrint("Stack underflow.\n"); return(0);}
#define Npop(n) sp -= (n)
#define So(n) if ((sp + (n)) > StackLimit) {fflush(stdout); DebugPrint("Stack overflow.\n"); return 0;}
#define Clear() sp = 0

#define S0  stack[sp - 1]
#define S1  stack[sp - 2]
#define S2  stack[sp - 3]
#define S3  stack[sp - 4]
#define S4  stack[sp - 5]
#define S5  stack[sp - 6]

static long stack[StackLimit];        /* Data stack */
static int sp;                        /* Stack pointer */
static long osres[OtherLimit];        /* Results from othersubrs */
static int orp;                       /* Othersubr result pointer */

#define ReturnStackLimit 10

static unsigned char *rstack[ReturnStackLimit]; /* Return stack */
static int rsp;                       /* Return stack pointer */

static int curx, cury;			      /* The current point */
static int flexing;			          /* If a Flex in progress ? */
static int flexx, flexy;              /* Flex current position */

static int bnum;                  	  /* Line segments per Bezier curve */

static int pcount;

/* 3D Studio bezier spline character from Adobe description code */

/*  BEZIER  --	Evaluate a Bezier curve defined by four control
		points.  */

static Spline3D *curSpline = NULL;
static int a1lastx,a1lasty;
static int nested=0;

/*static*/ void bezier(BezierShape &shape,int x0, int y0, int x1, int y1, int x2, int y2,
	int x3, int y3)
{
if(!curSpline)
	curSpline = shape.NewSpline();
int knots = curSpline->KnotCount();
if(knots == 0) {
	curSpline->AddKnot(SplineKnot(KTYPE_BEZIER, LTYPE_CURVE, Point3(x0,y0,0), Point3(x0,y0,0), Point3(x1,y1,0)));
	curSpline->AddKnot(SplineKnot(KTYPE_BEZIER, LTYPE_CURVE, Point3(x3,y3,0), Point3(x2,y2,0), Point3(x3,y3,0)));
	}
else {
	// First point of this curve must be the same as the last point on the output curve
	assert(curSpline->GetKnotPoint(knots-1) == Point3(x0,y0,0));
	curSpline->SetOutVec(knots-1, Point3(x1,y1,0));
	if(Point3(x3,y3,0) == curSpline->GetKnotPoint(0)) {
		curSpline->SetInVec(0, Point3(x2,y2,0));
		curSpline->SetClosed();
		curSpline = NULL;
		}
	else
		curSpline->AddKnot(SplineKnot(KTYPE_BEZIER, LTYPE_CURVE, Point3(x3,y3,0), Point3(x2,y2,0), Point3(x3,y3,0)));
	}
a1lastx=x3;
a1lasty=y3;
curx=x3;
cury=y3;
#ifdef DBGAT3
printf("%d %d to %d %d\n",x0,y0,x3,y3);
#endif
}

/*----------------------------------------------------------------------*
 * Interpret an "other subroutine".  I'm not 100% sure what all this
 * can be.  There are some predefined ones for all fonts,  but potentially they
 * can reside in the file too?
 *----------------------------------------------------------------------*/

static void othersubr(BezierShape &shape, int procno, int nargs, int argp)
{
    static int flexp;                 /* Flex argument pointer */
    static int flexarg[8][2];

    orp = 0;                          /* Reset othersubr result pointer */

	switch (procno) {
        case 0:                       /* Flex */
#ifdef DBGAT3
printf("Othersubr(0), Doing flex bezier\n");
#endif
			bezier(shape,
					flexarg[0][X], flexarg[0][Y],
					flexarg[2][X], flexarg[2][Y],
					flexarg[3][X], flexarg[3][Y],
					flexarg[4][X], flexarg[4][Y]);
			bezier(shape,
					flexarg[4][X], flexarg[4][Y],
					flexarg[5][X], flexarg[5][Y],
					flexarg[6][X], flexarg[6][Y],
					flexarg[7][X], flexarg[7][Y]);
            osres[orp++] = stack[argp + 3];
            osres[orp++] = stack[argp + 2];
            flexing = FALSE;          /* Terminate flex */
#ifdef DBGAT3
printf("Flexing OFF\n");
#endif
            break;

        case 1:                       /* Flex start */
#ifdef DBGAT3
printf("Othersubr(1), Flexing ON\n");
#endif
            flexing = TRUE;           /* Mark flex underway */
            flexx = curx;
            flexy = cury;
            flexp = 0;
            /* Note fall-through */
        case 2:                       /* Flex argument specification */
#ifdef DBGAT3
printf("Othersubr(2)\n");
#endif
            flexarg[flexp][X] = flexx;
            flexarg[flexp++][Y] = flexy;
            break;

        case 3:                       /* Hint replacement */
#ifdef DBGAT3
printf("Othersubr(3)\n");
#endif
			osres[orp++] = 3;		  /* This eventually results in
									   * subroutine 3 being called.
									   * Since subroutine 3 does nothing
									   * but return, one can only guess
									   * what Adobe had in mind designing
									   * this. */
            break;

        default:
    /*        fprintf(stderr, "\nCall to undefined othersubr %d\n",
                procno); */
			break;
    }
}

/*  EXCHARS  --  Execute charstring.  */

/*static*/ int exchars(Type1OpenFont *font,int *pwidth,unsigned char *cp,BezierShape &shape,int ix,int iy)
{
    int sub;

    sp = rsp = 0;		      /* Reset stack pointer */
	flexing = FALSE;          

    while (TRUE)
		{
		int c = *cp++;

		if (c < 32)
			{
			/* Command */
			if (c == 12)
				{
				/* Two byte command */
				c = *cp++ + 32;
				}

		    switch (c)
				{

				/* Commands for Starting and Finishing */

				case Endchar:	      /* 14: End character */
#ifdef DBGAT3
printf("Endchar\n");
#endif
				if(curSpline)
					assert(0);
				curSpline = NULL;
				a1lastx=a1lasty=0;
				Clear();
				goto exdone;

			case Hsbw:	      /* 13:  Set horizontal sidebearing */
			    Sl(2);
			    curx=a1lastx=S1+ix;
			    cury=a1lasty=0+iy;
				if(nested==0)
				    *pwidth=t1_width=S0;
#ifdef DBGAT3
printf("Hsbw: %d\n",t1_width);
#endif
			    Clear();
			    break;

			case Seac: {	      /* 12-6:	Standard encoding accented char */
				int lwidth;
				unsigned char *base, *accent;
				unsigned base_ix = S1, accent_ix = S0;
				int xoff = S3, yoff = S2, asb = S4; 
				int fx = curx, fy = cury;
			    Sl(5);
#ifdef DBGAT3
printf("Seac\n");
#endif
				nested=1;
				if (base_ix < BYTE_MAX)
					{
					if ((base = font->data.ascii_defs[base_ix]) != NULL)
						{
						if (exchars(font, &lwidth, base, shape, ix, iy)==0)
							{
							nested=0;
							DebugPrint("Error decoding Seac base");
							return(0);
							}
						}
					}
				if (accent_ix < BYTE_MAX)
					{
					if ((accent = font->data.ascii_defs[accent_ix]) != NULL)
						{
						if (exchars(font, &lwidth, accent, shape,
								ix+xoff+fx-asb, iy+fy+yoff)==0)
							{
							nested=0;
							DebugPrint("Error decoding Seac accent");
							return(0);
							}
						}
					}
				nested=0;
				if(curSpline)
					assert(0);
				curSpline = NULL;
				a1lastx=a1lasty=0;
			    Clear();
			    goto exdone;
				}

			case Sbw:	      /* 12-7:	Sidebearing point (x-y) */
			    Sl(4);
			    curx = a1lastx = S3+ix;
			    cury = a1lasty = S2+iy;
				if(nested==0)
				    *pwidth=t1_width=S0;
#ifdef DBGAT3
printf("Sbw: %d\n",t1_width);
#endif
			    Clear();
			    break;

			/* Path Construction Commands */

			case Closepath:       /* 9:  Close path */
#ifdef DBGAT3
printf("Closepath\n");
#endif
				if(curSpline) {
					curSpline->SetClosed();
					curSpline = NULL;
					}
			    Clear();
			    break;

			case Hlineto:	      /* 6: Horizontal line to */
			    Sl(1);
#ifdef DBGAT3
printf("Hlineto ");
#endif
			    bezier(shape,curx,cury,curx,cury,curx+S0,cury,curx+S0,cury);
			    Clear();
			    break;

			case Hmoveto:	      /* 22:  Horizontal move to */
			    Sl(1);
#ifdef DBGAT3
printf("Hmoveto ");
#endif
				if (flexing)
					flexx += S0;
				else
				    curx += S0;
			    Clear();
			    break;

			case Hvcurveto:       /* 31:  Horizontal-vertical curve to */
			    Sl(4);
#ifdef DBGAT3
printf("Hvcurveto ");
#endif
			    bezier(shape,curx, cury, curx + S3, cury,
				   curx + S3 + S2, cury + S1,
				   curx + S3 + S2, cury + S1 + S0);
			    Clear();
			    break;

			case Rlineto:	      /* 5:  Relative line to */
			    Sl(2);
#ifdef DBGAT3
printf("Rlineto ");
#endif
			    bezier(shape,curx,cury,curx,cury,
					curx+S1,cury+S0,curx+S1,cury+S0);
			    Clear();
			    break;

			case Rmoveto:	      /* 21:  Relative move to */
			    Sl(2);
#ifdef DBGAT3
printf("Rmoveto %d %d\n",curx+S1,cury+S0);
#endif
				if (flexing) 
					{
					flexx += S1;
					flexy += S0;
					}
				else
					{
				    curx += S1;
				    cury += S0;
					}
			    Clear();
			    break;

			case Rrcurveto:       /* 8:  Relative curve to */
			    Sl(6);
#ifdef DBGAT3
printf("Rrcurveto ");
#endif
			    bezier(shape, curx, cury, curx + S5, cury + S4,
				   curx + S5 + S3, cury + S4 + S2,
				   curx + S5 + S3 + S1, cury + S4 + S2 + S0);
			    Clear();
			    break;

			case Vhcurveto:       /* 30:  Vertical-horizontal curve to */
			    Sl(4);
#ifdef DBGAT3
printf("Vhcurveto ");
#endif
			    bezier(shape, curx, cury, curx, cury + S3,
				   curx + S2, cury + S3 + S1,
				   curx + S2 + S0, cury + S3 + S1);
			    Clear();
			    break;

			case Vlineto:	      /* 7:  Vertical line to */
			    Sl(1);
#ifdef DBGAT3
printf("Vlineto ");
#endif
			    bezier(shape, curx,cury,curx,cury,curx,cury+S0,curx,cury+S0);
			    Clear();
			    break;

			case Vmoveto:	      /* 4:  Vertical move to */
			    Sl(1);
#ifdef DBGAT3
printf("Vmoveto %d %d\n",curx,cury+S0);
#endif
				if (flexing)
					flexy += S0;
				else
				    cury += S0;
			    Clear();
			    break;

			/*  Hint Commands  */

			case Dotsection:      /* 12-0:	Dot section */
			    Clear();
#ifdef DBGAT3
printf("Dotsection\n");
#endif
			    break;

			case Hstem:	      /* 1:  Horizontal stem zone */
			    Sl(2);
			    Clear();
#ifdef DBGAT3
printf("Hstem\n");
#endif
			    break;

			case Hstem3:	      /* 12-2:	Three horizontal stem zones */
			    Sl(6);
			    Clear();
#ifdef DBGAT3
printf("Hstem3\n");
#endif
			    break;

			case Vstem:	      /* 3:  Vertical stem zone */
			    Sl(2);
			    Clear();
#ifdef DBGAT3
printf("Vstem\n");
#endif
			    break;

			case Vstem3:	      /* 12-1:	Three vertical stem zones */
			    Sl(6);
			    Clear();
#ifdef DBGAT3
printf("Vstem3\n");
#endif
			    break;

			/* Arithmetic command */

			case Div:	      /* 12 12:  Divide */
			    Sl(2);
#ifdef DBGAT3
printf("Div %d=(%d+(%d/2))/%d\n",(S1+(S0/2))/S0,S1,S0,S0);
#endif
			    S1 = (S1 + (S0 / 2)) / S0;
			    Npop(1);
			    break;

			/* Subroutine Commands */

			case Callothersubr:   /* 12 16:  Call other subroutine */
			    Sl(2);
#ifdef DBGAT3
printf("Callothersubr\n");
#endif
			    Sl(2 + S1);
				othersubr(shape, S0, S1, sp - (3 + S1));
			    Npop(2 + S1);
			    break;

			case Callsubr:	      /* 10:  Call subroutine */
			    Sl(1);
#ifdef DBGAT3
printf("Callsubr\n");
#endif
			    if (rsp >= ReturnStackLimit)
					{
					fflush(stdout);
					DebugPrint("ADOBET1: Stack limit err\n");
					return(0);
					}
			    rstack[rsp++] = cp;
			    sub = S0;
			    Npop(1);
			    if (sub < 0 || sub >= font->data.sub_count)
					{
					fflush(stdout);
					DebugPrint("ADOBET1: Bad subr\n");
					return(0);
				    }
			    if (font->data.subrs[sub] == NULL)
					{
					fflush(stdout);
					DebugPrint("ADOBET1: NULL subr\n");
					return(0);
				    }
			    cp = font->data.subrs[sub];  /* Set instruction pointer to subr code */
			    break;

			case Pop:	      /* 12 17:  Return argument from othersubr */
#ifdef DBGAT3
printf("Pop\n");
#endif
			    So(1);
				if (orp <= 0)
					{
					fflush(stdout);
					DebugPrint("Stack error in subr pop");
					return(0);
					}
				stack[sp++] = osres[--orp];
			    break;

			case Return:	      /* 11:  Return from subroutine */
#ifdef DBGAT3
printf("Return\n");
#endif
			    if (rsp < 1)
					{
					fflush(stdout);
					DebugPrint("ADOBET1: Return stack error\n");
					return(0);
				    }
			    cp = rstack[--rsp]; /* Restore pushed call address */
			    break;

			case Setcurrentpoint: /* 12 33:  Set current point */
#ifdef DBGAT3
printf("Setcurrent ");
#endif
			    Sl(2);
			    bezier(shape,a1lastx,a1lasty,a1lastx,a1lasty,curx,cury,curx,cury);
			    Clear();
			    break;
		    }
		}
	else
		{
	    long n;
#ifdef DBGAT3
printf("??? (%d)\n",c);
#endif

	    if (c <= 246)
			n = c - 139;
		else
		if (c <= 250)
			n = ((c - 247) << 8) + *cp++ + 108;
		else
		if (c < 255)
			n = -((c - 251) << 8) - *cp++ - 108;
		else
			{
			char ba[4];

			ba[0] = *cp++;
			ba[1] = *cp++;
			ba[2] = *cp++;
			ba[3] = *cp++;
			n = (((((ba[0] << 8) | ba[1]) << 8) | ba[2]) << 8) | ba[3];
		    }
	    if (sp >= StackLimit)
			{
			fflush(stdout);
			DebugPrint("ADOBET1: Stack limit err\n");
			}
		else
			{
			stack[sp++] = n;
#ifdef DBGAT3
printf("Pushed (%d) onto stack\n",n);
#endif
		    }
		}
    }

exdone:
if(!shape.SplineCount())
	return(0);
return(1);
}

//
// Our bezier font import code
//

// ReadFile flags
#define AT1_OPEN_FULL 0		// Get the whole magilla
#define AT1_OPEN_INFO 1		// Get just the information

class AdobeT1Import : public BezFont {
	private:
		Type1OpenFont *open;
	public:
		// required methods from BezFont class
		void EnumerateFonts(BezFontMgrEnumProc &proc, LPARAM userInfo);
		int OpenFont(TSTR name, DWORD flags, DllData *dllData);
		void CloseFont();
		BOOL BuildCharacter(UINT index, float height, BezierShape &shape, float &width, BOOL useMax1Shapes);
		// Our methods
		AdobeT1Import() { open = NULL; }
		~AdobeT1Import();
		BOOL ReadFile(TSTR fname, DWORD mode, BezFontInfo &info);
		int LoadType1(TSTR fname, int mode, Type1OpenFont *font);
		Errcode ReadFont(FILE *fp, int mode, Type1OpenFont *font);
		void RType1(FILE *fp, int mode, Type1OpenFont *font);
		Errcode FindAsciiValues(Type1OpenFont *font);
		BOOL Compout(int index, BezierShape &shape, int *width);
	};

class AT1DllData : public DllData {
	public:
		TSTR filename;
		AT1DllData(TSTR f) { filename = f; }
	};

void AdobeT1Import::EnumerateFonts(BezFontMgrEnumProc &proc, LPARAM userInfo) {
	FontMgrInterface *iface = theBezFontManager.GetInterface();
	int paths = iface->GetFontDirCount();
	WIN32_FIND_DATA data;
	for(int i = 0; i < paths; ++i) {
		TSTR dir = iface->GetFontDir(i);
		TSTR search = dir + _T("\\*.pfb");
		HANDLE h = FindFirstFile(search.data(), &data);
		if(h != INVALID_HANDLE_VALUE) {
			while(1) {
				TSTR fname = dir + _T("\\") + TSTR(data.cFileName);
				BezFontInfo info;
//DebugPrint("Checking [%s]\n",fname.data());
				if(ReadFile(fname, AT1_OPEN_INFO, info)) {
					AT1DllData *data = new AT1DllData(fname);	// Give manager the filename
					if(!proc.Entry(info, userInfo, data)) {
						FindClose(h);
						return;
						}
					}
				if(!FindNextFile(h, &data))
					break;
				}
			FindClose(h);
			}
		}
	}

int AdobeT1Import::OpenFont(TSTR name, DWORD flags, DllData *dllData) {
	AT1DllData *at1DllData = (AT1DllData *)dllData;
	BezFontInfo info;
	if(ReadFile(at1DllData->filename, AT1_OPEN_FULL, info)) {
		return 1;
		}
#ifdef DBGAT1
DebugPrint("ADOBET1: Something went wrong in open!\n");
#endif
	return 0;
	}

void AdobeT1Import::CloseFont() {
	if(open) {
		delete open;
		open = NULL;
		return;
		}
	assert(0);	// Didn't find it!
	}

BOOL AdobeT1Import::BuildCharacter(UINT index, float height, BezierShape &shape, float &width, int fontShapeVersion) {
	if(!open) {
		assert(0);		// Not a valid handle!
		width = 0.0f;
		return FALSE;
		}

	/* Look up the appropriate character */

	int chWidth;
	if(Compout(index, shape, &chWidth)==0)
		return(FALSE);
	// Update the selection set info -- Just to be safe
	shape.UpdateSels();
	// Scale the character according to the request
	float fontHeight = (float)(open->data.maxy - open->data.miny);
	if(height == 0.0f)
		height = 1.0f;
	float scaleFactor = height / fontHeight;
	Matrix3 tm = ScaleMatrix(Point3(scaleFactor, scaleFactor, 0.0f));
	shape.Transform(tm);
	width = float(chWidth) * scaleFactor;
	return TRUE;
	}

AdobeT1Import::~AdobeT1Import() {
	// Clean up our allocations
	if(open)
		delete open;
	}

BOOL AdobeT1Import::ReadFile(TSTR fname, DWORD mode, BezFontInfo &info) {
	int err;

	Type1OpenFont *theFont = new Type1OpenFont;
	if((err = LoadType1(fname, mode, theFont)) >= 0)
		{
		info = BezFontInfo(TSTR(theFont->data.fullname), TSTR(_T("")), BEZFONT_OTHER, (DWORD)0, BezFontMetrics());
		switch(mode) {
			case AT1_OPEN_INFO:
				delete theFont;
				return TRUE;
			case AT1_OPEN_FULL:
				open = theFont;
				return TRUE;
			} 
		assert(0);
		return(FALSE);
		}
#ifdef DBGAT1
DebugPrint("Type1 font load failed:%d",err);
#endif
	return(FALSE);
	}

int AdobeT1Import::LoadType1(TSTR fname, int mode, Type1OpenFont *font) {
	FILE *file;
	int err;

	if ((file = fopen(fname.data(), "rb")) == NULL)
		return(-1);
	if ((err = ReadFont(file, mode, font)) >= Success)
		{
		if(mode == AT1_OPEN_FULL)
			err = FindAsciiValues(font);
		}
	fclose(file);
	return err;
	}

Errcode AdobeT1Import::ReadFont(FILE *fp, int mode, Type1OpenFont *font) {
	Errcode err = Success;

	if ((err = setjmp(type1_load_errhandler)) != 0)
		{	/* Got here via longjmp. */
		err = Err_nogood;	// _something_ went wrong!
		}
	else
		{
		font->data.minx = font->data.miny=0;
		font->data.maxx = font->data.maxy=1000;
		RType1(fp, mode, font);
		}
	return err;
	}

#define FP(x) { DebugPrint("%s:%d\n",(x), ftell(fp)); }

void AdobeT1Import::RType1(FILE *fp, int mode, Type1OpenFont *font) {
    char token[256], ltoken[256], stoken[256], ptoken[256];
    int i;
	Errcode err;
	Type1_token tok;

	section = Header;
    ptoken[0] = stoken[0] = ltoken[0] = token[0] = EOS;
	if ((err = type1_check_signature(fp)) < Success)
		{
		sprintf(gp_buffer,"Can't find %%!FontType1 in .PFB file.");
		type1_load_error(gp_buffer);
		}
	type1_token_init(&tok, fp, at1_getc);
	type1_parse_to_eexec(font, &tok);
	if (font->data.encoding == NULL)
		{
		sprintf(gp_buffer,"No /Encoding array.");
		type1_load_error(gp_buffer);
		}

	// If just loading info, stop here
	if(mode != AT1_OPEN_FULL)
		return;

    for (i = 0; i < 6; i++) 
		{
        int c=at1_getc(fp);              /* Beats me, but there's 6 trash bytes */
		}
	type1_find_mode(font, fp);
    crypt_init(55665);

    /* Now burn the first four plaintext bytes. */

	for (i = 0; i < 4; i++)
		int burn = decrypt_byte_in(fp);

	type1_token_init(&tok, fp, decrypt_byte_in);
	for (;;)
		{
		type1_get_token(&tok);
		switch (tok.type)
		  {
		  case TTT_EOF:
			goto DONE;
		  case TTT_OTHER:
		    if (tok.string[0] == '/')
				{
				type1_get_token(&tok);
				if (tok.type == TTT_NAME)
					{
					if (strcmp(tok.string, "Subrs") == 0)
						type1_get_subrs(font, &tok);
					else if (strcmp(tok.string, "CharStrings") == 0)
						{
						type1_get_char_strings(font, &tok);
						goto DONE;
						}
					}
				}
			break;
		  default:
		    break;
		  }
	}
DONE:
	if (font->data.letter_defs == NULL)
		type1_load_error("No CharStrings!");
	}

/*****************************************************************************
 * Go through and build up an ascii-ordered array of character definitions.
 ****************************************************************************/
Errcode AdobeT1Import::FindAsciiValues(Type1OpenFont *font) {
	char *name;
	unsigned char *ascii_name;
	unsigned char **map = (unsigned char **)font->data.encoding;
	char **names = font->data.letter_names;
	unsigned char **defs = font->data.letter_defs;
	int i,ascii_val;
	int def_ix;
	int def_count;
	int matches = 0;

	/* Clear out the array to NULLs first */
	for(i=0; i<256; ++i)
		font->data.ascii_defs[i]=NULL;

/* The character defs are stored in the font file in *roughly* ascii order.
 * The logic in this routine takes some advantage of this by starting
 * the search for the next letter where the search for the current letter
 * left off. */
	def_ix = 0;		/* Initialize search starting position. */
	def_count = font->data.letter_count;
	for (ascii_val=0; ascii_val<font->data.encoding_count; ++ascii_val)
		{
		if ((ascii_name = *map++) != NULL)
			{
			i = def_count;
			while (--i >= 0)
				{
				if (++def_ix >= def_count)
					def_ix = 0;
				if ((name = names[def_ix]) != NULL)
					{
					if (strcmp(name, (char *)ascii_name) == 0)
						{
						font->data.ascii_defs[ascii_val] = defs[def_ix];
						++matches;
						break;
						}
					}
				}
			}
		}
	if (matches > 0)
		{
		return Success;
		}
	else
		{
		return Err_not_found;
		}
	}

int AdobeT1Import::Compout(int index, BezierShape &shape, int *width) {
	int j,chWidth;

	/* Make sure it's a valid ID */

	if(index < 0 || index >= open->data.encoding_count)
		{
#ifdef DBGAT1
DebugPrint("Outside encoding table limits\n");
#endif
		return(FALSE);
		}

	if(open->data.encoding[index]==NULL)
		{
#ifdef DBGAT1
DebugPrint("%d (%c) Not in encoding table\n",index,index);
#endif
		return(FALSE);
		}

	/* Look for char by name */

	for (j = 0; j < open->data.letter_count; j++) {
		if(open->data.letter_names[j]!=NULL) {
	  		if (strcmp((char *)open->data.letter_names[j],(char *)open->data.encoding[index]) == 0) {
	//printf("Executing char [%s]",tcd->letter_names[j]);
				if(exchars(open,&chWidth,(unsigned char *)open->data.letter_defs[j],shape,0,0)==0) {
#ifdef DBGAT1
DebugPrint("Error decoding character %d (%c)\n",index,index);
#endif
					return FALSE;
					}
				if(width)
					*width = chWidth;
				return TRUE;
				}
			}
		}
#ifdef DBGAT1
DebugPrint("%d (%c) Not in letter names\n",index,index);
#endif
	return FALSE;
	}

// Jaguar interface code

BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved) {
	hInstance = hinstDLL;
	switch(fdwReason) {
		case DLL_PROCESS_ATTACH:
			//MessageBox(NULL,L"ADOBET1.DLL: DllMain",L"BEZFONT",MB_OK);
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
			break;
		}
	return(TRUE);
	}


//------------------------------------------------------

class AT1ClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new AdobeT1Import; }
	const TCHAR *	ClassName() { return GetString(IDS_TH_ADOBET1_CLASSNAME); }
	SClass_ID		SuperClassID() { return BEZFONT_LOADER_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(ADOBET1_LOADER_CLASS_ID,0); }
	const TCHAR* 	Category() { return GetString(IDS_TH_ADOBET1_CATEGORY);  }
	};

static AT1ClassDesc AT1Desc;

//------------------------------------------------------
// This is the interface to Jaguar:
//------------------------------------------------------

__declspec( dllexport ) const TCHAR *
LibDescription() { return GetString(IDS_LIB_DESCRIPTION); }

__declspec( dllexport ) int
LibNumberClasses() { return 1; }

__declspec( dllexport ) ClassDesc *
LibClassDesc(int i) {
	switch(i) {
		case 0: return &AT1Desc; break;
		default: return 0; break;
		}

	}

// Return version so can detect obsolete DLLs
__declspec( dllexport ) ULONG 
LibVersion() { return VERSION_3DSMAX; }

TCHAR *GetString(int id)
	{
	static TCHAR buf[256];

	if (hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
	}

