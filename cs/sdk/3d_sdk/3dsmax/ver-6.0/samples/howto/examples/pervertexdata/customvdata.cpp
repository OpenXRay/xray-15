/*===========================================================================*\
 | 
 |  FILE:	CustomVData.cpp
 |			Project to demonstrate custom data per vertex
 |			Simply allows user to define a custom value and bind it to a vertex
 |			3D Studio MAX R3.0
 | 
 |  AUTH:   Harry Denholm
 |			Developer Consulting Group
 |			Copyright(c) Discreet 1999
 |
 |  HIST:	Started 6-4-99
 | 
\*===========================================================================*/

#include "CustomVData.h"


IObjParam* CVDModifier::ip = NULL;


/*===========================================================================*\
 |	Class Descriptor OSM
\*===========================================================================*/

class CVDClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic()					{ return TRUE; }
	void *			Create( BOOL loading )		{ return new CVDModifier; }
	const TCHAR *	ClassName()					{ return GetString(IDS_CLASSNAME); }
	SClass_ID		SuperClassID()				{ return OSM_CLASS_ID; }
	Class_ID 		ClassID()					{ return CVD_CLASSID; }
	const TCHAR* 	Category()					{ return _T("");  }

	// Hardwired name, used by MAX Script as unique identifier
	const TCHAR*	InternalName()				{ return _T("CVDMod"); }
	HINSTANCE		HInstance()					{ return hInstance; }
};

static CVDClassDesc CVDCD;
ClassDesc* GetCustomVDataDesc() {return &CVDCD;}


/*===========================================================================*\
 |	Basic implimentation of a dialog handler
\*===========================================================================*/

BOOL CVDModDlgProc::DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int id = LOWORD(wParam);
	switch (msg) 
	{
		case WM_INITDIALOG:
			break;
		case WM_DESTROY:
			break;
		case WM_COMMAND:
			break;
	}
	return FALSE;
}


/*===========================================================================*\
 |	Paramblock2 Descriptor
\*===========================================================================*/

static ParamBlockDesc2 cvd_param_blk ( cvd_params, _T("CVDParams"),  0, &CVDCD, P_AUTO_CONSTRUCT + P_AUTO_UI, 0, 
	//rollout
	IDD_CUSTOMDATA, IDS_PARAMETERS, 0, 0, NULL, 
	// params
	cvd_codev,	_T("Custom Data Value"),	TYPE_FLOAT,	P_ANIMATABLE,	IDS_CVD,
		p_default,		5.0f,
		p_range, 		-900.0f, 900.0f, 
		p_ui,			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_CVD_EDIT, IDC_CVD_SPIN, 1.0f,
		end,
	end
	);



/*===========================================================================*\
 |	Constructor
 |  Ask the ClassDesc2 to make the AUTO_CONSTRUCT paramblocks and wire them in
\*===========================================================================*/

CVDModifier::CVDModifier()
	{
	CVDCD.MakeAutoParamBlocks(this);
	assert(pblock);

	// Seed the random number generator once per instance
	int rseed = randomGen.get();
	randomGen.srand(rseed);
	}



/*===========================================================================*\
 |	Invalidate our UI (or the recently changed parameter)
\*===========================================================================*/

void CVDModifier::InvalidateUI()
{
	cvd_param_blk.InvalidateUI(pblock->LastNotifyParamID());
}



/*===========================================================================*\
 |	Open and Close dialog UIs
 |	We ask the ClassDesc2 to handle Beginning and Ending EditParams for us
\*===========================================================================*/

void CVDModifier::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
{
	this->ip = ip;

	CVDCD.BeginEditParams(ip, this, flags, prev);

	cvd_param_blk.SetUserDlgProc(new CVDModDlgProc(this));
}
		
void CVDModifier::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
{
	CVDCD.EndEditParams(ip, this, flags, next);

	this->ip = NULL;
}



/*===========================================================================*\
 |	Standard clone
\*===========================================================================*/


RefTargetHandle CVDModifier::Clone(RemapDir& remap) 
{	
	CVDModifier* newmod = new CVDModifier();	
	newmod->ReplaceReference(0,pblock->Clone(remap));
	BaseClone(this, newmod, remap);
	return(newmod);
}




/*===========================================================================*\
 |	Subanim & References support
\*===========================================================================*/

Animatable* CVDModifier::SubAnim(int i) 	{
	switch (i) {
		case 0: return pblock;
		default: return NULL;
		}
	}
TSTR CVDModifier::SubAnimName(int i) {
	switch (i) {
		case 0: return GetString(IDS_PARAMETERS);
		default: return _T("");
		}
	}

RefTargetHandle CVDModifier::GetReference(int i)
	{
	switch (i) {
		case 0: return pblock;
		default: return NULL;
		}
	}
void CVDModifier::SetReference(int i, RefTargetHandle rtarg)
	{
	switch (i) {
		case 0: pblock = (IParamBlock2*)rtarg; break;
		}
	}
RefResult CVDModifier::NotifyRefChanged(
		Interval changeInt, RefTargetHandle hTarget,
		PartID& partID,  RefMessage message) 
	{
	switch (message) {
		case REFMSG_CHANGE:
			cvd_param_blk.InvalidateUI();
			break;
		}
	return REF_SUCCEED;
	}




/*===========================================================================*\
 |	The validity of our parameters
 |	Start at FOREVER, and intersect with the validity of each item
\*===========================================================================*/

Interval CVDModifier::GetValidity(TimeValue t)
{
	float f;	
	Interval valid = FOREVER;
	pblock->GetValue(cvd_codev, t, f, valid);
	return valid;
}

Interval CVDModifier::LocalValidity(TimeValue t)
{
	return GetValidity(t);
}

