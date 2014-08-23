/**********************************************************************
 *<
	FILE: sampleEff.cpp

	DESCRIPTION: Simple render effect

	CREATED BY: Dan Silva

	HISTORY: 6/29/98

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "dllMain.h"
#include <iparamm2.h>
#include <bmmlib.h>

#define SAMPLE_EFF_CLASS_ID 0x7691234
static const Class_ID sampleEffClassID(SAMPLE_EFF_CLASS_ID, 0);

#define PBLOCK_REF 0
#define GIZMO_REF  1

//--- Parameter Maps ----------------------------------------------------

// JBW: IDs for ParamBlock2 blocks and parameters
// paramBlock IDs
enum { sampleEff_params, };
// param IDs
enum { prm_color, prm_strength, prm_slow, prm_iBack, prm_gizmo, };

class SampleEffect: public Effect {
	public:
		// Parameters
// JBW: use IParamBlock2's now
		IParamBlock2* pblock;
		BOOL slow;
		INode *gizmo;
						
		SampleEffect();
		~SampleEffect() { }

		// Animatable/Reference
		int NumSubs() {return 1;}
		Animatable* SubAnim(int i);
		TSTR SubAnimName(int i);
		int NumRefs() {return 2;}
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);
		Class_ID ClassID() {return sampleEffClassID;}
		void GetClassName(TSTR& s) { s = GetString(IDS_CLASS_NAME); }
		void DeleteThis() {delete this;}
		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
	         PartID& partID,  RefMessage message);
// JBW: direct ParamBlock access is added
		int	NumParamBlocks() { return 1; }					// return number of ParamBlocks in this instance
		IParamBlock2* GetParamBlock(int i) { return pblock; } // return i'th ParamBlock
		IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock->ID() == id) ? pblock : NULL; } // return id'd ParamBlock

		IOResult Load(ILoad *iload);

		// Effect
		TSTR GetName() { return GetString(IDS_NAME); }
		EffectParamDlg *CreateParamDialog(IRendParams *ip);
		DWORD GBufferChannelsRequired(TimeValue t) { return BMM_CHAN_Z; }
		int RenderBegin(TimeValue t, ULONG flags);
		int RenderEnd(TimeValue t);
		void Update(TimeValue t, Interval& valid);
		void Apply(TimeValue t, Bitmap *bm, RenderGlobalContext *gc, CheckAbortCallback *checkAbort);

		virtual int NumGizmos() {return 1;}
		virtual INode *GetGizmo(int i) {return gizmo;}
		virtual void DeleteGizmo(int i) { 
			ReplaceReference(GIZMO_REF,NULL); 
			}
		virtual void AppendGizmo(INode *node) {
			ReplaceReference(GIZMO_REF, node);
			}
		virtual BOOL OKGizmo(INode *node) { 
			if (node == gizmo) return FALSE;
			ObjectState os = node->EvalWorldState(GetCOREInterface()->GetTime());
			return os.obj->SuperClassID()==LIGHT_CLASS_ID;
			} 
		virtual void EditGizmo(INode *node) {} // selects this gizmo & displays params for it if any
	};

//JBW: removed the EffectParamDlg; UI handled by ClassDesc2 now

class SampleEffClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new SampleEffect; }
	const TCHAR *	ClassName() { return GetString(IDS_CDESC_CLASS_NAME); }
	SClass_ID		SuperClassID() { return RENDER_EFFECT_CLASS_ID; }
	Class_ID 		ClassID() { return sampleEffClassID; }
	const TCHAR* 	Category() { return _T("");  }
// JBW: new descriptor data accessors added.  Note that the 
//      internal name is hardwired since it must not be localized.
	const TCHAR*	InternalName() { return _T("sampleEffect"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle
	};

static SampleEffClassDesc sampleEffCD;
ClassDesc* GetSampleEffDesc() { return &sampleEffCD; }


// JBW: Example statically-defined dialog proc for the IParamMap2 for the following paramblock2, if needed.
// For dynamically-created ParamMapUserDlgProc's, call CreateAutoEParamDlg() or CreateRParamMap2() directly
// giving DlgProc.
class SampleEffDlgProc : public ParamMap2UserDlgProc 
{
	public:
		IParamMap *pmap;

		BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
		void DeleteThis() { }
};
  
BOOL SampleEffDlgProc::DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
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

static SampleEffDlgProc sampleEffDlgProc;

/* ------------- per instance sampleEffect paramblock --------------------------- */

static ParamBlockDesc2 sampleEff_pbDesc ( sampleEff_params, _T("sampleEff parameters"),  0, &sampleEffCD, P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF, 
	//rollout
	IDD_SAMPLE_EFFECT, IDS_PARAMS, 0, 0, &sampleEffDlgProc, 
	// params
	prm_color, _T("color"),	TYPE_RGBA, P_ANIMATABLE, IDS_COLOR,
		p_default, Color(0,0,0),
		p_ui, TYPE_COLORSWATCH, IDC_SAMP_COLOR,
		end,
	prm_strength, _T("strength"), TYPE_FLOAT, P_ANIMATABLE, IDS_STRENGTH,
		p_default, 0.5,
		p_range, 0.0, 1.0,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_STRENGTH, IDC_STRENGTH_SPIN, SPIN_AUTOSCALE,
		end,
	prm_slow, _T("slow"), TYPE_BOOL, 0, IDS_SLOW,
		p_default, FALSE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_SLOW,
		end,
	prm_iBack, _T("ignoreBack"), TYPE_BOOL, P_ANIMATABLE, IDS_IGN_BACK,
		p_default, FALSE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_IGN_BACKGROUND,
		end,
	prm_gizmo, _T("gizmo"), TYPE_REFTARG, P_OWNERS_REF, IDS_GIZMO,
		p_refno, GIZMO_REF,
		end,
	end
	);

//--- SampleEffect ----------------------------------------------------------

SampleEffect::SampleEffect()
	{
	slow = FALSE;
	gizmo = NULL;
// JBW: ask the ClassDes2 to make the AUTO_CONSTRUCT parablocks and wire them in
	sampleEffCD.MakeAutoParamBlocks(this);
	assert(pblock);
	}

IOResult SampleEffect::Load(ILoad *iload)
	{
	Effect::Load(iload);
	return IO_OK;
	}

EffectParamDlg *SampleEffect::CreateParamDialog(IRendParams *ip)
	{	
//JBW: ask the ClassDesc2 to make the AUTO_UI EffectParamDlg and IParamMap2s
	return sampleEffCD.CreateParamDialogs(ip, this);
	}

Animatable* SampleEffect::SubAnim(int i) 	{
	switch (i) {
		case 0: return pblock;
		default: return NULL;
		}
	}

TSTR SampleEffect::SubAnimName(int i) {
	switch (i) {
		case 0: return GetString(IDS_PARAMS);
		default: return _T("");
		}
	}

RefTargetHandle SampleEffect::GetReference(int i)
	{
	switch (i) {
		case 0: return pblock;
		case 1: return gizmo;
		default: return NULL;
		}
	}

void SampleEffect::SetReference(int i, RefTargetHandle rtarg)
	{
	switch (i) {
		case 0: pblock = (IParamBlock2*)rtarg; break;
		case 1: gizmo= (INode*)rtarg; break;
		}
	}

RefResult SampleEffect::NotifyRefChanged(
		Interval changeInt, RefTargetHandle hTarget,
		PartID& partID,  RefMessage message) 
	{
	switch (message) {
		case REFMSG_CHANGE:
// JBW: ask the block to update its UI
			if ( pblock )	// > 11/12/02 - 3:38pm --MQM-- #417502, need "if (pblock)"
				sampleEff_pbDesc.InvalidateUI( pblock->LastNotifyParamID() );
//			NotifyChanged();
			break;
		}
	return REF_SUCCEED;
	}

void SampleEffect::Update(TimeValue t, Interval& valid)
	{
	}

int SampleEffect::RenderBegin(TimeValue t, ULONG flags)
	{
	return 0;
	}

int SampleEffect::RenderEnd(TimeValue t)
	{
	return 0;
	}
		
#define MAX_COLf 65535.0f

void SampleEffect::Apply(TimeValue t, Bitmap *bm, RenderGlobalContext *gc,CheckAbortCallback *checkAbort)
{
// JBW: all pblock references now use the new permanent param IDs
	Color color;
	float fstr;
	BOOL iBack;
	pblock->GetValue(prm_color, t, color, FOREVER);
	pblock->GetValue(prm_strength, t, fstr, FOREVER);
	pblock->GetValue(prm_iBack, t, iBack, FOREVER);
	pblock->GetValue(prm_slow, t, slow, FOREVER);

	int w = bm->Width();
	int h = bm->Height();
	int s = int(4095.0f*fstr);
	int cs = 4095-s;
	float alpha;
	PixelBuf l64(w);
	BMM_Color_64 *p=l64.Ptr();

	BMM_Color_64 tempCol;
	BMM_Color_64 c = color;
	for (int y = 0; y<h; y++)
	{
		bm->GetPixels(0, y, w, p);
		for (int x=0; x<w; x++)
		{
			if ( iBack )
			{
				// skip transparent pixel
				if (p[x].a == 0)
					continue;

				// otherwise, blend on alpha value
				alpha = p[x].a / MAX_COLf;
				tempCol.r = (cs*p[x].r+s*((p[x].r+c.r)>>1))>>12;
				tempCol.g = (cs*p[x].g+s*((p[x].g+c.g)>>1))>>12;
				tempCol.b = (cs*p[x].b+s*((p[x].b+c.b)>>1))>>12;

				p[x].r = (USHORT)(tempCol.r*alpha + p[x].r*(1.0f-alpha));
				p[x].g = (USHORT)(tempCol.g*alpha + p[x].g*(1.0f-alpha));
				p[x].b = (USHORT)(tempCol.b*alpha + p[x].b*(1.0f-alpha));
			}
			else
			{
				p[x].r = (cs*p[x].r+s*((p[x].r+c.r)>>1))>>12;
				p[x].g = (cs*p[x].g+s*((p[x].g+c.g)>>1))>>12;
				p[x].b = (cs*p[x].b+s*((p[x].b+c.b)>>1))>>12;
			}

			if (slow)
			{
				double t1(10.0), t2(100.0);
				for (int k=0; k<20; k++)
					t1 += sqrt(t1+t2); // do something to slow things down for testing
			}
		}
		bm->PutPixels(0, y, w, p);
		if (((y&3)==0)&&checkAbort&&checkAbort->Progress(y,h)) 
			return;
	}
}
