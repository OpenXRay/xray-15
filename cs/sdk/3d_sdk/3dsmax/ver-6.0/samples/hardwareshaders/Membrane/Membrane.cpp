/**********************************************************************
 *<
	FILE: Membrane.cpp

	DESCRIPTION:	Membrane Lighting Vertex Shader	

	CREATED BY: Neil Hazzard, discreet

	HISTORY: created 1,9,01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "Membrane.h"


class MembraneShader;

static MembraneShaderClassDesc MembraneShaderDesc;
ClassDesc2* GetMembraneShaderDesc() {return &MembraneShaderDesc;}

static ParamBlockDesc2 pixelshader_param_blk ( pixelshader_params, _T("params"),  0, &MembraneShaderDesc, 
	P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF, 
	//rollout
	IDD_BPSPANEL, IDS_PSPARAMS, 0, 0, NULL,
	// params
	pb_spin, 			_T("spin"), 		TYPE_INT, 	P_ANIMATABLE, 	IDS_SPIN, 
		p_default, 		1, 
		p_range, 		1,5, 
		p_ui, 			TYPE_SPINNER,		EDITTYPE_INT, IDC_EDIT,	IDC_SPIN, SPIN_AUTOSCALE, 
		end,
	pb_tv,				_T("tv"),			TYPE_BOOL,	0,	IDS_TV,
		p_default,		FALSE,
		p_ui,			TYPE_SINGLECHEKBOX,	IDC_ANIMATE,
		end,
	pb_coords,			_T("coords"),		TYPE_REFTARG,	P_OWNERS_REF,	IDS_COORDS,
		p_refno,		COORD_REF, 
		end,
	end
	);




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


//--- MembraneShader -------------------------------------------------------
MembraneShader::MembraneShader()
{
	//TODO: Add all the initializing stuff
	pblock = NULL;
	MembraneShaderDesc.MakeAutoParamBlocks(this);
	pvs = new MembraneVertexShader(this);
	pps = new MembranePixelShader(this);
}

MembraneShader::~MembraneShader()
{
	if(pvs)
		delete pvs;
	if(pps)
		delete pps;
}


//From ReferenceMaker
RefTargetHandle MembraneShader::GetReference(int i) 
{
	return pblock;
}

void MembraneShader::SetReference(int i, RefTargetHandle rtarg) 
{
	pblock = (IParamBlock2 *)rtarg; 
}

//From ReferenceTarget 
RefTargetHandle MembraneShader::Clone(RemapDir &remap) 
{
	MembraneShader *mnew = new MembraneShader();
	//TODO: Add other cloning stuff
	mnew->ReplaceReference(0,pblock->Clone(remap));

	return (RefTargetHandle)mnew;
}

ParamDlg * MembraneShader::CreateEffectDlg(HWND hWnd, IMtlParams * imp)
{
	ParamDlg * dlg = MembraneShaderDesc.CreateParamDlgs(hWnd, imp, this);
	return dlg;
}


/*	 
Animatable* MembraneShader::SubAnim(int i) 
{
	//TODO: Return 'i-th' sub-anim
	switch (i) {
		case 0: return uvGen;
		case 1: return pblock;
		default: return subtex[i-2];
		}
}

TSTR MembraneShader::SubAnimName(int i) 
{
	//TODO: Return the sub-anim names
	switch (i) {
		case 0: return GetString(IDS_COORDS);		
		case 1: return GetString(IDS_VSPARAMS);
		default: return GetSubTexmapTVName(i-1);
		}
}
*/

RefResult MembraneShader::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
   PartID& partID, RefMessage message ) 
{
	//TODO: Handle the reference changed messages here	
	return(REF_SUCCEED);
}

IOResult MembraneShader::Save(ISave *isave) 
{
	//TODO: Add code to allow plugin to save its data
	return IO_OK;
}

IOResult MembraneShader::Load(ILoad *iload) 
{ 
	//TODO: Add code to allow plugin to load its data
	return IO_OK;
}

BaseInterface *MembraneShader::GetInterface(Interface_ID id)
{
	if (id == VIEWPORT_SHADER_CLIENT_INTERFACE) {
		return static_cast<IDXDataBridge*>(this);
	}

	else if (id == DX8_PIXEL_SHADER_INTERFACE_ID) {
		return (IDX8PixelShader *)pps;
	}
	else if (id == DX8_VERTEX_SHADER_INTERFACE_ID) {
		return (IDX8VertexShader *)pvs;
	}
	else {
		return BaseInterface::GetInterface(id);
	}
}



MembranePixelShader::MembranePixelShader(MembraneShader *m)
  : pd3dDevice(NULL),
	dwPixelShader(NULL),
	pCubeTexture(NULL),
	map(m)
{
		m_pShadeTexture = NULL;
}

MembranePixelShader::~MembranePixelShader()
{
	SAFE_RELEASE(pCubeTexture);

	if (dwPixelShader) {
		pd3dDevice->DeleteVertexShader(dwPixelShader);
	}

	SAFE_RELEASE(pd3dDevice);
}

void MembranePixelShader::LoadTexture( LPDIRECT3DTEXTURE8 * ppTex, TCHAR * filename )
{
	HRESULT hr;

	if( *ppTex != NULL )
	{
		SAFE_RELEASE( *ppTex );
	}

	//load texture
	TCHAR *cubeMapPath = FindMapFile(filename);
	hr = D3DXCreateTextureFromFile(pd3dDevice, cubeMapPath, ppTex);

	if( !FAILED(hr))
	{
		pd3dDevice->SetTexture(0, *ppTex);
		pd3dDevice->SetTexture(1, *ppTex);
	}
}


HRESULT MembranePixelShader::ConfirmDevice(ID3DGraphicsWindow *d3dgw)
{
	pd3dDevice = d3dgw->GetDevice();
	pd3dDevice->AddRef();

	D3DCAPS8 d3dCaps;
	pd3dDevice->GetDeviceCaps(&d3dCaps);

	if (!(d3dCaps.TextureCaps & D3DPTEXTURECAPS_CUBEMAP))
	{
		return E_FAIL;
	}

	if (!(d3dCaps.TextureCaps & D3DPTEXTURECAPS_PROJECTED))
	{
		return E_FAIL;
	}

	return S_OK;
}

HRESULT MembranePixelShader::ConfirmVertexShader(IDX8VertexShader *pvs)
{
	return S_OK;
}

HRESULT MembranePixelShader::Initialize(Material *mtl, INode *node)
{
	HRESULT hr = S_OK;

	//load textures - not needed here but stops us crashig later due to memory check
	TCHAR *cubeMapPath = FindMapFile("PurpleGreenBright.tga");
	D3DXCreateTextureFromFile(pd3dDevice, cubeMapPath, &m_pShadeTexture);

	return hr;
}

int MembranePixelShader::GetNumMultiPass()
{
	return 1;
}

HRESULT MembranePixelShader::SetPixelShader(ID3DGraphicsWindow *d3dgw, int numPass)
{
	HRESULT hr = S_OK;
	int effect;
	map->pblock->GetValue(pb_spin,0,effect,FOREVER);
	
	pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );

	pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE  );


	effect = effect-1;

	switch( effect )
	{
	case 0:
		LoadTexture( &m_pShadeTexture, "Inv_colors.tga" );
		break;
	case 1:
		LoadTexture( &m_pShadeTexture, "BlueBands.tga" );
		break;
	case 2:
		LoadTexture( &m_pShadeTexture, "YellowRed.tga" );
		break;
	case 3:
		LoadTexture( &m_pShadeTexture, "PurpleGreenBright.tga" );
		break;
	default:
		LoadTexture( &m_pShadeTexture, "PurpleGreenBright.tga" );
		break;
	}

	pd3dDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
	pd3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS );	    // drawing transparent things
	pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

	pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE );	// additive blending
	pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE );
	
	pd3dDevice->SetTexture(0, m_pShadeTexture);
	pd3dDevice->SetTexture(1, m_pShadeTexture);
	
	pd3dDevice->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
	
	pd3dDevice->SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_MIRROR);
	pd3dDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
	pd3dDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
	pd3dDevice->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTEXF_NONE);
	
	pd3dDevice->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);


	return hr;
}


MembraneVertexShader::MembraneVertexShader(ReferenceTarget *rt) : 
	rtarg(rt),
	initDone(false),
	pd3dDevice(NULL),
	pCode(NULL),
	dwVertexShader(NULL),
	pCubeTexture(NULL)
{
	stdDualVS = (IStdDualVS*) CreateInstance(REF_MAKER_CLASS_ID, STD_DUAL_VERTEX_SHADER);
	if(stdDualVS)
		stdDualVS->SetCallback((IStdDualVSCallback*)this);


	m_tc[0]  = 0.0f;
	m_tc[1]  = 0.0f;
	m_tc[2]  = 0.0f;
	m_tc[3]  = 0.0f;



}

MembraneVertexShader::~MembraneVertexShader()
{
	SAFE_RELEASE(pCode);
	SAFE_RELEASE(pCubeTexture);

	if (dwVertexShader) {
		pd3dDevice->DeleteVertexShader(dwVertexShader);
	}

	SAFE_RELEASE(pd3dDevice);
	if(stdDualVS)
		stdDualVS->DeleteThis();
}

HRESULT MembraneVertexShader::Initialize(Mesh *mesh, INode *node)
{
	if(stdDualVS)
		return stdDualVS->Initialize(mesh, node);
	else
		return E_FAIL;
}

HRESULT MembraneVertexShader::Initialize(MNMesh *mnmesh, INode *node)
{
	if(stdDualVS)
		return stdDualVS->Initialize(mnmesh, node);
	else
		return E_FAIL;
}

HRESULT MembraneVertexShader::ConfirmDevice(ID3DGraphicsWindow *d3dgw)
{
	pd3dDevice = d3dgw->GetDevice();
	pd3dDevice->AddRef();

	D3DCAPS8 d3dCaps;
	pd3dDevice->GetDeviceCaps(&d3dCaps);

	if (!(d3dCaps.TextureCaps & D3DPTEXTURECAPS_CUBEMAP))
	{
		return E_FAIL;
	}

	if (!(d3dCaps.TextureCaps & D3DPTEXTURECAPS_PROJECTED))
	{
		return E_FAIL;
	}

	m_pID3Dgw = d3dgw; 

	return S_OK;
}

HRESULT MembraneVertexShader::ConfirmPixelShader(IDX8PixelShader *pps)
{
	return S_OK;
}

ReferenceTarget *MembraneVertexShader::GetRefTarg()
{
	return rtarg;
}

VertexShaderCache *MembraneVertexShader::CreateVertexShaderCache()
{
	return new IDX8VertexShaderCache;
}

void MembraneVertexShader::GetCameraPosition(Point3 &Pos)
{
	Interface *ip = GetCOREInterface();
	INode *pObj;
	Matrix3 objTM;
	Pos.x = 0.0f;
	Pos.y = 0.0f;
	Pos.z = -200.0f;

	INode *pRoot =ip->GetRootNode();
	int numNodes = pRoot->NumberOfChildren();
	for( int ctr = 0; ctr < numNodes; ctr++) {
		pObj = pRoot->GetChildNode(ctr);
		// The ObjectState is a 'thing' that flows down the pipeline containing
		// all information about the object. By calling EvalWorldState() we tell
		// max to eveluate the object at end of the pipeline.
		ObjectState os = pObj->EvalWorldState(ip->GetTime());
		
		// The obj member of ObjectState is the actual object we will export.
		if (os.obj) {
			// We look at the super class ID to determine the type of the object.
			switch(os.obj->SuperClassID()) {
			case CAMERA_CLASS_ID:
				objTM  = pObj->GetNodeTM(ip->GetTime());
				Pos = objTM.GetTrans();
				break;
			}
		}
	}
}

HRESULT MembraneVertexShader::SetVertexShaderMatrices(ID3DGraphicsWindow *d3dgw)
{
	D3DXMATRIX worldITMat, viewITMat;
	D3DXMATRIX matAll, matAllTranspose;
	
	D3DXMATRIX matWorld = d3dgw->GetWorldXform();
	D3DXMATRIX matView = d3dgw->GetViewXform();
	D3DXMATRIX matProj = d3dgw->GetProjXform();

	
	D3DXMatrixInverse(&worldITMat, NULL, &matWorld);
	D3DXMatrixInverse(&viewITMat, NULL, &matView);
	

    D3DXMatrixMultiply( &matAll, &matWorld, &matView );
    D3DXMatrixMultiply( &matAll, &matAll, &matProj );
    D3DXMatrixTranspose( &matAll, &matAll );
    pd3dDevice->SetVertexShaderConstant(0, &matAll, 4 );
	
	pd3dDevice->SetVertexShaderConstant(5, &worldITMat, 4);
	pd3dDevice->SetVertexShaderConstant(11, &matWorld, 4);
	
	return S_OK;
}


HRESULT MembraneVertexShader::InitValid(Mesh *mesh, INode *node)
{
	HRESULT hr = S_OK;

	DebugPrint("DX8 Vertex Shader Init\n");
	
    // Create a vertex shader for doing the cartoon effect
	
    DWORD dwMembraneVertexDecl[] =
    {
        D3DVSD_STREAM( 0 ),
        D3DVSD_REG(D3DVSDE_POSITION, D3DVSDT_FLOAT3),		// v0 = Position
        D3DVSD_REG(D3DVSDE_NORMAL, D3DVSDT_FLOAT3),			// v1 = Normal
		D3DVSD_REG(D3DVSDE_TEXCOORD0, D3DVSDT_FLOAT1),
        D3DVSD_END()
    };

	TCHAR *vertexShaderPath = FindMapFile("membrane.nvv");	
	hr = D3DXAssembleShaderFromFile(vertexShaderPath , 0, NULL, &pCode, NULL);

	if(pCode == NULL)
		return hr;

	hr = pd3dDevice->CreateVertexShader(dwMembraneVertexDecl,(LPDWORD)pCode->GetBufferPointer(), &dwVertexShader, 0);
	
	// select the just created vertex shader as the thing to transform and light vertices
	hr = pd3dDevice->SetVertexShader(dwVertexShader);
	

	

	return hr;

}

HRESULT MembraneVertexShader::InitValid(MNMesh *mnmesh, INode *node)
{
	HRESULT hr = S_OK;

	DebugPrint("DX8 Vertex Shader Init\n");
		
    // Create a vertex shader for doing the cartoon effect
	
    DWORD dwMembraneVertexDecl[] =
    {
        D3DVSD_STREAM( 0 ),
        D3DVSD_REG(D3DVSDE_POSITION, D3DVSDT_FLOAT3),		// v0 = Position
        D3DVSD_REG(D3DVSDE_NORMAL, D3DVSDT_FLOAT3),			// v1 = Normal
		D3DVSD_REG(D3DVSDE_TEXCOORD0, D3DVSDT_FLOAT1),		// v2 = TVs
        D3DVSD_END()
    };

	TCHAR *vertexShaderPath = FindMapFile("membrane.nvv");	
	hr = D3DXAssembleShaderFromFile(vertexShaderPath , 0, NULL, &pCode, NULL);

	if(pCode == NULL)
		return hr;

	hr = pd3dDevice->CreateVertexShader(dwMembraneVertexDecl,(LPDWORD)pCode->GetBufferPointer(), &dwVertexShader, 0);
	
	// select the just created vertex shader as the thing to transform and light vertices
	hr = pd3dDevice->SetVertexShader(dwVertexShader);
	
	return hr;
}

bool MembraneVertexShader::CanTryStrips()
{
	return true;
}

int MembraneVertexShader::GetNumMultiPass()
{
	return 1;
}



HRESULT MembraneVertexShader::SetVertexShader(ID3DGraphicsWindow *d3dgw, int numPass)
{
	
   	HRESULT hr = S_OK;
	MembraneShader * bump = (MembraneShader*)GetRefTarg();
	BOOL cycle;

	bump->pblock->GetValue(pb_tv,0,cycle,FOREVER);

	if( cycle ==TRUE )
	{

		#define TCINC 0.005f
		float inc = TCINC;

		m_tc[0] += inc;
		if( m_tc[0] > 1.0f )
			inc = -TCINC;
		else if( m_tc[0] < 0.0f )
			inc =  TCINC;
	}
	else
		m_tc[0] =0.0f;

	SetVertexShaderMatrices(d3dgw);

	pd3dDevice->SetVertexShaderConstant(20, m_tc, 1);

	pd3dDevice->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
		
	// write the constant constants to constant memory now
	
	pd3dDevice->SetVertexShaderConstant(9, D3DXVECTOR4(0.0f, 0.5f, 1.0f, -1.0f), 1);

	// Lighting is not used in the shader but I guess it could be - Here is how you would get the light

	D3DLIGHT8  light;
	pd3dDevice->GetLight(0,&light);

	D3DXVECTOR4 lightDir(light.Direction.x,light.Direction.y,light.Direction.z,0.0f);

	D3DXVec4Normalize(&lightDir, &lightDir);
	
	pd3dDevice->SetVertexShaderConstant(4, &lightDir, 1);
	
	D3DXVECTOR4 constNums(0.0f, 0.5f, 1.0f, -1.0f);
	pd3dDevice->SetVertexShaderConstant(9, &constNums, 1);
	

	// We get the camera by retrieving the View Transformation.  THis contains the camera position.

	D3DXMATRIX viewITMat;
    D3DXMATRIX matView = d3dgw->GetViewXform();
	D3DXMatrixInverse(&viewITMat, NULL, &matView);

	DebugPrint("x = %f,y = %f, z= %f\n",viewITMat.m[3][0],viewITMat.m[3][1],viewITMat.m[3][2]);
	D3DXVECTOR3 eye;

	// Or we can do it the good old way.

	Point3 pos;
	GetCameraPosition(pos);
//	eye.x    = pos.x; eye.y	  = pos.y; eye.z	   = pos.z;
	eye.x = viewITMat.m[3][0]; eye.y = viewITMat.m[3][1];eye.z = viewITMat.m[3][2];
	pd3dDevice->SetVertexShaderConstant(10, &eye, 1);

	return S_OK;
}

bool MembraneVertexShader::DrawMeshStrips(ID3DGraphicsWindow *d3dgw, MeshData *data)
{
	return false;
}

bool MembraneVertexShader::DrawWireMesh(ID3DGraphicsWindow *d3dgw, WireMeshData *data)
{
	return false;
}

void MembraneVertexShader::StartLines(ID3DGraphicsWindow *d3dgw, WireMeshData *data)
{
}

void MembraneVertexShader::AddLine(ID3DGraphicsWindow *d3dgw, DWORD *vert, int vis)
{
}

bool MembraneVertexShader::DrawLines(ID3DGraphicsWindow *d3dgw)
{
	return false;
}

void MembraneVertexShader::EndLines(ID3DGraphicsWindow *d3dgw, GFX_ESCAPE_FN fn)
{
}

void MembraneVertexShader::StartTriangles(ID3DGraphicsWindow *d3dgw, MeshFaceData *data)
{

}

void MembraneVertexShader::AddTriangle(ID3DGraphicsWindow *d3dgw, DWORD index, int *edgeVis)
{
}

bool MembraneVertexShader::DrawTriangles(ID3DGraphicsWindow *d3dgw)
{
	return false;
}

void MembraneVertexShader::EndTriangles(ID3DGraphicsWindow *d3dgw, GFX_ESCAPE_FN fn)
{

}
