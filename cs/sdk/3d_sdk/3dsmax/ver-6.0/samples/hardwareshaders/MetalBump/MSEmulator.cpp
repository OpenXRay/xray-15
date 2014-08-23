/**********************************************************************
 *<
	FILE: MSEmulator.cpp

	DESCRIPTION:	Appwizard generated plugin

	CREATED BY: 

	HISTORY: 

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#include "MSEmulator.h"
#include "MetalBump.h"
#include "ICustAttribContainer.h"
#include "CustAttrib.h"
#include "IViewportManager.h"
#include "ShaderMat.h"

#define MSEMULATOR_CLASS_ID	Class_ID(0x6de34e16, 0x4796bc9a)



#define NSUBTEX		1 // TODO: number of sub-textures supported by this plugin 
#define COORD_REF	1

#define PBLOCK_REF	1

class MSEmulator;



class EmulatorParamDlg: public ParamDlg {
	ReferenceTarget *thing;
	HWND hwnd;
	public:
		EmulatorParamDlg(ReferenceTarget *t, IMtlParams *imp); 
		Class_ID ClassID() { return MSEMULATOR_CLASS_ID; }
		void SetThing(ReferenceTarget *m) {thing = m;}
		ReferenceTarget* GetThing() { return thing; }
		void SetTime(TimeValue t){}
		void ReloadDialog(){}
		void DeleteThis(){ delete this; }
		void ActivateDlg(BOOL onOff){}
	};

static INT_PTR CALLBACK  PanelDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
		// WIN64 Cleanup: Shuler
	return FALSE;
	}

EmulatorParamDlg::EmulatorParamDlg(ReferenceTarget *t, IMtlParams *imp) { 
	thing = t;
	imp->AddRollupPage( 
		hInstance,
		MAKEINTRESOURCE(IDD_EMULATOR),
		PanelDlgProc, _T("Emulator"),
		(LPARAM)this
		);		
	}


class MSEmulator;

class MSEmulatorSampler: public MapSampler {
	MSEmulator	*tex;
	public:
		MSEmulatorSampler() { tex= NULL; }
		MSEmulatorSampler(MSEmulator *c) { tex= c; }
		void Set(MSEmulator *c) { tex = c; }
		AColor Sample(ShadeContext& sc, float u,float v);
		AColor SampleFilter(ShadeContext& sc, float u,float v, float du, float dv);
		float SampleMono(ShadeContext& sc, float u,float v);
		float SampleMonoFilter(ShadeContext& sc, float u,float v, float du, float dv);
	} ;


class MSEmulator : public Texmap {
	public:


		// Parameter block
		Interval		ivalid;
		StdMat2 * parent;
		ReferenceTarget * shader;
		bool active;

		UVGen * uvGen;

		//From MtlBase
		ParamDlg* CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp);
		BOOL SetDlgThing(ParamDlg* dlg);
		void Update(TimeValue t, Interval& valid);
		void Reset();
		Interval Validity(TimeValue t);
		ULONG LocalRequirements(int subMtlNum);

		void LocalMappingsRequired(int subMtlNum, BitArray & mapreq, BitArray &bumpreq) {  
			uvGen->MappingsRequired(subMtlNum,mapreq,bumpreq); 
			}


		//TODO: Return the number of sub-textures
		int NumSubTexmaps() { return 0; }
		//TODO: Return the pointer to the 'i-th' sub-texmap
		Texmap* GetSubTexmap(int i) { return NULL; }
		void SetSubTexmap(int i, Texmap *m);
		TSTR GetSubTexmapSlotName(int i);
		
		//From Texmap
		RGBA EvalColor(ShadeContext& sc);
		float EvalMono(ShadeContext& sc);
		Point3 EvalNormalPerturb(ShadeContext& sc);

		//TODO: Returns TRUE if this texture can be used in the interactive renderer
		BOOL SupportTexDisplay() { return TRUE; }
		void ActivateTexDisplay(BOOL onoff);
		DWORD GetActiveTexHandle(TimeValue t, TexHandleMaker& thmaker);

		int SubNumToRefNum(int subNum) { return subNum; }

		void GetUVTransform(Matrix3 &uvtrans)		{ uvGen->GetUVTransform(uvtrans); }
		//TODO: Return the tiling state of the texture for use in the viewports
		int GetTextureTiling()						{ return(uvGen->GetTextureTiling()); }
		int GetUVWSource()							{ return(uvGen->GetUVWSource()); }
		UVGen *GetTheUVGen()						{ return(uvGen); }

		
		
		// Loading/Saving
		IOResult Load(ILoad *iload);
		IOResult Save(ISave *isave);

		//From Animatable
		Class_ID ClassID() {return MSEMULATOR_CLASS_ID;}		
		SClass_ID SuperClassID() { return TEXMAP_CLASS_ID; }
		void GetClassName(TSTR& s) {s = GetString(IDS_CLASS_NAME);}

		RefTargetHandle Clone( RemapDir &remap );
		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
			PartID& partID,  RefMessage message);


		int NumSubs() { return 0; }
		Animatable* SubAnim(int i); 
		TSTR SubAnimName(int i);

		// TODO: Maintain the number or references here 
		int NumRefs() { return 1; }
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);



		int	NumParamBlocks() { return 0; }					// return number of ParamBlocks in this instance
		IParamBlock2* GetParamBlock(int i) { return NULL; } // return i'th ParamBlock
		IParamBlock2* GetParamBlockByID(BlockID id) { return  NULL; } // return id'd ParamBlock

		void DeleteThis() { delete this; }		
		//Constructor/Destructor

		void FindParentShader();
		MSEmulator();
		~MSEmulator();		

};


class MSEmulatorClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return FALSE; }
	void *			Create(BOOL loading = FALSE) { return new MSEmulator(); }
	const TCHAR *	ClassName() { return GetString(IDS_CLASS_NAME); }
	SClass_ID		SuperClassID() { return TEXMAP_CLASS_ID; }
	Class_ID		ClassID() { return MSEMULATOR_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_CATEGORY); }

	const TCHAR*	InternalName() { return _T("MSEmulator"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle

};

class FindNodesProc : public DependentEnumProc {
	public:
	MSEmulator * tex;
	FindNodesProc(MSEmulator *ms){tex = ms;}
	int proc(ReferenceMaker *rmaker)
		{
		if (rmaker && rmaker->SuperClassID() == MATERIAL_CLASS_ID) {
			tex->parent = (StdMat2*)rmaker;
		}
		return DEP_ENUM_CONTINUE;
		}
	};


static MSEmulatorClassDesc MSEmulatorDesc;
ClassDesc2* GetMSEmulatorDesc() { return &MSEmulatorDesc; }






//--- MSEmulator -------------------------------------------------------

void MSEmulator::FindParentShader()
{
	FindNodesProc findNode(this);
	EnumDependents(&findNode);

	if(parent)
	{
		ICustAttribContainer* cc = parent->GetCustAttribContainer();
		for (int kk = 0; kk < cc->GetNumCustAttribs(); kk++)
		{
			CustAttrib *ca = cc->GetCustAttrib(kk);
			IViewportShaderManager *manager = (IViewportShaderManager*)ca->GetInterface(VIEWPORT_SHADER_MANAGER_INTERFACE);
			if (manager) {
				shader = manager->GetActiveEffect();
				if(shader->ClassID()==Class_ID(0x20542fe5, 0x8f74721f))
					active = true;
				else
					active = false;

			}
		}
	}

}	
MSEmulator::MSEmulator()
{
	parent = NULL;
	shader = NULL;
	uvGen = NULL;
	Reset();
	active = false;
}

MSEmulator::~MSEmulator()
{

}

//From MtlBase
void MSEmulator::Reset() 
{
	if(uvGen)
	{
		uvGen->Reset();
	}
	else 
	{
		ReplaceReference(0,GetNewDefaultUVGen());	
	}


}

void MSEmulator::Update(TimeValue t, Interval& valid) 
{	
	//TODO: Add code to evaluate anything prior to rendering
}

Interval MSEmulator::Validity(TimeValue t)
{
	//TODO: Update ivalid here
	return ivalid;
}

ParamDlg* MSEmulator::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp) 
{
	IAutoMParamDlg* masterDlg = MSEmulatorDesc.CreateParamDlgs(hwMtlEdit, imp, this);
	//TODO: Set the user dialog proc of the param block, and do other initialization	
	return new EmulatorParamDlg(this,imp);	
}

BOOL MSEmulator::SetDlgThing(ParamDlg* dlg)
{	
	return TRUE;
}

void MSEmulator::SetSubTexmap(int i, Texmap *m) 
{
	//TODO Store the 'i-th' sub-texmap managed by the texture
}

TSTR MSEmulator::GetSubTexmapSlotName(int i) 
{	
	//TODO: Return the slot name of the 'i-th' sub-texmap
	return TSTR(_T(""));
}


//From ReferenceMaker
RefTargetHandle MSEmulator::GetReference(int i) 
{
	if(i==0)
		return uvGen;
	else
		return NULL;
}

void MSEmulator::SetReference(int i, RefTargetHandle rtarg) 
{
	if(i==0)
		uvGen = (UVGen*)rtarg;

}

//From ReferenceTarget 
RefTargetHandle MSEmulator::Clone(RemapDir &remap) 
{
	MSEmulator *mnew = new MSEmulator();
	*((MtlBase*)mnew) = *((MtlBase*)this); // copy superclass stuff
	mnew->ReplaceReference(0,remap.CloneRef(uvGen));
	//TODO: Add other cloning stuff
	BaseClone(this, mnew, remap);
	return (RefTargetHandle)mnew;
}

	 
Animatable* MSEmulator::SubAnim(int i) 
{
	return NULL;
}

TSTR MSEmulator::SubAnimName(int i) 
{
	return TSTR("");
}

RefResult MSEmulator::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
   PartID& partID, RefMessage message ) 
{
	//TODO: Handle the reference changed messages here	
	return(REF_SUCCEED);
}

IOResult MSEmulator::Save(ISave *isave) 
{
	//TODO: Add code to allow plugin to save its data
	return IO_OK;
}

IOResult MSEmulator::Load(ILoad *iload) 
{ 
	//TODO: Add code to allow plugin to load its data
	return IO_OK;
}
AColor MSEmulator::EvalColor(ShadeContext &sc)
{
	unsigned long	PakColor;
	AColor			aColor,aColor2;
	Point3			UVW;
	ShaderMat		*Mat;
	float			U,V;
	float			NL,Cos;
	Point3			N,L;
	Color			LightCol;
	LightDesc		*Light;
	int				i;


//	INode *Node = sc.Node();
//	Object *Obj = sc.GetEvalObject();

	FindParentShader();


	if(active)
	{
		MaxShader * ms = (MaxShader * ) shader;

		BOOL sync;
		ms->m_PBlock->GetValue(SYNC_MATERIAL,0,sync,FOREVER);
		

		if(sync)
		{
			Mat			= ms->m_VS->m_Material;
			aColor.r	= 0.58f;
			aColor.g	= 0.58f;
			aColor.b	= 0.58f;
			aColor.a	= 1.0f;

			if(Mat && Mat->m_Shader[CHANNEL_DIFFUSE].IsLoaded())
			{	
				UVW	= sc.UVW();
				U   = -UVW.x;
				V   =  UVW.y;

				Rotate2DPoint(U,V,DEG_RAD(180.0f));

				PakColor = Mat->m_Shader[CHANNEL_DIFFUSE].GetBitmapPixel(U,V);
					
				aColor.r = (((PakColor >> 16) & 0xFF) * 0.00391f);
				aColor.g = (((PakColor >>  8) & 0xFF) * 0.00391f);
				aColor.b = (((PakColor >>  0) & 0xFF) * 0.00391f);
				aColor.a = 1.0f;

				if(Mat->m_Shader[CHANNEL_DETAIL].IsLoaded())
				{
					PakColor = Mat->m_Shader[CHANNEL_DETAIL].GetBitmapPixel(U,V);
						
					aColor2.r = (((PakColor >> 16) & 0xFF) * 0.00391f);
					aColor2.g = (((PakColor >>  8) & 0xFF) * 0.00391f);
					aColor2.b = (((PakColor >>  0) & 0xFF) * 0.00391f);
					aColor2.a = 1.0f;

					aColor = ColorLerp(aColor,aColor2,Mat->m_MixScale);
				}

			}


			N =  sc.Normal();

			Color		Amb	 = Color(Mat->m_Ambient.r,
									 Mat->m_Ambient.g,
									 Mat->m_Ambient.b);

			Color		Diff = Color(0.0f,0.0f,0.0f);

			Color		Tex	 = Color(aColor.r,
									 aColor.g,
									 aColor.b);

			for(i=0; i < sc.nLights; i++)
			{
				Light = sc.Light(i);

				if(!Light->Illuminate(sc,N,LightCol,L,NL,Cos))
				{
					continue;
				}

				Diff += Cos * LightCol * Tex;
			}

			sc.out.t = Color(0.0f,0.0f,0.0f);
			sc.out.c = Amb + Diff;

			return(sc.out.c);
		}
		else
			return AColor (0.0f,0.0f,0.0f,0.0f);

	}
	else
		return AColor (0.0f,0.0f,0.0f,0.0f);

}


float MSEmulator::EvalMono(ShadeContext& sc)
{
	return(Intens(EvalColor(sc)));
}

Point3 MSEmulator::EvalNormalPerturb(ShadeContext& sc)
{
	return(Point3(0,0,0));
}

ULONG MSEmulator::LocalRequirements(int subMtlNum)
{
	return(uvGen->Requirements(subMtlNum)); 
}

void MSEmulator::ActivateTexDisplay(BOOL onoff)
{
}

DWORD MSEmulator::GetActiveTexHandle(TimeValue t, TexHandleMaker& thmaker)
{
	return(NULL);
}
