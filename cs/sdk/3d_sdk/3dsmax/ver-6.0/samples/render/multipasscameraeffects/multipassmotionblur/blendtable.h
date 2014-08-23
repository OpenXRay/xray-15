/* -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

   FILE: blendTable.h

	 DESCRIPTION: declarations for class BlendTable

	 CREATED BY: kells elmquist (kae)

	 HISTORY: created July 17, 2000

   	 Copyright (c) 2000, All Rights Reserved

// -----------------------------------------------------------------------------
// -------------------------------------------------------------------------- */
#ifndef __BLEND_TABLE__H
#define __BLEND_TABLE__H


// maximum channel value for an unsigned 64 bit color
#define MAX_COLOR64U 65535

// min/max clamp
inline float Bound(float x, float min = 0.0f, float max = 1.0f) { return (x < min) ? min : (x > max) ? max : x; }

// noise function prototype for [0...1) noise
typedef float (* TNoiseFn)(Random &random);

// ----------------------------------
// ----------------------------------
// declarations of noise functions
// ----------------------------------
// ----------------------------------
float dummyNoise(Random &random);
float RandomNoise(Random &random);
float SumNoise(Random &random);
float CRCNoise(Random &random);
float GaussNoise(Random &random);


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

class Color64U

// a color class to extend functionality for accumulating images

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
{
public:
	unsigned short r;
	unsigned short g;
	unsigned short b;
	unsigned short a;

public:
	Color64U() { }
	Color64U( short R, short G, short B, short A ){ r=R; g=G; b=B; a=A; }
    Color64U& operator+=(const Color64U& c){ r += c.r ; g += c.g; b += c.b; a += c.a; return *this; }
    Color64U& operator*=(const float f){ r *= f; g *= f; b *= f; a *= f; return *this; }
    void AddSafe( Color64U& c )
	{
		int i = r + c.r;
		r = i > MAX_COLOR64U ? MAX_COLOR64U : i;
		i = g + c.g;
		g = i > MAX_COLOR64U ? MAX_COLOR64U : i;
		i = b + c.b;
		b = i > MAX_COLOR64U ? MAX_COLOR64U : i;
		i = a + c.a;
		a = i > MAX_COLOR64U ? MAX_COLOR64U : i;
	}
};


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

class BlendTable

// This builds a blending table of tables, each tableX by tableY pixels.
// There is one such table for each image.
//
// The blending algorithm starts with an even division for the image sum, 
// each image gets 1/nToblend of the total. If the ditherAmt is 0, then this
// uniform sum will be used to blend the images.
//
// if the ditherAmt is not 0, some noise will be added to each of the weights.
// the noise is scaled so that at ditherAmt = 1, the amplitude of the noise 
// is 1/nToBlend, & it is shifted so that half the range is above & half below
// the original uniform weight. ditherAmt = 0.1 is a good starting point.
//
// If normalizeTable is TRUE, then the weights of each pixel are scaled so 
// that the sum of the weights is 1.0.
//
// note also that the spectral character of the noise function given will
// have a fairly profound effect on the final look.

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
{
protected:
	int mTableSzX, mTableSzY;
	int mnToBlend;
	int mTableSzXY;		// layer sz
	int mTotalSz;		// all layers
	float* mpTable;		// the dither tables
	Random mRandom;		// maxsdk pseudo-random number generator

public:
	BlendTable(  int tableX, int tableY, // table size requested
				 int nToBlend,			 // n images to blend
				 float	ditherAmt,		 // scalar on noise
				 BOOL normalizeTable,	 // make pixels sum to 1
				 TNoiseFn noise);		 // 0..1 noise function to use

	~BlendTable(){ delete[] mpTable; mpTable = NULL; }

	// the raw index functions do not tile, asserts if not in range
	// index to beginning of a layer
	int LayerIndex( int nImage ){
		DbgAssert( nImage >=0 && nImage < mnToBlend );
		return nImage * mTableSzXY;
	}
	
	// x,y index within a layer....add to layerIndex
	int PixelIndex( int x, int y ){
		DbgAssert( x >=0 && x < mTableSzX && y >=0 && y < mTableSzY );
		return y * mTableSzX + x;
	}

	// 0,y index within a layer....add to layerIndex
	int RowIndex( int y ){
		DbgAssert( y >= 0 && y < mTableSzY );
		return y * mTableSzX;
	}

	// these functions wrap a coordinate around the tile sz x or y, 
	// return index on a tile given a general x,y
	int TileX( int x ){
		return x % mTableSzX;
	}
	int TileY( int y ){
		return y % mTableSzY;
	}
	
	// user level index call, wraps to tile...
	int Index( int nImage, int x, int y ){
		//wrap to the tile
		int x0 = TileX(x);	
		int y0 = TileY(y);
		int indx = LayerIndex( nImage ) + PixelIndex( x0, y0 );
		return indx;
	}

	// get the table pointer, do your own indexing....
	float* GetTable(){ return mpTable; }

	// slow, but they do it all
	float GetWeight( int nImage, int x, int y ){
		int indx = Index( nImage, x, y );
		return mpTable[ indx ];
	}
	void SetWeight( int nImage, int x, int y, float w ){
		int indx = Index( nImage, x, y );
		mpTable[ indx ] = w;
	}

	// returns tile size of the table
	int GetTileSz( int& xSz, int& ySz ){
		xSz = mTableSzX; ySz = mTableSzY;
	}

	// blend images via the table
	void BlendImages( Bitmap* pDestBM, Bitmap* pSrcBM, int nImage );
};


#endif

// ------------------------
// end of file blendTable.h
// ------------------------
