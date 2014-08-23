/*----------------------------------------------------------------------*
 |
 |	FILE: SManager.cpp
 | 
 |	DESC: Scene Manager plugin
 |		  Uses Christer's SceneAPI (CJAPIEXT)
 |
 |	AUTH: Harry Denholm, Kinetix
 |		  Copyright (c) 1998, All Rights Reserved.
 |
 |	HISTORY: 27.2.98
 |
 *----------------------------------------------------------------------*/


#include "scenemgr.h"

#define SMANAGER_CLASS_ID	Class_ID(0xb21d7f80, 0x5fc6636a)

HINSTANCE hInstance;
int controlsInit = FALSE;

BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved)
{
	hInstance = hinstDLL;

	if (!controlsInit) {
		controlsInit = TRUE;
		InitCustomControls(hInstance);
		InitCommonControls();
	}
			
	return (TRUE);
}

__declspec( dllexport ) const TCHAR* LibDescription()
{
	return GetString(IDS_LIBDESCRIPTION);
}

//TODO: Must change this number when adding a new class
__declspec( dllexport ) int LibNumberClasses()
{
	return 1;
}

__declspec( dllexport ) ClassDesc* LibClassDesc(int i)
{
	switch(i) {
		case 0: return GetSManagerDesc();
		default: return 0;
	}
}

__declspec( dllexport ) ULONG LibVersion()
{
	return VERSION_3DSMAX;
}

TCHAR *GetString(int id)
{
	static TCHAR buf[256];

	if (hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
}

class SManager : public UtilityObj {
	public:

		IUtil *iu;
		Interface *ip;
		HWND hPanel;
		
		//Constructor/Destructor
		SManager();
		~SManager();
		
		HWND hMapper;
		int m1,m2,m3,m4,m5;
		float totalRAM; 
		NameTab biDB; NameTab mtDB; NameTab txDB;


		void BeginEditParams(Interface *ip,IUtil *iu);
		void EndEditParams(Interface *ip,IUtil *iu);
		void DeleteThis() {}

		void Init(HWND hWnd);
		void Destroy(HWND hWnd);
		void Output(const TCHAR *format, ...);
		void SetStatus(const TCHAR *format, ...);

		void RefreshScanner(HWND hWnd);

		void DoObjects();
		int facecount,vertcount;
		void TraverseNode(INode* node, TimeValue t);
		void CountFaces(Interface* ip);
		
		void DoUVW();
		int UV1count,UV2count;
		void TraverseNodeUVW(INode* node, TimeValue t);
		void CountUVW(Interface* ip);

		void DoMaps();
		void DoDups();
		void DoLights();


};



static SManager MM;


class SManagerClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) {return &MM;}
	const TCHAR *	ClassName() {return GetString(IDS_CLASS_NAME);}
	SClass_ID		SuperClassID() {return UTILITY_CLASS_ID;}
	Class_ID		ClassID() {return SMANAGER_CLASS_ID;}
	const TCHAR* 	Category() {return _T("");}
	void ResetClassParams (BOOL fileReset);
};

static SManagerClassDesc SManagerDesc;
ClassDesc* GetSManagerDesc() {return &SManagerDesc;}

//TODO: Should implement this method to reset the plugin params when Max is reset
void SManagerClassDesc::ResetClassParams (BOOL fileReset) 
{

}



void SManager::Output(const TCHAR *format, ...){
	TCHAR buf[512];
	va_list args;
	va_start(args,format);
	_vsntprintf(buf,512,format,args);
	va_end(args);

	SendMessage(GetDlgItem(hMapper,IDC_OUT),LB_ADDSTRING,0,(LPARAM)buf);
}

void SManager::SetStatus(const TCHAR *format, ...){
	TCHAR buf[512];
	va_list args;
	va_start(args,format);
	_vsntprintf(buf,512,format,args);
	va_end(args);

	SetWindowText(GetDlgItem(hMapper,IDC_STATUS),buf);
	InvalidateRect(GetDlgItem(hMapper,IDC_STATUS),NULL,TRUE);
}



void SManager::RefreshScanner(HWND hWnd){
				HMENU hMenu = GetMenu(hWnd);
				if(m1==0){
					CheckMenuItem(GetSubMenu(hMenu, 1),ID_1,MF_UNCHECKED );
				}
				else {
					CheckMenuItem(GetSubMenu(hMenu, 1),ID_1,MF_CHECKED );
				}

				if(m2==0){
					CheckMenuItem(GetSubMenu(hMenu, 1),ID_2,MF_UNCHECKED );
				}
				else {
					CheckMenuItem(GetSubMenu(hMenu, 1),ID_2,MF_CHECKED );
				}
				if(m3==0){
					CheckMenuItem(GetSubMenu(hMenu, 1),ID_3,MF_UNCHECKED );
				}
				else {
					CheckMenuItem(GetSubMenu(hMenu, 1),ID_3,MF_CHECKED );
				}
				if(m4==0){
					CheckMenuItem(GetSubMenu(hMenu, 1),ID_4,MF_UNCHECKED );
				}
				else {
					CheckMenuItem(GetSubMenu(hMenu, 1),ID_4,MF_CHECKED );
				}
				if(m5==0){
					CheckMenuItem(GetSubMenu(hMenu, 1),ID_5,MF_UNCHECKED );
				}
				else {
					CheckMenuItem(GetSubMenu(hMenu, 1),ID_5,MF_CHECKED );
				}


}

static INT_PTR CALLBACK MMDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{

	switch(msg){

		case WM_INITDIALOG:
			MM.RefreshScanner(hWnd);
		break;

		case WM_DESTROY:
			MM.biDB.SetSize(0); MM.mtDB.SetSize(0); MM.txDB.SetSize(0);
			// kill variables
			MM.totalRAM=0; MM.vertcount=0; MM.facecount=0;
			MM.UV1count=MM.UV2count=0; 
		break;


		case WM_COMMAND:
		switch (LOWORD(wParam)) {

			case IDC_SAVE:{
				OPENFILENAME ofn;
				TCHAR filename[255] = _T("");

				memset(&ofn, 0, sizeof(OPENFILENAME));
				ofn.lStructSize = sizeof(OPENFILENAME);
				ofn.hwndOwner = hWnd;
				ofn.hInstance = hInstance;
				ofn.lpstrFile = filename;
				ofn.nMaxFile = sizeof(filename) / sizeof(TCHAR);
				ofn.lpstrFilter = _T("Text File\0*.txt\0");
				ofn.nFilterIndex = 1;
				ofn.lpstrTitle   = _T("Choose Output");
				ofn.lpstrFileTitle = NULL;
				ofn.nMaxFileTitle = 0;
				ofn.lpstrInitialDir = MM.ip->GetDir(APP_SCENE_DIR);
				ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

				if (GetSaveFileName(&ofn))
				{
					if(_tcsicmp(filename + _tcslen(filename) - 4, ".txt")) 
						strcat(filename,".txt");

					FILE *stream;
					stream = fopen(filename,"wt");
					HWND outHwnd = GetDlgItem(hWnd,IDC_OUT);
					if (stream)
					{
						for(int x=0; x<(SendMessage(outHwnd,LB_GETCOUNT,0,0));x++)
						{
							char q[255];
							SendMessage(outHwnd,LB_GETTEXT,x,(WPARAM)q);
							fprintf(stream,"%s\n",q);
						}
						fclose(stream);
					}
				}
			break;}

			case IDC_CLOSE:
				MM.hMapper=NULL;
				EndDialog(hWnd,1);
			break;
			case IDC_CLEAR:
				SendMessage(GetDlgItem(hWnd,IDC_OUT),LB_RESETCONTENT,0,0);
				InvalidateRect(GetDlgItem(hWnd,IDC_OUT),NULL,TRUE); 

			break;
			case IDC_GENERATE:
					
					// kill variables
					MM.totalRAM=0; MM.vertcount=0; MM.facecount=0;
					MM.UV1count=MM.UV2count=0; 
					MM.biDB.SetSize(0); MM.mtDB.SetSize(0); MM.txDB.SetSize(0);

					MM.SetStatus("");
					MM.Output("--- START OF REPORT ---");
					MM.Output("");
					MM.Output("");
					if(MM.m1==1) MM.DoObjects();
					if(MM.m2==1) MM.DoUVW();
					if(MM.m3==1) MM.DoMaps();
					if(MM.m4==1) MM.DoDups();
					if(MM.m5==1) MM.DoLights();
					MM.Output("");
					MM.Output("Total Memory Usage: %.2fMB",(float)MM.totalRAM/1024.0f);
					MM.Output("");MM.Output("--- END OF REPORT ---");MM.Output("");MM.Output("");
					MM.SetStatus("");
				break;

			case ID_ALL:
				MM.m1=MM.m2=MM.m3=MM.m4=MM.m5=1;MM.RefreshScanner(hWnd);
			break;
			case ID_NONE:
				MM.m1=MM.m2=MM.m3=MM.m4=MM.m5=0;MM.RefreshScanner(hWnd);
			break;

			case ID_1:{
				if(MM.m1==1){MM.m1=0;}
				else {MM.m1=1;}MM.RefreshScanner(hWnd);
			break;}
			case ID_2:{
				if(MM.m2==1){MM.m2=0;}
				else {MM.m2=1;}MM.RefreshScanner(hWnd);
			break;}
			case ID_3:{
				if(MM.m3==1){MM.m3=0;}
				else {MM.m3=1;}MM.RefreshScanner(hWnd);
			break;}
			case ID_4:{
				if(MM.m4==1){MM.m4=0;}
				else {MM.m4=1;}MM.RefreshScanner(hWnd);
			break;}
			case ID_5:{
				if(MM.m5==1){MM.m5=0;}
				else {MM.m5=1;}MM.RefreshScanner(hWnd);
			break;}

		}
		break;

		case WM_SIZE:{
				MoveWindow(GetDlgItem(hWnd,IDC_OUT),
					0,
					0, 
					LOWORD(lParam),
					HIWORD(lParam)-22,TRUE);
				Rect rw;
				GetWindowRect(GetDlgItem(hWnd,IDC_OUT),&rw);
				MoveWindow(GetDlgItem(hWnd,IDC_STATUS),
					0,
					rw.h()+1, 
					LOWORD(lParam),
					20,TRUE);
			break;}

		case WM_CLOSE:
			MM.hMapper=NULL;
			EndDialog(hWnd,1);
			break;
	}

	
	return FALSE;
	}


static INT_PTR CALLBACK SManagerDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
		case WM_INITDIALOG:
			MM.Init(hWnd);
			break;

		case WM_DESTROY:
			MM.Destroy(hWnd);
			break;

		case WM_ENABLE:
			if(MM.hMapper!=NULL) EnableWindow(MM.hMapper,(BOOL) wParam);
			break;

		case WM_COMMAND:
		switch (LOWORD(wParam)) {



			case IDC_LAUNCH:
				if(MM.hMapper==NULL) MM.hMapper = 
					CreateDialog(
					hInstance,
					MAKEINTRESOURCE(IDD_SM_FLOATER),
					GetCOREInterface()->GetMAXHWnd(),
					MMDlgProc);
				break;
		}
			break;

		default:
			return FALSE;
	}
	return TRUE;
}



//--- SManager -------------------------------------------------------
SManager::SManager()
{
	iu = NULL;
	hPanel = NULL;
	m1=m2=m3=m4=m5=1;
	totalRAM=0;
}

SManager::~SManager()
{

}

void SManager::BeginEditParams(Interface *ip,IUtil *iu) 
{
	this->iu = iu;
	this->ip = ip;
	hPanel = ip->AddRollupPage(
		hInstance,
		MAKEINTRESOURCE(IDD_SM_PANEL),
		SManagerDlgProc,
		GetString(IDS_PARAMS),
		0);
}
	
void SManager::EndEditParams(Interface *ip,IUtil *iu) 
{
	this->iu = NULL;
	ip->DeleteRollupPage(hPanel);
	hPanel = NULL;
}

void SManager::Init(HWND hWnd)
{
}

void SManager::Destroy(HWND hWnd)
{

}


//****************************************************************
// GENERATORS
//****************************************************************

class NullView : public View {

public:

Point2 ViewToScreen(Point3 p)

{ return Point2(p.x,p.y); }

NullView() {

worldToView.IdentityMatrix();
screenW=640.0f; screenH = 480.0f;
}

};

//****************************************************************
// GEOMETRY SCAN
//****************************************************************

class BeginEnum : public RefEnumProc {
	TimeValue t;
	public:
		BeginEnum(TimeValue time) { t = time; }
		void proc(ReferenceMaker *m) { 
				m->RenderBegin(t);  
			}
	};

class EndEnum : public RefEnumProc {
	TimeValue t;
	public:
		EndEnum(TimeValue time) { t = time; }
		void proc(ReferenceMaker *m) { 
				m->RenderEnd(t);  
			}
	};


void EnumRefs(RefEnumProc& renum,INode *inode) {
	EnumRefHierarchy(inode,renum);
	}


// Traverse the node counting polygons.  
void SManager::TraverseNode(INode* node, TimeValue t)
{
	BOOL deleteMesh;
	NullView nw;
	MM.SetStatus("Examining Object : %s",node->GetName());

	//node->RenderBegin(t);
	BeginEnum beginEnum(ip->GetTime());
	EnumRefs(beginEnum,node);

    const ObjectState& os = node->EvalWorldState(t);
    Object* ob = os.obj;
    if (ob!=NULL) {

        if (ob->SuperClassID() == GEOMOBJECT_CLASS_ID) {
            GeomObject* geo = (GeomObject *)ob;
            if(geo->CanConvertToType(triObjectClassID)) {

				MM.Output("%s :",node->GetName());
		               
				Mesh* mesh = geo->GetRenderMesh(t, node, nw, deleteMesh);

                facecount += mesh->getNumFaces();
				float fclc = (mesh->getNumFaces()*34.0f)/1024.0f;
                vertcount += mesh->getNumVerts();
				float vclc = (mesh->getNumVerts()*28.0f)/1024.0f;
				
				MM.Output("    * Face Count: %i -> %.1f kB",
					mesh->getNumFaces(),
					fclc);
				MM.Output("    * Vertex Count: %i -> %.1f kB",
					mesh->getNumVerts(),
					vclc);

                	if (deleteMesh) {
						delete mesh;
					}

            }
        }
    }
	//node->RenderEnd(t);
	EndEnum endEnum(ip->GetTime());
	EnumRefs(endEnum,node);


    int i, numChildren = node->NumberOfChildren();
    for(i=0; i<numChildren; i++)
        TraverseNode(node->GetChildNode(i), t);
}

// Traverse all the nodes in the scene graph.
void SManager::CountFaces(Interface* ip)
{
    TraverseNode(ip->GetRootNode(), ip->GetTime());
}

void SManager::DoObjects(){
	MM.Output("*** Beginning Object Pass  ------------------------------------------------------------------------");
	MM.Output("***");
	MM.SetStatus("Beginning Object Pass");
	CountFaces(ip);
	float fll = (facecount*34.0f)/1024.0f;
	float vll = (vertcount*28.0f)/1024.0f;
	float sll = ((facecount*34.0f)+(vertcount*28.0f))/1024.0f;
	MM.Output("");
	MM.Output("Total Scene Faces: %i -> %.1f kB",facecount,fll);
	MM.Output("Total Scene Vertices: %i -> %.1f kB",vertcount,vll);
	MM.Output("Total Geometry RAM: %.2f kB   |   %.2f MB",sll,sll/1024.0f);
	MM.Output("");

	totalRAM+=sll;

}

//****************************************************************
// UVW MAPPING SCAN
//****************************************************************

// Traverse the node counting UVW.  
void SManager::TraverseNodeUVW(INode* node, TimeValue t)
{
	BOOL deleteMesh;
	NullView nw;
	
	MM.SetStatus("Examining Object UVWs : %s",node->GetName());

	//node->RenderBegin(t);
	BeginEnum beginEnum(ip->GetTime());
	EnumRefs(beginEnum,node);


    const ObjectState& os = node->EvalWorldState(t);
    Object* ob = os.obj;
    if (ob!=NULL) {

		
        if (ob->SuperClassID() == GEOMOBJECT_CLASS_ID) {
            GeomObject* geo = (GeomObject *)ob;
            if(geo->CanConvertToType(triObjectClassID)) {
        
				Mesh* mesh = geo->GetRenderMesh(t, node, nw, deleteMesh);


				// are there any mapping coords to speak of?
				if(mesh->getNumVertCol()>0||
					mesh->getNumTVerts()>0) MM.Output("%s :",node->GetName());


                UV2count += mesh->getNumVertCol();
				float fclc = (mesh->getNumVertCol()*12.0f)/1024.0f;
                UV1count += mesh->getNumTVerts();
				float vclc = (mesh->getNumTVerts()*12.0f)/1024.0f;
				
				if(mesh->getNumTVerts()>0) {MM.Output("    * UV Channel 1: %i -> %.1f kB",
					mesh->getNumTVerts(),
					vclc);}
				if(mesh->getNumVertCol()>0) {MM.Output("    * UV Channel 2: %i -> %.1f kB",
					mesh->getNumVertCol(),
					fclc);}

                	if (deleteMesh) {
						delete mesh;
					}

            }
        }
    }
	//node->RenderEnd(t);
	EndEnum endEnum(ip->GetTime());
	EnumRefs(endEnum,node);


    int i, numChildren = node->NumberOfChildren();
    for(i=0; i<numChildren; i++)
        TraverseNodeUVW(node->GetChildNode(i), t);
}

// Traverse all the nodes in the scene graph.
void SManager::CountUVW(Interface* ip)
{
    TraverseNodeUVW(ip->GetRootNode(), ip->GetTime());
}

void SManager::DoUVW(){
	MM.Output("*** Beginning UVW Mapping Pass  -------------------------------------------------------------------");
	MM.Output("***");
	MM.SetStatus("Beginning UVW Mapping Pass");
	CountUVW(ip);
	float fll = (UV1count*12.0f)/1024.0f;
	float vll = (UV2count*12.0f)/1024.0f;
	float sll = ((UV1count*12.0f)+(UV2count*12.0f))/1024.0f;
	MM.Output("");
	MM.Output("Total UV Channel 1: %i -> %.1f kB",UV1count,fll);
	MM.Output("Total UV Channel 2: %i -> %.1f kB",UV2count,vll);
	MM.Output("Total Mapping RAM: %.2f kB   |   %.2f MB",sll,sll/1024.0f);
	MM.Output("");

	totalRAM+=sll;

}

int bmp_bpp(BitmapInfo bi){

	//calculate bits/pixel
	int bpp;
	if(bi.Type()==BMM_LINE_ART)		bpp=1;
	if(bi.Type()==BMM_GRAY_8)		bpp=8;
	if(bi.Type()==BMM_GRAY_16)		bpp=16;
	if(bi.Type()==BMM_TRUE_16)		bpp=16;
	if(bi.Type()==BMM_TRUE_32)		bpp=32;
	if(bi.Type()==BMM_TRUE_64)		bpp=64;
	if(bi.Type()==BMM_TRUE_24)		bpp=24;
	if(bi.Type()==BMM_TRUE_48)		bpp=48;
	if(bi.Type()==BMM_BMP_4)		bpp=4;
	if(bi.Type()==BMM_PALETTED)		bpp=8;
	return bpp;
}
//****************************************************************
// BITMAP SCAN
//****************************************************************
class MtlEnum {
	public:
	char mname[255];

		virtual void  proc(MtlBase *m){

			if(m->ClassID()==Class_ID(BMTEX_CLASS_ID,0)) {
				Texmap *bt = (Texmap*)m;
				BitmapTex *b = (BitmapTex*)bt;

				if(_stricmp(b->GetMapName(),"")!=0){
					
					Bitmap *bmp = b->GetBitmap(MM.ip->GetTime());

					if(BMMIsFile(b->GetMapName())){
					BitmapInfo bi = bmp->Storage()->bi;
					
					if(MM.biDB.FindName(b->GetMapName())==-1){
						
						float mem = (float)((bmp->Width()*bmp->Height()*bmp_bpp(bi))/1024)/8;

						MM.Output("[%s / %s] : %s",mname,b->GetName(),b->GetMapName());
						MM.Output("    * RAM Statistics:  W:%i, H:%i, BPP:%i  ->  %.1f kB",
							bmp->Width(),bmp->Height(),bmp_bpp(bi),
							mem);

						int FT = b->GetFilterType();
						if (FT==FILTER_PYR) (float)mem*=1.33f;
						if (FT==FILTER_SAT) (float)mem*=4.0f;
						MM.Output("    * Total After Filtering:  %.1f MB",mem/1024.0f);
						MM.Output("");

						MM.totalRAM+=(int)(mem);

						MM.biDB.AddName (b->GetMapName());
						MM.txDB.AddName (b->GetName());
						MM.mtDB.AddName (mname);
					}

					}else{
						char p[255],f[255],e[255];
						BMMSplitFilename(b->GetMapName(), p, f, e);
						MM.Output("[%s / %s] : %s",mname,b->GetName(),b->GetMapName());
 						MM.Output("    * Map file '%s%s' could not be found!",f,e);
						MM.Output("");
						
					}

				}
				else{
					MM.Output("Map Found: [Undefined/Unallocated]");
				}
			}
		}
	};


MtlEnum Enym;


// do the material enumeration
void EnumMtlTree(MtlBase *mb, MtlEnum &tenum) {
	tenum.proc(mb);
	for (int i=0; i<mb->NumSubTexmaps(); i++) {
		Texmap *st = mb->GetSubTexmap(i); 
		if (st) 
			EnumMtlTree(st,tenum);
		}
	if (IsMtl(mb)) {
		strcpy(tenum.mname,mb->GetName());
		Mtl *m = (Mtl *)mb;
		for (i=0; i<m->NumSubMtls(); i++) {
			Mtl *sm = m->GetSubMtl(i);
			if (sm) 
				EnumMtlTree(sm,tenum);
			}
		}
	}



void SManager::DoMaps(){
	MM.Output("*** Beginning Bitmap Materials Pass  --------------------------------------------------------------");
	MM.Output("***");
	MM.SetStatus("Beginning Bitmap Materials Pass");

	// Use the SceneAPI to check the material editor for anything..
	SceneAPI sceneapi(ip);
	
	MtlBaseLib* mlib;
	mlib = sceneapi.GetSceneMtls();

	if (mlib) {
		int numMtls = mlib->Count();
		for (int i=0; i<numMtls; i++) {
			MtlBase* mat = (*mlib)[i];
			EnumMtlTree(mat,Enym);				
		}
	}
	MM.Output("");
	MM.Output("*** Checking for duplicate map names...");
	MM.SetStatus("Checking for duplicate map names");

	char p[255],f[255],e[255];
	char p2[255],f2[255],e2[255];
	for(int y=0;y<biDB.Count();y++){
		BMMSplitFilename(biDB[y], p, f, e);

		for(int d=0;d<biDB.Count();d++){
			BMMSplitFilename(biDB[d], p2, f2, e2);
			if(_stricmp(f,f2)==0&&_stricmp(e,e2)==0&&_stricmp(p,p2)!=0){
				MM.Output("    * Duplicated map in [%s / %s] : %s%s%s",MM.mtDB[d],MM.txDB[d],p2,f2,e2);
			}
		}

	}
	MM.Output("");

}


// Enumerate the scene for lights
void LiteEnum(INode *root)
{
	for (int k=0; k<root->NumberOfChildren(); k++)
	{
		INode *node = root->GetChildNode(k);
		Object* ob = node->GetObjectRef();
		if(ob!=NULL&&ob->SuperClassID()==LIGHT_CLASS_ID)
		{
			LightObject *ltmp = (LightObject*) ob;
			int mSz = ltmp->GetMapSize(MM.ip->GetTime());
			if((mSz!=0)&&(ltmp->GetShadow())&&(ltmp->GetShadowMethod()==LIGHTSHADOW_MAPPED)){
				float msRAM = ((mSz*mSz)*4.0f)/1024.0f;
				MM.Output("%s -> Shadow Map size: %i --> %0.1f kB",node->GetName(),mSz,msRAM);
				MM.totalRAM+=(int)(msRAM);
			}
		}
		if(node->NumberOfChildren()>0) LiteEnum(node);
	}
}

void SManager::DoDups(){
	MM.Output("*** Beginning Lighting Pass  ----------------------------------------------------------------------");
	MM.Output("");
	INode *root = ip->GetRootNode();
	LiteEnum(root);
	MM.Output("");
}

void SManager::DoLights(){
	MM.Output("*** Beginning Render RAM Pass  --------------------------------------------------------------------");
	MM.Output("");
	int rWidth = ip->GetRendWidth();
	int rHeight = ip->GetRendHeight();
	float RAMrw = ((rWidth*rHeight)*8.0f)/1024.0f;

	MM.Output("Render Bitmap output: %ix%i --> %.1f kB / %.1f MB",rWidth,rHeight,RAMrw,RAMrw/1024.0f);

	MM.totalRAM+=(int)(RAMrw);
}
