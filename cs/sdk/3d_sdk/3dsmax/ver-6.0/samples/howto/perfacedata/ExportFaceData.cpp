/**********************************************************************
 *<
	FILE: FaceDataExport.cpp

	DESCRIPTION:	Appwizard generated plugin

	CREATED BY: 

	HISTORY: 

 *>	Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/

#include "PerFaceData.h"
#include "SampleFaceData.h"

#define FACEDATAEXPORT_CLASS_ID	Class_ID(0x2867f474, 0x2a06e46d)

class FaceDataExport : public SceneExport {
	static HWND hParams;
	FILE *fileStream;
	bool exportSelected;
public:
	FaceDataExport() : fileStream(NULL), exportSelected(false) { }

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
	int DoExport(const TCHAR *name,ExpInterface *ei,Interface *i, BOOL suppressPrompts=FALSE, DWORD options=0);
	BOOL SupportsOptions(int ext, DWORD options);

	BOOL nodeEnum(INode* node,Interface *ip);
};


class FaceDataExportClassDesc:public ClassDesc2 {
public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) {return new FaceDataExport();}
	const TCHAR *	ClassName() {return GetString(IDS_FACE_DATA_EXPORT);}
	SClass_ID		SuperClassID() {return SCENE_EXPORT_CLASS_ID;}
	Class_ID		ClassID() {return FACEDATAEXPORT_CLASS_ID;}
	const TCHAR* 	Category() {return GetString(IDS_CATEGORY);}
	const TCHAR*	InternalName() { return _T("FaceDataExport"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle
};

static FaceDataExportClassDesc FaceDataExportDesc;
ClassDesc2* GetFaceDataExportDesc() {return &FaceDataExportDesc;}

BOOL CALLBACK FaceDataExportOptionsDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam) {
	static FaceDataExport *imp = NULL;

	switch(message) {
	case WM_INITDIALOG:
		imp = (FaceDataExport *)lParam;
		CenterWindow(hWnd,GetParent(hWnd));
		return TRUE;

	case WM_CLOSE:
		EndDialog(hWnd, 0);
		return TRUE;
	}
	return FALSE;
}


//--- FaceDataExport -------------------------------------------------------
int FaceDataExport::ExtCount()
{
	//TODO: Returns the number of file name extensions supported by the plug-in.
	return 1;
}

const TCHAR *FaceDataExport::Ext(int n)
{		
	return _T("ffe");
}

const TCHAR *FaceDataExport::LongDesc()
{
	return _T("Example FaceFloats Export File");
}

const TCHAR *FaceDataExport::ShortDesc() 
{			
	return _T("FaceFloats Export");
}

const TCHAR *FaceDataExport::AuthorName()
{			
	return _T("DCG");
}

const TCHAR *FaceDataExport::CopyrightMessage() 
{	
	return _T("Copyright Discreet");
}

const TCHAR *FaceDataExport::OtherMessage1() 
{		
	return _T("");
}

const TCHAR *FaceDataExport::OtherMessage2() 
{		
	return _T("");
}

unsigned int FaceDataExport::Version()
{				
	return 110;
}

void FaceDataExport::ShowAbout(HWND hWnd)
{			
	// Optional
}

BOOL FaceDataExport::SupportsOptions(int ext, DWORD options)
{
	if(options & SCENE_EXPORT_SELECTED) 
		return TRUE;

	else 
		return FALSE;

}

BOOL FaceDataExport::nodeEnum(INode* node,Interface *ip) {	
	if(!exportSelected || node->Selected()) {
		ObjectState os = node->EvalWorldState(ip->GetTime());

		IFaceDataMgr *pFDMgr = NULL;
		if (os.obj->IsSubClassOf(triObjectClassID)) {
			TriObject *tobj = (TriObject *)os.obj;
			Mesh* mesh = &tobj->GetMesh();
			pFDMgr = static_cast<IFaceDataMgr*>(mesh->GetInterface( FACEDATAMGR_INTERFACE ));
		} else if (os.obj->IsSubClassOf (polyObjectClassID)) {
			PolyObject *pobj = (PolyObject *)os.obj;
			MNMesh *mesh = &pobj->GetMesh();
			pFDMgr = static_cast<IFaceDataMgr*>(mesh->GetInterface( FACEDATAMGR_INTERFACE ));
		}
		if (pFDMgr == NULL) return FALSE;

		SampleFaceData* SampleDataChan = NULL; 
		IFaceDataChannel* fdc = pFDMgr->GetFaceDataChan( FACE_MAXSAMPLEUSE_CLSID );
		if ( fdc != NULL ) SampleDataChan = dynamic_cast<SampleFaceData*>(fdc);
		if ( SampleDataChan == NULL) {
			fprintf(fileStream,"Node %s does not have our Face Data\n",node->GetName());
			return false;
		}

		//OK so We have Face data lets dump it out..
		fprintf(fileStream,"\nNode %s has %d faces with FaceFloats\n",node->GetName(), SampleDataChan->Count());
		for(ULONG i=0;i<SampleDataChan->Count();i++) {
			float data = SampleDataChan->data[i];
			fprintf(fileStream,"Face %d, float %f\n",i,data);
		}
	}

	// Recurse through this node's children, if any
	for (int c = 0; c < node->NumberOfChildren(); c++) {
		if (!nodeEnum(node->GetChildNode(c), ip)) return FALSE;
	}

	return TRUE;
}

int FaceDataExport::DoExport(const TCHAR *name,ExpInterface *ei,Interface *i, BOOL suppressPrompts, DWORD options) {
	exportSelected = (options & SCENE_EXPORT_SELECTED) ? true : false;
	
	fileStream = _tfopen(name,_T("wt"));
	if (!fileStream) return 0;
	fprintf(fileStream,"FaceFloats Export 1.1\n\n");
	int numChildren = i->GetRootNode()->NumberOfChildren();
	for (int idx=0; idx<numChildren; idx++) {
		nodeEnum(i->GetRootNode()->GetChildNode(idx), i);
	}
	fclose(fileStream);

	return TRUE;
}
	
