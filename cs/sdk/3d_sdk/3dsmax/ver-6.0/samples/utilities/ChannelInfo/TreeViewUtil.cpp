/**********************************************************************
 *<
	FILE: TreeViewUtil.cpp

	DESCRIPTION:	Appwizard generated plugin

	CREATED BY: 

	HISTORY: 	
	
  build unit test
    vc color

  add action items to menu

  sometimes copying and pasting a channel does not work
  
  do doc

 *>	Copyright (c) 2003, All Rights Reserved.
 **********************************************************************/

#include "TreeViewUtil.h"

static TreeViewUtil theTreeViewUtil;

#define EDITABLE_SURF_CLASS_ID Class_ID(0x76a11646, 0x12a822fb)


class TreeViewUtilClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return TRUE; }
	void *			Create(BOOL loading = FALSE) { return &theTreeViewUtil; }
	const TCHAR *	ClassName() { return GetString(IDS_CLASS_NAME); }
	SClass_ID		SuperClassID() { return UTILITY_CLASS_ID; }
	Class_ID		ClassID() { return TREEVIEWUTIL_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_CATEGORY); }

	const TCHAR*	InternalName() { return _T("TreeViewUtil"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle

};


static TreeViewUtilClassDesc TreeViewUtilDesc;
ClassDesc2* GetTreeViewUtilDesc() { return &TreeViewUtilDesc; }

//--- Publish bone creation -------------------------------------------------------------------

#define CHANNELINFO_FP_INTERFACE_ID Interface_ID(0x438a1122, 0xef966644)

class ChannelInfoFunctionPublish : public FPStaticInterface {
	public:
		DECLARE_DESCRIPTOR(ChannelInfoFunctionPublish);

		enum OpID {
			kDialog,
			kCopyChannel,kCopySubChannel,
			kPasteChannel,kPasteSubChannel,
			
			kAddChannel,
			kClearChannel,

			kNameChannel,kNameSubChannel,
			kUpdate,
			kGetSubComp, kSetSubComp,
			kGetLock, kSetLock
			
			};
		
		BEGIN_FUNCTION_MAP			
			VFN_0(kDialog,  Dialog)
			VFN_3(kCopyChannel, CopyChannel,TYPE_INODE,TYPE_INT,TYPE_INT)
			VFN_4(kCopySubChannel, CopySubChannel,TYPE_INODE,TYPE_INT,TYPE_INT,TYPE_INT)

			VFN_3(kPasteChannel, PasteChannel,TYPE_INODE,TYPE_INT,TYPE_INT)
			VFN_4(kPasteSubChannel, PasteSubChannel,TYPE_INODE,TYPE_INT,TYPE_INT,TYPE_INT)

			VFN_1(kAddChannel, AddChannel,TYPE_INODE)
			VFN_2(kClearChannel, ClearChannel,TYPE_INODE,TYPE_INT)

			VFN_4(kNameChannel, NameChannel,TYPE_INODE,TYPE_INT,TYPE_INT,TYPE_STRING)
			VFN_5(kNameSubChannel, NameSubChannel,TYPE_INODE,TYPE_INT,TYPE_INT,TYPE_INT,TYPE_STRING)
			
			VFN_0(kUpdate,  Update)
			
			PROP_FNS(kGetSubComp, GetSubComp, kSetSubComp, SetSubComp,TYPE_BOOL);
			PROP_FNS(kGetLock, GetLock, kSetLock, SetLock,TYPE_BOOL);
			

		END_FUNCTION_MAP

		
		void Dialog();
		void CopyChannel(INode *node, int channelType, int channel);
		void CopySubChannel(INode *node, int channelType, int channel, int subChannel);

		void PasteChannel(INode *node, int channelType, int channel);
		void PasteSubChannel(INode *node, int channelType, int channel, int subChannel);

		void ClearChannel(INode *node, int mapID);
		void AddChannel(INode *node);

		void NameChannel(INode *node, int channelType, int channel, TCHAR *name);
		void NameSubChannel(INode *node, int channelType, int channel, int subChannel, TCHAR *name);
		void Update();
		
		void SetSubComp(BOOL subComp);
		BOOL GetSubComp();

		void SetLock(BOOL lock);
		BOOL GetLock();

	};

static ChannelInfoFunctionPublish theChannelInfoFunctionPublish(
	CHANNELINFO_FP_INTERFACE_ID, _T("ChannelInfo"), -1, 0, FP_CORE,
	// The first operation, boneCreate:
	ChannelInfoFunctionPublish::kDialog, _T("dialog"), -1, TYPE_VOID, 0, 0,
	ChannelInfoFunctionPublish::kCopyChannel, _T("CopyChannel"), -1, TYPE_VOID, 0, 3,
		_T("node"), -1, TYPE_INODE,
		_T("channelType"), -1, TYPE_INT,
		_T("channel"), -1, TYPE_INT,
	ChannelInfoFunctionPublish::kCopySubChannel, _T("CopySubChannel"), -1, TYPE_VOID, 0, 4,
		_T("node"), -1, TYPE_INODE,
		_T("channelType"), -1, TYPE_INT,
		_T("channel"), -1, TYPE_INT,
		_T("subchannel"), -1, TYPE_INT,

	ChannelInfoFunctionPublish::kPasteChannel, _T("PasteChannel"), -1, TYPE_VOID, 0, 3,
		_T("node"), -1, TYPE_INODE,
		_T("channelType"), -1, TYPE_INT,
		_T("channel"), -1, TYPE_INT,
	ChannelInfoFunctionPublish::kPasteSubChannel, _T("PasteSubChannel"), -1, TYPE_VOID, 0, 4,
		_T("node"), -1, TYPE_INODE,
		_T("channelType"), -1, TYPE_INT,
		_T("channel"), -1, TYPE_INT,
		_T("subchannel"), -1, TYPE_INT,

	ChannelInfoFunctionPublish::kAddChannel, _T("AddChannel"), -1, TYPE_VOID, 0, 1,
		_T("node"), -1, TYPE_INODE,

	ChannelInfoFunctionPublish::kClearChannel, _T("ClearChannel"), -1, TYPE_VOID, 0, 2,
		_T("node"), -1, TYPE_INODE,
		_T("channel"), -1, TYPE_INT,


	ChannelInfoFunctionPublish::kNameChannel, _T("NameChannel"), -1, TYPE_VOID, 0, 4,
		_T("node"), -1, TYPE_INODE,
		_T("channelType"), -1, TYPE_INT,
		_T("channel"), -1, TYPE_INT,
		_T("name"), -1, TYPE_STRING,
		
	ChannelInfoFunctionPublish::kNameSubChannel, _T("NameSubChannel"), -1, TYPE_VOID, 0, 5,
		_T("node"), -1, TYPE_INODE,
		_T("channelType"), -1, TYPE_INT,
		_T("channel"), -1, TYPE_INT,
		_T("subchannel"), -1, TYPE_INT,
		_T("name"), -1, TYPE_STRING,

	ChannelInfoFunctionPublish::kUpdate, _T("update"), -1, TYPE_VOID, 0, 0,


	properties,

	ChannelInfoFunctionPublish::kGetSubComp, ChannelInfoFunctionPublish::kSetSubComp, _T("subComp"), -1, TYPE_BOOL,
	ChannelInfoFunctionPublish::kGetLock, ChannelInfoFunctionPublish::kSetLock, _T("lock"), -1, TYPE_BOOL,

	end);



void ChannelInfoFunctionPublish::Dialog()
{
	theTreeViewUtil.CreateNewFloater();
}

void ChannelInfoFunctionPublish::CopyChannel(INode *node, int channelType, int channel)
{
	channelType = channelType + 9;
	theTreeViewUtil.CopyToBuffer(node, channelType, channel, -50);

}

void ChannelInfoFunctionPublish::CopySubChannel(INode *node, int channelType, int channel, int subChannel)
{
	channelType = channelType + 9;
	theTreeViewUtil.CopyToBuffer(node, channelType, channel, subChannel);
}

void ChannelInfoFunctionPublish::PasteChannel(INode *node, int channelType, int channel)
{
	channelType = channelType + 9;
	theTreeViewUtil.PasteToNode(node, channelType, channel, -50);
//	theTreeViewUtil.CopyToBuffer(node, channelType, channel, -50);

}

void ChannelInfoFunctionPublish::PasteSubChannel(INode *node, int channelType, int channel, int subChannel)
{
    channelType = channelType + 9;
	theTreeViewUtil.PasteToNode(node, channelType, channel, subChannel);
//	theTreeViewUtil.CopyToBuffer(node, channelType, channel, subChannel);
}

void ChannelInfoFunctionPublish::ClearChannel(INode *node, int mapID)
{
	theTreeViewUtil.DeleteChannel(node,mapID);
}
void ChannelInfoFunctionPublish::AddChannel(INode *node)
{
	theTreeViewUtil.AddChannel(node);
}

void ChannelInfoFunctionPublish::NameChannel(INode *node, int channelType, int channel, TCHAR *name)
{
    channelType = channelType + 9;
	theTreeViewUtil.NameChannel(node, channelType, channel, -50, name);
}
void ChannelInfoFunctionPublish::NameSubChannel(INode *node, int channelType, int channel, int subChannel, TCHAR *name)
{
    channelType = channelType + 9;
	theTreeViewUtil.NameChannel(node, channelType, channel, subChannel, name);

}

void ChannelInfoFunctionPublish::Update()
{
	theTreeViewUtil.AddSelectedNodesToList();
	theTreeViewUtil.AddListViewItems();
}

void ChannelInfoFunctionPublish::SetSubComp(BOOL subComp)
{
	theTreeViewUtil.subComponents = subComp;
	if (theTreeViewUtil.iSubCompButton)
		{
		theTreeViewUtil.iSubCompButton->SetCheck(subComp);
		theTreeViewUtil.UpdateViewItems();
		theTreeViewUtil.AddListViewItems();
		}

}
BOOL ChannelInfoFunctionPublish::GetSubComp()
{
	return theTreeViewUtil.subComponents;
}

void ChannelInfoFunctionPublish::SetLock(BOOL lock)
{
	theTreeViewUtil.lockSelection = lock;
	if (theTreeViewUtil.iLockButton)
		theTreeViewUtil.iLockButton->SetCheck(lock);
	if ((!theTreeViewUtil.lockSelection) && (theTreeViewUtil.iLockButton))
		{
		theTreeViewUtil.AddSelectedNodesToList();
		theTreeViewUtil.AddListViewItems();
		}		

}
BOOL ChannelInfoFunctionPublish::GetLock()
{
	return theTreeViewUtil.lockSelection;
}



void UpdateUIRestore::Restore(int isUndo)

{
	if (theTreeViewUtil.floaterHWND != NULL)
	{
	theTreeViewUtil.UpdateViewItems();
	theTreeViewUtil.AddListViewItems();
	theTreeViewUtil.UpdateUI();
	}
}

void CopyBuffer::Clear()
	{
		uvwPatchFaces.ZeroCount();
		patchDeg.ZeroCount();
		
		w.ZeroCount();
		verts.ZeroCount();
		uvwFaces.ZeroCount();
		geomFaces.ZeroCount();

		for (int i =0; i < uvwMNFaces.Count(); i++)
		{
			if (uvwMNFaces[i])
			{			
				uvwMNFaces[i]->Clear();
				delete uvwMNFaces[i];
			}
		}

		uvwMNFaces.ZeroCount();
		
		
		for (i =0; i < geomMNFaces.Count(); i++)
		{
			if (geomMNFaces[i])
			{			
				geomMNFaces[i]->Clear();
				delete geomMNFaces[i];
			}
		}
		geomMNFaces.ZeroCount();
		mnVerts.ZeroCount();


	}


void  CopyBuffer::Copy(CopyBuffer *from)
	{
		copyType = from->copyType;
		subID = from->subID;
		numFaces = from->numFaces;
		
		numRealFaces = from->numRealFaces;

		w = from->w;

		verts = from->verts;
		mnVerts = from->mnVerts;

		uvwFaces = from->uvwFaces;
		geomFaces = from->geomFaces;
		
		patchDeg = from->patchDeg;
		uvwPatchFaces = from->uvwPatchFaces;
		

		SetGeomMNFaceCount(from->geomMNFaces.Count());
		SetMapMNFaceCount(from->uvwMNFaces.Count());

		for (int i = 0; i < from->geomMNFaces.Count(); i++)
		{
			geomMNFaces[i]->SetDeg(from->geomMNFaces[i]->deg);
			*geomMNFaces[i] = *from->geomMNFaces[i];
		}

		for (i = 0; i < from->uvwMNFaces.Count(); i++)
		{
			uvwMNFaces[i]->SetSize(from->uvwMNFaces[i]->deg);
			*uvwMNFaces[i] = *from->uvwMNFaces[i];
		}




		pasteToChannelType = from->pasteToChannelType;
		pasteToChannel = from->pasteToChannel;
		pasteToSubID = from->pasteToSubID;

		name.printf("%s",from->name);

		mFalloff = from->mFalloff ;
		mPinch = from->mPinch;
		mBubble = from->mBubble;
		mEdgeDist = from->mEdgeDist;
		mUseEdgeDists = from->mUseEdgeDists;
		mAffectBackface = from->mAffectBackface;
		mUseSoftSelections = from->mUseSoftSelections;

	}


#define PASTE_DATA		0x2000
#define COPY_DATA		0x2010

#define W_DATA			0x2020
#define VERT_DATA		0x2030

#define UVWFACE_DATA	0x2040
#define GEOMFACE_DATA	0x2050

#define POLYGEOMVERT_DATA	0x2060

#define POLYUVWFACE_DATA	0x2070
#define POLYGEOMFACE_COUNT	0x2080
#define POLYGEOMFACE_DATA	0x2090

#define REALFACECOUNT_DATA	0x2100

#define PATCHUVWFACE_DATA	0x2110
#define PATCHGEOMFACE_DATA	0x2120


IOResult CopyBuffer::Save(ISave *isave)
{
	IOResult	res;
	ULONG		nb;

//save the copy data
	isave->BeginChunk(COPY_DATA);
	res = isave->Write(&copyType, sizeof(copyType), &nb);
	res = isave->Write(&subID, sizeof(subID), &nb);
	res = isave->Write(&numFaces, sizeof(numFaces), &nb);
	isave->EndChunk();

	//save the paste data

	isave->BeginChunk(PASTE_DATA);
	res = isave->Write(&pasteToChannelType, sizeof(pasteToChannelType), &nb);
	res = isave->Write(&pasteToChannel, sizeof(pasteToChannel), &nb);
	res = isave->Write(&pasteToSubID, sizeof(pasteToSubID), &nb);
	isave->EndChunk();

	//save the num verts
	//save the verts
	isave->BeginChunk(VERT_DATA);
	int numVerts = verts.Count();
	res = isave->Write(&numVerts, sizeof(numVerts), &nb);
	if (numVerts > 0)
		res = isave->Write(verts.Addr(0), sizeof(Point3)*numVerts, &nb);
	isave->EndChunk();

	//save the num w
	//save the w
	isave->BeginChunk(W_DATA);
	numVerts = w.Count();
	res = isave->Write(&numVerts, sizeof(numVerts), &nb);
	if (numVerts > 0)
		res = isave->Write(w.Addr(0), sizeof(float)*numVerts, &nb);
	isave->EndChunk();

	
	//save the uvwFaces
	isave->BeginChunk(UVWFACE_DATA);
	int numFaces = uvwFaces.Count();
	res = isave->Write(&numFaces, sizeof(numFaces), &nb);
	if (numFaces > 0)
		res = isave->Write(uvwFaces.Addr(0), sizeof(TVFace)*numFaces, &nb);
	isave->EndChunk();

	//save the geomFaces
	isave->BeginChunk(GEOMFACE_DATA);
	numFaces = geomFaces.Count();
	res = isave->Write(&numFaces, sizeof(numFaces), &nb);
	if (numFaces > 0)
		res = isave->Write(geomFaces.Addr(0), sizeof(Face)*numFaces, &nb);
	isave->EndChunk();

	isave->BeginChunk(POLYGEOMVERT_DATA);
	int numMNVerts = mnVerts.Count();
	res = isave->Write(&numMNVerts, sizeof(numMNVerts), &nb);
	if (numMNVerts > 0)
		res = isave->Write(mnVerts.Addr(0), sizeof(MNVert)*numMNVerts, &nb);
	isave->EndChunk();

	isave->BeginChunk(POLYUVWFACE_DATA);
	int numMNFaces = uvwMNFaces.Count();
	res = isave->Write(&numMNFaces, sizeof(numMNFaces), &nb);
	if (numMNFaces > 0)
	{
		for (int i = 0; i < numMNFaces; i++)
		{
			int deg = uvwMNFaces[i]->deg;
			res = isave->Write(&deg, sizeof(deg), &nb);
			res = isave->Write(uvwMNFaces[i]->tv, sizeof(int)*deg, &nb);
		}
		
	}
	isave->EndChunk();

	isave->BeginChunk(POLYGEOMFACE_COUNT);
	numMNFaces = geomMNFaces.Count();
	res = isave->Write(&numMNFaces, sizeof(numMNFaces), &nb);
	isave->EndChunk();

	for (int i = 0; i < numMNFaces; i++)
	{
		isave->BeginChunk(POLYGEOMFACE_DATA);
		geomMNFaces[i]->Save(isave);
		isave->EndChunk();
	}


	isave->BeginChunk(REALFACECOUNT_DATA	);
	res = isave->Write(&numRealFaces, sizeof(numRealFaces), &nb);
	isave->EndChunk();
	

	isave->BeginChunk(PATCHUVWFACE_DATA);
	int numUVWPatchFaces = uvwPatchFaces.Count();
	res = isave->Write(&numUVWPatchFaces, sizeof(numUVWPatchFaces), &nb);
	if (numUVWPatchFaces > 0)
		res = isave->Write(uvwPatchFaces.Addr(0), sizeof(TVPatch)*numUVWPatchFaces, &nb);
	isave->EndChunk();
	


	
	return IO_OK;


}
IOResult CopyBuffer::Load(ILoad *iload)
{

	//TODO: Add code to allow plugin to load its data
	IOResult	res;
	ULONG		nb;

	int currentMNGeomFace = 0;


	while (IO_OK==(res=iload->OpenChunk())) 
		{
		switch(iload->CurChunkID())  
			{
			case COPY_DATA:
				{
				res = iload->Read(&copyType,sizeof(copyType), &nb);
				res = iload->Read(&subID,sizeof(subID), &nb);
				res = iload->Read(&numFaces,sizeof(numFaces), &nb);
				break;
				}
			case PASTE_DATA:
				{
				res = iload->Read(&pasteToChannelType,sizeof(pasteToChannelType), &nb);
				res = iload->Read(&pasteToChannel,sizeof(pasteToChannel), &nb);
				res = iload->Read(&pasteToSubID,sizeof(pasteToSubID), &nb);
				break;
				}

			case VERT_DATA:
				{
				int numVerts;
				res = iload->Read(&numVerts,sizeof(numVerts), &nb);
				verts.SetCount(numVerts);
				if (numVerts > 0)
					res = iload->Read(verts.Addr(0),sizeof(Point3)*numVerts, &nb);
				break;
				}
			case W_DATA:
				{
				int numVerts;
				res = iload->Read(&numVerts,sizeof(numVerts), &nb);
				w.SetCount(numVerts);
				if (numVerts > 0)
					res = iload->Read(w.Addr(0),sizeof(float)*numVerts, &nb);
				break;
				}

			case UVWFACE_DATA:
				{
				int numFaces;
				res = iload->Read(&numFaces,sizeof(numFaces), &nb);
				uvwFaces.SetCount(numFaces);
				if (numFaces > 0)
					res = iload->Read(uvwFaces.Addr(0),sizeof(TVFace)*numFaces, &nb);
				break;
				}

			case GEOMFACE_DATA:
				{
				int numFaces;
				res = iload->Read(&numFaces,sizeof(numFaces), &nb);
				geomFaces.SetCount(numFaces);

				if (numFaces > 0)
					res = iload->Read(geomFaces.Addr(0),sizeof(Face)*numFaces, &nb);
				break;
				}

			case POLYGEOMVERT_DATA:
				{
				int numVerts;
				res = iload->Read(&numVerts,sizeof(numVerts), &nb);
				mnVerts.SetCount(numVerts);

				if (numVerts > 0)
					res = iload->Read(mnVerts.Addr(0),sizeof(MNVert)*numVerts, &nb);
				break;
				}

			case POLYUVWFACE_DATA:
				{
				int numFaces;
				res = iload->Read(&numFaces,sizeof(numFaces), &nb);
				uvwMNFaces.SetCount(numFaces);

				if (numFaces > 0)
				{
					for (int i =0; i < numFaces; i++)
					{
						int deg;
						uvwMNFaces[i] = new MNMapFace();
						res = iload->Read(&deg,sizeof(deg), &nb);
						uvwMNFaces[i]->SetSize(deg);
						uvwMNFaces[i]->deg = deg;						
						res = iload->Read(uvwMNFaces[i]->tv,sizeof(int)*deg, &nb);
					}
				}
				break;
				}
			case POLYGEOMFACE_COUNT:
				{
				int numFaces;
				res = iload->Read(&numFaces,sizeof(numFaces), &nb);
				geomMNFaces.SetCount(numFaces);
				break;
				}

			case POLYGEOMFACE_DATA:
				{
				if (currentMNGeomFace < geomMNFaces.Count())
				{
					geomMNFaces[currentMNGeomFace] = new MNFace();
					res = geomMNFaces[currentMNGeomFace]->Load(iload);
				}
				currentMNGeomFace++;
				break;
				}
			case REALFACECOUNT_DATA:
				{
				res = iload->Read(&numRealFaces,sizeof(numRealFaces), &nb);
				break;
				}

			case PATCHUVWFACE_DATA:
				{
				int numUVWPatchFaces;
				res = iload->Read(&numUVWPatchFaces,sizeof(numUVWPatchFaces), &nb);
				uvwPatchFaces.SetCount(numUVWPatchFaces);

				if (numUVWPatchFaces > 0)
					res = iload->Read(uvwPatchFaces.Addr(0),sizeof(TVPatch)*numUVWPatchFaces, &nb);
				break;
				}


			}
		iload->CloseChunk();
		if (res!=IO_OK) return res;
		}

	return IO_OK;

}






static BOOL CALLBACK TreeViewUtilDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
		case WM_INITDIALOG:
			theTreeViewUtil.Init(hWnd);
			
			break;

		case WM_DESTROY:
			theTreeViewUtil.Destroy(hWnd);
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{
				case IDC_BUTTON1:
				theTreeViewUtil.CreateNewFloater();
				macroRecorder->FunctionCall(_T("channelInfo.Dialog"), 0, 0);

//				theTreeViewUtil.CreateNewFloater();
				break;

				case IDOK:
				case IDCANCEL:
					DestroyWindow(hWnd);
					break;
			}
			break;

			break;


		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			theTreeViewUtil.ip->RollupMouseMessage(hWnd,msg,wParam,lParam); 
			break;

		default:
			return FALSE;
	}
	return TRUE;
}




static INT_PTR CALLBACK ViewChannelsFloaterDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	switch (msg) {
		case WM_INITDIALOG: 
			{

			theTreeViewUtil.floaterHWND = hWnd;
			
			GetCOREInterface()->RegisterDlgWnd(hWnd);
			theTreeViewUtil.listViewHWND = GetDlgItem(hWnd,IDC_MAPLIST);

			theTreeViewUtil.InitWindow();

			theTreeViewUtil.UpdateUI();
			break;
			}

        case WM_SIZE:
			{
			HWND hWndTreeView = GetDlgItem(hWnd,IDC_MAPLIST);
            MoveWindow(hWndTreeView, 0, 45, LOWORD(lParam), HIWORD(lParam)-45, TRUE);


            break;
			}


		
		case WM_DESTROY: 
			{
			theTreeViewUtil.DestroyWindow();
			GetCOREInterface()->UnRegisterDlgWnd(hWnd);
			break;
			}

		case WM_RBUTTONDOWN:
			{
				POINT lpPt; 
				GetCursorPos(&lpPt);
				theTreeViewUtil.ShowRMenu(hWnd,lpPt.x,lpPt.y);
			}
			break;

		case WM_NOTIFY:
			return( theTreeViewUtil.NotifyHandler(hWnd, msg, wParam, lParam));
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{
				case IDC_LOCK:
				{
					theTreeViewUtil.lockSelection = theTreeViewUtil.iLockButton->IsChecked();
					
					TSTR subString;
					if (theTreeViewUtil.lockSelection)
						subString.printf("channelInfo.lock = true");
					else subString.printf("channelInfo.lock = false");	
					
					macroRecorder->ScriptString(subString);
					macroRecorder->EmitScript();									
					
					if (!theTreeViewUtil.lockSelection)
					{
						theTreeViewUtil.AddSelectedNodesToList();
						theTreeViewUtil.AddListViewItems();
					}
				}
				break;

				case IDC_COLLAPSE:
					theTreeViewUtil.Collapse();// = theTreeViewUtil.iCollapseButton->IsChecked();
					break;

				case IDC_SUBCOMPONENTS:
				{
					theTreeViewUtil.subComponents = theTreeViewUtil.iSubCompButton->IsChecked();
					
					TSTR subString;
					if (theTreeViewUtil.subComponents)
						subString.printf("channelInfo.subComp = true");
					else subString.printf("channelInfo.subComp = false");
					macroRecorder->ScriptString(subString);
					macroRecorder->EmitScript();
					
					
					theTreeViewUtil.UpdateViewItems();
					theTreeViewUtil.AddListViewItems();
				}
				break;


				case ID_MAPINFO_CLEAR:
				case IDC_CLEAR:
				{
					//get the selection
					int sel =  ListView_GetSelectionMark( theTreeViewUtil.listViewHWND);
					//see if valid entry
					if ((sel >=0) && (sel < theTreeViewUtil.nodeList.Count()))
					{
					//delete it
						macroRecorder->FunctionCall(_T("channelInfo.ClearChannel"), 2, 0,
														mr_reftarg,theTreeViewUtil.nodeList[sel]->node,
														mr_int,theTreeViewUtil.nodeList[sel]->channelID);
						macroRecorder->EmitScript();

						theTreeViewUtil.DeleteChannel(theTreeViewUtil.nodeList[sel]->node,theTreeViewUtil.nodeList[sel]->channelID);
						theTreeViewUtil.UpdateViewItems();
						theTreeViewUtil.AddListViewItems();
						theTreeViewUtil.UpdateUI();

						if (sel >= theTreeViewUtil.nodeList.Count())
							sel = theTreeViewUtil.nodeList.Count()-1;
						ListView_SetItemState (theTreeViewUtil.listViewHWND,         // handle to listview
											sel,         // index to listview item
											LVIS_FOCUSED | LVIS_SELECTED, // item state
											0x000F);                      // mask



					}
				}
				break;

				case ID_MAPINFO_COPY:
				case IDC_COPY:
				{
					int sel = theTreeViewUtil.GetSel();
					theTreeViewUtil.CopyToBuffer(sel);
					theTreeViewUtil.UpdateUI();
					break;
				}

				case ID_MAPINFO_PASTE:
				case IDC_PASTE:
				{
					int sel = theTreeViewUtil.GetSel();
					theTreeViewUtil.PasteToNode(sel);

					theTreeViewUtil.UpdateViewItems();
					theTreeViewUtil.AddListViewItems();
					theTreeViewUtil.UpdateUI();
					
					ListView_SetItemState (theTreeViewUtil.listViewHWND,         // handle to listview
											sel,         // index to listview item
											LVIS_FOCUSED | LVIS_SELECTED, // item state
											0x000F);                      // mask
					
					
					break;
				}

				case ID_MAPINFO_ADD:
				case IDC_ADD:
				{
					int sel = theTreeViewUtil.GetSel();
					theTreeViewUtil.AddChannel(sel);

					theTreeViewUtil.UpdateViewItems();
					theTreeViewUtil.AddListViewItems();
					theTreeViewUtil.UpdateUI();
					ListView_SetItemState (theTreeViewUtil.listViewHWND,         // handle to listview
											sel,         // index to listview item
											LVIS_FOCUSED | LVIS_SELECTED, // item state
											0x000F);                      // mask
					
					break;
				}


				case ID_MAPINFO_RENAME:
				{
					theTreeViewUtil.NameChannel();
					break;
				}
				case IDC_NAME:
				{
					theTreeViewUtil.NameChannel();
					SetFocus(theTreeViewUtil.floaterHWND);
					break;
				}

				case ID_MAPINFO_UPDATE:
				case IDC_UPDATE:
				{
						theTreeViewUtil.AddSelectedNodesToList();
						theTreeViewUtil.AddListViewItems();
						macroRecorder->FunctionCall(_T("channelInfo.Update"), 0, 0);						
				}
				break;

				case IDC_BUTTON1:
				break;

				case IDOK:
				case IDCANCEL:
					theTreeViewUtil.floaterHWND = NULL;
					DestroyWindow(hWnd);
				break;
			}
			break;

		default:
			return FALSE;
		}

	return TRUE;
	}



//--- TreeViewUtil -------------------------------------------------------
TreeViewUtil::TreeViewUtil()
{
	iu = NULL;
	ip = NULL;	
	hPanel = NULL;
	floaterHWND = NULL;
	subComponents = FALSE;
	collapse = FALSE;
	lockSelection = FALSE;
}

TreeViewUtil::~TreeViewUtil()
{

}

void TreeViewUtil::BeginEditParams(Interface *ip,IUtil *iu) 
{
	this->iu = iu;
	this->ip = ip;
	hPanel = ip->AddRollupPage(
		hInstance,
		MAKEINTRESOURCE(IDD_PANEL),
		TreeViewUtilDlgProc,
		GetString(IDS_PARAMS),
		0);


}
	
void TreeViewUtil::EndEditParams(Interface *ip,IUtil *iu) 
{
	this->iu = NULL;
	this->ip = NULL;
	ip->DeleteRollupPage(hPanel);
	hPanel = NULL;
}

void TreeViewUtil::Init(HWND hWnd)
{

}

void TreeViewUtil::Destroy(HWND hWnd)
{

}


void TreeViewUtil::CreateNewFloater()
	{
	if (floaterHWND == NULL)
		CreateDialog(
			hInstance,
			MAKEINTRESOURCE(IDD_COLORCLIP_FLOATER),
			GetCOREInterface()->GetMAXHWnd(),
			ViewChannelsFloaterDlgProc);
}


int TreeViewUtil::GetDeadGeomVerts(Mesh *msh)
{
	int ct = 0;
	BitArray numberSet;
	numberSet.SetSize(msh->numVerts);
	numberSet.ClearAll();

	for (int i = 0; i < msh->numFaces; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			int index = msh->faces[i].v[j];
			if (index < msh->numVerts)
				numberSet.Set(index);
		}
	}
	return msh->numVerts - numberSet.NumberSet();
}

int TreeViewUtil::GetDeadGeomVerts(MNMesh *msh)
{
//	int ct = 0;
	BitArray numberSet;
	numberSet.SetSize(msh->numv);
	numberSet.ClearAll();

	for (int i = 0; i < msh->numf; i++)
	{
//		MNFace f = msh->f[i];

		int deg = msh->f[i].deg;
		
		for (int j = 0; j < deg; j++)
		{
			int index = msh->f[i].vtx[j];
			numberSet.Set(index);
		}
	}
	int ct = msh->numv - numberSet.NumberSet();
	return ct;
}

int TreeViewUtil::GetDeadMapVerts(Mesh *msh, int mp)
{
	int ct = 0;

	if (!msh->mapSupport(mp)) return 0;

	int numberMapVerts = msh->getNumMapVerts(mp);

	BitArray numberSet;
	numberSet.SetSize(numberMapVerts);
	numberSet.ClearAll();

	TVFace *face = msh->mapFaces(mp);

	if (face)
	{
		for (int i = 0; i < msh->numFaces; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				int index = face[i].t[j];
				numberSet.Set(index);
			}
		}
		return numberMapVerts - numberSet.NumberSet();
	}
	return 0;

}

int TreeViewUtil::GetDeadMapVerts(MNMesh *msh, int mp)
{
	int ct = 0;

	MNMap *mnMap = msh->M(mp);

	if (mnMap == NULL) return 0;

//	if (!msh->mapSupport(mp)) return 0;

	int numberMapVerts = mnMap->numv;

	if (numberMapVerts == 0) return 0;

	BitArray numberSet;
	numberSet.SetSize(numberMapVerts);
	numberSet.ClearAll();

	MNMapFace *face = mnMap->f;

	if (face)
	{
		for (int i = 0; i < mnMap->numf; i++)
		{
			int deg = face[i].deg;
			for (int j = 0; j < deg; j++)
			{
				int index = face[i].tv[j];
				numberSet.Set(index);
			}
		}
		return numberMapVerts - numberSet.NumberSet();
	}
	return 0;

}


int TreeViewUtil::GetDeadGeomVerts(PatchMesh *msh)
{
	
	int ct = 0;
	BitArray numberVertSet;
	numberVertSet.SetSize(msh->numVerts);
	numberVertSet.ClearAll();

	BitArray numberVecSet;
	numberVecSet.SetSize(msh->numVecs);
	numberVecSet.ClearAll();

	for (int i = 0; i < msh->numPatches; i++)
	{
		int deg = 3;
		if (msh->patches[i].type == PATCH_QUAD) deg = 4;

		for (int j = 0; j < deg; j++)
		{
			int index = msh->patches[i].v[j];
			numberVertSet.Set(index);
			index =  msh->patches[i].interior[j];
			numberVecSet.Set(index);

			index =  msh->patches[i].vec[j*2];
			numberVecSet.Set(index);

			index =  msh->patches[i].vec[j*2+1];
			numberVecSet.Set(index);

		}
	}
	return (msh->numVerts + msh->numVecs) - (numberVertSet.NumberSet()+numberVecSet.NumberSet());
}

int TreeViewUtil::GetDeadMapVerts(PatchMesh *msh, int mp)
{

	int ct = 0;




	if (!msh->getMapSupport(mp)) return 0;

	int numberMapVerts = msh->getNumMapVerts(mp);

	if (numberMapVerts == 0) return 0;

	BitArray numberSet;
	numberSet.SetSize(numberMapVerts);
	numberSet.ClearAll();




//	if (face)
	{
		

		for (int i = 0; i < msh->numPatches; i++)
		{
			TVPatch face = msh->getMapPatch(mp, i);

			int deg = 3;
			if (msh->patches[i].type == PATCH_QUAD) deg = 4;


			for (int j = 0; j < deg; j++)
			{
				int index = face.tv[j];
				numberSet.Set(index);

				index =  face.interiors[j];
				if (index != -1)
					numberSet.Set(index);

				index =  face.handles[j*2];
				if (index != -1)
					numberSet.Set(index);

				index =  face.handles[j*2+1];
				if (index != -1)
					numberSet.Set(index);

			}
		}
		return numberMapVerts - numberSet.NumberSet();
	}
	
	return 0;

}

void TreeViewUtil::AddPatch(PatchMesh *msh, INode *node)
{
			//add the geom channel
			UVWData *data = new UVWData();

			int deadVerts = GetDeadGeomVerts(msh);

		//get the number of maps
			int numberMapChannels = msh->getNumMaps();

			for (int j = -2; j < numberMapChannels; j++)
			{
				UVWData *data = new UVWData();

				int deadVerts = GetDeadMapVerts(msh,j);

				if (!theTreeViewUtil.subComponents)
				{

					data->node = node;
					data->channelID = j;
					data->channelType = PATCHMAPCHANNEL;

					data->numFaces = msh->numPatches;
					data->numRealFaces = msh->numPatches;
					data->numVerts = msh->getNumMapVerts(j);
					data->kbsize = (sizeof(TVPatch) * data->numFaces + sizeof(Point3) * data->numVerts)/1000;

					data->channelName.printf(GetString(IDS_NONE));
					data->subID = -50;
					data->numOfDeadVerts = deadVerts;

					nodeList.Append(1,&data,50);
				}

				if (theTreeViewUtil.subComponents)
				{
					for (int m = 0; m < 3; m++)
					{
						UVWData *data = new UVWData();
						data->node = node;
						data->channelID = j;
						data->channelType = PATCHMAPCHANNEL;

						data->numFaces = msh->numPatches;
						data->numRealFaces = msh->numPatches;
						data->numVerts = msh->getNumMapVerts(j);
						data->kbsize = ( sizeof(float) * data->numVerts)/1000;
						data->subID = m;
						data->numOfDeadVerts = deadVerts;
						data->channelName.printf(GetString(IDS_NONE));
						nodeList.Append(1,&data,50);

					}

				}			
			}


}


void TreeViewUtil::AddPoly(MNMesh *msh, INode *node)
{
	
			//add the geom channel
			UVWData *data = new UVWData();

			int deadVerts = GetDeadGeomVerts(msh);

			if (!theTreeViewUtil.subComponents)
			{
				data->node = node;
				data->channelID = 0;
				data->subID = 0;
				data->channelType = POLYGEOMCHANNEL;

				data->numRealFaces = msh->numf;
				data->numFaces = msh->numf;
				data->numVerts = msh->numv;
				data->numOfDeadVerts = deadVerts;
				data->kbsize = (sizeof(MNFace) * data->numFaces + sizeof(MNVert) * data->numVerts)/1000;
				data->subID = -50;

				data->channelName.printf(GetString(IDS_NONE));

				nodeList.Append(1,&data,50);
			}

			if (theTreeViewUtil.subComponents)
			{
				for (int m = 0; m < 3; m++)
				{
					UVWData *data = new UVWData();
					data->node = node;
					data->channelID = 0;
					data->subID = m;
					data->channelType = POLYGEOMCHANNEL;

					data->numRealFaces = msh->numf;
					data->numFaces = msh->numf;
					data->numVerts = msh->numv;
					data->numOfDeadVerts = deadVerts;
					data->kbsize =  (sizeof(float) * data->numVerts)/1000;
					data->channelName.printf(GetString(IDS_NONE));
					nodeList.Append(1,&data,50);

				}

			}

			//add the sel channel
			data = new UVWData();
			data->node = node;
			data->channelID = 0;
			data->subID = 0;
			data->channelType = POLYSELCHANNEL;

			data->numRealFaces = msh->numf;
			data->numFaces = msh->numf;
			data->numVerts = msh->numv;
			data->numOfDeadVerts = 0;
			data->kbsize = ( sizeof(float) * data->numVerts)/1000;

			data->channelName.printf(GetString(IDS_NONE));

			nodeList.Append(1,&data,50);



		//get the number of maps
			int numberMapChannels = msh->numm;

			for (int j = -2; j < numberMapChannels; j++)
			{
				UVWData *data = new UVWData();

				MNMap *mnMap = msh->M(j);

				int nVerts = 0;
				if (mnMap)
					nVerts = mnMap->numv;

				int deadVerts = GetDeadMapVerts(msh,j);

				if (!theTreeViewUtil.subComponents)
				{

					data->node = node;
					data->channelID = j;
					data->channelType = POLYMAPCHANNEL;

					data->numRealFaces = msh->numf;
					data->numFaces = msh->M(j)->numf;
					data->numVerts = nVerts;
					data->kbsize = (sizeof(MNMapFace) * data->numFaces + sizeof(Point3) * data->numVerts)/1000;

					data->channelName.printf(GetString(IDS_NONE));
					data->subID = -50;
					data->numOfDeadVerts = deadVerts;

					nodeList.Append(1,&data,50);
				}

				if (theTreeViewUtil.subComponents)
				{
					for (int m = 0; m < 3; m++)
					{
						UVWData *data = new UVWData();
						data->node = node;
						data->channelID = j;
						data->channelType = POLYMAPCHANNEL;

						data->numRealFaces = msh->numf;
						data->numFaces = msh->M(j)->numf;
						data->numVerts = nVerts;
						data->kbsize = ( sizeof(float) * data->numVerts)/1000;
						data->subID = m;
						data->numOfDeadVerts = deadVerts;
						data->channelName.printf(GetString(IDS_NONE));
						nodeList.Append(1,&data,50);

					}

				}			
			}
			

}

void TreeViewUtil::AddMesh(Mesh *msh, INode *node)
{
			//add the geom channel
			UVWData *data = new UVWData();

			int deadVerts = GetDeadGeomVerts(msh);

			if (!theTreeViewUtil.subComponents)
			{
				data->node = node;
				data->channelID = 0;
				data->subID = 0;
				data->channelType = TRIGEOMCHANNEL;

				data->numFaces = msh->numFaces;
				data->numRealFaces = msh->numFaces;
				data->numVerts = msh->numVerts;
				data->numOfDeadVerts = deadVerts;
				data->kbsize = (sizeof(Face) * data->numFaces + sizeof(Point3) * data->numVerts)/1000;
				data->subID = -50;

				data->channelName.printf(GetString(IDS_NONE));

				nodeList.Append(1,&data,50);
			}

			if (theTreeViewUtil.subComponents)
			{
				for (int m = 0; m < 3; m++)
				{
					UVWData *data = new UVWData();
					data->node = node;
					data->channelID = 0;
					data->subID = m;
					data->channelType = TRIGEOMCHANNEL;

					data->numFaces = msh->numFaces;
					data->numRealFaces = msh->numFaces;
					data->numVerts = msh->numVerts;
					data->numOfDeadVerts = deadVerts;
					data->kbsize =  (sizeof(float) * data->numVerts)/1000;
					data->channelName.printf(GetString(IDS_NONE));
					nodeList.Append(1,&data,50);

				}

			}

			//add the sel channel
			data = new UVWData();
			data->node = node;
			data->channelID = 0;
			data->subID = 0;
			data->channelType = TRISELCHANNEL;

			data->numFaces = msh->numFaces;
			data->numRealFaces = msh->numFaces;
			data->numVerts = msh->numVerts;
			data->numOfDeadVerts = 0;
			data->kbsize = ( sizeof(float) * data->numVerts)/1000;

			data->channelName.printf(GetString(IDS_NONE));

			nodeList.Append(1,&data,50);



		//get the number of maps
			int numberMapChannels = msh->getNumMaps();

			for (int j = -2; j < numberMapChannels; j++)
			{
				UVWData *data = new UVWData();

				int deadVerts = GetDeadMapVerts(msh,j);

				if (!theTreeViewUtil.subComponents)
				{

					data->node = node;
					data->channelID = j;
					data->channelType = TRIMAPCHANNEL;

					data->numFaces = msh->numFaces;
					data->numRealFaces = msh->numFaces;
					data->numVerts = msh->getNumMapVerts(j);
					data->kbsize = (sizeof(TVFace) * data->numFaces + sizeof(Point3) * data->numVerts)/1000;

					data->channelName.printf(GetString(IDS_NONE));
					data->subID = -50;
					data->numOfDeadVerts = deadVerts;

					nodeList.Append(1,&data,50);
				}

				if (theTreeViewUtil.subComponents)
				{
					for (int m = 0; m < 3; m++)
					{
						UVWData *data = new UVWData();
						data->node = node;
						data->channelID = j;
						data->channelType = TRIMAPCHANNEL;

						data->numFaces = msh->numFaces;
						data->numRealFaces = msh->numFaces;
						data->numVerts = msh->getNumMapVerts(j);
						data->kbsize = ( sizeof(float) * data->numVerts)/1000;
						data->subID = m;
						data->numOfDeadVerts = deadVerts;
						data->channelName.printf(GetString(IDS_NONE));
						nodeList.Append(1,&data,50);

					}

				}			
			}


}

void TreeViewUtil::UpdateViewItems()
{
	Tab<INode *> nodes;


	for (int i = 0; i < nodeList.Count(); i++)
	{
		INode *node = nodeList[i]->node;
		BOOL hit = FALSE;
		for (int j =0; j < nodes.Count(); j++)
		{
			if (node == nodes[j]) 
			{
				hit = TRUE;
				j = nodes.Count();
			}
		}
		if (!hit) nodes.Append(1,&node);
	}

	for (i = 0; i < nodeList.Count(); i++)
	{
		if (nodeList[i]) delete nodeList[i];
	}

	int numberNodes = nodes.Count();

	nodeList.ZeroCount();
	
	TimeValue t = GetCOREInterface()->GetTime();



	for (i = 0; i < numberNodes; i++)
	{

		INode *node = nodes[i];

		ObjectState os = node->EvalWorldState(t);

		Object *obj = os.obj;

		//get the mesh
		Mesh *msh = NULL;
		MNMesh *mnmsh = NULL;
		PatchMesh *pmsh = NULL;
		
		TriObject *collapsedtobj = NULL;
		if (os.obj->IsSubClassOf(triObjectClassID))
		{
			TriObject *tobj = (TriObject*)os.obj;
			msh = &tobj->GetMesh();
		}
		else if (os.obj->IsSubClassOf(polyObjectClassID))
		{
			PolyObject *pobj = (PolyObject*)os.obj;
			mnmsh = &pobj->mm;

		}
		else if (os.obj->IsSubClassOf(patchObjectClassID))
		{
			PatchObject *pobj = (PatchObject*)os.obj;
			pmsh = &pobj->patch;

		}
//collapse it to a mesh
		else
		{
			BOOL canConvertToTri = os.obj->CanConvertToType(triObjectClassID);
			BOOL isNURBS = (os.obj->ClassID() == EDITABLE_SURF_CLASS_ID);//  ;CanConvertToType(EDITABLE_SURF_CLASS_ID)
			if ( canConvertToTri&& (!isNURBS) )
			{
				collapsedtobj = (TriObject*) os.obj->ConvertToType(t,triObjectClassID);
				msh = &collapsedtobj->GetMesh();
			}
			
		}

		if (msh)
		{
			AddMesh(msh,node);
		}
		if (pmsh)
		{
			AddPatch(pmsh,node);
		}
		if (mnmsh)
		{
			AddPoly(mnmsh,node);
		}
		if (collapsedtobj) collapsedtobj->DeleteThis();


		//get the number of verts and faces
		//compute the size
		
	}
	AddNames();
}

void TreeViewUtil::AddSelectedNodesToList()

{
	Interface *ip = GetCOREInterface();

	for (int i = 0; i < nodeList.Count(); i++)
	{
		if (nodeList[i]) delete nodeList[i];
	}

	int numberNodes = ip->GetSelNodeCount();

	nodeList.ZeroCount();
	
	TimeValue t = ip->GetTime();

	for (i = 0; i < numberNodes; i++)
	{

		INode *node = ip->GetSelNode(i);

		ObjectState os = node->EvalWorldState(t);

		Object *obj = os.obj;

		//get the mesh
		Mesh *msh = NULL;
		MNMesh *mnmsh = NULL;
		PatchMesh *pmsh = NULL;

		TriObject *collapsedtobj = NULL;
		if (os.obj->IsSubClassOf(triObjectClassID))
		{
			TriObject *tobj = (TriObject*)os.obj;
			msh = &tobj->GetMesh();
		}
		else if (os.obj->IsSubClassOf(polyObjectClassID))
		{
			PolyObject *pobj = (PolyObject*)os.obj;
			mnmsh = &pobj->mm;//GetMesh();

		}
		else if (os.obj->IsSubClassOf(patchObjectClassID))
		{
			PatchObject *pobj = (PatchObject*)os.obj;
			pmsh = &pobj->patch;

		}
//collapse it to a mesh
		else
		{
		
			if (os.obj->CanConvertToType(triObjectClassID) && (!os.obj->IsSubClassOf(EDITABLE_SURF_CLASS_ID )))
			{
				collapsedtobj = (TriObject*) os.obj->ConvertToType(t,triObjectClassID);
				msh = &collapsedtobj->GetMesh();
			}
			
		}

		if (msh)
		{

			AddMesh(msh,node);

		}
		if (pmsh)
		{

			AddPatch(pmsh,node);

		}
		else if (mnmsh)
		{

			AddPoly(mnmsh,node);

		}

		if (collapsedtobj) collapsedtobj->DeleteThis();


		//get the number of verts and faces
		//compute the size
		
	}
	AddNames();
}


void TreeViewUtil::InitListView()
{
	LV_COLUMN lvC;      // List View Column structure

	// Now initialize the columns we will need
	// Initialize the LV_COLUMN structure
	// the mask specifies that the .fmt, .ex, width, and .subitem members 


	// of the structure are valid,
	int numCols = 0;
	TSTR name(GetString(IDS_OBJECTNAME));
	lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvC.fmt = LVCFMT_LEFT;  // left align the column
	lvC.cx = 120;            // width of the column, in pixels
	lvC.pszText = name;
	lvC.iSubItem = numCols;
	ListView_InsertColumn(listViewHWND, numCols++, &lvC);

	TSTR cid(GetString(IDS_ID));
	lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvC.fmt = LVCFMT_LEFT;  // left align the column
	lvC.cx = 60;            // width of the column, in pixels
	lvC.pszText = cid;
	lvC.iSubItem = numCols;
	ListView_InsertColumn(listViewHWND, numCols++, &lvC);

	TSTR cname(GetString(IDS_CHANNELNAME));
	lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvC.fmt = LVCFMT_LEFT;  // left align the column
	lvC.cx = 120;            // width of the column, in pixels
	lvC.pszText = cname;
	lvC.iSubItem = numCols;
	ListView_InsertColumn(listViewHWND, numCols++, &lvC);



	TSTR numVerts(GetString(IDS_NUMVERTS));
	lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvC.fmt = LVCFMT_LEFT;  // left align the column
	lvC.cx = 70;            // width of the column, in pixels
	lvC.pszText = numVerts;
	lvC.iSubItem = numCols;
	ListView_InsertColumn(listViewHWND, numCols++, &lvC);

	TSTR numFaces(GetString(IDS_NUMFACES));
	lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvC.fmt = LVCFMT_LEFT;  // left align the column
	lvC.cx = 70;            // width of the column, in pixels
	lvC.pszText = numFaces;
	lvC.iSubItem = numCols;
	ListView_InsertColumn(listViewHWND, numCols++, &lvC);

	TSTR numDVerts(GetString(IDS_DEADVERTS));
	lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvC.fmt = LVCFMT_LEFT;  // left align the column
	lvC.cx = 70;            // width of the column, in pixels
	lvC.pszText = numDVerts;
	lvC.iSubItem = numCols;
	ListView_InsertColumn(listViewHWND, numCols++, &lvC);


	TSTR ksize(GetString(IDS_SIZEKB));
	lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvC.fmt = LVCFMT_LEFT;  // left align the column
	lvC.cx = 60;            // width of the column, in pixels
	lvC.pszText = ksize;
	lvC.iSubItem = numCols;
	ListView_InsertColumn(listViewHWND, numCols++, &lvC);


//	ListView_SetBkColor(listViewHWND,GetSysColor( COLOR_BTNSHADOW ));
//	ListView_SetTextBkColor(listViewHWND,GetSysColor( COLOR_BTNSHADOW ));
//	ListView_SetTextColor(listViewHWND,GetSysColor(COLOR_BTNHILIGHT));

	ListView_SetExtendedListViewStyle(listViewHWND,LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES );

	

}

void TreeViewUtil::AddListViewItems()
{

	LV_ITEM lvI;        // List view item structure
	int iSubItem;       // Index for inserting sub items

	// Finally, let's add the actual items to the control
	// Fill in the LV_ITEM structure for each of the items to add
	// to the list.
	// The mask specifies the the .pszText, .iImage, .lParam and .state
	// members of the LV_ITEM structure are valid.
	lvI.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE;
	lvI.state = 0;      //
	lvI.stateMask = 0;  //


	int sel = ListView_GetSelectionMark( listViewHWND);

	ListView_DeleteAllItems(listViewHWND);

	int numberOfNodes = nodeList.Count();

	for (int index = 0; index < numberOfNodes; index++)
	{
		lvI.iItem = index;
		lvI.iSubItem = 0;
		// The parent window is responsible for storing the text. The List view
		// window will send a LVN_GETDISPINFO when it needs the text to display/
		lvI.pszText = LPSTR_TEXTCALLBACK; 
		lvI.cchTextMax = 512;
		lvI.iImage = 0;
		lvI.lParam = (LPARAM)index;

		if (ListView_InsertItem(listViewHWND, &lvI) == -1)
			return;

		for (iSubItem = 1; iSubItem < 4; iSubItem++)
		{
			ListView_SetItemText( listViewHWND,
				index,
				iSubItem,
				LPSTR_TEXTCALLBACK);
		}
	}
	if (sel !=-1)
		{
		if (sel >= numberOfNodes)
			sel = numberOfNodes-1;
		ListView_SetItemState (listViewHWND,         // handle to listview
                          sel,         // index to listview item
                          LVIS_FOCUSED | LVIS_SELECTED, // item state
                          0x000F);                      // mask

		}

}


LRESULT TreeViewUtil::NotifyHandler( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LV_DISPINFO *pLvdi = (LV_DISPINFO *)lParam;
	NM_LISTVIEW *pNm = (NM_LISTVIEW *)lParam;
	int index = (int)(pLvdi->item.lParam);
	static TCHAR szText[512];

	if (wParam != IDC_MAPLIST)
		return 0L;

	switch(pLvdi->hdr.code)
	{

		case NM_RCLICK:
			POINT lpPt; GetCursorPos(&lpPt);
			ShowRMenu(hWnd,lpPt.x,lpPt.y);
			break;
		case LVN_ITEMCHANGED:
			UpdateUI();
			break;
		case LVN_GETDISPINFO:

			switch (pLvdi->item.iSubItem)
			{
				case 0:     // node name
					sprintf(szText, "%s", nodeList[index]->node->GetName());
					pLvdi->item.pszText = szText;

					break;

				case 1:     // channel ID

					if ((nodeList[index]->channelType == TRISELCHANNEL)||(nodeList[index]->channelType == POLYSELCHANNEL))
					{
						sprintf(szText,GetString(IDS_vsel));
						pLvdi->item.pszText = szText;

					}
					else if (nodeList[index]->channelType == TRIGEOMCHANNEL)
					{
						if (nodeList[index]->subID == -50)
							sprintf(szText, "%s", GetString(IDS_mesh));
						else if (nodeList[index]->subID == 0)
							sprintf(szText,  "%s", GetString(IDS_meshx));
						else if (nodeList[index]->subID == 1)
							sprintf(szText,  "%s", GetString(IDS_meshy));
						else if (nodeList[index]->subID == 2)
							sprintf(szText,  "%s", GetString(IDS_meshz));

						pLvdi->item.pszText = szText;

					}
					else if (nodeList[index]->channelType == POLYGEOMCHANNEL)
					{
						if (nodeList[index]->subID == -50)
							sprintf(szText,  "%s", GetString(IDS_poly));
						else if (nodeList[index]->subID == 0)
							sprintf(szText,  "%s", GetString(IDS_polyx));
						else if (nodeList[index]->subID == 1)
							sprintf(szText,  "%s", GetString(IDS_polyy));
						else if (nodeList[index]->subID == 2)
							sprintf(szText,  "%s", GetString(IDS_polyz));

						pLvdi->item.pszText = szText;

					}
					else if (nodeList[index]->channelType == PATCHGEOMCHANNEL)
					{
						if (nodeList[index]->subID == -50)
							sprintf(szText,  "%s", GetString(IDS_patch));
						else if (nodeList[index]->subID == 0)
							sprintf(szText,  "%s", GetString(IDS_patchx));
						else if (nodeList[index]->subID == 1)
							sprintf(szText,  "%s", GetString(IDS_patchy));
						else if (nodeList[index]->subID == 2)
							sprintf(szText,  "%s", GetString(IDS_patchz));

						pLvdi->item.pszText = szText;

					}

					else 
					{
						if (nodeList[index]->channelID == -2)
						{
							if (nodeList[index]->subID == -50)
								sprintf(szText, "%d:%s", nodeList[index]->channelID,GetString(IDS_alpha));
							else if (nodeList[index]->subID == 0)
								sprintf(szText, "%d:%s", nodeList[index]->channelID,GetString(IDS_alphax));
							else if (nodeList[index]->subID == 1)
								sprintf(szText, "%d:%s", nodeList[index]->channelID,GetString(IDS_alphay));
							else if (nodeList[index]->subID == 2)
								sprintf(szText, "%d:%s", nodeList[index]->channelID,GetString(IDS_alphaz));

						}
						else if (nodeList[index]->channelID == -1)
						{
							if (nodeList[index]->subID == -50)
								sprintf(szText, "%d:%s", nodeList[index]->channelID,GetString(IDS_illum));
							else if (nodeList[index]->subID == 0)
								sprintf(szText, "%d:%s", nodeList[index]->channelID,GetString(IDS_illumx));
							else if (nodeList[index]->subID == 1)
								sprintf(szText, "%d:%s", nodeList[index]->channelID,GetString(IDS_illumy));
							else if (nodeList[index]->subID == 2)
								sprintf(szText, "%d:%s", nodeList[index]->channelID,GetString(IDS_illumz));

						}
						else if (nodeList[index]->channelID == 0)
						{
							if (nodeList[index]->subID == -50)
								sprintf(szText, "%d:%s", nodeList[index]->channelID,GetString(IDS_vc));
							else if (nodeList[index]->subID == 0)
								sprintf(szText, "%d:%s", nodeList[index]->channelID,GetString(IDS_vcx));
							else if (nodeList[index]->subID == 1)
								sprintf(szText, "%d:%s", nodeList[index]->channelID,GetString(IDS_vcy));
							else if (nodeList[index]->subID == 2)
								sprintf(szText, "%d:%s", nodeList[index]->channelID,GetString(IDS_vcz));

						}
						else 
						{
							if (nodeList[index]->subID == -50)
								sprintf(szText, "%d:%s", nodeList[index]->channelID,GetString(IDS_map));
							else if (nodeList[index]->subID == 0)
								sprintf(szText, "%d:%s", nodeList[index]->channelID,GetString(IDS_mapx));
							else if (nodeList[index]->subID == 1)
								sprintf(szText, "%d:%s", nodeList[index]->channelID,GetString(IDS_mapy));
							else if (nodeList[index]->subID == 2)
								sprintf(szText, "%d:%s", nodeList[index]->channelID,GetString(IDS_mapz));

//							sprintf(szText, "%d : map", nodeList[index]->channelID);
						}
						pLvdi->item.pszText = szText;
					}

//					sprintf(szText, "%u", nodeList[index]->channelID);
//					pLvdi->item.pszText = szText;
					break;

				case 2:     // Channel name
					sprintf(szText, "%s", nodeList[index]->channelName);
					pLvdi->item.pszText = szText;
					break;

				case 3:     // num verts
					sprintf(szText, "%d", nodeList[index]->numVerts);
					pLvdi->item.pszText = szText;
					break;

				case 4:     // Number of faces
					sprintf(szText, "%d", nodeList[index]->numFaces);
					pLvdi->item.pszText = szText;
					break;
				case 5:     // num dead verts
					sprintf(szText, "%d", nodeList[index]->numOfDeadVerts);
					pLvdi->item.pszText = szText;
					break;

				case 6:     // size
					sprintf(szText, "%d%s", nodeList[index]->kbsize,GetString(IDS_kb));
					pLvdi->item.pszText = szText;
					break;

				default:
					break;
			}
			break;


		default:
			break;
	}
	return 0L;
}


void TreeViewUtil::NotifyPreDeleteNode(void* param, NotifyInfo* arg)
{
	//delete any entries that are being deleted
	TreeViewUtil* util = (TreeViewUtil*)param;
	if(util == NULL) return;
	INode* deleted_node = (INode*)arg->callParam;

	//rebuild the list view


	for (int index = 0; index <  util->nodeList.Count(); index++)
	{
		if (util->nodeList[index]->node == deleted_node)
			{
			delete util->nodeList[index];
			util->nodeList.Delete(index,1);
			index--;
			}
	}

	//update the window
	theTreeViewUtil.AddListViewItems();
	// CAL-09/09/03: update UI (Defect #519712)
	theTreeViewUtil.UpdateUI();
}
void TreeViewUtil::NotifySelectionChange(void* param, NotifyInfo*)
{
	//if locked skip
	TreeViewUtil* util = (TreeViewUtil*)param;
	if(util == NULL) return;

	//else rebuild the list
	if (!util->lockSelection)
	{
		util->AddSelectedNodesToList();
		util->AddListViewItems();
	}

}


void TreeViewUtil::InitWindow()
{

	theTreeViewUtil.AddSelectedNodesToList();
	theTreeViewUtil.InitListView();
	theTreeViewUtil.AddListViewItems();

	RegisterNotification(NotifyPreDeleteNode, this, NOTIFY_SCENE_PRE_DELETED_NODE);
	RegisterNotification(NotifySelectionChange, this, NOTIFY_SELECTIONSET_CHANGED);

	iCollapseButton = GetICustButton(GetDlgItem(floaterHWND,IDC_COLLAPSE));
	if (iCollapseButton)
	{
//		iCollapseButton->SetType(CBT_CHECK);
		iCollapseButton->SetHighlightColor(GREEN_WASH);
//		iCollapseButton->SetCheck(collapse);

	}

	iLockButton = GetICustButton(GetDlgItem(floaterHWND,IDC_LOCK));
	if (iLockButton)
	{
		iLockButton->SetType(CBT_CHECK);
		iLockButton->SetHighlightColor(GREEN_WASH);
		iLockButton->SetCheck(lockSelection);
	}

	iClearButton = GetICustButton(GetDlgItem(floaterHWND,IDC_CLEAR));
	if (iClearButton)
	{
		iClearButton->SetHighlightColor(GREEN_WASH);
	}

	iNameButton = GetICustButton(GetDlgItem(floaterHWND,IDC_NAME));
	if (iNameButton)
	{
		iNameButton->SetHighlightColor(GREEN_WASH);
	}

	iCopyButton = GetICustButton(GetDlgItem(floaterHWND,IDC_COPY));
	if (iCopyButton)
	{
		iCopyButton->SetHighlightColor(GREEN_WASH);
	}

	iPasteButton = GetICustButton(GetDlgItem(floaterHWND,IDC_PASTE));
	if (iPasteButton)
	{
		iPasteButton->SetHighlightColor(GREEN_WASH);
	}

	iAddButton = GetICustButton(GetDlgItem(floaterHWND,IDC_ADD));
	if (iAddButton)
	{
		iAddButton->SetHighlightColor(GREEN_WASH);
	}


	iSubCompButton = GetICustButton(GetDlgItem(floaterHWND,IDC_SUBCOMPONENTS));
	if (iSubCompButton)
	{
		iSubCompButton->SetType(CBT_CHECK);
		iSubCompButton->SetHighlightColor(GREEN_WASH);
		iSubCompButton->SetCheck(subComponents);
	}




}
void TreeViewUtil::DestroyWindow()
{
	if (iCollapseButton)
		ReleaseICustButton(iCollapseButton);
	iCollapseButton = NULL;

	if (iLockButton)
		ReleaseICustButton(iLockButton);
	iLockButton = NULL;

	if (iClearButton)
		ReleaseICustButton(iClearButton);
	iClearButton = NULL;

	if (iNameButton)
		ReleaseICustButton(iNameButton);
	iNameButton = NULL;

	if (iCopyButton)
		ReleaseICustButton(iCopyButton);
	iCopyButton = NULL;

	if (iPasteButton)
		ReleaseICustButton(iPasteButton);
	iPasteButton = NULL;

	if (iAddButton)
		ReleaseICustButton(iAddButton);
	iAddButton = NULL;

	if (iSubCompButton)
		ReleaseICustButton(iSubCompButton);
	iSubCompButton = NULL;


	UnRegisterNotification(NotifyPreDeleteNode, this,	NOTIFY_SCENE_PRE_DELETED_NODE);
	UnRegisterNotification(NotifySelectionChange, this, NOTIFY_SELECTIONSET_CHANGED);

	theTreeViewUtil.floaterHWND = NULL;

}


void TreeViewUtil::DeleteChannel(INode *node, int mapID)
{
	if (node)
	{
		Object* obj = node->GetObjectRef();

		if (obj)
		{
			theHold.Begin();
			theHold.Put(new UpdateUIRestore());


			TSTR key, name;
			name.printf("");
			
			key.printf("MapChannel:%d",mapID);
			if (node->UserPropExists(key))
			{
				node->SetUserPropString(key,name);
			}
			
			for (int k = 0; k < 3; k++)
			{
				key.printf("MapChannel:%d(%d)",mapID,k);
				if (node->UserPropExists(key))
				{
					node->SetUserPropString(key,name);
				}				
			}
			
				
			MapChannelDelete* mod;
			mod = (MapChannelDelete*)CreateInstance(OSM_CLASS_ID, MAPCHANNELDELETE_CLASS_ID);

			if (mod)
			{
        
				mod->SetMapChannel(mapID);
				IDerivedObject* dobj = CreateDerivedObject(obj);
				dobj->AddModifier(mod);
				node->SetObjectRef(dobj);
				mod->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
				mod->NotifyDependents(FOREVER,PART_ALL,REFMSG_NUM_SUBOBJECTTYPES_CHANGED);


				Interface *ip = GetCOREInterface();

				int selCount= ip->GetSelNodeCount();

				BOOL select = FALSE;
				for (int i = 0; i < selCount; i++)
					{
					INode *selNode = ip->GetSelNode(i);
					if (selNode == node)
						select = TRUE;
					}
				if (select)
					ip->SelectNode(node,0);

				ip->RedrawViews(ip->GetTime());			
			}
			theHold.Accept(GetString(IDS_DELETECHANNEL));

		}
	}
}


void TreeViewUtil::ShowRMenu(HWND hwnd, int x, int y) 
{
	HMENU hMenu = LoadMenu(hInstance,MAKEINTRESOURCE(IDR_MENU1));
	HMENU subMenu = GetSubMenu(hMenu, 0);

	int enableCopy = MF_ENABLED;
	if ((this->iCopyButton) && (!this->iCopyButton->IsEnabled()))
		enableCopy = MF_GRAYED;

	int numItem = ListView_GetItemCount(listViewHWND);

	int sel =-1;
	for(int i=0;i<numItem;i++)
	{
		if ((ListView_GetItemState(listViewHWND,i,LVIS_SELECTED))&LVIS_SELECTED )
			sel = i;
	}

	if ((sel < 0) || (sel >= nodeList.Count()))
	{
		EnableMenuItem(subMenu,ID_MAPINFO_COPY,enableCopy);

		EnableMenuItem(subMenu,ID_MAPINFO_PASTE,MF_GRAYED);

		EnableMenuItem(subMenu,ID_MAPINFO_CLEAR,MF_GRAYED);
		EnableMenuItem(subMenu,ID_MAPINFO_RENAME,MF_GRAYED);

		EnableMenuItem(subMenu,ID_MAPINFO_ADD,MF_GRAYED);



	}
	else
	{
		EnableMenuItem(subMenu,ID_MAPINFO_RENAME,MF_ENABLED);
		EnableMenuItem(subMenu,ID_MAPINFO_COPY,enableCopy);

		EnableMenuItem(subMenu,ID_MAPINFO_ADD,MF_ENABLED);

		if (buffer.copyType == EMPTY)
			{
			EnableMenuItem(subMenu,ID_MAPINFO_PASTE,MF_GRAYED);

			}
		else
			{
			if (nodeList[sel]->numRealFaces == buffer.numRealFaces)
				{
				if ((buffer.subID == -50) && (nodeList[sel]->subID == -50))
					EnableMenuItem(subMenu,ID_MAPINFO_PASTE,MF_ENABLED);
				else if ((buffer.subID != -50) && (nodeList[sel]->subID != -50))
					EnableMenuItem(subMenu,ID_MAPINFO_PASTE,MF_ENABLED);
				else EnableMenuItem(subMenu,ID_MAPINFO_PASTE,MF_GRAYED);
					
				}
			else EnableMenuItem(subMenu,ID_MAPINFO_PASTE,MF_GRAYED);

			}



		EnableMenuItem(subMenu,ID_MAPINFO_COPY,enableCopy);
		EnableMenuItem(subMenu,ID_MAPINFO_RENAME,MF_ENABLED);

		if ( (nodeList[sel]->channelType == TRIGEOMCHANNEL) || (nodeList[sel]->channelType == TRISELCHANNEL) ||
		     (nodeList[sel]->channelType == POLYSELCHANNEL) || (nodeList[sel]->channelType == PATCHSELCHANNEL)
		   )
		{

			EnableMenuItem(subMenu,ID_MAPINFO_CLEAR,MF_GRAYED);

		}
		else 
		{
			EnableMenuItem(subMenu,ID_MAPINFO_CLEAR,MF_ENABLED);
		}
	}


	TrackPopupMenu(subMenu, TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, 
		x-2, y, 0, hwnd, NULL);
	DestroyMenu(subMenu);
	DestroyMenu(hMenu);		

}


void TreeViewUtil::UpdateUI()
{

	if ( (buffer.numFaces != 0) && (buffer.copyType != EMPTY))
	{
		HWND textHWND = GetDlgItem(floaterHWND,IDC_COPYBUFFERSTATIC);
		SetWindowText(textHWND,buffer.name);
	}

	int numItem = ListView_GetItemCount(listViewHWND);

	int sel =-1;
	for(int i=0;i<numItem;i++)
	{
		if ((ListView_GetItemState(listViewHWND,i,LVIS_SELECTED))&LVIS_SELECTED )
			sel = i;
	}

//	int sel = ListView_GetSelectionMark( listViewHWND);
	iSubCompButton->Enable(TRUE);

	if ((sel < 0) || (sel >= nodeList.Count()))
	{
		iClearButton->Enable(FALSE);
		iNameButton->Enable(FALSE);

		iCopyButton->Enable(FALSE);
		iPasteButton->Enable(FALSE);
		iAddButton->Enable(FALSE);

	}
	else
	{

	
		iAddButton->Enable(TRUE);

		if (nodeList[sel]->numVerts == 0)
			iCopyButton->Enable(FALSE);
		else iCopyButton->Enable(TRUE);

		if (buffer.copyType == EMPTY)
		{
			iPasteButton->Enable(FALSE);
		}
		else
		{
			if (nodeList[sel]->numRealFaces == buffer.numRealFaces)
			{
				int bSubID = buffer.subID;
				int sSubID = nodeList[sel]->subID;
				if ((buffer.subID == -50) && (nodeList[sel]->subID == -50))
					iPasteButton->Enable(TRUE);
				else if ((buffer.subID != -50) && (nodeList[sel]->subID != -50))
					iPasteButton->Enable(TRUE);
				else iPasteButton->Enable(FALSE);
			}
			else iPasteButton->Enable(FALSE);
		}

		iNameButton->Enable(TRUE);


		if ( (nodeList[sel]->channelType == TRIGEOMCHANNEL) || (nodeList[sel]->channelType == TRISELCHANNEL) ||
		     (nodeList[sel]->channelType == POLYSELCHANNEL) || (nodeList[sel]->channelType == PATCHSELCHANNEL) )
		{
			iClearButton->Enable(FALSE);
		}
		else 
		{
			iClearButton->Enable(TRUE);
		}
	}
		

}


void TreeViewUtil::AddNames()
{
//loop through our channels and see if we have any names
	for (int i = 0; i < nodeList.Count(); i++)
	{
		INode *node = nodeList[i]->node;
		int id = nodeList[i]->channelID;
		if (node)
		{
			if ((nodeList[i]->channelType == TRIMAPCHANNEL) || (nodeList[i]->channelType == POLYMAPCHANNEL)|| (nodeList[i]->channelType == PATCHMAPCHANNEL))
			{
				TSTR key, name;
				if (nodeList[i]->subID == -50)
					key.printf("MapChannel:%d",id);
				else key.printf("MapChannel:%d(%d)",id,nodeList[i]->subID);
				if (node->UserPropExists(key))
				{
					node->GetUserPropString(key,name);
					if (name.Length() > 0)
					{
						nodeList[i]->channelName.printf("%s",name);
					}
				}
			}
			else if ((nodeList[i]->channelType == TRIGEOMCHANNEL)||(nodeList[i]->channelType == POLYGEOMCHANNEL))
			{
				TSTR key, name;

				if (nodeList[i]->subID == -50)
					key.printf("GeomChannel");
				else key.printf("GeomChannel(%d)",nodeList[i]->subID);

				if (node->UserPropExists(key))
				{
					node->GetUserPropString(key,name);
					if (name.Length() > 0)
					{
						nodeList[i]->channelName.printf("%s",name);
					}
				}
			}
			else if ((nodeList[i]->channelType == TRISELCHANNEL) || (nodeList[i]->channelType == POLYSELCHANNEL))
			{
				TSTR key, name;
				key.printf("SelChannel");
				if (node->UserPropExists(key))
				{
					node->GetUserPropString(key,name);
					if (name.Length() > 0)
					{
						nodeList[i]->channelName.printf("%s",name);
					}
				}
			}

		}
		
	}

}


static INT_PTR CALLBACK NameDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)

{

	switch (msg) {
		case WM_INITDIALOG:
			{			
			ICustEdit *iName = GetICustEdit(GetDlgItem(hWnd,IDC_NAME));

			if (iName)
			{
				iName->SetText(theTreeViewUtil.channelName);
				iName->WantReturn(TRUE);
				ReleaseICustEdit(iName);
			}

			return 1;
			}
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_NAME:

					if (HIWORD(wParam)==EN_CHANGE) 
					{
						ICustEdit *iName = GetICustEdit(GetDlgItem(hWnd,IDC_NAME));

						if (iName)
						{
							if (iName->GotReturn())
							{
								TCHAR tempName[512];
								iName->GetText(tempName,512);
								theTreeViewUtil.channelName.printf("%s",tempName);
						 		ReleaseICustEdit(iName);						 		
								EndDialog(hWnd,1);

							}
						}

					}
					break;

				case IDOK:
					{
					ICustEdit *iName = GetICustEdit(GetDlgItem(hWnd,IDC_NAME));

					if (iName)
					{
						TCHAR tempName[512];
						iName->GetText(tempName,512);
						theTreeViewUtil.channelName.printf("%s",tempName);
						ReleaseICustEdit(iName);
					}

					EndDialog(hWnd,1);
					break;
					}
				case IDCANCEL:
					EndDialog(hWnd,0);
					break;
				}
			return 1;
		}
	return 0;
}


void TreeViewUtil::NameChannel()
{

	int numItem = ListView_GetItemCount(listViewHWND);

	int sel =-1;
	for(int i=0;i<numItem;i++)
	{
		if ((ListView_GetItemState(listViewHWND,i,LVIS_SELECTED))&LVIS_SELECTED )
			sel = i;
	}

	if ((sel >=0) && (sel < nodeList.Count()))
	{
		NameChannel(sel);

	}
}

void TreeViewUtil::NameChannel(INode *node, int channelType, int channelID, int subID, TCHAR *name)
{
	TSTR tname, key;
	tname.printf("%s",name);

	if (channelType == CHANNEL_MAP)
	{
		if (subID == -50)
			key.printf("MapChannel:%d",channelID);
		else key.printf("MapChannel:%d(%d)",channelID,subID);
		node->SetUserPropString(key,tname);
	}
	else if (channelType == CHANNEL_GEOM)
	{
		if (subID == -50)
			key.printf("GeomChannel");
		else key.printf("GeomChannel(%d)",subID);
		node->SetUserPropString(key,tname);
	}
	else if (channelType == CHANNEL_SEL)
	{
		key.printf("SelChannel");
		node->SetUserPropString(key,tname);
	}



}


void TreeViewUtil::NameChannel(int sel)
{

	if ((sel >=0) && (sel < nodeList.Count()))
	{

		channelName.printf("%s",nodeList[sel]->channelName);

		SetFocus(GetDlgItem(theTreeViewUtil.floaterHWND,IDC_MAPLIST));
		
//pop up name dialog
		int iret = DialogBox(hInstance,MAKEINTRESOURCE(IDD_NAME),
		        listViewHWND,NameDlgProc);	
		if (iret)
		{
			nodeList[sel]->channelName.printf("%s",channelName);

			INode *node = nodeList[sel]->node;
			TSTR tname, key;
			tname.printf("%s",channelName);

			if ((nodeList[sel]->channelType == TRIMAPCHANNEL)||(nodeList[sel]->channelType == POLYMAPCHANNEL))
			{

				if (nodeList[sel]->subID == -50)
				{
					key.printf("MapChannel:%d",nodeList[sel]->channelID);
					macroRecorder->FunctionCall(_T("channelInfo.NameChannel"), 4, 0,
									mr_reftarg,theTreeViewUtil.nodeList[sel]->node,
									mr_int,3,
									mr_int,theTreeViewUtil.nodeList[sel]->channelID,
									mr_string,tname
									);

				}
				else 
				{
					key.printf("MapChannel:%d(%d)",nodeList[sel]->channelID,nodeList[sel]->subID);
					macroRecorder->FunctionCall(_T("channelInfo.NameSubChannel"), 5, 0,
									mr_reftarg,theTreeViewUtil.nodeList[sel]->node,
									mr_int,3,
									mr_int,theTreeViewUtil.nodeList[sel]->channelID,
									mr_int, nodeList[sel]->subID,
									mr_string,tname
									);

				}
				node->SetUserPropString(key,tname);
			}
			else if ((nodeList[sel]->channelType == TRIGEOMCHANNEL)||(nodeList[sel]->channelType == POLYGEOMCHANNEL))
			{


				if (nodeList[sel]->subID == -50)
				{
					key.printf("GeomChannel");
					macroRecorder->FunctionCall(_T("channelInfo.NameChannel"), 4, 0,
									mr_reftarg,theTreeViewUtil.nodeList[sel]->node,
									mr_int,1,
									mr_int,theTreeViewUtil.nodeList[sel]->channelID,
									mr_string,tname
									);
				}
				else 
				{
					key.printf("GeomChannel(%d)",nodeList[sel]->subID);
					macroRecorder->FunctionCall(_T("channelInfo.NameChannel"), 5, 0,
									mr_reftarg,theTreeViewUtil.nodeList[sel]->node,
									mr_int,1,
									mr_int,theTreeViewUtil.nodeList[sel]->channelID,
									mr_int,nodeList[sel]->subID,
									mr_string,tname
									);

				}
				node->SetUserPropString(key,tname);
			}
			else if ((nodeList[sel]->channelType == TRISELCHANNEL)||(nodeList[sel]->channelType == POLYSELCHANNEL))
			{
				macroRecorder->FunctionCall(_T("channelInfo.NameChannel"), 4, 0,
									mr_reftarg,theTreeViewUtil.nodeList[sel]->node,
									mr_int,3,
									mr_int,theTreeViewUtil.nodeList[sel]->channelID,
									mr_string,tname
									);

				key.printf("SelChannel");
				node->SetUserPropString(key,tname);
			}

			macroRecorder->EmitScript();

			ListView_RedrawItems(listViewHWND,sel,sel);
			UpdateWindow(listViewHWND);

		}
	}
}


int TreeViewUtil::GetSel()
{

	int numItem = ListView_GetItemCount(listViewHWND);

	int sel =-1;
	for(int i=0;i<numItem;i++)
	{
		if ((ListView_GetItemState(listViewHWND,i,LVIS_SELECTED))&LVIS_SELECTED )
			sel = i;
	}


	return sel;
}

void TreeViewUtil::CopyToBuffer(INode *node, int channelType, int channelID, int subID)
{
//clear out our old data
	buffer.Clear();

	//case our type

	TimeValue t = GetCOREInterface()->GetTime();
	ObjectState os = node->EvalWorldState(t);

	Object *obj = os.obj;

		//get the mesh
	Mesh *msh = NULL;
	MNMesh *mnmsh = NULL;
	PatchMesh *pmsh = NULL;
	TriObject *collapsedtobj = NULL;
	if (os.obj->IsSubClassOf(triObjectClassID))
	{
		TriObject *tobj = (TriObject*)os.obj;
		msh = &tobj->GetMesh();
	}
	else if (os.obj->IsSubClassOf(polyObjectClassID))
	{
		PolyObject *pobj = (PolyObject*)os.obj;
		mnmsh = &pobj->GetMesh();
	}
	else if (os.obj->IsSubClassOf(patchObjectClassID))
	{
		PatchObject *pobj = (PatchObject*)os.obj;
		pmsh = &pobj->patch;
	}
//collapse it to a mesh
	else
	{
		if (os.obj->CanConvertToType(triObjectClassID))
		{
			collapsedtobj = (TriObject*) os.obj->ConvertToType(t,triObjectClassID);
			msh = &collapsedtobj->GetMesh();
		}
	}

	if (channelType == CHANNEL_SEL)
	{
		TSTR selChan(GetString(IDS_SELCHANNEL));
		buffer.name.printf("%s%s %s",GetString(IDS_COPYBUFFER),node->GetName(),selChan);
	}
	else if (channelType == CHANNEL_GEOM)
	{
		TSTR geomChan(GetString(IDS_GEOMCHANNEL));
		if (subID == -50)
			buffer.name.printf("%s %s %s",GetString(IDS_COPYBUFFER),node->GetName(),geomChan);
		else if (subID == 0)
			buffer.name.printf("%s %s %s X ",GetString(IDS_COPYBUFFER),node->GetName(),geomChan);
		else if (subID == 1)
			buffer.name.printf("%s %s %s Y ",GetString(IDS_COPYBUFFER),node->GetName(),geomChan);
		else if (subID == 2)
			buffer.name.printf("%s %s %s Z ",GetString(IDS_COPYBUFFER),node->GetName(),geomChan);
	}
	else if (channelType == CHANNEL_MAP)
	{
		TSTR mapChan(GetString(IDS_MAPCHANNEL));
	
		if (subID == -50)
			buffer.name.printf("%s%s %s %d",GetString(IDS_COPYBUFFER),node->GetName(),mapChan, channelID);
		else if (subID == 0)
			buffer.name.printf("%s%s %s %d X",GetString(IDS_COPYBUFFER),node->GetName(),mapChan,channelID);
		else if (subID == 1)
			buffer.name.printf("%s%s %s %d Y",GetString(IDS_COPYBUFFER),node->GetName(),mapChan,channelID);
		else if (subID == 2)
			buffer.name.printf("%s%s %s %d Z",GetString(IDS_COPYBUFFER),node->GetName(),mapChan,channelID);

	}




	if (pmsh)
	{
		if (channelType == CHANNEL_SEL)
		{

		}
		else if (channelType == CHANNEL_GEOM)
		{
				
		}
		else if (channelType == CHANNEL_MAP)
		{
			if (subID == -50)
			{

				buffer.copyType = PATCHMESH_MAP;
				buffer.subID = -50;

				int numVerts,numFaces;
				int mapID = channelID;
				numVerts = pmsh->getNumMapVerts(mapID);
				numFaces = pmsh->numPatches;
				buffer.numRealFaces = pmsh->numPatches;
				if ((numVerts == 0) || (numFaces == 0))
					{
					buffer.copyType = EMPTY;
					return;
					}
				PatchTVert *uvw = pmsh->mapVerts(mapID);
//				TVPatch *uvwFace = pmsh->mapFaces(mapID);

				if ((uvw == NULL) || (numVerts == 0))
					{
					buffer.copyType = EMPTY;
					return;
					}

				buffer.verts.SetCount(numVerts);
				buffer.uvwPatchFaces.SetCount(numFaces);
				buffer.patchDeg.SetCount(numFaces);

				for (int i = 0; i < numVerts; i++)
				{
					buffer.verts[i] = uvw[i].p;
				}
				for (i = 0; i < numFaces; i++)
				{
					TVPatch face = pmsh->getMapPatch(mapID, i);
					buffer.patchDeg[i] = 3;
				
					if (pmsh->patches[i].type == PATCH_QUAD)
						buffer.patchDeg[i] = 4;

					buffer.uvwPatchFaces[i] = face;
				}
				buffer.numFaces = numFaces;
			}
			else
			{
				buffer.copyType = PATCHMESH_MAP;
				buffer.subID = subID;

				int numVerts,numFaces;
				int mapID = channelID;
				numVerts = pmsh->getNumMapVerts(mapID);
				numFaces = pmsh->numPatches;
				if ((numVerts == 0) || (numFaces == 0))
					{
					buffer.copyType = EMPTY;
					return;
					}
				PatchTVert *uvw = pmsh->mapVerts(mapID);
//				TVFace *uvwFace = msh->mapFaces(mapID);

				if (uvw == NULL)
					{
					buffer.copyType = EMPTY;
					return;
					}

				buffer.w.SetCount(numVerts);
				buffer.uvwPatchFaces.SetCount(numFaces);


				for (int i = 0; i < numVerts; i++)
				{
					buffer.w[i] = uvw[i].p[buffer.subID];
				}
				for (i = 0; i < numFaces; i++)
				{
					TVPatch face = pmsh->getMapPatch(mapID, i);
					buffer.uvwPatchFaces[i] = face;
				}
				buffer.numFaces = numFaces;
				buffer.numRealFaces = pmsh->numPatches;

			}

		}
	}

	if (msh)
	{
		if (channelType == CHANNEL_SEL)
		{
			buffer.copyType = TRIMESH_SEL;
			buffer.subID = 0;
			int numVerts,numFaces;
			numVerts = msh->numVerts;
			numFaces = msh->numFaces;
			buffer.w.SetCount(numVerts);
			buffer.geomFaces.SetCount(numFaces);
			buffer.numFaces = numFaces;
			buffer.numRealFaces = msh->numFaces;
			
			float *vsw = NULL;

			vsw =  msh->getVSelectionWeights ();

			for (int i = 0; i < numVerts; i++)
			{
				if (vsw)
				{
				buffer.w[i] = vsw[i];
				}
				else buffer.w[i] = 0.0f;
				if (msh->vertSel[i]) buffer.w[i] = 1.0f;
			}
			for (i = 0; i < numFaces; i++)
			{
				buffer.geomFaces[i] = msh->faces[i];
			}

		}
		else if (channelType == CHANNEL_GEOM)
		{
			if (subID == -50)
			{
				buffer.copyType = TRIMESH_GEOM;
				buffer.subID = -50;
				int numVerts,numFaces;
				numVerts = msh->numVerts;
				numFaces = msh->numFaces;
				buffer.numRealFaces = msh->numFaces;
				buffer.verts.SetCount(numVerts);
				buffer.geomFaces.SetCount(numFaces);
				buffer.numFaces = numFaces;
				for (int i = 0; i < numVerts; i++)
				{
					buffer.verts[i] = msh->verts[i];
				}
				for (i = 0; i < numFaces; i++)
				{
					buffer.geomFaces[i] = msh->faces[i];
				}
			}
			else
			{
				buffer.subID = subID ;
				buffer.copyType = TRIMESH_GEOM;
				int numVerts,numFaces;
				numVerts = msh->numVerts;
				numFaces = msh->numFaces;
				buffer.w.SetCount(numVerts);
				buffer.geomFaces.SetCount(numFaces);
				buffer.numFaces = numFaces;
				buffer.numRealFaces = msh->numFaces;
				for (int i = 0; i < numVerts; i++)
				{
					buffer.w[i] = msh->verts[i][buffer.subID];
				}
				for (i = 0; i < numFaces; i++)
				{
					buffer.geomFaces[i] = msh->faces[i];
				}
			}

				
		}
		else if (channelType == CHANNEL_MAP)
		{
			if (subID == -50)
			{

				buffer.copyType = TRIMESH_MAP;
				buffer.subID = -50;

				int numVerts,numFaces;
				int mapID = channelID;
				numVerts = msh->getNumMapVerts(mapID);
				numFaces = msh->numFaces;
				buffer.numRealFaces = msh->numFaces;
				if ((numVerts == 0) || (numFaces == 0))
					{
					buffer.copyType = EMPTY;
					return;
					}
				UVVert *uvw = msh->mapVerts(mapID);
				TVFace *uvwFace = msh->mapFaces(mapID);

				if ((uvw == NULL) || (uvwFace == NULL))
					{
					buffer.copyType = EMPTY;
					return;
					}

				buffer.verts.SetCount(numVerts);
				buffer.uvwFaces.SetCount(numFaces);


				for (int i = 0; i < numVerts; i++)
				{
					buffer.verts[i] = uvw[i];
				}
				for (i = 0; i < numFaces; i++)
				{
					buffer.uvwFaces[i] = uvwFace[i];
				}
				buffer.numFaces = numFaces;
			}
			else
			{
				buffer.copyType = TRIMESH_MAP;
				buffer.subID = subID;

				int numVerts,numFaces;
				int mapID = channelID;
				numVerts = msh->getNumMapVerts(mapID);
				numFaces = msh->numFaces;
				if ((numVerts == 0) || (numFaces == 0))
					{
					buffer.copyType = EMPTY;
					return;
					}
				UVVert *uvw = msh->mapVerts(mapID);
				TVFace *uvwFace = msh->mapFaces(mapID);

				if ((uvw == NULL) || (uvwFace == NULL))
					{
					buffer.copyType = EMPTY;
					return;
					}

				buffer.w.SetCount(numVerts);
				buffer.uvwFaces.SetCount(numFaces);


				for (int i = 0; i < numVerts; i++)
				{
					buffer.w[i] = uvw[i][buffer.subID];
				}
				for (i = 0; i < numFaces; i++)
				{
					buffer.uvwFaces[i] = uvwFace[i];
				}
				buffer.numFaces = numFaces;
				buffer.numRealFaces = msh->numFaces;

			}

		}
	}
	if (mnmsh)
	{
		if (channelType == CHANNEL_SEL)
		{
			buffer.copyType = POLYMESH_SEL;
			buffer.subID = 0;
			int numVerts,numFaces;
			numVerts = mnmsh->numv;
			numFaces = mnmsh->numf;
			buffer.w.SetCount(numVerts);
			buffer.SetGeomMNFaceCount(numFaces);
			buffer.numFaces = numFaces;
			buffer.numRealFaces = mnmsh->numf;
			
			float *vsw = NULL;

			vsw =  mnmsh->getVSelectionWeights ();

			for (int i = 0; i < numVerts; i++)
			{
				if (vsw)
				{
				buffer.w[i] = vsw[i];
				}
				else buffer.w[i] = 0.0f;

				if (mnmsh->v[i].GetFlag (MN_SEL)) 
					buffer.w[i] = 1.0f;
			}
			for (i = 0; i < numFaces; i++)
			{
				buffer.geomMNFaces[i]->SetDeg(mnmsh->f[i].deg);
				*buffer.geomMNFaces[i] = mnmsh->f[i];
			}

		}

		else if (channelType == CHANNEL_GEOM)
		{
			if (subID == -50)
			{
				buffer.copyType = POLYMESH_GEOM;
				buffer.subID = -50;
				int numVerts,numFaces;
				numVerts = mnmsh->numv;
				numFaces = mnmsh->numf;
				buffer.mnVerts.SetCount(numVerts);
				buffer.SetGeomMNFaceCount(numFaces);
				buffer.numFaces = numFaces;
				buffer.numRealFaces = mnmsh->numf;
				for (int i = 0; i < numVerts; i++)
				{
					buffer.mnVerts[i] = mnmsh->v[i];
				}
				for (i = 0; i < numFaces; i++)
				{
					*buffer.geomMNFaces[i] = mnmsh->f[i];
				}
			}
			else
			{
				buffer.subID = subID ;
				buffer.copyType = POLYMESH_GEOM;
				int numVerts,numFaces;
				numVerts = mnmsh->numv;
				numFaces = mnmsh->numf;
				buffer.w.SetCount(numVerts);
				buffer.SetGeomMNFaceCount(numFaces);
				buffer.numFaces = numFaces;
				buffer.numRealFaces = mnmsh->numf;
				for (int i = 0; i < numVerts; i++)
				{
					buffer.w[i] = mnmsh->v[i].p[buffer.subID];
				}
				for (i = 0; i < numFaces; i++)
				{
					*buffer.geomMNFaces[i] = mnmsh->f[i];
				}
			}

				
		}
		else if (channelType == CHANNEL_MAP)
		{
			if (subID == -50)
			{

				buffer.copyType = POLYMESH_MAP;
				buffer.subID = -50;

				int numVerts,numFaces;
				int mapID = channelID;
				numVerts = mnmsh->M(mapID)->numv;//msh->getNumMapVerts(mapID);
				numFaces = mnmsh->numf;//msh->numFaces;
				if ((numVerts == 0) || (numFaces == 0))
					{
					buffer.copyType = EMPTY;
					return;
					}

				UVVert *uvw = mnmsh->M(mapID)->v;//msh->mapVerts(mapID);
				MNMapFace *uvwFace = mnmsh->M(mapID)->f;//msh->mapFaces(mapID);

				if ((uvw == NULL) || (uvwFace == NULL))
					{
					buffer.copyType = EMPTY;
					return;
					}

				buffer.verts.SetCount(numVerts);
				buffer.SetMapMNFaceCount(numFaces);


				for (int i = 0; i < numVerts; i++)
				{
					buffer.verts[i] = uvw[i];
				}
				for (i = 0; i < numFaces; i++)
				{
					buffer.uvwMNFaces[i]->MakePoly(uvwFace[i].deg, uvwFace[i].tv);
				}
				buffer.numFaces = numFaces;
				buffer.numRealFaces = mnmsh->numf;
			}
			
			else
			{
				buffer.copyType = POLYMESH_MAP;
				buffer.subID = subID;

				int numVerts,numFaces;
				int mapID = channelID;
				numVerts = mnmsh->M(mapID)->numv;//mnmsh->getNumMapVerts(mapID);
				numFaces = mnmsh->numf;
				if ((numVerts == 0) || (numFaces == 0))
					{
					buffer.copyType = EMPTY;
					return;
					}
				UVVert *uvw = mnmsh->M(mapID)->v;//msh->mapVerts(mapID);
				MNMapFace *uvwFace = mnmsh->M(mapID)->f;//msh->mapFaces(mapID);

				if ((uvw == NULL) || (uvwFace == NULL))
					{
					buffer.copyType = EMPTY;
					return;
					}

				buffer.w.SetCount(numVerts);
				buffer.SetMapMNFaceCount(numFaces);


				for (int i = 0; i < numVerts; i++)
				{
					buffer.w[i] = uvw[i][buffer.subID];
				}
				for (i = 0; i < numFaces; i++)
				{
					buffer.uvwMNFaces[i]->MakePoly(uvwFace[i].deg, uvwFace[i].tv);
				}
				buffer.numFaces = numFaces;
				buffer.numRealFaces = mnmsh->numf;

			}

		}
	}

	if (collapsedtobj) collapsedtobj->DeleteThis();


}

void TreeViewUtil::CopyToBuffer(int whichChannel)

{
	// CAL-09/09/03: make sure it doesn't go out of bound. (Defect #519712)
	if ((whichChannel < 0) || (whichChannel >= nodeList.Count()))
		return;

//clear out our old data
	buffer.Clear();
//get our geom type
	INode *node = nodeList[whichChannel]->node;

	int channelType = CHANNEL_MAP;

	if ( (nodeList[whichChannel]->channelType == TRISELCHANNEL) ||
         (nodeList[whichChannel]->channelType == POLYSELCHANNEL) )
		channelType = CHANNEL_SEL;
	else if ( (nodeList[whichChannel]->channelType == TRIGEOMCHANNEL) ||
         (nodeList[whichChannel]->channelType == POLYGEOMCHANNEL) )
		channelType = CHANNEL_GEOM;

	if (nodeList[whichChannel]->subID==-50)
		macroRecorder->FunctionCall(_T("channelInfo.CopyChannel"), 3, 0,
									mr_reftarg,theTreeViewUtil.nodeList[whichChannel]->node,
									mr_int,channelType-9,
									mr_int,theTreeViewUtil.nodeList[whichChannel]->channelID
									);
	else macroRecorder->FunctionCall(_T("channelInfo.CopySubChannel"), 4, 0,
									mr_reftarg,theTreeViewUtil.nodeList[whichChannel]->node,
									mr_int,channelType-9,
									mr_int,theTreeViewUtil.nodeList[whichChannel]->channelID,
									mr_int,nodeList[whichChannel]->subID
									);
	macroRecorder->EmitScript();

	CopyToBuffer(node, channelType, nodeList[whichChannel]->channelID, nodeList[whichChannel]->subID);

}

void TreeViewUtil::PasteToNode(INode *node, int channelType, int channelID, int subID)
{
	if (node)
	{
		Object* obj = node->GetObjectRef();

		if (obj)
		{

			MapChannelPaste* mod;
			mod = (MapChannelPaste*)CreateInstance(OSM_CLASS_ID, MAPCHANNELPASTE_CLASS_ID);

			if (mod)
			{
				theHold.Begin();
				theHold.Put(new UpdateUIRestore());
        
				mod->buffer.Copy(&buffer);

				mod->buffer.pasteToChannel = channelID;
				mod->buffer.pasteToChannelType = channelType;
				mod->buffer.pasteToSubID = subID;

				IDerivedObject* dobj = CreateDerivedObject(obj);
				dobj->AddModifier(mod);
				node->SetObjectRef(dobj);


				mod->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
				node->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
				node->NotifyDependents(FOREVER,PART_ALL,REFMSG_SUBANIM_STRUCTURE_CHANGED);

				theHold.Accept(GetString(IDS_PASTECHANNEL));

				Interface *ip = GetCOREInterface();

				int selCount= ip->GetSelNodeCount();

				BOOL select = FALSE;
				for (int i = 0; i < selCount; i++)
					{
					INode *selNode = ip->GetSelNode(i);
					if (selNode == node)
						select = TRUE;
					}
				if (select)
					ip->SelectNode(node,0);

				ip->RedrawViews(ip->GetTime());			
			}
		}

	}

}


void TreeViewUtil::PasteToNode(int whichChannel)
{
//get our geom type
	if ((whichChannel < 0) || (whichChannel >= nodeList.Count()))
		return;

	INode *node = nodeList[whichChannel]->node;
	if (node)
	{
		Object* obj = node->GetObjectRef();

		if (obj)
		{

			MapChannelPaste* mod;
			mod = (MapChannelPaste*)CreateInstance(OSM_CLASS_ID, MAPCHANNELPASTE_CLASS_ID);

			if (mod)
			{
				theHold.Begin();
				theHold.Put(new UpdateUIRestore());
        
				int mapID = nodeList[whichChannel]->channelID;
				mod->SetMapChannel(mapID);
				if (nodeList[whichChannel]->channelType == TRIGEOMCHANNEL)
					mod->SetUseMapChannel(FALSE);
				else mod->SetUseMapChannel(TRUE);

				mod->buffer.Copy(&buffer);

				mod->buffer.pasteToChannel = nodeList[whichChannel]->channelID;
				mod->buffer.pasteToChannelType = nodeList[whichChannel]->channelType;
				mod->buffer.pasteToSubID = nodeList[whichChannel]->subID;

				int channelType = CHANNEL_MAP;

				if ( (nodeList[whichChannel]->channelType == TRISELCHANNEL) ||
					(nodeList[whichChannel]->channelType == POLYSELCHANNEL) )
					channelType = CHANNEL_SEL;
				else if ( (nodeList[whichChannel]->channelType == TRIGEOMCHANNEL) ||
					(nodeList[whichChannel]->channelType == POLYGEOMCHANNEL) )
					channelType = CHANNEL_GEOM;

				if (nodeList[whichChannel]->subID == -50)
					macroRecorder->FunctionCall(_T("channelInfo.PasteChannel"), 3, 0,
									mr_reftarg,theTreeViewUtil.nodeList[whichChannel]->node,
									mr_int,channelType-9,
									mr_int,theTreeViewUtil.nodeList[whichChannel]->channelID
									);
				else macroRecorder->FunctionCall(_T("channelInfo.PasteSubChannel"), 4, 0,
									mr_reftarg,theTreeViewUtil.nodeList[whichChannel]->node,
									mr_int,channelType-9,
									mr_int,theTreeViewUtil.nodeList[whichChannel]->channelID,
									mr_int,nodeList[whichChannel]->subID
									);

				macroRecorder->EmitScript();

				IDerivedObject* dobj = CreateDerivedObject(obj);
				dobj->AddModifier(mod);
				node->SetObjectRef(dobj);


				mod->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
				node->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
				node->NotifyDependents(FOREVER,PART_ALL,REFMSG_SUBANIM_STRUCTURE_CHANGED);


				Interface *ip = GetCOREInterface();

				int selCount= ip->GetSelNodeCount();

				BOOL select = FALSE;
				for (int i = 0; i < selCount; i++)
					{
					INode *selNode = ip->GetSelNode(i);
					if (selNode == node)
						select = TRUE;
					}
				if (select)
					ip->SelectNode(node,0);

				ip->RedrawViews(ip->GetTime());			



				TSTR none(GetString(IDS_NONE));
				if (strcmp(none,nodeList[whichChannel]->channelName) == 0)
				{
					NameChannel(whichChannel);
				}
				theHold.Accept(GetString(IDS_PASTECHANNEL));
			}
		}

	}


}

void TreeViewUtil::AddChannel(INode *node)
{
	if (node)
	{
		Object* obj = node->GetObjectRef();

		if (obj)
		{

			MapChannelAdd* mod;
			mod = (MapChannelAdd*)CreateInstance(OSM_CLASS_ID, MAPCHANNELADD_CLASS_ID);

			if (mod)
			{
        
				theHold.Begin();
				theHold.Put(new UpdateUIRestore());

				IDerivedObject* dobj = CreateDerivedObject(obj);
				dobj->AddModifier(mod);
				node->SetObjectRef(dobj);
				mod->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);


				Interface *ip = GetCOREInterface();

				int selCount= ip->GetSelNodeCount();

				BOOL select = FALSE;
				for (int i = 0; i < selCount; i++)
					{
					INode *selNode = ip->GetSelNode(i);
					if (selNode == node)
						select = TRUE;
					}
				if (select)
					ip->SelectNode(node,0);

				ip->RedrawViews(ip->GetTime());

				theHold.Accept(GetString(IDS_ADDCHANNEL));

			}
		}
	}

}

void TreeViewUtil::AddChannel(int whichChannel)
{
//get our geom type
	if ((whichChannel < 0) || (whichChannel >= nodeList.Count()))
		return;

	INode *node = nodeList[whichChannel]->node;

	if (node)
	{
		AddChannel(node);
		macroRecorder->FunctionCall(_T("channelInfo.addChannel"), 1, 0,
						mr_reftarg,node);
		macroRecorder->EmitScript();

	}

}



void TreeViewUtil::Collapse()
{
//get our geom type
	int numItem = ListView_GetItemCount(listViewHWND);

	int sel =-1;
	for(int i=0;i<numItem;i++)
	{
		if ((ListView_GetItemState(listViewHWND,i,LVIS_SELECTED))&LVIS_SELECTED )
			sel = i;
	}

	if ((sel >=0) && (sel < nodeList.Count()))
	{
	INode *node = nodeList[sel]->node;
	if (node)
		Collapse(node);
	}

}
void TreeViewUtil::Collapse(INode *node)
	{

	node->InvalidateWS();

	Interface *ip= GetCOREInterface();

//LAM : added following to handle extension objects 8/2/00
	NotifyCollapseEnumProc PreNCEP(true,node);
	EnumGeomPipeline(&PreNCEP,node);

	Object *oldObj = node->GetObjectRef();
	SClass_ID sc = oldObj->SuperClassID();

	ip->StopCreating();
	ip->ClearPickMode();
	
	if (sc == GEN_DERIVOB_CLASS_ID || sc == WSM_DERIVOB_CLASS_ID)
	{
		// stack not empty, collapse stack 
		ObjectState os = oldObj->Eval(ip->GetTime());			

//LAM : modified following to handle polymesh 7/21/00
//		Object *obj = (Object*)os.obj->Clone();
		Object *obj = os.obj->CollapseObject();
		// Now get rid of superfulous objects 		
		if(os.obj == obj)
		{
			obj = (Object*)obj->Clone();
		}

		obj->SetSubSelState(0); // Reset the selection level to object level (in case it happens to have an edit mesh modifier
		oldObj->SetAFlag(A_LOCK_TARGET);
		node->SetObjectRef(obj);

		node->InvalidateWS();

//		iAppInternal->InvalidateObCache(node);
		node->NotifyDependents(FOREVER, 0, REFMSG_SUBANIM_STRUCTURE_CHANGED);
		oldObj->ClearAFlag(A_LOCK_TARGET);

//LAM : added following to handle extension objects 8/2/00
// Notify all mods and objs in the pipleine, that they have been collapsed
		NotifyCollapseEnumProc PostNCEP(false,node,obj);
		EnumGeomPipeline(&PostNCEP,oldObj);

		oldObj->MaybeAutoDelete();
	}


	/*
	Interface *ip = GetCOREInterface();

	TriObject *tobj = NULL;

	Matrix3 tm1, tm2;


		// Eval the node's object (exclude WSMs)
	Object *oldObj = node->GetObjectRef();
		
	// Check for NULL
	if (!oldObj) return;

		// Skip bones
	if (oldObj->ClassID()==Class_ID(BONE_CLASS_ID,0)) return;		

		// RB 6/14/99: Skip system nodes too
	Control *tmCont = node->GetTMController();
	if (tmCont && GetMasterController(tmCont)) return;

	NotifyCollapseEnumProc PreNCEP(true,node);
	EnumGeomPipeline(&PreNCEP,node);


	ObjectState os = oldObj->Eval(ip->GetTime());

	Object *obj = (Object*)os.obj->CollapseObject();

	if(obj == os.obj)
			obj = (Object*)obj->Clone();

		
	// Make the result of the stack the new object
	oldObj->SetAFlag(A_LOCK_TARGET);
	node->SetObjectRef(obj);		
	oldObj->ClearAFlag(A_LOCK_TARGET);

	// NS: 4/6/00 Notify all mods and objs in the pipleine, that they have been collapsed
	NotifyCollapseEnumProc PostNCEP(false,node,obj);
	EnumGeomPipeline(&PostNCEP,oldObj);

	oldObj->MaybeAutoDelete();

	SetSaveRequiredFlag(TRUE);
*/
	}
