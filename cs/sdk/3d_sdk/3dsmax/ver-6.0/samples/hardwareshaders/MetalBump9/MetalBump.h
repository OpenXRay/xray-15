//_____________________________________________________________________________
//
//	File: MaxShader.h
//	
//
//_____________________________________________________________________________


#ifndef MAXSHADER_H
#define MAXSHADER_H

#if _MSC_VER >= 1000
#pragma once
#endif 


//_____________________________________________________________________________
//
//	Include files	
//
//_____________________________________________________________________________

#include "Max.h"
#include "resource.h"
#include "istdplug.h"
#include "iparamb2.h"
#include "iparamm2.h"
#include "stdmat.h"
#include "imtl.h"
#include "macrorec.h"
#include "ID3D9GraphicsWindow.h"
#include "IStdDualVS.h"
#include "ChannelMap.h"
#include "RenderMesh.h"
#include "Lighting.h"

#include "IViewportManager.h"
#include "IHardwareShader.h"
#include "IHardwareRenderer.h"

//_____________________________________________________________________________
//
//	Forward declare
//
//_____________________________________________________________________________

class ShaderMat;
class MaxShader;
class MaxVertexShader;

//_____________________________________________________________________________
//
//	Defines
//
//_____________________________________________________________________________

#define		MAX_SHADER_CLASS_ID	Class_ID(0x20542fe5, 0x8f74721f)

//_____________________________________
//
//	File Chunks
//
//_____________________________________

#define     CHUNK_STEP				10	

#define		DIFFUSE_CHUNK			5000
#define		NORMAL_CHUNK			5010
#define		SPECULAR_CHUNK			5020
#define		DETAIL_CHUNK			5030
#define		MASK_CHUNK				5040
#define		REFLECTION_CHUNK		5050
#define		BUMP_CHUNK				5060
#define		CHANNEL_CHUNK			5070

#define		COLOR_CHUNK				5100
#define		BUMP_SCALE_CHUNK		5110
#define		MIX_SCALE_CHUNK			5111
#define		ALPHA_CHUNK				5112
#define		MAPPING_CHUNK			5113
#define		REFLECT_SCALE_CHUNK		5114

enum 
{ 
	DIFFUSE_FILE,
	NORMAL_FILE,
	DETAIL_FILE,
	MASK_FILE,
	REFLECTION_FILE,
	BUMP_FILE,

	DIFFUSE_ON,
	NORMAL_ON,
	SPECULAR_ON,
	DETAIL_ON,
	MASK_ON,
	REFLECTION_ON,
	BUMP_ON,

	SPIN_BUMPSCALE,
	SPIN_MIXSCALE,
	SPIN_ALPHA,

	COLOR_AMBIENT,
	COLOR_DIFFUSE,
	COLOR_SPECULAR,

	MAPPING_DIFFUSE1,
	MAPPING_DIFFUSE2,
	MAPPING_SPECULAR,
	MAPPING_BUMP,

	SYNC_MATERIAL,
	SPIN_REFLECTSCALE,
	ALPHA_ON


};

//_____________________________________
//
//	Util
//
//_____________________________________

#define		MAX_MESH_CACHE			100
#define		SLIDER_SCALE			1000.0f


//_____________________________________________________________________________
//
//	Types
//
//_____________________________________________________________________________

extern TCHAR *GetString(int id);

//_____________________________________________________________________________
//
//	Class definitions
//
//_____________________________________________________________________________


//_____________________________________
//
//
//_____________________________________


//_____________________________________
//
//	MaxShaderDlg
//
//_____________________________________

class MaxShaderDlg: public ParamMap2UserDlgProc
{
	MaxShader * shader;
	public:
		MaxShaderDlg(MaxShader * ms){shader = ms;}
		void Update(TimeValue t){};
		BOOL DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void DeleteThis(){delete this;}

};

//_____________________________________
//
//	MaxShaderDAD
//
//_____________________________________

class MaxShaderDAD : public DADMgr
{

	MaxShader *gMaxShader;
	public:
		MaxShaderDAD(MaxShader * ms) {gMaxShader = ms;}
		SClass_ID		GetDragType(HWND hwnd, POINT p) { return BITMAPDAD_CLASS_ID; }
		BOOL			OkToDrop(ReferenceTarget *dropThis, HWND hfrom, HWND hto, POINT p, SClass_ID type, BOOL isNew);
		int				SlotOwner() { return OWNER_MTL_TEX;	}
		ReferenceTarget *GetInstance(HWND hwnd, POINT p, SClass_ID type);
		void			Drop(ReferenceTarget *dropThis, HWND hwnd, POINT p, SClass_ID type);
		BOOL			LetMeHandleLocalDAD() { return 0; } 

		void			Update(TimeValue t);
		BOOL			DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void			DeleteThis(){ delete this;  }	

};


//_____________________________________
//
//	MaxShader
//
//_____________________________________

class MaxShader : public ReferenceTarget, public IDX9DataBridge, TimeChangeCallback {

	public:

		// Parameter block
		MaxShaderDAD		*m_DAD[CHANNEL_MAX];
		IParamBlock2		*m_PBlock;					
		Texmap				*m_SubTex[CHANNEL_MAX];		
		static ParamDlg		*m_UVGenDlg;
		UVGen				*m_UVGen;
		MaxVertexShader	*m_VS;
		int					cubemapSize;
		BitmapInfo			biOutFile;			// for rendering cubic map.
		IMtlParams			*ip;

		ICustButton			*m_Maps[CHANNEL_MAX];

		int MappingCh[4];


		
		// Loading/Saving
		IOResult Load(ILoad *iload);
		IOResult Save(ISave *isave);

		//From Animatable
		Class_ID ClassID()							{ return(MAX_SHADER_CLASS_ID);}		
		SClass_ID SuperClassID()					{ return(REF_TARGET_CLASS_ID); }
		void GetClassName(TSTR& s)					{ s = GetString(IDS_BPSCLASS_NAME);}
		
		RefTargetHandle Clone( RemapDir &remap );
		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
								   PartID& partID,  RefMessage message);

		int NumSubs()								{ return 2; }
		int NumRefs()								{ return 2; }
		Animatable* SubAnim(int i); 
		TSTR SubAnimName(int i);
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);

		int	NumParamBlocks()						{ return 1; }		
		IParamBlock2* GetParamBlock(int i)			{ return m_PBlock; } 
		IParamBlock2* GetParamBlockByID(BlockID id);
		void GetValidity(TimeValue t, Interval &valid);
		void DeleteThis()							{ delete this; }		
		void EnumAuxFiles(NameEnumCallback& nameEnum, DWORD flags);
		//Constructor/Destructor
		MaxShader(BOOL Loading);
		~MaxShader();	

		LPDIRECT3DDEVICE9	GetDevice();

		BaseInterface		*GetInterface(Interface_ID id);
		RMatChannel			ButtonToChannel(int ID);

		ParamDlg * CreateEffectDlg(HWND hWnd, IMtlParams * imp);
		void DisableUI(){};
		void SetDXData(IHardwareMaterial * pHWMtl, Mtl * pMtl);
		TCHAR * GetName(){return _T("Metal Bump");}
		float GetDXVersion() {return 9.0f;}

		void TimeChanged(TimeValue t);
		void OverideMaterial();
		void RenderCubicMap(INode *node);

		int WriteBM(Bitmap *bm, TCHAR *name);
		int GetOutFileName(TSTR &fullname, TSTR &fname, int i);
		bool BrowseOutFile(TSTR &file);
};



//_____________________________________
//
//	IDX8VertexShaderCache
//
//_____________________________________

class IDX8VertexShaderCache : public VertexShaderCache
{
public:
		
};

//_____________________________________
//
//	MaxVertexShader
//
//_____________________________________

class MaxVertexShader : public IDX9VertexShader, public IStdDualVSCallback
{

public:

	IStdDualVS				*m_StdDualVS;
	ReferenceTarget			*m_RTarget;
	LPDIRECT3DDEVICE9		m_Device;
	bool					m_BuildTexture;
		ShaderMat				*m_Material;
	Lighting				m_Lighting;
	int						m_ChannelUse;
	TSTR					m_ChannelName[CHANNEL_MAX];
	bool					m_Draw;
	Color					m_Ambient;
	Color					m_Diffuse;
	Color					m_Specular;
	float					m_BumpScale;
	float					m_MixScale;
	float					m_ReflectScale;
	float					m_Alpha;
	Mtl						*m_Mtl;
	ID3D9GraphicsWindow		*m_GWindow;
	bool					m_Ready;
	TimeValue				m_T;
	int						m_MaxCache;
	int						m_CurCache;
	INode					*m_Node[MAX_MESH_CACHE];
	Mesh					*m_Mesh[MAX_MESH_CACHE];
	RenderMesh				m_RMesh[MAX_MESH_CACHE];
	bool					badDevice;

	MaxVertexShader(ReferenceTarget *rtarg);
	~MaxVertexShader();
	
	HRESULT					Initialize(Mesh *mesh, INode *node);
	HRESULT					Initialize(MNMesh *mnmesh, INode *node);
	// From FPInterface
	LifetimeType			LifetimeControl() { return noRelease; }

	// From IVertexShader
	HRESULT					ConfirmDevice(ID3D9GraphicsWindow *d3dgw);
	HRESULT					ConfirmPixelShader(IDX9PixelShader *pps);
	bool					CanTryStrips();
	int						GetNumMultiPass();
	LPDIRECT3DVERTEXSHADER9					GetVertexShaderHandle(int numPass) { return 0; }
	HRESULT					SetVertexShader(ID3D9GraphicsWindow *d3dgw, int numPass);
	
	// Draw 3D mesh as TriStrips
	bool					DrawMeshStrips(ID3D9GraphicsWindow *d3dgw, MeshData *data);

	// Draw 3D mesh as wireframe
	bool					DrawWireMesh(ID3D9GraphicsWindow *d3dgw, WireMeshData *data);

	// Draw 3D lines
	void					StartLines(ID3D9GraphicsWindow *d3dgw, WireMeshData *data);
	void					AddLine(ID3D9GraphicsWindow *d3dgw, DWORD *vert, int vis);
	bool					DrawLines(ID3D9GraphicsWindow *d3dgw);
	void					EndLines(ID3D9GraphicsWindow *d3dgw, GFX_ESCAPE_FN fn);

	// Draw 3D triangles
	void					StartTriangles(ID3D9GraphicsWindow *d3dgw, MeshFaceData *data);
	void					AddTriangle(ID3D9GraphicsWindow *d3dgw, DWORD index, int *edgeVis);
	bool					DrawTriangles(ID3D9GraphicsWindow *d3dgw);
	void					EndTriangles(ID3D9GraphicsWindow *d3dgw, GFX_ESCAPE_FN fn);

	// from IStdDualVSCallback
	virtual					ReferenceTarget *GetRefTarg();
	virtual					VertexShaderCache *CreateVertexShaderCache();
	virtual HRESULT			InitValid(Mesh* mesh, INode *node);
	virtual HRESULT			InitValid(MNMesh* mnmesh, INode *node);
			HRESULT			SetMatrices();
			HRESULT			SetCubemapMatrices();
			HRESULT			CreateTextures();
	void					SetRenderStates(bool HighFilter);
	void					SetColorStates(TimeValue T);
	void					Draw();
	int						GetMatIndex();
	void					SetMeshCache(Mesh *mesh, INode *node, TimeValue T);
	void					SetCheck();
	void					ResetDXStates();
	HRESULT					InternalCheck();
};

//_____________________________________
//
//	ClassDesc
//
//_____________________________________

class DefaultClassDesc:public ClassDesc2 
{
	public:
	int 			IsPublic()					 { return(1); }
	void *			Create(BOOL loading = FALSE) { return(new MaxShader(loading)); }
	const TCHAR*	ClassName()					 { return(GetString(IDS_BPSCLASS_NAME)); }
	SClass_ID		SuperClassID()				 { return(REF_TARGET_CLASS_ID); }
	Class_ID		ClassID()					 { return(MAX_SHADER_CLASS_ID); }
	const TCHAR* 	Category()					 { return(GetString(IDS_PSCATEGORY)); }
	const TCHAR*	InternalName()				 { return(_T("Metal Bump9")); }	
	HINSTANCE		HInstance()					 { return(hInstance); }				

};

//_____________________________________
//
//	Accessor
//
//_____________________________________

class PSCM_Accessor : public PBAccessor
{
	public:

		void Get(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t, Interval &valid){};
		void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t);

};

//_____________________________________
//
//	Param block Id
//
//_____________________________________

enum
{ 
	gDefaultShaderParams 
};


//_____________________________________
//
//	ColorLerp 
//
//_____________________________________

inline AColor ColorLerp(const AColor &Current, const AColor &Target, float T)
{
	float R,G,B,A;

	A = Current.a + ((Target.a - Current.a) * T);
	R = Current.r + ((Target.r - Current.r) * T);
	G = Current.g + ((Target.g - Current.g) * T);
	B = Current.b + ((Target.b - Current.b) * T);

	return(AColor(R,G,B,A));

}  


#endif 
