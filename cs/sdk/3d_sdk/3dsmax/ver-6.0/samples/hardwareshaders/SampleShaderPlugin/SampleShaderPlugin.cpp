/*!
 *  \file SampleShaderPlugin.cpp
 *  \brief DESCRIPTION:  Appwizard generated plugin
 *  \brief CREATED BY:  rudy cazabon
 *  \brief HISTORY: 
 *  \brief Discreet - a division of Autodesk, Inc.  
 *  \brief Copyright (c) 2001, All Rights Reserved.
*/

/*! 
	\include SampleShaderPlugin.h
*/
#include "SampleShaderPlugin.h"

// ********************************************************************************
// ********************************************************************************

#define SAMPLESHADERPLUGIN_CLASS_ID	Class_ID(0x5c52a420, 0xea5af83)

#define NSUBTEX		1 // TODO: number of sub-textures supported by this plugin 
#define COORD_REF	0

#define PBLOCK_REF	1

#define SAFE_DELETE(p)			{ if (p) { delete (p);		(p)=NULL; } }
#define SAFE_DELETE_ARRAY(p)	{ if (p) { delete[] (p);	(p)=NULL; } }
#define SAFE_RELEASE(p)			{ if (p) { (p)->Release();	(p)=NULL; } }

extern TCHAR *GetString(int id);

extern HINSTANCE hInstance;

//* ********************************************************************************
/*! ColorVertex Shader
 * This topic shows the steps necessary to initialize and use a simple vertex 
 * shader that uses a position and a diffuse color.
 *
 *
 * The first step is to declare the structures that hold the position and color 
 * as shown in the code example below.
 */
//*********************************************************************************** 
// Surface shader 
// ------------------------------------------------------------------------------
// Vertex type expected by this shader
// struct VERTEX
// {
//   D3DXVECTOR3 p;        // v0 = Vertex position
// };
// 
//  Expected vertex shaders constants
//     c0      = Diffuse color
//     c4-c7   = Transpose of transform matrix set
// 
// ------------------------------------------------------------------------------

const char SurfaceShader[] = 
"vs.1.0						; Shader version 1.0\n"\
"dp4 oPos.x, v0, c4			;                 | c4 |\n"\
"dp4 oPos.y, v0, c5			; [oPos] = [v0] * | c5 |\n"\
"dp4 oPos.z, v0, c6			;                 | c6 |\n"\
"dp4 oPos.w, v0, c7			;                 | c7 |\n"\
"mov oD0, c0				;						\n";


// ********************************************************************************
// ********************************************************************************


class SampleShaderPluginVertexShader;

/* ********************************************************************************
class SampleShaderPluginPixelShader;
******************************************************************************** */


class SampleShaderPlugin;


/*! \class SampleShaderPluginSampler SamplesShaderPlugin.cpp "./SamplesShaderPlugin.cpp"
 *  \brief Due to the idiosyncracies of MEdit an instance of this class needs to exist to
 *  \brief to make MEdit happy.
 */
class SampleShaderPluginSampler: public MapSampler 
{
	SampleShaderPlugin	*tex;

public:

	SampleShaderPluginSampler() 
	{ 
		tex= NULL; 
	}

	SampleShaderPluginSampler(SampleShaderPlugin *c) 
	{ 
		tex= c; 
	}

	void Set(SampleShaderPlugin *c) 
	{ 
		tex = c; 
	}

	AColor Sample(ShadeContext& sc, float u,float v);
	AColor SampleFilter(ShadeContext& sc, float u,float v, float du, float dv);
	float SampleMono(ShadeContext& sc, float u,float v);
	float SampleMonoFilter(ShadeContext& sc, float u,float v, float du, float dv);
};


/*! 
	\class SampleShaderPlugin SamplesShaderPlugin.cpp "./SamplesShaderPlugin.cpp"
	\brief This class is derived from class Texmap.  This is the "staging" area
	\brief where the HW shader is hosted so that it makes an appearance in the 
	\brief Material Editor...basically the UI shell for a hardware shader.
*/

class SampleShaderPlugin : public Texmap 
{
public:

    // Parameter block
    IParamBlock2    *pblock;    // ref 0
    
    SampleShaderPluginVertexShader *pvs;

    Texmap *subtex[NSUBTEX]; // array of sub-materials
    static ParamDlg *uvGenDlg;
    UVGen *uvGen;
    Interval ivalid;
    
    // From MtlBase
    ParamDlg* CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp);
    BOOL SetDlgThing(ParamDlg* dlg);
    void Update(TimeValue t, Interval& valid);
    void Reset();
    Interval Validity(TimeValue t);
    ULONG LocalRequirements(int subMtlNum);
    
    // TODO: Return the number of sub-textures
    int NumSubTexmaps() 
	{ 
		return NSUBTEX; 
	}

    // TODO: Return the pointer to the 'i-th' sub-texmap
    Texmap* GetSubTexmap(int i) 
	{ 
		return subtex[i]; 
	}

    void SetSubTexmap(int i, Texmap *m);
    TSTR GetSubTexmapSlotName(int i);
    
    // From Texmap
    RGBA EvalColor(ShadeContext& sc);
    float EvalMono(ShadeContext& sc);
    Point3 EvalNormalPerturb(ShadeContext& sc);
    
    // TODO: Returns TRUE if this texture can be used in the interactive renderer
    BOOL SupportTexDisplay() 
	{ 
		return TRUE; 
	}

    void ActivateTexDisplay(BOOL onoff);
    DWORD GetActiveTexHandle(TimeValue t, TexHandleMaker& thmaker);
    // TODO: Return UV transformation matrix for use in the viewports
    void GetUVTransform(Matrix3 &uvtrans) 
	{ 
		uvGen->GetUVTransform(uvtrans); 
	}

    // TODO: Return the tiling state of the texture for use in the viewports
    int GetTextureTiling() 
	{ 
		return  uvGen->GetTextureTiling(); 
	}

    int GetUVWSource() 
	{ 
		return uvGen->GetUVWSource(); 
	}

    UVGen *GetTheUVGen() 
	{
		return uvGen; 
	}

    // TODO: Return anim index to reference index
    int SubNumToRefNum(int subNum) 
	{ 
		return subNum; 
	}

    // Loading/Saving
    IOResult Load(ILoad *iload);
    IOResult Save(ISave *isave);
    
    // From Animatable
    Class_ID ClassID() 
	{
		return SAMPLESHADERPLUGIN_CLASS_ID;
	}        

    SClass_ID SuperClassID() 
	{ 
		return TEXMAP_CLASS_ID; 
	}

    void GetClassName(TSTR& s) 
	{
		s = GetString(IDS_CLASS_NAME);
	}

    RefTargetHandle Clone(RemapDir &remap);
    RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID,  RefMessage message);
    
	int NumSubs() 
	{ 
		return 2 + NSUBTEX; 
	}

    Animatable* SubAnim(int i); 
    TSTR SubAnimName(int i);
    
    // TODO: Maintain the number or references here 
    int NumRefs() 
	{ 
		return 2 + NSUBTEX; 
	}

    RefTargetHandle GetReference(int i);
    void SetReference(int i, RefTargetHandle rtarg);
    int NumParamBlocks() 
	{ 
		return 1; 
	}
	// return number of ParamBlocks in this instance
    IParamBlock2* GetParamBlock(int i) 
	{ 
		return pblock; 
	} // return i'th ParamBlock

    IParamBlock2* GetParamBlockByID(BlockID id) 
	{ 
		return (pblock->ID() == id) ? pblock : NULL; 
	} // return id'd ParamBlock

    void DeleteThis() 
	{ 
		delete this; 
	}      
    // Constructor/Destructor
    
    SampleShaderPlugin();
    ~SampleShaderPlugin();      
    
    BaseInterface *GetInterface(Interface_ID id);
};


/*! \class IDX8VertexShaderCache SamplesShaderPlugin.cpp "./SamplesShaderPlugin.cpp"
 *  \brief give it some documentation
 */
class IDX8VertexShaderCache : public VertexShaderCache {
public:
	
};



/*! \class SampleShaderPluginVertexShader SamplesShaderPlugin.cpp "./SamplesShaderPlugin.cpp"
 *  \brief This class inherits from the IDX8VertexShader interface.  In short, this interface
 *  \brief is what allows you to control the specific shader application and mesh drawing.
 */
class SampleShaderPluginVertexShader : public IDX8VertexShader, public IStdDualVSCallback {
	
public:
	bool initDone;

	SampleShaderPluginVertexShader(ReferenceTarget *rtarg);
	~SampleShaderPluginVertexShader();
	
	HRESULT Initialize(Mesh *mesh, INode *node);
	HRESULT Initialize(MNMesh *mnmesh, INode *node);
	// From FPInterface
	LifetimeType	LifetimeControl() { return noRelease; }
	
	// From IVertexShader
	HRESULT ConfirmDevice(ID3DGraphicsWindow *d3dgw);
	
	HRESULT ConfirmPixelShader(IDX8PixelShader *pps);
	
	bool CanTryStrips();
	
	int GetNumMultiPass();
	
	DWORD GetVertexShaderHandle(int numPass) { return dwVertexShader; }
	
	HRESULT SetVertexShader(ID3DGraphicsWindow *d3dgw, int numPass);
	
	// Draw 3D mesh as TriStrips
	bool	DrawMeshStrips(ID3DGraphicsWindow *d3dgw, MeshData *data);
	
	// Draw 3D mesh as wireframe
	bool	DrawWireMesh(ID3DGraphicsWindow *d3dgw, WireMeshData *data);
	
	// Draw 3D lines
	void	StartLines(ID3DGraphicsWindow *d3dgw, WireMeshData *data);
	void	AddLine(ID3DGraphicsWindow *d3dgw, DWORD *vert, int vis);
	bool	DrawLines(ID3DGraphicsWindow *d3dgw);
	void	EndLines(ID3DGraphicsWindow *d3dgw, GFX_ESCAPE_FN fn);
	
	// Draw 3D triangles
	void	StartTriangles(ID3DGraphicsWindow *d3dgw, MeshFaceData *data);
	void	AddTriangle(ID3DGraphicsWindow *d3dgw, DWORD index, int *edgeVis);
	bool	DrawTriangles(ID3DGraphicsWindow *d3dgw);
	void	EndTriangles(ID3DGraphicsWindow *d3dgw, GFX_ESCAPE_FN fn);
	
	// from IStdDualVSCallback
	virtual ReferenceTarget *GetRefTarg();
	virtual VertexShaderCache *CreateVertexShaderCache();
	virtual HRESULT  InitValid(Mesh* mesh, INode *node);
	virtual HRESULT  InitValid(MNMesh* mnmesh, INode *node);

//*! ********************************************************************************
//*! From here down are shader variables...some of them are from the 3ds max side...
//*! some of them are from DirectX 8...there is NO hard-and-fast boundaries at this
//*! time.  25 jun 2001  - rjc : 
//*! ********************************************************************************

/*! 3ds max variables that contain goodies that a shader might want */

	Color m_MtlColor; //!  The material color for the object in question

	INode* m_pINode; //!  pointer to the INode that is going to receive a shading
	
/*! 3ds max implementation shader specific variables */

	ID3DGraphicsWindow* m_pID3Dgw;

	DWORD dwVertexShader;

	IStdDualVS *stdDualVS;
	
	ReferenceTarget *rtarg;
	
	Tab<DWORD> Declaration; //! VertexShader Declarations

/*! 3ds max this plug-in ONLY helper methods */

	void UpdateColor(void *ptr);

/*! DirectX 8 specific variables */



	LPDIRECT3DDEVICE8 pd3dDevice; //!  The actual D3D device
	
	LPD3DXBUFFER pCode;  //! VertexShader Instructions _not_ to be confused with the Vertex Buffer
	
	Tab<D3DXVECTOR4> Constants; //! VertexShader Constants

    D3DXMATRIX m_matWorld; //! Local transform of the model
    D3DXMATRIX m_matLight; //! Transform for the light

};


/*! \class SampleShaderPluginClassDesc SamplesShaderPlugin.cpp "./SamplesShaderPlugin.cpp"
 *  \brief This is the standard 3ds max class descriptor for this plug-in
 */
class SampleShaderPluginClassDesc:public ClassDesc2 {
public:
	int 			IsPublic() { return TRUE; }
	void *			Create(BOOL loading = FALSE) { return new SampleShaderPlugin(); }
	const TCHAR *	ClassName() { return GetString(IDS_CLASS_NAME); }
	SClass_ID		SuperClassID() { return TEXMAP_CLASS_ID; }
	Class_ID		ClassID() { return SAMPLESHADERPLUGIN_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_CATEGORY); }
	
	const TCHAR*	InternalName() { return _T("SampleShaderPlugin"); }  // returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }  // returns owning module handle
	
};



static SampleShaderPluginClassDesc SampleShaderPluginDesc;
ClassDesc2* GetSampleShaderPluginDesc() { return &SampleShaderPluginDesc; }


enum { sampleshaderplugin_params };


//TODO: Add enums for various parameters
enum { 
	pb_spin,
	pb_coords,
	pb_map_on1,
	pb_map1,
};



static ParamBlockDesc2 sampleshaderplugin_param_blk ( sampleshaderplugin_params, _T("params"),0, &SampleShaderPluginDesc, 
P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF, 
//rollout
IDD_PANEL, IDS_PARAMS, 0, 0, NULL,
end
);

	

ParamDlg* SampleShaderPlugin::uvGenDlg;

//--- SampleShaderPlugin -------------------------------------------------------
SampleShaderPlugin::SampleShaderPlugin()
{
	for (int i=0; i<NSUBTEX; i++) subtex[i] = NULL;
	//TODO: Add all the initializing stuff
	pblock = NULL;
	SampleShaderPluginDesc.MakeAutoParamBlocks(this);
	uvGen = NULL;
	pvs = new SampleShaderPluginVertexShader(this);

	Reset();
}

SampleShaderPlugin::~SampleShaderPlugin()
{
	if(pvs)
		delete pvs;
}

//From MtlBase
void SampleShaderPlugin::Reset() 
{
	if (uvGen) uvGen->Reset();
	else ReplaceReference( 0, GetNewDefaultUVGen());	
	//TODO: Reset texmap back to its default values
	
	ivalid.SetEmpty();
	
}

void SampleShaderPlugin::Update(TimeValue t, Interval& valid) 
{	
	//TODO: Add code to evaluate anything prior to rendering
}

Interval SampleShaderPlugin::Validity(TimeValue t)
{
	//TODO: Update ivalid here
	return ivalid;
}

ParamDlg* SampleShaderPlugin::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp) 
{
	IAutoMParamDlg* masterDlg = SampleShaderPluginDesc.CreateParamDlgs(hwMtlEdit, imp, this);
	uvGenDlg = uvGen->CreateParamDlg(hwMtlEdit, imp);
	masterDlg->AddDlg(uvGenDlg);
	//TODO: Set the user dialog proc of the param block, and do other initialization	
	return masterDlg;	
}

BOOL SampleShaderPlugin::SetDlgThing(ParamDlg* dlg)
{	
	if (dlg == uvGenDlg)
		uvGenDlg->SetThing(uvGen);
	else 
		return FALSE;
	return TRUE;
}

void SampleShaderPlugin::SetSubTexmap(int i, Texmap *m) 
{
	ReplaceReference(i+2,m);
	//TODO Store the 'i-th' sub-texmap managed by the texture
}

TSTR SampleShaderPlugin::GetSubTexmapSlotName(int i) 
{	
	//TODO: Return the slot name of the 'i-th' sub-texmap
	return TSTR(_T(""));
}


//From ReferenceMaker
RefTargetHandle SampleShaderPlugin::GetReference(int i) 
{
	//TODO: Return the references based on the index	
	switch (i) {
	case 0: return uvGen;
	case 1: return pblock;
	default: return subtex[i-2];
	}
}

void SampleShaderPlugin::SetReference(int i, RefTargetHandle rtarg) 
{
	//TODO: Store the reference handle passed into its 'i-th' reference
	switch(i) {
	case 0: uvGen = (UVGen *)rtarg; break;
	case 1:	pblock = (IParamBlock2 *)rtarg; break;
	default: subtex[i-2] = (Texmap *)rtarg; break;
	}
}

//From ReferenceTarget 
RefTargetHandle SampleShaderPlugin::Clone(RemapDir &remap) 
{
	SampleShaderPlugin *mnew = new SampleShaderPlugin();
	*((MtlBase*)mnew) = *((MtlBase*)this); // copy superclass stuff
	//TODO: Add other cloning stuff
	BaseClone(this, mnew, remap);
	return (RefTargetHandle)mnew;
}


Animatable* SampleShaderPlugin::SubAnim(int i) 
{
	//TODO: Return 'i-th' sub-anim
	switch (i) {
	case 0: return uvGen;
	case 1: return pblock;
	default: return subtex[i-2];
	}
}

TSTR SampleShaderPlugin::SubAnimName(int i) 
{
	//TODO: Return the sub-anim names
	switch (i) {
	case 0: return GetString(IDS_COORDS);		
	case 1: return GetString(IDS_PARAMS);
	default: return GetSubTexmapTVName(i-1);
	}
}

RefResult SampleShaderPlugin::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
											   PartID& partID, RefMessage message ) 
{
	//TODO: Handle the reference changed messages here	
	return(REF_SUCCEED);
}

IOResult SampleShaderPlugin::Save(ISave *isave) 
{
	//TODO: Add code to allow plugin to save its data
	return IO_OK;
}

IOResult SampleShaderPlugin::Load(ILoad *iload) 
{ 
	//TODO: Add code to allow plugin to load its data
	return IO_OK;
}

AColor SampleShaderPlugin::EvalColor(ShadeContext& sc)
{
	//TODO: Evaluate the color of texture map for the context.
	return AColor (0.0f,0.0f,0.0f,0.0f);
}

float SampleShaderPlugin::EvalMono(ShadeContext& sc)
{
	//TODO: Evaluate the map for a "mono" channel
	return Intens(EvalColor(sc));
}

Point3 SampleShaderPlugin::EvalNormalPerturb(ShadeContext& sc)
{
	//TODO: Return the perturbation to apply to a normal for bump mapping
	return Point3(0, 0, 0);
}

ULONG SampleShaderPlugin::LocalRequirements(int subMtlNum)
{
	//TODO: Specify various requirements for the material
	return uvGen->Requirements(subMtlNum); 
}

void SampleShaderPlugin::ActivateTexDisplay(BOOL onoff)
{
	//TODO: Implement this only if SupportTexDisplay() returns TRUE
}

DWORD SampleShaderPlugin::GetActiveTexHandle(TimeValue t, TexHandleMaker& thmaker)
{
	//TODO: Return the texture handle to this texture map
	return NULL;
}

BaseInterface *SampleShaderPlugin::GetInterface(Interface_ID id)
{
	
	if (id == DX8_VERTEX_SHADER_INTERFACE_ID) {
		return (IDX8VertexShader *)pvs;
	}
	else {
		return Texmap::GetInterface(id);
	}
	
}



SampleShaderPluginVertexShader::SampleShaderPluginVertexShader(ReferenceTarget *rt) : 
	rtarg(rt),
	initDone(false),
	pd3dDevice(NULL),
	pCode(NULL),
	dwVertexShader(NULL),
	m_MtlColor(1.f, 0.f, 1.f)
{
	stdDualVS = (IStdDualVS*) CreateInstance(REF_MAKER_CLASS_ID, STD_DUAL_VERTEX_SHADER);

	if(stdDualVS)
		stdDualVS->SetCallback((IStdDualVSCallback*)this);
	
	rtarg = rt;
	pd3dDevice = NULL;
	dwVertexShader = NULL;
	
	
}

SampleShaderPluginVertexShader::~SampleShaderPluginVertexShader()
{
	
	if (dwVertexShader) {
		pd3dDevice->DeleteVertexShader(dwVertexShader);
	}
	
	SAFE_RELEASE(pd3dDevice);
	if(stdDualVS)
		stdDualVS->DeleteThis();
}

HRESULT SampleShaderPluginVertexShader::Initialize(Mesh *mesh, INode *node)
{
	if(stdDualVS)
		return stdDualVS->Initialize(mesh, node);
	else
		return E_FAIL;
}

HRESULT SampleShaderPluginVertexShader::Initialize(MNMesh *mnmesh, INode *node)
{
	if(stdDualVS)
		return stdDualVS->Initialize(mnmesh, node);
	else
		return E_FAIL;
}

HRESULT SampleShaderPluginVertexShader::ConfirmDevice(ID3DGraphicsWindow *d3dgw)
{
	pd3dDevice = d3dgw->GetDevice();
	pd3dDevice->AddRef();
	
	D3DCAPS8 d3dCaps;
	pd3dDevice->GetDeviceCaps(&d3dCaps);
	
	return S_OK;
}

HRESULT SampleShaderPluginVertexShader::ConfirmPixelShader(IDX8PixelShader *pps)
{
	return S_OK;
}

ReferenceTarget *SampleShaderPluginVertexShader::GetRefTarg()
{
	return rtarg;
}

VertexShaderCache *SampleShaderPluginVertexShader::CreateVertexShaderCache()
{
	return new IDX8VertexShaderCache;
}


TCHAR *FindMapFile(TCHAR *file)
{
	BOOL found = 0;

	static TCHAR  filepath[MAX_PATH];
	TCHAR* fpart;

	for (int i = 0; i < GetCOREInterface()->GetMapDirCount(); i++)
		if ((found = SearchPath(GetCOREInterface()->GetMapDir(i), file, NULL, MAX_PATH, filepath, &fpart)))
				break;
	if(found)
		return filepath;
	else
		return NULL;
}

// *********************************************************************************
//! Function name	: SampleShaderPluginVertexShader::InitValid
//! Description	    : 
//! Return type		: HRESULT 
//! Argument        : Mesh *mesh
//! Argument        : INode *node

HRESULT SampleShaderPluginVertexShader::InitValid(Mesh *mesh, INode *node) 
{
	
	HRESULT hr = S_OK;
	m_pINode = node;
	
    // Create a vertex shader for doing the effect
	
	LPD3DXBUFFER *ppCode = &pCode;
	
    LPD3DXBUFFER pBuffer = NULL;
	
	//! VertexShader Declarations
	
	
	//! VertexShader Constants
	Constants.SetCount(20);
	
	//! set up the material vertex color ... 
    D3DMATERIAL8 mtrl;
	ZeroMemory( &mtrl, sizeof(D3DMATERIAL8) );
	
	mtrl.Diffuse.r = mtrl.Ambient.r = m_MtlColor.r;
	mtrl.Diffuse.g = mtrl.Ambient.g = m_MtlColor.g;
	mtrl.Diffuse.b = mtrl.Ambient.b = m_MtlColor.b;
	mtrl.Diffuse.a = mtrl.Ambient.a = 1.0f;
    pd3dDevice->SetMaterial( &mtrl );
    pd3dDevice->SetRenderState( D3DRS_LIGHTING, TRUE );
    pd3dDevice->SetRenderState( D3DRS_AMBIENT, 0xffffffff );	
	/*!
	* D3DXAssembleShader
	* Assembles an ASCII description of a shader into binary form, where the shader source is in memory.
	*
	* @param pSrcData : [in] Pointer to the source code. 
	* @param SrcDataLen : [in] Size of the source code, in bytes. 
	* @param Flags : [in] A combination of the following flags, specifying assembly options. 
	* D3DXASM_DEBUG Inserts debugging information as comments in the assembled shader. 
	* D3DXASM_SKIPVALIDATION Do not validate the generated code against known capabilities and constraints. This option is recommended only when assembling a shader you know will function (that is, the shader has been assembled before without this option.) 
	* @param ppConstants : [out] Returns a pointer to an ID3DXBuffer interface, representing the returned constant declarations. These constants are returned as a vertex shader declaration fragment. It is up to the application to insert the contents of this buffer into their declaration. For pixel shaders this parameter is meaningless because constant declarations are included in the assembled shader. This parameter is ignored if it is NULL. 
	* @param ppCompiledShader : [out] Returns a pointer to an ID3DXBuffer interface, representing the returned compiled object code. This parameter is ignored if it is NULL. 
	* @param ppCompilationErrors : [out] Returns a pointer to an ID3DXBuffer interface, representing the returned ASCII error messages. This parameter is ignored if it is NULL. 
	*
	* @return HRESULT  : 
	
	  HRESULT D3DXAssembleShader(
	  LPCVOID pSrcData,
	  UINT SrcDataLen,
	  DWORD Flags,
	  LPD3DXBUFFER* ppConstants,
	  LPD3DXBUFFER* ppCompiledShader,
	  LPD3DXBUFFER* ppCompilationErrors
	  );
	  
	*/
    //! Specify the vertex format that the vertex shader will be using for doing the effect
    DWORD dwVertexFormatDefinition[] =
    {
        D3DVSD_STREAM( 0 ),	D3DVSD_REG( 0, D3DVSDT_FLOAT3 ),     // v0 = Position
		D3DVSD_REG( 1, D3DVSDT_FLOAT3 ),     // v1 = Normal
		D3DVSD_END()
    };	
	
#if 0
	TCHAR *vertexShaderPath = FindMapFile("ambient.njv");	
	hr = D3DXAssembleShaderFromFile(vertexShaderPath , 0, NULL, &pCode, NULL);
#else
	hr = D3DXAssembleShader( SurfaceShader , sizeof(SurfaceShader)-1 , 0 , NULL , &pCode , NULL ); 
#endif
	
	hr = pd3dDevice->CreateVertexShader(dwVertexFormatDefinition,(LPDWORD)pCode->GetBufferPointer(), &dwVertexShader, 0);
	
	initDone = true;
	
	return hr;
	
}

HRESULT SampleShaderPluginVertexShader::InitValid(MNMesh *mnmesh, INode *node)
{
	HRESULT hr = S_OK;
	return hr;
}

bool SampleShaderPluginVertexShader::CanTryStrips()
{
	return true;
}

int SampleShaderPluginVertexShader::GetNumMultiPass()
{
	return 1;
}




/*!
 * this is the heart of the application and user-controlled drawing of a 
 * 3ds max mesh with a specific shader.
 *
 * @param *d3dgw : 
 * @param numPass : 
 *
 * @return HRESULT  : 
 */
HRESULT SampleShaderPluginVertexShader::SetVertexShader(ID3DGraphicsWindow *d3dgw, int numPass)
{
	
   	HRESULT hr = S_OK;

	// Set matrices for the vertex shader
	D3DXMATRIX matWorld, matView, matProj, matAll, matAllTranspose;
	pd3dDevice->GetTransform( D3DTS_WORLD,&matWorld );
	pd3dDevice->GetTransform( D3DTS_VIEW,&matView );
	pd3dDevice->GetTransform( D3DTS_PROJECTION,&matProj );
	D3DXMatrixMultiply( &matAll, &matWorld,&matView );
	D3DXMatrixMultiply( &matAll, &matAll,&matProj );
	D3DXMatrixTranspose( &matAll, &matAll );
	pd3dDevice->SetVertexShaderConstant( 4, &matAll, 4 );
	
	// ********************************************************************************
	// Setting up the stages and vertex shaders per pass
	// ********************************************************************************

		// Set default states
		pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
		pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
		pd3dDevice->SetRenderState( D3DRS_ZENABLE,     TRUE );
		pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
		pd3dDevice->SetRenderState( D3DRS_ALPHATESTENABLE,  FALSE );
		pd3dDevice->SetRenderState( D3DRS_LIGHTING,    TRUE );
		pd3dDevice->SetRenderState( D3DRS_AMBIENT,     0xffffffff );
		// Set filtering to point sampling; otherwise we lose the sharp
		// transistions in the lighting that we wanted.
		pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_POINT );
		pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_POINT );
		
		// Set the texture coordinates to be clamped so we do not have to clamp the
		// result of the dot product in the texture gen code. We only really need
		// to clamp U because V is fixed at 0.0.
		pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP );
		pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP );
		
		// Draw the first pass, the textured visible polygons. We want to modulate 
		// the texture with the diffuse at stage 0. The texture is a banded gray 
		// scale to give the lighting and the diffuse is used as specified in the
		// vertex because vertex lighting is disabled. This is a useful way to set
		// things up because the base color can be changed without having to
		// change render state and therefore without having to break primitives.
		pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
		pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
		pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT1 );
		

		pd3dDevice->SetVertexShader( dwVertexShader );
		
		// Set constants for the vertex shader
		D3DXVECTOR4 vColor( m_MtlColor.r, m_MtlColor.g,m_MtlColor.b, 0.0f );
		pd3dDevice->SetVertexShaderConstant( 0, &vColor, 1 );

	return S_OK;
}

bool SampleShaderPluginVertexShader::DrawMeshStrips(ID3DGraphicsWindow *d3dgw, MeshData *data)
{
	return false;
}

bool SampleShaderPluginVertexShader::DrawWireMesh(ID3DGraphicsWindow *d3dgw, WireMeshData *data)
{
	return false;
}

void SampleShaderPluginVertexShader::StartLines(ID3DGraphicsWindow *d3dgw, WireMeshData *data)
{
}

void SampleShaderPluginVertexShader::AddLine(ID3DGraphicsWindow *d3dgw, DWORD *vert, int vis)
{
}

bool SampleShaderPluginVertexShader::DrawLines(ID3DGraphicsWindow *d3dgw)
{
	return false;
}

void SampleShaderPluginVertexShader::EndLines(ID3DGraphicsWindow *d3dgw, GFX_ESCAPE_FN fn)
{
}

void SampleShaderPluginVertexShader::StartTriangles(ID3DGraphicsWindow *d3dgw, MeshFaceData *data)
{
	
}

void SampleShaderPluginVertexShader::AddTriangle(ID3DGraphicsWindow *d3dgw, DWORD index, int *edgeVis)
{
}

bool SampleShaderPluginVertexShader::DrawTriangles(ID3DGraphicsWindow *d3dgw)
{
	return false;
}

void SampleShaderPluginVertexShader::EndTriangles(ID3DGraphicsWindow *d3dgw, GFX_ESCAPE_FN fn)
{
	
}


