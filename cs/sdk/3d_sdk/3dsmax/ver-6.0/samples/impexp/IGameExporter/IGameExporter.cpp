/**********************************************************************
 *<
	FILE: IGameExporter.cpp

	DESCRIPTION:	Sample to test the IGame Interfaces.  It follows a similar format
					to the Ascii Exporter.  However is does diverge a little in order
					to handle properties etc..

	TODO: Break the file down into smaller chunks for easier loading.

	CREATED BY:		Neil Hazzard	

	HISTORY:		parttime coding Summer 2002

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/

#include "msxml2.h"
#include "IGameExporter.h"
#include "XMLUtility.h"
#include "decomp.h"

#include "IGame.h"
#include "IGameObject.h"
#include "IGameProperty.h"
#include "IGameControl.h"
#include "IGameModifier.h"
#include "IConversionManager.h"
#include "IGameError.h"


#define IGAMEEXPORTER_CLASS_ID	Class_ID(0x79d613a4, 0x4f21c3ad)

#define BEZIER	0
#define TCB		1
#define LINEAR	2
#define SAMPLE	3


class IGameExporter : public SceneExport {
	public:

		IGameScene * pIgame;

		IXMLDOMDocument * pXMLDoc;
		IXMLDOMNode * pRoot;		//this is our root node 	
		CComPtr<IXMLDOMNode> iGameNode;	//the IGame child - which is the main node
		CComPtr<IXMLDOMNode> rootNode;
		static HWND hParams;

		int curNode;

		int staticFrame;
		int framePerSample;
		BOOL exportGeom;
		BOOL exportNormals;
		BOOL exportVertexColor;
		BOOL exportControllers;
		BOOL exportFaceSmgp;
		BOOL exportTexCoords;
		BOOL exportMappingChannel;
		BOOL exportConstraints;
		BOOL exportMaterials;
		BOOL exportSplines;
		BOOL exportModifiers;
		BOOL exportSkin;
		BOOL exportGenMod;
		BOOL forceSample;
		BOOL splitFile;
		BOOL exportQuaternions;
		BOOL exportObjectSpace;
		int cS;
		int exportCoord;
		bool showPrompts;
		bool exportSelected;

		TSTR splitPath;



		int				ExtCount();					// Number of extensions supported
		const TCHAR *	Ext(int n);					// Extension #n (i.e. "3DS")
		const TCHAR *	LongDesc();					// Long ASCII description (i.e. "Autodesk 3D Studio File")
		const TCHAR *	ShortDesc();				// Short ASCII description (i.e. "3D Studio")
		const TCHAR *	AuthorName();				// ASCII Author name
		const TCHAR *	CopyrightMessage();			// ASCII Copyright message
		const TCHAR *	OtherMessage1();			// Other message #1
		const TCHAR *	OtherMessage2();			// Other message #2
		unsigned int	Version();					// Version number * 100 (i.e. v3.01 = 301)
		void			ShowAbout(HWND hWnd);		// Show DLL's "About..." box

		BOOL SupportsOptions(int ext, DWORD options);
		int	DoExport(const TCHAR *name,ExpInterface *ei,Interface *i, BOOL suppressPrompts=FALSE, DWORD options=0);

		
		void ExportSceneInfo();
		void ExportNodeInfo(IGameNode * node);
		void ExportChildNodeInfo(CComPtr<IXMLDOMNode> parent, IGameNode * child);
		void ExportMaterials();
		void ExportPositionControllers(CComPtr<IXMLDOMNode> node, IGameControl * cont);
		void ExportRotationControllers(CComPtr<IXMLDOMNode> node, IGameControl * cont);
		void ExportScaleControllers(CComPtr<IXMLDOMNode> node, IGameControl * cont);
		void ExportControllers(CComPtr<IXMLDOMNode> node, IGameControl * cont);

		void DumpMaterial(CComPtr<IXMLDOMNode> node,IGameMaterial * mat, int index, int matID = -1);
		void DumpTexture(CComPtr<IXMLDOMNode> node,IGameMaterial * mat);
		void DumpBezierKeys(DWORD Type, IGameKeyTab Keys, CComPtr<IXMLDOMNode> prsData);
		void DumpTCBKeys(DWORD Type, IGameKeyTab Keys, CComPtr<IXMLDOMNode> prsData);
		void DumpLinearKeys(DWORD Type, IGameKeyTab Keys, CComPtr<IXMLDOMNode> prsData);
		void DumpConstraints(CComPtr<IXMLDOMNode> prsData, IGameConstraint * c);
		void DumpModifiers(CComPtr<IXMLDOMNode> prsData, IGameModifier * m);
		void DumpSkin(CComPtr<IXMLDOMNode> modNode, IGameSkin * s);
		void DumpIKChain(IGameIKChain * ikch, CComPtr<IXMLDOMNode> ikData);

		void DumpEulerController(IGameControl * sc,CComPtr<IXMLDOMNode> prsNode);
		void DumpProperties(CComPtr<IXMLDOMNode> node, IGameProperty * prop);
		void DumpMesh(IGameMesh *gm,CComPtr<IXMLDOMNode> geomData);
		void DumpSpline(IGameSpline *sp,CComPtr<IXMLDOMNode> splineData);
		void DumpLight(IGameLight *lt, CComPtr<IXMLDOMNode> parent);
		void DumpCamera(IGameCamera *ca, CComPtr<IXMLDOMNode> parent);
		void DumpGenericFace(FaceEx *fe, CComPtr<IXMLDOMNode> faceData, bool smgrp, bool n, bool vc, bool tv);
		void DumpSampleKeys(IGameControl * sc,CComPtr<IXMLDOMNode> prsNode, DWORD Type, bool quick = false);
		void DumpListController(IGameControl * sc,CComPtr<IXMLDOMNode> listNode);
		void DumpMatrix(Matrix3 tm,CComPtr<IXMLDOMNode> parent);


		void MakeSplitFilename(IGameNode * node, TSTR & buf);
		BOOL ReadConfig();
		void WriteConfig();
		TSTR GetCfgFilename();
		IGameExporter();
		~IGameExporter();		

};


class IGameExporterClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return TRUE; }
	void *			Create(BOOL loading = FALSE) { return new IGameExporter(); }
	const TCHAR *	ClassName() { return GetString(IDS_CLASS_NAME); }
	SClass_ID		SuperClassID() { return SCENE_EXPORT_CLASS_ID; }
	Class_ID		ClassID() { return IGAMEEXPORTER_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_CATEGORY); }

	const TCHAR*	InternalName() { return _T("IGameExporter"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle

};



static IGameExporterClassDesc IGameExporterDesc;
ClassDesc2* GetIGameExporterDesc() { return &IGameExporterDesc; }

int numVertex;

BOOL CALLBACK IGameExporterOptionsDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam) {
	IGameExporter *exp = (IGameExporter*)GetWindowLongPtr(hWnd,GWLP_USERDATA); 
	ISpinnerControl * spin;
	int ID;
	
	switch(message) {
		case WM_INITDIALOG:
			exp = (IGameExporter *)lParam;
			SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam); 
			CenterWindow(hWnd,GetParent(hWnd));
			spin = GetISpinner(GetDlgItem(hWnd, IDC_STATIC_FRAME_SPIN)); 
			spin->LinkToEdit(GetDlgItem(hWnd,IDC_STATIC_FRAME), EDITTYPE_INT ); 
			spin->SetLimits(0, 100, TRUE); 
			spin->SetScale(1.0f);
			spin->SetValue(exp->staticFrame ,FALSE);
			ReleaseISpinner(spin);
						
			spin = GetISpinner(GetDlgItem(hWnd, IDC_SAMPLE_FRAME_SPIN)); 
			spin->LinkToEdit(GetDlgItem(hWnd,IDC_SAMPLE_FRAME), EDITTYPE_INT ); 
			spin->SetLimits(1, 100, TRUE); 
			spin->SetScale(1.0f);
			spin->SetValue(exp->framePerSample ,FALSE);
			ReleaseISpinner(spin);
			CheckDlgButton(hWnd,IDC_EXP_GEOMETRY,exp->exportGeom);
			CheckDlgButton(hWnd,IDC_EXP_NORMALS,exp->exportNormals);
			CheckDlgButton(hWnd,IDC_EXP_CONTROLLERS,exp->exportControllers);
			CheckDlgButton(hWnd,IDC_EXP_FACESMGRP,exp->exportFaceSmgp);
			CheckDlgButton(hWnd,IDC_EXP_VCOLORS,exp->exportVertexColor);
			CheckDlgButton(hWnd,IDC_EXP_TEXCOORD,exp->exportTexCoords);
			CheckDlgButton(hWnd,IDC_EXP_MAPCHAN,exp->exportMappingChannel);
			CheckDlgButton(hWnd,IDC_EXP_MATERIAL,exp->exportMaterials);
			CheckDlgButton(hWnd,IDC_EXP_SPLINES,exp->exportSplines);
			CheckDlgButton(hWnd,IDC_EXP_MODIFIERS,exp->exportModifiers);
			CheckDlgButton(hWnd,IDC_EXP_SAMPLECONT,exp->forceSample);
			CheckDlgButton(hWnd,IDC_EXP_CONSTRAINTS,exp->exportConstraints);
			CheckDlgButton(hWnd,IDC_EXP_SKIN,exp->exportSkin);
			CheckDlgButton(hWnd,IDC_EXP_GENMOD,exp->exportGenMod);
			CheckDlgButton(hWnd,IDC_SPLITFILE,exp->splitFile);
			CheckDlgButton(hWnd,IDC_EXP_OBJECTSPACE,exp->exportObjectSpace);
			CheckDlgButton(hWnd,IDC_EXP_QUATERNIONS,exp->exportQuaternions);
			

			ID = IDC_COORD_MAX + exp->cS;
			CheckRadioButton(hWnd,IDC_COORD_MAX,IDC_COORD_OGL,ID);
/*			if(exp->cS == IGameConversionManager::IGAME_D3D)
				CheckDlgButton(hWnd,IDC_COORD_D3D,TRUE);
			else if(exp->cS == IGameConversionManager::IGAME_OGL)
				CheckDlgButton(hWnd,IDC_COORD_OGL,TRUE);
			else
				CheckDlgButton(hWnd,IDC_COORD_MAX,TRUE);
*/

			EnableWindow(GetDlgItem(hWnd, IDC_EXP_NORMALS), exp->exportGeom);
			EnableWindow(GetDlgItem(hWnd, IDC_EXP_FACESMGRP), exp->exportGeom);
			EnableWindow(GetDlgItem(hWnd, IDC_EXP_VCOLORS),  exp->exportGeom);
			EnableWindow(GetDlgItem(hWnd, IDC_EXP_TEXCOORD),  exp->exportGeom);
			EnableWindow(GetDlgItem(hWnd, IDC_EXP_MAPCHAN),  exp->exportGeom);
			
			EnableWindow(GetDlgItem(hWnd, IDC_EXP_CONSTRAINTS),  exp->exportControllers);
			EnableWindow(GetDlgItem(hWnd, IDC_EXP_SAMPLECONT),  exp->exportControllers);
	
			EnableWindow(GetDlgItem(hWnd, IDC_EXP_SKIN),  exp->exportModifiers);
			EnableWindow(GetDlgItem(hWnd, IDC_EXP_GENMOD),  exp->exportModifiers);


			return TRUE;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {

				case IDC_EXP_GEOMETRY:
					EnableWindow(GetDlgItem(hWnd, IDC_EXP_NORMALS), IsDlgButtonChecked(hWnd, IDC_EXP_GEOMETRY));
					EnableWindow(GetDlgItem(hWnd, IDC_EXP_FACESMGRP), IsDlgButtonChecked(hWnd, IDC_EXP_GEOMETRY));
					EnableWindow(GetDlgItem(hWnd, IDC_EXP_VCOLORS),  IsDlgButtonChecked(hWnd, IDC_EXP_GEOMETRY));
					EnableWindow(GetDlgItem(hWnd, IDC_EXP_TEXCOORD),  IsDlgButtonChecked(hWnd, IDC_EXP_GEOMETRY));
					EnableWindow(GetDlgItem(hWnd, IDC_EXP_MAPCHAN),  IsDlgButtonChecked(hWnd, IDC_EXP_GEOMETRY));
					break;
				case IDC_EXP_CONTROLLERS:
					EnableWindow(GetDlgItem(hWnd, IDC_EXP_CONSTRAINTS), IsDlgButtonChecked(hWnd, IDC_EXP_CONTROLLERS));
					EnableWindow(GetDlgItem(hWnd, IDC_EXP_SAMPLECONT),  IsDlgButtonChecked(hWnd, IDC_EXP_CONTROLLERS));
					break;
				case IDC_EXP_MODIFIERS:
					EnableWindow(GetDlgItem(hWnd, IDC_EXP_SKIN), IsDlgButtonChecked(hWnd, IDC_EXP_MODIFIERS));
					EnableWindow(GetDlgItem(hWnd, IDC_EXP_GENMOD),  IsDlgButtonChecked(hWnd, IDC_EXP_MODIFIERS));

					break;

				case IDOK:
					exp->exportGeom = IsDlgButtonChecked(hWnd, IDC_EXP_GEOMETRY);
					exp->exportNormals = IsDlgButtonChecked(hWnd, IDC_EXP_NORMALS);
					exp->exportControllers = IsDlgButtonChecked(hWnd, IDC_EXP_CONTROLLERS);
					exp->exportFaceSmgp = IsDlgButtonChecked(hWnd, IDC_EXP_FACESMGRP);
					exp->exportVertexColor = IsDlgButtonChecked(hWnd, IDC_EXP_VCOLORS);
					exp->exportTexCoords = IsDlgButtonChecked(hWnd, IDC_EXP_TEXCOORD);
					exp->exportMappingChannel = IsDlgButtonChecked(hWnd, IDC_EXP_MAPCHAN);
					exp->exportMaterials = IsDlgButtonChecked(hWnd, IDC_EXP_MATERIAL);
					exp->exportSplines = IsDlgButtonChecked(hWnd, IDC_EXP_SPLINES);
					exp->exportModifiers = IsDlgButtonChecked(hWnd, IDC_EXP_MODIFIERS);
					exp->forceSample = IsDlgButtonChecked(hWnd, IDC_EXP_SAMPLECONT);
					exp->exportConstraints = IsDlgButtonChecked(hWnd, IDC_EXP_CONSTRAINTS);
					exp->exportSkin = IsDlgButtonChecked(hWnd, IDC_EXP_SKIN);
					exp->exportGenMod = IsDlgButtonChecked(hWnd, IDC_EXP_GENMOD);
					exp->splitFile = IsDlgButtonChecked(hWnd,IDC_SPLITFILE);
					exp->exportQuaternions = IsDlgButtonChecked(hWnd,IDC_EXP_QUATERNIONS);
					exp->exportObjectSpace = IsDlgButtonChecked(hWnd,IDC_EXP_OBJECTSPACE);
					if (IsDlgButtonChecked(hWnd, IDC_COORD_MAX))
						exp->cS = IGameConversionManager::IGAME_MAX;
					else if (IsDlgButtonChecked(hWnd, IDC_COORD_OGL))
						exp->cS = IGameConversionManager::IGAME_OGL;
					else
						exp->cS = IGameConversionManager::IGAME_D3D;

					spin = GetISpinner(GetDlgItem(hWnd, IDC_STATIC_FRAME_SPIN)); 
					exp->staticFrame = spin->GetIVal(); 
					ReleaseISpinner(spin);
					spin = GetISpinner(GetDlgItem(hWnd, IDC_SAMPLE_FRAME_SPIN)); 
					exp->framePerSample = spin->GetIVal(); 
					ReleaseISpinner(spin);
					EndDialog(hWnd, 1);
					break;
				case IDCANCEL:
					EndDialog(hWnd,0);
					break;
			}
		
		default:
			return FALSE;
	
	}
	return TRUE;
	
}

void stripWhiteSpace(TSTR * buf, TCHAR &newBuf)
{

	TCHAR newb[256]={""};
	strcpy(newb,buf->data());

	int len = strlen(newb);

	int index = 0;

	for(int i=0;i<len;i++)
	{
		if((newb[i] != ' ') && (!ispunct(newb[i])))
			(&newBuf)[index++] = newb[i];
	}
}

//--- IGameExporter -------------------------------------------------------
IGameExporter::IGameExporter()
{

	staticFrame = 0;
	framePerSample = 4;
	exportGeom = TRUE;
	exportNormals = TRUE;
	exportVertexColor = FALSE;
	exportControllers = FALSE;
	exportFaceSmgp = FALSE;
	exportTexCoords = TRUE;
	exportMappingChannel = FALSE;
	exportMaterials = TRUE;
	exportConstraints = FALSE;
	exportSplines = FALSE;
	exportModifiers = FALSE;
	forceSample = FALSE;
	exportSkin = TRUE;
	exportGenMod = FALSE;
	cS = 0;	//max default
	pRoot = NULL;
	pXMLDoc = NULL;
	splitFile = TRUE;
	exportQuaternions = TRUE;
	exportObjectSpace = FALSE;
	
//	int a = IGameConversionManager::IGAME_MAX;
//	int b = IGameConversionManager::IGAME_OGL;
//	int c = IGameConversionManager::IGAME_D3D;

}

IGameExporter::~IGameExporter() 
{
	iGameNode = NULL;
	if(pRoot)
		pRoot->Release(); 
	pRoot = NULL;
	if(pXMLDoc)
		pXMLDoc->Release();
	
	pXMLDoc = NULL;
}

int IGameExporter::ExtCount()
{
	//TODO: Returns the number of file name extensions supported by the plug-in.
	return 1;
}

const TCHAR *IGameExporter::Ext(int n)
{		
	//TODO: Return the 'i-th' file name extension (i.e. "3DS").
	return _T("xml");
}

const TCHAR *IGameExporter::LongDesc()
{
	//TODO: Return long ASCII description (i.e. "Targa 2.0 Image File")
	return _T("XML Export for IGame");
}
	
const TCHAR *IGameExporter::ShortDesc() 
{			
	//TODO: Return short ASCII description (i.e. "Targa")
	return _T("IGame Exporter");
}

const TCHAR *IGameExporter::AuthorName()
{			
	//TODO: Return ASCII Author name
	return _T("Neil Hazzard");
}

const TCHAR *IGameExporter::CopyrightMessage() 
{	
	// Return ASCII Copyright message
	return _T("");
}

const TCHAR *IGameExporter::OtherMessage1() 
{		
	//TODO: Return Other message #1 if any
	return _T("");
}

const TCHAR *IGameExporter::OtherMessage2() 
{		
	//TODO: Return other message #2 in any
	return _T("");
}

unsigned int IGameExporter::Version()
{				
	//TODO: Return Version number * 100 (i.e. v3.01 = 301)
	return 100;
}

void IGameExporter::ShowAbout(HWND hWnd)
{			
	// Optional
}

BOOL IGameExporter::SupportsOptions(int ext, DWORD options)
{
	// TODO Decide which options to support.  Simply return
	// true for each option supported by each Extension 
	// the exporter supports.

	return TRUE;
}


void IGameExporter::MakeSplitFilename(IGameNode * node, TSTR & buf)
{
	buf = splitPath;
	buf = buf + "\\" + node->GetName() + ".xml";
	
}

void IGameExporter::ExportSceneInfo()
{
	TSTR buf;
	CComPtr <IXMLDOMNode> sceneNode;
	CComPtr <IXMLDOMNode> tempNode;

	struct tm *newtime;
	time_t aclock;

	time( &aclock );
	newtime = localtime(&aclock);

	TSTR today = _tasctime(newtime);	// The date string has a \n appended.
	today.remove(today.length()-1);		// Remove the \n

	CreateXMLNode(pXMLDoc, pRoot, _T("IGame"), &iGameNode);
	AddXMLAttribute(iGameNode, _T("Version"), _T("1.0"));
	AddXMLAttribute(iGameNode, _T("Date"), today.data());
	
	CreateXMLNode(pXMLDoc,iGameNode,_T("SceneInfo"),&sceneNode);

	CreateXMLNode(pXMLDoc,sceneNode,_T("Info"),&tempNode);
	AddXMLAttribute(tempNode,_T("FileName"),const_cast<TCHAR*>(pIgame->GetSceneFileName()));
	tempNode = NULL;

	CreateXMLNode(pXMLDoc,sceneNode,_T("Info"),&tempNode);
	buf.printf("%d",pIgame->GetSceneStartTime() / pIgame->GetSceneTicks());
	AddXMLAttribute(tempNode,_T("StartFrame"),buf.data());
	tempNode = NULL;
	
	CreateXMLNode(pXMLDoc,sceneNode,_T("Info"),&tempNode);
	buf.printf("%d",pIgame->GetSceneEndTime()/pIgame->GetSceneTicks());
	AddXMLAttribute(tempNode,_T("EndFrame"),buf.data());
	tempNode = NULL;
	
	CreateXMLNode(pXMLDoc,sceneNode,_T("Info"),&tempNode);
	buf.printf("%d",GetFrameRate());
	AddXMLAttribute(tempNode,_T("FrameRate"),buf.data());
	tempNode = NULL;
	
	CreateXMLNode(pXMLDoc,sceneNode,_T("Info"),&tempNode);
	buf.printf("%d",pIgame->GetSceneTicks());
	AddXMLAttribute(tempNode,_T("TicksPerFrame"),buf.data());
	tempNode = NULL;

	CreateXMLNode(pXMLDoc,sceneNode,_T("Info"),&tempNode);
	if(cS == 0)
		buf = TSTR("3dsmax");
	if(cS == 1)
		buf = TSTR("directx");
	if(cS == 2)
		buf = TSTR("opengl");

	AddXMLAttribute(tempNode,_T("CoordinateSystem"),buf.data());

	sceneNode = NULL;
	tempNode = NULL;


}

void IGameExporter::ExportChildNodeInfo(CComPtr<IXMLDOMNode> parent, IGameNode * child)
{
	IGameKeyTab poskeys;
	IGameKeyTab rotkeys;
	IGameKeyTab scalekeys;
	TSTR buf,data;

	CComPtr <IXMLDOMNode> prsNode,group;
	CComPtr <IXMLDOMNode> matIndex;
	CComPtr <IXMLDOMNode> geomData,splineData,ikData;
	CComPtr <IXMLDOMNode> tmNodeParent;

	IXMLDOMDocument * pSubDocMesh, * pSubDocCont;
	CComPtr <IXMLDOMNode> pRootSubCont,pRootSubMesh,pSubCont,pSubMesh;

	struct tm *newtime;
	time_t aclock;

	time( &aclock );
	newtime = localtime(&aclock);

	TSTR today = _tasctime(newtime);	// The date string has a \n appended.
	today.remove(today.length()-1);		// Remove the \n



	if(splitFile)
	{
		CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER,  IID_IXMLDOMDocument, (void**)&pSubDocCont);
		CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER,  IID_IXMLDOMDocument, (void**)&pSubDocMesh);
		pSubDocCont->QueryInterface(IID_IXMLDOMNode, (void **)&pRootSubCont);
		pSubDocMesh->QueryInterface(IID_IXMLDOMNode, (void **)&pRootSubMesh);
		CreateXMLNode(pXMLDoc, pRootSubCont, _T("IGame"), &pSubCont);
		AddXMLAttribute(pSubCont, _T("Version"), _T("1.0"));
		AddXMLAttribute(pSubCont, _T("Date"), today.data());

		CreateXMLNode(pXMLDoc, pRootSubMesh, _T("IGame"), &pSubMesh);
		AddXMLAttribute(pSubMesh, _T("Version"), _T("1.0"));
		AddXMLAttribute(pSubMesh, _T("Date"), today.data());

	}

	curNode ++;
	buf = TSTR("Processing: ") + TSTR(child->GetName());
	GetCOREInterface()->ProgressUpdate((int)((float)curNode/pIgame->GetTotalNodeCount()*100.0f),FALSE,buf.data()); 


	if(child->IsGroupOwner())
	{
		TSTR b(child->GetName());
		TCHAR nb[256]={""};
		stripWhiteSpace(&b,*nb);
		AddXMLAttribute(parent,_T("GroupName"),nb);
		buf.printf("%d",child->GetNodeID());
		AddXMLAttribute(parent,_T("NodeID"),buf.data());
		buf.printf("%d",child->GetChildCount());
		AddXMLAttribute(parent,_T("NumberInGroup"),buf.data());
	}
	else
	{
		TSTR b(child->GetName());
		TCHAR nb[256]={""};
		stripWhiteSpace(&b,*nb);
		AddXMLAttribute(parent,_T("Name"),nb);
		buf.printf("%d",child->GetNodeID());
		AddXMLAttribute(parent,_T("NodeID"),buf.data());

		IGameNode * p = child->GetNodeParent();
		if(p)
		{
			buf.printf("%d",p->GetNodeID());
			AddXMLAttribute(parent,_T("ParentNodeID"),buf.data());
		}

		CreateXMLNode(pXMLDoc,parent,_T("NodeTM"),&tmNodeParent);

		DumpMatrix(child->GetWorldTM(staticFrame).ExtractMatrix3(),tmNodeParent);

		ULONG  handle = child->GetMaxNode()->GetHandle();

		if(child->GetMaterialIndex() !=-1){
			CreateXMLNode(pXMLDoc,parent,_T("MaterialIndex"),&matIndex);
			buf.printf("%d",child->GetMaterialIndex());
			AddXMLText(pXMLDoc,matIndex,buf.data());
		}
		IGameObject * obj = child->GetIGameObject();

		IGameObject::MaxType T = obj->GetMaxType();

		bool xref = obj->IsObjectXRef();

		if(xref)
		{
			AddXMLAttribute(parent,_T("XRefObject"),_T("True"));
		}

		switch(obj->GetIGameType())
		{
			case IGameObject::IGAME_BONE:
			{
	//			CComPtr <IXMLDOMNode> boneNode;
	//			CreateXMLNode(pXMLDoc,parent,_T("BoneData"),&boneNode);
				AddXMLAttribute(parent,_T("NodeType"),_T("Bone"));
				IGameSupportObject * hO = (IGameSupportObject*)obj;
				IGameMesh * hm = hO->GetMeshObject();
				if(hm->InitializeData())
				{
					CreateXMLNode(pXMLDoc,parent,_T("GeomData"),&geomData);
					if(splitFile)
					{
						TSTR filename;
						MakeSplitFilename(child,filename);
						CComPtr<IXMLDOMNode>subMeshNode;						
						AddXMLAttribute(geomData, _T("Include"),filename.data());
						CreateXMLNode(pSubDocMesh,pSubMesh,_T("GeomData"),&subMeshNode);
						AddXMLAttribute(subMeshNode,_T("Node"),child->GetName());
						geomData = subMeshNode;

					}
					DumpMesh(hm,geomData);
				}

				break;
			}

			case IGameObject::IGAME_HELPER:
			{
	//			CComPtr <IXMLDOMNode> boneNode;
	//			CreateXMLNode(pXMLDoc,parent,_T("BoneData"),&boneNode);
				AddXMLAttribute(parent,_T("NodeType"),_T("Helper"));
				IGameSupportObject * hO = (IGameSupportObject*)obj;
				IPropertyContainer * cc = hO->GetIPropertyContainer();
				IGameProperty * prop = cc->QueryProperty(101);
				if(prop)
				{
					TCHAR * buf;
					prop->GetPropertyValue(buf);
				}
				prop = cc->QueryProperty(_T("IGameTestString"));

				if(prop)
				{
					TCHAR * buf;
					prop->GetPropertyValue(buf);
				}
				prop = cc->QueryProperty(_T("IGameTestString"));

				IGameMesh * hm = hO->GetMeshObject();
				if(hm->InitializeData())
				{
					CreateXMLNode(pXMLDoc,parent,_T("GeomData"),&geomData);
					if(splitFile)
					{
						TSTR filename;
						MakeSplitFilename(child,filename);
						CComPtr<IXMLDOMNode>subMeshNode;						
						AddXMLAttribute(geomData, _T("Include"),filename.data());
						CreateXMLNode(pSubDocMesh,pSubMesh,_T("GeomData"),&subMeshNode);
						AddXMLAttribute(subMeshNode,_T("Node"),child->GetName());
						geomData = subMeshNode;

					}
					DumpMesh(hm,geomData);
				}


				break;
			}
			case IGameObject::IGAME_LIGHT:
			{
				CComPtr <IXMLDOMNode> lightNode;
				CreateXMLNode(pXMLDoc,parent,_T("lightData"),&lightNode);

				AddXMLAttribute(parent,_T("NodeType"),_T("Light"));
				IGameLight * l = (IGameLight*)obj;
				if(l->GetLightType()==IGameLight::IGAME_OMNI)
					AddXMLAttribute(lightNode,_T("Type"),_T("Omni"));
				else if(l->GetLightType()==IGameLight::IGAME_TSPOT)
					AddXMLAttribute(lightNode,_T("Type"),_T("Targeted"));
				else if(l->GetLightType()==IGameLight::IGAME_FSPOT)
					AddXMLAttribute(lightNode,_T("Type"),_T("Free"));
				else if(l->GetLightType()==IGameLight::IGAME_TDIR)
					AddXMLAttribute(lightNode,_T("Type"),_T("TargetedDirectional"));
				else
					AddXMLAttribute(lightNode,_T("Type"),_T("Directional"));
				DumpLight(l,lightNode);
				break;
			}
			case IGameObject::IGAME_CAMERA:
			{
				CComPtr <IXMLDOMNode> camNode;
				CreateXMLNode(pXMLDoc,parent,_T("CameraData"),&camNode);
				AddXMLAttribute(parent,_T("NodeType"),_T("Camera"));
				IGameCamera * c = (IGameCamera*)obj;
				if(c->GetCameraTarget())
					AddXMLAttribute(camNode,_T("Type"),_T("Targeted"));
				else
					AddXMLAttribute(camNode,_T("Type"),_T("Free"));
				DumpCamera(c,camNode);
				break;
			}

			case IGameObject::IGAME_MESH:
				if(exportGeom )
				{
					AddXMLAttribute(parent,_T("NodeType"),_T("GeomObject"));
					IGameMesh * gM = (IGameMesh*)obj;
					CComPtr<IXMLDOMNode>subMeshNode;
					if(gM->InitializeData())
					{
						CreateXMLNode(pXMLDoc,parent,_T("GeomData"),&geomData);
						if(splitFile)
						{
							TSTR filename;
							MakeSplitFilename(child,filename);
							
							AddXMLAttribute(geomData, _T("Include"),filename.data());
							CreateXMLNode(pSubDocMesh,pSubMesh,_T("GeomData"),&subMeshNode);
							AddXMLAttribute(subMeshNode,_T("Node"),child->GetName());
							geomData = subMeshNode;

						}
						DumpMesh(gM,geomData);
					}
					else
					{
						CreateXMLNode(pXMLDoc,parent,_T("GeomData"),&geomData);
						AddXMLAttribute(geomData,_T("Error"),_T("BadObject"));

					}
						
				
				}
				break;
			case IGameObject::IGAME_SPLINE:
				if(exportSplines)
				{
					AddXMLAttribute(parent,_T("NodeType"),_T("SplineObject"));
					IGameSpline * sp = (IGameSpline*)obj;
					sp->InitializeData();
					CreateXMLNode(pXMLDoc,parent,_T("SplineData"),&splineData);
					DumpSpline(sp,splineData);
				}
				break;

			case IGameObject::IGAME_IKCHAIN:
				AddXMLAttribute(parent,_T("NodeType"),_T("IKChainObject"));
				IGameIKChain * ikch = (IGameIKChain*)obj;
				CreateXMLNode(pXMLDoc,parent,_T("IKChain"), & ikData);
				DumpIKChain(ikch,ikData);
				break;
				
		}

		if(splitFile)
		{
			TSTR buf;
			MakeSplitFilename(child,buf);
			pSubDocMesh->save(CComVariant(buf.data()));

		}
		//dump PRS Controller data
		prsNode = NULL;

		// In our "Game Engine" we deal with Bezier Position, Scale and TCB Rotation controllers !!
		// Only list controllers on position, rotation...
		if(exportControllers)
		{
			CComPtr<IXMLDOMNode>subContNode;
			bool exportBiped = false;
			CreateXMLNode(pXMLDoc,parent,_T("PRSData"),&prsNode);
			IGameControl * pGameControl = child->GetIGameControl();
			ExportControllers(prsNode,pGameControl);

/*			IGameControl::MaxControlType T = pGameControl->GetControlType(IGAME_POS);

			//position
			if(T==IGameControl::IGAME_MAXSTD && pGameControl->GetBezierKeys(poskeys,IGAME_POS) && !forceSample)
				DumpBezierKeys(IGAME_POS,poskeys,prsNode);
			else if(T==IGameControl::IGAME_POS_CONSTRAINT && !forceSample)
			{
				IGameConstraint * cnst = pGameControl->GetConstraint(IGAME_POS);
				DumpConstraints(prsNode,cnst);
			}
			else if(T==IGameControl::IGAME_LIST && !forceSample)
			{
				DumpListController(pGameControl,prsNode);

			}
			else if(T==IGameControl::IGAME_BIPED)
				exportBiped = true;
			else
			{
				if(forceSample || T==IGameControl::IGAME_UNKNOWN)
					DumpSampleKeys(child->GetIGameControl(),prsNode,IGAME_POS);
			}
				
		
			//rotation
			T = pGameControl->GetControlType(IGAME_ROT);

			if(T==IGameControl::IGAME_MAXSTD && pGameControl->GetTCBKeys(rotkeys,IGAME_ROT) && !forceSample)
				DumpTCBKeys(IGAME_ROT,rotkeys,prsNode);

			else if(T==IGameControl::IGAME_ROT_CONSTRAINT && !forceSample)
			{
				IGameConstraint * cnst = pGameControl->GetConstraint(IGAME_ROT);
				DumpConstraints(prsNode,cnst);
			}
			else if(T==IGameControl::IGAME_LIST&& !forceSample)
			{
				DumpListController(pGameControl,prsNode);

			}
			else if(T==IGameControl::IGAME_EULER&& !forceSample)
			{
				DumpEulerController(pGameControl,prsNode);
			}

			else if(T==IGameControl::IGAME_BIPED)
				exportBiped = true;

			else
			{
				if(forceSample || T==IGameControl::IGAME_UNKNOWN)
					DumpSampleKeys(child->GetIGameControl(),prsNode, IGAME_ROT);
			}

			//scale
			T = pGameControl->GetControlType(IGAME_SCALE);

			if(T==IGameControl::IGAME_MAXSTD && pGameControl->GetBezierKeys(scalekeys,IGAME_SCALE) && !forceSample)
				DumpBezierKeys(IGAME_SCALE,scalekeys,prsNode);
			else if(T==IGameControl::IGAME_BIPED)
				exportBiped = true;
			else
			{
				if(forceSample || T==IGameControl::IGAME_UNKNOWN)
					DumpSampleKeys(child->GetIGameControl(),prsNode, IGAME_SCALE);
			}

			if(exportBiped)
				DumpSampleKeys(child->GetIGameControl(),prsNode,IGAME_TM);

  */
		}

		if(exportModifiers)
		{
			int numMod = obj->GetNumModifiers();
			if(numMod > 0)
			{
				CComPtr <IXMLDOMNode> mod;
				CreateXMLNode(pXMLDoc,parent,_T("Modifiers"),&mod);
				TSTR Buf;
				buf.printf("%d",numMod);
				AddXMLAttribute(mod,_T("count"),buf.data());

				for(int i=0;i<numMod;i++)
				{
					IGameModifier * m = obj->GetIGameModifier(i);
					DumpModifiers(mod,m);
				}
			}
		}
	}	
	for(int i=0;i<child->GetChildCount();i++)
	{
		CComPtr <IXMLDOMNode> cNode = NULL;
		IGameNode * newchild = child->GetNodeChild(i);
		TSTR name;
		if(newchild->IsGroupOwner())
			name = TSTR("Group");
		else
			name = TSTR("Node");

//		stripWhiteSpace(&name,*buf);
		CreateXMLNode(pXMLDoc,parent,name.data(),&cNode);

		// we deal with targets in the light/camera section
		if(newchild->IsTarget())
			continue;

		ExportChildNodeInfo(cNode,newchild);
	}

	child->ReleaseIGameObject();

	prsNode = NULL;
	group = NULL;

}


void IGameExporter::ExportControllers(CComPtr<IXMLDOMNode> node, IGameControl * pGameControl)
{
	ExportPositionControllers(node,pGameControl);
	ExportRotationControllers(node,pGameControl);
	ExportScaleControllers(node,pGameControl);
}

void IGameExporter::ExportPositionControllers(CComPtr<IXMLDOMNode> node, IGameControl * pGameControl)
{
	IGameKeyTab poskeys;
	bool exportBiped = false;


	IGameControl::MaxControlType T = pGameControl->GetControlType(IGAME_POS);

	//position
	if(T==IGameControl::IGAME_MAXSTD && pGameControl->GetBezierKeys(poskeys,IGAME_POS) && !forceSample)
		DumpBezierKeys(IGAME_POS,poskeys,node);
	else if(T==IGameControl::IGAME_POS_CONSTRAINT && !forceSample)
	{
		IGameConstraint * cnst = pGameControl->GetConstraint(IGAME_POS);
		DumpConstraints(node,cnst);
	}
	else if(T==IGameControl::IGAME_LIST && !forceSample)
	{
		DumpListController(pGameControl,node);

	}
	else if(T==IGameControl::IGAME_BIPED)
		exportBiped = true;
	else
	{
		if(forceSample || T==IGameControl::IGAME_UNKNOWN)
			DumpSampleKeys(pGameControl,node,IGAME_POS);
	}
	if(exportBiped)
		DumpSampleKeys(pGameControl,node,IGAME_TM);
		
}

void IGameExporter::ExportRotationControllers(CComPtr<IXMLDOMNode> node, IGameControl * pGameControl)
{

	IGameKeyTab rotkeys;
	bool exportBiped = false;
	bool exported = false;		//this will at least export something !!

	//rotation
	IGameControl::MaxControlType T = pGameControl->GetControlType(IGAME_ROT);

	if(T==IGameControl::IGAME_MAXSTD && pGameControl->GetTCBKeys(rotkeys,IGAME_ROT) && !forceSample)
	{
		DumpTCBKeys(IGAME_ROT,rotkeys,node);
		exported = true;
	}

	else if(T==IGameControl::IGAME_ROT_CONSTRAINT && !forceSample)
	{
		IGameConstraint * cnst = pGameControl->GetConstraint(IGAME_ROT);
		DumpConstraints(node,cnst);
		exported = true;
	}
	else if(T==IGameControl::IGAME_LIST&& !forceSample)
	{
		DumpListController(pGameControl,node);
		exported = true;

	}
	else if(T==IGameControl::IGAME_EULER&& !forceSample)
	{
		DumpEulerController(pGameControl,node);
		exported = true;
	}

	else if(T==IGameControl::IGAME_BIPED)
	{
		exportBiped = true;
		exported = true;
	}

	else
	{
		if(forceSample || !exported || T==IGameControl::IGAME_UNKNOWN)
			DumpSampleKeys(pGameControl,node, IGAME_ROT);
	}
	if(exportBiped)
		DumpSampleKeys(pGameControl,node,IGAME_TM);
}

void IGameExporter::ExportScaleControllers(CComPtr<IXMLDOMNode> node, IGameControl * pGameControl)
{
	IGameKeyTab scalekeys;
	bool exportBiped = false;


	//scale
	IGameControl::MaxControlType T= pGameControl->GetControlType(IGAME_SCALE);

	if(T==IGameControl::IGAME_MAXSTD && pGameControl->GetBezierKeys(scalekeys,IGAME_SCALE) && !forceSample)
		DumpBezierKeys(IGAME_SCALE,scalekeys,node);
	else if(T==IGameControl::IGAME_BIPED)
		exportBiped = true;
	else
	{
		if(forceSample || T==IGameControl::IGAME_UNKNOWN)
			DumpSampleKeys(pGameControl,node, IGAME_SCALE);
	}

	if(exportBiped)
		DumpSampleKeys(pGameControl,node,IGAME_TM);


	
}

void IGameExporter::ExportNodeInfo(IGameNode * node)
{
		
	rootNode = NULL;
	TCHAR buf[256]={""};
	TSTR name;
	if(node->IsGroupOwner())
		name = TSTR("Group");
	else
		name = TSTR("Node");
	
//	stripWhiteSpace(&name,*buf);
	CreateXMLNode(pXMLDoc,iGameNode,name.data(),&rootNode);
	ExportChildNodeInfo(rootNode,node);

}

void IGameExporter::DumpMatrix(Matrix3 tm,CComPtr<IXMLDOMNode> parent)
{
	CComPtr <IXMLDOMNode> tmNode;
	AffineParts ap;
	float rotAngle;
	Point3 rotAxis;
	float scaleAxAngle;
	Point3 scaleAxis;
	Matrix3 m = tm;
	TSTR Buf;
	
	decomp_affine(m, &ap);

	// Quaternions are dumped as angle axis.
	AngAxisFromQ(ap.q, &rotAngle, rotAxis);
	AngAxisFromQ(ap.u, &scaleAxAngle, scaleAxis);

//	CreateXMLNode(pXMLDoc,parent,_T("NodeTM"),&tmNodeParent);
	CreateXMLNode(pXMLDoc,parent,_T("Translation"),&tmNode);
	Buf.printf("%f %f %f",ap.t.x,ap.t.y,ap.t.z);
	AddXMLText(pXMLDoc,tmNode,Buf.data());
	tmNode = NULL;
	CreateXMLNode(pXMLDoc,parent,_T("Rotation"),&tmNode);
	if(!exportQuaternions)
		Buf.printf("%f %f %f %f",rotAxis.x, rotAxis.y, rotAxis.z, rotAngle);
	else
		Buf.printf("%f %f %f %f",ap.q.x, ap.q.y, ap.q.z, ap.q.w);

	AddXMLText(pXMLDoc,tmNode,Buf.data());
	tmNode = NULL;
	CreateXMLNode(pXMLDoc,parent,_T("Scale"),&tmNode);

	Buf.printf("%f %f %f %f %f %f",ap.k.x, ap.k.y, ap.k.z, scaleAxis.x,scaleAxis.y,scaleAxis.z);
	AddXMLText(pXMLDoc,tmNode,Buf.data());


}

void GetKeyTypeName(TSTR &name, DWORD Type)
{
	if(Type==IGAME_POS)
		name.printf("Position");
	else if(Type==IGAME_ROT)
		name.printf("Rotation");
	else if(Type==IGAME_POINT3)
		name.printf("Point3");
	else if(Type==IGAME_FLOAT)
		name.printf("float");
	else if(Type==IGAME_SCALE)
		name.printf("Scale");
	else if(Type == IGameConstraint::IGAME_PATH)
		name.printf("Path");
	else if(Type == IGameConstraint::IGAME_POSITION)
		name.printf("Position");
	else if(Type == IGameConstraint::IGAME_ORIENTATION)
		name.printf("Orientation");
	else if (Type == IGameConstraint::IGAME_LOOKAT)
		name.printf("lookAt");
	else if(Type == IGAME_TM)
		name.printf("NodeTM");
	else if(Type == IGAME_EULER_X)
		name.printf("EulerX");
	else if(Type == IGAME_EULER_Y)
		name.printf("EulerY");
	else if(Type == IGAME_EULER_Z)
		name.printf("EulerZ");


	else
		name.printf("Huh!!");

}

void IGameExporter::DumpEulerController(IGameControl * sc,CComPtr<IXMLDOMNode> prsNode)
{
	TSTR data;
	CComPtr <IXMLDOMNode> eulerNode = NULL;

	IGameKeyTab xCont,yCont,zCont;

	if(sc->GetBezierKeys(xCont,IGAME_EULER_X))
	{
		if(eulerNode ==NULL)
			CreateXMLNode(pXMLDoc,prsNode,_T("EulerController"),&eulerNode);
		DumpBezierKeys(IGAME_EULER_X,xCont,eulerNode);
	}

	if(sc->GetBezierKeys(yCont,IGAME_EULER_Y))
	{
		if(eulerNode ==NULL)
			CreateXMLNode(pXMLDoc,prsNode,_T("EulerController"),&eulerNode);
		DumpBezierKeys(IGAME_EULER_Y,yCont,eulerNode);
	}

	if(sc->GetBezierKeys(zCont,IGAME_EULER_Z))
	{
		if(eulerNode ==NULL)
			CreateXMLNode(pXMLDoc,prsNode,_T("EulerController"),&eulerNode);
		DumpBezierKeys(IGAME_EULER_Z,zCont,eulerNode);
	}

}

void IGameExporter::DumpListController(IGameControl * sc,CComPtr<IXMLDOMNode> prsNode)
{
	TSTR data;
	CComPtr <IXMLDOMNode> listNode;
	CreateXMLNode(pXMLDoc,prsNode,_T("ListController"),&listNode);

	int subNum = sc->GetNumOfListSubControls(IGAME_ROT);
	data.printf("%d",subNum);
	AddXMLAttribute(listNode,_T("NumSubController"),data.data());

	for(int ii=0;ii<subNum;ii++)
	{
		IGameKeyTab SubCont;
		IGameControl * sub = sc->GetListSubControl(ii,IGAME_ROT);
		ExportRotationControllers(listNode,sub);

/*		if(sub->GetBezierKeys(SubCont,IGAME_ROT))
		{
			DumpBezierKeys(IGAME_ROT,SubCont,listNode);
		}
		if(sub->GetTCBKeys(SubCont,IGAME_ROT))
		{
			DumpTCBKeys(IGAME_ROT,SubCont,listNode);
		}
*/

	}


}

void IGameExporter::DumpSampleKeys(IGameControl * sc,CComPtr<IXMLDOMNode> prsNode, DWORD Type, bool quick)
{
	Tab<Matrix3>sKey;  
	Tab<GMatrix>gKey;
	IGameKeyTab Key;
	IGameControl * c = sc;
	CComPtr<IXMLDOMNode> sampleData;
	TSTR Buf;

	TSTR name;
	GetKeyTypeName(name,Type);
	sampleData = NULL;

	if(!c)
		return;

	if(!quick && c->GetFullSampledKeys(Key,framePerSample,IGameControlType(Type)) )
	{

		CreateXMLNode(pXMLDoc,prsNode,name,&sampleData);
		Buf.printf("%d",Key.Count());
		AddXMLAttribute(sampleData,_T("KeyCount"),Buf.data());
		AddXMLAttribute(sampleData,_T("Type"),_T("FullSampled"));
		Buf.printf("%d",framePerSample);
		AddXMLAttribute(sampleData,_T("SampleRate"),Buf.data());

		for(int i=0;i<Key.Count();i++)
		{
			CComPtr<IXMLDOMNode> data;
			CreateXMLNode(pXMLDoc,sampleData,_T("Sample"),&data);
			int fc = Key[i].t;
			Buf.printf("%d",fc);
			AddXMLAttribute(data,_T("frame"),Buf.data());

			if(Type ==IGAME_POS)
			{
				Point3 k = Key[i].sampleKey.pval;
				Buf.printf("%f %f %f",k.x,k.y,k.z); 
				AddXMLText(pXMLDoc,data,Buf.data());
			}
			if(Type == IGAME_ROT)
			{
				Quat q = Key[i].sampleKey.qval;
				AngAxis a(q);
				if(!exportQuaternions)
					Buf.printf("%f %f %f %f",a.axis.x, a.axis.y, a.axis.z, a.angle);
				else
					Buf.printf("%f %f %f %f",q.x,q.y, q.z, q.w);

				AddXMLText(pXMLDoc,data,Buf.data());

			}
			if(Type == IGAME_SCALE)
			{
				ScaleValue sval = Key[i].sampleKey.sval;
				Point3 s = sval.s;
				AngAxis a(sval.q);
				if(!exportQuaternions)
					Buf.printf("%f %f %f %f %f %f %f",sval.s.x,sval.s.y,sval.s.z,a.axis.x, a.axis.y, a.axis.z, a.angle);
				else
					Buf.printf("%f %f %f %f %f %f %f",sval.s.x,sval.s.y,sval.s.z,sval.q.x, sval.q.y,sval.q.z, sval.q.w);
				AddXMLText(pXMLDoc,data,Buf.data());

			}
			if(Type == IGAME_FLOAT)
			{
				float f = Key[i].sampleKey.fval;
				Buf.printf("%f",f);	
				AddXMLText(pXMLDoc,data,Buf.data());
			}
			if(Type == IGAME_TM)
			{
				//Even though its a 4x4 we dump it as a 4x3 ;-)
				GMatrix m;
				DumpMatrix(Key[i].sampleKey.gval.ExtractMatrix3(),data);
			}
		}
	}

	//mainly for the IK On/Off controller
	if(quick && c->GetQuickSampledKeys(Key,IGameControlType(Type)) )
	{

		CreateXMLNode(pXMLDoc,prsNode,name,&sampleData);
		Buf.printf("%d",Key.Count());
		AddXMLAttribute(sampleData,_T("KeyCount"),Buf.data());
		AddXMLAttribute(sampleData,_T("Type"),_T("QuickSampled"));

		for(int i=0;i<Key.Count();i++)
		{
			CComPtr<IXMLDOMNode> data;
			CreateXMLNode(pXMLDoc,sampleData,_T("Sample"),&data);
			int fc = Key[i].t;
			Buf.printf("%d",fc);
			AddXMLAttribute(data,_T("frame"),Buf.data());
			if(Type == IGAME_FLOAT)
			{
				float f = Key[i].sampleKey.fval;
				Buf.printf("%f",f);	
				AddXMLText(pXMLDoc,data,Buf.data());
			}

		}
	}

}
void IGameExporter::DumpSkin(CComPtr<IXMLDOMNode> modNode, IGameSkin * s)
{
	CComPtr <IXMLDOMNode> skinNode;
	TSTR Buf;

	if(s->GetSkinType()== IGameSkin::IGAME_PHYSIQUE)
		Buf.printf("%s",_T("Physique"));
	else
		Buf.printf("%s",_T("MaxSkin"));

	AddXMLAttribute(modNode,_T("skinType"),Buf.data());

	GMatrix skinTM;

	s->GetInitSkinTM(skinTM);

	for(int x=0; x<s->GetNumOfSkinnedVerts();x++)
	{
		int type = s->GetVertexType(x);
		if(type==IGameSkin::IGAME_RIGID)
		{
			CComPtr <IXMLDOMNode> boneNode;
			skinNode = NULL;
			CreateXMLNode(pXMLDoc,modNode,_T("Skin"),&skinNode);
			Buf.printf("%d",x);
			AddXMLAttribute(skinNode,_T("vertexID"),Buf.data());
			AddXMLAttribute(skinNode,_T("Type"),_T("Rigid"));
			CreateXMLNode(pXMLDoc,skinNode,_T("Bone"),&boneNode);
			ULONG id = s->GetBoneID(x,0);
			Buf.printf("%d",id);
			AddXMLAttribute(boneNode,_T("BoneID"),Buf.data());

		}
		else //blended
		{
			CComPtr <IXMLDOMNode> boneNode;
			skinNode = NULL;
			CreateXMLNode(pXMLDoc,modNode,_T("Skin"),&skinNode);
			Buf.printf("%d",x);
			AddXMLAttribute(skinNode,_T("vertexID"),Buf.data());
			AddXMLAttribute(skinNode,_T("Type"),_T("Blended"));

			for(int y=0;y<s->GetNumberOfBones(x);y++)
			{
				boneNode = NULL;
				CreateXMLNode(pXMLDoc,skinNode,_T("Bone"),&boneNode);
				ULONG id = s->GetBoneID(x,y);
				Buf.printf("%d",id);
				AddXMLAttribute(boneNode,_T("BoneID"),Buf.data());
				float weight = s->GetWeight(x,y);
				Buf.printf("%f",weight);
				AddXMLAttribute(boneNode,_T("Weight"),Buf.data());
			}
		}
	}
}

void IGameExporter::DumpModifiers(CComPtr<IXMLDOMNode> modNode, IGameModifier * m)
{
	CComPtr <IXMLDOMNode> propNode;
	TSTR buf;
	if(exportSkin || exportGenMod)
	{
	
		CreateXMLNode(pXMLDoc,modNode,_T("Modifier"),&propNode);
		AddXMLAttribute(propNode,_T("modName"),m->GetInternalName());
		bool bS = m->IsSkin();
		if(bS)
			buf.printf(_T("true"));
		else
			buf.printf(_T("false"));
		AddXMLAttribute(propNode,_T("IsSkin"),buf.data());
		
		if(m->IsSkin() && exportSkin)
		{
			IGameSkin * skin = (IGameSkin*)m;
			DumpSkin(propNode,skin);
		}
	}

}

void IGameExporter::DumpLight(IGameLight *lt, CComPtr<IXMLDOMNode> parent)
{

	IGameProperty * prop;
	CComPtr <IXMLDOMNode> propNode,targNode;


	CreateXMLNode(pXMLDoc,parent,_T("Properties"),&propNode);
	prop = lt->GetLightColor();
	DumpProperties(propNode,prop);
	prop = lt->GetLightMultiplier();
	DumpProperties(propNode,prop);
	prop = lt->GetLightAspectRatio();
	DumpProperties(propNode,prop);
	prop = lt->GetLightAttenEnd();
	DumpProperties(propNode,prop);
	prop = lt->GetLightAttenStart();
	DumpProperties(propNode,prop);
	prop = lt->GetLightFallOff();
	DumpProperties(propNode,prop);
	prop = lt->GetLightAspectRatio();
	DumpProperties(propNode,prop);
	
	if(lt->GetLightType()==TSPOT_LIGHT )
	{
		CreateXMLNode(pXMLDoc,parent,_T("Target"),&targNode);
		ExportChildNodeInfo(targNode,lt->GetLightTarget());
	}

}

void IGameExporter::DumpCamera(IGameCamera *ca, CComPtr<IXMLDOMNode> parent)
{

	IGameProperty * prop;
	CComPtr <IXMLDOMNode> propNode,targNode;


	CreateXMLNode(pXMLDoc,parent,_T("Properties"),&propNode);
	prop = ca->GetCameraFOV();
	DumpProperties(propNode,prop);
	prop = ca->GetCameraFarClip();
	DumpProperties(propNode,prop);
	prop = ca->GetCameraNearClip();
	DumpProperties(propNode,prop);
	prop = ca->GetCameraTargetDist();
	DumpProperties(propNode,prop);
	
	if(ca->GetCameraTarget())
	{
		CreateXMLNode(pXMLDoc,parent,_T("Target"),&targNode);
		ExportChildNodeInfo(targNode,ca->GetCameraTarget());
	}
	

}

void IGameExporter::DumpIKChain(IGameIKChain * ikch, CComPtr<IXMLDOMNode> ikData)
{
	CComPtr <IXMLDOMNode> ikRoot,ikNode, ikEnabled;
	TSTR buf;

	CreateXMLNode(pXMLDoc,ikData,_T("IKNodes"),&ikRoot);
	buf.printf("%d",ikch->GetNumberofBonesinChain());
	AddXMLAttribute(ikRoot,_T("NumOfNodesInChain"),buf.data());

	for(int i=0;i<ikch->GetNumberofBonesinChain();i++)
	{
		ikNode = NULL;
		IGameNode * node = ikch->GetIGameNodeInChain(i);
		CreateXMLNode(pXMLDoc,ikRoot,_T("ChainNode"),&ikNode);
		buf.printf("%d",node->GetNodeID());
		AddXMLAttribute(ikNode,_T("NodeID"),buf.data());
		
	}

	IGameControl * cont = ikch->GetIKEnabledController();
	if(cont)
	{
		CreateXMLNode(pXMLDoc,ikData,_T("IKEnabled"),&ikEnabled);
		DumpSampleKeys(cont,ikEnabled,IGAME_FLOAT,true);
	}

	
}

void IGameExporter::DumpSpline(IGameSpline * sp,CComPtr<IXMLDOMNode> splineData)
{
	CComPtr <IXMLDOMNode> spline,knotData;
	TSTR buf;

	buf.printf("%d",sp->GetNumberOfSplines());
	AddXMLAttribute(splineData,_T("NumOfSplines"),buf.data());

	for(int i=0;i<sp->GetNumberOfSplines();i++)
	{
		IGameSpline3D * sp3d = sp->GetIGameSpline3D(i);
		spline=NULL;
		CreateXMLNode(pXMLDoc,splineData,_T("Spline"),&spline);
		buf.printf("%d",i+1);
		AddXMLAttribute(spline,_T("index"),buf.data());
		int num = sp3d->GetIGameKnotCount();
		buf.printf("%d",num);
		AddXMLAttribute(spline,_T("NumOfKnots"),buf.data());
		for(int j=0;j<num;j++)
		{
			TSTR data;
			Point3 v;
			CComPtr <IXMLDOMNode> point,invec,outvec;
			knotData=NULL;
			IGameKnot * knot = sp3d->GetIGameKnot(j);
			CreateXMLNode(pXMLDoc,spline,_T("knot"),&knotData);
			CreateXMLNode(pXMLDoc,knotData,_T("Point"),&point);
			v = knot->GetKnotPoint();
			data.printf("%f %f %f",v.x,v.y,v.z);
			AddXMLText(pXMLDoc,point,data.data());
			CreateXMLNode(pXMLDoc,knotData,_T("inVec"),&invec);
			v = knot->GetInVec();
			data.printf("%f %f %f",v.x,v.y,v.z);
			AddXMLText(pXMLDoc,invec,data.data());
			CreateXMLNode(pXMLDoc,knotData,_T("outVec"),&outvec);
			v = knot->GetOutVec();
			data.printf("%f %f %f",v.x,v.y,v.z);
			AddXMLText(pXMLDoc,outvec,data.data());

		}
	}

	IPropertyContainer * cc = sp->GetIPropertyContainer();
	IGameProperty * prop = cc->QueryProperty(_T("IGameTestString"));
	prop = cc->QueryProperty(_T("IGameTestString"));

	if(prop)
	{
		TCHAR * name;
		prop->GetPropertyValue(name);

	}

		

}

void IGameExporter::DumpGenericFace(FaceEx *fe, CComPtr<IXMLDOMNode> faceData, bool smgrp, bool n, bool vc, bool tv)
{
	TSTR data;
	FaceEx *f = fe;
	CComPtr <IXMLDOMNode> prop = NULL;
	TSTR buf;

	CreateXMLNode(pXMLDoc,faceData,_T("vert"),&prop);
	buf.printf("%d %d %d",f->vert[0],f->vert[1],f->vert[2]);
	AddXMLText(pXMLDoc,prop,buf.data());
	
	prop =NULL;	
	if(smgrp){
		CreateXMLNode(pXMLDoc,faceData,_T("smGrp"),&prop);
		buf.printf("%d",f->smGrp);
		AddXMLText(pXMLDoc,prop,buf.data());
	}

	prop =NULL;
	CreateXMLNode(pXMLDoc,faceData,_T("MatID"),&prop);
	buf.printf("%d",f->matID);
	AddXMLText(pXMLDoc,prop,buf.data());

	if(exportNormals && n)
	{
		prop =NULL;
		CreateXMLNode(pXMLDoc,faceData,_T("norm"),&prop);
		buf.printf("%d %d %d",f->norm[0],f->norm[1],f->norm[2]);
		AddXMLText(pXMLDoc,prop,buf.data());
	}
	if(exportVertexColor && vc)
	{
		prop =NULL;
		CreateXMLNode(pXMLDoc,faceData,_T("vColor"),&prop);
		buf.printf("%d %d %d",f->color[0],f->color[1],f->color[2]);
		AddXMLText(pXMLDoc,prop,buf.data());
	}
	if(exportTexCoords && tv)
	{
		prop =NULL;
		CreateXMLNode(pXMLDoc,faceData,_T("tvert"),&prop);
		buf.printf("%d %d %d",f->texCoord[0],f->texCoord[1],f->texCoord[2]);
		AddXMLText(pXMLDoc,prop,buf.data());
	}


}

void IGameExporter::DumpMesh(IGameMesh * gm,CComPtr<IXMLDOMNode> geomData)
{
	CComPtr <IXMLDOMNode> vertData,normData,faceData,vcData,tvData;
	CComPtr <IXMLDOMNode> propNode;
	TSTR buf;
	bool vcSet = false;
	bool nSet = false;
	bool tvSet = false;


	// dump Vertices
	CreateXMLNode(pXMLDoc,geomData,_T("Vertices"),&vertData);
	int numVerts = gm->GetNumberOfVerts();
	numVertex = numVerts;

	buf.printf("%d",numVerts);
	AddXMLAttribute(vertData,_T("Count"),buf.data());
	for(int i = 0;i<numVerts;i++)
	{
		CComPtr <IXMLDOMNode> vert = NULL;
		TSTR data;
		CreateXMLNode(pXMLDoc,vertData,_T("vertex"),&vert);

		buf.printf("%d",i);
		AddXMLAttribute(vert,_T("index"),buf.data());
		Point3 v; 
		if(gm->GetVertex(i,v))
		{
			if(exportObjectSpace)
			{
				GMatrix g = gm->GetIGameObjectTM();
				Matrix3 gm = g.ExtractMatrix3();
				Matrix3 invgm = Inverse(gm);
				v = v * invgm;
			}
			data.printf("%f %f %f",v.x,v.y,v.z);
			AddXMLText(pXMLDoc,vert,data.data());
		}
			
	}
	if(exportNormals)
	{
		// dump Normals
		int numNorms = gm->GetNumberOfNormals();
		if(numNorms >0)
		{
			nSet = true;
			CreateXMLNode(pXMLDoc,geomData,_T("Normals"),&normData);
			buf.printf("%d",numNorms);
			AddXMLAttribute(normData,_T("Count"),buf.data());
		}
		
		for(int i = 0;i<numNorms;i++)
		{
			CComPtr <IXMLDOMNode> norm = NULL;
			TSTR data;
			CreateXMLNode(pXMLDoc,normData,_T("normal"),&norm);
			buf.printf("%d",i);
			AddXMLAttribute(norm,_T("index"),buf.data());
			Point3 n;
			if(gm->GetNormal(i,n))
			{
				buf.printf("%f %f %f",n.x,n.y,n.z);
				AddXMLText(pXMLDoc,norm,buf.data());
			}
		}
	}
	if(exportTexCoords)
	{
		// dump tvs
		int numTVs = gm->GetNumberOfTexVerts();
		if(numTVs >0)
		{
			tvSet = true;
			CreateXMLNode(pXMLDoc,geomData,_T("TextureCordinates"),&tvData);
			buf.printf("%d",numTVs);
			AddXMLAttribute(tvData,_T("Count"),buf.data());
		}
		for(int i = 0;i<numTVs;i++)
		{
			CComPtr <IXMLDOMNode> norm = NULL;
			TSTR data;
			CreateXMLNode(pXMLDoc,tvData,_T("tVertex"),&norm);
			buf.printf("%d",i);
			AddXMLAttribute(norm,_T("index"),buf.data());
			Point2 tv;
			if(gm->GetTexVertex(i,tv))
			{
				buf.printf("%f %f",tv.x,tv.y);
				AddXMLText(pXMLDoc,norm,buf.data());
			}
		}
	}

	if(exportVertexColor)
	{
		// dump VC
		int numVCs = gm->GetNumberOfColorVerts();
		if(numVCs >0)
		{
			vcSet = true;
			CreateXMLNode(pXMLDoc,geomData,_T("VertexColor"),&vcData);
			buf.printf("%d",numVCs);
			AddXMLAttribute(vcData,_T("Count"),buf.data());
		}
		for(int i = 0;i<numVCs;i++)
		{
			CComPtr <IXMLDOMNode> vc = NULL;
			TSTR data;
			CreateXMLNode(pXMLDoc,vcData,_T("vColor"),&vc);
			buf.printf("%d",i);
			AddXMLAttribute(vc,_T("index"),buf.data());
			Point3 n;
			if(gm->GetColorVertex(i,n))
			{
				buf.printf("%f %f %f",n.x,n.y,n.z);
				AddXMLText(pXMLDoc,vc,buf.data());
			}
		}
	}

	// dump Face data
	CreateXMLNode(pXMLDoc,geomData,_T("Faces"),&faceData);

	if(exportFaceSmgp)
	{
		Tab<DWORD> smGrps = gm->GetActiveSmgrps();

		for (int x=0;x<smGrps.Count();x++)
		{
			CComPtr <IXMLDOMNode> smgrpFace;
			TSTR data;
			Tab <int > smFace = gm->GetFaceIndexFromSmgrp(smGrps[x]);
			CreateXMLNode(pXMLDoc,faceData,_T("SmoothingBased"),&smgrpFace);
			buf.printf("%d",smGrps[x]);
			AddXMLAttribute(smgrpFace,_T("Group"),buf.data());
			buf.printf("%d",smFace.Count());
			AddXMLAttribute(smgrpFace,_T("Count"),buf.data());

			for(int f=0;f<smFace.Count();f++)
			{
				CComPtr <IXMLDOMNode> norm = NULL;
				CreateXMLNode(pXMLDoc,smgrpFace,_T("Face"),&norm);
				data.printf("%d",smFace[f]);
				AddXMLAttribute(norm,_T("index"),data.data());
				DumpGenericFace(gm->GetFace(smFace[f]),norm,false,nSet,vcSet,tvSet);
			}

		}
	}
	else
	{
		TSTR data;
		int numFaces = gm->GetNumberOfFaces();
		buf.printf("%d",numFaces);
		AddXMLAttribute(faceData,_T("Count"),buf.data());

		for(int f=0;f<numFaces;f++)
		{
			CComPtr <IXMLDOMNode> norm = NULL;
			CreateXMLNode(pXMLDoc,faceData,_T("Face"),&norm);
			data.printf("%d",f);
			AddXMLAttribute(norm,_T("index"),data.data());
			DumpGenericFace(gm->GetFace(f),norm,true,nSet,vcSet,tvSet);
		}
		
	}

	if(exportMappingChannel)
	{
		TSTR data;
		CComPtr <IXMLDOMNode> channelNode;
		Tab<int> mapNums = gm->GetActiveMapChannelNum();
		CreateXMLNode(pXMLDoc,geomData,_T("MappingChannels"),&channelNode);
		int mapCount = mapNums.Count();
		buf.printf("%d",mapCount);
		AddXMLAttribute(channelNode,_T("Count"),buf.data());

		for(int i=0;i < mapCount;i++)
		{
			CComPtr <IXMLDOMNode> channelItem, mvertData,vert,mfaceData,face;
			CreateXMLNode(pXMLDoc,channelNode,_T("MapChannel"),&channelItem);
			buf.printf("%d",mapNums[i]);
			AddXMLAttribute(channelItem,_T("ChannelNum"),buf.data());
			CreateXMLNode(pXMLDoc,channelItem,_T("MapVertexData"),&mvertData);
			int vCount = gm->GetNumberOfMapVerts(mapNums[i]);
			buf.printf("%d",vCount);
			AddXMLAttribute(mvertData,_T("Count"),buf.data());

			for(int j=0;j<vCount;j++)
			{
				vert = NULL;
				CreateXMLNode(pXMLDoc,mvertData,_T("mapvertex"),&vert);
				buf.printf("%d",j);
				AddXMLAttribute(vert,_T("index"),buf.data());
				Point3 v;
				if(gm->GetMapVertex(mapNums[i],j,v))
				{
					data.printf("%f %f %f",v.x,v.y,v.z);
					AddXMLText(pXMLDoc,vert,data.data());
				}
		
			}
			CreateXMLNode(pXMLDoc,channelItem,_T("MapFaceData"),&mfaceData);
			int fCount = gm->GetNumberOfFaces();
			buf.printf("%d",fCount);
			AddXMLAttribute(mfaceData,_T("Count"),buf.data());

			for(int k=0;k<fCount;k++)
			{
				face = NULL;
				CreateXMLNode(pXMLDoc,mfaceData,_T("mapface"),&face);
				buf.printf("%d",k);
				AddXMLAttribute(face,_T("index"),buf.data());
				DWORD  v[3];
				gm->GetMapFaceIndex(mapNums[i],k,v);
				data.printf("%d %d %d",v[0],v[1],v[2]);
				AddXMLText(pXMLDoc,face,data.data());
			
			}
		}

	}
//test code
	Tab<int> matids;

	matids = gm->GetActiveMatIDs();

	for(i=0;i<matids.Count();i++)
	{
		Tab<FaceEx*> faces;

		faces = gm->GetFacesFromMatID(matids[i]);

		for(int k=0; k<faces.Count();k++)
		{
			IGameMaterial * faceMat = gm->GetMaterialFromFace(faces[k]);
			if(faceMat)
				DebugPrint ("Face %d, Material Name = %s\n",k,faceMat->GetMaterialName());
		}
		
	}

	vertData = NULL;
	normData = NULL;
	faceData = NULL;
}



void IGameExporter::DumpConstraints(CComPtr<IXMLDOMNode> prsData, IGameConstraint * c)
{
	TSTR buf,name;
	CComPtr <IXMLDOMNode> prsChild;
	CComPtr <IXMLDOMNode> constNode;

	if(exportConstraints)
	{
	
		CreateXMLNode(pXMLDoc,prsData,_T("Constrainst"),&prsChild);
		GetKeyTypeName(name, c->GetConstraintType());
		AddXMLAttribute(prsChild,_T("Type"),name.data());
		buf.printf("%d",c->NumberOfConstraintNodes());
		AddXMLAttribute(prsChild,_T("NodeCount"),buf.data());

		for(int i=0;i<c->NumberOfConstraintNodes();i++ )
		{
			constNode = NULL;
			CreateXMLNode(pXMLDoc,prsChild,_T("ConstNode"),&constNode);
			float w = c->GetConstraintWeight(i);
			int id = c->GetConstraintNodes(i)->GetNodeID();
			
			buf.printf("%d", id);
			AddXMLAttribute(constNode,_T("NodeID"),buf.data());

			buf.printf("%f", w);
			AddXMLAttribute(constNode,_T("Weight"),buf.data());

		}
		for(i=0;i<c->GetIPropertyContainer()->GetNumberOfProperties();i++)
		{
			DumpProperties(prsChild,c->GetIPropertyContainer()->GetProperty(i));
		}
	}
}




void IGameExporter::DumpBezierKeys(DWORD Type, IGameKeyTab Keys, CComPtr<IXMLDOMNode> prsData)
{
	TSTR buf,name;
	CComPtr <IXMLDOMNode> prsChild;
	GetKeyTypeName(name,Type);

	if(Keys.Count()==0)
		return;

	CreateXMLNode(pXMLDoc,prsData,name,&prsChild);

	buf.printf("%d",Keys.Count());
	AddXMLAttribute(prsChild,_T("KeyCount"),buf.data());
	AddXMLAttribute(prsChild,_T("Type"),_T("Bezier"));

	for(int i = 0;i<Keys.Count();i++)
	{
		CComPtr <IXMLDOMNode> key = NULL;
		TSTR data;
		CreateXMLNode(pXMLDoc,prsChild,_T("Key"),&key);
		buf.printf("%d",Keys[i].t);
		AddXMLAttribute(key,_T("time"),buf.data());
		if(Type==IGAME_POS || Type==IGAME_POINT3)
		{
			Point3 k = Keys[i].bezierKey.pval;
			data.printf("%f %f %f",k.x,k.y,k.z); 
		}
		else if (Type == IGAME_ROT)
		{
			Quat q = Keys[i].bezierKey.qval;
			AngAxis a(q);
			if(!exportQuaternions)
				data.printf("%f %f %f %f",a.axis.x, a.axis.y, a.axis.z, a.angle);
			else
				data.printf("%f %f %f %f",q.x, q.y, q.z, q.w);

		}
		else if ( Type == IGAME_FLOAT || Type==IGAME_EULER_X || Type == IGAME_EULER_Y || Type==IGAME_EULER_Z)
		{
			float f = Keys[i].bezierKey.fval;
			data.printf("%f",f);
		}
		else{
			ScaleValue sval = Keys[i].bezierKey.sval;
			Point3 s = sval.s;
			AngAxis a(sval.q);
			if(!exportQuaternions)
				data.printf("%f %f %f %f %f %f %f",sval.s.x,sval.s.y,sval.s.z,a.axis.x, a.axis.y, a.axis.z, a.angle);
			else
				data.printf("%f %f %f %f %f %f %f",sval.s.x,sval.s.y,sval.s.z,sval.q.x, sval.q.y,sval.q.z, sval.q.w);

		}
		AddXMLText(pXMLDoc,key,data.data());
		
	}

	prsChild = NULL;

}
void IGameExporter::DumpLinearKeys(DWORD Type, IGameKeyTab Keys, CComPtr<IXMLDOMNode> prsData)
{
	TSTR buf,name;
	CComPtr <IXMLDOMNode> prsChild;
	GetKeyTypeName(name,Type);

	if(Keys.Count()==0)
		return;

	CreateXMLNode(pXMLDoc,prsData,name.data(),&prsChild);

	buf.printf("%d",Keys.Count());
	AddXMLAttribute(prsChild,_T("KeyCount"),buf.data());
	AddXMLAttribute(prsChild,_T("Type"),_T("Linear"));

	for(int i = 0;i<Keys.Count();i++)
	{
		CComPtr <IXMLDOMNode> key = NULL;
		TSTR data;
		CreateXMLNode(pXMLDoc,prsChild,_T("Key"),&key);
		buf.printf("%d",Keys[i].t);
		AddXMLAttribute(key,_T("time"),buf.data());
		if(Type==IGAME_POS)
		{
			Point3 k = Keys[i].linearKey.pval;
			data.printf("%f %f %f",k.x,k.y,k.z); 
		}
		else if (Type == IGAME_ROT)
		{
			Quat q = Keys[i].linearKey.qval;
			AngAxis a(q);
			if(!exportQuaternions)
				data.printf("%f %f %f %f",a.axis.x, a.axis.y, a.axis.z, a.angle);
			else
				data.printf("%f %f %f %f",q.x, q.y, q.z, q.w);

		}
		else if ( Type == IGAME_FLOAT || Type==IGAME_EULER_X || Type == IGAME_EULER_Y || Type==IGAME_EULER_Z)
		{
			float f = Keys[i].linearKey.fval;
			data.printf("%f",f);	
		}
		else{
			ScaleValue sval = Keys[i].linearKey.sval;
			Point3 s = sval.s;
			AngAxis a(sval.q);
			if(exportQuaternions)
				data.printf("%f %f %f %f %f %f %f",sval.s.x,sval.s.y,sval.s.z,a.axis.x, a.axis.y, a.axis.z, a.angle);
			else
				data.printf("%f %f %f %f %f %f %f",sval.s.x,sval.s.y,sval.s.z,sval.q.x, sval.q.y,sval.q.z, sval.q.w);

		}
		AddXMLText(pXMLDoc,key,data.data());
		
	}

	prsChild = NULL;


}
void IGameExporter::DumpTCBKeys(DWORD Type, IGameKeyTab Keys, CComPtr<IXMLDOMNode> prsData)
{
	TSTR buf,name;
	CComPtr <IXMLDOMNode> prsChild;

	if(Keys.Count()==0)
		return;

	GetKeyTypeName(name,Type);
	CreateXMLNode(pXMLDoc,prsData,name.data(),&prsChild);

	buf.printf("%d",Keys.Count());
	AddXMLAttribute(prsChild,_T("KeyCount"),buf.data());
	AddXMLAttribute(prsChild,_T("Type"),_T("TCB"));

	for(int i = 0;i<Keys.Count();i++)
	{
		CComPtr <IXMLDOMNode> key = NULL;
		TSTR data;
		CreateXMLNode(pXMLDoc,prsChild,_T("Key"),&key);
		buf.printf("%d",Keys[i].t);
		AddXMLAttribute(key,_T("time"),buf.data());
		if(Type==IGAME_POS)
		{
			Point3 k = Keys[i].tcbKey.pval;
			data.printf("%f %f %f",k.x,k.y,k.z); 
		}
		else if (Type == IGAME_ROT)
		{
			AngAxis a = Keys[i].tcbKey.aval;
			data.printf("%f %f %f %f",a.axis.x, a.axis.y, a.axis.z, a.angle);

		}
		else{
			ScaleValue sval = Keys[i].tcbKey.sval;
			Point3 s = sval.s;
			AngAxis a(sval.q);
			if(!exportQuaternions)
				data.printf("%f %f %f %f %f %f %f",sval.s.x,sval.s.y,sval.s.z,a.axis.x, a.axis.y, a.axis.z, a.angle);
			else
				data.printf("%f %f %f %f %f %f %f",sval.s.x,sval.s.y,sval.s.z,sval.q.x, sval.q.y,sval.q.z, sval.q.w);

		}
		AddXMLText(pXMLDoc,key,data.data());
		
	}

	prsChild = NULL;


}

void IGameExporter::DumpProperties(CComPtr<IXMLDOMNode> node, IGameProperty * prop)
{
	TSTR Buf;
	IGameKeyTab keys;
	CComPtr <IXMLDOMNode> propData;
	CComPtr <IXMLDOMNode> keyNode;

	if(!prop)	//fix me NH...
		return;
	CreateXMLNode(pXMLDoc,node,_T("Prop"),&propData);
	AddXMLAttribute(propData,_T("name"),prop->GetName());

	if(prop->GetType() == IGAME_POINT3_PROP)
	{
		Point3 p; 
		prop->GetPropertyValue(p);
		Buf.printf("%f %f %f",p.x,p.y,p.z);
	}
	else if( prop->GetType() == IGAME_FLOAT_PROP)
	{
		float f;
		prop->GetPropertyValue(f);
		Buf.printf("%f", f);
	}
	else if(prop->GetType()==IGAME_STRING_PROP)
	{
		TCHAR * b;
		prop->GetPropertyValue(b);
		Buf.printf("$s",b);
	}
	else
	{
		int i;
		prop->GetPropertyValue(i);
		Buf.printf("%f", i);

	}
	AddXMLAttribute(propData,_T("value"),Buf.data());

	if(prop->IsPropAnimated() && exportControllers)
	{
		IGameControl * c = prop->GetIGameControl();
		CreateXMLNode(pXMLDoc,propData,_T("PropKeyData"),&keyNode);


		if(prop->GetType() == IGAME_POINT3_PROP)
		{
			if(	c->GetBezierKeys(keys,IGAME_POINT3 )){
				DumpBezierKeys(IGAME_POINT3,keys,keyNode);
			}
		}
		if(prop->GetType()==IGAME_FLOAT_PROP)
		{
			if(	c->GetBezierKeys(keys,IGAME_FLOAT )){
				DumpBezierKeys(IGAME_FLOAT,keys,keyNode);
			}
		}
	}
	propData = NULL;
	keyNode = NULL;
}	


void IGameExporter::DumpTexture(CComPtr<IXMLDOMNode> node,IGameMaterial * mat)
{
	CComPtr <IXMLDOMNode> textureRoot;
	TSTR buf;
	int texCount = mat->GetNumberOfTextureMaps();

	if(texCount>0)
	{
		CreateXMLNode(pXMLDoc,node,_T("TextureMaps"),&textureRoot);
		buf.printf("%d",texCount);
		AddXMLAttribute(textureRoot,_T("Count"),buf.data());
	}
	
	for(int i=0;i<texCount;i++)
	{
		CComPtr <IXMLDOMNode> texture;
		IGameTextureMap * tex = mat->GetIGameTextureMap(i);
		CreateXMLNode(pXMLDoc,textureRoot,_T("Texture"),&texture);
		buf.printf("%d",i);
		AddXMLAttribute(texture,_T("index"),buf.data());
		TCHAR * name = tex->GetTextureName();
		AddXMLAttribute(texture,_T("name"),name);
		int slot = tex->GetStdMapSlot();
		buf.printf("%d",slot);
		AddXMLAttribute(texture,_T("StdSlotType"),buf.data());

		if(tex->IsEntitySupported())	//its a bitmap texture
		{
			CComPtr <IXMLDOMNode> bitmapTexture;
			CreateXMLNode(pXMLDoc,texture,_T("BitmapTexture"),&bitmapTexture);

			CComPtr <IXMLDOMNode> bitmapName;
			CreateXMLNode(pXMLDoc,bitmapTexture,_T("Name"),&bitmapName);
			AddXMLText(pXMLDoc,bitmapName,tex->GetBitmapFileName());

			IGameProperty * prop = tex->GetClipHData();
			DumpProperties(bitmapTexture,prop);

			prop = tex->GetClipUData();
			DumpProperties(bitmapTexture,prop);

			prop = tex->GetClipVData();
			DumpProperties(bitmapTexture,prop);

			prop = tex->GetClipWData();
			DumpProperties(bitmapTexture,prop);

		}

	}

}

void IGameExporter::DumpMaterial(CComPtr<IXMLDOMNode> node,IGameMaterial * mat, int index, int matID )
{
	TSTR buf;
	IGameProperty *prop;
	CComPtr <IXMLDOMNode> material;
	CComPtr <IXMLDOMNode> propNode;

	CreateXMLNode(pXMLDoc,node,_T("Material"),&material);
	buf.printf("%d",index);
	AddXMLAttribute(material,_T("index"),buf.data());
	
	if(matID !=-1)	// we are not a sub material
	{
		buf.printf("%d",matID);
		AddXMLAttribute(material,_T("MaterialID"),buf.data());
	}

	AddXMLAttribute(material,_T("Name"),mat->GetMaterialName());
	CComPtr <IXMLDOMNode> matData;
	CreateXMLNode(pXMLDoc,material,_T("NumSubMtls"),&matData);
	buf.printf("%d",mat->GetSubMaterialCount());
	AddXMLText(pXMLDoc,matData,buf.data());

	//WE ONLY WANT THE PROPERTIES OF THE ACTUAL MATERIAL NOT THE CONTAINER - FOR NOW.....
	if(!mat->IsMultiType())
	{
		CreateXMLNode(pXMLDoc,material,_T("Properties"),&propNode);
		prop = mat->GetDiffuseData();
		DumpProperties(propNode,prop);
		prop = mat->GetAmbientData();
		DumpProperties(propNode,prop);
		prop = mat->GetSpecularData();
		DumpProperties(propNode,prop);
		prop = mat->GetOpacityData();
		DumpProperties(propNode,prop);
		prop = mat->GetGlossinessData();
		DumpProperties(propNode,prop);
		prop = mat->GetSpecularLevelData();
		DumpProperties(propNode,prop);
	}

	//do the textures if they are there

	DumpTexture(material,mat);

	if(mat->IsMultiType())
	{
		CComPtr <IXMLDOMNode> multi;
		CreateXMLNode(pXMLDoc,material,_T("MultiMaterial"),&multi);
		for(int k=0;k<mat->GetSubMaterialCount();k++)
		{
			int mID = mat->GetMaterialID(k);
			IGameMaterial * subMat = mat->GetSubMaterial(k);
			DumpMaterial(multi,subMat,k, mID);
		}
	}
	
}

void IGameExporter::ExportMaterials()
{
	CComPtr <IXMLDOMNode> matNode;
	TSTR buf;
	if(exportMaterials)
	{
	
		CreateXMLNode(pXMLDoc,iGameNode,_T("MaterialList"),&matNode);
		int matCount = pIgame->GetRootMaterialCount();
		buf.printf("%d",matCount);
		AddXMLAttribute(matNode,_T("Count"),buf.data());

		for(int j =0;j<matCount;j++)
		{
			IGameMaterial * mat = pIgame->GetRootMaterial(j);
			if(mat)
				DumpMaterial(matNode,mat,j);
		}
	}
}

// Dummy function for progress bar
DWORD WINAPI fn(LPVOID arg)
{
	return(0);
}



class MyErrorProc : public IGameErrorCallBack
{
public:
	void ErrorProc(IGameError error)
	{
		TCHAR * buf = GetLastIGameErrorText();
		DebugPrint("ErrorCode = %d ErrorText = %s\n", error,buf);
	}
};


int	IGameExporter::DoExport(const TCHAR *name,ExpInterface *ei,Interface *i, BOOL suppressPrompts, DWORD options)
{
	HRESULT hr;

	Interface * ip = GetCOREInterface();

	MyErrorProc pErrorProc;

	UserCoord Whacky = {
		1,	//Right Handed
		1,	//X axis goes right
		3,	//Y Axis goes down
		4,	//Z Axis goes in.
		1,	//U Tex axis is right
		1,  //V Tex axis is Down
	};	

	SetErrorCallBack(&pErrorProc);

	ReadConfig();
	hr = CoInitialize(NULL); 
	// Check the return value, hr...
	hr = CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER,  IID_IXMLDOMDocument, (void**)&pXMLDoc);
	if(FAILED(hr))
		return false;
	// Check the return value, hr...
	hr = pXMLDoc->QueryInterface(IID_IXMLDOMNode, (void **)&pRoot);
	if(FAILED(hr))
		return false;

	// Set a global prompt display switch
	showPrompts = suppressPrompts ? FALSE : TRUE;

	exportSelected = (options & SCENE_EXPORT_SELECTED) ? true : false;


	if(showPrompts) 
	{
		// Prompt the user with our dialogbox, and get all the options.
		if (!DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_PANEL),
			i->GetMAXHWnd(), IGameExporterOptionsDlgProc, (LPARAM)this)) {
			return 1;
		}
	}

	curNode = 0;
	ip->ProgressStart(_T("Exporting Using IGame.."), TRUE, fn, NULL);
	
	pIgame = GetIGameInterface();

	IGameConversionManager * cm = GetConversionManager();
	cm->SetUserCoordSystem(Whacky);
//	cm->SetCoordSystem((IGameConversionManager::CoordSystem)cS);
//	pIgame->SetPropertyFile(_T("hello world"));
	pIgame->InitialiseIGame(exportSelected);
	pIgame->SetStaticFrame(staticFrame);
	
	TSTR path,file,ext;

	SplitFilename(TSTR(name),&path,&file,&ext);

	splitPath = path;
	
	ExportSceneInfo();
	ExportMaterials();


	for(int loop = 0; loop <pIgame->GetTopLevelNodeCount();loop++)
	{
		IGameNode * pGameNode = pIgame->GetTopLevelNode(loop);
		//check for selected state - we deal with targets in the light/camera section
		if(pGameNode->IsTarget())
			continue;
		ExportNodeInfo(pGameNode);

	}
	pIgame->ReleaseIGame();
	pXMLDoc->save(CComVariant(name));
//	pXMLDoc->Release();
	CoUninitialize();
	ip->ProgressEnd();	
	WriteConfig();
	return TRUE;
}


TSTR IGameExporter::GetCfgFilename()
{
	TSTR filename;
	
	filename += GetCOREInterface()->GetDir(APP_PLUGCFG_DIR);
	filename += "\\";
	filename += "IgameExport.cfg";
	return filename;
}

// NOTE: Update anytime the CFG file changes
#define CFG_VERSION 0x03

BOOL IGameExporter::ReadConfig()
{
	TSTR filename = GetCfgFilename();
	FILE* cfgStream;

	cfgStream = fopen(filename, "rb");
	if (!cfgStream)
		return FALSE;
	
	exportGeom = fgetc(cfgStream);
	exportNormals = fgetc(cfgStream);
	exportControllers = fgetc(cfgStream);
	exportFaceSmgp = fgetc(cfgStream);
	exportVertexColor = fgetc(cfgStream);
	exportTexCoords = fgetc(cfgStream);
	staticFrame = _getw(cfgStream);
	framePerSample = _getw(cfgStream);
	exportMappingChannel = fgetc(cfgStream);
	exportMaterials = fgetc(cfgStream);
	exportSplines = fgetc(cfgStream);
	exportModifiers = fgetc(cfgStream);
	forceSample = fgetc(cfgStream);
	exportConstraints = fgetc(cfgStream);
	exportSkin = fgetc(cfgStream);
	exportGenMod = fgetc(cfgStream);
	cS = fgetc(cfgStream);
	splitFile = fgetc(cfgStream);
	exportQuaternions = fgetc(cfgStream);
	exportObjectSpace = fgetc(cfgStream);
	fclose(cfgStream);
	return TRUE;
}

void IGameExporter::WriteConfig()
{
	TSTR filename = GetCfgFilename();
	FILE* cfgStream;

	cfgStream = fopen(filename, "wb");
	if (!cfgStream)
		return;

	
	fputc(exportGeom,cfgStream);
	fputc(exportNormals,cfgStream);
	fputc(exportControllers,cfgStream);
	fputc(exportFaceSmgp,cfgStream);
	fputc(exportVertexColor,cfgStream);
	fputc(exportTexCoords,cfgStream);
	_putw(staticFrame,cfgStream);
	_putw(framePerSample,cfgStream);
	fputc(exportMappingChannel,cfgStream);
	fputc(exportMaterials,cfgStream);
	fputc(exportSplines,cfgStream);
	fputc(exportModifiers,cfgStream);
	fputc(forceSample,cfgStream);
	fputc(exportConstraints,cfgStream);
	fputc(exportSkin,cfgStream);
	fputc(exportGenMod,cfgStream);
	fputc(cS,cfgStream);
	fputc(splitFile,cfgStream);
	fputc(exportQuaternions,cfgStream);
	fputc(exportObjectSpace,cfgStream);
	fclose(cfgStream);
}


