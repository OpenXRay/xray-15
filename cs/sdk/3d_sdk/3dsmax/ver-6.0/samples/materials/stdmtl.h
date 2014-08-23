/**********************************************************************
 *<
	FILE: stdmtl.h

	DESCRIPTION:

	CREATED BY: Dan Silva

	HISTORY:

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#ifndef __STDMTL__H
#define __STDMTL__H

#include "stdmat.h"
#include "texmaps.h"

// StdMtl flags values
#define STDMTL_ADD_TRANSP   (1<<0)
#define STDMTL_FALLOFF_OUT  (1<<1)
#define STDMTL_WIRE		  	(1<<2)
#define STDMTL_2SIDE		(1<<3)
#define STDMTL_SOFTEN       (1<<4)
#define STDMTL_FILT_TRANSP 	(1<<5)
#define STDMTL_WIRE_UNITS	(1<<6)
#define STDMTL_LOCK_AD      (1<<8)
#define STDMTL_LOCK_DS      (1<<9)
#define STDMTL_UNUSED1		(1<<10)
#define STDMTL_LOCK_ADTEX   (1<<11)
#define STDMTL_FACEMAP		(1<<12)
#define STDMTL_OLDSPEC      (1<<13)
#define STDMTL_SSAMP		(1<<14)

#define STDMTL_ROLLUP1_OPEN  (1<<28)
#define STDMTL_ROLLUP2_OPEN  (1<<29)
#define STDMTL_ROLLUP3_OPEN  (1<<30)
#define STDMTL_ROLLUP4_OPEN  (1<<27)

#define STDMTL_ROLLUP_FLAGS (STDMTL_ROLLUP1_OPEN|STDMTL_ROLLUP2_OPEN|STDMTL_ROLLUP3_OPEN|STDMTL_ROLLUP4_OPEN)


class StdMtlDlg;
class Sampler;
struct SIllumParams;

class SShader {
	public:
	virtual void Illum(ShadeContext &sc, SIllumParams &ip)=0;
	virtual void AffectReflMap(ShadeContext &sc, SIllumParams &ip, Color &rcol)=0;
	virtual void SetShininess(float shininess, float shineStr)=0;
	virtual float EvalHilite(float x)=0;
	};

class StdMtl: public StdMat {
	// Animatable parameters
	public:
		IParamBlock *pblock;   // ref 0
		Texmaps* maps;         // ref 1
		Interval ivalid;
		StdMtlDlg *paramDlg;
		ULONG flags;
		int rollScroll;
		int shading;
		SShader *curShader;

		// Cache values: must call Update() when change frame.
		Color ambient;
		Color diffuse;
		Color specular;
		Color filter;
		float shininess;  
		float shine_str;  
		float self_illum;	
		float opacity;	
		float opfall;
		float phongexp;
		float wireSize;
		float ioRefract;
		float dimIntens;
		float dimMult;
		float softThresh;
		BOOL dimReflect;

		void SetFlag(ULONG f, ULONG val);
		void EnableMap(int i, BOOL onoff);
		BOOL IsMapEnabled(int i) { return (*maps)[i].mapOn; }
		BOOL KeyAtTime(int id,TimeValue t) { return pblock->KeyFrameAtTime(id,t); }
		BOOL AmtKeyAtTime(int i, TimeValue t);
		void SetShadingNoNotify( int s);

		// from StdMat
		void SetAmbient(Color c, TimeValue t);		
		void SetDiffuse(Color c, TimeValue t);		
		void SetSpecular(Color c, TimeValue t);
		void SetFilter(Color c, TimeValue t);
		void SetShininess(float v, TimeValue t);		
		void SetShinStr(float v, TimeValue t);		
		void SetSelfIllum(float v, TimeValue t);		
		void SetTexmapAmt(int imap, float amt, TimeValue t);
		void SetWireSize(float s, TimeValue t);
		void SetSelfIllumColor(class Color,int){}
		void SetSelfIllumColorOn(BOOL){}
		int IsSelfIllumColorOn(){ return FALSE; }
		// >>>> sampling
		void SetSamplingOn( BOOL on ){}	
		BOOL GetSamplingOn(){ return FALSE; }	
		void SetSamplingQuality( float quality ) { }	
		float GetSamplingQuality(){ return 1.0f; }	
		void SetPixelSampler( Sampler * sampler ){}	
		Sampler * GetPixelSampler(int mtlId, BOOL backFace){ return NULL; }	

		// >>>Shaders
		void SetShaderId( long shaderId );
		long GetShaderId();
		void SetShader( Shader* pShader ){}
		Shader* GetShader(){ return NULL; }
		
	 	void SetShading(int s){ SetShaderId(s); }
		void SetSoften(BOOL onoff) { SetFlag(STDMTL_SOFTEN,onoff); }
		void SetFaceMap(BOOL onoff) { SetFlag(STDMTL_FACEMAP,onoff); }
		void SetTwoSided(BOOL onoff) { SetFlag(STDMTL_2SIDE,onoff); }
		void SetWire(BOOL onoff){ SetFlag(STDMTL_WIRE,onoff); }
		void SetWireUnits(BOOL onoff) { SetFlag(STDMTL_WIRE_UNITS,onoff); }
		void SetFalloffOut(BOOL onoff) { SetFlag(STDMTL_FALLOFF_OUT,onoff); }
		void SetTransparencyType(int type);
		void LockAmbDiffTex(BOOL onoff) { SetFlag(STDMTL_LOCK_ADTEX,onoff); }
		void SetOpacity(float v, TimeValue t);		
		void SetOpacFalloff(float v, TimeValue t);		
		void SetIOR(float v, TimeValue t);
		void SetDimIntens(float v, TimeValue t);
		void SetDimMult(float v, TimeValue t);
		void SetSoftenLevel(float v, TimeValue t);
		
	    int GetFlag(ULONG f) { return (flags&f)?1:0; }


		// from Mtl
		Color GetAmbient(int mtlNum=0, BOOL backFace=FALSE);		
	    Color GetDiffuse(int mtlNum=0, BOOL backFace=FALSE);		
		Color GetSpecular(int mtlNum=0, BOOL backFace=FALSE);
		float GetShininess(int mtlNum=0, BOOL backFace=FALSE) { return shininess; }		
		float GetShinStr(int mtlNum=0, BOOL backFace=FALSE) { return shine_str;  }
		float GetXParency(int mtlNum=0, BOOL backFace=FALSE) { if(opacity>0.9f && opfall>0.0f) return 0.1f; return 1.0f-opacity; }
		float WireSize(int mtlNum=0, BOOL backFace=FALSE) { return wireSize; }
		float GetSelfIllum(int mtlNum=0, BOOL backFace=FALSE) { return self_illum; }
		
		// Dynamics properties
		float GetDynamicsProperty(TimeValue t, int mtlNum, int propID);
		void SetDynamicsProperty(TimeValue t, int mtlNum, int propID, float value);

		// from StdMat
		int GetShading(){ return GetShaderId(); }
		BOOL GetSoften() { return GetFlag(STDMTL_SOFTEN); }
		BOOL GetFaceMap() { return GetFlag(STDMTL_FACEMAP); }
		BOOL GetTwoSided() { return GetFlag(STDMTL_2SIDE); }
		BOOL GetWire() { return GetFlag(STDMTL_WIRE); }
		BOOL GetWireUnits() { return GetFlag(STDMTL_WIRE_UNITS); }
		BOOL GetFalloffOut() { return GetFlag(STDMTL_FALLOFF_OUT); }  // 1: out, 0: in
		BOOL GetAmbDiffTexLock(){ return GetFlag(STDMTL_LOCK_ADTEX); } 
		int GetTransparencyType() {
			return (flags&STDMTL_FILT_TRANSP)?TRANSP_FILTER:
				flags&STDMTL_ADD_TRANSP ? TRANSP_ADDITIVE: TRANSP_SUBTRACTIVE;
			}
		Color GetAmbient(TimeValue t);		
		Color GetDiffuse(TimeValue t);		
		Color GetSpecular(TimeValue t);
		Color GetFilter(TimeValue t);
		float GetShininess( TimeValue t);		
		float GetShinStr(TimeValue t);		
		float GetSelfIllum(TimeValue t);		
		float GetOpacity( TimeValue t);		
		float GetOpacFalloff(TimeValue t);		
		float GetWireSize(TimeValue t);
		float GetIOR( TimeValue t);
		float GetDimIntens( TimeValue t);
		float GetDimMult( TimeValue t);
		float GetSoftenLevel( TimeValue t);
		BOOL MapEnabled(int i);
		float GetTexmapAmt(int imap, TimeValue t);

		// internal
		float GetSelfIll() { return self_illum; }		
		float GetOpacity() { return opacity; }		
		float GetOpacFalloff() { return opfall; }		
		float GetTexmapAmt(int imap);
		Color GetFilter();
		float GetIOR() { return ioRefract; }
		StdMtl(BOOL loading = FALSE);
		void SetParamDlg( StdMtlDlg *pd) {	paramDlg = pd; }
		BOOL ParamWndProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
		ParamDlg* CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp);
		Color TranspColor(ShadeContext& sc, float opac, Color& diff);
		void Shade(ShadeContext& sc);
		float EvalDisplacement(ShadeContext& sc); 
		Interval DisplacementValidity(TimeValue t); 
		void Update(TimeValue t, Interval& validr);
		void Reset();
		void OldVerFix(int loadVer);
		Interval Validity(TimeValue t);
		void NotifyChanged();
		void UpdateShader();


		// Requirements
		ULONG Requirements(int subMtlNum);

		// Methods to access texture maps of material
		int NumSubTexmaps() { return NTEXMAPS; }
		Texmap* GetSubTexmap(int i) { return (*maps)[i].map; }
		int MapSlotType(int i) { 
			if (i==ID_DP) return MAPSLOT_DISPLACEMENT;
			return (i==ID_RL||i==ID_RR)?MAPSLOT_ENVIRON:MAPSLOT_TEXTURE; 
			}
		void SetSubTexmap(int i, Texmap *m);
		TSTR GetSubTexmapSlotName(int i);
		int SubTexmapOn(int i) { return  MAPACTIVE(i); } 

		Class_ID ClassID();
		SClass_ID SuperClassID() { return MATERIAL_CLASS_ID; }
		void GetClassName(TSTR& s) { s = GetString(IDS_DS_STANDARD); }  

		void DeleteThis();

		int NumSubs() { return 2; }  
	    Animatable* SubAnim(int i);
		TSTR SubAnimName(int i);
		int SubNumToRefNum(int subNum) {return subNum;	}

		// From ref
 		int NumRefs() { return 2; }
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);

		RefTargetHandle Clone(RemapDir &remap = NoRemap());
		RefResult NotifyRefChanged( Interval changeInt, RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message );

		// IO
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);

	};

Mtl* CreateStdMtl();

#endif
