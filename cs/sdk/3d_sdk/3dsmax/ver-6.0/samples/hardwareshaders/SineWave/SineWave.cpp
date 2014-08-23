/**********************************************************************
 *<
	FILE: SineWaveMod.cpp

	DESCRIPTION: SineWave XTC Modifier

	CREATED BY: Neil Hazzard, Discreet

	HISTORY: Created 1/4/2001

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#include "SineWave.h"


static SineWaveModClassDesc SineWaveModDesc;
ClassDesc2* GetSineWaveModDesc() {return &SineWaveModDesc;}

enum { SineWaveMod_params };

//TODO: Add enums for various parameters
enum { 
	pb_amount,
	pb_height,
	pb_normals,
	pb_wireframe,
};

static ParamBlockDesc2 SineWaveMod_param_blk ( SineWaveMod_params, _T("params"),  0, &SineWaveModDesc, 
	P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF, 
	//rollout
	IDD_VSPANEL, IDS_VSPARAMS, 0, 0, NULL,
	// params
	pb_amount, 			_T("amount"), 		TYPE_FLOAT, 	P_ANIMATABLE, 	IDS_AMOUNT, 
		p_default, 		0.0f, 
		p_range, 		0.0f,10.0f, 
		p_ui, 			TYPE_SPINNER,		EDITTYPE_FLOAT, IDC_EDIT,	IDC_SPIN, 0.01f, 
		end,
	pb_height, 			_T("height"), 		TYPE_FLOAT, 	P_ANIMATABLE, 	IDS_HEIGHT, 
		p_default, 		3.5f, 
		p_range, 		0.0f,10.0f, 
		p_ui, 			TYPE_SPINNER,		EDITTYPE_FLOAT, IDC_HEIGHTEDIT,	IDC_HEIGHTSPIN, 0.01f, 
		end,

	pb_normals, 		_T("normals"), 		TYPE_BOOL, 	0, 	IDS_NORMAL, 
		p_default, 		FALSE, 
		p_ui, 			TYPE_SINGLECHEKBOX,	IDC_NORMAL,	
		end,

	pb_wireframe, 		_T("wireframe"), 		TYPE_BOOL, 	0, 	IDS_WIREFRAME, 
		p_default, 		FALSE, 
		p_ui, 			TYPE_SINGLECHEKBOX,	IDC_WIREFRAME,	
		end,


	end
	);



IObjParam *SineWaveMod::ip			= NULL;


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

//--- SineWaveMod -------------------------------------------------------
SineWaveMod::SineWaveMod()
{
	SineWaveModDesc.MakeAutoParamBlocks(this);
}

SineWaveMod::~SineWaveMod()
{
}


Interval SineWaveMod::LocalValidity(TimeValue t)
{
	if (TestAFlag(A_MOD_BEING_EDITED))
		return NEVER; 
	else
	{
		float amount;
		BOOL norm;
		Interval ivalid = FOREVER;
		pblock->GetValue(pb_amount,t,amount,ivalid);
		pblock->GetValue(pb_height,t,amount,ivalid);
		pblock->GetValue(pb_normals,t,norm,ivalid);
		return ivalid;
	}
}

Interval SineWaveMod::GetValidity(TimeValue t) 
{
	Interval ivalid = FOREVER;
	float f;
	BOOL norm;
	pblock->GetValue(pb_amount,t,f, ivalid);
	pblock->GetValue(pb_height,t,f,ivalid);
	pblock->GetValue(pb_normals,t,norm, ivalid);
	return ivalid;
}

RefTargetHandle SineWaveMod::Clone(RemapDir& remap)
{
	SineWaveMod* newmod = new SineWaveMod();	
	//TODO: Add the cloning code here
	newmod->ReplaceReference(0,pblock->Clone(remap));
	BaseClone(this, newmod, remap);
	return(newmod);
}

void SineWaveMod::ModifyObject(TimeValue t, ModContext &mc, ObjectState * os, INode *node) 
{
	float amount,height;
	BOOL norm,wire;
	Interval ivalid = FOREVER;
	if(!mc.localData)
	{
		mc.localData = new 	SineWaveModData;
	}
	
	((SineWaveModData*)mc.localData)->SetValid(false);

	pblock->GetValue(pb_amount,t,amount,ivalid);
	pblock->GetValue(pb_height,t,height,ivalid);
	pblock->GetValue(pb_normals,t,norm, ivalid);
	pblock->GetValue(pb_wireframe,t,wire,ivalid);

	XTCVShader *pObj = new XTCVShader((SineWaveModData*)mc.localData,amount,norm,height,wire);

	os->obj->AddXTCObject(pObj);
		
	os->obj->SetChannelValidity(EXTENSION_CHAN_NUM, GetValidity(t));

}

void SineWaveMod::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
{
	this->ip = ip;
	SineWaveModDesc.BeginEditParams(ip, this, flags, prev);
}

void SineWaveMod::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next)
{
	SineWaveModDesc.EndEditParams(ip, this, flags, next);
	this->ip = NULL;
}


//From ReferenceMaker 
RefResult SineWaveMod::NotifyRefChanged(
		Interval changeInt, RefTargetHandle hTarget,
		PartID& partID,  RefMessage message) 
{
	//TODO: Add code to handle the various reference changed messages
	return REF_SUCCEED;
}

//From Object
BOOL SineWaveMod::HasUVW() 
{ 
	//TODO: Return whether the object has UVW coordinates or not
	return TRUE; 
}

void SineWaveMod::SetGenUVW(BOOL sw) 
{  
	if (sw==HasUVW()) return;
	//TODO: Set the plugin internal value to sw				
}

IOResult SineWaveMod::Load(ILoad *iload)
{
	//TODO: Add code to allow plugin to load its data
	
	return IO_OK;
}

IOResult SineWaveMod::Save(ISave *isave)
{
	//TODO: Add code to allow plugin to save its data
	
	return IO_OK;
}

//--- VShaderModData -------------------------------------------------------

void SineWaveModData::SetValid(bool valid)
{
	bValid = valid;
}

bool SineWaveModData::GetValid()
{
	return bValid;
}
//--- XTCVShader -------------------------------------------------------

void XTCVShader::PostChanChangedNotify(TimeValue t, ModContext &mc, ObjectState* os, INode *node,Modifier *mod, bool bEndOfPipeline)
{
	lmd->SetValid(false);
	if (!bEndOfPipeline) {
		return;
	}
}

XTCVShader::XTCVShader(SineWaveModData *lmd,float amount,BOOL norm,float height,BOOL wire) 
{
	this->lmd = lmd;
	pvs = new IDX8SineWave(lmd,amount,norm,height,wire);
}

XTCVShader::XTCVShader(XTCVShader *mFrom)
{
	this->lmd = mFrom->lmd;
	pvs = new IDX8SineWave(mFrom->pvs);
}

XTCVShader::~XTCVShader()
{
	if(pvs)
		delete pvs;
}

//--- IDX8SineWave -------------------------------------------------------

BaseInterface *IDX8SineWave::CloneInterface(void* remapDir)
{
	return (BaseInterface*) new IDX8SineWave(this);
}

IDX8SineWave::IDX8SineWave(SineWaveModData *pLMD,float amount,BOOL norm,float height,BOOL wire)
  : lmd(pLMD),
	initDone(false),
	pd3dDevice(NULL),
	pCode(NULL),
	dwVertexShader(NULL),
	dwPixelShader(NULL),
	pCubeTexture(NULL)
{
	
	AmountConstant = amount;
	Normals = norm;
	Height = height;
	Wire =  wire;
}

IDX8SineWave::IDX8SineWave(IDX8SineWave *from)
{
	lmd = from->lmd;
	pd3dDevice = from->pd3dDevice;
	pCode = from->pCode;
	dwVertexShader = from->dwVertexShader;
	dwPixelShader = from->dwPixelShader;
	pCubeTexture = from->pCubeTexture;
	AmountConstant =  from->AmountConstant;
	Normals = from->Normals;
	Height = from->Height;
	Wire = from->Wire;


}

IDX8SineWave::~IDX8SineWave()
{
	SAFE_RELEASE(pCode);
	SAFE_RELEASE(pCubeTexture);
	if (dwVertexShader) {
		pd3dDevice->DeleteVertexShader(dwVertexShader);
	}

	SAFE_RELEASE(pd3dDevice);
}

HRESULT IDX8SineWave::ConfirmDevice(ID3DGraphicsWindow *d3dgw)
{
	pd3dDevice = d3dgw->GetDevice();
	pd3dDevice->AddRef();

	testgw = d3dgw;

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

HRESULT IDX8SineWave::ConfirmPixelShader(IDX8PixelShader *pps)
{
	return S_OK;
}

HRESULT IDX8SineWave::Initialize(Mesh *mesh, INode *node)
{
	HRESULT hr = S_OK;

	if (!lmd->GetValid()) {
		// do initialization here 
		DebugPrint("DX8 Vertex Shader Init\n");

		// VertexShader Instructions
		DWORD dwDecl[] =
		{
			D3DVSD_STREAM(0),
			D3DVSD_REG(0, D3DVSDT_FLOAT3),
			D3DVSD_END()
		};

		LPD3DXBUFFER *ppCode = &pCode;

		// VertexShader Declarations
		TCHAR *vertexShaderPath = FindMapFile("sinewave.njv");
		

		hr = D3DXAssembleShaderFromFile(vertexShaderPath, 0, NULL, &pCode, NULL);

		// VertexShader Constants
		Constants.SetCount(20);

		Constants[CV_ZERO] = D3DXVECTOR4(0.0f,0.0f,0.0f,0.0f);
		Constants[CV_ONE] = D3DXVECTOR4(1.0f,1.0f,1.0f,1.0f);
		Constants[CV_HALF] = D3DXVECTOR4(0.5f,0.5f,0.5f,0.5f);

		hr = pd3dDevice->CreateVertexShader(dwDecl,
			(LPDWORD)pCode->GetBufferPointer(), &dwVertexShader, 0);

		pd3dDevice->SetVertexShader(dwVertexShader);



		
		TCHAR *cubeMapPath = FindMapFile("sky_cube_mipmap.dds");

		hr = D3DXCreateCubeTextureFromFileEx(pd3dDevice,
			cubeMapPath, 
			D3DX_DEFAULT,
			1,
			0,
			D3DFMT_UNKNOWN,
			D3DPOOL_MANAGED,
			D3DX_FILTER_LINEAR,
			D3DX_FILTER_LINEAR,
			0,
			NULL,
			NULL,
			&pCubeTexture);

		initDone = true;
		pd3dDevice->SetTexture(0, pCubeTexture);
		pd3dDevice->SetVertexShaderConstant(CV_ZERO, Constants[CV_ZERO], 1);
		pd3dDevice->SetVertexShaderConstant(CV_ONE, Constants[CV_ONE], 1);
		pd3dDevice->SetVertexShaderConstant(CV_HALF, Constants[CV_HALF], 1);

		lmd->SetValid(true);
	}

	return hr;
}


HRESULT IDX8SineWave::Initialize(MNMesh *mnmesh, INode *node)
{
	HRESULT hr = S_OK;

	if (!lmd->GetValid()) {
		// do initialization here 
		DebugPrint("DX8 Vertex Shader Init\n");

		// VertexShader Instructions
		DWORD dwDecl[] =
		{
			D3DVSD_STREAM(0),
			D3DVSD_REG(0, D3DVSDT_FLOAT3),
			D3DVSD_END()
		};

		LPD3DXBUFFER *ppCode = &pCode;

		// VertexShader Declarations
		TCHAR *vertexShaderPath = FindMapFile("sinewave.njv");
		

		hr = D3DXAssembleShaderFromFile(vertexShaderPath, 0, NULL, &pCode, NULL);

		// VertexShader Constants
		Constants.SetCount(20);

		Constants[CV_ZERO] = D3DXVECTOR4(0.0f,0.0f,0.0f,0.0f);
		Constants[CV_ONE] = D3DXVECTOR4(1.0f,1.0f,1.0f,1.0f);
		Constants[CV_HALF] = D3DXVECTOR4(0.5f,0.5f,0.5f,0.5f);

		hr = pd3dDevice->CreateVertexShader(dwDecl,
			(LPDWORD)pCode->GetBufferPointer(), &dwVertexShader, 0);

		pd3dDevice->SetVertexShader(dwVertexShader);



		
		TCHAR *cubeMapPath = FindMapFile("sky_cube_mipmap.dds");

		hr = D3DXCreateCubeTextureFromFileEx(pd3dDevice,
			cubeMapPath, 
			D3DX_DEFAULT,
			1,
			0,
			D3DFMT_UNKNOWN,
			D3DPOOL_MANAGED,
			D3DX_FILTER_LINEAR,
			D3DX_FILTER_LINEAR,
			0,
			NULL,
			NULL,
			&pCubeTexture);

		initDone = true;
		pd3dDevice->SetTexture(0, pCubeTexture);
		pd3dDevice->SetVertexShaderConstant(CV_ZERO, Constants[CV_ZERO], 1);
		pd3dDevice->SetVertexShaderConstant(CV_ONE, Constants[CV_ONE], 1);
		pd3dDevice->SetVertexShaderConstant(CV_HALF, Constants[CV_HALF], 1);

		lmd->SetValid(true);
	}
	return hr;
}

bool IDX8SineWave::CanTryStrips()
{
	return true;
}

int IDX8SineWave::GetNumMultiPass()
{
	return 1;
}

HRESULT IDX8SineWave::SetVertexShader(ID3DGraphicsWindow *d3dgw, int numPass)
{
	HRESULT hr = S_OK;
	pd3dDevice->SetVertexShader(dwVertexShader);

	D3DXMATRIX matWorld = d3dgw->GetWorldXform();
	D3DXMATRIX matView = d3dgw->GetViewXform();
	D3DXMATRIX matProj = d3dgw->GetProjXform();
	D3DXMATRIX matTemp;
	D3DXMATRIX matWorldView;
	D3DXMATRIX matWorldViewIT;
	D3DXMATRIX matWorldViewProj;

	D3DXMatrixMultiply(&matWorldView, &matWorld, &matView);
	D3DXMatrixMultiply(&matWorldViewProj, &matWorldView, &matProj);
	D3DXMatrixInverse(&matWorldViewIT, NULL, &matWorldView);
	
	// Projection to clip space
	D3DXMatrixTranspose(&matWorldViewProj, &matWorldViewProj);
	pd3dDevice->SetVertexShaderConstant(CV_WORLDVIEWPROJ_0, &matWorldViewProj(0, 0), 4);
	D3DXMatrixTranspose(&matWorldViewProj, &matWorldViewProj);

	// Worldview transform
	D3DXMatrixTranspose(&matWorldView, &matWorldView);
	pd3dDevice->SetVertexShaderConstant(CV_WORLDVIEW_0, &matWorldView(0, 0), 4);
	D3DXMatrixTranspose(&matWorldView, &matWorldView);

	// Worldview ITtransform
	pd3dDevice->SetVertexShaderConstant(CV_WORLDVIEWIT_0, &matWorldViewIT(0, 0), 1);
	pd3dDevice->SetVertexShaderConstant(CV_WORLDVIEWIT_1, &matWorldViewIT(1, 0), 1);
	pd3dDevice->SetVertexShaderConstant(CV_WORLDVIEWIT_2, &matWorldViewIT(2, 0), 1);

	float fSinTime = AmountConstant/100.0f;

	D3DXVECTOR4 vecTime(fSinTime * 13.0f, 0.0f, 0.5f, 1.0f);
    pd3dDevice->SetVertexShaderConstant(CV_TIME, &vecTime, 1);
		
	// create Pi-related constants 
    D3DXVECTOR4 vecPi(D3DX_PI, 1.0f / (2.0f * D3DX_PI), 2.0f * D3DX_PI, Height);
    pd3dDevice->SetVertexShaderConstant(CV_PIS, &vecPi, 1);

    // Taylor series coefficients for sin and cos
    D3DXVECTOR4 vecSin(1.0f, -1.0f / 6.0f, 1.0f / 120.0f, -1.0f / 5040.0f);
    D3DXVECTOR4 vecCos(1.0f, -1.0f / 2.0f, 1.0f / 24.0f, -1.0f / 720.0f);
    pd3dDevice->SetVertexShaderConstant(CV_VECSIN, &vecSin, 1);
    pd3dDevice->SetVertexShaderConstant(CV_VECCOS, &vecCos, 1);
	pd3dDevice->SetTexture(0, pCubeTexture);
	pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

	pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_CURRENT);

	pd3dDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
	pd3dDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
	pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP,  (Normals) ? D3DTOP_SELECTARG2 : D3DTOP_SELECTARG1);

	pd3dDevice->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR) ;

	pd3dDevice->SetRenderState(D3DRS_FILLMODE, Wire ? D3DFILL_WIREFRAME : D3DFILL_SOLID);



	return hr;
}

bool IDX8SineWave::DrawMeshStrips(ID3DGraphicsWindow *d3dgw, MeshData *data)
{
	return false;
}

bool IDX8SineWave::DrawWireMesh(ID3DGraphicsWindow *d3dgw, WireMeshData *data)
{
	return false;
}

void IDX8SineWave::StartLines(ID3DGraphicsWindow *d3dgw, WireMeshData *data)
{
}

void IDX8SineWave::AddLine(ID3DGraphicsWindow *d3dgw, DWORD *vert, int vis)
{
}

bool IDX8SineWave::DrawLines(ID3DGraphicsWindow *d3dgw)
{
	return false;
}

void IDX8SineWave::EndLines(ID3DGraphicsWindow *d3dgw, GFX_ESCAPE_FN fn)
{
}

void IDX8SineWave::StartTriangles(ID3DGraphicsWindow *d3dgw, MeshFaceData *data)
{
}

void IDX8SineWave::AddTriangle(ID3DGraphicsWindow *d3dgw, DWORD index, int *edgeVis)
{
}

bool IDX8SineWave::DrawTriangles(ID3DGraphicsWindow *d3dgw)
{
	return false;
}

void IDX8SineWave::EndTriangles(ID3DGraphicsWindow *d3dgw, GFX_ESCAPE_FN fn)
{
}
