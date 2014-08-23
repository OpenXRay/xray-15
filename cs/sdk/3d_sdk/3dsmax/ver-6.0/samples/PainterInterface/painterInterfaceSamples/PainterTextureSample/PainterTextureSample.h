/**********************************************************************
 *<
	FILE: PainterTextureSample.h

	DESCRIPTION:	Includes for Plugins

	CREATED BY:

	HISTORY:

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#ifndef __PAINTERTEXTURESAMPLE__H
#define __PAINTERTEXTURESAMPLE__H

#include "Max.h"
#include "resource.h"
#include "istdplug.h"
#include "iparamb2.h"
#include "iparamm2.h"

#include "iPainterInterface.h"

#include "stdmat.h"
#include "imtl.h"
#include "macrorec.h"


extern TCHAR *GetString(int id);

extern HINSTANCE hInstance;


#define PAINTERTEXTURESAMPLE_CLASS_ID	Class_ID(0x3e6aeb2b, 0x6d648522)



#define NSUBTEX		1 // TODO: number of sub-textures supported by this plugin 
#define COORD_REF	0

#define PBLOCK_REF	1


enum { paintertexturesample_params };


//TODO: Add enums for various parameters
enum { 
	pb_bitmap,  pb_color,
	pb_coords,
};


class PainterTextureSample;

class PainterTextureSampleSampler: public MapSampler {
	PainterTextureSample	*tex;
	public:
		PainterTextureSampleSampler() { tex= NULL; }
		PainterTextureSampleSampler(PainterTextureSample *c) { tex= c; }
		void Set(PainterTextureSample *c) { tex = c; }
		AColor Sample(ShadeContext& sc, float u,float v);
		AColor SampleFilter(ShadeContext& sc, float u,float v, float du, float dv);
		float SampleMono(ShadeContext& sc, float u,float v);
		float SampleMonoFilter(ShadeContext& sc, float u,float v, float du, float dv);
	} ;


class PainterTextureSample : public Texmap, public IPainterCanvasInterface_V5   {
	public:


		// Parameter block
		IParamBlock2	*pblock;	//ref 0


		Texmap			*subtex[NSUBTEX]; //array of sub-materials
		static ParamDlg *uvGenDlg;
		UVGen			*uvGen;
		Interval		ivalid;

		//From MtlBase
		ParamDlg* CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp);
		BOOL SetDlgThing(ParamDlg* dlg);
		void Update(TimeValue t, Interval& valid);
		void Reset();
		Interval Validity(TimeValue t);
		ULONG LocalRequirements(int subMtlNum);

		//TODO: Return the number of sub-textures
		int NumSubTexmaps() { return NSUBTEX; }
		//TODO: Return the pointer to the 'i-th' sub-texmap
		Texmap* GetSubTexmap(int i) { return subtex[i]; }
		void SetSubTexmap(int i, Texmap *m);
		TSTR GetSubTexmapSlotName(int i);
		
		//From Texmap
		RGBA EvalColor(ShadeContext& sc);
		float EvalMono(ShadeContext& sc);
		Point3 EvalNormalPerturb(ShadeContext& sc);

		//TODO: Returns TRUE if this texture can be used in the interactive renderer
		BOOL SupportTexDisplay() { return TRUE; }
		void ActivateTexDisplay(BOOL onoff);
		void DiscardTexHandle() ;
		BITMAPINFO* GetVPDisplayDIB(TimeValue t, TexHandleMaker& thmaker, Interval &valid, BOOL mono=FALSE, BOOL forceW=0, BOOL forceH=0);
		DWORD GetActiveTexHandle(TimeValue t, TexHandleMaker& thmaker);
		//TODO: Return UV transformation matrix for use in the viewports
		void GetUVTransform(Matrix3 &uvtrans) { uvGen->GetUVTransform(uvtrans); }
		//TODO: Return the tiling state of the texture for use in the viewports
		int GetTextureTiling() { return  uvGen->GetTextureTiling(); }
		int GetUVWSource() { return uvGen->GetUVWSource(); }
		UVGen *GetTheUVGen() { return uvGen; }
		
		//TODO: Return anim index to reference index
		int SubNumToRefNum(int subNum) { return subNum; }
		
		
		// Loading/Saving
		IOResult Load(ILoad *iload);
		IOResult Save(ISave *isave);

		//From Animatable
		Class_ID ClassID() {return PAINTERTEXTURESAMPLE_CLASS_ID;}		
		SClass_ID SuperClassID() { return TEXMAP_CLASS_ID; }
		void GetClassName(TSTR& s) {s = GetString(IDS_CLASS_NAME);}

		RefTargetHandle Clone( RemapDir &remap );
		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
			PartID& partID,  RefMessage message);


		int NumSubs() { return 2+NSUBTEX; }
		Animatable* SubAnim(int i); 
		TSTR SubAnimName(int i);

		// TODO: Maintain the number or references here 
		int NumRefs() { return 2+NSUBTEX; }
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);



		int	NumParamBlocks() { return 1; }					// return number of ParamBlocks in this instance
		IParamBlock2* GetParamBlock(int i) { return pblock; } // return i'th ParamBlock
		IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock->ID() == id) ? pblock : NULL; } // return id'd ParamBlock

		void DeleteThis() { delete this; }		
		//Constructor/Destructor

		PainterTextureSample();
		~PainterTextureSample();	
		
		void* GetInterface(ULONG id);

// These are the IPainterCanvasInterface_V5 you must instatiate
		// This is called when the user tart a pen stroke
		BOOL  StartStroke();

		//This is called as the user strokes across the mesh or screen with the mouse down
		BOOL  PaintStroke(
						  BOOL hit,
						  IPoint2 mousePos, 
						  Point3 worldPoint, Point3 worldNormal,
						  Point3 localPoint, Point3 localNormal,
						  Point3 bary,  int index,
						  BOOL shift, BOOL ctrl, BOOL alt, 
						  float radius, float str,
						  float pressure, INode *node,
						  BOOL mirrorOn,
						  Point3 worldMirrorPoint, Point3 worldMirrorNormal,
						  Point3 localMirrorPoint, Point3 localMirrorNormal
						  ) ;

		// This is called as the user ends a strokes when the users has it set to always update
		BOOL  EndStroke();

		// This is called as the user ends a strokes when the users has it set to update on mouse up only
		// the canvas gets a list of all points, normals etc instead of one at a time
		//		int ct - the number of elements in the following arrays
		//  <...> see paintstroke() these are identical except they are arrays of values
		BOOL  EndStroke(int ct, BOOL *hit, IPoint2 *mousePos, 
						  Point3 *worldPoint, Point3 *worldNormal,
						  Point3 *localPoint, Point3 *localNormal,
						  Point3 *bary,  int *index,
						  BOOL *shift, BOOL *ctrl, BOOL *alt, 
						  float *radius, float *str,
						  float *pressure, INode **node,
						  BOOL mirrorOn,
						  Point3 *worldMirrorPoint, Point3 *worldMirrorNormal,
						  Point3 *localMirrorPoint, Point3 *localMirrorNormal);

		// This is called as the user cancels a stroke by right clicking
		BOOL  CancelStroke();	

		//This is called when the painter want to end a paint session for some reason.
		BOOL  SystemEndPaintSession();

		void PainterDisplay(TimeValue t, ViewExp *vpt, int flags) {}

		void  Paint();
		void  PaintOptions();

		void  InitUI(HWND hWnd);
		void  DestroyUI(HWND hWnd);

	private:
		ICustButton		*iPaintButton;
		IPainterInterface_V5 *pPainter;
		Tab<Point3>		uvwPoints;
		Tab<TVFace>		uvwFaces;

		TexHandle *texHandle;
		Interval texHandleValid;

		Bitmap *bm;
		int width;
		Bitmap *BuildBitmap(int size) ;
		Color ac;
		BMM_Color_64 bit;

		int lagRate, lagCount;

		Tab<Point3> worldSpaceList;
		Tab<Point3> uvwList;
		INode *node;



		void BuildWorldSpaceData(Mesh *msh, Matrix3 tm);
		HWND hwnd;


};


class PaintTextureTestDlgProc : public ParamMap2UserDlgProc {
	public:
		PainterTextureSample *tex;
		PaintTextureTestDlgProc(PainterTextureSample *m) {tex = m;}
		BOOL DlgProc(TimeValue t,IParamMap2* map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void DeleteThis() { delete this;}
	};


class PainterTextureSampleClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return TRUE; }
	void *			Create(BOOL loading = FALSE) { return new PainterTextureSample(); }
	const TCHAR *	ClassName() { return GetString(IDS_CLASS_NAME); }
	SClass_ID		SuperClassID() { return TEXMAP_CLASS_ID; }
	Class_ID		ClassID() { return PAINTERTEXTURESAMPLE_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_CATEGORY); }

	const TCHAR*	InternalName() { return _T("PainterTextureSample"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle

};


#endif // __PAINTERTEXTURESAMPLE__H
