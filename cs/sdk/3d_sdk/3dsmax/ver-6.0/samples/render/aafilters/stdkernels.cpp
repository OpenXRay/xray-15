/**********************************************************************
 *<
	FILE: StdKernels.cpp	

	DESCRIPTION:Standard prefilter kernel plugins

	CREATED BY: Kells Elmquist

	HISTORY:	Created 7/29/98

 *>	Copyright (c) 1998, All Rights Reserved.
 **********************************************************************/

#include "kernelhdr.h"
#include "kernelres.h"
#include "iparamm.h"

#define M_PI	3.141592654

#define LOW_THRESH	0.5f	// halfpixel radius minimum


// Class IDs for kernel plug-ins
#define STD_KERNEL_CLASS_ID				0x77912300
//#define AREA_KERNEL_CLASS_ID			0x77912301
#define CYLINDER_KERNEL_CLASS_ID		0x77912302
#define CONE_KERNEL_CLASS_ID			0x77912303
#define QUADRATIC_KERNEL_CLASS_ID		0x77912304
#define CUBIC_KERNEL_CLASS_ID			0x77912305
#define CATROM_KERNEL_CLASS_ID			0x77912306
#define GAUSS_NARROW_KERNEL_CLASS_ID	0x77912307
#define GAUSS_MEDIUM_KERNEL_CLASS_ID	0x77912308
#define GAUSS_WIDE_KERNEL_CLASS_ID	    0x77912309
#define PAVIC_UNITVOL_KERNEL_CLASS_ID   0x7791230a
#define PAVIC_OPTIMIZED_KERNEL_CLASS_ID 0x7791230b
#define HANNING_KERNEL_CLASS_ID			0x7791230d
#define HAMMING_KERNEL_CLASS_ID			0x7791230e
#define BLACKMAN_KERNEL_CLASS_ID		0x7791230f
#define BLACKMAN367_KERNEL_CLASS_ID		0x77912310
#define BLACKMAN361_KERNEL_CLASS_ID		0x77912311
#define BLACKMAN492_KERNEL_CLASS_ID		0x77912312
#define BLACKMAN474_KERNEL_CLASS_ID		0x77912313
#define MAX1_KERNEL_CLASS_ID			0x77912314
#define MAX2_KERNEL_CLASS_ID			0x77912315
#define MAX3_KERNEL_CLASS_ID			0x77912316
#define MITNET_OP_KERNEL_CLASS_ID		0x77912317
#define MITNET_NOTCH_KERNEL_CLASS_ID	0x77912318
#define NTSC_KERNEL_CLASS_ID			0x77912319
#define COOK_OP_KERNEL_CLASS_ID			0x7791231a
#define PIXELSIZE_KERNEL_CLASS_ID		0x7791231b

// variable width filters
#define COOK_VAR_KERNEL_CLASS_ID		0x77912330
#define GAUSS_VAR_KERNEL_CLASS_ID		0x77912331
#define CYLINDER_VAR_KERNEL_CLASS_ID	0x77912332
#define CONE_VAR_KERNEL_CLASS_ID		0x77912333
#define QUADRATIC_VAR_KERNEL_CLASS_ID	0x77912334
#define CUBIC_VAR_KERNEL_CLASS_ID		0x77912335

// parameterized filters
#define MITNET_VAR_KERNEL_CLASS_ID		0x77912350
#define STOCKING_KERNEL_CLASS_ID		0x77912351
#define BLEND_KERNEL_CLASS_ID			0x77912352

// ClassID statics
Class_ID StdKernelClassID(STD_KERNEL_CLASS_ID,0);
Class_ID AreaKernelClassID(AREA_KERNEL_CLASS_ID,0);
Class_ID ConeKernelClassID(CONE_KERNEL_CLASS_ID,0);
Class_ID QuadraticKernelClassID(QUADRATIC_KERNEL_CLASS_ID,0);
Class_ID CubicKernelClassID(CUBIC_KERNEL_CLASS_ID,0);
Class_ID CatRomKernelClassID(CATROM_KERNEL_CLASS_ID,0);
Class_ID GaussNarrowKernelClassID(GAUSS_NARROW_KERNEL_CLASS_ID,0);
Class_ID GaussMediumKernelClassID(GAUSS_MEDIUM_KERNEL_CLASS_ID,0);
Class_ID GaussWideKernelClassID(GAUSS_WIDE_KERNEL_CLASS_ID,0);
Class_ID PavicUnitVolKernelClassID(PAVIC_UNITVOL_KERNEL_CLASS_ID,0);
Class_ID PavicOpKernelClassID(PAVIC_OPTIMIZED_KERNEL_CLASS_ID,0);
Class_ID HanningKernelClassID(HANNING_KERNEL_CLASS_ID,0);
Class_ID HammingKernelClassID(HAMMING_KERNEL_CLASS_ID,0);
Class_ID BlackmanKernelClassID(BLACKMAN_KERNEL_CLASS_ID,0);
Class_ID Blackman367KernelClassID(BLACKMAN367_KERNEL_CLASS_ID,0);
Class_ID Blackman361KernelClassID(BLACKMAN361_KERNEL_CLASS_ID,0);
Class_ID Blackman474KernelClassID(BLACKMAN474_KERNEL_CLASS_ID,0);
Class_ID Blackman492KernelClassID(BLACKMAN492_KERNEL_CLASS_ID,0);
Class_ID Max1KernelClassID(MAX1_KERNEL_CLASS_ID,0);
Class_ID Max2KernelClassID(MAX2_KERNEL_CLASS_ID,0);
Class_ID Max3KernelClassID(MAX3_KERNEL_CLASS_ID,0);
Class_ID MitNetOpKernelClassID(MITNET_OP_KERNEL_CLASS_ID,0);
Class_ID MitNetNotchKernelClassID(MITNET_NOTCH_KERNEL_CLASS_ID,0);
Class_ID NTSCKernelClassID(NTSC_KERNEL_CLASS_ID,0);
Class_ID CookOpKernelClassID(COOK_OP_KERNEL_CLASS_ID,0);
Class_ID PixelSizeKernelClassID(PIXELSIZE_KERNEL_CLASS_ID,0);


// classes with parameters
Class_ID CookVarKernelClassID(COOK_VAR_KERNEL_CLASS_ID,0);
Class_ID MitNetVarKernelClassID(MITNET_VAR_KERNEL_CLASS_ID,0);
Class_ID GaussVarKernelClassID(GAUSS_VAR_KERNEL_CLASS_ID,0);
Class_ID CylinderVarKernelClassID(CYLINDER_VAR_KERNEL_CLASS_ID,0);
Class_ID ConeVarKernelClassID(CONE_VAR_KERNEL_CLASS_ID,0);
Class_ID QuadraticVarKernelClassID(QUADRATIC_VAR_KERNEL_CLASS_ID,0);
Class_ID CubicVarKernelClassID(CUBIC_VAR_KERNEL_CLASS_ID,0);
Class_ID StockingKernelClassID(STOCKING_KERNEL_CLASS_ID,0);
Class_ID BlendKernelClassID(BLEND_KERNEL_CLASS_ID,0);

// ClassNames
//#define AREA_KERNEL_CLASSNAME	GetString(IDS_KE_AREA)

//-------------------------------------------------------

//class GenericDlgProc;

// global generic do nothing dlg proc
//static GenericDlgProc *pGenericDlgProc = NULL;

// Generic filter kernel class to handle most of the stuff
class GenericKernel: public FilterKernel {
	public:
		GenericKernel(){};
		~GenericKernel(){};

		RefTargetHandle Clone(RemapDir& remap) 
		{ 
			GenericKernel *copy = new GenericKernel;
			BaseClone(this, copy, remap);
			return copy;	
		}	

		RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
									PartID& partID, RefMessage message ) 
		{ 
			switch (message) {
				case REFMSG_TARGET_DELETED:
					break;
			}							
		   	return REF_SUCCEED; 
		}

		// From Animatable
		void *GetInterface(ULONG id) {  return NULL; }
		void DeleteThis() { delete this; }
		int NumSubs() {return 0;}
		Animatable* SubAnim(int i){ return NULL; }
		TSTR SubAnimName(int i){ return _T( "" ); }
		virtual TCHAR *GetObjectName() { return NULL; } // dummy, overridden by subclass
		void GetClassName(TSTR& s) { s = GetObjectName(); };
		TSTR GetName() { TSTR s = GetObjectName(); return s; };
//		Class_ID ClassID() { return standinClassID; }

		// From ref
		int NumRefs() { return 0; }
		RefTargetHandle GetReference(int i) { return NULL; }
		void SetReference(int i, RefTargetHandle rtarg) {}
		SClass_ID  	SuperClassID() { return FILTER_KERNEL_CLASS_ID; }

//		FilterKernelParamDlg *CreateParamDialog(IRendParams *ip) {
//			return NULL; //new GenericKernelParamDlg(this,ip);
//		}
		TCHAR * GetDefaultComment() { return GetString( IDS_KE_BLANK); }

		// there are 2 optional 0...1 parameters
		virtual long GetNFilterParams() { return 0; }
		virtual TCHAR * GetFilterParamName( long nParam ) { return GetString( IDS_KE_BLANK ); }
		virtual double GetFilterParam( long nParam ){ return 0.0; }
		virtual void SetFilterParam( long nParam, double val ){}

		// for 2d returns y support, for 1d returns 0
		long GetKernelSupportY(){ return 0; };

		bool Is2DKernel(){ return FALSE; };
		bool IsVariableSz(){ return FALSE; };
		// 1-D filters ignore the y parameter, r = x/2
		void SetKernelSz( double x, double y = 0.0 ) {};
		// return 2...the most common case
		void GetKernelSz( double& x, double& y ){ x = 2.0; y = 0.0; }

		// returning true will disable the built-in normalizer
		bool IsNormalized(){ return FALSE; };

		// this is for alpha enables
		bool HasNegativeLobes(){ return FALSE; };

		IOResult Load (ILoad *iload){ SpecialFX::Load (iload);	return IO_OK; }
		IOResult Save (ISave *isave){ SpecialFX::Save(isave); return IO_OK; }

		// just dummies so we can instance the abstract class
		virtual long GetKernelSupport() { return 0; };
		virtual double KernelFn( double x, double y ) {return 0.0;}

};

//---------------------------- the filter kernel classes -----------------------------------
class AreaFilterKernel : public GenericKernel {
public:
		double w;
		AreaFilterKernel(){ w = 0.75; }
		RefTargetHandle Clone(RemapDir& remap)
		{ AreaFilterKernel* a = new AreaFilterKernel(); a->w = w; BaseClone(this, a, remap); return a; }	
		Class_ID ClassID()      { return AreaKernelClassID; }
		TCHAR *GetObjectName()  { return GetString(IDS_KE_AREA_KERNEL); }
		TCHAR * GetDefaultComment() { return GetString(IDS_KE_AREA_COMMENT); }
		long GetKernelSupport() { return long( w + 0.5 ); }
		bool IsVariableSz(){ return TRUE; };
		void SetKernelSz( double x, double y = 0.0 ) { w = (x < LOW_THRESH)? LOW_THRESH : x; };
		void GetKernelSz( double& x, double& y ){ x = w; y = 0.0; }
		double KernelFn( double x, double ) {
			if ( x > w ) return 0.0;
			return 1.0;
		}
};

//////////////////////////////////////////////////////////////////////////////
//
//	 PixelsSize filter, for 2.5 compatibility & camera mapping
//
#ifdef INCLUDE_PIXELSIZE 

#define PIXELSIZE_MAX	1.0f

class PixelSizeKernel : public GenericKernel {
public:
		double w;
		PixelSizeKernel(){ w = 0.75; }
		RefTargetHandle Clone(RemapDir& remap)
		{ PixelSizeKernel* a = new PixelSizeKernel(); a->w = w; BaseClone(this, a, remap); return a; }	
		Class_ID ClassID()      { return PixelSizeKernelClassID; }
		TCHAR *GetObjectName()  { return GetString(IDS_KE_PIXELSIZE_KERNEL); }
		TCHAR * GetDefaultComment() { return GetString(IDS_KE_PIXELSIZE_COMMENT); }
		long GetKernelSupport() { return 0; }
		bool IsVariableSz(){ return TRUE; };

		void SetKernelSz( double x, double y = 0.0 ) { w = (x < LOW_THRESH)? LOW_THRESH : (x > PIXELSIZE_MAX)? PIXELSIZE_MAX : x; };
		void GetKernelSz( double& x, double& y ){ x = w; y = -1.0; } // the flag
		double KernelFn( double x, double ) { return 0.0; } // dummy
};
#endif /////////////////////////////////////////////////////////////////////////


class ConeFilterKernel : public GenericKernel {
public:
		Class_ID ClassID()      { return ConeKernelClassID; }
		TCHAR *GetObjectName()  { return GetString(IDS_KE_CONE_KERNEL); }
//		TCHAR * GetDefaultComment() { return GetString(IDS_KE_CONE_COMMENT); }
		long GetKernelSupport() { return 1; }
		void GetKernelSz( double& x, double& y ){ x = 1.0; y = 0.0; }
		RefTargetHandle Clone(RemapDir& remap)
		{ ConeFilterKernel* a = new ConeFilterKernel(); BaseClone(this, a, remap); return a; }	
		double KernelFn( double x, double y ) {return ( x < 1.0f ) ? 1.0f - x : 0.0;}
};

class QuadraticFilterKernel : public GenericKernel {
public:
		Class_ID ClassID()      { return QuadraticKernelClassID; }
		TCHAR *GetObjectName()  { return GetString(IDS_KE_QUADRATIC_KERNEL); }
		TCHAR * GetDefaultComment() { return GetString(IDS_KE_QUAD_COMMENT); }
		RefTargetHandle Clone(RemapDir& remap)
		{ QuadraticFilterKernel* a = new QuadraticFilterKernel(); BaseClone(this, a, remap); return a; }	
		long GetKernelSupport() { return 1; };
		void GetKernelSz( double& x, double& y ){ x = 1.5; y = 0.0; }
		double KernelFn( double x, double ) {
			if ( x < 0.5 ) return 0.75 - x * x;
			if ( x < 1.5 ) {
				double t = x - 1.5; 
				return 0.5 * t * t;
			}
			return 0.0f;
		}
};

class CubicFilterKernel : public GenericKernel {
public:
		Class_ID ClassID()      { return CubicKernelClassID; }
		TCHAR *GetObjectName()  { return GetString(IDS_KE_CUBIC_KERNEL); }
		TCHAR * GetDefaultComment() { return GetString(IDS_KE_CUBIC_COMMENT); }
		long GetKernelSupport() { return 2; };
		RefTargetHandle Clone(RemapDir& remap)
		{ CubicFilterKernel* a = new CubicFilterKernel(); BaseClone(this, a, remap); return a; }	
		double KernelFn( double x, double ) {
			if ( x < 1.0 ) return (4.0 + x * x * (-6.0 + x * 3.0))/6.0;
			if ( x < 2.0 ) { 
				double t = 2.0 - x; 
				return t * t * t / 6.0; 
			}
			return 0.0;
		}
};

class CatRomFilterKernel : public GenericKernel {
public:
		Class_ID ClassID()      { return CatRomKernelClassID; }
		TCHAR *GetObjectName()  { return GetString(IDS_KE_CATROM_KERNEL); }
		TCHAR * GetDefaultComment() { return GetString(IDS_KE_CATROM_COMMENT); }
		long GetKernelSupport() { return 2; };
		bool HasNegativeLobes(){ return TRUE; };
		RefTargetHandle Clone(RemapDir& remap)
		{ CatRomFilterKernel* a = new CatRomFilterKernel(); BaseClone(this, a, remap); return a; }	
		double KernelFn( double x, double ) {
			if ( x < 1.0 ) return 0.5 * ( 2.0 + x * x *( -5.0 + x * 3.0));
			if ( x < 2.0 ) return 0.5 * ( 4.0 + x * ( -8.0 + x * (5.0 - x )));
			return 0.0;
		}
};

class GaussNarrowFilterKernel : public GenericKernel {
public:
		Class_ID ClassID()      { return GaussNarrowKernelClassID; }
		TCHAR *GetObjectName()  { return GetString(IDS_KE_GAUSS_NARROW_KERNEL); }
		long GetKernelSupport() { return 1; };
		void GetKernelSz( double& x, double& y ){ x = 1.25; y = 0.0; }
		RefTargetHandle Clone(RemapDir& remap)
		{ GaussNarrowFilterKernel* a = new GaussNarrowFilterKernel(); BaseClone(this, a, remap); return a; }	
		double KernelFn( double x, double ) {
			const double sqrt2overPi = sqrt( 2.0 / M_PI );
			return exp( -2.0 * x * x ) * sqrt2overPi ;
		}
};

class GaussMediumFilterKernel : public GenericKernel {
public:
		Class_ID ClassID()      { return GaussMediumKernelClassID; }
		TCHAR *GetObjectName()  { return GetString(IDS_KE_GAUSS_MEDIUM_KERNEL); }
		long GetKernelSupport() { return 1; };
		void GetKernelSz( double& x, double& y ){ x = 1.5; y = 0.0; }
		RefTargetHandle Clone(RemapDir& remap)
		{ GaussMediumFilterKernel* a = new GaussMediumFilterKernel(); BaseClone(this, a, remap); return a; }	
		double KernelFn( double x, double ) {
			return pow( 2.0, -2.0 * x * x );
		}
};

class GaussWideFilterKernel : public GenericKernel {
public:
		Class_ID ClassID()      { return GaussWideKernelClassID; }
		TCHAR *GetObjectName()  { return GetString(IDS_KE_GAUSS_WIDE_KERNEL); }
		long GetKernelSupport() { return 2; };
		RefTargetHandle Clone(RemapDir& remap)
		{ GaussWideFilterKernel* a = new GaussWideFilterKernel(); BaseClone(this, a, remap); return a; }	
		double KernelFn( double x, double ) {
			return pow( 2.0, -4.0 * x * x );
		}
};

class PavicUnitVolFilterKernel : public GenericKernel {
public:
		Class_ID ClassID()      { return PavicUnitVolKernelClassID; }
		TCHAR *GetObjectName()  { return GetString(IDS_KE_PAVIC_UNITVOL_KERNEL); }
		long GetKernelSupport() { return 1; };
		void GetKernelSz( double& x, double& y ){ x = 1.0; y = 0.0; }
		RefTargetHandle Clone(RemapDir& remap)
		{ PavicUnitVolFilterKernel* a = new PavicUnitVolFilterKernel(); BaseClone(this, a, remap); return a; }	
		double KernelFn( double x, double ) {
			if ( x > 1.0 ) return 0.0;
			const double beta = -2.378;
			return ( 1.0 - (1.0 - exp( beta * x * x )) / (1.0 - exp( beta )) );
		}
};

class PavicOpFilterKernel : public GenericKernel {
public:
		Class_ID ClassID()      { return PavicOpKernelClassID; }
		TCHAR *GetObjectName()  { return GetString(IDS_KE_PAVIC_OP_KERNEL); }
		long GetKernelSupport() { return 1; };
		void GetKernelSz( double& x, double& y ){ x = 1.0; y = 0.0; }
		RefTargetHandle Clone(RemapDir& remap)
		{ PavicOpFilterKernel* a = new PavicOpFilterKernel(); BaseClone(this, a, remap); return a; }	
		double KernelFn( double x, double ) {
			if ( x > 1.0 ) return 0.0;
			double alpha = 0.918;
			double beta = -1.953;
			return alpha * (1.0 - (1.0 - exp( beta * x * x )) / (1.0 - exp( beta )) );
		}
};

class HanningFilterKernel : public GenericKernel {
public:
		Class_ID ClassID()      { return HanningKernelClassID; }
		TCHAR *GetObjectName()  { return GetString(IDS_KE_HANNING_KERNEL); }
		TCHAR * GetDefaultComment() { return GetString(IDS_KE_HANNING_COMMENT); }
		long GetKernelSupport() { return 2; };
		bool HasNegativeLobes(){ return TRUE; };
		RefTargetHandle Clone(RemapDir& remap)
		{ HanningFilterKernel* a = new HanningFilterKernel(); BaseClone(this, a, remap); return a; }	
		double KernelFn( double x, double ) {
			if ( x >= 2.0 ) return 0.0;
			double s = M_PI * x;
			double sincX = ( s == 0.0 ) ? 1.0 : sin( s ) / s;
			const double a = 0.5;
			const double b = 0.5;
			double t = M_PI * 0.5 * ( 2.0 - x );
			return (sincX * ( a - b * cos( t ) ));
		}
};

class HammingFilterKernel : public GenericKernel {
public:
		Class_ID ClassID()      { return HammingKernelClassID; }
		TCHAR *GetObjectName()  { return GetString(IDS_KE_HAMMING_KERNEL); }
		long GetKernelSupport() { return 2; };
		bool HasNegativeLobes(){ return TRUE; };
		RefTargetHandle Clone(RemapDir& remap)
		{ HammingFilterKernel* a = new HammingFilterKernel(); BaseClone(this, a, remap); return a; }	
		double KernelFn( double x, double ) {
			if ( x >= 2.0 ) return 0.0;
			double s = M_PI * x;
			double sincX = ( s == 0.0 ) ? 1.0 : sin( s ) / s;
			const double a = 0.54;
			const double b = 0.46;
			double t = M_PI * 0.5 * ( 2.0 - x );
			// want cos( pi ) at x == 0, cos( 0 or 2pi ) at x == 2
			// cos( pi * t ) where t = (2 - r)/2 
			return sincX * ( a - b * cos( t ) );
		}
};

class BlackmanFilterKernel : public GenericKernel {
public:
		Class_ID ClassID()      { return BlackmanKernelClassID; }
		TCHAR *GetObjectName()  { return GetString(IDS_KE_BLACKMAN_KERNEL); }
		bool HasNegativeLobes(){ return TRUE; };
		long GetKernelSupport() { return 2; };
		RefTargetHandle Clone(RemapDir& remap)
		{ BlackmanFilterKernel* a = new BlackmanFilterKernel(); BaseClone(this, a, remap); return a; }	
		double KernelFn( double x, double ) {
			if ( x >= 2.0 ) return 0.0;
			double s = M_PI * x;
			double sincX = ( s == 0.0 ) ? 1.0 : sin( s ) / s;
			const double a = 0.42;
			const double b = 0.5;
			const double c = 0.08;
			double t = M_PI * 0.5 * ( 2.0 - x );
			return sincX * ( a - b * cos(  t ) + c * cos( 2.0 * t ) );
		}
};

class Blackman361FilterKernel : public GenericKernel {
public:
		Class_ID ClassID()      { return Blackman361KernelClassID; }
		TCHAR *GetObjectName()  { return GetString(IDS_KE_BLACKMAN361_KERNEL); }
		long GetKernelSupport() { return 2; };
		bool HasNegativeLobes(){ return TRUE; };
		RefTargetHandle Clone(RemapDir& remap)
		{ Blackman361FilterKernel* a = new Blackman361FilterKernel(); BaseClone(this, a, remap); return a; }	
		double KernelFn( double x, double ) {
			if ( x >= 2.0 ) return 0.0;
			double s = M_PI * x;
			double sincX = ( s == 0.0 ) ? 1.0 : sin( s ) / s;
			const double a = 0.44959;
			const double b = 0.49364;
			const double c = 0.05677;
			double t = M_PI * 0.5 * ( 2.0 - x );
			return sincX * ( a - b * cos(  t ) + c * cos( 2.0 * t ) );
		}
};

class Blackman367FilterKernel : public GenericKernel {
public:
		Class_ID ClassID()      { return Blackman367KernelClassID; }
		TCHAR *GetObjectName()  { return GetString(IDS_KE_BLACKMAN367_KERNEL); }
		bool HasNegativeLobes(){ return TRUE; };
		long GetKernelSupport() { return 2; };
		RefTargetHandle Clone(RemapDir& remap)
		{ Blackman367FilterKernel* a = new Blackman367FilterKernel(); BaseClone(this, a, remap); return a; }	
		double KernelFn( double x, double ) {
			if ( x >= 2.0 ) return 0.0;
			double s = M_PI * x;
			double sincX = ( s == 0.0 ) ? 1.0 : sin( s ) / s;
			const double a = 0.42323;
			const double b = 0.49755;
			const double c = 0.07922;
			double t = M_PI * 0.5 * ( 2.0 - x );
			return sincX * ( a - b * cos(  t ) + c * cos( 2.0 * t ) );
		}
};

class Blackman474FilterKernel : public GenericKernel {
public:
		Class_ID ClassID()      { return Blackman474KernelClassID; }
		TCHAR *GetObjectName()  { return GetString(IDS_KE_BLACKMAN474_KERNEL); }
		TCHAR * GetDefaultComment() { return GetString(IDS_KE_BLACKMAN_COMMENT); }
		bool HasNegativeLobes(){ return TRUE; };
		long GetKernelSupport() { return 2; };
		RefTargetHandle Clone(RemapDir& remap)
		{ Blackman474FilterKernel* a = new Blackman474FilterKernel(); BaseClone(this, a, remap); return a; }	
		double KernelFn( double x, double ) {
			if ( x >= 2.0 ) return 0.0;
			double s = M_PI * x;
			double sincX = ( s == 0.0 ) ? 1.0 : sin( s ) / s;
			const double a = 0.40217;
			const double b = 0.49703;
			const double c = 0.09392;
			const double d = 0.00183;
			double t = M_PI * 0.5 * ( 2.0 - x );
			return sincX * ( a - b * cos( t ) + c * cos( 2.0 * t ) - d * cos( 4.0 * t ) );
		}
};

class Blackman492FilterKernel : public GenericKernel {
public:
		Class_ID ClassID()      { return Blackman492KernelClassID; }
		TCHAR *GetObjectName()  { return GetString(IDS_KE_BLACKMAN492_KERNEL); }
		long GetKernelSupport() { return 2; };
		bool HasNegativeLobes(){ return TRUE; };
		RefTargetHandle Clone(RemapDir& remap)
		{ Blackman492FilterKernel* a = new Blackman492FilterKernel(); BaseClone(this, a, remap); return a; }	
		double KernelFn( double x, double ) {
			if ( x >= 2.0 ) return 0.0;
			double s = M_PI * x;
			double sincX = ( s == 0.0 ) ? 1.0 : sin( s ) / s;
			const double a = 0.35875;
			const double b = 0.48829;
			const double c = 0.14128;
			const double d = 0.01168;
			double t = M_PI * 0.5 * ( 2.0 - x );
			return sincX * ( a - b * cos( t ) + c * cos( 2.0 * t ) - d * cos( 4.0 * t ) );
		}
};

class Max1FilterKernel : public GenericKernel {
public:
		Class_ID ClassID()      { return Max1KernelClassID; }
		TCHAR *GetObjectName()  { return GetString(IDS_KE_MAX1_KERNEL); }
		TCHAR * GetDefaultComment() { return GetString(IDS_KE_MAX_COMMENT); }
		long GetKernelSupport() { return 1; };
		void GetKernelSz( double& x, double& y ){ x = 1.4; y = 0.0; }
		RefTargetHandle Clone(RemapDir& remap)
		{ Max1FilterKernel* a = new Max1FilterKernel(); BaseClone(this, a, remap); return a; }	
		double KernelFn( double x, double ) {
			const double t = 1.3778;
			const double s = 0.4848;
			const double stInv = 1.0 / (s * t);
			const double ttms = t * (t - s);

			if ( x >= t ) return 0.0;
			if ( x > s ) 
				return (t - x) * (t - x) / ttms;
			return 1.0 - x * x * stInv;
		}
};

class Max2FilterKernel : public GenericKernel {
public:
		Class_ID ClassID()      { return Max2KernelClassID; }
		TCHAR *GetObjectName()  { return GetString(IDS_KE_MAX2_KERNEL); }
		long GetKernelSupport() { return 1; };
		void GetKernelSz( double& x, double& y ){ x = 1.4; y = 0.0; }
		RefTargetHandle Clone(RemapDir& remap)
		{ Max2FilterKernel* a = new Max2FilterKernel(); BaseClone(this, a, remap); return a; }	
		double KernelFn( double x, double ) {
			const double t = 1.3778;
			const double s = 0.4848;
			const double stInv = 1.0 / (s * t);
			const double ttms = t * (t - s);

			if ( x >= t ) return 0.0;
			if ( x > s ) 
				return (t - x) * (t - x) / ttms;
			return 1.0 - x * x * stInv;
		}
};

class Max3FilterKernel : public GenericKernel {
public:
		Class_ID ClassID()      { return Max3KernelClassID; }
		TCHAR *GetObjectName()  { return GetString(IDS_KE_MAX3_KERNEL); }
		long GetKernelSupport() { return 1; };
		void GetKernelSz( double& x, double& y ){ x = 1.3; y = 0.0; }
		RefTargetHandle Clone(RemapDir& remap)
		{ Max3FilterKernel* a = new Max3FilterKernel(); BaseClone(this, a, remap); return a; }	
		double KernelFn( double x, double y ) {
			const double t = 1.3682;
			const double s = 0.4792;
			const double stInv = 1.0 / (s * t);
			const double ttms = t * (t - s);

			if ( x >= t ) return 0.0;
			if ( x > s ) 
				return (t - x) * (t - x) / ttms;
			return 1.0 - x * x * stInv;
		}
};

class MitNetOpFilterKernel : public GenericKernel {
public:
		Class_ID ClassID()      { return MitNetOpKernelClassID; }
		TCHAR *GetObjectName()  { return GetString(IDS_KE_MITNET_OP_KERNEL); }
		TCHAR * GetDefaultComment() { return GetString(IDS_KE_MITNET_COMMENT); }
		bool HasNegativeLobes(){ return TRUE; };
		long GetKernelSupport() { return 2; };
		RefTargetHandle Clone(RemapDir& remap)
		{ MitNetOpFilterKernel* a = new MitNetOpFilterKernel(); BaseClone(this, a, remap); return a; }	
		double KernelFn( double x, double ) {
			const double b = 0.33333;
			const double c = 0.33333;

			if ( x >= 2.0 ) return 0.0;
			if ( x < 1.0 ) {
				// coeffs for x < 1, compute as const
				const double D0 = 6.0 - 2.0 * b ;  				// constant term
				const double C0 = 0.0;							// linear term
				const double B0 = -18.0 + 12.0 * b + 6.0 * c;	// quadratic term
				const double A0 = 12.0 - 9.0 * b - 6.0 * c ;	// cubic term
				return x * ( x * (A0 * x + B0) + C0 ) + D0;
			}
			// 1 < x < 2
			const double D1 = 8.0 * b + 24.0 * c ;  			// constant term
			const double C1 = -12.0 * b - 48.0 * c;				// linear term
			const double B1 = 6.0 * b + 30.0 * c;				// quadratic term
			const double A1 = - b - 6.0 * c ;					// cubic term
			return x * ( x * (A1 * x + B1) + C1 ) + D1;
		}
};

class NTSCFilterKernel : public GenericKernel {
public:
		Class_ID ClassID()      { return NTSCKernelClassID; }
		TCHAR *GetObjectName()  { return GetString(IDS_KE_NTSC_KERNEL); }
		TCHAR * GetDefaultComment() { return GetString(IDS_KE_NTSC_COMMENT); }
		long GetKernelSupport() { return 2; };
		RefTargetHandle Clone(RemapDir& remap)
		{ NTSCFilterKernel* a = new NTSCFilterKernel(); BaseClone(this, a, remap); return a; }	
		double KernelFn( double x, double ) {
			if ( x >= 2.0 ) return 0.0;
			if ( x < 1.0 )
				return -0.25 * x * x + 0.5;
			// 1 < x < 2
			return 0.25 * x * x  - x + 1.0;
		}
};

class MitNetNotchFilterKernel : public GenericKernel {
public:
		Class_ID ClassID()      { return MitNetNotchKernelClassID; }
		TCHAR *GetObjectName()  { return GetString(IDS_KE_MITNET_NOTCH_KERNEL); }
		long GetKernelSupport() { return 2; };
		bool HasNegativeLobes(){ return TRUE; };
		RefTargetHandle Clone(RemapDir& remap)
		{ MitNetNotchFilterKernel* a = new MitNetNotchFilterKernel(); BaseClone(this, a, remap); return a; }	
		double KernelFn( double x, double ) {
			const double b = 1.5;
			const double c = -0.25;

			if ( x >= 2.0 ) return 0.0;
			if ( x < 1.0 ) {
				// coeffs for x < 1, compute as const
				const double D0 = 6.0 - 2.0 * b ;  				// constant term
				const double C0 = 0.0;							// linear term
				const double B0 = -18.0 + 12.0 * b + 6.0 * c;	// quadratic term
				const double A0 = 12.0 - 9.0 * b - 6.0 * c ;	// cubic term
				return x * ( x * (A0 * x + B0) + C0 ) + D0;
			}
			// 1 < x < 2
			const double D1 = 8.0 * b + 24.0 * c ;  			// constant term
			const double C1 = -12.0 * b - 48.0 * c;				// linear term
			const double B1 = 6.0 * b + 30.0 * c;				// quadratic term
			const double A1 = - b - 6.0 * c ;					// cubic term
			return x * ( x * (A1 * x + B1) + C1 ) + D1;
		}
};

class CookOpFilterKernel : public GenericKernel {
public:
		Class_ID ClassID()      { return CookOpKernelClassID; }
		TCHAR *GetObjectName()  { return GetString(IDS_KE_COOK_OP_KERNEL); }
		long GetKernelSupport() { return 2; }
		void GetKernelSz( double& x, double& y ){ x = 1.5; y = 0.0; }
		RefTargetHandle Clone(RemapDir& remap)
		{ CookOpFilterKernel* a = new CookOpFilterKernel(); BaseClone(this, a, remap); return a; }	
		double KernelFn( double x, double ) {
			const double w = 1.5;
			if ( x > w ) return 0.0;
			const double expW2 = exp( -(w * w) );

			return exp( -(x * x) ) - expW2;
		}
};

///////////////////////////////////////////////////////////////////////////////////
//
//	Variable size filters
//

class CylinderVarFilterKernel : public GenericKernel {
	private:
		double	w;
	public:
		CylinderVarFilterKernel() : w(3.0) {}
		RefTargetHandle Clone(RemapDir& remap)
		{ CylinderVarFilterKernel* a = new CylinderVarFilterKernel(); a->w = w; BaseClone(this, a, remap); return a; }	
		Class_ID ClassID()      { return CylinderVarKernelClassID; }
		TCHAR *GetObjectName()  { return GetString(IDS_KE_CYLINDER_VAR_KERNEL); }
		TCHAR * GetDefaultComment() { return GetString(IDS_KE_CYLVAR_COMMENT); }
		long GetKernelSupport() { return long( w + 0.5 ); }
		bool IsVariableSz(){ return TRUE; };
		// 1-D filters ignore the y parameter
		void SetKernelSz( double x, double y = 0.0 ) { w = (x < LOW_THRESH)? LOW_THRESH : x; };
		void GetKernelSz( double& x, double& y ){ x = w; y = 0.0; }
		double KernelFn( double x, double ) {
			if ( x > w ) return 0.0;
			return 1.0;
		}
};

class ConeVarFilterKernel : public GenericKernel {
	private:
		double	w;
	public:
		ConeVarFilterKernel() : w(1.5) {}
		RefTargetHandle Clone(RemapDir& remap)
		{ ConeVarFilterKernel* a = new ConeVarFilterKernel(); a->w = w; BaseClone(this, a, remap); return a; }	
		Class_ID ClassID()      { return ConeVarKernelClassID; }
		TCHAR *GetObjectName()  { return GetString(IDS_KE_CONE_VAR_KERNEL); }
		TCHAR * GetDefaultComment() { return GetString(IDS_KE_CONEVAR_COMMENT); }
		long GetKernelSupport() { return long( w + 0.5 ); }
		bool IsVariableSz(){ return TRUE; };
		// 1-D filters ignore the y parameter
//		void SetKernelSz( double x, double y = 0.0 ) { w = (x == 0.0)? 1.0 : x; };
		void SetKernelSz( double x, double y = 0.0 ) { w = (x < LOW_THRESH)? LOW_THRESH : x; };
		void GetKernelSz( double& x, double& y ){ x = w; y = 0.0; }
		double KernelFn( double x, double y ) {return ( x < w ) ? (w - x)/w : 0.0;}
};

class QuadraticVarFilterKernel : public GenericKernel {
	private:
		double	w;
	public:
		QuadraticVarFilterKernel() : w(1.5) {}
		RefTargetHandle Clone(RemapDir& remap)
		{ QuadraticVarFilterKernel* a = new QuadraticVarFilterKernel(); a->w = w; BaseClone(this, a, remap); return a; }	
		Class_ID ClassID()      { return QuadraticVarKernelClassID; }
		TCHAR *GetObjectName()  { return GetString(IDS_KE_QUADRATIC_VAR_KERNEL); }
		long GetKernelSupport() { return long( w + 0.5 ); }
		bool IsVariableSz(){ return TRUE; };
		// 1-D filters ignore the y parameter
//		void SetKernelSz( double x, double y = 0.0 ) { w = (x == 0.0)? 1.5 : x; };
		void SetKernelSz( double x, double y = 0.0 ) { w = (x < LOW_THRESH)? LOW_THRESH : x; };
		void GetKernelSz( double& x, double& y ){ x = w; y = 0.0; }
		
		double KernelFn( double x, double ) {
			assert ( w != 0.0 );
			double r = 1.5 * x / w;
			if ( r < 0.5 ) return 0.75 - r * r;
			if ( r < 1.5 ) {
				double t = r - 1.5; 
				return 0.5 * t * t;
			}
			return 0.0f;
		}
};

class CubicVarFilterKernel : public GenericKernel {
	private:
		double	w;
	public:
		CubicVarFilterKernel() : w(2.0) {}
		RefTargetHandle Clone(RemapDir& remap)
		{ CubicVarFilterKernel* a = new CubicVarFilterKernel(); a->w = w; BaseClone(this, a, remap); return a; }	
		Class_ID ClassID()      { return CubicVarKernelClassID; }
		TCHAR *GetObjectName()  { return GetString(IDS_KE_CUBIC_VAR_KERNEL); }
		long GetKernelSupport() { return long( w + 0.5 ); }
		bool IsVariableSz(){ return TRUE; };
		// 1-D filters ignore the y parameter
//		void SetKernelSz( double x, double y = 0.0 ) { w = (x == 0.0)? 2.0 : x; };
		void SetKernelSz( double x, double y = 0.0 ) { w = (x < LOW_THRESH)? LOW_THRESH : x; };
		void GetKernelSz( double& x, double& y ){ x = w; y = 0.0; }
		double KernelFn( double x, double ) {
			assert ( w != 0.0 );
			double r = 2.0 * x / w;
			if ( r < 1.0 ) return (4.0 + r * r * (-6.0 + r * 3.0))/6.0;
			if ( r < 2.0 ) { 
				double t = 2.0 - r; 
				return t * t * t / 6.0; 
			}
			return 0.0;
		}
};

class CookVarFilterKernel : public GenericKernel {
	private:
		double	w;
	public:
		CookVarFilterKernel() : w(1.25) {}
		RefTargetHandle Clone(RemapDir& remap)
		{ CookVarFilterKernel* a = new CookVarFilterKernel(); a->w = w; BaseClone(this, a, remap); return a; }	
		Class_ID ClassID()      { return CookVarKernelClassID; }
		TCHAR *GetObjectName()  { return GetString(IDS_KE_COOK_VAR_KERNEL); }
		TCHAR * GetDefaultComment() { return GetString(IDS_KE_COOK_COMMENT); }
		long GetKernelSupport() { return long( w + 0.5 ); }
		bool IsVariableSz(){ return TRUE; };
		// 1-D filters ignore the y parameter
//		void SetKernelSz( double x, double y = 0.0 ) { w = (x == 0.0)? 1.5 : x; };
		void SetKernelSz( double x, double y = 0.0 ) { w = (x < LOW_THRESH)? LOW_THRESH : x; };
		void GetKernelSz( double& x, double& y ){ x = w; y = 0.0; }
		double KernelFn( double x, double ) {
			if ( x > w ) return 0.0;
			double expW2 = exp( -(w * w) );

			return exp( -(x * x) ) - expW2;
		}
};

class GaussVarFilterKernel : public GenericKernel {
	private:
		double	w;
	public:
		GaussVarFilterKernel() : w(3.0) {};
		RefTargetHandle Clone(RemapDir& remap)
		{ GaussVarFilterKernel* a = new GaussVarFilterKernel(); a->w = w; BaseClone(this, a, remap); return a; }	
		Class_ID ClassID()      { return GaussVarKernelClassID; }
		TCHAR *GetObjectName()  { return GetString(IDS_KE_GAUSS_VAR_KERNEL); }
		TCHAR * GetDefaultComment() { return GetString(IDS_KE_GAUSSVAR_COMMENT); }
		long GetKernelSupport() { return long( w + 0.5 ); }
		bool IsVariableSz(){ return TRUE; };
		// 1-D filters ignore the y parameter
//		void SetKernelSz( double x, double y = 0.0 ) { w = (x == 0.0)? 1.5 : x; };
		void SetKernelSz( double x, double y = 0.0 ) { w = (x < LOW_THRESH)? LOW_THRESH : x; };
		void GetKernelSz( double& x, double& y ){ x = w; y = 0.0; }
		double KernelFn( double x, double ) {
			double r = 2.0 * x / w;
			return pow( 2.0, -2.0 * r * r );
		}
};



//------------------------------ Class Descriptors ----------------------------------------
class GenericKernelClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	SClass_ID		SuperClassID() { return FILTER_KERNEL_CLASS_ID; }
	const TCHAR* 	Category() { return _T("");  }
};

class AreaKernelClassDesc : public GenericKernelClassDesc {
public:
	void *			Create(BOOL loading) { return new AreaFilterKernel; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_AREA_KERNEL); }
	Class_ID		ClassID() { return AreaKernelClassID; }
};
static AreaKernelClassDesc areaCD;
ClassDesc* GetAreaKernelDesc() {return &areaCD;}

//////////////////////////////////////////////////////////////////////////////
//	 PixelsSize filter, for 2.5 compatibility & camera mapping
#ifdef INCLUDE_PIXELSIZE 
class PixelSizeKernelClassDesc : public GenericKernelClassDesc {
public:
	void *			Create(BOOL loading) { return new PixelSizeKernel; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_PIXELSIZE_KERNEL); }
	Class_ID		ClassID() { return PixelSizeKernelClassID; }
};
static PixelSizeKernelClassDesc pixelSizeCD;
ClassDesc* GetPixelSizeKernelDesc() {return &pixelSizeCD;}
#endif ////////////////////////////////////////////////////////////////////////

class ConeKernelClassDesc : public GenericKernelClassDesc {
public:
	void *			Create(BOOL loading) { return new ConeFilterKernel; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_CONE_KERNEL); }
	Class_ID		ClassID() { return ConeKernelClassID; }
};
static ConeKernelClassDesc coneCD;
ClassDesc* GetConeKernelDesc() {return &coneCD;}

class QuadraticKernelClassDesc : public GenericKernelClassDesc {
public:
	void *			Create(BOOL loading) { return new QuadraticFilterKernel; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_QUADRATIC_KERNEL); }
	Class_ID		ClassID() { return QuadraticKernelClassID; }
};
static QuadraticKernelClassDesc quadCD;
ClassDesc* GetQuadraticKernelDesc() {return &quadCD;}

class CubicKernelClassDesc : public GenericKernelClassDesc {
public:
	void *			Create(BOOL loading) { return new CubicFilterKernel; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_CUBIC_KERNEL); }
	Class_ID		ClassID() { return CubicKernelClassID; }
};
static CubicKernelClassDesc cubicCD;
ClassDesc* GetCubicKernelDesc() {return &cubicCD;}

class CatRomKernelClassDesc : public GenericKernelClassDesc {
public:
	void *			Create(BOOL loading) { return new CatRomFilterKernel; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_CATROM_KERNEL); }
	Class_ID		ClassID() { return CatRomKernelClassID; }
};
static CatRomKernelClassDesc CatRomCD;
ClassDesc* GetCatRomKernelDesc() {return &CatRomCD;}

class GaussNarrowKernelClassDesc : public GenericKernelClassDesc {
public:
	void *			Create(BOOL loading) { return new GaussNarrowFilterKernel; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_GAUSS_NARROW_KERNEL); }
	Class_ID		ClassID() { return GaussNarrowKernelClassID; }
};
static GaussNarrowKernelClassDesc GaussNarrowCD;
ClassDesc* GetGaussNarrowKernelDesc() {return &GaussNarrowCD;}

class GaussMediumKernelClassDesc : public GenericKernelClassDesc {
public:
	void *			Create(BOOL loading) { return new GaussMediumFilterKernel; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_GAUSS_MEDIUM_KERNEL); }
	Class_ID		ClassID() { return GaussMediumKernelClassID; }
};
static GaussMediumKernelClassDesc GaussMediumCD;
ClassDesc* GetGaussMediumKernelDesc() {return &GaussMediumCD;}

class GaussWideKernelClassDesc : public GenericKernelClassDesc {
public:
	void *			Create(BOOL loading) { return new GaussWideFilterKernel; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_GAUSS_WIDE_KERNEL); }
	Class_ID		ClassID() { return GaussWideKernelClassID; }
};
static GaussWideKernelClassDesc GaussWideCD;
ClassDesc* GetGaussWideKernelDesc() {return &GaussWideCD;}

class PavicUnitVolKernelClassDesc : public GenericKernelClassDesc {
public:
	void *			Create(BOOL loading) { return new PavicUnitVolFilterKernel; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_PAVIC_UNITVOL_KERNEL); }
	Class_ID		ClassID() { return PavicUnitVolKernelClassID; }
};
static PavicUnitVolKernelClassDesc PavicUnitVolCD;
ClassDesc* GetPavicUnitVolKernelDesc() {return &PavicUnitVolCD;}

class PavicOpKernelClassDesc : public GenericKernelClassDesc {
public:
	void *			Create(BOOL loading) { return new PavicOpFilterKernel; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_PAVIC_OP_KERNEL); }
	Class_ID		ClassID() { return PavicOpKernelClassID; }
};
static PavicOpKernelClassDesc PavicOpCD;
ClassDesc* GetPavicOpKernelDesc() {return &PavicOpCD;}

class HanningKernelClassDesc : public GenericKernelClassDesc {
public:
	void *			Create(BOOL loading) { return new HanningFilterKernel; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_HANNING_KERNEL); }
	Class_ID		ClassID() { return HanningKernelClassID; }
};
static HanningKernelClassDesc HanningCD;
ClassDesc* GetHanningKernelDesc() {return &HanningCD;}

class HammingKernelClassDesc : public GenericKernelClassDesc {
public:
	void *			Create(BOOL loading) { return new HammingFilterKernel; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_HAMMING_KERNEL); }
	Class_ID		ClassID() { return HammingKernelClassID; }
};
static HammingKernelClassDesc HammingCD;
ClassDesc* GetHammingKernelDesc() {return &HammingCD;}

class BlackmanKernelClassDesc : public GenericKernelClassDesc {
public:
	void *			Create(BOOL loading) { return new BlackmanFilterKernel; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_BLACKMAN_KERNEL); }
	Class_ID		ClassID() { return BlackmanKernelClassID; }
};
static BlackmanKernelClassDesc BlackmanCD;
ClassDesc* GetBlackmanKernelDesc() {return &BlackmanCD;}

class Blackman367KernelClassDesc : public GenericKernelClassDesc {
public:
	void *			Create(BOOL loading) { return new Blackman367FilterKernel; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_BLACKMAN367_KERNEL); }
	Class_ID		ClassID() { return Blackman367KernelClassID; }
};
static Blackman367KernelClassDesc Blackman367CD;
ClassDesc* GetBlackman367KernelDesc() {return &Blackman367CD;}

class Blackman361KernelClassDesc : public GenericKernelClassDesc {
public:
	void *			Create(BOOL loading) { return new Blackman361FilterKernel; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_BLACKMAN361_KERNEL); }
	Class_ID		ClassID() { return Blackman361KernelClassID; }
};
static Blackman361KernelClassDesc Blackman361CD;
ClassDesc* GetBlackman361KernelDesc() {return &Blackman361CD;}

class Blackman474KernelClassDesc : public GenericKernelClassDesc {
public:
	void *			Create(BOOL loading) { return new Blackman474FilterKernel; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_BLACKMAN474_KERNEL); }
	Class_ID		ClassID() { return Blackman474KernelClassID; }
};
static Blackman474KernelClassDesc Blackman474CD;
ClassDesc* GetBlackman474KernelDesc() {return &Blackman474CD;}

class Blackman492KernelClassDesc : public GenericKernelClassDesc {
public:
	void *			Create(BOOL loading) { return new Blackman492FilterKernel; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_BLACKMAN492_KERNEL); }
	Class_ID		ClassID() { return Blackman492KernelClassID; }
};
static Blackman492KernelClassDesc Blackman492CD;
ClassDesc* GetBlackman492KernelDesc() {return &Blackman492CD;}

class	Max1KernelClassDesc : public GenericKernelClassDesc {
public:
	void *			Create(BOOL loading) { return new Max1FilterKernel; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_MAX1_KERNEL); }
	Class_ID		ClassID() { return Max1KernelClassID; }
};
static Max1KernelClassDesc Max1CD;
ClassDesc* GetMax1KernelDesc() {return &Max1CD;}

class	Max2KernelClassDesc : public GenericKernelClassDesc {
public:
	void *			Create(BOOL loading) { return new Max2FilterKernel; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_MAX2_KERNEL); }
	Class_ID		ClassID() { return Max2KernelClassID; }
};
static Max2KernelClassDesc Max2CD;
ClassDesc* GetMax2KernelDesc() {return &Max2CD;}

class	Max3KernelClassDesc : public GenericKernelClassDesc {
public:
	void *			Create(BOOL loading) { return new Max3FilterKernel; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_MAX3_KERNEL); }
	Class_ID		ClassID() { return Max3KernelClassID; }
};
static Max3KernelClassDesc Max3CD;
ClassDesc* GetMax3KernelDesc() {return &Max3CD;}

class	MitNetOpKernelClassDesc : public GenericKernelClassDesc {
public:
	void *			Create(BOOL loading) { return new MitNetOpFilterKernel; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_MITNET_OP_KERNEL); }
	Class_ID		ClassID() { return MitNetOpKernelClassID; }
};
static MitNetOpKernelClassDesc MitNetOpCD;
ClassDesc* GetMitNetOpKernelDesc() {return &MitNetOpCD;}

class	MitNetNotchKernelClassDesc : public GenericKernelClassDesc {
public:
	void *			Create(BOOL loading) { return new MitNetNotchFilterKernel; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_MITNET_NOTCH_KERNEL); }
	Class_ID		ClassID() { return MitNetNotchKernelClassID; }
};
static MitNetNotchKernelClassDesc MitNetNotchCD;
ClassDesc* GetMitNetNotchKernelDesc() {return &MitNetNotchCD;}

class	NTSCKernelClassDesc : public GenericKernelClassDesc {
public:
	void *			Create(BOOL loading) { return new NTSCFilterKernel; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_NTSC_KERNEL); }
	Class_ID		ClassID() { return NTSCKernelClassID; }
};
static NTSCKernelClassDesc NtscCD;
ClassDesc* GetNTSCKernelDesc() {return &NtscCD;}

class	CookOpKernelClassDesc : public GenericKernelClassDesc {
public:
	void *			Create(BOOL loading) { return new CookOpFilterKernel; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_COOK_OP_KERNEL); }
	Class_ID		ClassID() { return CookOpKernelClassID; }
};
static CookOpKernelClassDesc CookOpCD;
ClassDesc* GetCookOpKernelDesc() {return &CookOpCD;}

// ----------------- soften filters -----------------------------
class	GaussVarKernelClassDesc : public GenericKernelClassDesc {
public:
	void *			Create(BOOL loading) { return new GaussVarFilterKernel; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_GAUSS_VAR_KERNEL); }
	Class_ID		ClassID() { return GaussVarKernelClassID; }
};
static GaussVarKernelClassDesc GaussVarCD;
ClassDesc* GetGaussVarKernelDesc() {return &GaussVarCD;}

class	CookVarKernelClassDesc : public GenericKernelClassDesc {
public:
	void *			Create(BOOL loading) { return new CookVarFilterKernel; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_COOK_VAR_KERNEL); }
	Class_ID		ClassID() { return CookVarKernelClassID; }
	bool IsVariableSz(){ return TRUE; };
};

static CookVarKernelClassDesc CookVarCD;
ClassDesc* GetCookVarKernelDesc() {return &CookVarCD;}

class	CylinderVarKernelClassDesc : public GenericKernelClassDesc {
public:
	void *			Create(BOOL loading) { return new CylinderVarFilterKernel; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_CYLINDER_VAR_KERNEL); }
	Class_ID		ClassID() { return CylinderVarKernelClassID; }
	bool IsVariableSz(){ return TRUE; };
};
static CylinderVarKernelClassDesc CylinderVarCD;
ClassDesc* GetCylinderVarKernelDesc() {return &CylinderVarCD;}


class	ConeVarKernelClassDesc : public GenericKernelClassDesc {
public:
	void *			Create(BOOL loading) { return new ConeVarFilterKernel; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_CONE_VAR_KERNEL); }
	Class_ID		ClassID() { return ConeVarKernelClassID; }
	bool IsVariableSz(){ return TRUE; };
};
static ConeVarKernelClassDesc ConeVarCD;
ClassDesc* GetConeVarKernelDesc() {return &ConeVarCD;}

class	QuadraticVarKernelClassDesc : public GenericKernelClassDesc {
public:
	void *			Create(BOOL loading) { return new QuadraticVarFilterKernel; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_QUADRATIC_VAR_KERNEL); }
	Class_ID		ClassID() { return QuadraticVarKernelClassID; }
	bool IsVariableSz(){ return TRUE; };
};
static QuadraticVarKernelClassDesc QuadraticVarCD;
ClassDesc* GetQuadraticVarKernelDesc() {return &QuadraticVarCD;}

class	CubicVarKernelClassDesc : public GenericKernelClassDesc {
public:
	void *			Create(BOOL loading) { return new CubicVarFilterKernel; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_CUBIC_VAR_KERNEL); }
	Class_ID		ClassID() { return CubicVarKernelClassID; }
	bool IsVariableSz(){ return TRUE; };
};
static CubicVarKernelClassDesc CubicVarCD;
ClassDesc* GetCubicVarKernelDesc() {return &CubicVarCD;}

// ----------------------- Mitchell / Netravali variable filter ----------------------------

// Parameter Description
#define PB_B	0
#define PB_C	1

/*
static ParamUIDesc descParam[] = {
	// B Spinner
	ParamUIDesc(
		PB_B,
		EDITTYPE_FLOAT, 
		IDC_MITNET_B, IDC_MITNET_B_SPIN, 
		0.0, 1.0, 
		SPIN_AUTOSCALE), 

	// C Spinner
	ParamUIDesc(
		PB_C, 
		EDITTYPE_FLOAT, 
		IDC_MITNET_C, IDC_MITNET_C_SPIN, 
		0.0, 1.0, 
		SPIN_AUTOSCALE), 
};
#define PARAMDESC_LENGTH 2
*/

static ParamBlockDescID MitNetDescV0[] = {
	{ TYPE_FLOAT, NULL, FALSE, PB_B }, // B [0]
	{ TYPE_FLOAT, NULL, FALSE, PB_C }  // C [1]
}; 	
#define MNPBLOCK_LENGTH 2

// Version definition
#define MNNUM_OLDVERSIONS	0
#define MNCURRENT_VERSION	1
static ParamVersionDesc curMitnetVersion( MitNetDescV0, MNPBLOCK_LENGTH, MNCURRENT_VERSION );


//--- Variable Mitchell / Netvravali filter --------------------------------------------
//class MitNetDlgProc;
//class MitNetParamDlg;

class MitNetVarFilterKernel : public GenericKernel {
	private:
		double b ;
		double c ;
	public:
//		static MitNetDlgProc *	pDlgProc;
		// Parameters
		IParamBlock *pParamBlock;

		MitNetVarFilterKernel();

		// Animatable/Reference
		int NumSubs() { return 1; }
		Animatable* SubAnim(int i);
		TSTR SubAnimName(int i);

		int NumRefs() { return 1; }
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rTarget);

		RefResult NotifyRefChanged(
					Interval changeInt, 
					RefTargetHandle hTarget, 
					PartID& partID,  
					RefMessage message	);

//		FilterKernelParamDlg * CreateParamDialog(IRendParams *ip);

		Class_ID ClassID()      { return MitNetVarKernelClassID; }
		TCHAR *GetObjectName()  { return GetString(IDS_KE_MITNET_VAR_KERNEL); }
		TCHAR * GetDefaultComment() { return GetString(IDS_KE_MITNET_COMMENT); }
		
		// support both the optional parameters
		long GetNFilterParams() { return 2; }
		TCHAR * GetFilterParamName( long nParam ) { 
			return GetString( nParam ? IDS_KE_MITNET_PARAM_C:IDS_KE_MITNET_PARAM_B );
		}
		double GetFilterParam( long nParam ){ return nParam ? c : b; }
		void SetFilterParam( long nParam, double val ){ 
			if (nParam) {  
				c = val;	
				pParamBlock->SetValue( PB_C, 0, float( val ) );		
 			} else { 
				b = val; 
				pParamBlock->SetValue( PB_B, 0, float( val ) );		
			}
		}
		RefTargetHandle Clone(RemapDir& remap)
		{ MitNetVarFilterKernel* a = new MitNetVarFilterKernel();
		   a->SetFilterParam(0,b); a->SetFilterParam(1,c); BaseClone(this, a, remap); return a; }	

		void Update(TimeValue t, Interval& valid){
			float val;
			pParamBlock->GetValue( PB_B, t, val, valid ); b = val;
			pParamBlock->GetValue( PB_C, t, val, valid ); c = val;
		}

		long GetKernelSupport() { return 2; }
		double KernelFn( double x, double ) {

			// fill in b & c 
			float val = 0.0f;
			Interval valid;

			pParamBlock->GetValue( PB_B, 0, val, valid );
			b = double( val );
			pParamBlock->GetValue( PB_C, 0, val, valid );
			c = double( val );

			if ( x >= 2.0 ) return 0.0;
			if ( x < 1.0 ) {
				// coeffs for x < 1, compute as const
				double D0 = 6.0 - 2.0 * b ;  			// constant term
				double B0 = -18.0 + 12.0 * b + 6.0 * c;	// quadratic term
				double A0 = 12.0 - 9.0 * b - 6.0 * c ;	// cubic term
				return x * ( x * (A0 * x + B0) ) + D0;
			}
			// 1 < x < 2
			double D1 = 8.0 * b + 24.0 * c ;  			// constant term
			double C1 = -12.0 * b - 48.0 * c;			// linear term
			double B1 = 6.0 * b + 30.0 * c;				// quadratic term
			double A1 = - b - 6.0 * c ;					// cubic term
			return x * ( x * (A1 * x + B1) + C1 ) + D1;
		}
};


// MitNetDlgProc *MitNetVarFilterKernel::pDlgProc = NULL;

MitNetVarFilterKernel::MitNetVarFilterKernel()
{
	MakeRefByID(FOREVER, 0, CreateParameterBlock(MitNetDescV0, MNPBLOCK_LENGTH, MNCURRENT_VERSION));
	assert(pParamBlock);
	//valid.SetEmpty();
	b = c = 0.33333f;
	pParamBlock->SetValue( PB_B, 0, 0.3333f );		
	pParamBlock->SetValue( PB_C, 0, 0.3333f );		
}


Animatable* MitNetVarFilterKernel::SubAnim(int i) 
{
	switch (i) {
		case 0: return pParamBlock;
		default: return NULL;
	}
}



TSTR MitNetVarFilterKernel::SubAnimName(int i) 
{
	switch (i) {
		case 0: return GetString(IDS_KE_MITNET_PARAMS);
		default: return _T("");
	}
}

RefTargetHandle MitNetVarFilterKernel::GetReference(int i)
{
	switch (i) {
		case 0: return pParamBlock;
		default: return NULL;
	}
}

void MitNetVarFilterKernel::SetReference(int i, RefTargetHandle rtArg)
{
	switch (i) {
		case 0: pParamBlock = (IParamBlock*)rtArg; break;
	}
}

RefResult MitNetVarFilterKernel::NotifyRefChanged(
		Interval changeInt, RefTargetHandle hTarget,
		PartID& partID,  RefMessage message) 
{
	switch (message) {
		case REFMSG_CHANGE:
			//valid.SetEmpty();
//			if (pDlgProc)
//				pDlgProc->Invalidate();
			;
			break;

		case REFMSG_GET_PARAM_DIM: {
			GetParamDim *gpd = (GetParamDim*)partID;
			switch (gpd->index) {
				case PB_B:			 	gpd->dim = stdNormalizedDim; break;
				case PB_C:				gpd->dim = stdNormalizedDim; break;
				default: 				gpd->dim = defaultDim;
			}
			return REF_STOP; 
		}

		case REFMSG_GET_PARAM_NAME: {
			GetParamName *gpn = (GetParamName*)partID;
			switch (gpn->index) {
				case PB_B:				gpn->name = GetString(IDS_KE_MITNET_PARAM_B); break;
				case PB_C:				gpn->name = GetString(IDS_KE_MITNET_PARAM_C); break;
				default:				gpn->name = _T(""); break;
			}
			return REF_STOP; 
		}
	}
	return REF_SUCCEED;
}


// --------------------- Class Description ----------------------------------------
class	MitNetVarKernelClassDesc : public GenericKernelClassDesc {
public:
	void *			Create(BOOL loading) { return new MitNetVarFilterKernel; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_MITNET_VAR_KERNEL); }
	Class_ID		ClassID() { return MitNetVarKernelClassID; }
};
static MitNetVarKernelClassDesc MitNetVarCD;
ClassDesc* GetMitNetVarKernelDesc() {return &MitNetVarCD;}


// ----------------------- Stocking variable filter ----------------------------

// Parameter Description
#define PB_BLEND		0
#define PB_SHARPNESS	1


static ParamBlockDescID StockingDescV0[] = {
	{ TYPE_FLOAT, NULL, FALSE, PB_BLEND }, // blend, 0 is sharp 1 is blurry
	{ TYPE_FLOAT, NULL, FALSE, PB_SHARPNESS }, // radius of sharp cylinder
}; 	
#define STPBLOCK_LENGTH 2

// Version definition
#define STNUM_OLDVERSIONS	0
#define STCURRENT_VERSION	1
static ParamVersionDesc curStockingVersion( StockingDescV0, STPBLOCK_LENGTH, STCURRENT_VERSION );


//-------- Stocking filter: blend between area & gauss soften --------------------------
#define N_STOCKING_FILTER_PARAMS	2		// 1 is just blend, 2 includes sharpness
class StockingFilterKernel : public GenericKernel {
	private:
		float   blend ;
		float   sharpness;
		double  cylRadius ;	//internal
		double	w;

	public:
		// Parameters
		IParamBlock *pParamBlock;

		StockingFilterKernel();

		RefTargetHandle Clone(RemapDir& remap)
		{ StockingFilterKernel* a = new StockingFilterKernel();
		   a->SetFilterParam(0,blend); a->SetFilterParam(1,sharpness); 
		   a->w = w; 
BaseClone(this, a, remap); 		   return a; 
		}	
		void Update(TimeValue t, Interval& valid){
			float val;
			pParamBlock->GetValue( PB_BLEND, t, val, valid ); blend = val;
			pParamBlock->GetValue( PB_SHARPNESS, t, val, valid ); sharpness = val;
		}

		// Animatable/Reference
		int NumSubs() { return 1; }
		Animatable* SubAnim(int i);
		TSTR SubAnimName(int i);

		int NumRefs() { return 1; }
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rTarget);

		RefResult NotifyRefChanged(
					Interval changeInt, 
					RefTargetHandle hTarget, 
					PartID& partID,  
					RefMessage message	);

		Class_ID ClassID()      { return StockingKernelClassID; }
		TCHAR *GetObjectName()  { return GetString(IDS_KE_STOCKING_KERNEL); }
		TCHAR * GetDefaultComment() { return GetString(IDS_KE_STOCKING_COMMENT); }
		
		// support both the optional parameters
		long GetNFilterParams() { return N_STOCKING_FILTER_PARAMS; }
		TCHAR * GetFilterParamName( long nParam ) { return  GetString( nParam==0 ? IDS_KE_STOCKING_BLEND: IDS_KE_STOCKING_SHARPNESS ); }
		double GetFilterParam( long nParam ){ return nParam ? sharpness : blend; }
		void SetFilterParam( long nParam, double val );

		long GetKernelSupport() { return long( w + 0.5 ); }
		bool IsVariableSz(){ return TRUE; };
		// 1-D filters ignore the y parameter
		void SetKernelSz( double x, double y = 0.0 ) { w = (x == 0.0)? 0.0001 : x; };
		void GetKernelSz( double& x, double& y ){ x = w; y = 0.0; }

		double KernelFn( double x, double ) {
			if ( x > w ) return 0.0;
			double r = 2.0 * x / w;
			double h = 2.0f * cylRadius / w; //correct for un-equal volumes
			double g = blend * h * pow( 2.0, -2.0 * r * r );
			if ( x < cylRadius ) {
				// inside the cylindrical area filter, add it's part
				g += (1.0 - blend);
			}
			if ( g > 1.0 ) g = 1.0;
			return g;
		}

};



StockingFilterKernel::StockingFilterKernel ()
{
	MakeRefByID(FOREVER, 0, CreateParameterBlock(StockingDescV0, STPBLOCK_LENGTH, STCURRENT_VERSION));
	assert(pParamBlock);
	blend = 0.3f;
	w = 4.0;
	pParamBlock->SetValue( PB_BLEND, 0, blend );
	sharpness = 0.6f; // d = 2 at s == 0 && d = 1 at s == 1; r = 0.5d
	cylRadius = (2.0f - sharpness) * 0.5f;
	pParamBlock->SetValue( PB_SHARPNESS, 0, sharpness );		
}

void StockingFilterKernel::SetFilterParam( long nParam, double val )
{ 
	if ( nParam == 0 ) {
		blend = float(val);	
		pParamBlock->SetValue( PB_BLEND, 0, blend );		
	} else {
		sharpness = float(val);	
		cylRadius = (2.0f - sharpness) * 0.5f;
		pParamBlock->SetValue( PB_SHARPNESS, 0, sharpness );		
	}
}

Animatable* StockingFilterKernel::SubAnim(int i) 
{
	switch (i) {
		case 0: return pParamBlock;
		default: return NULL;
	}
}



TSTR StockingFilterKernel::SubAnimName(int i) 
{
	switch (i) {
		case 0: return GetString(IDS_KE_STOCKING_PARAMS);
		default: return _T("");
	}
}

RefTargetHandle StockingFilterKernel::GetReference(int i)
{
	switch (i) {
		case 0: return pParamBlock;
		default: return NULL;
	}
}

void StockingFilterKernel::SetReference(int i, RefTargetHandle rtArg)
{
	switch (i) {
		case 0: pParamBlock = (IParamBlock*)rtArg; break;
	}
}

RefResult StockingFilterKernel::NotifyRefChanged(
		Interval changeInt, RefTargetHandle hTarget,
		PartID& partID,  RefMessage message) 
{
	switch (message) {
		case REFMSG_CHANGE:
			;
			break;

		case REFMSG_GET_PARAM_DIM: {
			GetParamDim *gpd = (GetParamDim*)partID;
			switch (gpd->index) {
				case PB_SHARPNESS:	 	gpd->dim = stdNormalizedDim; break;
				case PB_BLEND:		 	gpd->dim = stdNormalizedDim; break;
				default: 				gpd->dim = defaultDim;
			}
			return REF_STOP; 
		}

		case REFMSG_GET_PARAM_NAME: {
			GetParamName *gpn = (GetParamName*)partID;
			switch (gpn->index) {
			case PB_BLEND:			gpn->name = GetString(IDS_KE_STOCKING_BLEND); break;
			case PB_SHARPNESS:		gpn->name = GetString(IDS_KE_STOCKING_SHARPNESS); break;
				default:			gpn->name = _T(""); break;
			}
			return REF_STOP; 
		}
	}
	return REF_SUCCEED;
}


// --------------------- Class Description ----------------------------------------
class	StockingClassDesc : public GenericKernelClassDesc {
public:
	void *			Create(BOOL loading) { return new StockingFilterKernel; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_STOCKING_KERNEL); }
	Class_ID		ClassID() { return StockingKernelClassID; }
};
static StockingClassDesc StockingCD;
ClassDesc* GetStockingKernelDesc() {return &StockingCD;}


// ----------------------- Blend variable filter ----------------------------

// Parameter Description
#define PB_BLEND		0


static ParamBlockDescID BlendDescV0[] = {
	{ TYPE_FLOAT, NULL, FALSE, PB_BLEND }, // blend, 0 is sharp 1 is blurry
}; 	
#define BLPBLOCK_LENGTH 1

// Version definition
#define BLNUM_OLDVERSIONS	0
#define BLCURRENT_VERSION	1
static ParamVersionDesc curBlendVersion( BlendDescV0, BLPBLOCK_LENGTH, BLCURRENT_VERSION );




//-------- New Blend filter: blend between max1 and quadratic soften ------------------
class BlendFilterKernel : public GenericKernel {
	private:
		float   blend ;
		float	w;

	public:
		// Parameters
		IParamBlock *pParamBlock;

		BlendFilterKernel();
		RefTargetHandle Clone(RemapDir& remap)
		{ BlendFilterKernel* a = new BlendFilterKernel();
		   a->SetFilterParam(0,blend); a->w = w; 
BaseClone(this, a, remap); 		   return a; 
		}	
		void Update(TimeValue t, Interval& valid){
			float val;
			pParamBlock->GetValue( PB_BLEND, t, val, valid ); blend = val;
		}

		// Animatable/Reference
		int NumSubs() { return 1; }
		Animatable* SubAnim(int i);
		TSTR SubAnimName(int i);

		int NumRefs() { return 1; }
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rTarget);

		RefResult NotifyRefChanged(
					Interval changeInt, 
					RefTargetHandle hTarget, 
					PartID& partID,  
					RefMessage message	);

		Class_ID ClassID()      { return BlendKernelClassID; }
		TCHAR *GetObjectName()  { return GetString(IDS_KE_BLEND_KERNEL); }
		TCHAR * GetDefaultComment() { return GetString(IDS_KE_BLEND_COMMENT); }
		
		// support both the optional parameters
		long GetNFilterParams() { return 1; }
		TCHAR * GetFilterParamName( long nParam ) { return  nParam==0 ? GetString(IDS_KE_STOCKING_BLEND) : NULL; }
		double GetFilterParam( long nParam ){ return nParam==0 ? blend : 0.0; }
		void SetFilterParam( long nParam, double val );

		long GetKernelSupport() { return long( w + 0.5 ); }
		bool IsVariableSz(){ return TRUE; };
		// 1-D filters ignore the y parameter
		void SetKernelSz( double x, double y = 0.0 ) { w = (x < LOW_THRESH)? LOW_THRESH : x; };
		void GetKernelSz( double& x, double& y ){ x = w; y = 0.0; }

		double KernelFn( double x, double ) {
			if ( x > w ) return 0.0;

			const double Pi = 3.14159;
			const double t = 1.3778;
			const double s = 0.4848;
			const double t2 = t*t;
			const double s2 = s*s;
			const double stInv = 1.0 / (s * t);
			const double ttms = t * (t - s);
			const double v = 2.0 * Pi * (s2 * 0.5 - s2*s/(4.0 * t) + t2 * t2 /(12.0*ttms)
							+ s2*t2/(2.0*ttms) + 2.0*s2*s*t/(3.0*ttms) - s2*s2/(4.0*ttms) );
			double h =/* 2.0* */ v / (Pi * w * w); 
			double g = blend * h;

			if ( x >= t ) return g;

			if ( x > s ) 
				g += (1.0-blend) * (t - x) * (t - x) / ttms;
			else 
				g += (1.0-blend) * (1.0 - x * x * stInv);

			if ( g > 1.0 ) g = 1.0;
			return g;
		}

};



BlendFilterKernel::BlendFilterKernel ()
{
	MakeRefByID(FOREVER, 0, CreateParameterBlock(BlendDescV0, BLPBLOCK_LENGTH, BLCURRENT_VERSION));
	assert(pParamBlock);
	blend = 0.3f;
	w = 4.0;
	pParamBlock->SetValue( PB_BLEND, 0, blend );
}

void BlendFilterKernel::SetFilterParam( long nParam, double val )
{ 
	if ( nParam == 0 ) {
		blend = float(val);	
		pParamBlock->SetValue( PB_BLEND, 0, blend );		
	}
}

Animatable* BlendFilterKernel::SubAnim(int i) 
{
	switch (i) {
		case 0: return pParamBlock;
		default: return NULL;
	}
}



TSTR BlendFilterKernel::SubAnimName(int i) 
{
	switch (i) {
		case 0: return GetString(IDS_KE_STOCKING_PARAMS);
		default: return _T("");
	}
}

RefTargetHandle BlendFilterKernel::GetReference(int i)
{
	switch (i) {
		case 0: return pParamBlock;
		default: return NULL;
	}
}

void BlendFilterKernel::SetReference(int i, RefTargetHandle rtArg)
{
	switch (i) {
		case 0: pParamBlock = (IParamBlock*)rtArg; break;
	}
}

RefResult BlendFilterKernel::NotifyRefChanged(
		Interval changeInt, RefTargetHandle hTarget,
		PartID& partID,  RefMessage message) 
{
	switch (message) {
		case REFMSG_CHANGE:
			;
			break;

		case REFMSG_GET_PARAM_DIM: {
			GetParamDim *gpd = (GetParamDim*)partID;
			switch (gpd->index) {
				case PB_BLEND:		 	gpd->dim = stdNormalizedDim; break;
				default: 				gpd->dim = defaultDim;
			}
			return REF_STOP; 
		}

		case REFMSG_GET_PARAM_NAME: {
			GetParamName *gpn = (GetParamName*)partID;
			switch (gpn->index) {
			case PB_BLEND:			gpn->name = GetString(IDS_KE_STOCKING_BLEND); break;
				default:			gpn->name = _T(""); break;
			}
			return REF_STOP; 
		}
	}
	return REF_SUCCEED;
}


// --------------------- Class Description ----------------------------------------
class	BlendClassDesc : public GenericKernelClassDesc {
public:
	void *			Create(BOOL loading) { return new BlendFilterKernel; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_BLEND_KERNEL); }
	Class_ID		ClassID() { return BlendKernelClassID; }
};
static BlendClassDesc BlendCD;
ClassDesc* GetBlendKernelDesc() {return &BlendCD;}




/***************** rollup version

//--- Mitchell/Netravali DlgProc -----------------------------------------

class MitNetDlgProc : public ParamMapUserDlgProc {
	public:
		IParamMap *pMap;
		MitNetVarFilterKernel * pKernel;
		IRendParams *pParams;
		HWND hWnd;

		MitNetDlgProc( IParamMap *map, MitNetVarFilterKernel *kern, IRendParams *par );
		~MitNetDlgProc();

		void Init();
		void SetState();
		void Invalidate() { pMap->Invalidate(); } //{ if (hWnd) InvalidateRect(hWnd,NULL,0); }
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void DeleteThis() { delete this; }
	};
  
MitNetDlgProc::MitNetDlgProc(IParamMap *map, MitNetVarFilterKernel *kern, IRendParams *par) 
{
	this->pMap = map;
	pKernel = kern;
	pParams  = par;
	pKernel->pDlgProc = this;
}

MitNetDlgProc::~MitNetDlgProc()
{
	pKernel->pDlgProc = NULL;
}

void MitNetDlgProc::Init()
{
}

void MitNetDlgProc::SetState()
{
}

BOOL MitNetDlgProc::DlgProc(
		TimeValue t, IParamMap *map, HWND hWnd,
		UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch (msg) {
		case WM_INITDIALOG:
			this->hWnd = hWnd;
			Init();
			SetState();
			break;

		case WM_DESTROY:
			break;

		case WM_COMMAND:
			break;
	}
	return FALSE;
}


//--- Mitchell/ Netravali ParamDlg -------------------------------------------------------

class MitNetParamDlg : public FilterKernelParamDlg {
	public:
		MitNetVarFilterKernel * pKernel;
		IRendParams *pParams;
		IParamMap *pMap;

		MitNetParamDlg( MitNetVarFilterKernel *kern, IRendParams *par ); 
		Class_ID ClassID() { return StdKernelClassID; }
		ReferenceTarget* GetThing() { return pKernel;}
		void SetThing( ReferenceTarget *m );		
		void DeleteThis();
};


MitNetParamDlg::MitNetParamDlg( MitNetVarFilterKernel *kern, IRendParams *par ) 
{
	pKernel = kern;
	pParams = par;	
	pMap = CreateRParamMap(
		descParam, PARAMDESC_LENGTH,
		pKernel->pParamBlock,
		par,
		hInstance,
		MAKEINTRESOURCE(IDD_MITNET_PARAMS),
		GetString(IDS_KE_MITNET_PARAMS),
		0);
	
	pMap->SetUserDlgProc(new MitNetDlgProc(pMap, pKernel, pParams));	
}

void MitNetParamDlg::SetThing(ReferenceTarget *m)
{
	assert(m->ClassID()== pKernel->ClassID());
	pKernel = (MitNetVarFilterKernel*)m;
	pMap->SetParamBlock( pKernel->pParamBlock );
	pMap->SetUserDlgProc( new MitNetDlgProc(pMap, pKernel, pParams) );	
	if (pKernel->pDlgProc) {
		pKernel->pDlgProc->pKernel = pKernel;		
		pKernel->pDlgProc->Init();
		pKernel->pDlgProc->SetState();
	}
}

void MitNetParamDlg::DeleteThis()
{
	DestroyRParamMap(pMap);
	delete this;
}

// ------------------------ must be here due to class interlock dependency ------------------------
FilterKernelParamDlg * MitNetVarFilterKernel::CreateParamDialog(IRendParams *ip)
{	
	return new MitNetParamDlg(this,ip);
}

**************/




////////////////////////////////////////////////////////////////////
// anisotropic filters not supported yet
// shirley's filter, 1 pixels
float shirleyFilter( float x, float y )
{
	if ( x >= 1.0f || y >= 1.0f ) return 0.0f;
	return (1.0f - x ) * ( 1.0f - y);
}

// simple box filter, equivalent to default filter, 1 pixel
float boxFilter( float x, float y )
{
	if ( x >= 1.0f || y >= 1.0f ) return 0.0f;
	return 1.0f;
}

