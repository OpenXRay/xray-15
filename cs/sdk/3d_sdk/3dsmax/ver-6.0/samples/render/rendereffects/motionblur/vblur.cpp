/**********************************************************************
 *<
	FILE: vblur.cpp : Velocity Buffer Motion bluring.

	DESCRIPTION:

	CREATED BY: Dan Silva

	HISTORY: created Feb 97

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "dllmain.h"
#include <bmmlib.h>

//#define DISPPASS1		 // display buffer after pass1, for debugging 

// DS 2/24/00 added BMM_CHAN_WEIGHT and BMM_CHAN_COLOR for doing blur with transparency.
#define TR_VBLURCHAN (BMM_CHAN_Z|BMM_CHAN_COVERAGE|BMM_CHAN_VELOC|BMM_CHAN_NODE_RENDER_ID|BMM_CHAN_WEIGHT|BMM_CHAN_COLOR)
#define VBLURCHAN (BMM_CHAN_Z|BMM_CHAN_COVERAGE|BMM_CHAN_VELOC|BMM_CHAN_NODE_RENDER_ID)
		  


static void InitSqFuncTable();

//#define TESTAA
#define NOPASS1   0		// inhibit pass 1
#define NPASSES	  2     // set to 1 to inhibit pass 2;
#define DBLBUF          // ON
#define TWOPASS			// ON


//#define TOSSMAPS

// velocity threshold
#define VTHRESH .02f

// these functions are applied to each pixel that has a motion vector
typedef void (*MotVecFunc)(int x, int y, Point2 v);

// these functions are applied to each pixel along a motion vector
typedef void (*PixFunc)(int x, int y, int frac);

static MotVecFunc motVecFunc=NULL;
static PixFunc pixFunc = NULL;

static void FilterPixel( int ix, int iy, Point2 v);
static void SmearPixel( int ix, int iy, Point2 v);
static void AASmearPixel( int ix, int iy, Point2 v);
static void ErasePixel( int ix, int iy, Point2 v);
static void AccumPixel( int ix, int iy, Point2 v);
static void AccumFilterPixel( int ix, int iy, Point2 v);


static void doPixFilter(int x, int y, int frac);
static void doPixSmear(int x, int y, int frac);
static void doAAPixSmear(int x, int y, int frac);
static void doPixErase(int x, int y, int frac);
static void doPixAccum(int x, int y, int frac);
static void doPixAccumFilter(int x, int y, int frac);

	
MotVecFunc passVecFunc[] = { FilterPixel, SmearPixel, ErasePixel, AASmearPixel, AccumPixel, AccumFilterPixel };
PixFunc    passPixFunc[] = { doPixFilter, doPixSmear, doPixErase, doAAPixSmear, doPixAccum, doPixAccumFilter };

static inline void BlurPixel(int ix, int iy);


// various functions
#define F_FILT    	0	  // filter (blur) pixels along vector
#define F_SMEAR   	1	  // smear the moving pixel along the vector
#define F_ERASE   	2     // attempt to erase the moving pixel to the average bg color
#define F_AASMEAR 	3     // exchange color at begin of vector with color at end
#define F_ACCUM   	4     // accumulate smear changes before applying
#define F_ACCUMFILT 5     // accumulate filter changes before applying
#define F_NOOP  	255   // nothing

#define VBLUR_ALGORITHM 3		  // Select which algorithm to use

// First smear then filter
#if VBLUR_ALGORITHM==1  
#define PASS1FUNC F_SMEAR   // function to perform on pass 1
#define PASS2FUNC F_FILT    // function to perform on pass 2
#define RANDPASS1 0			// enumerate pixels in random order on pass 1
#define RANDPASS2 0			// enumerate pixels in random order on pass 2
#endif

// First attempt "erase" original object by averaging background colors,
// then smear object (using accumulate buffer to avoid order dependent
// artifacts ( requires 2 extra buffers!!) 
#if VBLUR_ALGORITHM==2
#define MAXCOUNT
#define DOACCUM			
#define PASS1FUNC F_ERASE   // function to perform on pass 1
#define PASS2FUNC F_ACCUM  // function to perform on pass 2
#define RANDPASS1 0			// enumerate pixels in random order on pass 1
#define RANDPASS2 1			// enumerate pixels in random order on pass 2
#endif


// First smear (using an accumulate buffer to avoid order-dependent artifacts), 
// then filter to blur the original object (and minimize remaining artifacts)
#if VBLUR_ALGORITHM==3				
#undef DBLBUF				// don't need this with the accumulator
#define DOACCUM	
#define ACCUMPASS1 		
#define PASS1FUNC F_ACCUM   // function to perform on pass 1
#define PASS2FUNC F_FILT    // function to perform on pass 2
#define RANDPASS1 0			// order doesn't matter because of accum buffer
#define RANDPASS2 1			// enumerate pixels in random order on pass 2
#endif

// First smear (using an accumulate buffer to avoid order-dependent artifacts), 
// then filter to blur the, again using accumulate buffer
#if VBLUR_ALGORITHM==4				
#undef DBLBUF				// don't need this with the accumulator
#define DOACCUM			
#define ACCUMPASS1 		
#define ACCUMPASS2 		
#define PASS1FUNC F_ACCUM   	// function to perform on pass 1
#define PASS2FUNC F_ACCUMFILT   // function to perform on pass 2
#define RANDPASS1 0			// order doesn't matter because of accum buffer
#define RANDPASS2 0			// order doesn't matter because of accum buffer
#endif


#ifdef ACCUMPASS1
#define ACCUM1 1
#else 
#define ACCUM1 0
#endif

#ifdef ACCUMPASS2
#define ACCUM2 1
#else 
#define ACCUM2 0
#endif

//#define COPYBACK 			// OFF	// copy bitmap into work buffer after pass 1;

//--------------------------------------------------------------------------------
/*-----------------------------------------------------------------------------------
 Current Algorithm: ( # 3 )

=======Pass 1: SMEAR ======== ( AccumPixel, doPixAccum) 
 
	 Smear The pixels along the velocity vectors.  Use a full image size accumulation 
	 buffer which at each pixel has:
	   struct AccumRec {
		int r,g,b,a;
		int f;		// fraction.
		int k;
		};

	 Call this once at each pixel:

	 static inline void BlurPixel(int ix, int iy) {
		Point2 v = GETV(ix,iy);
		if (fabs(v.x)>VTHRESH||fabs(v.y)>VTHRESH) 
			(*motVecFunc)(ix,iy,v);
		}
     
     For pass 1 motVecFunc is AccumPixel: This calls
	 BlurLine() on the pixel, which runs a DDA along the line and calls
	 pixFunc at each pixel. It keeps track of a fraction (frac) so that if the
	 line is fractional in length it can attenuate the fractional  end 
	 part. For this pass pixFunc is doPixAccum. 

	 Note the weight is also attenuated proportional to the length of 
	 the line.
	 
	 doPixAccum  evaluates both the current point and its mirror image about the
	 curX, curY. It adds the current "dragCol" into each of those
	 points AccumRec, first multiplying it with the frac.
	 It also adds frac into ac.f, and multiplies (1-frac) into the current ac.k.

	 When Pass1 is all done, it scans over the entire image, and computes
	 a color from each accumRec (if it has been touched) by
	      
		   c = ac.c*(1-ac.k)/ac.f + ac.k*c;

	 the result is stored in pmap, the map being blurred.


=======Pass 2: FILTER (blur)========  ( FilterPixel, doPixFilter )

	This runs a linear blur filter along the velocity vectors.  

	Image pixels are evaluated in a psuedo random order, and FilterPixel is called 
	at each pixel of the image exactly once. 

	It in turn calls BlurLine, evaluating lines in positive and negative V (velocity)
	directions, each half the length of V, with doPixFilter as the pixFunc.
	After each call to BlurLine it calls FilterLine: 
	
      BlurLine calls doPixFilter, at each point which just builds (in the 1-dimensional array pixRec)
      a list of pixels  (f, p(x,y)). Here is what a pixRec element looks like:
			struct PixRec {
			  IPoint2 p;
			  BMM_Color_64 pcol;
			  LongCol c;
			  FracInt f;
			  };

	   FilterLine- First gets from the output image all the colors for the pixels 
	   in pixRec into pixRec. Then goes along pix rec, computing a running average of some
	   number of pixels along the line, and lerp it into the accumulator for that pixel.  
	   he pixels fraction "f" is used as the lerp fraction, so higher fraction pixels are 
	   effected more.
	   The number of pixels in the running average is a heuristically determined function of the
	   number of pixels in the total line, represented by the table nfilt. The maximum is 3.
	 
---------------------------------------------------------------------*/


typedef int FracInt;

#define FRBITS		10
#define CFBITS		FRBITS
#define CF1			(1<<CFBITS)
#define FRBITS2		(FRBITS>>1)
#define FRSCALE		float((1<<FRBITS))
#define FR1			(1<<FRBITS)
#define FR2			(2<<FRBITS)
#define FRHALF		(FR1>>1)
#define FRQ			(FR1>>2)
#define FR3Q		(FR1-FRQ)
#define FRMASK		(FR1-1)
#define FRHIMASK	(~FRMASK)


static inline int Frac(float f) { 
	return FracInt(f*FRSCALE + 0.5f); 
	}

static inline FracInt FrMul(FracInt a, FracInt b) { 
	return 
	  (((a&FRMASK) * (b&FRMASK)) >> FRBITS) + (a>>FRBITS)*b + (b>>FRBITS)*(a&FRMASK) ; 
	}

static inline FracInt FrDiv(FracInt num, FracInt den) { 
//	return Frac(float(num)/float(den));
	// Could do this better, but at least this keeps it from overflowing
	return (num<<(FRBITS/2))/(den>>(FRBITS/2));
	}

// Pixel Radius
#define PIXRBITS (FRBITS-1)
#define PIXRAD (1<<PIXRBITS)

// HALF Initial line thickness
#define INITHTHICK (FR1>>1)  

//--------------------------------------------------------------------------------
// Masks for use in the random order loop
//--------------------------------------------------------------------------------
static int randmasks[] = {
	0,			// 0
	0,			// 1
	0x0003,		// 2
	0x0006,		// 4
	0x000C,		// 5
	0x0014,		// 6
	0x0030,		// 7
	0x0060,		// 8
	0x00B8,		// 9
	0x0110,		// 10
	0x0240,		// 11
	0x0500,		// 12
	0x0CA0,		// 13
	0x1B00,		// 14
	0x3500,		// 15
	0x6000,		// 16
	0xB400,		// 17
	0x00012000,	// 18
	0x00020400,	// 19
	0x00072000,	// 20
	0x00090000,	// 21
	0x00140000,	// 22
	0x00300000,	// 23
	0x00D80000,	// 24
	0x01200000,	// 25
	0x03880000,	// 26
	0x07200000,	// 27
	0x09000000,	// 28
	0x14000000,	// 29
	0x32800000,	// 30
	0x48000000,	// 31
	0xA3000000	// 32
	};	

static int bitwidth (int N)	{
	int width = 0;	
	while (N != 0) {   N >>= 1;   width++;	}	
	return (width);	
	}

//----------------------------------------------------------------------
// Blend color c2 into color c1: return (1-f)*c1 + f*c2
//---------------------------------------------------------------------
static inline BMM_Color_64 BLEND(int frac, BMM_Color_64 c1, BMM_Color_64 c2) {
	int ufrac = (FR1-frac);
	BMM_Color_64 c;
	c.r = (ufrac*c1.r + frac*c2.r) >> FRBITS;
	c.g = (ufrac*c1.g + frac*c2.g) >> FRBITS;
	c.b = (ufrac*c1.b + frac*c2.b) >> FRBITS;
	c.a = (ufrac*c1.a + frac*c2.a) >> FRBITS;
	return c;
	}


//--------------------------------------------------------------------------------

class LongCol {
	public:
	ULONG r,g,b,a;
	LongCol&	operator=(const BMM_Color_64& c) { 
		r=c.r; g = c.g; b = c.b; a = c.a; 
		return *this; 
		}
	LongCol& operator+=(const LongCol& c) {
		r += c.r;	g += c.g;	b += c.b; a += c.a;
		return *this;
		}
	LongCol& operator-=(const LongCol& c) {
		r -= c.r;	g -= c.g;	b -= c.b; a -= c.a;
		return *this;
		}
	operator BMM_Color_64() { 
		BMM_Color_64 c; 
		c.r = (USHORT)r; 
		c.b = (USHORT)b; 
		c.g = (USHORT)g; 
		c.a = (USHORT)a; 
		return c; 
		}
	};

struct PixRec {
	IPoint2 p;
	BMM_Color_64 pcol;
	LongCol c;
	FracInt f;
	};

struct AccumRec {
	int r,g,b,a;
	int f;
	int k;
	};
	 
static BMM_Color_64 REDCOL = {0xffff,0,0,0xffff};
static Point2 pzero(0.0f,0.0f);
static Bitmap *thebm,*workbm;
static Point2 curVeloc;
static UBYTE *covBuf;
static Point2 *velBuf;
static float *velocMult;
static float imbDur;
static BOOL doTransp;
static UWORD *nodeIdBuf;
static Color24 *colorBuf;
//static Color24 *transpBuf;
static FracInt curVMag;
static float *zbuf, pixZ;
static PixRec *pixRec=NULL;
static AccumRec *paccum=NULL;
static int bmw, bmh;
static float fbmw, fbmh;
static UWORD curObj;
static int curX, curY, nField, fieldH;
static BOOL fieldRender;
static int accum_r,accum_g,accum_b,accum_a, nacc;
static BOOL passNum;
static BMM_Color_64 dragCol;
static BOOL randOrder[2]= { RANDPASS1,  RANDPASS2};
static BOOL doAccum[2]=   { ACCUM1, ACCUM2};
static int funcNum[2] =   { PASS1FUNC,  PASS2FUNC};
static int fdenom,coverage;
static GBufReader* gbRdr1 = NULL;
static GBufReader* gbRdr2 = NULL;
static BOOL inGBLayer = FALSE;
static GBufData gbData,gbData2;


static inline pixOffset(int ix, int iy) {
	if (!fieldRender) return (bmw*iy+ix);	
	else {
		int n = bmw*((iy<<1)+nField)+ix;
		return n;
		}
	}

#define GETACC(ix,iy) paccum[pixOffset(ix,iy)] 

//----------------------------------------------------------------------
//-------PIXMAP--------------------------------------------------------
//----------------------------------------------------------------------
class PixMap{
	public:
		virtual BMM_Color_64 GetColor(int ix, int iy)=0;
		virtual void SetColor(int ix, int iy, BMM_Color_64 c)=0;
	};


class DefPixMap: public PixMap {
	public:
	BMM_Color_64 GetColor(int ix, int iy) {
		BMM_Color_64 col;
		if (fieldRender) iy = (iy<<1)+nField;
	    thebm->GetLinearPixels( ix, iy, 1, &col);
		return col;
		}
	void SetColor(int ix, int iy, BMM_Color_64 c) {
		if (fieldRender) iy = (iy<<1)+nField;
		thebm->PutPixels(ix, iy, 1, &c);
		}
	};

class PixMap16: public PixMap {
	public:
	BMM_Color_48 *map;
	UWORD *alpha;
	BMM_Color_64 GetColor(int ix, int iy) {
		BMM_Color_64 col;
		int poffs = pixOffset(ix,iy);
		BMM_Color_48 c = map[poffs];
		col.r = c.r; col.g = c.g; col.b = c.b;
		col.a = alpha[poffs];
		return col;
		}
	void SetColor(int ix, int iy, BMM_Color_64 c) {
		BMM_Color_48 col;
		col.r = c.r; col.g = c.g; col.b = c.b;
		int poffs = pixOffset(ix,iy);
		map[poffs] = col;
		alpha[poffs] = c.a;
		}
	};


class PixMap8: public PixMap {
	public:
	BMM_Color_24 *map;
	BYTE *alpha;
	BMM_Color_64 GetColor(int ix, int iy) {
		BMM_Color_64 col;
		int poffs = pixOffset(ix,iy);
		BMM_Color_24 c = map[poffs];
		col.r = c.r<<8; col.g = c.g<<8;	col.b = c.b<<8;
		col.a = alpha[poffs]<<8;
		return col;
		}
	void SetColor(int ix, int iy, BMM_Color_64 c) {
		BMM_Color_24 col;
		col.r = c.r>>8;	col.g = c.g>>8;	col.b = c.b>>8;
		int poffs = pixOffset(ix,iy);
		map[poffs] = col;
		alpha[poffs] = c.a>>8;
		}
	};

static PixMap *pmap;  	// Accesses the map being blurred
static DefPixMap defPixMap;
static PixMap16 pixMap16,workMap;
static PixMap8 pixMap8;

static void CopyBuffer(Bitmap *bmto, Bitmap *bmfrom) {
	PixelBuf l64(bmw);
	for (int iy=0; iy<bmh; iy++) {
		BMM_Color_64 *p64=l64.Ptr();
		bmfrom->GetPixels(0, iy, bmw, p64); 
		bmto->PutPixels(0,iy,bmw,p64);		
		}
	}

static inline void drawPixel(int x, int y, FracInt frac) {
	BMM_Color_64 c = pmap->GetColor(x,y);
	pmap->SetColor(x,y,BLEND(frac,c,REDCOL));
	}

static int passTitle[3] = {IDS_DS_APPLYVBLUR, IDS_DS_APPLYVBLUR2 };

ULONG IMBOpsImp::ChannelsRequired(ULONG flags) { 
	return 	flags&IMB_TRANSP?TR_VBLURCHAN:VBLURCHAN; 
	}

//--------------------------------------------------------------------------------
// Apply Velocity Motion Blur
//--------------------------------------------------------------------------------
int IMBOpsImp::ApplyMotionBlur(Bitmap *bm, CheckAbortCallback *progCallback, float duration, ULONG flags, Bitmap *extraBM) {
	Rect r;
	int res = 1;
	int maxodd=0;
	doTransp = (flags&IMB_TRANSP)?1:0;
	
	imbDur = duration;

	ULONG ctype;
	ULONG chan = bm->ChannelsPresent();
	if ((chan&VBLURCHAN)!=VBLURCHAN) return 0;
	covBuf = (UBYTE*)bm->GetChannel(BMM_CHAN_COVERAGE,ctype);
	if (covBuf==NULL) return 0;
	velBuf = (Point2*)bm->GetChannel(BMM_CHAN_VELOC,ctype);
	if (velBuf==NULL) return 0;
	zbuf = (float*)bm->GetChannel(BMM_CHAN_Z,ctype);
	if (zbuf==NULL) return 0;
	nodeIdBuf = (UWORD *)bm->GetChannel(BMM_CHAN_NODE_RENDER_ID,ctype);
	if (nodeIdBuf==NULL) return 0;

	if ((bm->ChannelsPresent()&(BMM_CHAN_WEIGHT|BMM_CHAN_COLOR))==0)
		doTransp = 0;
	if (doTransp) {
		colorBuf = (Color24 *)bm->GetChannel(BMM_CHAN_COLOR,ctype);
		if (colorBuf==NULL) doTransp = 0;
		}


	GBuffer *gbuf = bm->GetGBuffer();
	assert(gbuf);

	Tab<float> &imbmult = gbuf->ImageBlurMultiplierTab();
	velocMult = imbmult.Addr(0);
	assert(velocMult);
	
 	gbRdr1 = gbuf->CreateReader();
	gbRdr2 = gbuf->CreateReader();

#ifdef AALINE
	InitSqFuncTable();
#endif
	
	bmw = bm->Width();
	fieldH = bmh = bm->Height();
	fbmh = (float)bmh;
	fbmw = (float)bmw;
	int npix = bmw*bmh;

	r.top = r.left = 0;
	r.bottom = bmh;
	r.right = bmw;
	
    RenderInfo* ri = bm->GetRenderInfo();
	fieldRender = FALSE;
	if (ri) {
		if (ri->fieldRender) {
			fieldRender = TRUE;
			fieldH = bmh/2;
			}
		}
	thebm = extraBM? extraBM : bm;
	int stype;
	int atype;
	void *ps = thebm->GetStoragePtr(&stype);
	void *pa = thebm->GetAlphaPtr(&atype);

	if (stype == BMM_TRUE_48 && atype==BMM_GRAY_16 && pa ) {
		pixMap16.map =   (BMM_Color_48 *)ps;
		pixMap16.alpha = (UWORD *)pa;
		pmap = &pixMap16;
		}
	else 
	if (stype == BMM_TRUE_24 && atype==BMM_GRAY_8 && pa ) {
		pixMap8.map =   (BMM_Color_24 *)ps;
		pixMap8.alpha = (BYTE *)pa;
		pmap = &pixMap8;
		}
	else pmap = &defPixMap;

#ifdef TESTAA
	{
	int xc = Frac(160.0f);
	int yc = Frac(100.0f);
	float rad = 80.0f;
	float rad0 = 40.0f;
	float a = DegToRad(-3.0f);
	int x0 = Frac(cos(a)*rad0)+xc;
	int y0 = Frac(sin(a)*rad0)+yc;
	int x1 = Frac(cos(a)*rad)+xc;
	int y1 = Frac(sin(a)*rad)+yc;
	pixFunc = drawPixel;
	BlurLine( x0, y0, x1, y1, 0);
#if 0
	for (int i=0; i<120; i++) {
		a = DegToRad(3.0f*float(i));
		x0 = Frac(cos(a)*rad0)+xc;
		y0 = Frac(sin(a)*rad0)+yc;
		x1 = Frac(cos(a)*rad)+xc;
		y1 = Frac(sin(a)*rad)+yc;
		BlurLine( x0, y0, x1, y1, 0);
		}
#endif
	bm->RefreshWindow(&r);
	return 1;
	}
#endif

	pixRec = NULL;
	if (funcNum[0]==F_FILT||funcNum[1]==F_FILT
		||funcNum[0]==F_ACCUMFILT||funcNum[1]==F_ACCUMFILT
		) {
		pixRec = new PixRec[2048];
#ifdef TOSSMAPS
		if (!pixRec) {
			GetCOREInterface()->FreeSceneBitmaps();
			pixRec = new PixRec[2048];
			}
#endif
		if (!pixRec) {
			TSTR buf = GetString(IDS_DS_NOMEMFOR_VELOCBUF);
			GetCOREInterface()->Log()->LogEntry(SYSLOG_ERROR, TRUE, GetString(IDS_RB_RENDERERROR), _T(" %s\n"), buf);
			res = 0;
			goto bail;
			}
		}

#ifdef DBLBUF
	BitmapInfo bi;	
	bi.SetWidth(bmw);
	bi.SetHeight(bmh);
	bi.SetType(BMM_TRUE_64);
	bi.SetFlags(MAP_HAS_ALPHA);
	workbm = TheManager->Create(&bi);
	if (!workbm) {
		TSTR buf = GetString(IDS_DS_NOMEMFOR_VELOCBUF);
		GetCOREInterface()->Log()->LogEntry(SYSLOG_ERROR, TRUE, GetString(IDS_RB_RENDERERROR), _T(" %s\n"), buf);
		res = 0;
		goto bail;
		}
	ps = workbm->GetStoragePtr(&stype);
	pa = workbm->GetAlphaPtr(&atype);

	if (stype == BMM_TRUE_48 && atype==BMM_GRAY_16 && pa ) {
		workMap.map =   (BMM_Color_48 *)ps;
		workMap.alpha = (UWORD *)pa;
		}
	else {
		res = 0;
		goto bail;
		}

	CopyBuffer(workbm,bm);  // workbm <- bm
#endif	


#ifdef DOACCUM
	paccum = new AccumRec[npix];

#ifdef TOSSMAPS
	if (paccum==NULL) {
		GetCOREInterface()->FreeSceneBitmaps();
		paccum = new AccumRec[npix];
		}
#endif

	if (paccum==NULL) {
		TSTR buf = GetString(IDS_DS_NOMEMFOR_VELOCBUF);
		GetCOREInterface()->Log()->LogEntry(SYSLOG_ERROR, TRUE, GetString(IDS_RB_RENDERERROR), _T(" %s\n"), buf);
		goto bail;
		}

#endif

	for (passNum=0; passNum<NPASSES; passNum++) {
		if (doAccum[passNum]) {
			for (int i=0; i<npix; i++) {
				AccumRec &ac = paccum[i];
				ac.r = ac.b = ac.g = ac.a = 0;
				ac.f = 0;
				ac.k = FR1;
				}
			}
		progCallback->SetTitle(GetString(passTitle[passNum]));
		if (passNum==0&&NOPASS1) continue;
		// plug in functions:
		int fnum = funcNum[passNum];
		if (fnum ==F_NOOP) 
			continue;
		pixFunc = passPixFunc[funcNum[passNum]];
		motVecFunc = passVecFunc[funcNum[passNum]];
		for (nField=0; nField< (fieldRender?2:1); nField++) {
			if (randOrder[passNum]) {
				// Enumerate Pixels of image in psuedo random order.
				// Graphics Gems I, p.221 ff

				// There is a time penalty for random order evaluation:
				// it is mostly due to the non-sequential access of memory
				// not the code to generate the sequence.

				int pixels, lastnum;	// number of pixels,  
				int regwidth;		// "width" of sequence generator 
				register long mask;	// mask to XOR with to create sequence 
				register unsigned long element; // one element of random sequence 
				register int iy, ix; // iy and ix numbers for a pixel 
				register int numPix=0;

				// Find smallest register which produces enough pixel numbers 
				pixels = fieldH * bmw; // compute number of pixels to dissolve 
				lastnum = pixels-1;	// find last element (they go 0..lastnum) 
				regwidth = bitwidth ((unsigned int)lastnum); // how wide must the register be? 
				mask = randmasks [regwidth];	// which mask is for that width?

				// Now cycle through all sequence elements. 
				element = 1;	// 1st element (could be any nonzero) 
				do {
					iy = element / bmw;		// how many iys down is this pixel? 
					ix = element % bmw;		// and how many ixs across? 
					if (iy < fieldH) {		// is this seq element in the array? 
						BlurPixel (ix, iy);		// yes: copy the (x,y)'th pixel 
						if (((++numPix)&0x3fff)==0)
							if 	(progCallback->Progress(numPix,pixels)) goto abort;
						}
				    // Compute the next sequence element 
					if (element & 1)		
						element = (element >>1)^mask;	// yes: shift value,  XOR in mask 
					else element = (element >>1);	// no: just shift the value 
					} while (element != 1);		// loop until we return  to beginning
				BlurPixel(0, 0);		// kludge: the loop doesn't produce (0,0) 
				}
			else {
				for (int iy = 0; iy<fieldH; iy++) {
					if ((iy&31)==0)
						if (progCallback->Progress(iy,fieldH)) goto abort;
					for (int ix=0; ix<bmw; ix++) { 
						BlurPixel(ix,iy);
						}
					}
				}
#ifdef DOACCUM
			if (doAccum[passNum]) {
				if (passNum==0) {
					for (int iy=0; iy<fieldH; iy++) {
						for (int ix=0; ix<bmw; ix++) {
							AccumRec& ac = GETACC(ix,iy);
							FracInt f = ac.f;
							if (ix==180&&iy==120)
								passNum = 2*passNum;
							if (f!=0) {
								BMM_Color_64 c;
								c = pmap->GetColor(ix,iy);
								FracInt g = FR1-ac.k; 
								c.r = ac.r*g/f + FrMul(c.r,ac.k);
								c.g = ac.g*g/f + FrMul(c.g,ac.k);
								c.b = ac.b*g/f + FrMul(c.b,ac.k);
								c.a = ac.a*g/f + FrMul(c.a,ac.k);
								pmap->SetColor(ix,iy,c);
								}
							}
						}
					}
				else {
					for (int iy=0; iy<fieldH; iy++) {
						for (int ix=0; ix<bmw; ix++) {
							AccumRec& ac = GETACC(ix,iy);
							FracInt f = ac.f;
							if (f!=0) {
								BMM_Color_64 c;
								c.r = ac.r/f;
								c.g = ac.g/f;
								c.b = ac.b/f;
								c.a = ac.a/f;
								pmap->SetColor(ix,iy,c);
								}
							}
						}
					}							
				}			
#endif
			}// for (nField=0; nField< (fieldRender?2:1); nField++) 

#ifdef TWOPASS
		if (passNum==0) {

#ifdef DISPPASS1
			bm->RefreshWindow();
			MessageBox(NULL, _T("hi"), _T("After Pass 1"), MB_OK|MB_ICONEXCLAMATION);
#endif

#ifdef COPYBACK
			CopyBuffer(workbm, bm);  // workbm <- bm
#endif
			}
#endif
		} //for (passNum=0; passNum<3; passNum++) 

	abort:
	if( extraBM==NULL )
		bm->RefreshWindow(&r);

	bail:
#ifdef DBLBUF
	if (workbm) {
		workbm->DeleteThis();
		workbm = NULL;
		}
#endif
	if (pixRec) {
		delete [] pixRec;
		pixRec = NULL;
		}
	if (paccum) {delete [] paccum;
		paccum = NULL;
		}
	if (gbRdr1) {
	 	gbuf->DestroyReader(gbRdr1);
		gbRdr1 = NULL;
		}
	if (gbRdr2) {
	 	gbuf->DestroyReader(gbRdr2);
		gbRdr2 = NULL;
		}
	return res;
	}



int randDith() {
	return (rand()&FRHALF)-FRQ;
	}
#define SGN(x) (x>=0?1:-1)


//---BlurLine-------------------------------------------------
//
// coordinates are defined with FRBITS fractional bits:
//
//---------------------------------------------------------
void BlurLine(FracInt x1, FracInt y1, FracInt x2, FracInt y2, FracInt dhThick) {
    int d,x,y;
    FracInt ax, ay, sx, sy, dx, dy;
	FracInt adx,ady,dinc;

	dx = x2-x1;
	dy = y2-y1;
	
	adx = ABS(dx);
	ady = ABS(dy);
    ax = adx<<1;  sx = SGN(dx);
    ay = ady<<1;  sy = SGN(dy);
	x = x1>>FRBITS;
	y = y1>>FRBITS;

//	int frac;  
    if (ax>ay) {		// x dominant 
		dinc = ay-ax; // diagonal increment
		d = ay-adx;
		adx += FR1;
//		int n = (adx>>FRBITS); 
#ifdef MAXCOUNT
		maxcount = n;
#endif
//		frac = (1<<(FRBITS))/n;
		for (;;) {
			if (adx<FR1) {
				(*pixFunc)( x, y, adx /* (frac*adx)>>FRBITS*/);
				return;
				}
			(*pixFunc)( x, y, FR1 /*frac*/);
			adx -= FR1;
		    if (d>=0 ) { 
				y += sy; d += dinc;  
				}
			else  
				d += ay;
		    x += sx;
			}
	    }
    else {			//y dominant 
	   	dinc = ax-ay; // diagonal increment
		d = ax-ady;
		ady += FR1;
//		int n = (ady>>FRBITS); 
#ifdef MAXCOUNT
		maxcount = n;
#endif
//		frac = (1<<(FRBITS))/n;
		for (;;) {
			if (ady<FR1) {
				(*pixFunc)( x, y, ady /*(frac*ady)>>FRBITS*/);
				return;
				}
			(*pixFunc)( x, y, FR1 /*frac*/);
			ady -= FR1;
		    if (d>=0 ) { 
				x += sx; d += dinc;  
				}
			else  
				d += ax;
		    y += sy;
			}
	    }
	}


//---------------------------------------------------------------------------

static inline Point2 Getv(int ix,int iy) {
	int k;
	Point2 v;
	if (inGBLayer) {
		k = gbData.rend_id;
		v = (k==0xffff)? gbData.veloc : gbData.veloc*velocMult[k];
		}
	else {
		int n;
		k = nodeIdBuf[n = pixOffset(ix,iy)];
		v = (k==0xffff)? velBuf[n] : velBuf[n]*velocMult[k];
		}
	return v*imbDur;
	} 


Point2 ClipVelocity(int ix, int iy) {
	Point2 v = Getv(ix,iy);
	Point2 p;

	if (v.x==0.0f&&v.y==0.0f) return v;

	v *= 0.5f;
	p.x = float(ix);
	p.y = float(iy);
	float x,y;
	
	x = p.x+v.x;
	if (x>fbmw) 
		v *= (float)fabs((fbmw-p.x)/v.x);
	else if (x<0.0f) 
		v *= (float)fabs(p.x/v.x);
	
	x = p.x - v.x;
	if (x>fbmw) 
		v *= (float)fabs((fbmw-p.x)/v.x);
	else if (x<0.0f) 
		v *= (float)fabs(p.x/v.x);

	y = p.y+v.y;
	if (y>fbmh) 
		v *= (float)fabs((fbmh-p.y)/v.y);
	else if (y<0.0f)
		v *=  (float)fabs(p.y/v.y);

	y = p.y-v.y;
	if (y>fbmh) 
		v *=(float)fabs((fbmh-p.y)/v.y);
	else if (y<0.0f) 
		v *= (float)fabs(p.y/v.y);

	return 2.0f*v;
	}

#define GETV(ix,iy) ClipVelocity(ix,iy)
#define GETOBNUM(ix,iy) (inGBLayer?gbData.rend_id:nodeIdBuf[pixOffset(ix,iy)]) 
#define GETZ(ix,iy) (inGBLayer?gbData.z:zbuf[pixOffset(ix,iy)]) 


static inline int GETCOV(int ix, int iy) {
	int cv;
	cv =  covBuf[pixOffset(ix,iy)];
	//return (GETOBNUM(ix,iy)==65535)?255-cv:cv; 
	return (cv==0)?255:cv; // for bluring background 
	}

static inline FracInt VelMag(Point2 v) {
	return Frac((float)(fabs(v.x)+fabs(v.y)));
	}


//-----------------------------------------------------------------------------

// Incorporate transparency DS 2/25/00
static inline void BlurPixel(int ix, int iy) {
	inGBLayer = FALSE;
	Point2 v = GETV(ix,iy);
	if (fabs(v.x)>VTHRESH||fabs(v.y)>VTHRESH) 
		(*motVecFunc)(ix,iy,v);
	if(doTransp) {
#if 1	
		// > 10/11/02 - 12:31am --MQM-- bug #420449
		// ooops...if we're field rendering, the "iy" passed in is actually 
		// in the range of 0..height/2.  we need to adjust this back up in order 
		// to access the correct line in the g-buffer.  GETV(), pixOffset(), etc 
		// compensate for this, just not this one.
		int newiy = fieldRender ? ( (iy<<1) + nField ) : iy;
		if (gbRdr1->StartPixel(ix,newiy)>0)
#else
		if (gbRdr1->StartPixel(ix,iy)>0)
#endif
			{
			inGBLayer = TRUE;
			while (gbRdr1->StartNextLayer()) {
				if (gbRdr1->ReadAllData(&gbData)) {
					v = GETV(ix,iy);
					if (fabs(v.x)>VTHRESH||fabs(v.y)>VTHRESH) 
						(*motVecFunc)(ix,iy,v);
					}
				}
			}
		}
	}


static Color24 white = {255,255,255};

// Returns visibility: 0 = invisibity, 255 is totally visible
static int Visibility(int ix, int iy, Color24 &w) {
	int poffs;
	if (nodeIdBuf[poffs= pixOffset(ix,iy)]==curObj) { 
		w = white;
		return 255; // can't self occlude
		}
	if (pixZ>=zbuf[poffs]) {
		w = white;
		return 255;  // not occluded
		}
#if 1
	// > 10/11/02 - 12:31am --MQM-- bug #420449
	// ooops...if we're field rendering, the "iy" passed in is actually 
	// in the range of 0..height/2.  we need to adjust this back up in order 
	// to access the correct line in the g-buffer.  GETV(), pixOffset(), etc 
	// compensate for this, just not this one.
	int newiy = fieldRender ? ( (iy<<1) + nField ) : iy;
	if (gbRdr2->StartPixel(ix,newiy)<=0) {
#else
	if (gbRdr2->StartPixel(ix,iy)<=0) {
#endif
		w = white;
		return 255-GETCOV(ix,iy); 
		}
	BOOL gotLayers = FALSE;
	while (gbRdr2->StartNextLayer()) {
		gotLayers = TRUE;
		if (gbRdr2->ReadAllData(&gbData2)) {
			if ((gbData2.z<pixZ)||(gbData2.rend_id==curObj)) 
				break;
			}
			}

	if (gotLayers) {
		// The coverage of the background pixel is already factored into the weight, and need to remove it.
		if(gbData2.coverage==0)
		{
				w.r = 0.0f;
				w.g = 0.0f;
				w.b = 0.0f;
				return 0.0f;
		}
		else
		{
			w.r = 255*gbData2.weight.r/gbData2.coverage;
			w.g = 255*gbData2.weight.g/gbData2.coverage;
			w.b = 255*gbData2.weight.b/gbData2.coverage;
			return (w.r+w.g+w.b)/3;
		}
	}
	else {
		int vis = 255-GETCOV(ix,iy);
		w.r = w.g = w.b = vis;
		return vis; 
	}
	return 0;
	}

 
static inline int GBGetCov(int ix, int iy) {
	int cv;
	if (doTransp) 
		cv = inGBLayer?gbData.coverage:covBuf[pixOffset(ix,iy)];
	else
		cv =  covBuf[pixOffset(ix,iy)];
	return (cv==0)?255:cv; // for bluring background 
	}

static inline void  SetCurZ(int ix, int iy) {
	if (inGBLayer) {
		pixZ = gbData.z;
		curObj = gbData.rend_id;
		}
	else 
		{
		int po = pixOffset(ix,iy);
		pixZ = zbuf[po];
		curObj = nodeIdBuf[po];
		}
	}



// Returns 0 if occluded
static inline int TestZDepth(int ix, int iy) {
	int poffs;
	if (nodeIdBuf[poffs= pixOffset(ix,iy)]!=curObj) {
		if (pixZ<zbuf[poffs]) {
			if (GETCOV(ix,iy)==255) 
				return 0; // if moving pixel is behind object, skip it.
			}
		}
	return 1;
	}

//-----------------------------------------------------------------------------------
// ACCUM  (PASS 1)
//-----------------------------------------------------------------------------------
// (motVecFunc) called once for each layer at a pixel

static int count;
static int maxn;
static void AccumPixel( int ix, int iy, Point2 v) {
	curX = ix;
	curY = iy;
	int dx = Frac(0.5f*v.x); 
	int dy = Frac(0.5f*v.y);
	SetCurZ(ix,iy);
	if (inGBLayer) {
		dragCol.r = gbData.color.r<<8;
		dragCol.g = gbData.color.g<<8;
		dragCol.b = gbData.color.b<<8;
		}
	else {
		dragCol = pmap->GetColor(ix,iy);
		}

	int ixf = ix<<FRBITS;
	int iyf = iy<<FRBITS;
	int adx = ABS(dx);
	int ady = ABS(dy); 
	maxn = (adx>ady)?adx:ady;
	count = maxn;
	fdenom = maxn+FR1;     // distribute less per pixel on longer vectors
	//if (fdenom<FR1) fdenom = FR1;
	int cov = GBGetCov(ix,iy); 
	fdenom = (fdenom*255)/cov;
	BlurLine(ixf, iyf, ixf+dx, iyf+dy, 0);  // get both directions in one pass
	}

static inline void doPixAccum1(int x, int y, FracInt frac) {
	if (x>=0&&x<bmw&&y>=0&&y<fieldH) {
		if(doTransp) {
			Color24 wt;
			int vis = (Visibility(x,y,wt));  // is point (x,y,curZ) partially occluded?
			if (!vis) return;
			AccumRec &ac = GETACC(x,y);
			ac.r += FrMul(dragCol.r,frac*wt.r/255);
			ac.g += FrMul(dragCol.g,frac*wt.g/255);
			ac.b += FrMul(dragCol.b,frac*wt.b/255);

			frac = (vis*frac)/255;
			ac.a += FrMul(dragCol.a,frac);
			ac.k  = FrMul(ac.k,FR1-frac);
			ac.f += frac;
			}
		else {
			if (!TestZDepth(x,y))
				return;	
			AccumRec &ac = GETACC(x,y);
			ac.r += FrMul(dragCol.r,frac);
			ac.g += FrMul(dragCol.g,frac);
			ac.b += FrMul(dragCol.b,frac);
			ac.a += FrMul(dragCol.a,frac);
			ac.k  = FrMul(ac.k,FR1-frac);
			ac.f += frac;
			}
		}
	}

// (pixFunc): called along vector at each point by BlurLine
static void doPixAccum(int x, int y, int frac) {
	if (x==curX&&y==curY) return;
	frac = FrDiv(frac,fdenom);
//	frac  = frac*count/maxn;
//	count--;
	if (frac) {
		doPixAccum1(x,y,frac);
		doPixAccum1(2*curX-x,2*curY-y,frac);
		}
	}

//-----------------------------------------------------------------------------------
// FILTER   ( PASS 2 )
//-----------------------------------------------------------------------------------
//static int nfilt[30] = {
//	0,0,1,1,1,2,2,2,2,2,
//	2,2,2,2,3,3,3,3,3,3,
//	3,3,3,3,3,3,3,3,3,3
//	};

static void FilterLine(int nacc) {
	int n,nsum;
	int f;
	LongCol c,ac;
	BMM_Color_64 csum;

	if (nacc<2) return;
//	if (nacc<30) n = nfilt[nacc];
//	else n = 3;
	
	// DS 2/29/00 This filters less and makes the falloff more gradual.
	if (nacc<14) n = 1;  
	else n = 2;

	register PixRec *pbuf = pixRec;
	

	int fmult = FR1;
	int fstep = (FR1-(FR1>>4))/nacc;

	// Filter pixels less as get farther away from source point
	for (int i=0; i<nacc; i++,pbuf++) {
		c = pbuf->pcol = pmap->GetColor(pbuf->p.x,pbuf->p.y);
		f = (pbuf->f*fmult)>>FRBITS;
		pbuf->f = f;
		pbuf->c.r = f*c.r;
		pbuf->c.g = f*c.g;
		pbuf->c.b = f*c.b;
		pbuf->c.a = f*c.a;
		fmult -= fstep;
		}

	ac.r = ac.g = ac.b = ac.a = 0;
	nsum =0;
	pbuf = pixRec;
	for (int j=0; j<n; j++, pbuf++) {
		c = pbuf->c;
	   	ac += pbuf->c;
		nsum += pbuf->f;
		}

	for (i=0; i<nacc; i++) {
		if (i>n) { // remove oldest one
			pbuf = &pixRec[i-n-1];
			c = pbuf->c;
		  	ac -= pbuf->c;
			nsum -= pbuf->f;
			}
		if (i+n<nacc) { // add in new one
			pbuf = &pixRec[i+n];
			c = pbuf->c;
		  	ac += pbuf->c;
			nsum += pbuf->f;
			}
		if (nsum!=0) {
			csum.r = USHORT(ac.r/nsum);
			csum.g = USHORT(ac.g/nsum);
			csum.b = USHORT(ac.b/nsum);
			csum.a = USHORT(ac.a/nsum);
			}
		else 
			csum.r = csum.g = csum.b = csum.a = 0;

		PixRec &pr = pixRec[i];
		if (pr.f!=FR1) 
			csum = BLEND(pr.f,pr.pcol,csum);		
//		if (pr.p.x == 206 && pr.p.y == 46) {
//			DebugPrint("f = %d ,pcol = (%d,%d,%d) col = %d,%d,%d,%d  (%d,%d,%d,%d)\n",
//				pr.f, pr.pcol.r>>8, pr.pcol.g>>8, pr.pcol.b>>8,
//				csum.r, csum.g, csum.b, csum.a,
//				csum.r>>8, csum.g>>8, csum.b>>8, csum.a>>8);
//			whoa();
//			}
		pmap->SetColor(pr.p.x, pr.p.y, csum);		
		}
	}


#define DITHMASK 0xFFF

// (motVecFunc): called once for each layer at each pixel
static void FilterPixel( int ix, int iy, Point2 v) {
	curX = ix;
	curY = iy;
	int dx = Frac(0.5f*v.x);// + rand()&DITHMASK;
	int dy = Frac(0.5f*v.y);// + rand()&DITHMASK;

	SetCurZ(ix,iy);   // set pixZ and curObj
	FracInt ixf = (ix<<FRBITS);
	FracInt iyf = (iy<<FRBITS);

	nacc = 0;
	BlurLine(ixf, iyf, ixf+dx, iyf+dy, 0);
	FilterLine(nacc);

	nacc = 0;
	BlurLine(ixf, iyf, ixf-dx, iyf-dy, 0);
	FilterLine(nacc);
	}


// (pixFunc): called along vector at each point by BlurLine
static void doPixFilter(int x, int y, int frac) {
	if (x<0||x>=bmw||y<0||y>=fieldH) return;
	if (doTransp) {
		Color24 wt;
		int vis = (Visibility(x,y,wt));
		frac = frac*vis/255;
		}
	else {
		int poffs;
		if (nodeIdBuf[poffs= pixOffset(x,y)]!=curObj) {
			if (pixZ<zbuf[poffs]) 
				return; // if moving pixel is behind object, skip it.
			}
		}
	if (frac==0) 
		return;
	pixRec[nacc].f = frac;
	pixRec[nacc++].p = IPoint2(x,y);   // just accumulate a list of pixels
	}


//**************************************************************************************************//
//**************************************************************************************************//
//*************** UNUSED CODE FOLLOWS ************************************************************* //
//**************************************************************************************************//
//**************************************************************************************************//


#if 1

static void AccumFilterPixel( int ix, int iy, Point2 v) {}
static void doPixAccumFilter(int x, int y, int frac) {}
static void ErasePixel( int ix, int iy, Point2 v) {}
static void doPixErase(int x, int y, int frac) {}
static void SmearPixel( int ix, int iy, Point2 v) {}
static void doPixSmear(int x, int y, int frac) {}
static void AASmearPixel( int ix, int iy, Point2 v) {}
static inline void doAAPixSmear(int x, int y, FracInt frac) {}

#else

//----------------------------------------------------------
// ACCUM FILTER
//----------------------------------------------------------
static void AccumFilterLine(int nacc) {
	int cr, cg, cb,ca;
	int n,nsum;
	LongCol c;

	if (nacc<2) return;
	n = nacc/5;
	register PixRec *pbuf = pixRec;
	for (int i=0; i<nacc; i++,pbuf++) {
		pbuf->c = pmap->GetColor(pbuf->p.x,pbuf->p.y);
		}
	cr = cg = cb = ca = 0;
	nsum =0;
	pbuf = pixRec;
	for (int j=0; j<n; j++, pbuf++) {
		c = pbuf->c;
	   	cr += c.r; 	cg += c.g; 	cb += c.b; 	ca += c.a;
		nsum++;
		}
	for (i=0; i<nacc; i++) {
		if (i>n) { // remove oldest one
			c = pixRec[i-n-1].c;
		   	cr -= c.r; 	cg -= c.g; 	cb -= c.b; 	ca -= c.a;
			nsum--;
			}
		if (i+n<nacc) { // add in new one
			c = pixRec[i+n].c;
		   	cr += c.r; 	cg += c.g; 	cb += c.b; 	ca += c.a;
			nsum++;
			}
		PixRec &pr = pixRec[i];
		AccumRec &ac = GETACC(pr.p.x,pr.p.y);
		ac.r += cr/nsum;
		ac.g += cg/nsum;
		ac.b += cb/nsum;
		ac.a += ca/nsum;
		ac.f += 1;
		}
	}

static void AccumFilterPixel( int ix, int iy, Point2 v) {
	curX = ix;
	curY = iy;
	int dx = Frac(0.5f*v.x);// + rand()&DITHMASK;
	int dy = Frac(0.5f*v.y);// + rand()&DITHMASK;
	SetCurZ(ix,iy);
	FracInt ixf = (ix<<FRBITS);
	FracInt iyf = (iy<<FRBITS);
	nacc = 0;
	BlurLine(ixf, iyf, ixf+dx, iyf+dy, 0); // accumulate list of pixels
	AccumFilterLine(nacc);   // use list to filter
	nacc = 0;
	BlurLine(ixf, iyf, ixf-dx, iyf-dy, 0); // accumulate list of pixels
	AccumFilterLine(nacc);	// use list to filter
	}

static void doPixAccumFilter(int x, int y, int frac) {
	if (x<0||x>=bmw||y<0||y>=fieldH) return;
	if (!TestZDepth(x,y)) return;
	pixRec[nacc].f =   frac;
	pixRec[nacc++].p = IPoint2(x,y);   // just accumulate a list of pixels
	}



//----------------------------------------------------------
// EXCHG
//----------------------------------------------------------

#ifdef DBLBUF
#define GETCOL(ix,iy) workMap.GetColor(ix,iy)
#else
#define GETCOL(ix,iy) pmap->GetColor(ix,iy)
#endif

//----------------------------------------------------------
// ERASE
//----------------------------------------------------------
static int count, maxcount;
static void ErasePixel( int ix, int iy, Point2 v) {
	curX = ix;
	curY = iy;
	curVeloc = v;
	curVMag = VelMag(v);
	int dx = Frac(0.5f*v.x); 
	int dy = Frac(0.5f*v.y);
	SetCurZ(ix,iy);
	BMM_Color_64 pcol = workMap.GetColor(ix,iy);
	dragCol = pcol;
	FracInt ixf = ix<<FRBITS;
	FracInt iyf = iy<<FRBITS;
	nacc = 0;
	accum_r = 0;
	accum_g = 0;
	accum_b = 0;
	accum_a = 0;
	count = 0;
	BlurLine(ixf, iyf, ixf+dx, iyf+dy, 0); // get both directions in one
	BMM_Color_64 c;
	if (nacc) {
		c.r = (accum_r)/nacc;
		c.g = (accum_g)/nacc;
		c.b = (accum_b)/nacc;
		c.a = (accum_a)/nacc;
		pmap->SetColor(ix,iy,c);
		}
	}

#define EPS (FR1>>9)
static void doPixErase1(int x, int y, int frac) {
	if (x>=0&&x<bmw&&y>=0&&y<fieldH) {
		BMM_Color_64 cw = workMap.GetColor(x,y);
		Point2  v = GETV(x,y);
		int dFact = ((maxcount-count)<<8)/maxcount;
		if (v==pzero) {
			addAll:
			accum_r += dFact*cw.r;	
			accum_g += dFact*cw.g;	
			accum_b += dFact*cw.b;	
			accum_a += dFact*cw.a;	
			nacc+=dFact;
			return;
			}
		Point2  dv = Point2(v.x-curVeloc.x, v.y-curVeloc.y);
		FracInt d = FrDiv(VelMag(dv),curVMag); // THIS CAN OVERFLOW!!
		if (d>=FR1) goto addAll;
		if (d>EPS) {
			d =  ((d>>4)*dFact)>>8;
			accum_r += cw.r*d;	
			accum_g += cw.g*d;	
			accum_b += cw.b*d;	
			accum_a += cw.a*d;	
			nacc+=d;
			}
		}
	}

static void doPixErase(int x, int y, int frac) {
	doPixErase1(x,y,frac);
	doPixErase1(2*curX-x,2*curY-y,frac);
	count++;
	}

//----------------------------------------------------------
// SMEAR
//----------------------------------------------------------
static void SmearPixel( int ix, int iy, Point2 v) {
	curX = ix;
	curY = iy;
	int dx = Frac(0.5f*v.x);// + rand()&DITHMASK; 
	int dy = Frac(0.5f*v.y);// + rand()&DITHMASK;

	SetCurZ(ix,iy);
#ifdef DBLBUF
	BMM_Color_64 pcol = workMap.GetColor(ix,iy);
#else 
	BMM_Color_64 pcol = pmap->GetColor(ix,iy);
#endif
	dragCol = pcol;
	nacc = 0;
	accum_r = accum_g = accum_b = accum_a = 0;

	FracInt ixf = (ix<<FRBITS);
	FracInt iyf = (iy<<FRBITS);
	BlurLine(ixf, iyf, ixf+dx, iyf+dy, 0);  // get both directions in one
	}

static inline void doPixSmear1(int x, int y, FracInt frac) {
	if (x>=0&&x<bmw&&y>=0&&y<fieldH) {
		if (!TestZDepth(x,y)) return;
		BMM_Color_64 c = pmap->GetColor(x,y);
		pmap->SetColor(x, y,BLEND(frac,c,dragCol));
		}
	}

static void doPixSmear(int x, int y, int frac) {
	if (x==curX&&y==curY) return;
	doPixSmear1(x,y,frac);
	doPixSmear1(2*curX-x,2*curY-y,frac);
	}

//----------------------------------------------------------
// AASMEAR
//----------------------------------------------------------
static void AASmearPixel( int ix, int iy, Point2 v) {
	curX = ix;
	curY = iy;
	int dx = Frac(0.5f*v.x); 
	int dy = Frac(0.5f*v.y);

	SetCurZ(ix,iy);
#ifdef DBLBUF
	BMM_Color_64 pcol = workMap.GetColor(ix,iy);
#else 
	BMM_Color_64 pcol = pmap->GetColor(ix,iy);
#endif
	dragCol = pcol;
	nacc = 0;
	accum_r = accum_g = accum_b = accum_a = 0;

	int ixf = ix<<FRBITS;
	int iyf = iy<<FRBITS;
	FracInt dt = 0;//Frac(v.z);
#ifdef AALINE
	AABlurLine(ixf, iyf, ixf+dx, iyf+dy, dt);  // get both directions in one
	AABlurLine(ixf, iyf, ixf-dx, iyf-dy, -dt);  // get both directions in one
#endif
	}

static inline void doAAPixSmear(int x, int y, FracInt frac) {
	if (x>=0&&x<bmw&&y>=0&&y<fieldH) {
		if (!TestZDepth(x,y)) return;
		BMM_Color_64 c = pmap->GetColor(x,y);
		pmap->SetColor(x, y,BLEND(frac,c,dragCol));
		}
	}

#endif
