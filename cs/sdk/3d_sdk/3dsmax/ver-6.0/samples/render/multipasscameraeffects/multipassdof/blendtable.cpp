/* -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

   FILE: blendTable.cpp

	 DESCRIPTION: definitions for class BlendTable

	 CREATED BY: kells elmquist (kae)

	 HISTORY: created July 17, 2000

   	 Copyright (c) 2000, All Rights Reserved

// -----------------------------------------------------------------------------
// -------------------------------------------------------------------------- */

/*
// system includes
#include <time.h>
*/

// maxsdk includes
#include "max.h"
#include "bmmlib.h"

// local includes
#include "blendTable.h"

const int NUM_SUMS(10);


// ------------------------------
// ------------------------------
// definitions of noise functions
// ------------------------------
// ------------------------------
float dummyNoise(Random &random)
{
	// to be done ...
	return 0.5f;
}

float RandomNoise(Random &random)
{
/*
//	srand( (unsigned)time( NULL ) );
	float noiseVal = rand() / float(RAND_MAX);
	return noiseVal;
*/
	return random.getf();
}

float SumNoise(Random &random)
{
	float noiseVal(0.0f);
	for (int i=0; i<NUM_SUMS; i++)
	{
		noiseVal += RandomNoise(random);
	}
	return noiseVal / NUM_SUMS;
}

float CRCNoise(Random &random)
{
	// to be done ...
	return 0.5f;
}

float GaussNoise(Random &random)
{
	// to be done ...
	return 0.5f;
}


// ---------------------------------------
// ---------------------------------------
// method definitions for class BlendTable
// ---------------------------------------
// ---------------------------------------
BlendTable::BlendTable( int tableX, int tableY,	// table size requested
				 int nToBlend,					// n images to blend
				 float	ditherAmt,				// scalar on noise
				 BOOL normalizeTable,			// make pixels sum to 1
				 TNoiseFn noise )				// pointer to noise function
{
	// start w/ the size of each image layer of the table
	mTableSzX = (tableX <= 0) ? 1 : tableX;
	mTableSzY = (tableY <= 0) ? 1 : tableY;;
	mTableSzXY = mTableSzX * mTableSzY;

	// then get the total table size
	mnToBlend = nToBlend;
	mTotalSz = mTableSzXY * mnToBlend;
	DbgAssert( mTotalSz > 0 );

	// alloc the 3D table, nImages of tableX x tableY layers
	// we have to manage the indices ourselves
	mpTable = new float[ mTotalSz ];

	// the initial weight for each image is 1/nToBlend; 10 images, each 1/10th
	float initWeight = 1.0f / ((mnToBlend == 0) ? 1.0f : float(mnToBlend) );

	// noise function is 0..1, so noiseAmplitude scales the 0..1 range
	// such that at ditherAmt == 1.0, the ampitude of the noise is the
	// same as the initial weight
	float noiseAmplitude = initWeight * ditherAmt;

	// fill in the table, just go thru the table linearly, not xy specific
	// worry about normalizing things later...
	for( int i = 0; i < mTotalSz; ++i ){
		// compute weight for this sample. note we shift the 0..1 noise
		// to be -0.5 to +0.5 prior to scaling
		float noiseWeight = noiseAmplitude * ( noise(mRandom) - 0.5f );
		float weight = initWeight + noiseWeight;

		// i think it's possible things cd be out of range, since ditherAmt
		// is not bounded
		mpTable[ i ] = Bound( weight, 0.0f, 1.0f );
	}

	// see if we need to normalize the table
	if( normalizeTable ){
		// yes , normalize it

		// for each pixel in the table....
		for( int y = 0; y < mTableSzY; ++y ){
			for( int x = 0; x < mTableSzX; ++x ){
				// useful to pre compute this, since it's 
				// fixed as we scan the layers 
				int pixelIndex = PixelIndex( x, y );

				// find the sum of all the layers thru one pixel
				float sum = 0.0f;
				for( int n = 0; n < mnToBlend; ++n ){
					// compute final index by adding in layer
					int index = LayerIndex(n) + pixelIndex;
					// add this layer to the sum
					sum += mpTable[ index ];
				}

				// now normalize...make the sum == 1.0
				float norm = 1.0f / ((sum == 0.0f)? 1.0f : sum );

				for( n = 0; n < mnToBlend; ++n ){
					int index = LayerIndex(n) + pixelIndex;
					// scale this layer by the norm
					mpTable[ index ] *= norm;
				}
			} // end, for each pixel of a line
		} // end, for each line
	} // end, normalize
}


void
BlendTable::BlendImages( Bitmap* pDstBM, Bitmap* pSrcBM, int nImage )
{
	// get xSz & ySz
	int xSz = pDstBM->Width();
	int ySz = pDstBM->Height();

	// check for same sz
	DbgAssert( xSz == pSrcBM->Width() && ySz == pSrcBM->Height() );

	// index at (nImage,0,0)
	int layerIndex = LayerIndex(nImage);

	// get line buffers
	Color64U *pOut = new Color64U[ xSz ];
	Color64U *pIn = new Color64U[ xSz ];

	// for each line of the image...
	for( int nLine = 0; nLine < ySz; ++nLine ){

		// get src & dst lines
		pDstBM->GetPixels( 0, nLine, xSz, (BMM_Color_64*)(pOut) );
		pSrcBM->GetPixels( 0, nLine, xSz, (BMM_Color_64*)(pIn) );

		// get the row index of the table, the index at (0, nLine)
		int rowIndex = RowIndex( TileY( nLine ) ) + layerIndex;

		// multiply line of src pixels * weights from table
		// & sum into dst
		for( int nPix = 0; nPix < xSz; ++nPix ){
			float w =  mpTable[ rowIndex + TileX( nPix ) ];
			pIn[ nPix ] *= w;
			// Now one of: pOut[ nPix ].AddSafe( pIn[ nPix ] );
			// Or: pOut[ nPix ] += pIn[ nPix ];
			if( nImage < (mnToBlend - 2) )
				pOut[ nPix ] += pIn[ nPix ];
			else
				pOut[ nPix ].AddSafe( pIn[ nPix ] ); // last 2, use safe add
		}

		// & write back to the frame buffer
		pDstBM->PutPixels(0, nLine, xSz, (BMM_Color_64*)pOut );

	} // end, for each line

	// free line buffers
	delete[] pIn;
	delete[] pOut;
}

// --------------------------
// end of file blendTable.cpp
// --------------------------
