/* 
 * LWOB2.C -- Dump a LightWave LWO2 file.
 *
 *         Copyright (c) 1999 D-STORM, Inc. 
 *         All Rights Reserved 
 *
 * written by Yoshiaki Tazaki
 *
 *         Feb 09, 2000   Update
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAKE_ID(a,b,c,d)	\
	((unsigned long) (a)<<24 | (unsigned long) (b)<<16 | \
	 (unsigned long) (c)<<8 | (unsigned long) (d))

/* Universal IFF identifiers */

#define ID_FORM		MAKE_ID('F','O','R','M')
#define ID_LWO2		MAKE_ID('L','W','O','2')

/**  PRIMARY CHUNK ID  **/
#define ID_LAYR		MAKE_ID('L','A','Y','R')
#define ID_PNTS		MAKE_ID('P','N','T','S')
#define ID_VMAP		MAKE_ID('V','M','A','P')
#define ID_POLS		MAKE_ID('P','O','L','S')
#define ID_TAGS		MAKE_ID('T','A','G','S')
#define ID_PTAG		MAKE_ID('P','T','A','G')
#define ID_ENVL		MAKE_ID('E','N','V','L')
#define ID_CLIP		MAKE_ID('C','L','I','P')
#define ID_SURF		MAKE_ID('S','U','R','F')
#define ID_BBOX		MAKE_ID('B','B','O','X')
#define ID_DESC		MAKE_ID('D','E','S','C')
#define ID_TEXT		MAKE_ID('T','E','X','T')
#define ID_ICON		MAKE_ID('I','C','O','N')

/**  POLS TYPE  **/
#define ID_FACE		MAKE_ID('F','A','C','E')
#define ID_CRVS		MAKE_ID('C','U','R','V')
#define ID_PCHS		MAKE_ID('P','T','C','H')
#define ID_MBAL		MAKE_ID('M','B','A','L')
#define ID_BONE		MAKE_ID('B','O','N','E')

/**  PTAG TYPE  **/
#define ID_SURF		MAKE_ID('S','U','R','F')
#define ID_BNID		MAKE_ID('B','N','I','D')
#define ID_SGMP		MAKE_ID('S','G','M','P')
#define ID_PART		MAKE_ID('P','A','R','T')

/**  IMAGE SUB-CHUNK ID  */
#define ID_STIL		MAKE_ID('S','T','I','L')
#define ID_ISEQ		MAKE_ID('I','S','E','Q')
#define ID_ANIM		MAKE_ID('A','N','I','M')
#define ID_XREF		MAKE_ID('X','R','E','F')
#define ID_STCC		MAKE_ID('S','T','C','C')
#define ID_CONT		MAKE_ID('C','O','N','T')
#define ID_BRIT		MAKE_ID('B','R','I','T')
#define ID_SATR		MAKE_ID('S','A','T','R')
#define ID_HUE		MAKE_ID('H','U','E',' ')
#define ID_GAMM		MAKE_ID('G','A','M','M')
#define ID_NEGA		MAKE_ID('N','E','G','A')
#define ID_CROP		MAKE_ID('C','R','O','P')
#define ID_ALPH		MAKE_ID('A','L','P','H')
#define ID_COMP		MAKE_ID('C','O','M','P')
#define ID_IFLT		MAKE_ID('I','F','L','T')
#define ID_PFLT		MAKE_ID('P','F','L','T')

/**  ENVELOPE SUB-CHUNK  **/
#define ID_PRE		MAKE_ID('P','R','E',' ')
#define ID_POST		MAKE_ID('P','O','S','T')
#define ID_KEY		MAKE_ID('K','E','Y',' ')
#define ID_SPAN		MAKE_ID('S','P','A','N')
#define ID_CHAN		MAKE_ID('C','H','A','N')

/**  SURFACE SUB-CHUNK ID  */
#define ID_COLR		MAKE_ID('C','O','L','R')
#define ID_DIFF		MAKE_ID('D','I','F','F')
#define ID_LUMI		MAKE_ID('L','U','M','I')
#define ID_SPEC		MAKE_ID('S','P','E','C')
#define ID_REFL		MAKE_ID('R','E','F','L')
#define ID_TRAN		MAKE_ID('T','R','A','N')
#define ID_TRNL		MAKE_ID('T','R','N','L')
#define ID_GLOS		MAKE_ID('G','L','O','S')
#define ID_SHRP		MAKE_ID('S','H','R','P')
#define ID_BUMP		MAKE_ID('B','U','M','P')
#define ID_SIDE		MAKE_ID('S','I','D','E')
#define ID_SMAN		MAKE_ID('S','M','A','N')
#define ID_RFOP		MAKE_ID('R','F','O','P')
#define ID_RIMG		MAKE_ID('R','I','M','G')
#define ID_RSAN		MAKE_ID('R','S','A','N')
#define ID_RIND		MAKE_ID('R','I','N','D')
#define ID_CLRH		MAKE_ID('C','L','R','H')
#define ID_TROP		MAKE_ID('T','R','O','P')
#define ID_TIMG		MAKE_ID('T','I','M','G')
#define ID_CLRF		MAKE_ID('C','L','R','F')
#define ID_ADTR		MAKE_ID('A','D','T','R')
#define ID_GLOW		MAKE_ID('G','L','O','W')
#define ID_LINE		MAKE_ID('L','I','N','E')
#define ID_ALPH		MAKE_ID('A','L','P','H')
#define ID_AVAL		MAKE_ID('A','V','A','L')
#define ID_GVAL		MAKE_ID('G','V','A','L')
#define ID_BLOK		MAKE_ID('B','L','O','K')
#define ID_LCOL		MAKE_ID('L','C','O','L')
#define ID_LSIZ		MAKE_ID('L','S','I','Z')
#define ID_CMNT		MAKE_ID('C','M','N','T')

/**  TEXTURE LAYER  **/
#define ID_CHAN		MAKE_ID('C','H','A','N')
#define ID_TYPE		MAKE_ID('T','Y','P','E')
#define ID_NAME		MAKE_ID('N','A','M','E')
#define ID_ENAB		MAKE_ID('E','N','A','B')
#define ID_OPAC		MAKE_ID('O','P','A','C')
#define ID_FLAG		MAKE_ID('F','L','A','G')
#define ID_PROJ		MAKE_ID('P','R','O','J')
#define ID_STCK		MAKE_ID('S','T','C','K')
#define ID_TAMP		MAKE_ID('T','A','M','P')

/**  TEXTURE MAPPING  **/
#define ID_TMAP		MAKE_ID('T','M','A','P')
#define ID_AXIS		MAKE_ID('A','X','I','S')
#define ID_CNTR		MAKE_ID('C','N','T','R')
#define ID_SIZE		MAKE_ID('S','I','Z','E')
#define ID_ROTA		MAKE_ID('R','O','T','A')
#define ID_OREF		MAKE_ID('O','R','E','F')
#define ID_FALL		MAKE_ID('F','A','L','L')
#define ID_CSYS		MAKE_ID('C','S','Y','S')

/**  IMAGE MAP  **/
#define ID_IMAP		MAKE_ID('I','M','A','P')
#define ID_IMAG		MAKE_ID('I','M','A','G')
#define ID_WRAP		MAKE_ID('W','R','A','P')
#define ID_WRPW		MAKE_ID('W','R','P','W')
#define ID_WRPH		MAKE_ID('W','R','P','H')
#define ID_VMAP		MAKE_ID('V','M','A','P')
#define ID_AAST		MAKE_ID('A','A','S','T')
#define ID_PIXB		MAKE_ID('P','I','X','B')

/**  PROCUDUAL TEXTURE  **/
#define ID_PROC		MAKE_ID('P','R','O','C')
#define ID_COLR		MAKE_ID('C','O','L','R')
#define ID_VALU		MAKE_ID('V','A','L','U')
#define ID_FUNC		MAKE_ID('F','U','N','C')
#define ID_FTPS		MAKE_ID('F','T','P','S')
#define ID_ITPS		MAKE_ID('I','T','P','S')
#define ID_ETPS		MAKE_ID('E','T','P','S')

/**  GRADIENT **/
#define ID_GRAD		MAKE_ID('G','R','A','D')
#define ID_GRST		MAKE_ID('G','R','S','T')
#define ID_GREN		MAKE_ID('G','R','E','N')

/**  SHADER PLUGIN  */
#define ID_SHDR		MAKE_ID('S','H','D','R')
#define ID_DATA		MAKE_ID('D','A','T','A')


#ifdef _WIN32
#define MSB2			_SwapTwoBytes
#define MSB4			_SwapFourBytes 
#define LSB2(w)			(w)
#define LSB4(w)			(w)
#else
#define MSB2(w)			(w)
#define MSB4(w)			(w)
#define LSB2			_SwapTwoBytes
#define LSB4			_SwapFourBytes 
#endif


typedef char            I1;
typedef short           I2;
typedef int             I4;
typedef unsigned char   U1;
typedef unsigned short  U2;
typedef unsigned int    U4;
typedef float           F4;
typedef unsigned int    VX;
typedef float           FP4;
typedef float           ANG4;
typedef float           VEC12[3];
typedef float           COL12[3];
typedef char            ID4[5];
typedef char            S0[255];
typedef char            FNAM0[255];

/*
 *  Swap byte order for WIN32
 */
unsigned short _SwapTwoBytes (unsigned short w)
{
	unsigned short tmp;
	tmp =  (w & 0x00ff);
	tmp = ((w & 0xff00) >> 0x08) | (tmp << 0x08);
	return tmp;
}

unsigned long _SwapFourBytes (unsigned long w)
{
	unsigned long tmp;
	tmp =  (w & 0x000000ff);
	tmp = ((w & 0x0000ff00) >> 0x08) | (tmp << 0x08);
	tmp = ((w & 0x00ff0000) >> 0x10) | (tmp << 0x08);
	tmp = ((w & 0xff000000) >> 0x18) | (tmp << 0x08);
	return tmp;
}

static int seek_pad( int size, FILE *file )
{
	if (size > 0) fseek(file, (long)size, SEEK_CUR);
	return size;
}

static int read_u1( U1 *vals, int num, FILE *file )
{
    return (int) fread((void *)vals, sizeof(U1), num, file);
}

static int read_u2( U2 *vals, int num, FILE *file )
{
	int   i, bytesread;

    bytesread = (int) fread((void *)vals, sizeof(U2), num, file);
    if (bytesread != num) {
		return 0;
	} else {
		for (i = 0; i < num; i++) {
			vals[i] = MSB2(vals[i]);
		}
		return sizeof(U2) * num;
	}
}


static int read_u4( U4 *vals, int num, FILE *file )
{
	int   i, bytesread;

    bytesread = (int) fread((void *)vals, sizeof(U4), num, file);
    if (bytesread != num) {
		return 0;
	} else {
		for (i = 0; i < num; i++) {
			vals[i] = MSB4(vals[i]);
		}
		return sizeof(U4) * num;
	}
}


static int read_i1( I1 *vals, int num, FILE *file )
{
	return read_u1((U1 *)vals, num, file);
}


static int read_i2( I2 *vals, int num, FILE *file )
{
	return read_u2((U2 *)vals, num, file);
}


static int read_f4( F4 *vals, int num, FILE *file )
{
	return read_u4((U4 *)vals, num, file);
}


static int read_col12( COL12 *col, int num, FILE *file )
{
	return read_f4((F4 *)col, 3 * num, file);
}


static int read_vec12( VEC12 *vec, int num, FILE *file )
{
	return read_f4((F4 *)vec, 3 * num, file);
}


static int read_vx( VX *vx, FILE *file )
{
	int   ch, bytesread = 0;

	*vx = 0L;
	ch = fgetc(file);
	if (ch == 0xff) {
		ch = fgetc(file); *vx |= ch << 16;
		ch = fgetc(file); *vx |= ch << 8;
		ch = fgetc(file); *vx |= ch;
		bytesread = 4;
	} else {
		*vx |= ch << 8;
		ch = fgetc(file); *vx |= ch;
		bytesread = 2;
	}
	return bytesread;
}


static int read_name( S0 name, FILE *file )
{
	int   ch, bytesread = 0;

    do  {
        ch = fgetc(file);
        name[bytesread++] = ch;
    } while (ch);

	if (bytesread & 1) {
        ch = fgetc(file); bytesread++;
	}
	return bytesread;
}


static int read_id4( ID4 id, FILE *file )
{
	int   bytesread = 0;

    id[bytesread++] = fgetc(file);
    id[bytesread++] = fgetc(file);
    id[bytesread++] = fgetc(file);
    id[bytesread++] = fgetc(file);
    id[bytesread  ] = 0x00;

	return bytesread;
}


/*
 * Prints an error message to stderr and exits.
 */

static void error(const char *message)
{
    fprintf(stderr, "Error: %s\n", message);
    exit(1);
}


/*
 * Reads vertices, and returns bytes read.
 */

static U4 read_pnts(U4 nbytes, FILE *file)
{
	U4     nPts, i, bytesread = 0;
    VEC12  *fpt;

    nPts = nbytes/sizeof(VEC12);
    printf("PNTS [%d] nPts [%d]\n", nbytes, nPts);

    fpt = (VEC12 *) malloc (sizeof(VEC12) * nPts);
	bytesread = read_vec12(fpt, nPts, file);

    for (i = 0; i < nPts; i++) 
	{
		printf("\t[%d] [%f,%f,%f]\n", i, fpt[i][0], fpt[i][1], fpt[i][2]);
    }

    free(fpt);
    return(bytesread);
}


/*
 * Reads Bounding Box.
 * Returns the bytes to read.
 */

static U4 read_bbox(U4 nbytes, FILE *file)
{
    F4	bbox[6];

	read_f4(bbox, 6, file);
    printf("BBOX [%d]\n", nbytes);
    printf("\tMIN [%f,%f,%f]\n", bbox[0], bbox[1], bbox[2]);
    printf("\tMAX [%f,%f,%f]\n", bbox[3], bbox[4], bbox[5] );

    return(nbytes);
}


/*
 * Reads polygons, and returns bytes read.
 */

static U4 read_pols(U4 nbytes, FILE *file)
{
    U2	  numvert, flags;
    U4	  nPols, bytesread;
	VX    vx;
	ID4   id;
	int   n;

    printf("POLS [%d]", nbytes);

	bytesread = 0L;
    nPols = 0L;

	bytesread += read_id4( id, file );
	printf(" [%s]\n", id);

	while (bytesread < nbytes)
	{
		bytesread += read_u2(&numvert, 1, file);
		flags      = (0xfc00 & numvert) >> 10;
		numvert    =  0x03ff & numvert;
		printf("\t[%d] NVERT[%d] FLAG[%02x] <", nPols, numvert, flags);
		nPols++;

		for (n = 0; n < (int)numvert; n++)
		{
			bytesread += read_vx( &vx, file );
			if (n+1 == numvert) printf("%d>\n", vx);
			else                printf("%d, ", vx);
		}
	}
	if (bytesread != nbytes) printf("??? %d != %d\n", bytesread, nbytes);

    return(bytesread);
}


/*
 * Reads polygon tags, and returns bytes read.
 */

static U4 read_ptag(U4 nbytes, FILE *file)
{
    U2	 tag;
    U4	 nTags, bytesread;
	VX   vx;
	ID4  id;

    printf("PTAG [%d]", nbytes);

	bytesread = 0L;
    nTags = 0L;

	bytesread += read_id4( id, file );
	printf(" [%s]\n", id);

	while (bytesread < nbytes)
	{
		bytesread += read_vx( &vx, file );
		bytesread += read_u2( &tag, 1, file );
		printf("\tPOLY[%d] TAG[%d]\n", vx, tag); nTags++;
	}
	if (bytesread != nbytes) printf("??? %d != %d\n", bytesread, nbytes);

    return(bytesread);
}


/*
 * Reads vertex mapping, and returns bytes read.
 */

static U4 read_vmap(U4 nbytes, FILE *file)
{
    U2	 dim;
    U4	 bytesread;
	VX   vx;
	F4   value;
	S0   name;
	ID4  id;
	int  n;

    printf("VMAP [%d]", nbytes);

	bytesread = 0L;

	bytesread += read_id4( id, file );
	printf(" [%s]", id);

	bytesread += read_u2  ( &dim, 1, file );
	bytesread += read_name( name, file );
	printf(" DIM [%d] NAME [%s]\n", dim, name);

	while (bytesread < nbytes)
	{
		bytesread += read_vx( &vx, file );
		if (dim == 0) {
			printf("\tVERT[%d]\n", vx);
		} else {
			printf("\tVERT[%d] VALS[", vx);
			for (n = 0; n < (int) dim; n++) {
				bytesread += read_f4(&value, 1, file);
				if (n+1 == dim) printf("%f]\n", value);
				else            printf("%f, " , value);
			}
		}
	}
	if (bytesread != nbytes) printf("??? %d != %d\n", bytesread, nbytes);

    return(bytesread);
}


/*
 * Reads a BLOK header chunk, and returns bytes read.
 */

static U4 read_head(U4 nbytes, FILE *file)
{
    U4	    bytesread = 0, type, byteshold;
	U2      size, u2[4];
	VX      vx[4];
	F4      f4[4];
    ID4     id;
    S0		name;

	bytesread += read_name(name, file);
	printf("<%s>\n", name);

    while (bytesread < nbytes) 
    {

      /* Handle the various sub-chunks. */
	  bytesread += read_id4( id, file );
	  bytesread += read_u2( &size, 1, file );
	  type = MAKE_ID(id[0],id[1],id[2],id[3]);

	  byteshold = bytesread;
      printf("\t\t\t[%s] (%d) ", id, size);

      switch (type)
      {
      case ID_CHAN:
		bytesread += read_id4(id, file);
	    printf("<%s>\n", id);
	    break;
      case ID_NAME:
      case ID_OREF:
		bytesread += read_name(name, file);
	    printf("<%s>\n", name);
	    break;
      case ID_ENAB:
      case ID_AXIS:
      case ID_NEGA:
		bytesread += read_u2(u2, 1, file);
	    printf("<%d>\n", u2[0]);
	    break;
      case ID_OPAC:
		bytesread += read_u2(u2, 1, file);
		bytesread += read_f4(f4, 1, file);
		bytesread += read_vx(vx, file);
	    printf("<%d> <%f> <%d>\n", u2[0], f4[0], vx[0]);
	    break;
      default:
		bytesread += seek_pad(size, file);
        printf("(%d bytes)\n", size);
		break;
	  }
	  if ((size - bytesread + byteshold) > 0) {
		  bytesread += seek_pad((size - bytesread + byteshold), file);
	  }
    }

    return(bytesread);
}


/*
 * Reads a TMAP chunk, and returns bytes read.
 */

static U4 read_tmap(U4 nbytes, FILE *file)
{
    U4	    bytesread = 0, type, byteshold;
	U2      size, u2[4];
	VX      vx[4];
    ID4     id;
    VEC12	vec;
    S0		name;

    printf("\n");

    while (bytesread < nbytes) 
    {

      /* Handle the various sub-chunks. */
	  bytesread += read_id4( id, file );
	  bytesread += read_u2( &size, 1, file );
	  type = MAKE_ID(id[0],id[1],id[2],id[3]);

	  byteshold = bytesread;
      printf("\t\t\t[%s] (%d) ", id, size);

      switch (type)
      {
      case ID_CNTR:
      case ID_SIZE:
      case ID_ROTA:
		bytesread += read_vec12(&vec, 1, file);
		bytesread += read_vx(vx, file);
	    printf("<%f,%f,%f> <%d>\n", vec[0], vec[1], vec[2], vx[0]);
	    break;
      case ID_FALL:
		bytesread += read_u2(u2, 1, file);
		bytesread += read_vec12(&vec, 1, file);
		bytesread += read_vx(vx, file);
	    printf("<%d> <%f,%f,%f> <%d>\n", u2[0], vec[0], vec[1], vec[2], vx[0]);
	    break;
      case ID_OREF:
		bytesread += read_name(name, file);
	    printf("<%s>\n", name);
	    break;
      case ID_CSYS:
		bytesread += read_u2(u2, 1, file);
	    printf("<%d>\n", u2[0]);
	    break;
      default:
		bytesread += seek_pad(size, file);
        printf("(%d bytes)\n", size);
		break;
	  }
	  if ((size - bytesread + byteshold) > 0) {
		  bytesread += seek_pad((size - bytesread + byteshold), file);
	  }
    }

    return(bytesread);
}


/*
 * Reads a BLOK chunk, and returns bytes read.
 */

static U4 read_blok(U4 nbytes, FILE *file)
{
    U4	    bytesread = 0, type, byteshold;
	U2      size, u2[4];
	U1		u1[10];
	S0      name;
	F4      f4[256];
	I2      i2[256];
	VX      vx[4];
    ID4     id;
	COL12   col;
	int     i, n;

    while (bytesread < nbytes) 
    {

      /* Handle the various sub-chunks. */
	  bytesread += read_id4( id, file );
	  bytesread += read_u2( &size, 1, file );
	  type = MAKE_ID(id[0],id[1],id[2],id[3]);

	  byteshold = bytesread;
      printf("\t\t[%s] (%d) ", id, size);

      switch (type)
      {
      case ID_IMAP:
      case ID_PROC:
      case ID_GRAD:
      case ID_SHDR:
	    bytesread += read_head(size, file);
	    break;
      case ID_VMAP:
		bytesread += read_name(name, file);
	    printf("<%s>\n", name);
	    break;
      case ID_FLAG:
      case ID_AXIS:
      case ID_PROJ:
      case ID_PIXB:
		bytesread += read_u2(u2, 1, file);
	    printf("<%d>\n", u2[0]);
	    break;
      case ID_TMAP:
	    bytesread += read_tmap(size, file);
	    break;
      case ID_IMAG:
		bytesread += read_vx(vx, file);
	    printf("<%d>\n", vx[0]);
	    break;
      case ID_WRAP:
		bytesread += read_u2(u2, 2, file);
	    printf("<%d, %d>\n", u2[0], u2[1]);
	    break;
      case ID_WRPW:
      case ID_WRPH:
      case ID_TAMP:
		bytesread += read_f4(f4, 1, file);
		bytesread += read_vx(vx, file);
	    printf("<%f> <%d>\n", f4[0], vx[0] );
	    break;
      case ID_VALU:
		bytesread += read_f4(f4, size / sizeof(F4), file);
		for (i = 0; i < (int)(size / sizeof(F4)); i++) {
	    	printf("<%f> ", f4[i] );
		}
	    printf("\n");
	    break;
      case ID_AAST:
      case ID_STCK:
		bytesread += read_u2(u2, 1, file);
		bytesread += read_f4(f4, 1, file);
	    printf("<%d> <%f>\n", u2[0], f4[0] );
	    break;
      case ID_GRST:
      case ID_GREN:
		bytesread += read_f4(f4, 1, file);
	    printf("<%f>\n", f4[0] );
	    break;
      case ID_COLR:
		bytesread += read_col12(&col, 1, file);
		bytesread += read_vx(vx, file);
	    printf("<%f,%f,%f> <%d>\n", col[0], col[1], col[2], vx[0]);
	    break;
      case ID_FUNC:
		bytesread += n = read_name(name, file);
	    printf("<%s> ", name);
		for (i = 0; i < (size -n); i++) {
			bytesread += read_u1(u1, 1, file);
			printf("<0x%02x> ", u1[0]);
		}
		printf("\n");
		break;
      case ID_FTPS:
		n = size / sizeof(F4);
		bytesread += read_f4(f4, n, file);
		for (i = 0; i < n; i++) printf("<%f> ", f4[i]);
		printf("\n");
		break;
      case ID_ITPS:
		n = size / sizeof(I2);
		bytesread += read_i2(i2, n, file);
		for (i = 0; i < n; i++) printf("<%d> ", i2[i]);
		printf("\n");
	    break;
      case ID_ETPS:
		while (size > 0) {
			n = read_vx(vx, file);
			bytesread += n;
			size -= n;
			printf("<%d> ", vx[0]);
		}
	    printf("\n");
	    break;
      default:
		bytesread += seek_pad(size, file);
        printf("(%d bytes)\n", size);
		break;
	  }
	  if ((size - bytesread + byteshold) > 0) {
		  bytesread += seek_pad((size - bytesread + byteshold), file);
	  }
    }

    return(bytesread);
}


/*
 * Reads a SURF chunk, and returns bytes read.
 */

static U4 read_surf(U4 nbytes, FILE *file)
{
    U4	    bytesread = 0, type, byteshold;
    U2      size, u2[4];
    F4      f4[4];
	VX      vx[4];
    S0      name, source, s0;
	ID4     id;
	COL12   col;

    printf("SURF [%d]\n", nbytes);

	bytesread += read_name( name  , file );
	bytesread += read_name( source, file );

    printf("[%s] [%s]\n", name, source);

    while (bytesread < nbytes) 
    {
	  if ((nbytes - bytesread) < 6) {
		  bytesread += seek_pad((nbytes - bytesread), file);
    	  return(bytesread);
	  }

      /* Handle the various sub-chunks. */
	  bytesread += read_id4( id, file );
	  bytesread += read_u2( &size, 1, file );
	  type = MAKE_ID(id[0],id[1],id[2],id[3]);

	  byteshold = bytesread;
      printf("\t[%s] (%d) ", id, size);

      switch (type)
      {
      case ID_COLR:
      case ID_LCOL:
		bytesread += read_col12(&col, 1, file);
		bytesread += read_vx(vx, file);
	    printf("<%f,%f,%f> <%d>\n", col[0], col[1], col[2], vx[0]);
	    break;
      case ID_DIFF:
      case ID_LUMI:
      case ID_SPEC:
      case ID_REFL:
      case ID_TRAN:
      case ID_TRNL:
      case ID_GLOS:
      case ID_SHRP:
      case ID_BUMP:
      case ID_RSAN:
      case ID_RIND:
      case ID_CLRH:
      case ID_CLRF:
      case ID_ADTR:
      case ID_GVAL:
      case ID_LSIZ:
		bytesread += read_f4(f4, 1, file);
		bytesread += read_vx(vx, file);
	    printf("<%f> <%d>\n", f4[0], vx[0]);
		break;
      case ID_SIDE:
      case ID_RFOP:
      case ID_TROP:
		bytesread += read_u2(u2, 1, file);
		bytesread += seek_pad(size-sizeof(U2), file);
	    printf("<%d>\n", u2[0]);
		break;
      case ID_SMAN:
		bytesread += read_f4(f4, 1, file);
	    printf("<%f>\n", f4[0]);
		break;
      case ID_RIMG:
      case ID_TIMG:
		bytesread += read_vx(vx, file);
	    printf("<%d>\n", vx[0]);
		break;
      case ID_GLOW:
		bytesread += read_u2(u2  , 1, file);
		bytesread += read_f4(f4  , 1, file);
		bytesread += read_vx(vx  , file);
		bytesread += read_f4(f4+1, 1, file);
		bytesread += read_vx(vx+1, file);
	    printf("<%d> <%f> <%d> <%f> <%d>\n", u2[0], f4[0], vx[0], f4[1], vx[1]);
		break;
      case ID_LINE:
		bytesread += read_u2(u2, 1, file);
      if ( size > 2 ) {
		   bytesread += read_f4(f4, 1, file);
		   bytesread += read_vx(vx, file);
		   if (size > 8) {
			   bytesread += read_col12(&col, 1, file);
			   bytesread += read_vx(vx+1, file);
	    	   printf("<%d> <%f> <%d> <%f,%f,%f> <%d>\n", 
				   u2[0], f4[0], vx[0], col[0], col[1], col[2], vx[1]);
		   } else {
	    	   printf("<%d> <%f> <%d>\n", u2[0], f4[0], vx[0]);
		   }
      }
      else
         printf("<%d>\n", u2[0]);
		break;
      case ID_ALPH:
		bytesread += read_u2(u2, 1, file);
		bytesread += read_f4(f4, 1, file);
	    printf("<%d> <%f>\n", u2[0], f4[0]);
		break;
      case ID_AVAL:
		bytesread += read_f4(f4, 1, file);
	    printf("<%f>\n", f4[0]);
		break;
      case ID_BLOK:
	    printf("\n");
	    bytesread += read_blok(size, file);
		break;
      case ID_CMNT:
		memset( s0, 0x00, sizeof(s0) );
		bytesread += read_u1(s0, size, file);
	    printf("<%s>\n", s0);
		break;
      default:
		bytesread += seek_pad(size, file);
        printf("(%d bytes)\n", size);
      }
	  if ((size - bytesread + byteshold) > 0) {
		  bytesread += seek_pad((size - bytesread + byteshold), file);
	  }
    }

    return(bytesread);
}


/*
 * Reads a CLIP chunk, and returns bytes read.
 */

static U4 read_clip(U4 nbytes, FILE *file)
{
    U4	  bytesread = 0, bytes, type, index, byteshold;
    U2	  size, u2[4];
	U1    u1[4];
	I2    i2[5];
    F4    f4[4];
	VX    vx[4];
    S0    name, ext, server;
	ID4   id;

	bytesread += read_u4( &index, 1, file );
    printf("CLIP [%d] [%d]\n", nbytes, index);

    while (bytesread < nbytes) 
    {
	  if ((nbytes - bytesread) < 6) {
		  bytesread += seek_pad((nbytes - bytesread), file);
    	  return(bytesread);
	  }

      /* Handle the various sub-chunks. */
	  bytesread += read_id4( id, file );
	  bytesread += read_u2( &size, 1, file );
	  type = MAKE_ID(id[0],id[1],id[2],id[3]);
	  byteshold = bytesread;

      printf("\t[%s] (%d) ", id, size);

      switch (type)
      {
      case ID_STIL:
		bytesread += read_name(name, file);
	    printf("<%s>\n", name);
	    break;
      case ID_ISEQ:
		bytesread += read_u1(u1, 2, file);
		bytesread += read_i2(i2, 2, file);
		bytesread += read_name(name, file);
		bytesread += read_name(ext, file);
	    printf("<%d> <%d> <%d> <%d> <%s> <%s>\n", 
						u1[0], u1[1], i2[0], i2[1], name, ext);
		break;
      case ID_ANIM:
		bytesread += read_name(name, file);
		bytesread += read_name(server, file);
	    printf("<%s> <%s>\n", name, server);
	    break;
      case ID_XREF:
		bytesread += read_u4(&index, 1, file);
		bytesread += read_name(name, file);
	    printf("<%d> <%s>\n", index, name);
		break;
      case ID_ALPH:
		bytesread += read_vx(vx, file);
	    printf("<%d>\n", vx[0]);
		break;
      case ID_STCC:
		bytesread += read_i2(i2, 2, file);
		bytesread += read_name(name, file);
	    printf("<%d> <%d> <%s> <%s>\n", i2[0], i2[1], name);
		break;
      case ID_CONT:
      case ID_BRIT:
      case ID_SATR:
      case ID_HUE:
      case ID_GAMM:
		bytesread += read_f4(f4, 1, file);
		bytesread += read_vx(vx, file);
	    printf("<%f> <%d>\n", f4[0], vx[0]);
		break;
      case ID_NEGA:
		bytesread += read_u2(u2, 1, file);
	    printf("<%d>\n", u2[0]);
		break;
      case ID_CROP:
		bytesread += read_f4(f4, 4, file);
	    printf("<%f> <%f> <%f> <%f>\n", f4[0], f4[1], f4[2], f4[3]);
		break;
      case ID_COMP:
		bytesread += read_vx(vx, file);
		bytesread += read_f4(f4, 1, file);
		bytesread += read_vx(vx+1, file);
	    printf("<%d> <%f> <%d>\n", vx[0], f4[0], vx[1]);
		break;
      case ID_IFLT:
      case ID_PFLT:
        bytes = bytesread;
		bytesread += read_name(name, file);
		bytesread += read_i2(i2, 1, file);
		bytesread += seek_pad(size-(bytesread-bytes), file);
	    printf("<%s> <%d> \n", name, i2[0]);
		break;
      default:
		bytesread += seek_pad(size, file);
        printf("(%d bytes)\n", size);
      }
	  if ((size - bytesread + byteshold) > 0) {
		  bytesread += seek_pad((size - bytesread + byteshold), file);
	  }
    }

    return(bytesread);
}


/*
 * Reads a ENVL chunk, and returns bytes read.
 */

static U4 read_envl(U4 nbytes, FILE *file)
{
    U4   bytesread = 0, bytes, type, byteshold;
    U2	 size, u2[4], n, count;
	U1	 u1[4];
	I2	 index;
    F4	 f4[4];
    S0   name;
	ID4  id;

	bytesread += read_i2( &index, 1, file );
    printf("ENVL [%d] [%d]\n", nbytes, index);

    while (bytesread < nbytes) 
    {
	  if ((nbytes - bytesread) < 6) {
		  bytesread += seek_pad((nbytes - bytesread), file);
    	  return(bytesread);
	  }

      /* Handle the various sub-chunks. */
	  bytesread += read_id4( id, file );
	  bytesread += read_u2( &size, 1, file );
	  type = MAKE_ID(id[0],id[1],id[2],id[3]);

	  byteshold = bytesread;
      printf("\t[%s] (%d) ", id, size);

      switch (type)
      {
      case ID_PRE:
      case ID_POST:
		bytesread += read_u2(u2, 1, file);
	    printf("<%d>\n", u2[0]);
		break;
      case ID_TYPE:
		bytesread += read_u2(u2, 1, file);
	    printf("<%04x>\n", u2[0]);
		break;
      case ID_KEY:
		bytesread += read_f4(f4, 2, file);
	    printf("<%f> <%f>\n", f4[0], f4[1]);
		if (size != 8) size = 8;				// BUG FOR SurfaceEditor
		break;
      case ID_SPAN:
		bytes = bytesread;
		bytesread += read_id4(id, file);
	    printf("<%s>", id);
		count = (size - bytesread + bytes) / sizeof(F4);
		for (n = 0; n < count; n++) {
			bytesread += read_f4(f4, 1, file);
	    	printf(" <%f>", f4[0]);
		}
	    printf("\n");
		break;
      case ID_CHAN:
		bytes = bytesread;
		bytesread += read_name(name, file);
		bytesread += read_u2(u2, 1, file);
	    printf("<%s> <%d>\n", name, u2[0]);
		for (n = 0; n < (size-bytesread+bytes); n++) {
			bytesread += read_u1(u1, 1, file);
			if (!(n % 8)) printf("\t");
			printf("<0x%02x> ", u1[0]);
			if (!((n+1) % 8)) printf("\n");
		}
		printf("\n");
	    break;
      case ID_NAME:
		bytesread += read_name(name, file);
	    printf("<%s>\n", name);
	    break;
      default:
		bytesread += seek_pad(size, file);
        printf("(%d bytes)\n", size);
      }
	  if ((size - bytesread + byteshold) > 0) {
		  bytesread += seek_pad((size - bytesread + byteshold), file);
	  }
    }

    return(bytesread);
}


/*
 * Reads the TAGS chunk, and returns bytes read.
 */

static U4 read_tags(U4 nbytes, FILE *file)
{
	U4    bytesread = 0L, n = 0;
    S0    name;

    printf("TAGS [%d]\n", nbytes);

    while (bytesread < nbytes) 
    {
		bytesread += read_name(name, file);
		printf("\t[%d] [%s]\n", n++, name );
	}

    return(bytesread);
}


/*
 * Reads the LAYR chunk, and returns bytes read.
 */

static U4 read_layr(U4 nbytes, FILE *file)
{
	U4       bytesread = 0L;
	U2       flags[2], parent[1];
	VEC12    pivot;
    S0       name;

    printf("\nLAYR [%d]", nbytes);

	/*  Layer no. and flags  */
	bytesread += read_u2(flags, 2, file);

	/*  Pivot point  */
	bytesread += read_vec12(&pivot, 1, file);
	bytesread += read_name(name, file);

	printf(" NO [%d] NAME [%s]\n", flags[0], name);
	printf("\tFLAGS [0x%04x] PIVOT [%f,%f,%f]\n", flags[1], pivot[0], pivot[1], pivot[2]);

	if ((nbytes-bytesread) == sizeof(U2)) {
		bytesread += read_u2(parent, 1, file);
		printf("\tPARENT [%d]\n", parent[0]);
	}

    return (bytesread);
}


void main(int argc, char *argv[]) 
{
    FILE  *file;
    U4	  datasize, bytesread = 0L;
    U4	  type, size;
    ID4   id;

    if (argc != 2) {
       fprintf( stderr, "Usage: %s [> dump.dat] object.lwo\n", argv[ 0 ] );
       exit( 1 );
    }

	/* Open the LWO2 file */
    file = fopen(argv[1], "rb");

    /* Make sure the Lightwave file is an IFF file. */
	read_id4(id, file);
	type = MAKE_ID(id[0],id[1],id[2],id[3]);
    if (type != ID_FORM)
      error("Not an IFF file (Missing FORM tag)");

	read_u4(&datasize, 1, file);

    printf("FORM [%d]\n", datasize);

    /* Make sure the IFF file has a LWO2 form type. */
	bytesread += read_id4(id, file);
	type = MAKE_ID(id[0],id[1],id[2],id[3]);
    if (type != ID_LWO2)
      error("Not a lightwave object (Missing LWO2 tag)");

    printf("LWO2\n");

    /* Read all Lightwave chunks. */

    while (bytesread < datasize) {

	  bytesread += read_id4(id, file);
	  bytesread += read_u4(&size, 1, file);

	  type = MAKE_ID(id[0],id[1],id[2],id[3]);

      switch (type) {
        case ID_TAGS:	read_tags(size, file); break;
        case ID_CLIP:	read_clip(size, file); break;
        case ID_ENVL:	read_envl(size, file); break;
        case ID_LAYR:	read_layr(size, file); break;
        case ID_PNTS:	read_pnts(size, file); break;
        case ID_BBOX:	read_bbox(size, file); break;
        case ID_POLS:	read_pols(size, file); break;
        case ID_PTAG:	read_ptag(size, file); break;
        case ID_VMAP:	read_vmap(size, file); break;
        case ID_SURF:	read_surf(size, file); break;
        default:
		  seek_pad(size, file);
          printf("%s [%d]\n", id, size);
          break;
      }

      bytesread += size;

    }
}

