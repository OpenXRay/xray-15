/**********************************************************************
 *<
	FILE: utiltest.cpp

	DESCRIPTION:  A test bed for APIs

	CREATED BY: Rolf Berteig

	HISTORY: created January 20 1995

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/
#include "util.h"
#include "utilapi.h"
#include "istdplug.h"
#include "modstack.h"
#include "stdmat.h"
#include "bmmlib.h"
#include "SetKeyMode.h"
#include "iparamb2.h"

#define UTILTEST_CLASS_ID		0x99bb61a5

class UtilTest : public UtilityObj {
	public:
		IUtil *iu;
		Interface *ip;
		HWND hPanel;		

		UtilTest();
		void BeginEditParams(Interface *ip,IUtil *iu);
		void EndEditParams(Interface *ip,IUtil *iu);
		void DeleteThis() {}

		void Init(HWND hWnd);
		void Destroy(HWND hWnd);
		void MakeObject();
		void MakeGroup();
		void GroupObjs();
		void OpenGroup();
		void CloseGroup();
		void ExplodeGroup();
		void Ungroup();
		void SaveToFile();
		void LoadFromFile();
		void SetEnvironmentMap();
		void SetWAV();
		void SetAnimate(BOOL onOff);
		void RenderFrame();
		void SetSetKeyMode(BOOL onOff);
		void SetKeyCommit();
		void SetKeyRestore();
	};
static UtilTest theUtilTest;

class UtilTestClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) {return &theUtilTest;}
	const TCHAR *	ClassName() {return GetString(IDS_UTILITY_TESTER);}
	SClass_ID		SuperClassID() {return UTILITY_CLASS_ID;}
	Class_ID		ClassID() {return Class_ID(UTILTEST_CLASS_ID,0);}
	const TCHAR* 	Category() {return _T("");}
	};

static UtilTestClassDesc utilTestDesc;
ClassDesc* GetUtilTestDesc() {return &utilTestDesc;}


static INT_PTR CALLBACK UtilTestDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	switch (msg) {
		case WM_INITDIALOG:
			theUtilTest.Init(hWnd);			
			break;
		
		case WM_DESTROY:
			theUtilTest.Destroy(hWnd);
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:
					theUtilTest.iu->CloseUtility();
					break;

				case IDC_SETKEY_MODE:
					theUtilTest.SetSetKeyMode(
						IsDlgButtonChecked(hWnd, IDC_SETKEY_MODE));
					break;

				case IDC_SETKEY_COMMIT:
					theUtilTest.SetKeyCommit();
					break;

				case IDC_SETKEY_RESTORE:
					theUtilTest.SetKeyRestore();
					break;

				case IDC_TESTER_MAKEOBJECT:
					theUtilTest.MakeObject();
					break;

				case IDC_TESTER_MAKEGROUP:
					theUtilTest.MakeGroup();
					break;

				case IDC_TESTER_GROUPOBJS:
					theUtilTest.GroupObjs();
					break;

				case IDC_TESTER_OPENGROUP:
					theUtilTest.OpenGroup();
					break;

				case IDC_TESTER_CLOSEGROUP:
					theUtilTest.CloseGroup();
					break;

				case IDC_TESTER_EXPLODEGROUP:
					theUtilTest.ExplodeGroup();
					break;

				case IDC_TESTER_UNGROUP:
					theUtilTest.Ungroup();
					break;

				case IDC_TESTER_SAVETOFILE:
					theUtilTest.SaveToFile();
					break;

				case IDC_TESTER_LOADFROMFILE:
					theUtilTest.LoadFromFile();
					break;

				case IDC_TESTER_SETENV:
					theUtilTest.SetEnvironmentMap();
					break;

				case IDC_TESTER_SETWAV:
					theUtilTest.SetWAV();
					break;

				case IDS_TESTER_ANIMON:
					theUtilTest.SetAnimate(TRUE);
					break;

				case IDS_TESTER_ANIMOFF:
					theUtilTest.SetAnimate(FALSE);
					break;

				case IDS_TESTER_RENDER:
					theUtilTest.RenderFrame();
					break;
				}
			break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			theUtilTest.ip->RollupMouseMessage(hWnd,msg,wParam,lParam); 
			break;

		default:
			return FALSE;
		}
	return TRUE; 
	}


UtilTest::UtilTest()
	{

	}

void UtilTest::BeginEditParams(Interface *ip,IUtil *iu)
	{
	this->iu = iu;
	this->ip = ip;
	hPanel = ip->AddRollupPage(
		hInstance,
		MAKEINTRESOURCE(IDD_UTILTEST_PANEL),
		UtilTestDlgProc,
		_T("Util Test"),
		0);
	}

void UtilTest::EndEditParams(Interface *ip,IUtil *iu)
	{	
	this->iu = NULL;
	this->ip = NULL;
	ip->DeleteRollupPage(hPanel);
	hPanel = NULL;
	}

void UtilTest::Init(HWND hWnd)
	{
	CheckDlgButton(hWnd, IDC_SETKEY_MODE, GetSetKeyMode());
	}

void UtilTest::Destroy(HWND hWnd)
	{
	}

void UtilTest::MakeObject()
	{
	// Create a new object through the CreateInstance() API
	Object *obj = (Object*)ip->CreateInstance(
		GEOMOBJECT_CLASS_ID,
		Class_ID(CYLINDER_CLASS_ID,0));
	assert(obj);

	// Get a hold of the parameter block
	IParamArray *iCylParams = obj->GetParamBlock();
	assert(iCylParams);

	// Set the value of radius, height and segs.
	int rad = obj->GetParamBlockIndex(CYLINDER_RADIUS);
	assert(rad>=0);
	iCylParams->SetValue(rad,TimeValue(0),30.0f);
	int height = obj->GetParamBlockIndex(CYLINDER_HEIGHT);
	assert(height>=0);
	iCylParams->SetValue(height,TimeValue(0),100.0f);
	int segs = obj->GetParamBlockIndex(CYLINDER_SEGMENTS);
	assert(segs>=0);
	iCylParams->SetValue(segs,TimeValue(0),10);

	// Create a derived object that references the cylinder
	IDerivedObject *dobj = CreateDerivedObject(obj);

	// Create a bend modifier
	Modifier *bend = (Modifier*)ip->CreateInstance(
		OSM_CLASS_ID,
		Class_ID(BENDOSM_CLASS_ID,0));

	// Set the bend angle - ParamBlock2
	IParamBlock2* iBendBlock = ((Animatable*)bend)->GetParamBlock(0);  //only one pblock2
	assert(iBendBlock);
	iBendBlock->SetValue(BEND_ANGLE,TimeValue(0),90.0f);

	// Add the bend modifier to the derived object.
	dobj->AddModifier(bend);

	// Create a node in the scene that references the derived object
	INode *node = ip->CreateObjectNode(dobj);
	
	// Name the node and make the name unique.
	TSTR name(_T("MyNode"));
	ip->MakeNameUnique(name);
	node->SetName(name);

	// Get ready to add WSMs to this node
	node->CreateWSMDerivedObject();
	IDerivedObject *wsdobj = node->GetWSMDerivedObject();
	if (wsdobj) {
		WSMObject *swobj = (WSMObject*)ip->CreateInstance(
			WSM_OBJECT_CLASS_ID,
			Class_ID(SINEWAVE_OBJECT_CLASS_ID,0));
		int ix;
		IParamArray *iRipParams = swobj->GetParamBlock();
		
		ix = swobj->GetParamBlockIndex(RWAVE_AMPLITUDE);
		iRipParams->SetValue(ix,TimeValue(0),10.0f);

		ix = swobj->GetParamBlockIndex(RWAVE_AMPLITUDE2);
		iRipParams->SetValue(ix,TimeValue(0),10.0f);

		ix = swobj->GetParamBlockIndex(RWAVE_WAVELEN);
		iRipParams->SetValue(ix,TimeValue(0),40.0f);

		ix = swobj->GetParamBlockIndex(RWAVE_CIRCLES);
		iRipParams->SetValue(ix,TimeValue(0),10);

		ix = swobj->GetParamBlockIndex(RWAVE_DIVISIONS);
		iRipParams->SetValue(ix,TimeValue(0),4);

		ix = swobj->GetParamBlockIndex(RWAVE_SEGMENTS);
		iRipParams->SetValue(ix,TimeValue(0),16);

		INode *swnode = ip->CreateObjectNode(swobj);
		
		TSTR swname(_T("RippleNode"));
		ip->MakeNameUnique(swname);
		node->SetName(swname);

		// Create a Space Warp Modifier
		Modifier *swmod = swobj->CreateWSMMod(swnode);
		if (swmod) {
			wsdobj->AddModifier(swmod);
			}
		}
	
	// Redraw the views
	ip->RedrawViews(ip->GetTime());
	}

static INode *MakeGroupObj(Interface *ip,Point3 p)
	{
	Object *obj = (Object*)ip->CreateInstance(
		GEOMOBJECT_CLASS_ID,
		Class_ID(CYLINDER_CLASS_ID,0));
	IParamArray *iCylParams = obj->GetParamBlock();
	int rad = obj->GetParamBlockIndex(CYLINDER_RADIUS);	
	iCylParams->SetValue(rad,TimeValue(0),20.0f);
	int height = obj->GetParamBlockIndex(CYLINDER_HEIGHT);	
	iCylParams->SetValue(height,TimeValue(0),50.0f);
	INode *node = ip->CreateObjectNode(obj);
	Matrix3 tm(1);
	tm.SetTrans(p);
	node->SetNodeTM(0,tm);
	return node;
	}

void UtilTest::MakeGroup()
	{
	INode *node1, *node2, *node3, *node4, *gnode1, *gnode2;

	node1 = MakeGroupObj(ip,Point3(-50,0,0));
	node2 = MakeGroupObj(ip,Point3( 50,0,0));
	node3 = MakeGroupObj(ip,Point3(-50,100,0));
	node4 = MakeGroupObj(ip,Point3( 50,100,0));
	
	INodeTab tab1;
	tab1.Append(1,&node1);
	tab1.Append(1,&node2);
	
	INodeTab tab2;
	tab2.Append(1,&node3);
	tab2.Append(1,&node4);
	
	TSTR nam1("Group1");
	TSTR nam2("Group2");
	gnode1 = ip->GroupNodes(&tab1,&nam1,FALSE);
	gnode2 = ip->GroupNodes(&tab2,&nam2,FALSE);
	
	INodeTab tab3;
	tab3.Append(1,&gnode1);
	tab3.Append(1,&gnode2);
	TSTR nam3("Main Group");
	ip->GroupNodes(&tab3,&nam3,FALSE);

	ip->RedrawViews(ip->GetTime());
	}

void UtilTest::GroupObjs()
	{
	ip->GroupNodes();
	ip->RedrawViews(ip->GetTime());
	}

void UtilTest::OpenGroup()
	{
	ip->OpenGroup();
	ip->RedrawViews(ip->GetTime());
	}

void UtilTest::CloseGroup()
	{
	ip->CloseGroup();
	ip->RedrawViews(ip->GetTime());
	}

void UtilTest::ExplodeGroup()
	{
	ip->ExplodeNodes();
	ip->RedrawViews(ip->GetTime());
	}

void UtilTest::Ungroup()
	{
	ip->UngroupNodes();
	ip->RedrawViews(ip->GetTime());
	}

void UtilTest::SaveToFile()
	{
	ip->SaveToFile(_T("c:\\savetest.max"));
	MessageBox(hPanel,_T("Save Completed."),_T("SaveToFile() Test"),MB_OK);
	}

void UtilTest::LoadFromFile()
	{
	ip->LoadFromFile(_T("c:\\savetest.max"));
	MessageBox(hPanel,_T("File Loaded."),_T("LoadFromFile() Test"),MB_OK);
	}

void UtilTest::SetEnvironmentMap()
	{
	// Make a bitmap texture map.
	BitmapTex *map = NewDefaultBitmapTex();
	
	// Get the UVGen
	StdUVGen *uvGen = map->GetUVGen();
	
	// Set up the coords. to be screen environment.
	uvGen->SetCoordMapping(UVMAP_SCREEN_ENV);

	// Set the bitmap file.
	map->SetMapName(_T("A_MAX.TGA"));

	// Make this the new environment map.
	ip->SetEnvironmentMap(map);
	}

void UtilTest::SetWAV()
	{
	// Get the current sound object.
	SoundObj *snd = ip->GetSoundObject();
	
	// See if we can get a wave interface
	IWaveSound *iWav = GetWaveSoundInterface(snd);
	if (iWav) {
		// Set the sound file
		if (!iWav->SetSoundFileName(_T("test.wav"))) {
			MessageBox(hPanel,
				_T("Unable to load TEST.WAV"),
				_T("Util Test"),MB_OK);
			return;
			}
		// Set the offset to 10 frames.
		iWav->SetStartTime(GetTicksPerFrame() * 10);
	} else {
		MessageBox(hPanel,
			_T("No IWaveSound interface"),
			_T("Util Test"),MB_OK);
		}
	}

void UtilTest::SetAnimate(BOOL onOff)
	{
	ip->SetAnimateButtonState(onOff);
	}

void UtilTest::RenderFrame()
	{
	int res;
	
	// Create a blank bitmap
	static Bitmap *bm = NULL;
	if (!bm) {
		BitmapInfo bi;		
		bi.SetWidth(320);
		bi.SetHeight(200);
		bi.SetType(BMM_TRUE_64);
		bi.SetFlags(MAP_HAS_ALPHA);
		bi.SetAspect(1.0f);
		bm = TheManager->Create(&bi);
		}

	// Get the active viewport to render
	ViewExp *view = ip->GetActiveViewport();
	
	// Display the bitmap
	bm->Display(_T("Test"));

	// Open up the renderer, render a frame and close it.
	res = ip->OpenCurRenderer(NULL,view);
	res = ip->CurRendererRenderFrame(
		ip->GetTime(),bm);
	ip->CloseCurRenderer();	

	// We're done with the viewport.
	ip->ReleaseViewport(view);
	}

//-----------------------------------------------------------

#define TEST_SOUNDOBJ_CLASS_ID	0x5eda4197

class TestSoundObj : public SoundObj {
	public:		
		BOOL Play(TimeValue tStart,TimeValue t0,TimeValue t1,TimeValue frameStep)
			{ return FALSE;}
		void Scrub(TimeValue t0,TimeValue t1) {}
		TimeValue Stop() {return 0;}
		TimeValue GetTime() {return 0;}
		BOOL Playing() {return FALSE;}
		void SaveSound(PAVIFILE pfile,TimeValue t0,TimeValue t1) {}
		void SetMute(BOOL mute) {}
		BOOL IsMute() {return FALSE;}

		void DeleteThis() {delete this;}
		Class_ID ClassID() {return Class_ID(TEST_SOUNDOBJ_CLASS_ID,0);}
		void GetClassName(TSTR& s) {s=_T("Test Sound Object");}

		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
	         PartID& partID,  RefMessage message) {return REF_SUCCEED;}
	};

class TestSoundObjClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1;}
	void *			Create(BOOL loading) {return new TestSoundObj;}
	const TCHAR *	ClassName() { return _T("Test Sound Object"); }
	SClass_ID		SuperClassID() { return SClass_ID(SOUNDOBJ_CLASS_ID); }
	Class_ID 		ClassID() { return Class_ID(TEST_SOUNDOBJ_CLASS_ID,0); }
	const TCHAR* 	Category() { return _T("");  }
	};
static TestSoundObjClassDesc sndObjDesc;
ClassDesc *GetTestSoundObjDescriptor() {return &sndObjDesc;}

//----------------------------------------------------------------

void UtilTest::SetSetKeyMode(BOOL onOff)
	{
	SetKeyModeInterface *iSetKey = GetSetKeyModeInterface(ip);
	if (iSetKey) {
		iSetKey->ActivateSetKeyMode(onOff);
		}
	ip->RedrawViews(ip->GetTime());
	}

void UtilTest::SetKeyCommit()
	{
	SetKeyModeInterface *iSetKey = GetSetKeyModeInterface(ip);
	if (iSetKey) {
		iSetKey->AllTracksCommitSetKeyBuffer();
		}
	ip->RedrawViews(ip->GetTime());
	}

void UtilTest::SetKeyRestore()
	{
	SetKeyModeInterface *iSetKey = GetSetKeyModeInterface(ip);
	if (iSetKey) {
		iSetKey->AllTracksRevertSetKeyBuffer();
		}
	ip->RedrawViews(ip->GetTime());
	}

