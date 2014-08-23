/*===========================================================================*\
 | 
 |  FILE:	MorpherView.cpp
 |			MorpherView Utility - demonstrates use of MorpherAPI access
 |			3D Studio MAX R3.0
 | 
 |  AUTH:   Harry Denholm
 |			Developer Consulting Group
 |			Copyright(c) Discreet 1999
 |
 |  HIST:	Started 4-4-99
 | 
\*===========================================================================*/

#include "MorpherView.h"


/*===========================================================================*\
 |	Class Descriptor
\*===========================================================================*/

class MorphViewClassDesc:public ClassDesc {
	public:
	int 			IsPublic()					{ return TRUE; }
	void *			Create( BOOL loading )		{ return &theMVUtility; }
	const TCHAR *	ClassName()					{ return GetString(IDS_CLASSNAME); }
	SClass_ID		SuperClassID()				{ return UTILITY_CLASS_ID; }
	Class_ID 		ClassID()					{ return MRVUTIL_CLASSID; }
	const TCHAR* 	Category()					{ return GetString(IDS_CATEGORY);  }
	void ResetClassParams (BOOL fileReset);
};

static MorphViewClassDesc MorpherViewCD;
ClassDesc* GetMorpherViewDesc() {return &MorpherViewCD;}

// Reset all the utility values on File/Reset
void MorphViewClassDesc::ResetClassParams (BOOL fileReset) 
{
}



/*===========================================================================*\
 |	Dialog Handler for the MorpherView Utility
\*===========================================================================*/

static INT_PTR CALLBACK DefaultDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int id = LOWORD(wParam);
	int notify = HIWORD(wParam);

	switch (msg) {
		case WM_INITDIALOG:
			theMVUtility.Init(hWnd);
			break;

		case WM_DESTROY:
			theMVUtility.Destroy(hWnd);
			break;

		case WM_COMMAND:
			
			// If the user chooses a channel from the list,
			// Ask the LoadChannelInfo fn to update the info for us
			if (notify==CBN_SELCHANGE){
				if(id==IDC_MCLIST){
					int mcSel = SendMessage(GetDlgItem(hWnd, IDC_MCLIST), CB_GETCURSEL, 0, 0);
					theMVUtility.LoadChannelInfo(hWnd,mcSel);
				}
			}

			switch (LOWORD(wParam)) {
				case IDC_CLOSE:
					theMVUtility.iu->CloseUtility();
					break;

				// Start the pick mode to get a morpher pointer
				case IDC_GET:
					{
						theModPickmode_MorpherView.mvup = &theMVUtility;
						theMVUtility.ip->SetPickMode(&theModPickmode_MorpherView);
					break;}


				// Ask the channel for a memory usage approximation
				case IDC_GETMEM:
					{
					MorphR3 *mp = theMVUtility.mp;
					int mcSel = SendMessage(GetDlgItem(hWnd, IDC_MCLIST), CB_GETCURSEL, 0, 0);

						if(mp)
						{
							float tmSize = 0.0f;
							tmSize += mp->chanBank[mcSel].getMemSize();
							char s[50];
							sprintf(s,"%i KB", (int)tmSize/1000);
							SetWindowText(GetDlgItem(hWnd,IDC_GETMEM),s);					
						}
					break;}


				// Rebuild the channel
				// If it has a INode connection, rebuild from that
				// Otherwise, just call rebuildChannel()
				case IDC_REBUILD:
					{
					MorphR3 *mp = theMVUtility.mp;
					int mcSel = SendMessage(GetDlgItem(hWnd, IDC_MCLIST), CB_GETCURSEL, 0, 0);

						if(mp)
						{
							if(mp->chanBank[mcSel].mConnection)
							{
								mp->chanBank[mcSel].buildFromNode(mp->chanBank[mcSel].mConnection);
							}
							else if(mp->chanBank[mcSel].mActive)
							{
								mp->chanBank[mcSel].rebuildChannel();
							}

							// Refresh system
							mp->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
							theMVUtility.ip->RedrawViews(theMVUtility.ip->GetTime());
							theMVUtility.LoadChannelInfo(hWnd,mcSel);
						}
					break;}


				// Delete a channel, and reassign it a new paramblock (necessary)
				case IDC_DEL:
					{
					MorphR3 *mp = theMVUtility.mp;
					int mcSel = SendMessage(GetDlgItem(hWnd, IDC_MCLIST), CB_GETCURSEL, 0, 0);

						if(mp)
						{
							if(mp->chanBank[mcSel].mConnection) 
								mp->DeleteReference(101+mcSel);

							// Ask channel to reset itself
							mp->chanBank[mcSel].ResetMe();

							// Reassign paramblock info
							ParamBlockDescID *channelParams = new ParamBlockDescID[1];

							ParamBlockDescID add;
							add.type=TYPE_FLOAT;
							add.user=NULL;
							add.animatable=TRUE;
							add.id=1;
							channelParams[0] = add;

							mp->MakeRefByID(FOREVER, 1+mcSel, CreateParameterBlock(channelParams,1,1));	
							assert(mp->chanBank[mcSel].cblock);

							Control *c = (Control*)CreateInstance(CTRL_FLOAT_CLASS_ID,GetDefaultController(CTRL_FLOAT_CLASS_ID)->ClassID());

							mp->chanBank[mcSel].cblock->SetValue(0,0,0.0f);
							mp->chanBank[mcSel].cblock->SetController(0,c);

							delete channelParams;

							// Refresh system
							mp->Update_channelFULL();
							mp->Update_channelParams();	
							mp->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
							mp->NotifyDependents(FOREVER,PART_ALL,REFMSG_SUBANIM_STRUCTURE_CHANGED);
							theMVUtility.ip->RedrawViews(theMVUtility.ip->GetTime());
							theMVUtility.LoadMorpherInfo(hWnd);
							theMVUtility.LoadChannelInfo(hWnd,mcSel);
						}
					break;}


				// Pick a node from the scene, and use it to build a new channel
				// See MorphPick.cpp for code
				case IDC_BUILDNODE:
					{
					MorphR3 *mp = theMVUtility.mp;
					int mcSel = SendMessage(GetDlgItem(hWnd, IDC_MCLIST), CB_GETCURSEL, 0, 0);

						if(mp)
						{
							theTargetPickMode_MorpherView.mvup = &theMVUtility;
							theTargetPickMode_MorpherView.idx = mcSel;
							theTargetPickMode_MorpherView.mp = mp;
							theMVUtility.ip->SetPickMode(&theTargetPickMode_MorpherView);
						}
					break;}
			}
			break;


		default:
			return FALSE;
	}
	return TRUE;
}



/*===========================================================================*\
 |  Utility implimentations
\*===========================================================================*/

MorphViewUtil::MorphViewUtil()
{
	iu = NULL;
	ip = NULL;	
	hPanel = NULL;
	pickBut = NULL;
	bnode = NULL;
	Wnode = NULL;
	mp = NULL;
}

MorphViewUtil::~MorphViewUtil()
{

}

/*===========================================================================*\
 |	Load and Unload our UI panel
\*===========================================================================*/

void MorphViewUtil::BeginEditParams(Interface *ip,IUtil *iu) 
{
	this->iu = iu;
	this->ip = ip;

	hPanel = ip->AddRollupPage(
		hInstance,
		MAKEINTRESOURCE(IDD_MRV_UTIL),
		DefaultDlgProc,
		GetString(IDS_PARAMETERS),
		0);
}
	
void MorphViewUtil::EndEditParams(Interface *ip,IUtil *iu) 
{
	this->iu = NULL;
	this->ip = NULL;
	ip->DeleteRollupPage(hPanel);
	hPanel = NULL;
	pickBut = NULL;
	bnode = NULL;
	Wnode = NULL;
	mp = NULL;
}


/*===========================================================================*\
 |	Initialize the UI panel - register the two custom pick buttons
\*===========================================================================*/

void MorphViewUtil::Init(HWND hWnd)
{
	pickBut = GetICustButton(GetDlgItem(hWnd,IDC_GET));
	pickBut->SetHighlightColor(GREEN_WASH);
	pickBut->SetType(CBT_CHECK);

	bnode = GetICustButton(GetDlgItem(hWnd,IDC_BUILDNODE));
	bnode->SetHighlightColor(GREEN_WASH);
	bnode->SetType(CBT_CHECK);

	hPanel = hWnd;
}


/*===========================================================================*\
 |	Unload the UI panel
\*===========================================================================*/

void MorphViewUtil::Destroy(HWND hWnd)
{
	if(pickBut)
	{
		ReleaseICustButton(pickBut);
		pickBut = NULL;
	}
	if(bnode)
	{
		ReleaseICustButton(bnode);
		bnode = NULL;
	}

}

