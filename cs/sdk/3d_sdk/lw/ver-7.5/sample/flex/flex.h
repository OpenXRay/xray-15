/*
======================================================================
flex.h

Definitions and function prototypes for the FPBM reader/writer.

Ernie Wright  4 Dec 00
====================================================================== */

#ifndef FLEX_H
#define FLEX_H

/* chunk IDs */

#define CKID_(a,b,c,d) (((a)<<24)|((b)<<16)|((c)<<8)|(d))
#define ID_FORM CKID_('F','O','R','M')
#define ID_FPBM CKID_('F','P','B','M')
#define ID_FPHD CKID_('F','P','H','D')
#define ID_FLEX CKID_('F','L','E','X')
#define ID_LYHD CKID_('L','Y','H','D')
#define ID_LAYR CKID_('L','A','Y','R')

typedef struct st_FPHeader {
   short width;
   short height;
   short numLayers;
   short numFrames;
   short numBuffers;
   short flags;
   short srcLayerDepth;
   short pad2;
   float pixelAspect;
   float pixelWidth;
   float framesPerSecond;
} FPHeader;

/* file flags */

#define Source_Int    (0 << 0)
#define Source_FP     (1 << 0)
#define InterlaceFlag (1 << 1)

typedef struct st_FrameHeader {
   short numLayers;
} FrameHeader;

typedef struct st_LayerHeader {
   short flags;
   short layerType;
   short layerDepth;
   short compression;
   float blackPoint;
   float whitePoint;
   float gamma;
} LayerHeader;

/* layer flags */

#define Layer_Int       (0 << 0)
#define Layer_FP        (1 << 0)
#define Layer_Interlace (1 << 1)
#define Layer_EvenField (0 << 2)
#define Layer_OddField  (1 << 2)

/* layer types */

#define Layer_MONO       0
#define Layer_RED        1
#define Layer_GREEN      2
#define Layer_BLUE       3
#define Layer_ALPHA      4
#define Layer_OBJECT     5
#define Layer_SURFACE    6
#define Layer_COVERAGE   7
#define Layer_ZDEPTH     8
#define Layer_WDEPTH     9
#define Layer_GEOMETRY  10
#define Layer_SHADOW    11
#define Layer_SHADING   12
#define Layer_DFSHADING 13
#define Layer_SPSHADING 14
#define Layer_TEXTUREU  15
#define Layer_TEXTUREV  16
#define Layer_TEXTUREW  17
#define Layer_NORMALX   18
#define Layer_NORMALY   19
#define Layer_NORMALZ   20
#define Layer_REFLECT   21
#define Layer_MOTIONX   22
#define Layer_MOTIONY   23

/* compression */

#define NoCompression    0
#define HorizontalRLE    1
#define HorizontalDelta  2
#define VerticalRLE      3
#define VerticalDelta    4

typedef struct st_Layer {
   LayerHeader    hdr;
   int            offset;
   int            size;
   int            w, h;
} Layer;

typedef struct st_Frame {
   FrameHeader    hdr;
   Layer         *layer;
   int            offset;
} Frame;

typedef struct st_Flex {
   FPHeader       hdr;
   FILE          *fp;
   Frame         *frame;
} Flex;


/* read.c */

void   flexFree( Flex *flex );
Flex * flexReadBegin( char *filename, unsigned int *failID, int *failpos );
Layer *flexFindLayer( Flex *flex, int fnum, int layerType );
int    flexLayerSize( Flex *flex, int fnum, int layerType );
Layer *flexReadLayer( Flex *flex, int fnum, int layerType, char *dst );
void   flexReadDone( Flex *flex );
void   flexLayerToByte( Layer *layer, void *src, unsigned char *dst,
          int drowbytes, int dstep );
void   flexLayerToWord( Layer *layer, void *src, unsigned short *dst,
          int drowwords, int dstep );

/* write.c */

int   flexWriteBegin( Flex *flex, char *filename );
int   flexWriteFrame( Flex *flex, int nlayers );
int   flexWriteLayer( Flex *flex, Layer *layer, void *buf );
void  flexWriteDone( Flex *flex );
void  flexByteToLayer( Layer *layer, unsigned char *src, void *dst,
         int drowbytes, int dstep );
void  flexWordToLayer( Layer *layer, unsigned short *src, void *dst,
         int drowwords, int dstep );

/* lwio.c */

void  set_flen( int i );
int   get_flen( void );
void  revbytes( void *bp, int elsize, int elcount );
void  skipbytes( FILE *fp, int n );
void  putbytes( FILE *fp, char *buf, int n );
short getI2( FILE *fp );
void  putI2( FILE *fp, short i );
unsigned int getU4( FILE *fp );
void  putU4( FILE *fp, unsigned int i );
float getF4( FILE *fp );
void  putF4( FILE *fp, float f );

#ifdef _WIN32
  void revbytes( void *bp, int elsize, int elcount );
#else
  #define revbytes( b, s, c )
#endif

#endif
