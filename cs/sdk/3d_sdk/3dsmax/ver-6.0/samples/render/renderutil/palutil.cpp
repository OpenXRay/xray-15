/*******************************************************************
 *
 *    DESCRIPTION: Color Palette Functions
 *
 *    AUTHOR: & Dan Silva
 *
 *    HISTORY: Started coding 12/12/94
 *
 *******************************************************************/

/**********************************************************************

		C Implementation of Wu's Color Quantizer

Algorithm: Greedy orthogonal bipartition of RGB space for variance
	   minimization aided by inclusion-exclusion tricks.

		See: Graphics Gems II, pp 126 ff.
**********************************************************************/ 
#include "max.h"
#include "palutil.h"

#define xDBG

#define local static

#define MAXCOLOR        256			// Max number of palette entries
 
#define RED     			2
#define GREEN   			1
#define BLUE    			0

BMM_Color_48 bg_color = {0,0,0};

struct ColorBox {
	int r0,g0,b0;  
	int r1,g1,b1;  
	int vol;
	};

typedef float FCubePlane[33][33];  
typedef long ICubePlane[33][33];
typedef FCubePlane FCube[33];
typedef ICubePlane ICube[33];

class PixGen {
	public:
	virtual BMM_Color_64 GetPix(int i)=0;
	};


class QuantImp: public Quantizer {
	FCubePlane *m2;
	ICubePlane *wt,*mr,*mg,*mb;
	int size;		//image size
	int lut_size;	//color look-up table size
	int quant_incr;
	void M3d();
	long Vol(ColorBox *cube, ICube mmt);
	long QBottom(ColorBox *cube, int dir, ICube mmt);
	long QTop(ColorBox *cube, int dir,int pos, ICube mmt);
	float Maximize(ColorBox *cube, int dir, int first,  int last, int *cut,
		long int whole_r, long int whole_g, long int whole_b, long int whole_w);
	int Cut(ColorBox *set1, ColorBox *set2);
	float Var(ColorBox *gcube);
	void SetInc(int i) { quant_incr = (i<1)?1:i; }
	void FreeHistogram(void);
	public:
 		QuantImp();
		int AllocHistogram(void);
		~QuantImp() { FreeHistogram(); }
		int Partition(BMM_Color_48 *pal, int palsize, BMM_Color_64 *forceCol);
		void AddToHistogram(BMM_Color_64 *image, int npix); 
		void AddToHistogram(BMM_Color_48 *image, int npix);	
		void AddToHistogram(BMM_Color_24 *image, int npix);	
		void AddToHistogram(PixGen &pg, int npix);
		void DeleteThis() { delete this; }
	};

Quantizer *NewQuantizer() { return new QuantImp; }

double  max_3pts ( double val1, double val2, double val3)	{
   double  max_val;
	max_val = val1;
	if ( val2 > max_val)	max_val = val2;
	if ( val3 > max_val)	max_val = val3;
   return max_val;
	}	   // End of function: max_3pts()

double  min_3pts ( double val1, double val2, double val3) {
	double  min_val;
	min_val = val1;
	if ( val2 < min_val)	min_val = val2;
	if ( val3 < min_val)	min_val = val3;
	return min_val;
	}	   // End of function: min_3pts()

QuantImp::QuantImp() {
	m2 = NULL;
	wt = mr = mg = mb = NULL;
	quant_incr = 1;
	}

/* Free the histogram data structures */
void QuantImp::FreeHistogram(void) {
	delete [] m2;
	delete [] wt;
	delete [] mr;
	delete [] mg;
	delete [] mb;
	m2 = NULL;
	wt = NULL;
	mr = mg = mb = NULL;
	}

/* Allocate the histogram data structures (all .7 Meg of 'em-- Yikes!) */
int QuantImp::AllocHistogram(void) {
	size = 0;
	m2 = new FCubePlane[33];
	wt = new ICubePlane[33];
	mr = new ICubePlane[33];
	mg = new ICubePlane[33];
	mb = new ICubePlane[33];
	if (m2==0||wt==0||mr==0||mg==0||mb==0) {
	 	FreeHistogram();
		return(0);
		}
	memset(m2, 0, 33*sizeof(FCubePlane));
	memset(wt, 0, 33*sizeof(ICubePlane));
	memset(mr, 0, 33*sizeof(ICubePlane));
	memset(mg, 0, 33*sizeof(ICubePlane));
	memset(mb, 0, 33*sizeof(ICubePlane));
	return(1);
	}


/* Add 'image', comprised of 'npix' pixels, into the current histogram */
void QuantImp::AddToHistogram(PixGen &pg, int npix)	{
	/* build 3-D color density */
	register int  ind, r, g, b;
	int	inr, ing, inb, table[256];
	long int i;
	long int	*vwt, *vmr, *vmg, *vmb, oldsize;
	float	*vm2;
	BMM_Color_64 pix;

#ifdef DBG
	printf("QuantAddHist: quant_incr = %d \n",quant_incr);

#endif
	oldsize = size;
	size += npix;

	for(i=0; i<256; ++i) table[i]=i*i;

	vwt = &wt[0][0][0];
	vmr = &mr[0][0][0];
	vmg = &mg[0][0][0];
	vmb = &mb[0][0][0];
	vm2 = &m2[0][0][0];

	for (i=((rand()&0x7fff)%quant_incr); i<npix; i+=quant_incr) {
		pix = pg.GetPix(i);

		r = pix.r >> 8; 
		g = pix.g >> 8; 
		b = pix.b >> 8;
		 
		inr=(r>>3)+1; 
		ing=(g>>3)+1; 
		inb=(b>>3)+1; 

		/*[inr][ing][inb]*/

		/* calculate (inb + 33*ing + (33*33*inr))  */
 
		ind = (inr<<10) + (inr<<6) + inr + (ing<<5) + ing + inb;

		vwt[ind]++;
		vmr[ind] += r;	  	// only get six bits per gun on VGA
		vmg[ind] += g;
		vmb[ind] += b;

		vm2[ind] += (float)(table[r]+table[g]+table[b]);
		}
	quant_incr = 1;
	}


class Pixgen64: public PixGen {
	BMM_Color_64 *c;
	public:
	Pixgen64(BMM_Color_64 *x) { c = x; }
	BMM_Color_64 GetPix(int i) { return c[i]; }
	};

class Pixgen48: public PixGen {
	BMM_Color_48 *c;
	public:
	Pixgen48(BMM_Color_48 *x) { c = x; }
	BMM_Color_64 GetPix(int i) { 
		BMM_Color_64 a;
		a.r = c[i].r;
		a.g = c[i].g;
		a.b = c[i].b;
		a.a = 0;
		return a;
		}
	};

class Pixgen24: public PixGen {
	BMM_Color_24 *c;
	public:
	Pixgen24(BMM_Color_24 *x) { c = x; }
	BMM_Color_64 GetPix(int i) { 
		BMM_Color_64 a;
		a.r = c[i].r<<8;
		a.g = c[i].g<<8;
		a.b = c[i].b<<8;
		a.a = 0;
		return a;
		}
	};

void QuantImp::AddToHistogram(BMM_Color_64 *image, int npix)	{
	Pixgen64 pg(image);
	AddToHistogram(pg,npix);
	}

void QuantImp::AddToHistogram(BMM_Color_24 *image, int npix)	{
	Pixgen24 pg(image);
	AddToHistogram(pg,npix);
	}

void QuantImp::AddToHistogram(BMM_Color_48 *image, int npix)	{
	Pixgen48 pg(image);
	AddToHistogram(pg,npix);
	}

/* We now convert histogram into moments so that we can rapidly calculate
 * the sums of the above quantities over any desired box.
 */
void QuantImp::M3d() {
	register unsigned short i, r, g, b;
	long int  line, line_r, line_g, line_b,
	 		area[33], area_r[33], area_g[33], area_b[33];
	float	line2, area2[33];
	register int ind1, ind2;
	long int *vwt, *vmr, *vmg, *vmb;
	float	*vm2;
	vwt = &wt[0][0][0];
	vmr = &mr[0][0][0];
	vmg = &mg[0][0][0];
	vmb = &mb[0][0][0];
	vm2 = &m2[0][0][0];

   for (r=1; r<=32; ++r) {
		for (i=0; i<=32; ++i) { 
		   area[i]=area_r[i]=area_g[i]=area_b[i]= 0;
		   area2[i]=(float)0.0;
		   }

		for(g=1; g<=32; ++g){
	   		line2 = (float)0.0;
	   		line = line_r = line_g = line_b = 0;
			for(b=1; b<=32; ++b){
				ind1 = (r<<10) + (r<<6) + r + (g<<5) + g + b; /* [r][g][b] */
				
				line += vwt[ind1];
				line_r += vmr[ind1]; 
				line_g += vmg[ind1]; 
				line_b += vmb[ind1];
				line2 += vm2[ind1];

				area[b] += line;
				area_r[b] += line_r;
				area_g[b] += line_g;
				area_b[b] += line_b;
				area2[b] += line2;

				ind2 = ind1 - 1089; /* [r-1][g][b] */
				vwt[ind1] = vwt[ind2] + area[b];
				vmr[ind1] = vmr[ind2] + area_r[b];
				vmg[ind1] = vmg[ind2] + area_g[b];
				vmb[ind1] = vmb[ind2] + area_b[b];
				vm2[ind1] = vm2[ind2] + area2[b];
				}
			}
		}
	}

long QuantImp::Vol(ColorBox *cube, ICube mmt) {
	 return( mmt[cube->r1][cube->g1][cube->b1] 
			  -mmt[cube->r1][cube->g1][cube->b0]
			  -mmt[cube->r1][cube->g0][cube->b1]
			  +mmt[cube->r1][cube->g0][cube->b0]
			  -mmt[cube->r0][cube->g1][cube->b1]
	   	  +mmt[cube->r0][cube->g1][cube->b0]
			  +mmt[cube->r0][cube->g0][cube->b1]
	   	  -mmt[cube->r0][cube->g0][cube->b0] );
	}


/* The next two routines allow a slightly more efficient calculation
 * of Vol() for a proposed subbox of a given box.  The sum of Top()
 * and Bottom() is the Vol() of a subbox split in the given direction
 * and with the specified new upper bound.
 */
long QuantImp::QBottom(ColorBox *cube, int dir, ICube mmt)	{
	switch(dir){
		case RED:
	   	return( -mmt[cube->r0][cube->g1][cube->b1]
		   	+mmt[cube->r0][cube->g1][cube->b0]
		 	+mmt[cube->r0][cube->g0][cube->b1]
				-mmt[cube->r0][cube->g0][cube->b0] );
  		case GREEN:
	   	return( -mmt[cube->r1][cube->g0][cube->b1]
		 	   +mmt[cube->r1][cube->g0][cube->b0]
		   	+mmt[cube->r0][cube->g0][cube->b1]
		   	-mmt[cube->r0][cube->g0][cube->b0] );
		case BLUE:
		default:
			return( -mmt[cube->r1][cube->g1][cube->b0]
		   	+mmt[cube->r1][cube->g0][cube->b0]
		   	+mmt[cube->r0][cube->g1][cube->b0]
		   	-mmt[cube->r0][cube->g0][cube->b0] );
   	 	}
	}


long QuantImp::QTop(ColorBox *cube, int dir,int pos, ICube mmt) {
	switch(dir){
		case RED:
			return( mmt[pos][cube->g1][cube->b1] 
			  -mmt[pos][cube->g1][cube->b0]
		   	  -mmt[pos][cube->g0][cube->b1]
		   	  +mmt[pos][cube->g0][cube->b0] );
		case GREEN:
			return( mmt[cube->r1][pos][cube->b1] 
		   	  -mmt[cube->r1][pos][cube->b0]
		   	  -mmt[cube->r0][pos][cube->b1]
		   	  +mmt[cube->r0][pos][cube->b0] );
		case BLUE:
		default:
			return( mmt[cube->r1][cube->g1][pos]
		   	  -mmt[cube->r1][cube->g0][pos]
		   	  -mmt[cube->r0][cube->g1][pos]
		   	  +mmt[cube->r0][cube->g0][pos] );
		}
	}

/* Compute the weighted variance of a box */
/* NB: as with the raw statistics, this is really the variance * size */
float QuantImp::Var(ColorBox *gcube) {
	float 		dr, dg, db, xx;

   dr = (float)Vol(gcube, mr); 
   dg = (float)Vol(gcube, mg); 
   db = (float)Vol(gcube, mb);

   xx =  m2[gcube->r1][gcube->g1][gcube->b1] 
		-m2[gcube->r1][gcube->g1][gcube->b0]
	 	-m2[gcube->r1][gcube->g0][gcube->b1]
	 	+m2[gcube->r1][gcube->g0][gcube->b0]
	 	-m2[gcube->r0][gcube->g1][gcube->b1]
	 	+m2[gcube->r0][gcube->g1][gcube->b0]
	 	+m2[gcube->r0][gcube->g0][gcube->b1]
	 	-m2[gcube->r0][gcube->g0][gcube->b0];

   return( xx - (dr*dr+dg*dg+db*db)/(float)Vol(gcube,wt) );
	}

/* We want to minimize the sum of the variances of two subboxes.
 * The sum(c^2) terms can be ignored since their sum over both subboxes
 * is the same (the sum for the whole box) no matter where we split.
 * The remaining terms have a minus sign in the variance formula,
 * so we drop the minus sign and MAXIMIZE the sum of the two terms.
 */
float QuantImp::Maximize(ColorBox *cube, int dir, int first,  int last, int *cut,
		long int whole_r, long int whole_g, long int whole_b, long int whole_w)

	{
	register long 	int half_r, half_g, half_b, half_w;
	long int 		base_r, base_g, base_b, base_w;
	register 		int i;
	register float temp, max;
	
	base_r = QBottom(cube, dir, mr);
	base_g = QBottom(cube, dir, mg);
	base_b = QBottom(cube, dir, mb);
	base_w = QBottom(cube, dir, wt);

	max = (float)0.0;
	*cut = -1;
	for(i=first; i<last; i++){
		half_r = base_r + QTop(cube, dir, i, mr);
		half_g = base_g + QTop(cube, dir, i, mg);
		half_b = base_b + QTop(cube, dir, i, mb);
		half_w = base_w + QTop(cube, dir, i, wt);

		/* now half_x is sum over lower half of box, if split at i */
		if (half_w == 0) {	  /* subbox could be empty of pixels! */
			continue;			 /* never split into an empty box */
			}
		else
			temp = ((float)half_r*half_r + (float)half_g*half_g +
				(float)half_b*half_b)/half_w;

		half_r = whole_r - half_r;
		half_g = whole_g - half_g;
		half_b = whole_b - half_b;
		half_w = whole_w - half_w;
		if (half_w == 0) {	  /* subbox could be empty of pixels! */
			continue;			 /* never split into an empty box */
			} 
		else
			temp += ((float)half_r*half_r + (float)half_g*half_g +
				(float)half_b*half_b)/half_w;

		if (temp > max) {max=temp; *cut=i;}
		}
	return(max);
	}

int QuantImp::Cut(ColorBox *set1, ColorBox *set2) {
	int 	dir;
	int 				cutr, cutg, cutb;
	float			maxr, maxg, maxb;
	long int 		whole_r, whole_g, whole_b, whole_w;

	whole_r = Vol(set1, mr);
	whole_g = Vol(set1, mg);
	whole_b = Vol(set1, mb);
	whole_w = Vol(set1, wt);

	maxr = Maximize(set1, RED  , set1->r0+1, set1->r1, &cutr,
						whole_r, whole_g, whole_b, whole_w);
	maxg = Maximize(set1, GREEN, set1->g0+1, set1->g1, &cutg,
						 whole_r, whole_g, whole_b, whole_w);
	maxb = Maximize(set1, BLUE , set1->b0+1, set1->b1, &cutb,
						whole_r, whole_g, whole_b, whole_w);

#ifdef DBG
	printf("cutr = %d cutg = %d cutb = %d \n",cutr,cutg,cutb);
   printf("maxr = %f maxg = %f maxb = %f \n",maxr,maxg,maxb);
#endif
	if( (maxr>=maxg)&&(maxr>=maxb) ) {
		dir = RED;
		if (cutr < 0) return 0; /* can't split the box */
		}
	else
		if( (maxg>=maxr)&&(maxg>=maxb) )
			dir = GREEN;
		else
			dir = BLUE;

	set2->r1 = set1->r1;
	set2->g1 = set1->g1;
	set2->b1 = set1->b1;

	switch (dir){
		case RED:
			set2->r0 = set1->r1 = cutr;
			set2->g0 = set1->g0;
			set2->b0 = set1->b0;
			break;
		case GREEN:
			set2->g0 = set1->g1 = cutg;
			set2->r0 = set1->r0;
			set2->b0 = set1->b0;
			break;
		case BLUE:
			set2->b0 = set1->b1 = cutb;
			set2->r0 = set1->r0;
			set2->g0 = set1->g0;
			break;
		}
	set1->vol=(set1->r1-set1->r0)*(set1->g1-set1->g0)*(set1->b1-set1->b0);
	set2->vol=(set2->r1-set2->r0)*(set2->g1-set2->g0)*(set2->b1-set2->b0);
	return 1;
	}

void SortPalByWeight(BMM_Color_48 *pal, int npal, long *wt) {
	int k,sortit = 0;
	long l;
	BMM_Color_48 tmp;
	do {
		sortit = 0;
		for ( k = 0; k < (npal-1); k++ ) {
			if ( wt[k+1] > wt[k] ) {
				tmp = pal[k]; pal[k] = pal[k+1];	pal[k+1] = tmp;
			 	l = wt[k]; wt[k] = wt[k+1]; wt[k+1] = l;
				sortit = 1;
				}
			}
		} while (sortit == 1);
	}

/* -- Cut a palette based on current histogram */
int QuantImp::Partition(BMM_Color_48 *pal, int palsize, BMM_Color_64 *forceCol)  {
	float	 vv[MAXCOLOR], temp;
	ColorBox cube[MAXCOLOR];
	long  	 wts[MAXCOLOR];
	int   	 next,k,n;
	register int	i, weight;

#ifdef DBG
	{
	int ir,ig,ib;
	ir=(12>>3)+1; 
	ig=(36>>3)+1; 
	ib=(56>>3)+1; 
	printf("===(%d,%d,%d) wt = %d, mr = %d, mg = %d, mb = %d \n",
	 ir,ig,ib,wt[ir][ig][ib],mr[ir][ig][ib],mg[ir][ig][ib],mb[ir][ig][ib]);
	}
#endif

	lut_size = palsize;
	if (lut_size>MAXCOLOR) lut_size = MAXCOLOR;
	
	if (forceCol)
		lut_size -= 1;

	M3d(); 	 

#ifdef DBG	
	printf("moments done\n");
#endif

	for (i = 0; i < MAXCOLOR; ++i) {
		cube[i].r0  = cube[i].g0 = cube[i].b0 = 0;
		cube[i].r1  = cube[i].g1 = cube[i].b1 = 32;
		cube[i].vol = 0;
		}

	cube[0].r0 = cube[0].g0 = cube[0].b0 = 0;
	cube[0].r1 = cube[0].g1 = cube[0].b1 = 32;

	next = 0;

	for(i=1; i<lut_size; ++i){
#ifdef DBG	
		printf("Cut(&cube[%d], &cube[%d]);\n",next,i);
#endif
		if (Cut(&cube[next], &cube[i])) {
	 		/* volume test ensures we won't try to cut one-cell box */
			vv[next] = (cube[next].vol>1) ? Var(&cube[next]) : (float)0.0;
			vv[i] = (cube[i].vol>1) ? Var(&cube[i]) : (float)0.0;
			} 
		else {
			vv[next] = (float)0.0;	/* don't try to split this box again */
			i--;					/* didn't create box i */
			}
		next = 0;
		temp = vv[0];

		for(k=1; k<=i; ++k)
			if (vv[k] > temp) {
				temp = vv[k]; next = k;
				}

		if (temp <= 0.0) {
			lut_size = i+1;
#ifdef DBG
			printf("Only got %d boxes\n", lut_size);
#endif
			break;
			}
		}

#ifdef DBG
	printf("Partition done\n");
#endif

	for(k=0,n=0; k<lut_size; ++k){
		weight = Vol(&cube[k], wt);
		if (weight) { 
			pal[n].r = (Vol(&cube[k], mr) / weight) << 8;
			pal[n].g = (Vol(&cube[k], mg) / weight) << 8;
			pal[n].b = (Vol(&cube[k], mb) / weight) << 8;
			wts[n] = weight;
			n++;
			}
		}

	/* put most important colors first */
	SortPalByWeight(pal,lut_size,wts);


	if (forceCol) {
#if 0
		printf(" FORCING BG COLOR INTO PALETTE (%d,%d,%d)\n",
			bg_color.r,bg_color.g,bg_color.b);
#endif
		for (i=lut_size; i>0; i--) 
			pal[i] = pal[i-1];
		pal[0].r = forceCol->r;
		pal[0].g = forceCol->g;
		pal[0].b = forceCol->b;
		lut_size++;
		}

#ifdef DBG
	for(k=0; k<lut_size; k++)
		printf(" sorted_pal[%d] = (%d,%d,%d)\n",k,pal[k].r,pal[k].g,pal[k].b);
#endif

//	FreeHistogram();;

	return(lut_size);
	}


#ifndef ABS
#define ABS(x) (((x) < 0) ? -(x):(x))
#endif

/*-------------------------------------------------------------------------
  Floyd-Steinberg Error redistribution:

	dithcon1: 16ths to right
	dithcon2: 16ths below to left
	dithcon3: 16ths below 
	dithcon4: 16ths below to right

	Floyd-Steinberg dither is 7,3,5,1 --   I like 6,2,5,1 better. --DS

	if (dithcon2==dithcon3==dithcon4==0) it is
 	  scan-line local dithering, which works better for animation.

----------------------------------------------*/
#define HASHBUFSIZE 4096
	
struct ColorLong { int r,g,b; };
struct HashEntry {
	UWORD valid, closest;
	UWORD  r, g, b;
	};

class ColorPackerX: public ColorPacker {
	int width;
	BOOL dither;
	BOOL floyd; // really means propogate between lines
	BOOL inited;
	HashEntry *hash;
	ColorLong *dith_errbuf;
	ColorLong errq[2];
	BMM_Color_48* thepal; 
	int thepalN;
	BYTE *theRemap;
	int dithcon1; 
	int dithcon2;
	int dithcon3;
	int dithcon4;
	int dith_x;

	int	InitBuffers();
	void ResetDithLine();
	void FreeDitherBuf() { 	delete [] dith_errbuf;	dith_errbuf = NULL;	}
	int FindClosestColor(const BMM_Color_48& c);
	int ClosestColor(const BMM_Color_64& col);

	public:
		ColorPackerX(int w, BMM_Color_48 *pal, int npal, BYTE* remap=NULL, BOOL dither=TRUE);
		void EnableDither(BOOL onoff);  // default: MAX default
		void PropogateErrorBetweenLines(BOOL onoff);  // default ON; 
		void PackLine( BMM_Color_64* in, BYTE *out, int w);
		void PackLine( BMM_Color_48* in, BYTE *out, int w);
		virtual void DeleteThis() { delete this; }
		~ColorPackerX();
	};

ColorPackerX::ColorPackerX(int w, BMM_Color_48 *pal, int npal, BYTE* remap, BOOL dith) {
	width = w; 
	thepal = pal;
	thepalN = npal;
	hash = NULL; 
	dith_errbuf = NULL;
	theRemap = remap;
	dither = dith;
	inited = FALSE;
	PropogateErrorBetweenLines(TRUE); 
	}

ColorPackerX::~ColorPackerX() {
	FreeDitherBuf();
	delete [] hash;
	}

void ColorPackerX::EnableDither(BOOL onoff) { dither = onoff; }
void ColorPackerX::PropogateErrorBetweenLines(BOOL onoff) { 
	floyd = onoff; 
	if (floyd) {
		dithcon1 = 6;
		dithcon2 = 2;
		dithcon3 = 5;
		dithcon4 = 1;
		}
	else {
		dithcon1 = 14;
		dithcon2 = dithcon3 = dithcon4 = 0;
		}
	}

int	ColorPackerX::InitBuffers() {
	if (dither)	{
		FreeDitherBuf();
		if (dithcon2+dithcon3+dithcon4>0) {
			dith_errbuf = new ColorLong[width+1];
			if (dith_errbuf)
				memset(dith_errbuf,0,(width+1)*sizeof(ColorLong));
			}
		else {
			delete [] dith_errbuf;
			dith_errbuf=NULL;
			}
		ResetDithLine();
		}
	hash = new HashEntry[HASHBUFSIZE];
	if (hash)
		memset(hash,0,HASHBUFSIZE*sizeof(HashEntry));
	inited = 1;
	return hash?1:0;
	}

#define LIM16BITS(x) ((x)<0?0:((x)>65535?65535:(x)))

void ColorPackerX::ResetDithLine() {
	if (dither)	{
		memset(errq,0,2*sizeof(ColorLong));
		dith_x = 0;
		if (dith_errbuf) {
			errq[0] = dith_errbuf[0];
			dith_errbuf[0].r = dith_errbuf[0].g = dith_errbuf[0].b = 0;
			}
		}
	}

int ColorPackerX::ClosestColor(const BMM_Color_64& col) {
	int i,ic;
	int rerr,gerr,berr,r,g,b;
	BMM_Color_48 c;
	ColorLong *dbuf;
	HashEntry *h;
	int xx = sizeof(HashEntry);
	if (dither)	{
		r = (int)col.r + errq[0].r;
		g = (int)col.g + errq[0].g;
		b = (int)col.b + errq[0].b;
		c.r =  LIM16BITS(r);
		c.g =  LIM16BITS(g);
		c.b =  LIM16BITS(b);
		}
	else {
		c.r = col.r;
		c.g = col.g;
		c.b = col.b;
		}

	/* first look for a hash hit */
	i = ((c.b>>8)+(c.g>>4)+(c.r&0xF00)+((c.r&0xF000)>>12))&0xfff; 
	h = hash+i;
	if (! (h->valid && h->r == c.r && h->g == c.g && h->b == c.b) ) {
		h->closest = FindClosestColor(c);
#ifdef DBGDITH
		if (dbgdith)
			DebugPrint("  closest(%d,%d,%d) = pal[%d] = (%d,%d,%d) x=%d y=%d, hash# %d\n",
				c.r,c.g,c.b,h->closest,thepal[h->closest].r,thepal[h->closest].g,thepal[h->closest].b,
				dbgx,dbgy,i);
#endif
		h->r = c.r;
		h->g = c.g;
		h->b = c.b;
		h->valid = 1;
		}

	ic = h->closest;

	if (dither)	{
#ifdef DBGDITH
		if (dbgdith) 
			DebugPrint("rgb = (%d,%d,%d) , err=(%d,%d,%d) closest=%d (%d,%d,%d) x=%d y=%d\n",	
				rgb[0],rgb[1],rgb[2],rerr,gerr,berr,h->closest,
				thepal[ic].r,thepal[ic].g,thepal[ic].b,dbgx,dbgy);
#endif
	
		rerr = r - thepal[ic].r;
		gerr = g - thepal[ic].g;
		berr = b - thepal[ic].b;

		/*---- advance the Queue ---*/
		errq[0] = errq[1];
		
		/* 7/16 of error goes to next pixel to right */
		errq[0].r += ((rerr*dithcon1)>>4);
		errq[0].g += ((gerr*dithcon1)>>4);
		errq[0].b += ((berr*dithcon1)>>4);
		
		if (dith_errbuf!=NULL) {
			if (dith_x>=width) {
				//assert(0);
				goto err;
				}
			errq[1] = dith_errbuf[dith_x+1];

			/* 3/16 of error goes to pixel below to left */
			if (dith_x>0) {
				dbuf = &dith_errbuf[dith_x-1];
				dbuf->r += ((rerr*dithcon2)>>4);
				dbuf->g += ((gerr*dithcon2)>>4);
				dbuf->b += ((berr*dithcon2)>>4);
				}
	
			/* 5/16 of error goes to pixel below */
			dbuf = &dith_errbuf[dith_x];
			dbuf->r += ((rerr*dithcon3)>>4);
			dbuf->g += ((gerr*dithcon3)>>4);
			dbuf->b += ((berr*dithcon3)>>4);

			/* 1/16 of error goes to pixel below to right*/
			dbuf++;
			dbuf->r = ((rerr*dithcon4)>>4);
			dbuf->g = ((gerr*dithcon4)>>4);
			dbuf->b = ((berr*dithcon4)>>4);
			}

		dith_x++;
		}
	err:
	return(theRemap?theRemap[ic]:ic);
	}

int ColorPackerX::FindClosestColor(const BMM_Color_48& c) {
	register int sum,i,min,imin;
	register BMM_Color_48 *pal = thepal;
	BMM_Color_48 col;
	min = 1000000;
	for (i=0; i<thepalN; i++) {
		col = *pal++;
		sum = abs(c.r-col.r)+abs(c.g-col.g)+abs(c.b-col.b);
		if (sum<min) {
			imin = i; 
			min = sum;	
			if (sum==0) break;
			}
		}
#ifdef DBGDITH
	if (dbgdith)
		DebugPrint(" find_closest_col of (%d,%d,%d) = pal[%d] = (%d,%d,%d) \n",
			c.r,c.g,c.b,imin,thepal[imin].r,thepal[imin].g,thepal[imin].b);
#endif
	return(imin);
	}

void ColorPackerX::PackLine( BMM_Color_64* in, BYTE *out, int w) {
	if (!inited) 
		InitBuffers();
	dith_x = 0;
	if (hash==NULL) return;
	for (int i=0; i<w; i++)
		*out++ = ClosestColor(*in++);
	}


void ColorPackerX::PackLine( BMM_Color_48* in, BYTE *out, int w) {
	if (!inited) 
		InitBuffers();
	dith_x = 0;
	if (hash==NULL) return;
	BMM_Color_64 c;
	for (int i=0; i<w; i++) {
		c.r = in->r;
		c.g = in->g;
		c.b = in->b;
		*out++ = ClosestColor(c);
		in++;
		}
	}

ColorPacker *NewColorPacker(int w, BMM_Color_48 *pal, int npal, BYTE* remap, BOOL dither) {
	return new ColorPackerX(w,pal,npal,remap,dither);
	}

void FixPaletteForWindows(BMM_Color_48 *pal, BMM_Color_48 *newpal,int ncols, BYTE *remap) {
	int i,imin;
	long v, mincol;
	int n;
	BMM_Color_48 c;
	BYTE map[256];
	n = ncols;
	if (n>236) n=236;
	memset(newpal,0,256*sizeof(BMM_Color_48));
	for (i=0; i<n; i++) 
		map[i] = i+10;
	if (ncols<=236) 
		goto finish;
	n = ncols;
	if (n>246) n=246;
	for (i=236; i<n; i++) 
		map[i] = i-236;
	for (i=246; i<ncols; i++) 
		map[i] = i;

	finish:
	
	// find darkest color to put in slot 0
	imin=0;
	mincol = 1000000;
	for (i=0; i<ncols; i++) {
		c = pal[i];
		v = abs(c.r) + abs(c.g) + abs(c.b);
		if (v<mincol) { 
			mincol = v; 
			imin = i; 
			if (mincol==0) break;
			}
		}

	// OK, pal[imin] is the darkest color: it now maps to map[imin].  Want it 
    // to map to 0, and what now maps to 0 to map to what map[imin]
	if (ncols>236) map[236]  = map[imin];
	map[imin] = 0;

	/* map the palette */
	for (i=0; i<ncols; i++) 
		newpal[map[i]] = pal[i];		
	if (remap) {
		for (i=0;i<256; i++) remap[i] = map[i];
		}
	}
