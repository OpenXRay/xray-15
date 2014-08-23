/*===========================================================================*\
 | 
 |  FILE:	ScP_Read.cpp
 |			Utility that accesses a scripted plugin's parameters
 |			Demonstrates SDK -> MAX Script plugins techniques
 |			3D Studio MAX R3.0
 | 
 |  AUTH:   Harry Denholm
 |			Developer Consulting Group
 |			Copyright(c) Discreet 1999
 |
 |  HIST:	Started 7-4-99
 | 
\*===========================================================================*/

/*===========================================================================*\
 |
 |	NOTES:
 |	
 |	The scripted plugin MS code is found at the bottom of this CPP file - 
 |	copy and paste it into MAX Script while running R3, assign a new GameMtl
 |	to an object and then run this utility. Choosing the assigned material
 |	will load its custom values out using ParamBlock2 methods and display them
 |	in the interface.
 |
 |
 |	TECHNIQUE:
 |
 |	This is a simple example of accessing Scripted Plugins using the SDK. One
 |	of the main principles is that you, the developer, will probably know exactly
 |	how your scripted plugin is built so you can get the information out of it
 |	quite precisely. Even if you don't know the exact structure, however, these 
 |	techniques still do work to some extent.
 |
 |	Basically, when you create a scripted plugin, and define some parameters
 |	for it's interface, it will create ParamBlock2 instances for each block
 |	of parameters you define in script. These are accessable through the 
 |	class Animatable method GetParamBlock(...). In this way, once you get
 |	hold of a pointer to your new scripted plugin in the scene, you can 
 |	have total control over the state of your custom values.
 |
 |	Also, if your scripted plugin is overriding the UI of an existing plugin,
 |	or if it is extending the UI in some way, the original plugin that lies
 |	'underneith' the scripted plugin is called the delegate. You can get access
 |	to this original plugin, as it is stored at Reference 0 of your custom plugin.
 |	In this way, you can still control the delegate that your scripted plugin
 |	controls.
 |	If, however, the scripted plugin creates a brand new plugin type, the paramblock2
 |	items are stored upwards from Reference 0. It is advisable to use GetParamBlock
 |  in any case.
 |
\*===========================================================================*/



#include "Utility.h"


/*===========================================================================*\
 |	Class Descriptor
\*===========================================================================*/

class SCPClassDesc:public ClassDesc {
	public:
	int 			IsPublic()					{ return TRUE; }
	void *			Create( BOOL loading )		{ return &theSCPUtil; }
	const TCHAR *	ClassName()					{ return GetString(IDS_CLASSNAME); }
	SClass_ID		SuperClassID()				{ return UTILITY_CLASS_ID; }
	Class_ID 		ClassID()					{ return SCPUTIL_CLASSID; }
	const TCHAR* 	Category()					{ return GetString(IDS_CATEGORY);  }
	void ResetClassParams (BOOL fileReset);
};

static SCPClassDesc SCPUtilCD;
ClassDesc* GetSCPUtilDesc() {return &SCPUtilCD;}

// Reset all the utility values on File/Reset
void SCPClassDesc::ResetClassParams (BOOL fileReset) 
{
}



/*===========================================================================*\
 |	Dialog Handler for the Utility
 |  All the action happens in here - check the IDC_BTN1 handler to see
 |	how we access the scripted plugin's data
\*===========================================================================*/

static INT_PTR CALLBACK DefaultDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
		case WM_INITDIALOG:
			theSCPUtil.Init(hWnd);
			break;

		case WM_DESTROY:
			theSCPUtil.Destroy(hWnd);
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_CLOSE:
					theSCPUtil.iu->CloseUtility();
					break;

				case IDC_BTN1:
					{
					// Open a material browser to find the material to inspect
					Interface *ip = theSCPUtil.ip;

					BOOL newMat,cancel;
					MtlBase *mtlb = ip->DoMaterialBrowseDlg(hWnd,BROWSE_MATSONLY|BROWSE_INSTANCEONLY,newMat,cancel);


					if(!cancel&&mtlb)
					{
						// Check to see if this is one of ours
						if(mtlb->ClassID() == GAMEMTL_CLASSID)
						{

						Mtl *scp = (Mtl*)mtlb;

						// Get the number of pblock2s and references on the mtl
						int num_pblock2 = scp->NumParamBlocks();
						int num_refs = scp->NumRefs();

							// Get the custom parameters
							// We defined two custom parameters in our scripted plugin
							// called GM_Custom1 and GM_Custom2
							// When we walk through the retrieved paramblock2, we can
							// get access to the hardwired internal name and check that
							// against our own names, to see if its the parameter we want
							// This way, its position-independant : it doesn't matter
							// where the parameters ARE in the pblock2, we will find them.
							if(num_pblock2>0)
							{
								// Get the first paramblock2 (the only one in our scripted plugin)
								IParamBlock2 *pb2 = scp->GetParamBlock(0);
								// The the descriptor to 'decode'
								ParamBlockDesc2 *pdc = pb2->GetDesc();

								// Loop through all the defined parameters therein
								for(int i=0;i<pdc->count;i++)
								{
									// Get a ParamDef structure for the parameter
									ParamDef pD = pdc->paramdefs[i];
									

									// Now compare against our names
									// When we match against one we want, we get the 
									// ParamID from the ParamDef and pass it to GetValue of ParamBlock2
									// which will retrieve us the value
									if(stricmp(pD.int_name,"GM_Custom1")==0)
									{
										int itmp = pb2->GetInt(pD.ID,theSCPUtil.ip->GetTime());

										char s[255];
										sprintf(s,"%i",itmp);
										SetWindowText(GetDlgItem(hWnd,IDC_GM1),s);
									}

									if(stricmp(pD.int_name,"GM_Custom2")==0)
									{
										float ftmp = pb2->GetFloat(pD.ID,theSCPUtil.ip->GetTime());

										char s[255];
										sprintf(s,"%.1f%%",ftmp);
										SetWindowText(GetDlgItem(hWnd,IDC_GM2),s);
									}

								}

								// Mustn't forget to...
								pb2->ReleaseDesc();
							}


							// With a scripted plugin that overrides/extends an existing plugin,
							// the original "delegate" is kept as Reference 0.
							// If the scripted plugin is a brand new one, Ref 0 will be the first
							// paramblock2, and ref n will be paramblock2 n, if applicable.
							//
							// In this case, we get a poitner back to the original Standard Material
							// that we override, and get its Diffuse color
							Mtl *delegate = (Mtl*)scp->GetReference(0);
							theSCPUtil.cs->SetColor(delegate->GetDiffuse());

						}
						else
						{
							// The user chose something that wasn't a GameMtl class
							MessageBox(hWnd,"Chosen Material Is NOT a GAMEMTL!","Error",MB_OK);
						}
					}

					}
					break;
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

SCPUtility::SCPUtility()
{
	iu = NULL;
	ip = NULL;	
	hPanel = NULL;
}

SCPUtility::~SCPUtility()
{

}


void SCPUtility::BeginEditParams(Interface *ip,IUtil *iu) 
{
	this->iu = iu;
	this->ip = ip;

	hPanel = ip->AddRollupPage(
		hInstance,
		MAKEINTRESOURCE(IDD_SCPLUTIL),
		DefaultDlgProc,
		GetString(IDS_PARAMETERS),
		0);
}
	
void SCPUtility::EndEditParams(Interface *ip,IUtil *iu) 
{
	this->iu = NULL;
	this->ip = NULL;
	ip->DeleteRollupPage(hPanel);
	hPanel = NULL;
}


void SCPUtility::Init(HWND hWnd)
{
	cs = GetIColorSwatch(GetDlgItem(hWnd,IDC_DIFFUSE),Color(0,0,0),_T("Diffuse"));
}

void SCPUtility::Destroy(HWND hWnd)
{
	ReleaseIColorSwatch(cs);
	cs = NULL;
}



/*===========================================================================*\


-- ----------------------------------------------------------
-- Simple example Scripted Plugin
-- For use with the ScPRead Utility in MAXSDK\HOWTO\R3_EXAMPLES
--
-- Harry Denholm, Discreet 1999
--
-- ----------------------------------------------------------


plugin Material GameMtl
    name:"GameMtl"
    classID:#(0x67296df6, 0x0)
    extends:Standard replaceUI:True version:1
(

	-- the parameters list equates to a ParamBlock2
	-- Each one is put into a new PB2, which you can access 
	-- through the GetParamBlock() call
	-- The name we specify in here becomes the hardwired 'internal name' in the
	-- ParamDef structure
	parameters main rollout:params
	(
    	GM_Custom1  type:#integer animatable:true ui:GM_Custom1  default:10
	   	on GM_Custom1 set val do (
			Delegate.glossiness = val;
			Delegate.specularLevel = val;
			)

    	GM_Custom2  type:#float animatable:true ui:GM_Custom2 default:100
	   	on GM_Custom2 set val do (
			Delegate.opacity = val;
			Delegate.filterColor = Color (val*2) 0 0
			)
	)

	-- Define the UI layout
	-- We have two parameters that will affect the delegate plugin (standardMtl)
	-- and two that are custom, and controlled by the parameter definition above	
    rollout params "Default Parameters"
    (
		label sp1 height:15
		ColorPicker GMdiffuse "Diffuse Color :       " color:[250,0,0] across:2
		ColorPicker GMillum   "SelfIllumination :    " color:[0,0,0]
		label sp2 height:15
		group "Custom Paramters"
		(
			Spinner GM_Custom1 "Custom 1 : " range:[0,100,10] width:90 align:#center
			Slider GM_Custom2 "Custom 2 : " range:[0,100,100] width:150 align:#center
		)
		
		fn updateParams =
		(
			Delegate.diffuse = GMdiffuse.color
			Delegate.selfIllumColor = GMillum.color
		)
		
		on params open do ( updateParams() )
		on GMdiffuse changed col do ( updateParams() )
		on GMillum changed col do ( updateParams() )

	)
)

\*===========================================================================*/
