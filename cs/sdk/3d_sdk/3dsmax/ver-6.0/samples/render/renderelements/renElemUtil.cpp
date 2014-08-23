//////////////////////////////////////////////////////////////
//
//	Render element utilities
//
#include "renElemPch.h"
#include "renElemUtil.h"
#include <toneop.h>

// returns shadow fraction & shadowClr
// move this util to shade context at first opportunity
float IllumShadow( ShadeContext& sc, Color& shadowClr ) 
{ 
	IlluminateComponents illumComp;
	IIlluminationComponents* pIComponents;	
	Color illumClr(0,0,0);
	Color illumClrNS(0,0,0);
	shadowClr.Black();
	Point3 N = sc.Normal();
	
	for (int i = 0; i < sc.nLights; i++) {
		LightDesc* l = sc.Light( i );
		pIComponents = (IIlluminationComponents*)l->GetInterface( IID_IIlluminationComponents );
		if( pIComponents ){
			// use component wise illuminate routines
			if (!pIComponents->Illuminate( sc, N, illumComp ))
				continue;

			illumClr += (illumComp.finalColor - illumComp.shadowColor ) * illumComp.geometricAtten;
			illumClrNS += illumComp.finalColorNS * illumComp.geometricAtten;

			if( illumComp.rawColor != illumComp.filteredColor ){
				// light is filtered by a transparent object, sum both filter & user shadow color
				shadowClr += illumComp.finalColor * illumComp.geometricAtten; //attenuated filterColor 
			} else {
				// no transparency, sum in just the shadow color
				shadowClr += illumComp.shadowColor * illumComp.geometricAtten;
			}

		} else {
			// no component interface, shadow clr is black
			Color lightCol;
			Point3 L;
			register float NL, diffCoef;
			if( sc.shadow ){
				sc.shadow = FALSE;
				BOOL illum = l->Illuminate(sc, N, lightCol, L, NL, diffCoef);
				sc.shadow = TRUE;
				if (!illum)
					continue;
				if (diffCoef <= 0.0f)	  
					continue;
				illumClrNS += diffCoef * lightCol;
			}

			if (!l->Illuminate(sc, N, lightCol, L, NL, diffCoef))
				continue;
			if (diffCoef <= 0.0f)	  
				continue;
			lightCol *= diffCoef;
			illumClr += lightCol;

			if( !sc.shadow ){
				illumClrNS += lightCol;
			}
		}
	}// end, for each light

	if (sc.globContext != NULL && sc.globContext->pToneOp != NULL
			&& sc.globContext->pToneOp->Active(sc.CurTime())) {
		sc.globContext->pToneOp->ScaledToRGB(illumClrNS);
		sc.globContext->pToneOp->ScaledToRGB(illumClr);
	}

	float intensNS = Intens(illumClrNS);
//	Clamp( intensNS );
	if( intensNS < 0.0f ) intensNS = 0.0f;
	float intens = Intens(illumClr);
//	Clamp( intens );
	if( intens < 0.0f ) intens = 0.0f;
	float atten = (intensNS > 0.005f)? intens/intensNS : 1.0f;

	// correction for negative lights
	if (atten > 1.0f)
		atten = 1.0f/atten;

	return atten;
}

///////////////////////////////////////////////////////////////////////
//
//	Compute Illuminance - utility to compute illuminance for light maps
//
Color computeIlluminance( ShadeContext& sc, BOOL ambOn, BOOL diffOn )
{
	Color illumClr(0,0,0);
	Point3 N = sc.Normal();

	// for each light
	for (int i = 0; i < sc.nLights; i++) {

		LightDesc* l = sc.Light( i );

		Color lightCol;
		Point3 L;
		register float NL, diffCoef;
		// get illumination from the light
		if (!l->Illuminate(sc, N, lightCol, L, NL, diffCoef))
			continue;

		// ambient
		if ( l->ambientOnly ){
			// all ambient only lights thru this path, if ambOn add in color
			if( ambOn )
				illumClr += lightCol;
			continue;	// no diffuse or specular components on these lights
		}

		// diffuse
		if (diffCoef <= 0.0f)	  
			continue;

		if (diffOn && l->affectDiffuse)
			illumClr += diffCoef * lightCol;

		// specular doesn't make sense, no directions 
//		if (specOn && l->affectSpecular) {
//			illumClr += lightCol;
//		}

	}// end, for each light

	return illumClr;
}