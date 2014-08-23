/*
======================================================================
surf.c

Ernie Wright  29 Mar 00

Surface conversion for VideoScape .geo objects.
====================================================================== */

#include <lwtypes.h>
#include <lwsurf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/*
----------------------------------------------------------------------
surface data

The appearance of a surface in VideoScape is completely described by a
single color code ranging from 0 to 260.  For codes from 0 to 255, the
low 4 bits select one of the 16 supported colors, and the high 4 bits
select surface attributes.  Bits 4 and 5 taken together choose from

   0 - Matte
   1 - Glossy
   2 - Luminous
   3 - Outline

Bit 6 is a transparency flag, and bit 7 is a smoothing flag.

Codes from 256 to 260 are special effects surfaces.

   256 - invisible
   257 - subtractive
   258 - additive
   259 - mirror
   260 - smooth mirror

Since we can represent the surface descriptions for all of these color
codes with an array of only 21 colors and 21 attribute sets, it makes
sense to precalculate and store these arrays statically.

In fact, we could store the actual byte pattern, and we wouldn't need
to swap bytes or construct SURF chunks in memory, but I wanted to show
how SURF chunks can be built dynamically.
---------------------------------------------------------------------- */

typedef struct {
   int            flags;
   float          lumi;
   float          diff;
   float          spec;
   float          tran;
   float          rind;
   float          refl;
   float          glos;
   float          sman;
   unsigned short rfop;
   unsigned short trop;
   unsigned short side;
   unsigned short line;
} VSSurf;

#define F_LUMI  (1 << 0)
#define F_DIFF  (1 << 1)
#define F_SPEC  (1 << 2)
#define F_TRAN  (1 << 3)
#define F_RIND  (1 << 4)
#define F_REFL  (1 << 5)
#define F_GLOS  (1 << 6)
#define F_SMAN  (1 << 7)
#define F_RFOP  (1 << 8)
#define F_TROP  (1 << 9)
#define F_SIDE  (1 << 10)
#define F_LINE  (1 << 11)


static LWID ckid[] = {
   LWID_( 'C','O','L','R' ),
   LWID_( 'L','U','M','I' ),
   LWID_( 'D','I','F','F' ),
   LWID_( 'S','P','E','C' ),
   LWID_( 'T','R','A','N' ),
   LWID_( 'R','I','N','D' ),
   LWID_( 'R','E','F','L' ),
   LWID_( 'G','L','O','S' ),
   LWID_( 'S','M','A','N' ),
   LWID_( 'R','F','O','P' ),
   LWID_( 'T','R','O','P' ),
   LWID_( 'S','I','D','E' ),
   LWID_( 'L','I','N','E' )
};

static float color[ 21 ][ 3 ] = {
   0.000000f, 0.000000f, 0.000000f,       //   0,   0,   0
   0.000000f, 0.313726f, 0.627451f,       //   0,  80, 160
   0.000000f, 0.549020f, 0.000000f,       //   0, 140,   0
   0.000000f, 0.549020f, 0.549020f,       //   0, 140, 140
   0.627451f, 0.000000f, 0.000000f,       // 160,   0,   0
   0.549020f, 0.000000f, 0.627451f,       // 140,   0, 160
   0.627451f, 0.470588f, 0.000000f,       // 160, 120,   0
   0.549020f, 0.549020f, 0.549020f,       // 140, 140, 140
   0.000000f, 0.000000f, 0.000000f,       //   0,   0,   0
   0.000000f, 0.470588f, 0.941177f,       //   0, 120, 240
   0.000000f, 0.941177f, 0.000000f,       //   0, 240,   0
   0.000000f, 0.941177f, 0.941177f,       //   0, 240, 240
   0.941177f, 0.000000f, 0.000000f,       // 240,   0,   0
   0.784314f, 0.000000f, 0.941177f,       // 200,   0, 240
   0.941177f, 0.784314f, 0.000000f,       // 240, 200,   0
   0.941177f, 0.941177f, 0.941177f,       // 240, 240, 240
   0.000000f, 0.000000f, 0.000000f,       //   0,   0,   0
   0.000000f, 0.000000f, 0.000000f,       //   0,   0,   0
   1.000000f, 1.000000f, 1.000000f,       // 255, 255, 255
   1.000000f, 1.000000f, 1.000000f,       // 255, 255, 255
   1.000000f, 1.000000f, 1.000000f,       // 255, 255, 255
};

static VSSurf surf[] = {
   F_DIFF | F_RFOP | F_TROP | F_SIDE,
   0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1, 1, 1, 0,
   F_DIFF | F_SPEC | F_GLOS | F_RFOP | F_TROP | F_SIDE,
   0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.4f, 0.0f, 1, 1, 1, 0,
   F_LUMI | F_DIFF | F_RFOP | F_TROP | F_SIDE,
   1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1, 1, 1, 0,
   F_DIFF | F_RFOP | F_TROP | F_SIDE | F_LINE,
   0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1, 1, 1, 1,
   F_DIFF | F_TRAN | F_RIND | F_RFOP | F_TROP | F_SIDE,
   0.0f, 1.0f, 0.0f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1, 1, 1, 0,
   F_DIFF | F_SPEC | F_TRAN | F_RIND | F_GLOS | F_RFOP | F_TROP | F_SIDE,
   0.0f, 1.0f, 1.0f, 0.5f, 1.0f, 0.0f, 0.4f, 0.0f, 1, 1, 1, 0,
   F_LUMI | F_DIFF | F_TRAN | F_RIND | F_RFOP | F_TROP | F_SIDE,
   1.0f, 0.0f, 0.0f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1, 1, 1, 0,
   F_DIFF | F_TRAN | F_RIND | F_RFOP | F_TROP | F_SIDE | F_LINE,
   0.0f, 1.0f, 0.0f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1, 1, 1, 1,
   F_DIFF | F_SMAN | F_RFOP | F_TROP | F_SIDE | F_LINE,
   0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.562070f, 1, 1, 1, 0,
   F_DIFF | F_SPEC | F_GLOS | F_SMAN | F_RFOP | F_TROP | F_SIDE,
   0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.4f, 1.562070f, 1, 1, 1, 0,
   F_LUMI | F_DIFF | F_SMAN | F_RFOP | F_TROP | F_SIDE,
   1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.562070f, 1, 1, 1, 0,
   F_DIFF | F_SMAN | F_RFOP | F_TROP | F_SIDE | F_LINE,
   0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.562070f, 1, 1, 1, 1,
   F_DIFF | F_TRAN | F_RIND | F_SMAN | F_RFOP | F_TROP | F_SIDE,
   0.0f, 1.0f, 0.0f, 0.5f, 1.0f, 0.0f, 0.0f, 1.562070f, 1, 1, 1, 0,
   F_DIFF | F_SPEC | F_TRAN | F_RIND | F_GLOS | F_SMAN | F_RFOP | F_TROP | F_SIDE,
   0.0f, 1.0f, 1.0f, 0.5f, 1.0f, 0.0f, 0.4f, 1.562070f, 1, 1, 1, 0,
   F_LUMI | F_DIFF | F_TRAN | F_RIND | F_SMAN | F_RFOP | F_TROP | F_SIDE,
   1.0f, 0.0f, 0.0f, 0.5f, 1.0f, 0.0f, 0.0f, 1.562070f, 1, 1, 1, 0,
   F_DIFF | F_TRAN | F_RIND | F_SMAN | F_RFOP | F_TROP | F_SIDE | F_LINE,
   0.0f, 1.0f, 0.0f, 0.5f, 1.0f, 0.0f, 0.0f, 1.562070f, 1, 1, 1, 1,
   F_DIFF | F_TRAN | F_RIND | F_RFOP | F_TROP | F_SIDE,
   0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1, 1, 1, 0,
   F_LUMI | F_DIFF | F_TRAN | F_RIND | F_RFOP | F_TROP | F_SIDE,
   1.0f, 0.0f, 0.0f, 0.75f, 1.0f, 0.0f, 0.0f, 0.0f, 1, 1, 1, 0,
   F_LUMI | F_DIFF | F_TRAN | F_RIND | F_RFOP | F_TROP | F_SIDE,
   1.0f, 0.0f, 0.0f, 0.75f, 1.0f, 0.0f, 0.0f, 0.0f, 1, 1, 1, 0,
   F_DIFF | F_SPEC | F_REFL | F_GLOS | F_RFOP | F_TROP | F_SIDE,
   0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.4f, 0.0f, 1, 1, 1, 0,
   F_DIFF | F_SPEC | F_REFL | F_GLOS | F_SMAN | F_RFOP | F_TROP | F_SIDE,
   0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.4f, 1.562070f, 1, 1, 1, 0
};


#ifdef _WIN32
/*
=====================================================================
revbytes()

Reverses byte order in place.

INPUTS
   bp       bytes to reverse
   elsize   size of the underlying data type
   elcount  number of elements to swap

RESULTS
   Reverses the byte order in each of elcount elements.

IFF files in general, and the SURF chunks in LightWave object files
in particular, use a byte order variously called "big-endian",
"Motorola" or "Internet order".  So do most systems.  The important
exception is Windows, which is where the surface code needs this
function.  An empty macro is substituted for this function on other
systems.
===================================================================== */

static void revbytes( void *bp, int elsize, int elcount )
{
   register unsigned char *p, *q;

   p = ( unsigned char * ) bp;

   if ( elsize == 2 ) {
      q = p + 1;
      while ( elcount-- ) {
         *p ^= *q;
         *q ^= *p;
         *p ^= *q;
         p += 2;
         q += 2;
      }
      return;
   }

   while ( elcount-- ) {
      q = p + elsize - 1;
      while ( p < q ) {
         *p ^= *q;
         *q ^= *p;
         *p ^= *q;
         ++p;
         --q;
      }
      p += elsize >> 1;
   }
}

#else
#define revbytes(b,s,c)
#endif

#define BWRITE(b,p,s,c) (memcpy(b,p,s*c), revbytes(b,s,c), b+=s*c)

/*
======================================================================
get_surf()

Return the SURF name and data for a VideoScape color code.

Writes the surface name in the name string, if it's not NULL, and
writes the raw LightWave surface data in the buf argument, if that's
not NULL and bufsize is big enough.  Always returns the size of the
surface data in bytes, or 0 if the VideoScape code is invalid.
====================================================================== */

int get_surf( int index, char *name, unsigned char *buf, int bufsize )
{
   unsigned char *p = buf;
   unsigned short s, uzero = 0;
   int i, c, n;

   if ( index < 0 || index > 260 ) return 0;

   if ( index < 256 ) {
      i = index >> 4;
      c = index & 0xF;
   }
   else
      i = c = index - 240;

   if ( name ) sprintf( name, "VideoScape_%d", index );

   s = 14;
   n = s + 6;
   if ( buf && ( bufsize >= n )) {
      BWRITE( p, &ckid[ 0 ], 4, 1 );
      BWRITE( p, &s, 2, 1 );
      BWRITE( p, color[ c ], 4, 3 );
      BWRITE( p, &uzero, 2, 1 );
   }

   if ( surf[ i ].flags & F_LUMI ) {
      s = 6;
      n += s + 6;
      if ( buf && ( bufsize >= n )) {
         BWRITE( p, &ckid[ 1 ], 4, 1 );
         BWRITE( p, &s, 2, 1 );
         BWRITE( p, &surf[ i ].lumi, 4, 1 );
         BWRITE( p, &uzero, 2, 1 );
      }
   }

   if ( surf[ i ].flags & F_DIFF ) {
      s = 6;
      n += s + 6;
      if ( buf && ( bufsize >= n )) {
         BWRITE( p, &ckid[ 2 ], 4, 1 );
         BWRITE( p, &s, 2, 1 );
         BWRITE( p, &surf[ i ].diff, 4, 1 );
         BWRITE( p, &uzero, 2, 1 );
      }
   }

   if ( surf[ i ].flags & F_SPEC ) {
      s = 6;
      n += s + 6;
      if ( buf && ( bufsize >= n )) {
         BWRITE( p, &ckid[ 3 ], 4, 1 );
         BWRITE( p, &s, 2, 1 );
         BWRITE( p, &surf[ i ].spec, 4, 1 );
         BWRITE( p, &uzero, 2, 1 );
      }
   }

   if ( surf[ i ].flags & F_TRAN ) {
      s = 6;
      n += s + 6;
      if ( buf && ( bufsize >= n )) {
         BWRITE( p, &ckid[ 4 ], 4, 1 );
         BWRITE( p, &s, 2, 1 );
         BWRITE( p, &surf[ i ].tran, 4, 1 );
         BWRITE( p, &uzero, 2, 1 );
      }
   }

   if ( surf[ i ].flags & F_RIND ) {
      s = 6;
      n += s + 6;
      if ( buf && ( bufsize >= n )) {
         BWRITE( p, &ckid[ 5 ], 4, 1 );
         BWRITE( p, &s, 2, 1 );
         BWRITE( p, &surf[ i ].rind, 4, 1 );
         BWRITE( p, &uzero, 2, 1 );
      }
   }

   if ( surf[ i ].flags & F_REFL ) {
      s = 6;
      n += s + 6;
      if ( buf && ( bufsize >= n )) {
         BWRITE( p, &ckid[ 6 ], 4, 1 );
         BWRITE( p, &s, 2, 1 );
         BWRITE( p, &surf[ i ].refl, 4, 1 );
         BWRITE( p, &uzero, 2, 1 );
      }
   }

   if ( surf[ i ].flags & F_GLOS ) {
      s = 6;
      n += s + 6;
      if ( buf && ( bufsize >= n )) {
         BWRITE( p, &ckid[ 7 ], 4, 1 );
         BWRITE( p, &s, 2, 1 );
         BWRITE( p, &surf[ i ].glos, 4, 1 );
         BWRITE( p, &uzero, 2, 1 );
      }
   }

   if ( surf[ i ].flags & F_SMAN ) {
      s = 4;
      n += s + 6;
      if ( buf && ( bufsize >= n )) {
         BWRITE( p, &ckid[ 8 ], 4, 1 );
         BWRITE( p, &s, 2, 1 );
         BWRITE( p, &surf[ i ].sman, 4, 1 );
      }
   }

   if ( surf[ i ].flags & F_RFOP ) {
      s = 2;
      n += s + 6;
      if ( buf && ( bufsize >= n )) {
         BWRITE( p, &ckid[ 9 ], 4, 1 );
         BWRITE( p, &s, 2, 1 );
         BWRITE( p, &surf[ i ].rfop, 2, 1 );
      }
   }

   if ( surf[ i ].flags & F_TROP ) {
      s = 2;
      n += s + 6;
      if ( buf && ( bufsize >= n )) {
         BWRITE( p, &ckid[ 10 ], 4, 1 );
         BWRITE( p, &s, 2, 1 );
         BWRITE( p, &surf[ i ].trop, 2, 1 );
      }
   }

   if ( surf[ i ].flags & F_SIDE ) {
      s = 2;
      n += s + 6;
      if ( buf && ( bufsize >= n )) {
         BWRITE( p, &ckid[ 11 ], 4, 1 );
         BWRITE( p, &s, 2, 1 );
         BWRITE( p, &surf[ i ].side, 2, 1 );
      }
   }

   if ( surf[ i ].flags & F_LINE ) {
      s = 2;
      n += s + 6;
      if ( buf && ( bufsize >= n )) {
         BWRITE( p, &ckid[ 12 ], 4, 1 );
         BWRITE( p, &s, 2, 1 );
         BWRITE( p, &surf[ i ].line, 2, 1 );
      }
   }

   return n;
}


/*
======================================================================
vcolor()

Return the nearest VideoScape color, given RGB levels.
====================================================================== */

static int vcolor( float r, float g, float b )
{
   float dr, dg, db, d2, dmin;
   int i, index;

   dmin = 3.1f;

   for ( i = 0; i < 16; i++ ) {
      dr = color[ i ][ 0 ] - r;
      dg = color[ i ][ 1 ] - g;
      db = color[ i ][ 2 ] - b;
      d2 = dr * dr + dg * dg + db * db;
      if ( d2 < dmin ) { dmin = d2; index = i; }
   }

   return index;
}


/*
======================================================================
get_colorcode()

Return the VideoScape color code that best matches the surface.
====================================================================== */

int get_colorcode( LWSurfaceFuncs *surff, LWSurfaceID surfid )
{
   VSSurf s;
   double *dval;
   float r, g, b;
   int c;

   dval = surff->getFlt( surfid, SURF_COLR );
   r = ( float ) dval[ 0 ];
   g = ( float ) dval[ 1 ];
   b = ( float ) dval[ 2 ];

   dval = surff->getFlt( surfid, SURF_LUMI );
   s.lumi = ( float ) *dval;

   dval = surff->getFlt( surfid, SURF_DIFF );
   s.diff = ( float ) *dval;

   dval = surff->getFlt( surfid, SURF_SPEC );
   s.spec = ( float ) *dval;

   dval = surff->getFlt( surfid, SURF_REFL );
   s.refl = ( float ) *dval;

   dval = surff->getFlt( surfid, SURF_TRAN );
   s.tran = ( float ) *dval;

   dval = surff->getFlt( surfid, SURF_RIND );
   s.rind = ( float ) *dval;

   dval = surff->getFlt( surfid, SURF_GLOS );
   s.glos = ( float ) *dval;

   dval = surff->getFlt( surfid, SURF_SMAN );
   s.sman = ( float ) *dval;

   s.rfop = ( unsigned short ) surff->getInt( surfid, SURF_RFOP );
   s.trop = ( unsigned short ) surff->getInt( surfid, SURF_TROP );
   s.line = ( unsigned short ) surff->getInt( surfid, SURF_LINE );

   if ( s.refl > 0.5f ) {
      if ( s.sman > 0.0f )
         c = 260;                                // chrome smooth
      else
         c = 259;                                // chrome facet
   }
   else if ( s.tran > 0.99f ) {
      c = 256;                                   // invisible
   }
   else if ( s.lumi > 0.9f && s.diff < 0.1f && s.tran > 0.7f ) {
      if ( r < 0.75f && g < 0.75f && b < 0.75f )
         c = 257;                                // subtractive
      else
         c = 258;                                // additive
   }
   else {
      c = vcolor( r, g, b );

      if ( s.line )               c += 48;       // outline
      else if ( s.lumi > s.diff ) c += 32;       // luminous
      else if ( s.spec > 0.0f )   c += 16;       // glossy

      if ( s.tran > 0.0f )        c += 64;       // transparent
      if ( s.sman > 0.0f )        c += 128;      // smooth
   }

   return c;
}
