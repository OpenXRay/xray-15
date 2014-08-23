/**********************************************************************
 *<
	FILE:			Wood.cpp

	DESCRIPTION:	Wood 3D Texture map Class Decl.

	CREATED BY:		Suryan Stalin

	HISTORY:		Modified from Marble.cpp by adding IPAS wood stuff

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#ifndef __WOOD__H
#define __WOOD__H

#include "iparamm2.h"

//	defines
#define WOOD_CLASS_ID 		0x0000214
#define NSUBTEX				2
#define NCOLS				2
#define WOOD_VERSION		1

#define WOOD_VERS			0x3AE2
#define MTL_HDR_CHUNK		0x4000
#define WOODVERS1_CHUNK		0x4001
#define WOOD_NOISE_CHUNK	0x4002
#define WD					.02f
#define FACT				500.0f
#define SZ					1.0f //50.0


#define LIM255(c)			(((c)>=255)?255:(c))
#define SIZE				1
#define R1					2
#define R2					3
#define COL1				10		
#define COL2				11
#define NOISE_DIM			20    
#define FNOISE_DIM			20.0

#define SHOW_3DMAPS_WITH_2D

//	C Prototypes
ClassDesc*		GetWoodDesc();
class Wood: public Tex3D 
{ 
	friend class WoodPostLoad;
	static ParamDlg *xyzGenDlg;
	XYZGen *xyzGen;		   // ref #0
	Texmap* subTex[NSUBTEX];  // More refs
	Interval ivalid;
	int rollScroll;
#ifdef SHOW_3DMAPS_WITH_2D
	TexHandle *texHandle;
	Interval texHandleValid;
#endif

	float WoodFunc(Point3 p);

	public:
		BOOL Param1;
		Color col[NCOLS];
		float r1,r2,size;
		BOOL mapOn[NSUBTEX];
		int vers;
		int seed;
		IParamBlock2 *pblock;   // ref #1
		Wood();
		~Wood() {
#ifdef SHOW_3DMAPS_WITH_2D
			DiscardTexHandle();
#endif
			}
		ParamDlg* CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp);
		void Update(TimeValue t, Interval& valid);
		void Init();
		void Reset();
		Interval Validity(TimeValue t) { Interval v; Update(t,v); return ivalid; }

		void SetColor(int i, Color c, TimeValue t);
		void SetSize(float f, TimeValue t);
		void SetR1(float f, TimeValue t);
		void SetR2(float f, TimeValue t);
		void NotifyChanged();
		void SwapInputs(); 

		void ReadSXPData(TCHAR *name, void *sxpdata);

		XYZGen *GetTheXYZGen() { return xyzGen; }

		// Wood noise functions
		void	InitNoise();
		float	WoodNoise(float x);
		void	LerpColor(RGBA *c, RGBA *a, RGBA *b, float f);
		float	SmoothStep(float x0, float x1, float v);

		// Evaluate the color of map for the context.
		RGBA EvalColor(ShadeContext& sc);

		// For Bump mapping, need a perturbation to apply to a normal.
		// Leave it up to the Texmap to determine how to do this.
		Point3 EvalNormalPerturb(ShadeContext& sc);

		// Requirements
		ULONG LocalRequirements(int subMtlNum) { return xyzGen->Requirements(subMtlNum); }
		void LocalMappingsRequired(int subMtlNum, BitArray & mapreq, BitArray &bumpreq) {  
			xyzGen->MappingsRequired(subMtlNum,mapreq,bumpreq); 
			}

		// Methods to access texture maps of material
		int NumSubTexmaps() { return NSUBTEX; }
		Texmap* GetSubTexmap(int i) { return subTex[i]; }
		void SetSubTexmap(int i, Texmap *m);
		TSTR GetSubTexmapSlotName(int i);

		Class_ID ClassID();
		SClass_ID SuperClassID() { return TEXMAP_CLASS_ID; }
		void GetClassName(TSTR& s) { s= GetString(IDS_DS_WOOD); }  
		void DeleteThis() { delete this; }	

		int NumSubs() { return 2+NSUBTEX; }  
		Animatable* SubAnim(int i);
		TSTR SubAnimName(int i);
		int SubNumToRefNum(int subNum) { return subNum; }

		// From ref
 		int NumRefs() { return 2+NSUBTEX; }
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);

		RefTargetHandle Clone(RemapDir &remap = NoRemap());
		RefResult NotifyRefChanged( Interval changeInt, RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message );

#ifdef SHOW_3DMAPS_WITH_2D
		void DiscardTexHandle() {
			if (texHandle) {
				texHandle->DeleteThis();
				texHandle = NULL;
				}
			}
		BOOL SupportTexDisplay() { return TRUE; }
		void ActivateTexDisplay(BOOL onoff) {
			if (!onoff) DiscardTexHandle();
			}
		DWORD_PTR GetActiveTexHandle(TimeValue t, TexHandleMaker& thmaker);
#endif SHOW_3DMAPS_WITH_2D


		// IO
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);
// JBW: direct ParamBlock access is added
		int	NumParamBlocks() { return 1; }					// return number of ParamBlocks in this instance
		IParamBlock2* GetParamBlock(int i) { return pblock; } // return i'th ParamBlock
		IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock->ID() == id) ? pblock : NULL; } // return id'd ParamBlock
		BOOL SetDlgThing(ParamDlg* dlg);

		// Same as Marble
		bool IsLocalOutputMeaningful( ShadeContext& sc ) { return true; }

};

class WoodClassDesc:public ClassDesc2 
{
	public:
	int 			IsPublic() { return GetAppID() != kAPP_VIZR; }
	void *			Create(BOOL loading) { 	return new Wood; }
	const TCHAR *	ClassName() { return GetString(IDS_DS_WOOD_CDESC); } // mjm - 2.10.99
	SClass_ID		SuperClassID() { return TEXMAP_CLASS_ID; }
	Class_ID 		ClassID();
	const TCHAR* 	Category() { return TEXMAP_CAT_3D;  }
// PW: new descriptor data accessors added.  Note that the 
//      internal name is hardwired since it must not be localized.
	const TCHAR*	InternalName() { return _T("wood"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }			// returns owning module handle
};



#pragma pack(1)
struct WoodState 
{
	ulong version;
	float size;
	float dx, dy, dz;
	float r1;
	float r2;
	Col24 col1,col2;
};
#pragma pack()

#endif
