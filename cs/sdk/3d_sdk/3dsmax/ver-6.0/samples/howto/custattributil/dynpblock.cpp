/**********************************************************************
 *<
	FILE: DynPBlock.cpp

	DESCRIPTION:	Handles the adding and removing of Custom Attributes
					In this sample we only allow one CA to be added per 
					Object,modifier or Material

	CREATED BY:		Nikolai Sander

	HISTORY:		Created:  5/26/00
					Turnd into Sample Neil Hazzard - DCG: 12/5/00

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#include "DynPBlock.h"
#include "modstack.h"
#include "ICustAttribContainer.h"
#include "CASample.h"
#include "IMtlEdit.h"

#define DYNPBLOCK_CLASS_ID	Class_ID(0x1c33be45, 0x5e3005a8)
#define CC_NOT_LOADED -1
#define CC_UNKNOWN_CUST_ATTRIB -2

static TCHAR strUnknownCAFoundMsg[] = _T("Unknown or standin CA Found, Aborting");
static TCHAR strWarning[] = _T("Warning");

class DynPBlock : public UtilityObj {
	public:
		HWND			hPanel;
		IUtil			*iu;
		Interface		*ip;
		
		void BeginEditParams(Interface *ip,IUtil *iu);
		void EndEditParams(Interface *ip,IUtil *iu);

		void Init(HWND hWnd);
		void Destroy(HWND hWnd);
		
		void DeleteThis() { }		
		//Constructor/Destructor
		DynPBlock();
		~DynPBlock();		

		void AddObjectCA(BOOL Insert=FALSE);
		void Remove();

		void RemoveMat();
		void AddMaterialCA(BOOL Insert = FALSE);

		int CheckCCIsLoaded(ICustAttribContainer * cc,int which,BOOL Mat = FALSE);

};


static DynPBlock theDynPBlock;

class DynPBlockClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) {return &theDynPBlock;}
	const TCHAR *	ClassName() {return GetString(IDS_CLASS_NAME);}
	SClass_ID		SuperClassID() {return UTILITY_CLASS_ID;}
	Class_ID		ClassID() {return DYNPBLOCK_CLASS_ID;}
	const TCHAR* 	Category() {return GetString(IDS_CATEGORY);}
	const TCHAR*	InternalName() { return _T("DynPBlock"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle
};



static DynPBlockClassDesc DynPBlockDesc;

// When adding new CAs make sure the ClassName appears here.  Itis used to check 
// whether the CA is loaded or not

#define CA_COUNT 4
static  char * ca_name[CA_COUNT] = {"Ca 1","Ca 2","Ca 3","Mat 1"};


ClassDesc2* GetDynPBlockDesc() {return &DynPBlockDesc;}


static BOOL CALLBACK DynPBlockDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hWndList = GetDlgItem(hWnd,IDC_CALIST);
	int curSel;

	switch (msg) {
		case WM_INITDIALOG:
			theDynPBlock.Init(hWnd);
			break;

		case WM_DESTROY:
			theDynPBlock.Destroy(hWnd);
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{
			case IDC_ADD:
				curSel = ListBox_GetCurSel(hWndList);
				if(curSel <= 2)
					theDynPBlock.AddObjectCA();
				else
					theDynPBlock.AddMaterialCA();
				break;
			
			case IDC_INSERT:
				curSel = ListBox_GetCurSel(hWndList);
				if(curSel <= 2)
					theDynPBlock.AddObjectCA(TRUE);
				else
					theDynPBlock.AddMaterialCA(TRUE);
				break;

			case IDC_REMOVE:
				curSel = ListBox_GetCurSel(hWndList);
				if(curSel <=2)
					theDynPBlock.Remove();
				else
					theDynPBlock.RemoveMat();
				break;
/*			case IDC_APPLYMAT:
				theDynPBlock.ApplyMat();
				break;
			case IDC_REMOVE_MAT:
				theDynPBlock.RemoveMat();
				break;
*/
			}


		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			theDynPBlock.ip->RollupMouseMessage(hWnd,msg,wParam,lParam); 
			break;

		default:
			return FALSE;
	}
	return TRUE;
}


//--- DynPBlock -------------------------------------------------------
DynPBlock::DynPBlock()
{
	iu = NULL;
	ip = NULL;	
	hPanel = NULL;
}

DynPBlock::~DynPBlock()
{

}

void DynPBlock::BeginEditParams(Interface *ip,IUtil *iu) 
{
	this->iu = iu;
	this->ip = ip;
	hPanel = ip->AddRollupPage(
		hInstance,
		MAKEINTRESOURCE(IDD_PANEL2),
		DynPBlockDlgProc,
		GetString(IDS_PARAMS),
		0);
}
	
void DynPBlock::EndEditParams(Interface *ip,IUtil *iu) 
{
	this->iu = NULL;
	this->ip = NULL;
	ip->DeleteRollupPage(hPanel);
	hPanel = NULL;
}



void DynPBlock::Init(HWND hWnd)
{
	// Setup the Drop Down list using the ClassName as a Cheat.
	HWND hWndList = GetDlgItem(hWnd,IDC_CALIST);

	for(int i=0;i<CA_COUNT;i++)
	{
		ListBox_AddString(hWndList, ca_name[i]);

	}
	ListBox_SetCurSel(hWndList,0);
}

void DynPBlock::Destroy(HWND hWnd)
{

}

int DynPBlock::CheckCCIsLoaded(ICustAttribContainer * cc,int which,BOOL Mat)
{
	HWND hWndList = GetDlgItem(hPanel,IDC_CALIST);



	for(int i=0;i<cc->GetNumCustAttribs();i++)
	{
		CustAttrib * ca = cc->GetCustAttrib(i);

		// aszabo|Sep.04.03|#436072
		// Minimal handling of th case when the param blk is NULL. 
		// This can happen when the CA is a standin.
		IParamBlock2* pb = ca->GetParamBlock(0);
		ParamBlockDesc2 *pd = NULL;
		if (pb != NULL)
			pd = pb->GetDesc();
		if (NULL == pd)
			return CC_UNKNOWN_CUST_ATTRIB;

		if(!Mat){
			TCHAR  buf[50];
			ListBox_GetText(hWndList, which, &buf);
			if(!strcmp(pd->cd->ClassName(),buf))
			{
				return i;
			}
		}
		else
		{
			TCHAR  buf[50];
			ListBox_GetText(hWndList, which, &buf);
			char *test = (char*) pd->cd->ClassName();
			if(!strcmp(pd->cd->ClassName(),buf))
			{
				return i;
			}

		}

	}
	return CC_NOT_LOADED; // Not loaded

}

/*********************************************************************************************************
*
*	InsertCustAttrib take an index as the first Parameter.  The specifies where to insert the CA. If the
*	the index is greater than the number of Actual CAs then it will not insert.  The sample does not check
*	for this but instead forces it to use the first position.
*
\*********************************************************************************************************/

void DynPBlock::AddObjectCA(BOOL insert)
{
	INode *node;
	HWND hWndList;
	Interface *ip = GetCOREInterface();
	if (ip->GetSelNodeCount() == 1) 
		node = ip->GetSelNode(0); 
	else 	
	{	
		node = NULL; 
		return;
	}
	BaseObject *obj = node->GetObjectRef();

// Lets grap the first modifier in out stack
	if(obj->SuperClassID() == GEN_DERIVOB_CLASS_ID )
	{
		IDerivedObject *pDerObj = (IDerivedObject *) obj;
		obj = pDerObj->GetModifier(0);
	}

	//If there is no modifier we could just bail out here but instead we apply
	//the CA to the base object

//	
	ICustAttribContainer* cc = obj->GetCustAttribContainer();

	if(!cc)
	{
		obj->AllocCustAttribContainer();
		cc = obj->GetCustAttribContainer();
	}
	
//  Is our CA already installed ?

	hWndList = GetDlgItem(hPanel,IDC_CALIST);
	int curSel = ListBox_GetCurSel(hWndList);

	int index = CheckCCIsLoaded(cc,curSel);
	
	if (index == CC_UNKNOWN_CUST_ATTRIB)
	{
		MessageBox(hPanel, strUnknownCAFoundMsg, strWarning, NULL);
		return;
	}
	else if(index >=0)
	{
		MessageBox(hPanel,"CA Already Present, Not Loading", strWarning, NULL);
		return;
	}

// Create the correct CA
	theHold.Begin(); 
	if(curSel ==0){
		SimpleCustAttrib  * ca = new SimpleCustAttrib();
		if(!insert)
			cc->AppendCustAttrib(ca);
		else
			cc->InsertCustAttrib(0,ca);  // Insert at the beginning
	}
	else if(curSel==1){
		SwatchCustAttrib *ca = new SwatchCustAttrib();
		if(!insert)
			cc->AppendCustAttrib(ca);
		else
			cc->InsertCustAttrib(0,ca);  // Insert at the beginning	
	}
	else if(curSel==2){
		NodeCustAttrib *ca = new NodeCustAttrib();
		if(!insert)
			cc->AppendCustAttrib(ca);
		else
			cc->InsertCustAttrib(0,ca);  // Insert at the beginning	
	}
	else{
		SimpleCustAttrib  * ca = new SimpleCustAttrib();
		if(!insert)
			cc->AppendCustAttrib(ca);
		else
			cc->InsertCustAttrib(0,ca);  // Insert at the beginning
	}
	theHold.Accept(_T("Add Custom Attribute"));

	
}

void DynPBlock::Remove()
{
	INode *node;
	HWND hWndList;
	Interface *ip = GetCOREInterface();

	if (ip->GetSelNodeCount() == 1) 
		node = ip->GetSelNode(0); 
	else 	
	{	
		node = NULL; 
		return;
	}
	BaseObject *obj = node->GetObjectRef();
	if(obj->SuperClassID() == GEN_DERIVOB_CLASS_ID )
	{
		IDerivedObject *pDerObj = (IDerivedObject *) obj;
		obj = pDerObj->GetModifier(0);
	}

	ICustAttribContainer* cc = obj->GetCustAttribContainer();
	if(cc!=NULL)
	{
		// Make sure we only remove ourselves and not other CAs
		hWndList = GetDlgItem(hPanel,IDC_CALIST);
		int curSel = ListBox_GetCurSel(hWndList);

		int index = CheckCCIsLoaded(cc,curSel);

		if(index >= 0) {
			theHold.Begin(); 
			cc->RemoveCustAttrib(index);
			theHold.Accept(_T("Remove Custom Attribute"));
		}
		else if (index == CC_NOT_LOADED)
			MessageBox(hPanel,"Nothing to Remove", strWarning, NULL);
		else 	if (index == CC_UNKNOWN_CUST_ATTRIB)
			MessageBox(hPanel,strUnknownCAFoundMsg, strWarning, NULL);
	}
}


void DynPBlock::AddMaterialCA(BOOL Insert)
{
	HWND hWndList;
	// Get an interface to the Material Editior
	IMtlEditInterface *mtlEdit = (IMtlEditInterface *)GetCOREInterface(MTLEDIT_INTERFACE);
	
	if(!mtlEdit)
		return;
	//Grap the current material

	MtlBase *mtl = mtlEdit->GetCurMtl();
	
	if(!mtl)
		return;

	ICustAttribContainer* cc = mtl->GetCustAttribContainer();
	if(!cc)
	{
		mtl->AllocCustAttribContainer();
		cc = mtl->GetCustAttribContainer();
	}
	hWndList = GetDlgItem(hPanel,IDC_CALIST);

	int curSel = ListBox_GetCurSel(hWndList);
	int index = CheckCCIsLoaded(cc,curSel,TRUE);

	if (index == CC_UNKNOWN_CUST_ATTRIB)
	{
		MessageBox(hPanel, strUnknownCAFoundMsg, strWarning, NULL);
		return;
	}
	else if(index >=0)
	{
		MessageBox(hPanel,"CC Already Present, Not Loading", strWarning, NULL);
		return;
	}
	theHold.Begin(); 

	if(!Insert)
		cc->AppendCustAttrib((CustAttrib *)CreateInstance(CUST_ATTRIB_CLASS_ID,MCA_CLASS_ID));
	else
		cc->InsertCustAttrib(0,(CustAttrib *)CreateInstance(CUST_ATTRIB_CLASS_ID,MCA_CLASS_ID));
	
	theHold.Accept(_T("Add Material Custom Attribute"));

}

void DynPBlock::RemoveMat()
{
	HWND hWndList;

	IMtlEditInterface *mtlEdit = (IMtlEditInterface *)GetCOREInterface(MTLEDIT_INTERFACE);
	
	if(!mtlEdit)
		return;
	
	MtlBase *mtl = mtlEdit->GetCurMtl();
	
	if(!mtl)
		return;
	
	ICustAttribContainer* cc = mtl->GetCustAttribContainer();
	if(cc!=NULL)
	{
		hWndList = GetDlgItem(hPanel,IDC_CALIST);
		int curSel = ListBox_GetCurSel(hWndList);

		int index = CheckCCIsLoaded(cc,curSel,TRUE);

		// Make sure we only remove ourselves and not other CAs
		if(index >= 0){
			theHold.Begin(); 
			cc->RemoveCustAttrib(index);
			theHold.Accept(_T("Remove Material Custom Attribute"));
		}
		else if (index == CC_NOT_LOADED)
			MessageBox(hPanel,"Nothing to Remove", strWarning, NULL);
		else if (index == CC_UNKNOWN_CUST_ATTRIB)
			MessageBox(hPanel, strUnknownCAFoundMsg, strWarning, NULL);
		
	}

}


