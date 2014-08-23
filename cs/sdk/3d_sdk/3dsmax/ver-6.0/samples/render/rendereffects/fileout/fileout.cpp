/* -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

   FILE: fileOut.cpp

	 DESCRIPTION: post-render file output

	 CREATED BY: michael malone (mjm)

	 HISTORY: created October 15, 1998

		 mjm - 9.9.99 -- restructured to better handle cancelled render notification
		 
   	 Copyright (c) 1998, All Rights Reserved

// -----------------------------------------------------------------------------
// -------------------------------------------------------------------------- */

// local headers
#include "dllMain.h"

// sdk headers
#include <iparamm2.h>
#include <bmmlib.h>

// IDs to references
#define PBLOCK_REF 0
#define DEF_OFFSET 0

// parameter blocks IDs
enum { pbIDFOut };

// parameters for pbIDFOut
enum { prmBitmap, 
	   prmActive,
	   prmAffectSrc,
	   prmChanType,
	   prmCameraNode,
	   prmNearZ,
	   prmFarZ,
	   prmFitScene, };

enum { image, luminance, depth, alpha };

// global instances
static const Class_ID fileOutClassID(0x51763499, 0x637e0353);
const int MAX_COL16(65535); // maximum 16 bit color value


// ----------------------------------------
// color balance effect - class declaration
// ----------------------------------------
class FileOut: public Effect
{
protected:
	Bitmap *mp_srcBM, *mp_writeBM;
	WORD *mp_srcMap, *mp_srcAlpha;
	float *mp_zBuf;
	PBBitmap* m_pPBBMap;
	BitmapInfo *m_pBi;
	CheckAbortCallback *mp_lastCheckAbort;
	int m_srcW, m_srcH, m_writeW, m_writeH;
	int m_firstFrame, m_lastFrame, m_thisFrame, m_fileFrame;
	BOOL m_writing, m_affectSrc, m_active, m_bmOpen, m_singleFrame;
	int m_channel;

	// an empty instance of a bitmap is needed for initialization
	PBBitmap m_PBBMap;


	void processImage();
	void processLum();
	void processDepth();
	void processAlpha();
	void processBMap();

public:
	IParamBlock2 *mp_pblock;

	FileOut();
	~FileOut();
	bool GetStartEndFrames();

	// Animatable/Reference
	int NumSubs() { return 1; }
	Animatable* SubAnim(int i) { return GetReference(i); }
	TSTR SubAnimName(int i);
	int NumRefs() { return 1; }
	RefTargetHandle GetReference(int i);
	void SetReference(int i, RefTargetHandle rtarg);
	Class_ID ClassID() { return fileOutClassID; }
	void GetClassName(TSTR& s) { s = GetString(IDS_FILE_OUT); }
	void DeleteThis() { delete this; }
	RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID,  RefMessage message);
	int	NumParamBlocks() { return 1; }
	IParamBlock2* GetParamBlock(int i) { return mp_pblock; } // only one
	IParamBlock2* GetParamBlockByID(BlockID id) { return (mp_pblock->ID() == id) ? mp_pblock : NULL; }
	IOResult Load(ILoad *iload);

	// Effect
	TSTR GetName() { return GetString(IDS_FILE_OUT); }
	EffectParamDlg *CreateParamDialog(IRendParams *ip);
	DWORD GBufferChannelsRequired(TimeValue t);
	void Update(TimeValue t, Interval& valid) { }
	int RenderBegin(TimeValue t, ULONG flags) { return 0; }
	int RenderEnd(TimeValue t);
	void Apply(TimeValue t, Bitmap *bm, RenderGlobalContext *gc, CheckAbortCallback *checkAbort);
};


// --------------------------------------------------
// color balance class descriptor - class declaration
// --------------------------------------------------
class FileOutClassDesc : public ClassDesc2
{
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new FileOut; }
	const TCHAR *	ClassName() { return GetString(IDS_FILE_OUT); }
	SClass_ID		SuperClassID() { return RENDER_EFFECT_CLASS_ID; }
	Class_ID 		ClassID() { return fileOutClassID; }
	const TCHAR* 	Category() { return _T(""); }
	const TCHAR*	InternalName() { return _T("fileOut"); } // hard-coded for scripter
	HINSTANCE		HInstance() { return hInstance; }
	};

// global instance
static FileOutClassDesc fileOutCD;
// external access function (through dllMain.h)
ClassDesc* GetFileOutDesc() { return &fileOutCD; }


// -------------------------------------------
// parameter accessor - class declaration
// -------------------------------------------
class FileOutPBAccessor : public PBAccessor
{
public:
	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)
	{
		// since this effect's params are not animatable, time is always 0
		static IParamBlock2* p_pblock;
		p_pblock = ((FileOut *)owner)->mp_pblock;

		// this accessor ensures that min <= max
		switch (id)
		{
			case prmNearZ:
				if ( v.f < p_pblock->GetFloat(prmFarZ) )
					p_pblock->SetValue(prmFarZ, 0, v.f - 0.1f);
				break;
			case prmFarZ:
				if ( v.f > p_pblock->GetFloat(prmNearZ) )
					p_pblock->SetValue(prmNearZ, 0, v.f + 0.1f);
				break;
		}
	}
};

// global instance
static FileOutPBAccessor fOutAccessor;


// -------------------------------------------
// parameter validator - class declaration
// -------------------------------------------
class FileOutPBValidator : public PBValidator
{
	// this validator picks only camera objects
	BOOL Validate(PB2Value &v)
	{
		if (!v.r) return FALSE; // this will return if MAXScript tries to set the object to &undefined (NULL)
		static Object *ob;
		ob = (((INode *)v.r)->EvalWorldState(0)).obj;
		if ( ob && (ob->SuperClassID() == CAMERA_CLASS_ID) )
			return TRUE;
		else
			return FALSE;
	}
};

// global instance
static FileOutPBValidator fOutValidator;

typedef TCHAR TChBuffer[MAX_PATH];

// -------------------------------------------
// rollup dialog procedure - class declaration
// -------------------------------------------
#define NUMCHANNELS 4
class FileOutDlgProc : public ParamMap2UserDlgProc 
{
	TChBuffer mStrings[NUMCHANNELS];

	BOOL hasSetup(BitmapInfo *pBi);
	BOOL hasAbout(BitmapInfo *pBi);
	BOOL setup(BitmapInfo *pBi, HWND hWnd);
	void about(BitmapInfo *pBi, HWND hWnd);
	void handleSetup (BitmapInfo *pBi, HWND hWnd);
	void handleText (BitmapInfo *pBi, HWND hWnd);
	void showZCtrls(HWND hWnd, int state);
	void enableZCtrls(HWND hWnd, IParamMap2 *map, BOOL state);
	void copyZVals(IParamMap2 *map);

public:
	void SetThing(ReferenceTarget *newEff) { }
	BOOL DlgProc(TimeValue t,IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void DeleteThis() { }
	void Update(TimeValue t);
	void ShowControls(TimeValue t, IParamMap2 *map, HWND hWnd);
	ICustButton *m_iCopyBtn;
	BOOL m_camPicked;
};

// global instance
static FileOutDlgProc fOutDlgProc;


// ---------------------------------------------
// parameter block description - global instance
// ---------------------------------------------
static ParamBlockDesc2 pbdFOut(pbIDFOut, _T("fileOut parameters"), 0, &fileOutCD, P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF,
	//rollout
	IDD_FILE_OUT_EFFECT, IDS_FILE_OUT_PARAMS, 0, 0, &fOutDlgProc,
	// params
	prmBitmap, _T("bitmap"), TYPE_BITMAP, 0, IDS_BITMAP,			// not animatable
		end,
	prmActive, _T("active"), TYPE_BOOL, P_ANIMATABLE, IDS_ACTIVE,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDB_ACTIVE,
		end,
	prmAffectSrc, _T("affectSource"), TYPE_BOOL, P_ANIMATABLE, IDS_AFFECT_SRC,
		p_default, FALSE,
		p_ui, TYPE_SINGLECHEKBOX, IDB_AFFECT_SRC,
		end,
	prmChanType, _T("channelType"), TYPE_INT, 0, IDS_CHANNEL_TYPE,	// not animatable
		p_range, image, alpha,
		p_default, image,
		end,
	prmCameraNode, _T("cameraNode"), TYPE_INODE, 0, IDS_CAM_NODE,	// not animatable
		p_ui, TYPE_PICKNODEBUTTON, IDB_PICK_CAM,
		p_prompt, IDS_PICK_CAM_PROMPT,
		p_validator, &fOutValidator,
		end,
	prmNearZ, _T("nearZ"), TYPE_FLOAT, P_ANIMATABLE, IDS_NEARZ,
		p_range, -1.0e10f, 1.0e10f,
		p_default, 0.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_NEARZ_EDIT, IDC_NEARZ_SPIN, SPIN_AUTOSCALE,
		p_accessor, &fOutAccessor,
		end,
	prmFarZ, _T("farZ"), TYPE_FLOAT, P_ANIMATABLE, IDS_FARZ,
		p_range, -1.0e10f, 1.0e10f,
		p_default, -500.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_FARZ_EDIT, IDC_FARZ_SPIN, SPIN_AUTOSCALE,
		p_accessor, &fOutAccessor,
		end,
	prmFitScene, _T("fitScene"), TYPE_BOOL, P_ANIMATABLE, IDS_FIT_SCENE,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDB_FIT_SCENE,
		end,
	end
	);


// --------------------------------------------
// rollup dialog procedure - method definitions
// --------------------------------------------
BOOL FileOutDlgProc::hasSetup(BitmapInfo *pBi)
{
	if (!pBi->Device()[0])
		return (FALSE);
	else
	{
		DWORD devcap = TheManager->ioList.GetDeviceCapabilities(pBi->Device());
		return (devcap & BMMIO_CONTROLWRITE)==0? FALSE: TRUE;
	}   
}

BOOL FileOutDlgProc::hasAbout(BitmapInfo *pBi)
{
	if (!_tcslen(pBi->Device()))
		return (FALSE);
	else   
		return (TRUE);
}

BOOL FileOutDlgProc::setup(BitmapInfo *pBi, HWND hWnd)
{
	if (pBi->Device()[0])
	{
		BitmapIO *IO = TheManager->ioList.CreateDevInstance(pBi->Device());
		if(IO)
		{
			if (pBi->GetPiData())
			{
				if (!IO->ValidatePiData(pBi))
				  pBi->ResetPiData();
				else
				  IO->LoadConfigure(pBi->GetPiData());
			}
			if (IO->ShowControl(hWnd,BMMIO_CONTROLWRITE))
			{
				DWORD size = IO->EvaluateConfigure();
				if (pBi->AllocPiData(size))
				{
					if (!IO->SaveConfigure(pBi->GetPiData()))
					  pBi->ResetPiData();
				}
			}
			delete IO;
			return (TRUE);
		}
	}
	return (FALSE);
}

void FileOutDlgProc::about(BitmapInfo *pBi, HWND hWnd) {
	if (!_tcslen(pBi->Device()))
		return;
	BitmapIO *IO = TheManager->ioList.CreateDevInstance(pBi->Device());
	if(IO) {
		IO->ShowAbout(hWnd);
		delete IO;
	}
}

void FileOutDlgProc::handleSetup (BitmapInfo *pBi, HWND hWnd)
{
	HWND hDlg = GetDlgItem(hWnd,IDB_SETUP);
	EnableWindow(hDlg, hasSetup(pBi));
	hDlg = GetDlgItem(hWnd,IDB_ABOUT);
	EnableWindow(hDlg, hasAbout(pBi));
}

void FileOutDlgProc::handleText (BitmapInfo *pBi, HWND hWnd) {
	TCHAR file[MAX_PATH];

	if (!_tcslen(pBi->Name()))
		_tcscpy(file, GetString(IDS_NONE_AVAIL));
	else
		_tcscpy(file, pBi->Name());

	TSTR text;

	// handle file line
	if ( (_tcslen(file) > 40) && (_tcslen(pBi->Name())) ) {
		TCHAR txt[MAX_PATH];
		_tcscpy(txt,file);
		int i = 6;
		do {
			if (txt[i] == '\\')
				break;
			if (txt[i] == '/')
				break;
		} while (i++ < 12);
		txt[i+1] = 0;
		_tcscat(txt,"...\\");
		_tcscat(txt, pBi->Filename());
		SetDlgItemText(hWnd, IDT_UPPER_TEXT, txt);
	} else
		SetDlgItemText(hWnd, IDT_UPPER_TEXT, file);

	// handle device line
	text = pBi->Device();
	if (!text.length())
		text = GetString(IDS_UNDEFINED);
	SetDlgItemText(hWnd, IDT_LOWER_TEXT, text);   
}

void FileOutDlgProc::showZCtrls(HWND hWnd, int state)
{
	ShowWindow( GetDlgItem(hWnd, IDC_PICK_CAM_TEXT), state );
	ShowWindow( GetDlgItem(hWnd, IDB_COPY), state );
	ShowWindow( GetDlgItem(hWnd, IDB_PICK_CAM), state );
	ShowWindow( GetDlgItem(hWnd, IDC_NEARZ_EDIT), state );
	ShowWindow( GetDlgItem(hWnd, IDC_NEARZ_SPIN), state );
	ShowWindow( GetDlgItem(hWnd, IDC_NEARZ_TEXT), state );
	ShowWindow( GetDlgItem(hWnd, IDC_FARZ_EDIT), state );
	ShowWindow( GetDlgItem(hWnd, IDC_FARZ_SPIN), state );
	ShowWindow( GetDlgItem(hWnd, IDC_FARZ_TEXT), state );
	ShowWindow( GetDlgItem(hWnd, IDB_FIT_SCENE), state );
}

void FileOutDlgProc::enableZCtrls(HWND hWnd, IParamMap2 *map, BOOL state)
{
	EnableWindow( GetDlgItem(hWnd, IDC_PICK_CAM_TEXT), state );
	EnableWindow( GetDlgItem(hWnd, IDC_NEARZ_TEXT), state );
	EnableWindow( GetDlgItem(hWnd, IDC_FARZ_TEXT), state );

	if (m_camPicked)
	{
		m_iCopyBtn->Enable(state);
	}

	map->Enable( prmCameraNode, state );
	map->Enable( prmNearZ, state );
	map->Enable( prmFarZ, state );
}

void FileOutDlgProc::copyZVals(IParamMap2 *map)
{
	INode *p_node;
	map->GetParamBlock()->GetValue(prmCameraNode, 0, p_node, FOREVER);
	if (p_node)
	{
		CameraObject *p_cam = (CameraObject*)(p_node->EvalWorldState(0).obj);

		TimeValue t	= GetCOREInterface()->GetTime();
		float dist = p_cam->GetClipDist(t, CAM_HITHER_CLIP);
		map->GetParamBlock()->SetValue(prmNearZ, 0, -dist);
		dist = p_cam->GetClipDist(t, CAM_YON_CLIP);
		map->GetParamBlock()->SetValue(prmFarZ, 0, -dist);
	}
}

void FileOutDlgProc::Update(TimeValue t)
{
	IParamMap2 *map = fileOutCD.GetParamMap(&pbdFOut);
	if (map)
	{
		HWND hWnd = map->GetHWnd(); DbgAssert(hWnd);
		ShowControls(t, map, hWnd);
	}
}

void FileOutDlgProc::ShowControls(TimeValue t, IParamMap2 *map, HWND hWnd)
{
	int index;
	PBBitmap* pPBBMap;
	map->GetParamBlock()->GetValue(prmBitmap, t, pPBBMap, FOREVER); DbgAssert(pPBBMap);
	BitmapInfo*	pBi = &(pPBBMap->bi);

	handleText(pBi, hWnd);
	handleSetup(pBi, hWnd);
	map->GetParamBlock()->GetValue(prmChanType, t, index, FOREVER);
	SendMessage( GetDlgItem(hWnd, IDC_CHANNEL_LIST), CB_SETCURSEL, (WPARAM)index, 0 );
	ShowWindow( GetDlgItem(hWnd, IDB_AFFECT_SRC), (index != image) ? SW_SHOW : SW_HIDE );
	INode *p_node;
	map->GetParamBlock()->GetValue(prmCameraNode, 0, p_node, FOREVER);
	m_iCopyBtn->Enable( m_camPicked = (p_node) ? TRUE : FALSE );
	showZCtrls( hWnd, (index == depth) ? SW_SHOW : SW_HIDE );
	enableZCtrls( hWnd, map, IsDlgButtonChecked( hWnd, IDB_FIT_SCENE) != BST_CHECKED );
}

BOOL FileOutDlgProc::DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static TCHAR oldname[MAX_PATH];
	static HWND hCtrl;
	static PBBitmap* pPBBMap;
	static BitmapInfo*	pBi;

	map->GetParamBlock()->GetValue(prmBitmap, t, pPBBMap, FOREVER);
	DbgAssert(pPBBMap);
	pBi = &(pPBBMap->bi);

	switch (msg)
	{
		case WM_INITDIALOG:
		{
			// initialize strings for combo boxe
			_tcscpy( mStrings[0], GetString(IDS_IMAGE) );
			_tcscpy( mStrings[1], GetString(IDS_LUM));
			_tcscpy( mStrings[2], GetString(IDS_DEPTH));
			_tcscpy( mStrings[3], GetString(IDS_ALPHA));

			// insert strings into listbox
			hCtrl = GetDlgItem(hWnd, IDC_CHANNEL_LIST);
			for (int index=0; index<NUMCHANNELS; index++)
				SendMessage( hCtrl, CB_ADDSTRING, 0, (LPARAM)mStrings[index] );

			// some button settings
			map->SetTooltip( prmCameraNode, TRUE, GetString(IDS_PICK_CAM_TIP) );
			m_iCopyBtn = GetICustButton( GetDlgItem(hWnd, IDB_COPY) );
			m_iCopyBtn->SetTooltip(TRUE, GetString(IDS_COPY_VALS_TIP) );
			break;
		}

		case WM_SHOWWINDOW:
			if (wParam) // window is being shown
				ShowControls(t, map, hWnd);
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_CHANNEL_LIST:
					if ( HIWORD(wParam) == CBN_SELCHANGE )
					{
						hCtrl = (HWND)lParam;
						int index = SendMessage(hCtrl, CB_GETCURSEL, 0, 0);
						map->GetParamBlock()->SetValue(prmChanType, 0, index); // not animatable - time 0
						ShowWindow( GetDlgItem(hWnd, IDB_AFFECT_SRC), (index != image) ? SW_SHOW : SW_HIDE );
						showZCtrls( hWnd, (index == depth) ? SW_SHOW : SW_HIDE );
					}
					break;
				case IDB_FILES:
					_tcscpy(oldname, pBi->Name());
					if (TheManager->SelectFileOutput(pBi, hWnd)) {
						handleText(pBi, hWnd);
						handleSetup(pBi, hWnd);
					}
					break;
				case IDB_DEVICES:
					_tcscpy(oldname, pBi->Device());
					if (TheManager->SelectDeviceOutput(pBi, hWnd)) {
						handleText(pBi, hWnd);
						handleSetup(pBi, hWnd);
					}
					break;
				case IDB_CLEAR:
					// set to an 'unnamed' bitmap
					pBi->SetName( _T("") );
					pBi->SetDevice( _T("") );
					handleText(pBi, hWnd);
					handleSetup(pBi, hWnd);
					break;
				case IDB_ABOUT:
					about(pBi, hWnd);
					break;
				case IDB_SETUP:
					setup(pBi, hWnd);
					break;
				case IDB_COPY:
					copyZVals(map);
					break;
				case IDB_FIT_SCENE:
					enableZCtrls( hWnd, map, IsDlgButtonChecked( hWnd, IDB_FIT_SCENE) != BST_CHECKED );
					break;
			}
			break;

		case WM_DESTROY:
			ReleaseICustButton(m_iCopyBtn);
			break;
	}
	return FALSE;
}

// -----------------------------------------
// file output effect - method definitions
// -----------------------------------------
FileOut::FileOut()
{
	mp_srcBM = mp_writeBM = NULL;
	mp_srcMap = mp_srcAlpha = NULL;
	mp_zBuf = NULL;
	m_pPBBMap = NULL;
	m_pBi = NULL;
	mp_lastCheckAbort = NULL;
	m_srcW = m_srcH = m_writeW = m_writeH = m_firstFrame = m_lastFrame = m_thisFrame = m_fileFrame = 0;
	m_writing = m_affectSrc = m_active = m_bmOpen = m_singleFrame = FALSE;
	m_channel = image;

	fileOutCD.MakeAutoParamBlocks(this);
	DbgAssert(mp_pblock);

	// initialize to an 'unnamed' bitmap
	mp_pblock->SetValue(prmBitmap, 0, &m_PBBMap);
}

FileOut::~FileOut()
{
	if (mp_writeBM)
		mp_writeBM->DeleteThis();
}

IOResult FileOut::Load(ILoad *iload)
{
	Effect::Load(iload);
	return IO_OK;
}

EffectParamDlg *FileOut::CreateParamDialog(IRendParams *ip)
{	
	return fileOutCD.CreateParamDialogs(ip, this);
}

DWORD FileOut::GBufferChannelsRequired(TimeValue t)
{
	int channel;
	mp_pblock->GetValue(prmChanType, 0, channel, FOREVER); // not animatable - time 0
	return ( (channel == depth) ? BMM_CHAN_Z : BMM_CHAN_NONE );
}

TSTR FileOut::SubAnimName(int i)
{
	switch (i)
	{
	case 0:
		return GetString(IDS_FILE_OUT_PARAMS);
	default:
		return _T("");
	}
}

RefTargetHandle FileOut::GetReference(int i)
{
	switch (i)
	{
	case 0:
		return mp_pblock;
	default:
		return NULL;
	}
}

void FileOut::SetReference(int i, RefTargetHandle rtarg)
{
	switch (i)
	{
	case 0:
		mp_pblock = (IParamBlock2*)rtarg;
		break;
	}
}

RefResult FileOut::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message)
{
	static int prmID;
	switch (message)
	{
		case REFMSG_CHANGE:
			switch (prmID = mp_pblock->LastNotifyParamID())
			{
				case prmCameraNode:
				{
					INode *p_node;
					mp_pblock->GetValue(prmCameraNode, 0, p_node, FOREVER);
					IParamMap2* map = mp_pblock->GetMap();
					if (map != NULL) {
						FileOutDlgProc *proc = (FileOutDlgProc*)(map->GetUserDlgProc());
						proc->m_iCopyBtn->Enable( proc->m_camPicked = (p_node) ? TRUE : FALSE );
					}
					break;
				}
				case prmBitmap:
				case prmActive:
				case prmNearZ:
				case prmFarZ:
				case prmFitScene:
					break;
			}
			pbdFOut.InvalidateUI(prmID);
			break;
	}
	return REF_SUCCEED;
}

void FileOut::processImage()
{
	DbgAssert(m_writing); // if not writing, shouldn't be here
	int type;
	mp_srcMap = (WORD*)mp_srcBM->GetStoragePtr(&type);
	DbgAssert(type == BMM_TRUE_48);
	mp_srcAlpha = (WORD*)mp_srcBM->GetAlphaPtr(&type);
	DbgAssert(type == BMM_GRAY_16);
	WORD *writeMap = (WORD*)mp_writeBM->GetStoragePtr(&type);
	DbgAssert(type == BMM_TRUE_48);
	WORD *writeAlpha = (WORD*)mp_srcBM->GetAlphaPtr(&type);
	DbgAssert(type == BMM_GRAY_16);

	if(mp_srcMap&&mp_srcAlpha&&writeMap&&writeAlpha)
	{
		int i(0), p(0), imageSz(m_srcW*m_srcH);
		for ( ; i<imageSz; i++, p+=3)
		{
			writeMap[p]   = mp_srcMap[p];
			writeMap[p+1] = mp_srcMap[p+1];
			writeMap[p+2] = mp_srcMap[p+2];
			writeAlpha[i] = mp_srcAlpha[i];
		}
	}
	else
	{
		BMM_Color_64* buf = new BMM_Color_64[mp_srcBM->Width()];
		for(int i=0;i<mp_srcBM->Height();i++)
		{
			mp_srcBM->GetPixels(0,i,mp_srcBM->Width(),buf);
			mp_writeBM->PutPixels(0,i,mp_srcBM->Width(),buf);
		}
		delete [] buf;
	}
}

void FileOut::processLum()
{
	int type;
	mp_srcMap = (WORD*)mp_srcBM->GetStoragePtr(&type);
	DbgAssert(type == BMM_TRUE_48);

	// perceptually based luminance (from "A Technical Introduction to Digital Video", C. Poynton)
	int i(0), p(0), imageSz(m_srcW*m_srcH);
	if (m_writing)
	{
		mp_srcAlpha = (WORD*)mp_srcBM->GetAlphaPtr(&type);
		DbgAssert(type == BMM_GRAY_16);
		WORD *writeMap = (WORD*)mp_writeBM->GetStoragePtr(&type);
		DbgAssert(type == BMM_TRUE_48);
		WORD *writeAlpha = (WORD*)mp_srcBM->GetAlphaPtr(&type);
		DbgAssert(type == BMM_GRAY_16);


		if(mp_srcMap&&mp_srcAlpha &&writeMap &&writeAlpha)
		{

			if (m_affectSrc)
			{
				for ( ; i<imageSz; i++, p+=3)
				{
					mp_srcMap[p] = mp_srcMap[p+1] = mp_srcMap[p+2] = writeMap[p] = writeMap[p+1] = writeMap[p+2] = (WORD)(mp_srcMap[p]*0.2125f + mp_srcMap[p+1]*0.7154f + mp_srcMap[p+2]*0.0721f); // r = g = b = source luminance
					writeAlpha[i] = mp_srcAlpha[i];
				}
			}
			else
			{
				for ( ; i<imageSz; i++, p+=3)
				{
					writeMap[p] = writeMap[p+1] = writeMap[p+2] = (WORD)(mp_srcMap[p]*0.2125f + mp_srcMap[p+1]*0.7154f + mp_srcMap[p+2]*0.0721f); // r = g = b = source luminance
					writeAlpha[i] = mp_srcAlpha[i];
				}
			}
		}
		else
		{
			
			BMM_Color_64* buf = new BMM_Color_64[mp_srcBM->Width()];

			for(int i=0;i<mp_srcBM->Height();i++)
			{
				mp_srcBM->GetPixels(0,i,mp_srcBM->Width(),buf);
				for(int j=0;j<mp_srcBM->Width();j++)
				{
					buf[j].r = buf[j].g = buf[j].b = (WORD)(buf[j].r*0.2125f + buf[j].g*0.7154f + buf[j].b*0.0721f); // r = g = b = source luminance
				}
				mp_writeBM->PutPixels(0,i,mp_srcBM->Width(),buf);
				if (m_affectSrc)
					mp_srcBM->PutPixels(0,i,mp_srcBM->Width(),buf);
			}

			delete [] buf;

		}
	}
	else if (m_affectSrc)
	{

		if(mp_srcMap)
		{
			for ( ; i<imageSz; i++, p+=3)
				mp_srcMap[p] = mp_srcMap[p+1] = mp_srcMap[p+2] = (WORD)(mp_srcMap[p]*0.2125f + mp_srcMap[p+1]*0.7154f + mp_srcMap[p+2]*0.0721f); // r = g = b = source luminance
		}
		else
		{
			BMM_Color_64* buf = new BMM_Color_64[mp_srcBM->Width()];

			for(int i=0;i<mp_srcBM->Height();i++)
			{
				mp_srcBM->GetPixels(0,i,mp_srcBM->Width(),buf);
				for(int j=0;j<mp_srcBM->Width();j++)
				{
					buf[j].r = buf[j].g = buf[j].b = (WORD)(buf[j].r*0.2125f + buf[j].g*0.7154f + buf[j].b*0.0721f); // r = g = b = source luminance
				}
				mp_srcBM->PutPixels(0,i,mp_srcBM->Width(),buf);
			}

			delete [] buf;
		}
	}
}

void FileOut::processDepth()
{
	int type;
	mp_srcMap = (WORD*)mp_srcBM->GetStoragePtr(&type);
	DbgAssert(type == BMM_TRUE_48);
	mp_srcAlpha = (WORD*)mp_srcBM->GetAlphaPtr(&type);
	DbgAssert(type == BMM_GRAY_16);

	DWORD zType;
	if ( !(mp_zBuf = (float *)mp_srcBM->GetChannel(BMM_CHAN_Z, zType)) )
		return;

	BOOL fitScene;
	mp_pblock->GetValue(prmFitScene, 0, fitScene, FOREVER);	// not animatable - time 0

	int i, p(0), imageSz(m_srcW*m_srcH);
	float nearZ(-1.0e30f), farZ(1.0e30f), z;
	if (fitScene)
	{
		for (i=0; i<imageSz; i++)
		{
			z = mp_zBuf[i];
			if (z == -1.0e30f) // default z
				continue;
			if (nearZ < z)
				nearZ = z;
			if (farZ > z)
				farZ = z;
		}
	}
	else
	{
		mp_pblock->GetValue(prmNearZ, 0, nearZ, FOREVER);	// not animatable - time 0
		mp_pblock->GetValue(prmFarZ,  0, farZ,  FOREVER);	// not animatable - time 0
	}

	float range(farZ - nearZ);
	if (m_writing)
	{
		WORD *writeMap = (WORD*)mp_writeBM->GetStoragePtr(&type);
		DbgAssert(type == BMM_TRUE_48);
		WORD *writeAlpha = (WORD*)mp_srcBM->GetAlphaPtr(&type);
		DbgAssert(type == BMM_GRAY_16);

		if(writeMap&&writeAlpha&&mp_srcMap&&mp_srcAlpha)
		{
			if (m_affectSrc)
			{
				for (i=0; i<imageSz; i++, p+=3)
				{
					z = (range == 0.0f) ? 1.0f : (mp_zBuf[i] - nearZ) / range;
					if ( (z < 0.0f) || (z > 1.0f) )
						z = 1.0f; // values outside of range set to black
					mp_srcMap[p] = mp_srcMap[p+1] = mp_srcMap[p+2] = writeMap[p] = writeMap[p+1] = writeMap[p+2] = (WORD)((1.0f-z) * MAX_COL16); // r = g = b = normalized z
					writeAlpha[i] = mp_srcAlpha[i];
				}
			}
			else
			{
				for (i=0; i<imageSz; i++, p+=3)
				{
					z = (range == 0.0f) ? 1.0f : (mp_zBuf[i] - nearZ) / range;
					if ( (z < 0.0f) || (z > 1.0f) )
						z = 1.0f; // values outside of range set to black
					writeMap[p] = writeMap[p+1] = writeMap[p+2] = (WORD)((1.0f-z) * MAX_COL16); // r = g = b = normalized z
					writeAlpha[i] = mp_srcAlpha[i];
				}
			}
		}
		else
		{
			BMM_Color_64* buf = new BMM_Color_64[mp_srcBM->Width()];

			for(int i=0;i<mp_srcBM->Height();i++)
			{
				mp_srcBM->GetPixels(0,i,mp_srcBM->Width(),buf);
				for(int j=0;j<mp_srcBM->Width();j++)
				{
					z = (range == 0.0f) ? 1.0f : (mp_zBuf[mp_srcBM->Width()*i+j] - nearZ) / range;
					if ( (z < 0.0f) || (z > 1.0f) )
						z = 1.0f; // values outside of range set to black
					buf[j].r = buf[j].g = buf[j].b = (WORD)((1.0f-z) * MAX_COL16); // r = g = b = normalized z
				}
				mp_writeBM->PutPixels(0,i,mp_srcBM->Width(),buf);
				if (m_affectSrc)
					mp_srcBM->PutPixels(0,i,mp_srcBM->Width(),buf);
			}

			delete [] buf;
		}
	}
	else if (m_affectSrc)
	{
		if(mp_srcMap)
		{
			for (i=0; i<imageSz; i++, p+=3)
			{
					z = (range == 0.0f) ? 1.0f : (mp_zBuf[i] - nearZ) / range;
					if ( (z < 0.0f) || (z > 1.0f) )
						z = 1.0f; // values outside of range set to black
					mp_srcMap[p] = mp_srcMap[p+1] = mp_srcMap[p+2] = (WORD)((1.0f-z) * MAX_COL16); // r = g = b = normalized z
			}
		}
		else
		{
			BMM_Color_64* buf = new BMM_Color_64[mp_srcBM->Width()];

			for(int i=0;i<mp_srcBM->Height();i++)
			{
				mp_srcBM->GetPixels(0,i,mp_srcBM->Width(),buf);
				for(int j=0;j<mp_srcBM->Width();j++)
				{
					z = (range == 0.0f) ? 1.0f : (mp_zBuf[mp_srcBM->Width()*i+j] - nearZ) / range;
					if ( (z < 0.0f) || (z > 1.0f) )
						z = 1.0f; // values outside of range set to black
					buf[j].r = buf[j].g = buf[j].b = (WORD)((1.0f-z) * MAX_COL16); // r = g = b = normalized z
				}
				mp_srcBM->PutPixels(0,i,mp_srcBM->Width(),buf);
			}

			delete [] buf;

		}
	}
}

void FileOut::processAlpha()
{
	int type;
	mp_srcMap = (WORD*)mp_srcBM->GetStoragePtr(&type);
	DbgAssert(type == BMM_TRUE_48);
	mp_srcAlpha = (WORD*)mp_srcBM->GetAlphaPtr(&type);
	DbgAssert(type == BMM_GRAY_16);

	int i(0), p(0), imageSz(m_srcW*m_srcH);
	if (m_writing)
	{
		WORD *writeMap = (WORD*)mp_writeBM->GetStoragePtr(&type);
		DbgAssert(type == BMM_TRUE_48);
		WORD *writeAlpha = (WORD*)mp_srcBM->GetAlphaPtr(&type);
		DbgAssert(type == BMM_GRAY_16);

		if(writeMap&&writeAlpha&&mp_srcMap&&mp_srcAlpha)
		{
			if (m_affectSrc)
			{
				for ( ; i<imageSz; i++, p+=3)
				{
					mp_srcMap[p] = mp_srcMap[p+1] = mp_srcMap[p+2] = writeMap[p] = writeMap[p+1] = writeMap[p+2] = writeAlpha[i] = mp_srcAlpha[i]; // r = g = b = source alpha
				}
			}
			else
			{
				for ( ; i<imageSz; i++, p+=3)
				{
					writeMap[p] = writeMap[p+1] = writeMap[p+2] = writeAlpha[i] = mp_srcAlpha[i]; // r = g = b = source alpha
				}
			}
		}
		else
		{
			BMM_Color_64* buf = new BMM_Color_64[mp_srcBM->Width()];

			for(int i=0;i<mp_srcBM->Height();i++)
			{
				mp_srcBM->GetPixels(0,i,mp_srcBM->Width(),buf);
				for(int j=0;j<mp_srcBM->Width();j++)
				{
					buf[j].r = buf[j].g = buf[j].b = buf[j].a; // r = g = b = normalized z
				}
				mp_writeBM->PutPixels(0,i,mp_srcBM->Width(),buf);
				if (m_affectSrc)
					mp_srcBM->PutPixels(0,i,mp_srcBM->Width(),buf);
			}

			delete [] buf;
		}
	}
	else if (m_affectSrc)
	{
		if(mp_srcMap&&mp_srcAlpha)
		{
			for ( ; i<imageSz; i++, p+=3)
				mp_srcMap[p] = mp_srcMap[p+1] = mp_srcMap[p+2] = mp_srcAlpha[i]; // r = g = b = source alpha
		}
		else
		{
			BMM_Color_64* buf = new BMM_Color_64[mp_srcBM->Width()];

			for(int i=0;i<mp_srcBM->Height();i++)
			{
				mp_srcBM->GetPixels(0,i,mp_srcBM->Width(),buf);
				for(int j=0;j<mp_srcBM->Width();j++)
				{
					buf[j].r = buf[j].g = buf[j].b = buf[j].a; // r = g = b = normalized z
				}
				mp_srcBM->PutPixels(0,i,mp_srcBM->Width(),buf);
			}

			delete [] buf;
		}

	}
}

void FileOut::processBMap()
{
	m_srcW = mp_srcBM->Width();
	m_srcH = mp_srcBM->Height();

	if ( m_writing && !m_bmOpen )
	{
		m_fileFrame = -1;
		GetStartEndFrames();
		DbgAssert( m_thisFrame >= m_firstFrame && m_thisFrame <= m_lastFrame);

		m_pBi->SetWidth( m_srcW );
		m_pBi->SetHeight( m_srcH );
		m_pBi->SetAspect( mp_srcBM->Aspect() );
		m_pBi->SetGamma( mp_srcBM->Gamma() );
		m_pBi->SetFlags( mp_srcBM->Flags() );
		m_pBi->SetType( BMM_TRUE_64 );
		m_pBi->SetFirstFrame( m_thisFrame );
		m_pBi->SetLastFrame( m_thisFrame );
		m_pBi->SetCurrentFrame( m_thisFrame );

		if ( (m_srcW != m_writeW) || (m_srcH != m_writeH) )
		{
			if (mp_writeBM)
				mp_writeBM->DeleteThis();

			mp_writeBM = TheManager->Create(m_pBi);
			m_writeW = m_srcW;
			m_writeH = m_srcH;
		}

		if ( mp_writeBM->OpenOutput(m_pBi) != BMMRES_SUCCESS )
			GetCOREInterface()->Log()->LogEntry(SYSLOG_ERROR, DISPLAY_DIALOG, GetString(IDS_ERROR_TITLE), GetString(IDS_ERROR_WRITE), _T("") );
		else
			m_bmOpen = TRUE;
	}

	switch (m_channel)
	{
		case image:
			processImage();
			break;
		case luminance:
			processLum();
			break;
		case depth:
			processDepth();
			break;
		case alpha:
			processAlpha();
			break;
	}

	if (m_writing)
	{
		if (m_singleFrame)
			m_fileFrame = BMM_SINGLEFRAME;
		else
			m_pBi->SetLastFrame( ++m_fileFrame );

		if ( mp_writeBM->Write( m_pBi, m_fileFrame ) != BMMRES_SUCCESS )
			GetCOREInterface()->Log()->LogEntry(SYSLOG_ERROR, DISPLAY_DIALOG, GetString(IDS_ERROR_TITLE), GetString(IDS_ERROR_WRITE), _T("") );
	}
}

int FileOut::RenderEnd(TimeValue t)
{
	if ( m_bmOpen )
	{
		mp_writeBM->Close(m_pBi);
		m_bmOpen = FALSE;
	}
	return 1;
}

//---------------------------
// parser for frame sequences
//---------------------------
const TCHAR CZERO('0');
const TCHAR CNINE('9');
enum ItemType { FLSingle, FLRange, FLError, FLEnd };

inline BOOL IsNumeric(TCHAR c) { return ( c >= CZERO && c <= CNINE ) ? 1 : 0; }

static ItemType GetNextItem(TCHAR *s, int& nextc, int &n1, int &n2 )
{
	BOOL gotN1 = FALSE;
	BOOL gotN2 = FALSE;
	int i = nextc;
	for (;;)
	{
		if ( IsNumeric(s[i]) )
		{
			int n = s[i++] - CZERO;
			while ( IsNumeric(s[i]) )
			{
				n = n*10 + s[i++] - CZERO;
			}
			if (gotN1)
			{ n2 = n; gotN2 = TRUE; }
			else
			{ n1 = n; gotN1 = TRUE;	}
		} 
		else
		{
			switch(s[i])
			{
				case _T(' '):
					break;
				case _T(','):
					i++;
					// fall thru:
				case 0:
					// done with item
					if (!gotN1)
						return FLError;
					nextc = i;
					return gotN2 ? FLRange : FLSingle;
				case _T('-'):
					if (!gotN1)
						return FLError;
					break;
				default:
					 return FLError;
			}
			i++;
		}
	}
	return FLError;
}

static bool ParseFrameList(TCHAR *s, IntTab& nums)
{
	int n1, n2, nc(0);
	nums.SetCount(0);
	while (s[nc] != 0)
	{
		switch ( GetNextItem(s, nc, n1, n2) )
		{
			case FLSingle:
				nums.Append(1, &n1, 8);
				break;
			case FLRange:  		
			{
				if (n2 < n1) 
					return false;
				int n = nums.Count();
				nums.SetCount(n+n2-n1+1);
				for (int j=n1; j<=n2; j++) 
					nums[n+j-n1] = j;
				break;
			}
			case FLError:
				return false;
			case FLEnd:		
				return true;
		}
	}
	return true;
}

bool FileOut::GetStartEndFrames()
{
	Interface *ip = GetCOREInterface();
	int tpf = GetTicksPerFrame();

	switch ( ip->GetRendTimeType() )
	{
		case REND_TIMESINGLE:
			m_firstFrame = m_lastFrame = m_thisFrame;
			m_singleFrame = true;
			return true;

		case REND_TIMESEGMENT:
			m_firstFrame = GetAnimStart() / tpf;
			m_lastFrame  = GetAnimEnd()   / tpf;
			return true;

		case REND_TIMERANGE:
			m_firstFrame = ip->GetRendStart() / tpf;
			m_lastFrame  = ip->GetRendEnd()   / tpf;
			return true;

		case REND_TIMEPICKUP:
		{
			IntTab fNums;
			TSTR &frameStr = ip->GetRendPickFramesString();

			if ( !ParseFrameList(frameStr, fNums) ) 
				return false;
			else
			{
				m_firstFrame =  99999999;
				m_lastFrame  = -99999999;
				for (int i=0; i<fNums.Count(); i++)
				{
					if (fNums[i] < m_firstFrame) m_firstFrame = fNums[i];
					if (fNums[i] > m_lastFrame)  m_lastFrame  = fNums[i];
				}
			}
			return true;
		}
		default:
			return false;
	}
}

void FileOut::Apply(TimeValue t, Bitmap *bm, RenderGlobalContext *gc, CheckAbortCallback *checkAbort)
{
	mp_srcBM = bm;
	mp_lastCheckAbort = checkAbort;
	m_thisFrame = t / GetTicksPerFrame();
	m_singleFrame = (!gc) ? true : false;  // (!gc) indicates call from render effects dialog

	mp_pblock->GetValue(prmActive, t, m_active, FOREVER);

	mp_pblock->GetValue(prmBitmap, 0, m_pPBBMap, FOREVER); // not animatable - get value at time 0
	DbgAssert(m_pPBBMap);
	m_pBi = &(m_pPBBMap->bi);
	m_writing = ( _tcslen(m_pBi->Filename()) || _tcslen(m_pBi->Device()) ) ? TRUE : FALSE;

	mp_pblock->GetValue(prmAffectSrc, t, m_affectSrc, FOREVER);
	mp_pblock->GetValue(prmChanType, t, m_channel, FOREVER);
	if ( !m_active || ( !m_writing && ( !m_affectSrc || m_channel == image ) ) ) // with m_affectSrc, could be previewing but not writing file -- channel == image would not affect source
		return;

	processBMap();
}
