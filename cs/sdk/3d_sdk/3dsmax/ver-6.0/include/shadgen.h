/**********************************************************************
 *<
FILE: shadgen.h : pluggable shadow generators.

	DESCRIPTION:

	CREATED BY: Dan Silva

	HISTORY: Created 10/27/98

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#ifndef __SHADGEN__H

#define __SHADGEN__H

/////////////////////////////////////////////////////////////////////////
// Shadow Generator flags
//
// These define the the 'flags' parameter to the CreateShadowGeerator call
// of the ShadowType class
// 
#define SHAD_PARALLEL       2	// light is directional, parallel projection
#define SHAD_OMNI           4	// generator can do omni, so do it on create
#define SHAD_2SIDED         8	// both sides of geometry shd cast shadows

class ShadowGenerator;
class AreaShadowGenerator;
class IAreaShadowType;
class ParamBlockDescID;
class IParamBlock;

class ShadowParamDlg {
public:
	virtual void DeleteThis()=0;
	};

//
// NB: This Class needs to be made extensible w/ derivation from baseInterfaceServer
// This will be done the next time the API is changed
//
// This class carries the parameters for the shadow type, and puts up the parameter rollup.
#define AREA_SHADOW_TYPE_INTERFACE_ID Interface_ID(0x68436888, 0x5b5b2ab0)

class ShadowType: public ReferenceTarget {
	public:
		SClass_ID SuperClassID() { return SHADOW_TYPE_CLASS_ID;}
		virtual ShadowParamDlg *CreateShadowParamDlg(Interface *ip) { return NULL; }
		virtual ShadowGenerator* CreateShadowGenerator(LightObject *l,  ObjLightDesc *ld, ULONG flags)=0;
		virtual BOOL SupportStdMapInterface() { return FALSE; }

		BOOL BypassPropertyLevel() { return TRUE; }  // want to promote shadowtype props to light level

		// If the shadow generator can handle omni's directly, this should return true. If it does,
		// then when doing an Omni light, the SHAD_OMNI flag will be passed in to 
		// the CreateShadowGenerator call, and only one ShadowGenerator will be created
		// instead of the normal 6 (up,down,right,left,front,back).
		virtual BOOL CanDoOmni() { return FALSE; }

		// This method used for converting old files: only needs to be supported by default 
		// shadow map and ray trace shadows.
		virtual void ConvertParamBlk( ParamBlockDescID *descOld, int oldCount, IParamBlock *oldPB ) { }

		// This method valid iff SupportStdMapInterface returns TRUE
		virtual int MapSize(TimeValue t) { return 512; } 

		// This interface is solely for the default shadow map type ( Class_ID(STD_SHADOW_MAP_CLASS_ID,0) )
		virtual void SetMapRange(TimeValue t, float f) {}
		virtual float GetMapRange(TimeValue t, Interval& valid = Interval(0,0)) { return 0.0f; }
		virtual void SetMapSize(TimeValue t, int f) {}
		virtual int GetMapSize(TimeValue t, Interval& valid = Interval(0,0)) { return 0; }
		virtual void SetMapBias(TimeValue t, float f) {} 
		virtual float GetMapBias(TimeValue t, Interval& valid = Interval(0,0)) { return 0.0f; }
		virtual void SetAbsMapBias(TimeValue t, int a) {}
		virtual int GetAbsMapBias(TimeValue t, Interval& valid = Interval(0,0)) { return 0; }

		// This interface is solely for the default raytrace shadow type ( Class_ID(STD_RAYTRACE_SHADOW_CLASS_ID,0) )
		virtual float GetRayBias(TimeValue t, Interval &valid = Interval(0,0)) { return 0.0f; }
		virtual	void SetRayBias(TimeValue t, float f) {}
		virtual int GetMaxDepth(TimeValue t, Interval &valid = Interval(0,0)) { return 1; } 
		virtual void SetMaxDepth(TimeValue t, int f) {}

		// Because this method is inlined and only uses existing methods
		// it doesn't break the SDK. Return the IAreaShadowType interface
		IAreaShadowType* GetAreaShadowType();
			
	};

// This class carries the parameters for area shadows. It also creates
// an AreaShadowGenerator to process the shadows during rendering.
class IAreaShadowType : public BaseInterface {
	public:
		// Create the AreaShadowGenerator to process shadows during rendering
		virtual AreaShadowGenerator* CreateAreaShadowGenerator(LightObject *l,  ObjLightDesc *ld, ULONG flags)=0;

		// This method can be used to disable the area related controls
		// in the UI. It is used by area and linear lights to disable
		// these controls because the lights will control the area shadows.
		virtual void EnableAreaUI(bool onoff) {}

		// These are the area shadow parameters. You don't need to
		// implement these if you only want to use the AreaShadowGenerator interface.
		virtual float GetLength(TimeValue t) { return 0.0f; }
		virtual void SetLength(TimeValue t, float w) {}
		virtual float GetWidth(TimeValue t) { return 0.0f; }
		virtual void SetWidth(TimeValue t, float w) {}
	};

inline IAreaShadowType* ShadowType::GetAreaShadowType()
{
	return static_cast<IAreaShadowType*>(GetInterface(AREA_SHADOW_TYPE_INTERFACE_ID));
}

///////////////////////////////////////////////////////////////
//
//	This controls the default value for the 2Sided shadow attribute
//	of viz4 shadow generators. must be compile time const as it's
//  compiled into pb2's
//
#ifdef DESIGN_VER	
#define TWO_SIDED_SHADOW_DEFAULT	TRUE
#else
#define TWO_SIDED_SHADOW_DEFAULT	FALSE
#endif


// This class generates the shadows. It only exists during render, one per instance of the light.
class ShadowGenerator {
public:
	virtual int Update(
		TimeValue t,
		const RendContext& rendCntxt,   // Mostly for progress bar.
		RenderGlobalContext *rgc,       // Need to get at instance list.
		Matrix3& lightToWorld, // light to world space: not necessarly same as that of light
		float aspect,      // aspect
		float param,   	   // persp:field-of-view (radians) -- parallel : width in world coords
		float clipDist = DONT_CLIP  
		)=0;

	virtual int UpdateViewDepParams(const Matrix3& worldToCam)=0;

	virtual void FreeBuffer()=0;
	virtual void DeleteThis()=0; // call this to destroy the ShadowGenerator

	// Generic shadow sampling function
	// Implement this when ShadowType::SupportStdMapInterface() returns FALSE. 
	virtual float Sample(ShadeContext &sc, Point3 &norm, Color& color) { return 1.0f; }

	// Implement these methods when ShadowType::SupportStdMapInterface() returns TRUE. 
	// This interface allows illuminated atmospherics
	// Note: Sample should return a small NEGATIVE number ehen the sample falls outside of the shadow buffer, so
	//    the caller can know to take appropriate action.
	virtual	float Sample(ShadeContext &sc, float x, float y, float z, float xslope, float yslope) { return 1.0f; }
	virtual BOOL QuickSample(int x, int y, float z) { return 1; }
	virtual float FiltSample(int x, int y, float z, int level) { return 1.0f; }
	virtual float LineSample(int x1, int y1, float z1, int x2, int y2, float z2) { return 1.0f; }

	};

// For performance reasons, the area shadow generator is broken into two
// parts, the AreaShadowGenerator and the AreaShadowSampler. This is to allow
// some coherency in the multiple samples that are required to sample
// and area light. The AreaShadowGenerator returns the size of the AreaShadowSampler,
// which is allocated by the caller. The AreaShadowGenerator then initializes
// the allocate memory and return the address of the sampler. The destructor
// for the AreaShadowSampler is not called when the memory is freed. The
// AreaShadowSampler may not be used by multiple threads.

class AreaShadowSampler;

// The class generates area shadows. It only exists during render, one per instance of the light.
class AreaShadowGenerator : public ShadowGenerator {
	public:
		// Get the size of the sampler.
		virtual int GetSamplerSize() = 0;

		// Initialize the sampler and return a pointer to it. Memory must
		// be allocated by the caller and must be at least GetSamplerSize.
		virtual AreaShadowSampler* InitializeSampler(void* memory, ShadeContext& sc, bool antialias) = 0;

		// Return the number of samples we should use for
		// determining visibility of an area.
		virtual int GetNumSamples() { return 1; }
	};

// The class samples area shadows. It only exists during render, and is dynamically
// allocated on the stack using _alloca.
class AreaShadowSampler {
	public:
		// Sample the area shadow generator. This call samples the occlusion
		// from sourcePnt to the point being shaded. SourcePnt needs to be
		// in local light space coordinates if the light is parallel,
		// and in camera space coordinate if the light is not parallel.
		// If antialias is true, the result is antialiased using the settings
		// for the generator. If antialias is false a single sample is returned.
		// The default implementation ignores the sourcePnt and anitalias
		// arguments and simply returns the result of the standard sampler.
		virtual float Sample(
			ShadeContext&	sc,
			const Point3&	sourcePnt,
			Point3&			norm,
			Color&			color
			) = 0;
	};

// This returns a new default shadow-map shadow generator
CoreExport ShadowType *NewDefaultShadowMapType();

// This returns a new default ray-trace shadow generator
CoreExport ShadowType *NewDefaultRayShadowType();


#endif __SHADGEN__H
