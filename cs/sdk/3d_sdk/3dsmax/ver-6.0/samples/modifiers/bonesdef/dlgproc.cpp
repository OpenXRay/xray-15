/**********************************************************************
 
	FILE: DlgProc.cpp

	DESCRIPTION:  Main DLg proc to handle windows stuff mainly the list box and buttons

	CREATED BY: Peter Watje

	HISTORY: 8/5/98




 *>	Copyright (c) 1998, All Rights Reserved.
 **********************************************************************/

#include "mods.h"
#include "iparamm.h"
#include "shape.h"
#include "spline3d.h"
#include "splshape.h"
#include "linshape.h"

// This uses the linked-list class templates
#include "linklist.h"
#include "bonesdef.h"
#include "macrorec.h"


static HIMAGELIST hParamImages = NULL;


CreateCrossSectionMode::CreateCrossSectionMode(BonesDefMod* bmod, IObjParam *i) :
                        fgProc(bmod), eproc(bmod,i) 
	{
	mod=bmod;
	}


void BoneTimeChangeCallback::TimeChanged(TimeValue t){
	if ( (mod->ModeBoneIndex >=0) && (mod->ModeBoneIndex < mod->BoneData.Count()) )
		{
		float val;
		mod->pblock_param->GetValue(skin_local_squash,t, val, FOREVER, mod->ModeBoneIndex);
		ISpinnerControl* spin = SetupFloatSpinner(mod->hParam, IDC_SQUASHSPIN, IDC_SQUASH, 0.0f, 10.0f, val, .1f);
		ReleaseISpinner(spin);

		float radius;
		if (mod->ModeBoneEnvelopeSubType<4)
			{
			
			Interval iv;
			int tempID = mod->ModeBoneIndex;
			tempID = mod->ConvertSelectedBoneToListID(tempID)+1;
			if ((mod->ModeBoneIndex >= 0) && (mod->ModeBoneIndex < mod->BoneData.Count()))
				{
				if ((mod->ModeBoneEnvelopeIndex >= 0) && (mod->ModeBoneEnvelopeIndex < mod->BoneData[mod->ModeBoneIndex].CrossSectionList.Count()))
					mod->BoneData[mod->ModeBoneIndex].CrossSectionList[mod->ModeBoneEnvelopeIndex].InnerControl->GetValue(t,&radius,iv);
				}

			}
		else if (mod->ModeBoneEnvelopeSubType<8)
			{
			Interval iv;
			int tempID = mod->ModeBoneIndex;
			tempID = mod->ConvertSelectedBoneToListID(tempID)+1;
			if ((mod->ModeBoneIndex >= 0) && (mod->ModeBoneIndex < mod->BoneData.Count()))
				{
				if ((mod->ModeBoneEnvelopeIndex >= 0) && (mod->ModeBoneEnvelopeIndex < mod->BoneData[mod->ModeBoneIndex].CrossSectionList.Count()))
					mod->BoneData[mod->ModeBoneIndex].CrossSectionList[mod->ModeBoneEnvelopeIndex].OuterControl->GetValue(t,&radius,iv);
				}

			}

		SuspendAnimate();
		mod->pblock_param->SetValue(skin_cross_radius,t,radius);
		ResumeAnimate();

		}
	if (mod->pPainterInterface && mod->pPainterInterface->InPaintMode())
		{
		mod->CanvasStartPaint();
		}
}



void BonesDefMod::RegisterClasses()
	{
	if (!hParamImages) {
		HBITMAP hBitmap, hMask;	
		hParamImages = ImageList_Create(16, 15, TRUE, 4, 0);
		hBitmap = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_PARAMS));
		hMask   = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_PARAMS_MASK));
		ImageList_Add(hParamImages,hBitmap,hMask);
		DeleteObject(hBitmap);
		DeleteObject(hMask);
		}

	}



void SpinnerOff(HWND hWnd,int SpinNum,int Winnum)
{	
	ISpinnerControl *spin2 = GetISpinner(GetDlgItem(hWnd,SpinNum));
	if (spin2 != NULL)
		{
		spin2->Enable(FALSE);
		EnableWindow(GetDlgItem(hWnd,Winnum),FALSE);
		ReleaseISpinner(spin2);
		}

};

void SpinnerOn(HWND hWnd,int SpinNum,int Winnum)
{
	ISpinnerControl *spin2 = GetISpinner(GetDlgItem(hWnd,SpinNum));
	if (spin2 != NULL)
		{
		spin2->Enable(TRUE);
		EnableWindow(GetDlgItem(hWnd,Winnum),TRUE);
		ReleaseISpinner(spin2);
		}

};

TCHAR *BonesDefMod::GetBoneName(int index)
{

	if (BoneData[index].Node != NULL)
		{
		Class_ID bid(BONE_CLASS_ID,0);
		ObjectState os = BoneData[index].Node->EvalWorldState(RefFrame);

		if (( os.obj->ClassID() == bid) && (BoneData[index].name.Length()) )
			{
			return BoneData[index].name;
			}
		else return BoneData[index].Node->GetName();

		}

	return NULL;

}

static INT_PTR CALLBACK PasteDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	BonesDefMod *mod = (BonesDefMod*)GetWindowLongPtr(hWnd,GWLP_USERDATA);

	switch (msg) {
		case WM_INITDIALOG:
			{
			mod = (BonesDefMod*)lParam;
			SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);

			for (int i=0; i < mod->BoneData.Count(); i++)
				{
				TCHAR *temp;
				temp = mod->GetBoneName(i);
				if (temp)
					{
					TCHAR title[200];

					_tcscpy(title,temp);

					SendMessage(GetDlgItem(hWnd,IDC_LIST1),
						LB_ADDSTRING,0,(LPARAM)(TCHAR*)title);



					}

				}

			break;

			}

		case WM_COMMAND:
			switch (LOWORD(wParam)) 
				{
				case IDOK:
					{
					int listCt = SendMessage(GetDlgItem(hWnd,IDC_LIST1),
						LB_GETCOUNT,0,0);
					int selCt =  SendMessage(GetDlgItem(hWnd,IDC_LIST1),
						LB_GETSELCOUNT ,0,0);
					int *selList;
					selList = new int[selCt];

					SendMessage(GetDlgItem(hWnd,IDC_LIST1),
						LB_GETSELITEMS  ,(WPARAM) selCt,(LPARAM) selList);
					mod->pasteList.SetCount(selCt);
					for (int i=0; i < selCt; i++)
						{
						mod->pasteList[i] = selList[i];
						}

					delete [] selList;

					EndDialog(hWnd,1);
					break;
					}
				case IDCANCEL:
					mod->pasteList.ZeroCount();
					EndDialog(hWnd,0);
					break;
				}
			break;

		
		case WM_CLOSE:
			mod->pasteList.ZeroCount();
			EndDialog(hWnd, 0);
//			DestroyWindow(hWnd);			
			break;

	
		default:
			return FALSE;
		}
	return TRUE;
	}



BOOL BMDModEnumProc::proc (ModContext *mc) {
	if (mc->localData == NULL) return TRUE;


	BoneModData *bmd = (BoneModData *) mc->localData;
	bmdList.Append(1,&bmd);
	return TRUE;
}




BOOL MapDlgProc::DlgProc(
		TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
			
	{
//	int i;
	static BOOL firstMouse = TRUE;

	static BOOL ccChange = FALSE;
	static float prevQ = 0.0f;

	switch (msg) {
		case WM_INITDIALOG:
			{

			mod->hParam = hWnd;
			mod->RefillListBox();

			mod->iCrossSectionButton = GetICustButton(GetDlgItem(hWnd, IDC_CREATE_CROSS_SECTION));
			mod->iCrossSectionButton->SetType(CBT_CHECK);
			mod->iCrossSectionButton->SetHighlightColor(GREEN_WASH);

			mod->iEditEnvelopes = GetICustButton(GetDlgItem(hWnd, IDC_EDITENVLOPES));
			mod->iEditEnvelopes->SetType(CBT_CHECK);
			mod->iEditEnvelopes->SetHighlightColor(SUBOBJ_COLOR);

	
			mod->iPaintButton = GetICustButton(GetDlgItem(hWnd, IDC_PAINT));
			mod->iPaintButton->SetType(CBT_CHECK);
			mod->iPaintButton->SetHighlightColor(GREEN_WASH);


			mod->iCrossSectionButton->Disable();
			mod->iPaintButton->Disable();
			EnableWindow(GetDlgItem(hWnd,IDC_CREATE_REMOVE_SECTION),FALSE);


			EnableWindow(GetDlgItem(hWnd,IDC_FILTER_VERTICES_CHECK),FALSE);
			EnableWindow(GetDlgItem(hWnd,IDC_FILTER_BONES_CHECK),FALSE);
			EnableWindow(GetDlgItem(hWnd,IDC_FILTER_ENVELOPES_CHECK),FALSE);
			EnableWindow(GetDlgItem(hWnd,IDC_DRAWALL_ENVELOPES_CHECK),FALSE);
			EnableWindow(GetDlgItem(hWnd,IDC_DRAW_VERTICES_CHECK),FALSE);

			ICustButton *iBut = GetICustButton(GetDlgItem(hWnd, IDC_EXCLUDE));
			iBut->Enable(FALSE);
			ReleaseICustButton(iBut);
			iBut = GetICustButton(GetDlgItem(hWnd, IDC_INCLUDE));
			iBut->Enable(FALSE);
			ReleaseICustButton(iBut);
			iBut = GetICustButton(GetDlgItem(hWnd, IDC_SELECT_EXCLUDED));
			iBut->Enable(FALSE);
			ReleaseICustButton(iBut);



			SpinnerOff(hWnd,IDC_PAINT_STR_SPIN2,IDC_PAINT_STR2);
			SpinnerOff(hWnd,IDC_FEATHERSPIN,IDC_FEATHER);
			SpinnerOff(hWnd,IDC_SRADIUSSPIN,IDC_SRADIUS);
			SpinnerOff(hWnd,IDC_EFFECTSPIN,IDC_EFFECT);
			SpinnerOff(hWnd,IDC_ERADIUSSPIN,IDC_ERADIUS);


  
			int ct = 0;
			for (int i =0; i < mod->BoneData.Count(); i++)
				{
				if (mod->BoneData[i].Node) ct++;
				}
			if (ct ==0)
				EnableWindow(GetDlgItem(hWnd,IDC_REMOVE),FALSE);
			else EnableWindow(GetDlgItem(hWnd,IDC_REMOVE),TRUE);

			mod->ModeBoneIndex = mod->LastSelected;
			
			if ((mod->ModeBoneIndex < 0) && (ct >0))
				{
				for (int i =0; i < mod->BoneData.Count(); i++)
					{
					if (mod->BoneData[i].Node) 
						{
						mod->ModeBoneIndex = i;
						i = mod->BoneData.Count();
						}
					}

				}


			int fsel = mod->ConvertSelectedBoneToListID(mod->ModeBoneIndex);
			 
			SendMessage(GetDlgItem(mod->hParam,IDC_LIST1),
				LB_SETCURSEL ,fsel,0);
			



				//set check
	

			mod->RegisterClasses();
			mod->iParams = GetICustToolbar(GetDlgItem(hWnd,IDC_BONE_TOOLBAR));
			mod->iParams->SetBottomBorder(FALSE);	
			mod->iParams->SetImage(hParamImages);

			mod->iParams->AddTool(
					ToolButtonItem(CTB_CHECKBUTTON,
					13, 12, 25, 24, 16, 15, 23, 22, ID_ABSOLUTE));
			mod->iParams->AddTool(
					ToolButtonItem(CTB_CHECKBUTTON,
					14, 15, 27, 26, 16, 15, 23, 22, ID_DRAW_ENVELOPE));
			mod->iParams->AddTool(
					ToolButtonItem(CTB_PUSHBUTTON,
					0, 0, 1, 1, 16, 15, 23, 22, ID_FALLOFF));
			mod->iParams->AddTool(
					ToolButtonItem(CTB_PUSHBUTTON,
					16, 16, 18, 18, 16, 15, 23, 22, ID_COPY));
			mod->iParams->AddTool(
					ToolButtonItem(CTB_PUSHBUTTON,
					17, 17, 19, 19, 16, 15, 23, 22, ID_PASTE));

	
//			mod->iLock    = mod->iParams->GetICustButton(ID_LOCK);
			mod->iAbsolute= mod->iParams->GetICustButton(ID_ABSOLUTE);
			mod->iEnvelope= mod->iParams->GetICustButton(ID_DRAW_ENVELOPE);
			mod->iFalloff = mod->iParams->GetICustButton(ID_FALLOFF);
			mod->iCopy = mod->iParams->GetICustButton(ID_COPY);
			mod->iPaste = mod->iParams->GetICustButton(ID_PASTE);

//			mod->iLock->SetTooltip(TRUE,GetString(IDS_PW_LOCK));
			mod->iAbsolute->SetTooltip(TRUE,GetString(IDS_PW_ABSOLUTE));
			mod->iEnvelope->SetTooltip(TRUE,GetString(IDS_PW_ENVELOPE));
			mod->iFalloff->SetTooltip(TRUE,GetString(IDS_PW_FALLOFFSLOW));
			mod->iCopy->SetTooltip(TRUE,GetString(IDS_PW_COPY));
			mod->iPaste->SetTooltip(TRUE,GetString(IDS_PW_PASTE));

			if (mod->CopyBuffer.CList.Count() == 0) mod->iPaste->Enable(FALSE);

			FlyOffData fdata1[] = {
				{ 8,  8,  9,  9},
				{10, 10, 11, 11}
				};
			int lock=0;
//			mod->iLock->SetFlyOff(2,fdata1,mod->ip->GetFlyOffTime(),lock,FLY_DOWN);

//			FlyOffData fdata2[] = {
//				{13, 13, 13, 13},
//				{12, 12, 12, 12}
//				};
			int absolute=0;
//			mod->iAbsolute->SetFlyOff(2,fdata2,mod->ip->GetFlyOffTime(),absolute,FLY_DOWN);

			FlyOffData fdata3[] = {
				{14, 14, 14, 14},
				{15, 15, 15, 15}
				};
			int envelope=0;
//			mod->iEnvelope->SetFlyOff(2,fdata3,mod->ip->GetFlyOffTime(),envelope,FLY_DOWN);

			FlyOffData fdata4[] = {
				{ 0,  0,  1,  1},
				{ 2,  2,  3,  3},
				{ 4,  4,  5,  5},
				{ 6,  6,  7,  7},
				};
			int falloff=0;
			mod->iFalloff->SetFlyOff(4,fdata4,mod->ip->GetFlyOffTime(),falloff,FLY_DOWN);

			FlyOffData fdata5[] = {
				{ 17, 17, 19, 19},
				{ 20, 20, 21, 21},
				{ 22, 22, 23, 23},
				};
			int paste=0;
			mod->iPaste->SetFlyOff(3,fdata5,mod->ip->GetFlyOffTime(),paste,FLY_DOWN);


			if ((mod->ModeBoneIndex < (mod->BoneData.Count())) && (ct>0) && (mod->ModeBoneIndex!=-1))
				{
				mod->ResetSelection();
				}
 // bug fix 206160 and 207093 9/8/99	watje
			if (mod->iAbsolute!=NULL)
				mod->iAbsolute->Disable();
			if (mod->iEnvelope!=NULL)
				mod->iEnvelope->Disable();
			if (mod->iFalloff!=NULL)
				mod->iFalloff->Disable();
			if (mod->iCopy!=NULL)
				mod->iCopy->Disable();
			if (mod->iPaste!=NULL)
				mod->iPaste->Disable();


			if ((mod->ModeBoneIndex >=0) && (mod->ModeBoneIndex <mod->pblock_param->Count(skin_local_squash)))
				{
				float val;
				mod->pblock_param->GetValue(skin_local_squash,mod->ip->GetTime(), val, FOREVER, mod->ModeBoneIndex);
				ISpinnerControl* spin = SetupFloatSpinner(hWnd, IDC_SQUASHSPIN, IDC_SQUASH, 0.0f, 10.0f, val, .1f);
				ReleaseISpinner(spin);

				}
			SpinnerOff(hWnd,IDC_SQUASHSPIN,IDC_SQUASH);

			EnableWindow(GetDlgItem(hWnd,IDC_NORMALIZE_CHECK),FALSE);
			EnableWindow(GetDlgItem(hWnd,IDC_RIGID_CHECK),FALSE);
			EnableWindow(GetDlgItem(hWnd,IDC_RIGIDHANDLES_CHECK),FALSE);

			break;
			}
		case WM_CUSTEDIT_ENTER :
			switch (LOWORD(wParam)) {
			case IDC_SQUASH:
				{
				ISpinnerControl *spin = GetISpinner(GetDlgItem(hWnd, IDC_SQUASHSPIN));

				if (spin)
					{
					float q = spin->GetFVal();
					ReleaseISpinner(spin);

					if ((ccChange) && (q != prevQ))
						{
						ccChange = FALSE;
						BOOL accept = FALSE;
						if (!theHold.Holding())
							{
							theHold.Begin();
							accept = TRUE;
							}

//						mod->pblock_param->SetValue(skin_local_squash,mod->ip->GetTime(),q,mod->ModeBoneIndex);

						theHold.Put(new UpdateSquashUIRestore(mod,prevQ,mod->ModeBoneIndex));

						if (accept)
							{							
							theHold.Accept(GetString(IDS_PW_EDIT_ENVELOPE));
							}
						}



					}

				break;
				}
			case IDC_ERADIUS:
				if (Animating())
					{
					Control *c = mod->pblock_param->GetController(skin_cross_radius);
					float r;
					mod->pblock_param->GetValue(skin_cross_radius,mod->ip->GetTime(),r,FOREVER);
					if (c)
						{
						IKeyControl *ikc = GetKeyControlInterface(c);
						if (ikc) ikc->SetNumKeys(0);
						}
					

					macroRecorder->Disable();
					mod->pblock_param->SetValue(skin_cross_radius,0,r);
					macroRecorder->Enable();

					mod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
					mod->ip->RedrawViews(mod->ip->GetTime());
					}
				else
					{
					mod->updateP = TRUE;
					mod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
					mod->ip->RedrawViews(mod->ip->GetTime());
					}
				if (mod->ModeBoneEnvelopeSubType<4)
					{
					float inner;
					Interval iv;
					int tempID = mod->ModeBoneIndex;
					tempID = mod->ConvertSelectedBoneToListID(tempID)+1;
					if ((mod->ModeBoneIndex >=0) && (mod->ModeBoneIndex < mod->BoneData.Count()))
						{
						if ((mod->BoneData[mod->ModeBoneIndex].Node) && (mod->ModeBoneEnvelopeIndex >= 0) &&
							(mod->ModeBoneEnvelopeIndex < mod->BoneData[mod->ModeBoneIndex].CrossSectionList.Count()))
							{

							mod->BoneData[mod->ModeBoneIndex].CrossSectionList[mod->ModeBoneEnvelopeIndex].InnerControl->GetValue(mod->currentTime,&inner,iv);
		
							macroRecorder->FunctionCall(_T("skinOps.setInnerRadius"), 4, 0, mr_reftarg, mod, mr_int, tempID,mr_int,mod->ModeBoneEnvelopeIndex+1, mr_float,inner);
							}
						}
					}
				else if (mod->ModeBoneEnvelopeSubType<8)
					{
					float outer;
					Interval iv;
					int tempID = mod->ModeBoneIndex;
					tempID = mod->ConvertSelectedBoneToListID(tempID)+1;

					if ((mod->ModeBoneIndex >=0) && (mod->ModeBoneIndex < mod->BoneData.Count()))
						{
						if ((mod->BoneData[mod->ModeBoneIndex].Node) && (mod->ModeBoneEnvelopeIndex >= 0) &&
							(mod->ModeBoneEnvelopeIndex < mod->BoneData[mod->ModeBoneIndex].CrossSectionList.Count()))
							{
							mod->BoneData[mod->ModeBoneIndex].CrossSectionList[mod->ModeBoneEnvelopeIndex].OuterControl->GetValue(mod->currentTime,&outer,iv);

							macroRecorder->FunctionCall(_T("skinOps.setOuterRadius"), 4, 0, mr_reftarg, mod, mr_int, tempID, mr_int,mod->ModeBoneEnvelopeIndex+1, mr_float,outer);
							}
						}
					}

				break;
			case IDC_EFFECT:
				{
				ModContextList mcList;		
				INodeTab nodes;

				mod->ip->GetModContexts(mcList,nodes);
				int objects = mcList.Count();
				for ( int i = 0; i < objects; i++ ) 
					{
					BoneModData *bmd = (BoneModData*)mcList[i]->localData;
					bmd->effect = -1.0f;
					}
				mod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
				mod->ip->RedrawViews(mod->ip->GetTime());


				}
				break;

			}

		case CC_SPINNER_BUTTONDOWN:
			switch (LOWORD(wParam)) {
			case IDC_EFFECTSPIN:
				mod->HoldWeights();
				break;
			case IDC_ERADIUSSPIN:
				firstMouse = TRUE;
				macroRecorder->Disable();
				break;

			case IDC_SQUASHSPIN:
				{
				firstMouse = TRUE;

				break;
				}



			}
			break;

		case CC_SPINNER_CHANGE:
			switch( LOWORD(wParam) ) {
				case IDC_SQUASHSPIN:
					{
					ccChange = TRUE;
					if (firstMouse)
						{
						BOOL accept = FALSE;
						firstMouse = FALSE;
						float q1;
						mod->pblock_param->GetValue(skin_local_squash,mod->ip->GetTime(),q1,FOREVER,mod->ModeBoneIndex);

						if (!theHold.Holding())
							{
							theHold.Begin();
							accept = TRUE;
							}

						theHold.Put(new UpdateSquashUIRestore(mod,q1,mod->ModeBoneIndex));


						if (accept)
							{
								
							theHold.Accept(GetString(IDS_PW_EDIT_ENVELOPE));
							}
						}

					mod->pblock_param->GetValue(skin_local_squash,mod->ip->GetTime(),prevQ,FOREVER,mod->ModeBoneIndex);
				
					float q = ((ISpinnerControl *)lParam)->GetFVal();
					theHold.Suspend();
					mod->pblock_param->SetValue(skin_local_squash,mod->ip->GetTime(),q,mod->ModeBoneIndex);
					theHold.Resume();
					break;
					}

				case IDC_ERADIUSSPIN:
					float r;
					mod->pblock_param->GetValue(skin_cross_radius,0,r,FOREVER);
					if (Animating())
						{
						Control *c = mod->pblock_param->GetController(skin_cross_radius);
						float r;
						mod->pblock_param->GetValue(skin_cross_radius,mod->ip->GetTime(),r,FOREVER);
						if (c)
							{
							IKeyControl *ikc = GetKeyControlInterface(c);
							if (ikc) ikc->SetNumKeys(0);
							}
						
						mod->pblock_param->SetValue(skin_cross_radius,0,r);
//					mod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
//					mod->ip->RedrawViews(mod->ip->GetTime());
						}
					else
						{

						}


					if (mod->ModeBoneEnvelopeSubType<4)
						{
//						float inner;
						Interval iv;
						int tempID = mod->ModeBoneIndex;
						tempID = mod->ConvertSelectedBoneToListID(tempID)+1;
						if ( (mod->ModeBoneIndex >=0) && (mod->ModeBoneIndex<mod->BoneData.Count()))
							{
							if ((mod->ModeBoneEnvelopeIndex>=0) && (mod->ModeBoneEnvelopeIndex<mod->BoneData[mod->ModeBoneIndex].CrossSectionList.Count()))
								{
								BOOL accept = FALSE;
								if (!theHold.Holding())
									{
									theHold.Begin();
									accept = TRUE;
									}

								mod->BoneData[mod->ModeBoneIndex].CrossSectionList[mod->ModeBoneEnvelopeIndex].InnerControl->SetValue(mod->currentTime,&r,TRUE,CTRL_ABSOLUTE);

								
								if (accept)
									{
									theHold.Put(new UpdatePRestore(mod));
									theHold.Accept(GetString(IDS_PW_EDIT_ENVELOPE));
									}
								else
									{	
									if (firstMouse)
										theHold.Put(new UpdatePRestore(mod));
									firstMouse = FALSE;

									}

	
								mod->LimitOuterRadius(r);

//						mod->BoneData[mod->ModeBoneIndex].CrossSectionList[mod->ModeBoneEnvelopeIndex].InnerControl->GetValue(0,&inner,iv);
								macroRecorder->Enable();
								macroRecorder->FunctionCall(_T("skinOps.setInnerRadius"), 4, 0, mr_reftarg, mod, mr_int, tempID,mr_int,mod->ModeBoneEnvelopeIndex+1, mr_float,r);
								macroRecorder->Disable();
								}
							}

						}
					else if (mod->ModeBoneEnvelopeSubType<8)
						{
//						float outer;
						Interval iv;
						int tempID = mod->ModeBoneIndex;
						tempID = mod->ConvertSelectedBoneToListID(tempID)+1;

						if ((mod->ModeBoneIndex >=0) && (mod->ModeBoneIndex<mod->BoneData.Count()))
							{

//						mod->BoneData[mod->ModeBoneIndex].CrossSectionList[mod->ModeBoneEnvelopeIndex].OuterControl->GetValue(0,&outer,iv);
							if ((mod->ModeBoneEnvelopeIndex>=0) && (mod->ModeBoneEnvelopeIndex<mod->BoneData[mod->ModeBoneIndex].CrossSectionList.Count()))
								{
								BOOL accept = FALSE;
								if (!theHold.Holding())
									{
									theHold.Begin();
									accept = TRUE;
									}
								mod->BoneData[mod->ModeBoneIndex].CrossSectionList[mod->ModeBoneEnvelopeIndex].OuterControl->SetValue(mod->currentTime,&r,TRUE,CTRL_ABSOLUTE);
								
								if (accept)
									{
									theHold.Put(new UpdatePRestore(mod));
									theHold.Accept(GetString(IDS_PW_EDIT_ENVELOPE));
									}
								else 
									{
									if (firstMouse)
										theHold.Put(new UpdatePRestore(mod));
									firstMouse = FALSE;
									}

								mod->LimitInnerRadius(r);
	
								macroRecorder->Enable();
								macroRecorder->FunctionCall(_T("skinOps.setOuterRadius"), 4, 0, mr_reftarg, mod, mr_int, tempID, mr_int,mod->ModeBoneEnvelopeIndex+1, mr_float,r);
								macroRecorder->Disable();
								}
							}

						}

					break;
				}

			break;


		case CC_SPINNER_BUTTONUP:
			switch( LOWORD(wParam) ) {
				case IDC_SQUASHSPIN:
					{

					float q = ((ISpinnerControl *)lParam)->GetFVal();

					theHold.Suspend();
					mod->pblock_param->SetValue(skin_local_squash,mod->ip->GetTime(),q,mod->ModeBoneIndex);
					theHold.Resume();
					firstMouse = TRUE;

					break;
					}

				case IDC_EFFECTSPIN:
					mod->AcceptWeights(HIWORD(wParam));
					break;
				case IDC_ERADIUSSPIN:
					macroRecorder->Enable();
					firstMouse = FALSE;
					if (HIWORD(wParam))
						{
						if (Animating())
							{
							Control *c = mod->pblock_param->GetController(skin_cross_radius);
							float r;
							mod->pblock_param->GetValue(skin_cross_radius,mod->ip->GetTime(),r,FOREVER);
							if (c)
								{
								IKeyControl *ikc = GetKeyControlInterface(c);
								if (ikc) ikc->SetNumKeys(0);
								}
							

							macroRecorder->Disable();
							mod->pblock_param->SetValue(skin_cross_radius,0,r);
							macroRecorder->Enable();

							mod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
							mod->ip->RedrawViews(mod->ip->GetTime());
							}
						else
							{
							mod->updateP = TRUE;
							mod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
							mod->ip->RedrawViews(mod->ip->GetTime());
							}

						}
					break;



				}
			break;

		case WM_COMMAND:

			switch (LOWORD(wParam)) 
				{
/*
				case IDC_ALWAYSDEFORM_CHECK:
//				block_param->GetValue(PB_ALWAYS_DEFORM,t,AlwaysDeform,valid);
				mod->forceRecomuteBaseNode = TRUE;
				mod->UpdateEndPointDelta();
				break;
*/
				case IDC_PAINT:
					mod->StartPaintMode();
// PW: macro-recorder
					macroRecorder->FunctionCall(_T("skinOps.paintWeightsButton"), 1, 0, mr_reftarg, mod);

					break;
				case IDC_PAINTOPTIONS:
// PW: macro-recorder
					macroRecorder->FunctionCall(_T("skinOps.paintOptionsButton"), 1, 0, mr_reftarg, mod);

					mod->PaintOptions();
					break;
				case IDC_EDITENVLOPES:
					if (mod->ip)
						{
						int level = mod->ip->GetSubObjectLevel();
						if (level == 0)
							level = 1;
						else level = 0;
						mod->ip->SetSubObjectLevel(level);

						}

					break;

	
				case IDC_CREATE_CROSS_SECTION:
					{
					mod->StartCrossSectionMode(0);
					break;
					}

				case IDC_CREATE_REMOVE_SECTION:
					{
					theHold.Begin();
					theHold.Put(new PasteRestore(mod));
					

					mod->RemoveCrossSection();
					theHold.Accept(GetString(IDS_PW_ADDCROSSSECTION));
					macroRecorder->FunctionCall(_T("skinOps.removeCrossSection"), 1,0, mr_reftarg, mod
															 );

					break;
					}
/*				case IDC_UNLOCK_VERTS:
					{
					mod->unlockVerts = TRUE;
					mod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
					macroRecorder->FunctionCall(_T("skinOps.resetSelectedVerts"), 1,0, mr_reftarg, mod );
					
					break;


					}
				case IDC_RESET_ALL:
					{
					mod->unlockBone = TRUE;
					mod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
					macroRecorder->FunctionCall(_T("skinOps.resetSelectedBone"), 1,0, mr_reftarg, mod );

					break;
					}
*/
				case IDC_ADD:
					{
					theHold.Begin();

					theHold.Put(new AddBoneRestore(mod));

					BMDModEnumProc lmdproc(mod);
					mod->EnumModContexts(&lmdproc);

					for ( int i = 0; i < lmdproc.bmdList.Count(); i++ ) 
						{
						BoneModData *bmd = (BoneModData*)lmdproc.bmdList[i];
						theHold.Put(new WeightRestore(mod,bmd));
						}



					int preCt = 0;
					for (i =0; i < mod->BoneData.Count(); i++)
						{
						if (mod->BoneData[i].Node) preCt++;
						}

					
					mod->ip->DoHitByNameDialog(new DumpHitDialog(mod));

					theHold.Accept(GetString(IDS_PW_ADDBONE));

					int ct = 0;
					for (i =0; i < mod->BoneData.Count(); i++)
						{
						if (mod->BoneData[i].Node) ct++;
						}
					if (ct ==0)
						EnableWindow(GetDlgItem(hWnd,IDC_REMOVE),FALSE);
					else EnableWindow(GetDlgItem(hWnd,IDC_REMOVE),TRUE);
					if (ct!=0) 
						{
						mod->ResetSelection();
						if (mod->ModeBoneIndex < 0) mod->ModeBoneIndex = 0;
						}
					
					mod->UpdatePropInterface();

					mod->EnableRadius(FALSE);
					if (mod->ip)
						{
						ModContextList mcList;		
						INodeTab nodes;

						mod->ip->GetModContexts(mcList,nodes);
						int objects = mcList.Count();

						for ( int i = 0; i < objects; i++ ) 
							{
							BoneModData *bmd = (BoneModData*)mcList[i]->localData;

							mod->UpdateEffectSpinner(bmd);
							}
						}
					break;
					}
				case IDC_EXCLUDE:
					{
					ModContextList mcList;		
					INodeTab nodes;

					theHold.Begin();
					mod->ip->GetModContexts(mcList,nodes);
					int objects = mcList.Count();
					for ( int i = 0; i < objects; i++ ) 
						{
						BoneModData *bmd = (BoneModData*)mcList[i]->localData;

						theHold.Put(new ExclusionListRestore(mod,bmd,mod->ModeBoneIndex));  

						Tab<int> tempTable;
						tempTable.SetCount(bmd->selected.NumberSet());
						int k = 0;
						for (int j =0; j < bmd->selected.GetSize(); j++)
							{
							if (bmd->selected[j])
								tempTable[k++] = j;
							}
						bmd->ExcludeVerts(mod->ModeBoneIndex,tempTable);
						}
					theHold.Accept(GetString(IDS_PW_EXCLUSION));

					macroRecorder->FunctionCall(_T("skinOps.ButtonExclude"), 1,0, mr_reftarg, mod );

					mod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
					mod->ip->RedrawViews(mod->ip->GetTime());

					break;
					}
				case IDC_INCLUDE:
					{
					ModContextList mcList;		
					INodeTab nodes;

					mod->ip->GetModContexts(mcList,nodes);
					int objects = mcList.Count();
					theHold.Begin();
					for ( int i = 0; i < objects; i++ ) 
						{
						BoneModData *bmd = (BoneModData*)mcList[i]->localData;
						theHold.Put(new ExclusionListRestore(mod,bmd,mod->ModeBoneIndex));  

						Tab<int> tempTable;
						tempTable.SetCount(bmd->selected.NumberSet());
						int k = 0;
						for (int j =0; j < bmd->selected.GetSize(); j++)
							{
							if (bmd->selected[j])
								tempTable[k++] = j;
							}
						bmd->IncludeVerts(mod->ModeBoneIndex,tempTable);
						}
					theHold.Accept(GetString(IDS_PW_EXCLUSION));
//watje 9-7-99  198721 
					mod->Reevaluate(TRUE);
					macroRecorder->FunctionCall(_T("skinOps.ButtonInclude"), 1,0, mr_reftarg, mod );
					mod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
					mod->ip->RedrawViews(mod->ip->GetTime());

					break;
					}
				case IDC_SELECT_EXCLUDED:
					{
					ModContextList mcList;		
					INodeTab nodes;

					theHold.Begin();
					
					mod->ip->GetModContexts(mcList,nodes);
					int objects = mcList.Count();
					for ( int i = 0; i < objects; i++ ) 
						{
						BoneModData *bmd = (BoneModData*)mcList[i]->localData;
						theHold.Put(new SelectionRestore(mod,bmd));

						bmd->SelectExcludedVerts(mod->ModeBoneIndex);
						}
					theHold.Accept(GetString(IDS_PW_SELECT));
//watje 9-7-99  198721 
					mod->Reevaluate(TRUE);
					macroRecorder->FunctionCall(_T("skinOps.ButtonSelectExcluded"), 1,0, mr_reftarg, mod );
					mod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
					mod->ip->RedrawViews(mod->ip->GetTime());


					break;
					}


				case ID_FALLOFF:
					{
					if (mod->BoneData.Count() > 0)
						{

//watje 9-7-99  198721 
						mod->Reevaluate(TRUE);
						int foff = mod->iFalloff->GetCurFlyOff();

						if (foff == 0)
							{
							mod->BoneData[mod->ModeBoneIndex].FalloffType = BONE_FALLOFF_X_FLAG;
							mod->iFalloff->SetTooltip(TRUE,GetString(IDS_PW_FALLOFFLINEAR));
							}
						else if (foff == 1)
							{
							mod->BoneData[mod->ModeBoneIndex].FalloffType = BONE_FALLOFF_SINE_FLAG;
							mod->iFalloff->SetTooltip(TRUE,GetString(IDS_PW_FALLOFFSINUAL));
							}
						else if (foff == 2)
							{
							mod->BoneData[mod->ModeBoneIndex].FalloffType = BONE_FALLOFF_3X_FLAG;
							mod->iFalloff->SetTooltip(TRUE,GetString(IDS_PW_FALLOFFFAST));
							}
						else if (foff == 3)
							{
							mod->BoneData[mod->ModeBoneIndex].FalloffType = BONE_FALLOFF_X3_FLAG;
							mod->iFalloff->SetTooltip(TRUE,GetString(IDS_PW_FALLOFFSLOW));
							}

						macroRecorder->FunctionCall(_T("skinOps.setSelectedBonePropFalloff"), 2,0, mr_reftarg, mod,
													mr_int,foff+1);

						mod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);

						}

					break;
					}

				case ID_COPY:
					{
					mod->CopyBone();
					mod->iPaste->Enable(TRUE);
					macroRecorder->FunctionCall(_T("skinOps.copySelectedBone"), 1,0, mr_reftarg, mod);
					break;
					}
				case ID_PASTE:
					{
					int pasteMode = mod->iPaste->GetCurFlyOff();
					if (pasteMode == 0)
						{
						mod->iPaste->SetTooltip(TRUE,GetString(IDS_PW_PASTE));
						if (mod->BoneData.Count() > 0)
							{
							theHold.Begin();
							theHold.Put(new PasteRestore(mod));

							mod->PasteBone();
							theHold.Accept(GetString(IDS_PW_PASTE));
							macroRecorder->FunctionCall(_T("skinOps.pasteToSelectedBone"), 1,0, mr_reftarg, mod);
							}
						break;
						}
					else if (pasteMode == 1)
						{
						mod->iPaste->SetTooltip(TRUE,GetString(IDS_PW_PASTEALL));

						if (mod->BoneData.Count() > 0)
							{
							theHold.Begin();
							theHold.Put(new PasteToAllRestore(mod));

							mod->PasteToAllBones();
							theHold.Accept(GetString(IDS_PW_PASTE));
							macroRecorder->FunctionCall(_T("skinOps.pasteToAllBones"), 1,0, mr_reftarg, mod);
							}
						break;
						}
					else if (pasteMode == 2)
						{
						mod->iPaste->SetTooltip(TRUE,GetString(IDS_PW_PASTEMULT));

						int iret = DialogBoxParam(hInstance,MAKEINTRESOURCE(IDD_PASTE_DIALOG),
							hWnd,PasteDlgProc,(LPARAM)mod);	
						if (iret)
							{
							for (int i = 0; i < mod->pasteList.Count(); i++)
								{
								int id = mod->pasteList[i];
								macroRecorder->FunctionCall(_T("skinOps.pasteToBone"), 2,0, mr_reftarg, mod, mr_int, id);
								macroRecorder->EmitScript();
								}

							theHold.Begin();
							theHold.Put(new PasteToAllRestore(mod));

							mod->PasteToSomeBones();
							theHold.Accept(GetString(IDS_PW_PASTE));
							

							}
						break;
						}



					}



				case ID_ABSOLUTE:
					{
					int abso = mod->iAbsolute->IsChecked();
						//mod->iAbsolute->GetCurFlyOff();

					if (mod->BoneData.Count() > 0)
						{
						if (abso==0)
							{
							mod->BoneData[mod->ModeBoneIndex].flags |= BONE_ABSOLUTE_FLAG;
							}
							else 
							{
							mod->BoneData[mod->ModeBoneIndex].flags &= ~BONE_ABSOLUTE_FLAG;

							}
						macroRecorder->FunctionCall(_T("skinOps.setSelectedBonePropRelative"), 2,0, mr_reftarg, mod,
													mr_int,abso);

						mod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);

						}
					break;
					}
				case ID_DRAW_ENVELOPE:
//				case IDC_DISPLAY_ENVELOPE_CHECK:
					{
					if (mod->BoneData.Count() > 0)
						{

						int disp = mod->iEnvelope->IsChecked();//GetCurFlyOff();
						if (disp)
							{
							mod->BoneData[mod->ModeBoneIndex].flags |= BONE_DRAW_ENVELOPE_FLAG;
							}
							else 
							{
							mod->BoneData[mod->ModeBoneIndex].flags &= ~BONE_DRAW_ENVELOPE_FLAG;
							}
							
						macroRecorder->FunctionCall(_T("skinOps.setSelectedBonePropEnvelopeVisible"), 2,0, mr_reftarg, mod,
													mr_int,disp);

						mod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);


						}
					break;
					}
 				case IDC_REMOVE:
					{
					int tempID = mod->ModeBoneIndex;
//					tempID = mod->ConvertSelectedBoneToListID(tempID)+1;
//					if ((tempID >= mod->BoneData.Count()) || (tempID < 0))
					if ((tempID >= mod->BoneData.Count()) || (tempID < 0))
						{
						}
					else
						{
						int fsel = SendMessage(GetDlgItem(hWnd,IDC_LIST1),LB_GETCURSEL,0,0);	

						macroRecorder->FunctionCall(_T("skinOps.removeBone"), 2,0, mr_reftarg, mod,
															 mr_int,fsel+1
															 );

						theHold.Begin();
						theHold.Put(new DeleteBoneRestore(mod,mod->ModeBoneIndex));
						mod->RemoveBone();
						theHold.Accept(GetString(IDS_PW_REMOVE));
						}
//					GetSystemSetting(SYSSET_CLEAR_UNDO);
//					SetSaveRequiredFlag(TRUE);
					int ct = 0;
					for (int i =0; i < mod->BoneData.Count(); i++)
						{
						if (mod->BoneData[i].Node) ct++;
						}
					if (ct ==0)
						EnableWindow(GetDlgItem(hWnd,IDC_REMOVE),FALSE);
					else EnableWindow(GetDlgItem(hWnd,IDC_REMOVE),TRUE);

					break;
					}
				case IDC_FILTER_VERTICES_CHECK:
					{
//					mod->ClearVertexSelections();
					break;
					}
				case IDC_FILTER_BONES_CHECK:
					{
					mod->ClearBoneEndPointSelections();
					break;
					}
				case IDC_FILTER_ENVELOPES_CHECK:
					{
					mod->ClearEnvelopeSelections();
					break;
					}
				case IDC_LIST1:
					{
					if (HIWORD(wParam)==LBN_SELCHANGE) {
						int fsel;

//						if (theHold.Holding() ) 
						if (mod->ip)
							{
							ModContextList mcList;		
							INodeTab nodes;

							mod->ip->GetModContexts(mcList,nodes);
							int objects = mcList.Count();

							for ( int i = 0; i < objects; i++ ) 
								{
								BoneModData *bmd = (BoneModData*)mcList[i]->localData;

								theHold.Begin();
								theHold.Put(new SelectionRestore(mod,bmd));
								theHold.Accept(GetString(IDS_PW_SELECT));
								if (!mod->shadeVC)
									GetCOREInterface()->NodeInvalidateRect(nodes[i]);

								}
							}

						fsel = SendMessage(
							GetDlgItem(hWnd,IDC_LIST1),
							LB_GETCURSEL,0,0);	

// PW: macro-recorder
						macroRecorder->FunctionCall(_T("skinOps.SelectBone"), 2, 0, mr_reftarg, mod, mr_int, fsel+1);

						int sel = mod->ConvertSelectedListToBoneID(fsel);

						mod->cacheValid = TRUE;
						mod->ModeBoneIndex = sel;
						if	(mod->FilterBones)
							{
							mod->BoneData[mod->ModeBoneIndex].end1Selected = FALSE;
							mod->BoneData[mod->ModeBoneIndex].end2Selected = FALSE;
							mod->ModeBoneEndPoint = -1;
							}
						else 
							{
							mod->BoneData[mod->ModeBoneIndex].end1Selected = TRUE;
							mod->ModeBoneEndPoint = 0;
							}
						mod->ModeBoneEnvelopeIndex = -1;
						mod->ModeBoneEnvelopeSubType = -1;
						if (!mod->shadeVC)
							{
							if  (mod->ip) mod->ip->RedrawViews(mod->ip->GetTime());
							}
						else mod->NotifyDependents(FOREVER,PART_DISPLAY,REFMSG_CHANGE);

						mod->LastSelected = sel;

						
						mod->UpdatePropInterface();

						mod->EnableRadius(FALSE);
						if (mod->ip)
							{
							ModContextList mcList;		
							INodeTab nodes;

							mod->ip->GetModContexts(mcList,nodes);
							int objects = mcList.Count();

							for ( int i = 0; i < objects; i++ ) 
								{
								BoneModData *bmd = (BoneModData*)mcList[i]->localData;

								mod->UpdateEffectSpinner(bmd);

								}
							}
						mod->SetMirrorBone();

//WEIGHTTABLE
						mod->PaintAttribList();

						}
					break;
					}
				case IDC_WEIGHTABLE:
					{
					mod->fnWeightTable();
// PW: macro-recorder
					macroRecorder->FunctionCall(_T("skinOps.buttonWeightTable"), 1, 0, mr_reftarg, mod);
					break;
					}
				case IDC_NORMALIZE_CHECK:
					{
					UINT norm = IsDlgButtonChecked(hWnd,IDC_NORMALIZE_CHECK);
					if (norm == BST_CHECKED) mod->NormalizeSelected(TRUE);
					else mod->NormalizeSelected(FALSE);
					
					if (mod->ip)
						{
						ModContextList mcList;		
						INodeTab nodes;

						mod->ip->GetModContexts(mcList,nodes);
						int objects = mcList.Count();

						for ( int i = 0; i < objects; i++ ) 
							{
							BoneModData *bmd = (BoneModData*)mcList[i]->localData;

							mod->UpdateEffectSpinner(bmd);
							}
						}
					mod->PaintAttribList();
					break;
					
					}
				case IDC_RIGID_CHECK:
					{
					UINT rigid = IsDlgButtonChecked(hWnd,IDC_RIGID_CHECK);
					if (rigid == BST_CHECKED) mod->RigidSelected(TRUE);
					else mod->RigidSelected(FALSE);
					
					if (mod->ip)
						{
						ModContextList mcList;		
						INodeTab nodes;

						mod->ip->GetModContexts(mcList,nodes);
						int objects = mcList.Count();

						for ( int i = 0; i < objects; i++ ) 
							{
							BoneModData *bmd = (BoneModData*)mcList[i]->localData;

							mod->UpdateEffectSpinner(bmd);
							}
						mod->ip->RedrawViews(mod->ip->GetTime());
						}

							
					mod->NotifyDependents(FOREVER,PART_DISPLAY,REFMSG_CHANGE);
					mod->PaintAttribList();
					break;
					
					}
				case IDC_RIGIDHANDLES_CHECK:
					{
					UINT rigid = IsDlgButtonChecked(hWnd,IDC_RIGIDHANDLES_CHECK);
					if (rigid == BST_CHECKED) mod->RigidHandleSelected(TRUE);
					else mod->RigidHandleSelected(FALSE);
					
					if (mod->ip)
						{
						ModContextList mcList;		
						INodeTab nodes;

						mod->ip->GetModContexts(mcList,nodes);
						int objects = mcList.Count();

						for ( int i = 0; i < objects; i++ ) 
							{
							BoneModData *bmd = (BoneModData*)mcList[i]->localData;

							mod->UpdateEffectSpinner(bmd);
							}
						mod->ip->RedrawViews(mod->ip->GetTime());
						}
							
					mod->NotifyDependents(FOREVER,PART_DISPLAY,REFMSG_CHANGE);
					mod->PaintAttribList();
					break;
					
					}
				case IDC_BAKEWEIGHTS:
					{
					mod->BakeSelectedVertices();
// PW: macro-recorder
					macroRecorder->FunctionCall(_T("skinOps.bakeSelectedVerts"), 1, 0, mr_reftarg, mod );

					if (mod->ip) mod->ip->RedrawViews(mod->ip->GetTime());
					mod->NotifyDependents(FOREVER,PART_DISPLAY,REFMSG_CHANGE);
					mod->PaintAttribList();

					}




				}
			break;


		}
	return FALSE;
	}


BOOL AdvanceMapDlgProc::DlgProc(
		TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
			
	{


	switch (msg) {
		case WM_INITDIALOG:
			{

			mod->hParamAdvance = hWnd;

//5.1.03
			if (mod->hasStretchTM)
				EnableWindow(GetDlgItem(hWnd,IDC_IGNOREBONESCALE_CHECK),TRUE);
			else EnableWindow(GetDlgItem(hWnd,IDC_IGNOREBONESCALE_CHECK),FALSE);

			break;
			}
		case WM_COMMAND:

			switch (LOWORD(wParam)) 
				{
				case IDC_ALWAYSDEFORM_CHECK:
					mod->ForceRecomuteBaseNode (TRUE);
					mod->UpdateEndPointDelta();
					break;

	
				case IDC_UNLOCK_VERTS:
					{
					mod->UnlockVerts();
					mod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
					macroRecorder->FunctionCall(_T("skinOps.resetSelectedVerts"), 1,0, mr_reftarg, mod );
					
					break;


					}
				case IDC_RESET_ALL:
					{
					mod->unlockBone = TRUE;
					mod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
					macroRecorder->FunctionCall(_T("skinOps.resetSelectedBone"), 1,0, mr_reftarg, mod );

					break;
					}
				case IDC_RESET_ALL_BONES:
					{
					mod->unlockAllBones = TRUE;
					mod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
					macroRecorder->FunctionCall(_T("skinOps.resetAllBones"), 1,0, mr_reftarg, mod );

					break;
					}


				case IDC_SAVE_ENVELOPES:
					{
					mod->SaveEnvelopeDialog();
//					macroRecorder->FunctionCall(_T("skinOps.SaveEnvelopes FIX ME"), 1,0, mr_reftarg, mod );

					break;

					}
				case IDC_LOAD_ENVELOPES:
					{
					mod->LoadEnvelopeDialog();
//					macroRecorder->FunctionCall(_T("skinOps.LoadEnvelopes FIX ME"), 1,0, mr_reftarg, mod );

					break;

					}


				}
			break;


		}
	return FALSE;
	}

static void ReFillListBox(HWND hWnd, BonesDefMod *mod)
{
int selCount;

selCount = SendMessage(GetDlgItem(hWnd,IDC_LIST2),
			LB_GETSELCOUNT,0,0);
int *selList;
selList = new int[selCount];



SendMessage(GetDlgItem(hWnd,IDC_LIST2),
			LB_GETSELITEMS  ,(WPARAM) selCount,(LPARAM) selList);


SendMessage(GetDlgItem(hWnd,IDC_LIST2),
			LB_RESETCONTENT,0,0);

SendMessage(GetDlgItem(hWnd,IDC_LIST_IVERT),
			LB_RESETCONTENT,0,0);

for (int i=0; i < mod->dataList.Count(); i++)
	{
	if (mod->dataList[i] == NULL)
		{
		TSTR blank("<blank>");
		SendMessage(GetDlgItem(hWnd,IDC_LIST2),
			LB_ADDSTRING,0,(LPARAM)(TCHAR*)blank.data());
		}
	else SendMessage(GetDlgItem(hWnd,IDC_LIST2),
			LB_ADDSTRING,0,(LPARAM)(TCHAR*)mod->dataList[i]->name);
	}

for (i=0; i < mod->vertexLoadList.Count(); i++)
	{
	if (mod->vertexLoadList[i] == NULL)
		{
		TSTR blank("<blank>");
		SendMessage(GetDlgItem(hWnd,IDC_LIST_IVERT),
			LB_ADDSTRING,0,(LPARAM)(TCHAR*)blank.data());
		}
	else SendMessage(GetDlgItem(hWnd,IDC_LIST_IVERT),
			LB_ADDSTRING,0,(LPARAM)(TCHAR*)mod->vertexLoadList[i]->name);
	}

for (i=0; i < selCount; i++)
	{
	SendMessage(GetDlgItem(hWnd,IDC_LIST2),
			LB_SETSEL,1,selList[i]);
	}

delete [] selList;


}
static INT_PTR CALLBACK LoadEnvelopeDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	BonesDefMod *mod = (BonesDefMod*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
	

	switch (msg) {
		case WM_INITDIALOG:
			{
			mod = (BonesDefMod*)lParam;
			SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);

			for (int i=0; i < mod->BoneData.Count(); i++)
				{
				TCHAR title[200];

				if (mod->BoneData[i].Node != NULL)
					{
					Class_ID bid(BONE_CLASS_ID,0);
					ObjectState os = mod->BoneData[i].Node->EvalWorldState(mod->RefFrame);

					if (( os.obj->ClassID() == bid) && (mod->BoneData[i].name.Length()) )
						{
						_tcscpy(title,mod->BoneData[i].name);
						}
					else _tcscpy(title,mod->BoneData[i].Node->GetName());

					SendMessage(GetDlgItem(hWnd,IDC_LIST1),
						LB_ADDSTRING,0,(LPARAM)(TCHAR*)title);
					}

				}
			for (i=0; i < mod->dataList.Count(); i++)
				{
				
				SendMessage(GetDlgItem(hWnd,IDC_LIST2),
						LB_ADDSTRING,0,(LPARAM)(TCHAR*)mod->dataList[i]->name);

				}


			mod->BuildBaseNodeData();

			for (i=0; i < mod->vertexLoadList.Count(); i++)
				{
				if (mod->vertexLoadList[i])
					SendMessage(GetDlgItem(hWnd,IDC_LIST_IVERT),
							LB_ADDSTRING,0,(LPARAM)(TCHAR*)mod->vertexLoadList[i]->name);				
				else
					{
					TSTR blank("<blank>");
					SendMessage(GetDlgItem(hWnd,IDC_LIST_IVERT),
						LB_ADDSTRING,0,(LPARAM)(TCHAR*)blank.data());
					}

				}

			for (i=0; i < mod->loadBaseNodeData.Count(); i++)
				{
				SendMessage(GetDlgItem(hWnd,IDC_LIST_CVERT),
							LB_ADDSTRING,0,(LPARAM)(TCHAR*)mod->loadBaseNodeData[i].node->GetName());				

				}


			CheckDlgButton(hWnd,IDC_LOAD_POINTS,TRUE);
			CheckDlgButton(hWnd,IDC_LOAD_CROSSSECTIONS,TRUE);
			CheckDlgButton(hWnd,IDC_LOAD_VERTEXDATA,mod->loadVertData);
			CheckDlgButton(hWnd,IDC_LOAD_EXCLUSIONLISTS,mod->loadExclusionData);

//5.1.02
			CheckDlgButton(hWnd,IDC_LOAD_VERTEXBYINDEX,mod->loadByIndex);

			EnableWindow(GetDlgItem(hWnd,IDC_LIST_CVERT),mod->loadVertData|mod->loadExclusionData);
			EnableWindow(GetDlgItem(hWnd,IDC_LIST_IVERT),mod->loadVertData|mod->loadExclusionData);
			EnableWindow(GetDlgItem(hWnd,IDC_MOVE_UPVERT),mod->loadVertData|mod->loadExclusionData);
			EnableWindow(GetDlgItem(hWnd,IDC_MOVE_DOWNVERT),mod->loadVertData|mod->loadExclusionData);
			EnableWindow(GetDlgItem(hWnd,IDC_CREATE_BLANKVERT),mod->loadVertData|mod->loadExclusionData);


			break;

			}

		case WM_COMMAND:
			switch (LOWORD(wParam)) 
				{
//5.1.02
				case IDC_LOAD_VERTEXBYINDEX:
					mod->loadByIndex = IsDlgButtonChecked(hWnd,IDC_LOAD_VERTEXBYINDEX);
					break;

				case IDC_LOAD_VERTEXDATA:
					{
					mod->loadVertData = IsDlgButtonChecked(hWnd,IDC_LOAD_VERTEXDATA);


					EnableWindow(GetDlgItem(hWnd,IDC_LIST_CVERT),mod->loadVertData|mod->loadExclusionData);
					EnableWindow(GetDlgItem(hWnd,IDC_LIST_IVERT),mod->loadVertData|mod->loadExclusionData);
					EnableWindow(GetDlgItem(hWnd,IDC_MOVE_UPVERT),mod->loadVertData|mod->loadExclusionData);
					EnableWindow(GetDlgItem(hWnd,IDC_MOVE_DOWNVERT),mod->loadVertData|mod->loadExclusionData);					EnableWindow(GetDlgItem(hWnd,IDC_CREATE_BLANKVERT),mod->loadVertData|mod->loadExclusionData);

					break;
					}
				case IDC_LOAD_EXCLUSIONLISTS:
					{
					mod->loadExclusionData = IsDlgButtonChecked(hWnd,IDC_LOAD_EXCLUSIONLISTS);
					EnableWindow(GetDlgItem(hWnd,IDC_LIST_CVERT),mod->loadVertData|mod->loadExclusionData);
					EnableWindow(GetDlgItem(hWnd,IDC_LIST_IVERT),mod->loadVertData|mod->loadExclusionData);
					EnableWindow(GetDlgItem(hWnd,IDC_MOVE_UPVERT),mod->loadVertData|mod->loadExclusionData);
					EnableWindow(GetDlgItem(hWnd,IDC_MOVE_DOWNVERT),mod->loadVertData|mod->loadExclusionData);					EnableWindow(GetDlgItem(hWnd,IDC_CREATE_BLANKVERT),mod->loadVertData|mod->loadExclusionData);

					break;
					}


				case IDC_MOVE_UP:
					{
					int selCount;

					selCount = SendMessage(GetDlgItem(hWnd,IDC_LIST2),
						LB_GETSELCOUNT,0,0);
					int *selList;
					selList = new int[selCount];

					SendMessage(GetDlgItem(hWnd,IDC_LIST2),
						LB_GETSELITEMS  ,(WPARAM) selCount,(LPARAM) selList);
					for (int i = 0; i <selCount;  i++)
						{

						int moveTo= selList[i];
						if (moveTo!=0)
							{
//swap						
							LoadEnvelopeClass *tempData;
							tempData = mod->dataList[moveTo-1];
							mod->dataList[moveTo-1] = mod->dataList[moveTo];
							mod->dataList[moveTo] = tempData;	
							SendMessage(GetDlgItem(hWnd,IDC_LIST2),
								LB_SETSEL,0,selList[i]);							
							}
						selList[i] -= 1;
						}
					
					ReFillListBox(hWnd, mod);
					for (i=0; i < selCount; i++)
						{
						if (selList[i] >= 0)
							SendMessage(GetDlgItem(hWnd,IDC_LIST2),
								LB_SETSEL,1,selList[i]);
						}

					delete [] selList;
					break;
					}
				case IDC_MOVE_DOWN:
					{
					int selCount;

					selCount = SendMessage(GetDlgItem(hWnd,IDC_LIST2),
						LB_GETSELCOUNT,0,0);
					int *selList;
					selList = new int[selCount];

					SendMessage(GetDlgItem(hWnd,IDC_LIST2),
						LB_GETSELITEMS  ,(WPARAM) selCount,(LPARAM) selList);
					for (int i = (selCount-1); i >=0;  i--)
						{

						int moveTo= selList[i];
						if (moveTo < (mod->dataList.Count()-1))
							{
//swap						
							LoadEnvelopeClass *tempData;
							tempData = mod->dataList[moveTo+1];
							mod->dataList[moveTo+1] = mod->dataList[moveTo];
							mod->dataList[moveTo] = tempData;				
							SendMessage(GetDlgItem(hWnd,IDC_LIST2),
								LB_SETSEL,0,selList[i]);							
							
							}
						selList[i] += 1;

						}

					ReFillListBox(hWnd, mod);
					for (i=0; i < selCount; i++)
						{
						if (selList[i] < mod->dataList.Count())
							SendMessage(GetDlgItem(hWnd,IDC_LIST2),
								LB_SETSEL,1,selList[i]);
						}
					delete [] selList;

					break;
					}
				case IDC_CREATE_BLANK:
					{
					int selCount;

					selCount = SendMessage(GetDlgItem(hWnd,IDC_LIST2),
						LB_GETSELCOUNT,0,0);
					int *selList;
					selList = new int[selCount];

					SendMessage(GetDlgItem(hWnd,IDC_LIST2),
						LB_GETSELITEMS  ,(WPARAM) selCount,(LPARAM) selList);
					int insertAt;
					int c = 0;
					for (int i = 0; i <selCount;  i++)
						{

						LoadEnvelopeClass *tempData = NULL;
						if (i==0)
							{
							insertAt= selList[i];
							c = insertAt;
							}
						else
							{
							insertAt= selList[i]-insertAt;
							c += insertAt;
							}
						mod->dataList.Insert(insertAt,1,&tempData);
						}

					delete [] selList;
					ReFillListBox(hWnd, mod);
					break;
					}
				case IDC_MOVE_UPVERT:
					{
					int selected = 0;
					selected = SendMessage(GetDlgItem(hWnd,IDC_LIST_IVERT),
						LB_GETCURSEL   ,(WPARAM) 0,(LPARAM) 0);
					if ((selected != 0) && (selected!=-1) )
						{
						LoadVertexDataClass *temp = mod->vertexLoadList[selected-1];
						mod->vertexLoadList[selected-1] = mod->vertexLoadList[selected];
						mod->vertexLoadList[selected] = temp;

						}
					ReFillListBox(hWnd, mod);

					if ((selected != 0) && (selected!=-1) )
						{
						selected = SendMessage(GetDlgItem(hWnd,IDC_LIST_IVERT),
							LB_SETCURSEL   ,(WPARAM) selected-1,(LPARAM) 0);
						}
					break;
					}
				case IDC_MOVE_DOWNVERT:
					{
					int selected = 0;
					selected = SendMessage(GetDlgItem(hWnd,IDC_LIST_IVERT),
						LB_GETCURSEL   ,(WPARAM) 0,(LPARAM) 0);
					int count = 0;
					count = SendMessage(GetDlgItem(hWnd,IDC_LIST_IVERT),
						LB_GETCOUNT   ,(WPARAM) 0,(LPARAM) 0);
					if ((selected < (count-1)) && (selected!=-1) )
						{
						LoadVertexDataClass *temp = mod->vertexLoadList[selected+1];
						mod->vertexLoadList[selected+1] = mod->vertexLoadList[selected];
						mod->vertexLoadList[selected] = temp;

						}
					ReFillListBox(hWnd, mod);

					if ((selected < (count-1)) && (selected!=-1) )
						{
						selected = SendMessage(GetDlgItem(hWnd,IDC_LIST_IVERT),
							LB_SETCURSEL   ,(WPARAM) selected+1,(LPARAM) 0);
						}
					break;
					}
				case IDC_CREATE_BLANKVERT:
					{
					int insertAt = 0;
					insertAt = SendMessage(GetDlgItem(hWnd,IDC_LIST_IVERT),
						LB_GETCURSEL   ,(WPARAM) 0,(LPARAM) 0);
					LoadVertexDataClass *temp=NULL;
					if (insertAt < 0)
						insertAt = SendMessage(GetDlgItem(hWnd,IDC_LIST_IVERT), LB_GETCOUNT   ,(WPARAM) 0,(LPARAM) 0)-1;
					if (insertAt >= 0)
						{
						mod->vertexLoadList.Insert(insertAt,1,&temp);
						ReFillListBox(hWnd, mod);
						}
					break;
					}
				case IDC_MATCH:
					{
// get match type
//loop through current
					BOOL removeIncomingPrefix = IsDlgButtonChecked(hWnd,IDC_INCOMING_CHECK);
					BOOL removeCurrentPrefix = IsDlgButtonChecked(hWnd,IDC_CURRENT_CHECK);
					int incomingPrefix =0;
					int currentPrefix =0;


					int currentCount= SendMessage(GetDlgItem(hWnd,IDC_LIST1),
						LB_GETCOUNT,0,0);
					if (currentCount==0) break;
					if (removeCurrentPrefix)
						{
						TCHAR firstString[255];
						SendMessage(GetDlgItem(hWnd,IDC_LIST1),
							LB_GETTEXT,0,(LPARAM)firstString);
						if (firstString)
							{
							int firstLength = _tcslen(firstString);
							for (int i =0; i < firstLength; i++)
								{
								BOOL dif = FALSE;
								char firstLetter = firstString[i];
								for (int j = 1; j < currentCount; j++)
									{
									TCHAR currentString[255];
									SendMessage(GetDlgItem(hWnd,IDC_LIST1),
										LB_GETTEXT,j,(LPARAM)currentString);

									char currentLetter = currentString[i];
									if (currentLetter != firstLetter)
										{
										dif = TRUE;
										j = currentCount;
										}
									}
								if (dif)
									{
									i = firstLength;
									}	
								else currentPrefix++;
								}
							}
						}

					if (removeIncomingPrefix)
						{
						TCHAR *firstString=NULL;
						int start =0;
						for (int i = 0; i < mod->dataList.Count(); i++)
							{
							if (mod->dataList[i])
								{
								firstString = mod->dataList[i]->name;
								start = i;
								i = mod->dataList.Count();
								}
							}
						if (firstString)
							{
							int firstLength = _tcslen(firstString);

							for (i =0; i < firstLength; i++)
								{
								BOOL dif = FALSE;

								char firstLetter = firstString[i];
								for (int j = start; j < mod->dataList.Count(); j++)
									{
									if (mod->dataList[j])
										{
										TCHAR *currentString;
										currentString= mod->dataList[j]->name;
										char currentLetter = currentString[i];
										if (currentLetter != firstLetter)
											{
											dif = TRUE;
											j = mod->dataList.Count();
											}
										}
									}
								if (dif)
									{
									i = firstLength;
									}	
								else incomingPrefix++;
								}
							}
						}


					for (int i = 0; i < currentCount; i++)
						{
						TCHAR currentName[255];
						SendMessage(GetDlgItem(hWnd,IDC_LIST1),
							LB_GETTEXT,i,(LPARAM)currentName);
						BOOL hit = FALSE;
						for (int j = 0; j < mod->dataList.Count(); j++)
							{
	//look for match in incoming if so swap
							if (mod->dataList[j])
								{
								TCHAR *incomingStr, *currentStr;
								currentStr = &currentName[currentPrefix];
								incomingStr = &mod->dataList[j]->name[incomingPrefix];

								if (_tcscmp(incomingStr,currentStr) == 0)
									{
//equal	
									if (i >= mod->dataList.Count())
										{
										int dCount = mod->dataList.Count();
										for (int k = dCount; k <= i; k++)
											{
											LoadEnvelopeClass *tempData = NULL;
											mod->dataList.Append(1,&tempData);
											}
										}
//swap the 2
									LoadEnvelopeClass *tempData;
									tempData = mod->dataList[j];
									mod->dataList[j] = mod->dataList[i];
									mod->dataList[i] = tempData;				

									j = mod->dataList.Count();
									hit = TRUE;
									}
								}
							}
						if (!hit)
							{
//insert a blank here		
							if (i >= mod->dataList.Count())
								{
								int dCount = mod->dataList.Count();
								for (int k = dCount; k <= i; k++)
									{
									LoadEnvelopeClass *tempData = NULL;
									mod->dataList.Append(1,&tempData);
									}

								}
							else if (mod->dataList[i])
								{
								LoadEnvelopeClass *tempData = NULL;
								mod->dataList.Insert(i,1,&tempData);
								}

							}
						}
					ReFillListBox(hWnd, mod);
					break;
					}
				case IDC_DELETE:
					{
					int selCount;

					selCount = SendMessage(GetDlgItem(hWnd,IDC_LIST2),
						LB_GETSELCOUNT,0,0);
					int *selList;
					selList = new int[selCount];

					SendMessage(GetDlgItem(hWnd,IDC_LIST2),
						LB_GETSELITEMS  ,(WPARAM) selCount,(LPARAM) selList);
					for (int i = selCount-1; i >= 0; i--)
						{
						LoadEnvelopeClass *tempData = NULL;
						int deleteAt = selList[i];
						if (mod->dataList[deleteAt])
							{
							delete mod->dataList[deleteAt]->name;
							delete mod->dataList[deleteAt];
							mod->dataList[deleteAt] = NULL;
							}
						mod->dataList.Delete(deleteAt,1);
						}
\
					delete [] selList;
					ReFillListBox(hWnd, mod);

					break;
					}
				case IDOK:
					{
//fillout paste cross and point
					mod->pasteEnds = IsDlgButtonChecked(hWnd,IDC_LOAD_POINTS);
					mod->pasteCross = IsDlgButtonChecked(hWnd,IDC_LOAD_CROSSSECTIONS);
					EndDialog(hWnd,1);
					break;
					}
				case IDCANCEL:
					mod->pasteList.ZeroCount();
					EndDialog(hWnd,0);
					break;
				}
			break;

		
		case WM_CLOSE:
			mod->pasteList.ZeroCount();
			EndDialog(hWnd, 0);
//			DestroyWindow(hWnd);			
			break;

	
		default:
			return FALSE;
		}
	return TRUE;
	}


BOOL BonesDefMod::LoadEnvelope(TCHAR *fname, BOOL asText)
{
	FILE *file = NULL;
	if (asText)
		file = fopen(fname,_T("rt"));
	else file = fopen(fname,_T("rb"));

	if (file == NULL) 
		{
		MessageBox(GetCOREInterface()->GetMAXHWnd(),"Error loading file, file not found","Load Error",MB_OK);
		return FALSE;
		}
//ver
	int ver;
	TCHAR tempStr[600];
	if (asText)
		fscanf(file,"%s %d",tempStr,&ver);
	else fread(&ver, sizeof(ver), 1,file);

	if (feof(file))
		ver = -1;
	if ( (ver < 0) || (ver > 3) ) 
		{
		MessageBox(GetCOREInterface()->GetMAXHWnd(),"Error loading file, corrupt file","Load Error",MB_OK);
		return FALSE;
		}

//number bones
	int ct =0;
	if (asText)
		fscanf(file,"%s %d",tempStr,&ct);
	else fread(&ct, sizeof(ct), 1,file);


	dataList.SetCount(ct);

	TCHAR tempName[455];
	if (asText)
		fgets(tempName,455,file);

	for (int i =0; i < ct; i++)
		{
		dataList[i] = new LoadEnvelopeClass;
		int fnameLength;
//bone name length size

		if (asText)
			{
			TSTR tempBoneName;
			fgets(tempName,455,file);
			tempBoneName = tempName;
			int first = tempBoneName.first(']')+1;
			tempBoneName = tempBoneName.remove(0,first+1);
			int l = tempBoneName.Length();
			dataList[i]->name = new TCHAR[tempBoneName.Length()+1];
			_tcscpy(dataList[i]->name,tempBoneName);
			dataList[i]->name[tempBoneName.Length()-1] = 0;

			}
		else
			{
			fread(&fnameLength, sizeof(fnameLength), 1,file);
			dataList[i]->name = new TCHAR[fnameLength+1];

//bone name
			fread(dataList[i]->name, sizeof(TCHAR)*fnameLength, 1,file);
			dataList[i]->name[fnameLength] = 0;
			}
		if (ver > 1)
			{
			int boneID;
			if (asText)
				fscanf(file,"%s %d",tempStr,&boneID);
			else fread(&boneID, sizeof(boneID), 1,file);
			dataList[i]->id = boneID;
			}

		
//flags
		if (asText)
			{
			dataList[i]->flags = 0;
			BOOL flag;
			fscanf(file,"%s %d\n",tempStr,&flag);
			if (flag) dataList[i]->flags |= BONE_LOCK_FLAG;
			fscanf(file,"%s %d\n",tempStr,&flag);
			if (flag) dataList[i]->flags |= BONE_ABSOLUTE_FLAG;
			fscanf(file,"%s %d\n",tempStr,&flag);
			if (flag) dataList[i]->flags |= BONE_SPLINE_FLAG;
			fscanf(file,"%s %d\n",tempStr,&flag);
			if (flag) dataList[i]->flags |= BONE_SPLINECLOSED_FLAG;
			fscanf(file,"%s %d\n",tempStr,&flag);
			if (flag) dataList[i]->flags |= BONE_DRAW_ENVELOPE_FLAG;
			fscanf(file,"%s %d\n",tempStr,&flag);
			if (flag) dataList[i]->flags |= BONE_BONE_FLAG;
			fscanf(file,"%s %d\n",tempStr,&flag);
			if (flag) dataList[i]->flags |= BONE_DEAD_FLAG;
			}
		else fread(&dataList[i]->flags, sizeof(dataList[i]->flags), 1,file);
//FalloffType;
		if (asText)
			fscanf(file,"%s %d\n",tempStr,&dataList[i]->falloffType);
		else fread(&dataList[i]->falloffType, sizeof(dataList[i]->falloffType), 1,file);
//start point
		if (asText)
			fscanf(file,"%s %f %f %f\n",tempStr,&dataList[i]->E1.x,&dataList[i]->E1.y,&dataList[i]->E1.z);
		else fread(&dataList[i]->E1, sizeof(dataList[i]->E1), 1,file);
//end point
		if (asText)
			fscanf(file,"%s %f %f %f\n",tempStr,&dataList[i]->E2.x,&dataList[i]->E2.y,&dataList[i]->E2.z);
		else fread(&dataList[i]->E2, sizeof(dataList[i]->E2), 1,file);
//number cross sections
		int crossCount;
		if (asText)
			fscanf(file,"%s %d\n",tempStr,&crossCount);
		else fread(&crossCount, sizeof(crossCount), 1,file);
		dataList[i]->inner.SetCount(crossCount);
		dataList[i]->outer.SetCount(crossCount);
		dataList[i]->u.SetCount(crossCount);
		for (int j=0;j<crossCount; j++)
			{
	//inner
	//outer
			float inner, outer,u;
			if (asText)
				{
				fscanf(file,"%s %f\n",tempStr,&inner);
				fscanf(file,"%s %f\n",tempStr,&outer);
				fscanf(file,"%s %f\n",tempStr,&u);
				}
			else
				{
				fread(&inner, sizeof(inner), 1,file);
				fread(&outer, sizeof(outer), 1,file);
				fread(&u, sizeof(u), 1,file);
				}
			dataList[i]->inner[j] = inner;
			dataList[i]->outer[j] = outer;
			dataList[i]->u[j] = u;
			}

		}

//read in vertex data
    BOOL loadVertexData = TRUE;
	if ((loadVertexData)  && (ver > 1))  //version one did not support vertex data
		{
		if (asText)  //reads off the [Vertex Data]
			fgets(tempName,455,file);

	
		int nodeCount = 0;
		if (asText)  //read the node count
			fscanf(file,"%s %d\n",tempStr,&nodeCount);
		else fread(&nodeCount, sizeof(nodeCount), 1,file);

		if (nodeCount > 0)
			{
			vertexLoadList.SetCount(nodeCount);
			int fnameLength;
	//read the node name
			for (int i =0; i < nodeCount; i++)
				{
				vertexLoadList[i] = new LoadVertexDataClass();
				if (asText)
					{
					TSTR tempBoneName;
					fgets(tempName,455,file);
					tempBoneName = tempName;
					int first = tempBoneName.first(']')+1;
					tempBoneName = tempBoneName.remove(0,first+1);
					int l = tempBoneName.Length();
					vertexLoadList[i]->name = new TCHAR[tempBoneName.Length()+1];
					_tcscpy(vertexLoadList[i]->name,tempBoneName);
					vertexLoadList[i]->name[tempBoneName.Length()-1] = 0;

					}
				else
					{
					fread(&fnameLength, sizeof(fnameLength), 1,file);
					vertexLoadList[i]->name = new TCHAR[fnameLength+1];
					fread(vertexLoadList[i]->name, sizeof(TCHAR)*fnameLength, 1,file);
					vertexLoadList[i]->name[fnameLength] = 0;
					}

		
				int vertexCount = 0;
				if (asText)  //read the vertex count
					fscanf(file,"%s %d\n",tempStr,&vertexCount);
				else fread(&vertexCount, sizeof(vertexCount), 1,file);

				vertexLoadList[i]->vertexData.SetCount(vertexCount);

		//loop through verts
				for (int j =0; j < vertexCount; j++)
					{
					vertexLoadList[i]->vertexData[j] = new VertexListClass();
					if (asText)  
						{
						fscanf(file,"%s\n",tempStr);
						vertexLoadList[i]->vertexData[j]->flags = 0;
						BOOL flag;
						fscanf(file,"%s %d\n",tempStr,&flag); //read modified flag
						vertexLoadList[i]->vertexData[j]->Modified(flag);
						BOOL mod = vertexLoadList[i]->vertexData[j]->IsModified();
						
						fscanf(file,"%s %d\n",tempStr,&flag); //read rigid flag
						vertexLoadList[i]->vertexData[j]->Rigid(flag);
						mod = vertexLoadList[i]->vertexData[j]->IsModified();

						fscanf(file,"%s %d\n",tempStr,&flag); //read rigid handle flag
						vertexLoadList[i]->vertexData[j]->RigidHandle(flag);
						mod = vertexLoadList[i]->vertexData[j]->IsModified();

						fscanf(file,"%s %d\n",tempStr,&flag); //read unnormalized flag
						vertexLoadList[i]->vertexData[j]->UnNormalized(flag);
						mod = vertexLoadList[i]->vertexData[j]->IsModified();

						
						}
					else
						{
						DWORD	flags; 
						fread(&flags, sizeof(flags), 1,file);
						vertexLoadList[i]->vertexData[j]->flags = flags;
						}

	    //read local pos
					Point3 localPos(0.0f,0.0f,0.0f);
					if (asText)  
						fscanf(file,"%s %f %f %f\n",tempStr,&localPos.x,&localPos.y,&localPos.z);
					else fread(&localPos, sizeof(localPos), 1,file);
					vertexLoadList[i]->vertexData[j]->LocalPos = localPos;

		//read weight count
					int weightCount = 0;
					if (asText)  //read the vertex count
						fscanf(file,"%s %d\n",tempStr,&weightCount);
					else fread(&weightCount, sizeof(weightCount), 1,file);

	        //read weights
		//loop through verts
					if (asText)  
						fscanf(file,"%s\n",tempStr);

					vertexLoadList[i]->vertexData[j]->d.SetCount(weightCount);

					for (int k =0; k < weightCount; k++)
						{
						int id;
						float weight;
						if (asText)  
							{
							fscanf(file," %d,%f\n",&id,&weight);
							}
						else
							{
							fread(&id, sizeof(id), 1,file);
							fread(&weight, sizeof(weight), 1,file);
							}
						vertexLoadList[i]->vertexData[j]->d[k].Bones = id;
						vertexLoadList[i]->vertexData[j]->d[k].normalizedInfluences = weight;
						vertexLoadList[i]->vertexData[j]->d[k].Influences = weight;
						}

					if (ver >= 3)
						{
						if (asText)  
							fscanf(file,"%s\n",tempStr);


						for (int k =0; k < weightCount; k++)
							{
							float u = 0.0f;
							int curve = 0;
							int seg = 0;
							Point3 p,t;
							if (asText)  
								{
								fscanf(file," %f %d %d %f %f %f %f %f %f\n",&u,&curve,&seg,&p.x,&p.y,&p.z,&t.x,&t.y,&t.z);
								}
							else
								{
								fread(&u, sizeof(u), 1,file);
								fread(&curve, sizeof(curve), 1,file);
								fread(&seg, sizeof(seg), 1,file);
								fread(&p, sizeof(p), 1,file);
								fread(&t, sizeof(t), 1,file);
								}
							vertexLoadList[i]->vertexData[j]->d[k].u = u;
							vertexLoadList[i]->vertexData[j]->d[k].SubCurveIds = curve;
							vertexLoadList[i]->vertexData[j]->d[k].SubSegIds = seg;
							vertexLoadList[i]->vertexData[j]->d[k].OPoints = p;
							vertexLoadList[i]->vertexData[j]->d[k].Tangents = t;
							}

						}
					

					}
//read in exclusion list count
				int exCount;
				if (asText)  
					fscanf(file,"%s %d\n",tempStr,&exCount);
				else fread(&exCount, sizeof(exCount), 1,file);
				if (exCount >0)
					{

					vertexLoadList[i]->exclusionList.SetCount(exCount);
					for (int k =0; k < exCount; k++)
						{
						vertexLoadList[i]->exclusionList[k] = NULL;
					//read dummy line
						if (asText)  
							{
							fscanf(file,"%s\n",tempStr);
							fscanf(file,"%s\n",tempStr);
							}

					//read count
						int vertCount = 0;
						if (asText)  
							fscanf(file,"%s %d\n",tempStr,&vertCount);
						else fread(&vertCount, sizeof(vertCount), 1,file);

						if ((asText)  && (vertCount > 0))
							fscanf(file,"%s \n",tempStr);

						if (vertCount > 0)
							vertexLoadList[i]->exclusionList[k] = new ExclusionListClass();
						Tab<int> exList;
						exList.SetCount(vertCount);
						for (int m = 0; m < vertCount; m++)
							{
							int index;
							if (asText)  
								fscanf(file," %d\n",&index);
							else fread(&index, sizeof(index), 1,file);
							exList[m] = index;							
							}
						if (vertCount > 0)
							vertexLoadList[i]->exclusionList[k]->SetExclusionVerts(exList);

						
				//put exclusion list reader here
						}
					}
				}

			}
		
		}
	int iret = DialogBoxParam(hInstance,MAKEINTRESOURCE(IDD_LOAD_ENVELOPES_DIALOG),
						GetCOREInterface()->GetMAXHWnd(),LoadEnvelopeDlgProc,(LPARAM)this);	



	if (iret)
		{
		theHold.Begin();
		theHold.Put(new PasteToAllRestore(this));
		
		SuspendAnimate();
		AnimateOff();

		for (int m =0; m < dataList.Count(); m ++)	
			{
			if (dataList[m])
				{
//get correct bone
				int boneID = -1;
				int ct = 0;
				for (int n = 0; n < BoneData.Count(); n++)
					{
					if (BoneData[n].Node)
						{
						if (ct == m)
							{
							boneID = n;
							n = BoneData.Count();
							}						
						ct++;
						}
					
					
					}
//transform end points back
				if ((boneID < BoneData.Count()) && (boneID!=-1) && (BoneData[boneID].Node))
					{
//Delete all old cross sections
					int ct = BoneData[boneID].CrossSectionList.Count();



					BoneData[boneID].FalloffType = dataList[m]->falloffType;
					BoneData[boneID].flags  = dataList[m]->flags;

					if (pasteEnds)
						{
						BoneData[boneID].EndPoint1Control->SetValue(currentTime,&dataList[m]->E1,TRUE,CTRL_ABSOLUTE);
						BoneData[boneID].EndPoint2Control->SetValue(currentTime,&dataList[m]->E2,TRUE,CTRL_ABSOLUTE);
						}
					if (pasteCross)
						{
						for (int i =(ct-1); i >= 0 ; i--)
							RemoveCrossSection(boneID, i);
						for (i =0; i < dataList[m]->u.Count() ; i++)
							{
							AddCrossSection(boneID,dataList[m]->u[i],dataList[m]->inner[i],dataList[m]->outer[i]);
							}
						}	
					}
				}
			}
		ResumeAnimate();


		theHold.Accept(GetString(IDS_PW_PASTE));


//rebuild bone connection list
		Tab<int> boneIDList;
		if (loadVertData || loadExclusionData)
			{
			int larg = 0;
			for (int j = 0; j <  dataList.Count(); j++)
			{
				if (dataList[j] && (dataList[j]->id > larg)) larg = dataList[j]->id;
			}
			boneIDList.SetCount(larg+1);

			for (j = 0; j <  boneIDList.Count(); j++)
				boneIDList[j] = -1;

			for (j = 0; j <  dataList.Count(); j++)
				{
				if (dataList[j])
					{
					int id = dataList[j]->id;
//					if ((id >=0) && (id < BoneData.Count()))
						{
						if ((j < BoneData.Count())/* && (BoneData[j].Node)*/)
							{
							int boneID = -1;
							int ct = 0;
							for (int n = 0; n < BoneData.Count(); n++)
								{
								if (BoneData[n].Node)
									{
									if (ct == j)
										{
										boneID = n;
										n = BoneData.Count();
										}						
									ct++;
									}
								
								
								}
							boneIDList[id] = boneID;
							}
						}
					}
				}
							
			for (int i = 0; i < loadBaseNodeData.Count(); i++)
				{
				loadBaseNodeData[i].matchData = NULL;
				}
			for (i = 0; i < loadBaseNodeData.Count(); i++)
				{
				if (i<vertexLoadList.Count())
					loadBaseNodeData[i].matchData = vertexLoadList[i];
				}
			}





		if (loadVertData || loadExclusionData)
			{

			for (int i = 0; i < loadBaseNodeData.Count(); i++)
				{

//now remap the weights since bones can get reordered by the user
				if (loadBaseNodeData[i].matchData)
					{
					for (int j = 0; j < loadBaseNodeData[i].matchData->vertexData.Count(); j++)
						{	
//						int weightCount;
//						weightCount = loadBaseNodeData[i].matchData->vertexData[j]->d.Count();
						for (int k = 0; k < loadBaseNodeData[i].matchData->vertexData[j]->d.Count(); k++)
							{
							int id = loadBaseNodeData[i].matchData->vertexData[j]->d[k].Bones;
							BOOL failed = TRUE;
							int newID = -1;
							if ((id >= 0) && (id < boneIDList.Count()))
								newID = boneIDList[id];
							if (newID >= 0)
								{
								loadBaseNodeData[i].matchData->vertexData[j]->d[k].Bones = newID;
								}
							else
								{
								loadBaseNodeData[i].matchData->vertexData[j]->d.Delete(k,1);
								k--;
								}

							}

						if (ver <= 2)
							{
							for (k = 0; k < loadBaseNodeData[i].matchData->vertexData[j]->d.Count(); k++)
								{
								loadBaseNodeData[i].matchData->vertexData[j]->d[k].u = 0.0f;
								loadBaseNodeData[i].matchData->vertexData[j]->d[k].SubCurveIds = 0;
								loadBaseNodeData[i].matchData->vertexData[j]->d[k].SubSegIds = 0;
							
								}
							}
		
						}

	//remap exclusion lists also
					Tab<ExclusionListClass*> tempExclusionList;
					tempExclusionList.SetCount(loadBaseNodeData[i].matchData->exclusionList.Count());
					for (j = 0; j < loadBaseNodeData[i].matchData->exclusionList.Count(); j++)
						{
						tempExclusionList[j] = loadBaseNodeData[i].matchData->exclusionList[j];
						}
					loadBaseNodeData[i].matchData->exclusionList.SetCount(BoneData.Count());
					for (j = 0; j < BoneData.Count(); j++)
						loadBaseNodeData[i].matchData->exclusionList[j] = NULL;
					for (j = 0; j < BoneData.Count(); j++)
						{
						int newID = -1;
						for (int k = 0; k < boneIDList.Count(); k++)
						{
							if (boneIDList[k] == j) newID = k;
						}
						if ((newID >= 0) && (newID<tempExclusionList.Count())) 
							loadBaseNodeData[i].matchData->exclusionList[j] = tempExclusionList[newID];
													
						}


	//build a reasonable threshold
					Box3 bbox;
					bbox.Init();
					for (j = 0; j < loadBaseNodeData[i].matchData->vertexData.Count(); j++)
						bbox+=loadBaseNodeData[i].matchData->vertexData[j]->LocalPos;
					float threshold = Length(bbox.pmin-bbox.pmax)/10.0f;

					if (loadExclusionData)
						RemapExclusionData(loadBaseNodeData[i].bmd, threshold, i,NULL);
					if (loadVertData )
						RemapVertData(loadBaseNodeData[i].bmd, threshold, i,NULL);
					if (loadExclusionData)
						loadBaseNodeData[i].bmd->CleanUpExclusionLists();
					}

				}

			}


		UpdatePropInterface();
		for (int i = 0; i < loadBaseNodeData.Count(); i++)
			UpdateEffectSpinner(loadBaseNodeData[i].bmd);
		}

		

	ct = dataList.Count();
	for (i =0; i < ct; i++)
		{
		if (dataList[i])
			{
			delete [] dataList[i]->name;
			delete dataList[i];
			}
		dataList[i] = NULL;
		}

fclose(file);
reloadSplines = TRUE;
Reevaluate(TRUE);
return TRUE;
}


void GizmoMapDlgProc::UpdateCopyPasteButtons()
{
if ((mod->pblock_gizmos->Count(skin_gizmos_list) < 0) || (mod->currentSelectedGizmo<0))
	{
	EnableWindow(GetDlgItem(mod->hParamGizmos, IDC_COPY),FALSE);
	EnableWindow(GetDlgItem(mod->hParamGizmos, IDC_PASTE),FALSE);
	}
else
	{
	EnableWindow(GetDlgItem(mod->hParamGizmos, IDC_COPY),TRUE);
	if (mod->copyGizmoBuffer == NULL)
		EnableWindow(GetDlgItem(mod->hParamGizmos, IDC_PASTE),FALSE);
	else
		{
		ReferenceTarget *ref = mod->pblock_gizmos->GetReferenceTarget(skin_gizmos_list,0,mod->currentSelectedGizmo);

		if	(!ref)
			EnableWindow(GetDlgItem(mod->hParamGizmos, IDC_PASTE),FALSE);
		else
			{
			if (ref->ClassID() == mod->copyGizmoBuffer->cid)
				EnableWindow(GetDlgItem(mod->hParamGizmos, IDC_PASTE),TRUE);
			else EnableWindow(GetDlgItem(mod->hParamGizmos, IDC_PASTE),FALSE);
			}
		}
	}
}


BOOL GizmoMapDlgProc::DlgProc(
		TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
			
	{


	switch (msg) {
		case WM_INITDIALOG:
			{

			mod->hParamGizmos = hWnd;
//fill out drop down list box with gizmo plugs
			HWND hwGizmoType = GetDlgItem(hWnd, IDC_DEFORMERS);
			SubClassList *scList = GetCOREInterface()->GetDllDir().ClassDir().GetClassList(REF_TARGET_CLASS_ID);
			int cursel = -1;
			mod->gizmoIDList.ZeroCount();
			for ( long i = 0; i < scList->Count(ACC_ALL); ++i) 
				{
				ClassDesc* pClassD = (*scList)[ i ].CD();

				const TCHAR *cat = pClassD->Category();
				TCHAR *defcat = GetString(IDS_PW_GIZMOCATEGORY);
				

				if ((cat) && (_tcscmp(cat,defcat) == 0)) 
					{
					int iret = SendMessage(hwGizmoType, CB_ADDSTRING, 0L, (LPARAM)(pClassD->ClassName()) );
					Class_ID cid = pClassD->ClassID();
					mod->gizmoIDList.Append(1,&cid);

//					SendMessage(hwGizmoType, LB_ADDSTRING, 0L, (LPARAM)(cat) );
					}

				}
			int iret = SendMessage(hwGizmoType, CB_SETCURSEL, 0L, 0 );
			mod->currentSelectedGizmoType = 0;
			mod->currentSelectedGizmo = -1;
			mod->UpdateGizmoList();
			SendMessage(GetDlgItem(hWnd, IDC_LIST1), LB_SETCURSEL, 0L, 0 );
			if (mod->pblock_gizmos->Count(skin_gizmos_list) > 0)
				{
				ReferenceTarget *ref;
				ref = mod->pblock_gizmos->GetReferenceTarget(skin_gizmos_list,0,0);
				GizmoClass *gizmo = (GizmoClass *)ref;
				gizmo->BeginEditParams(mod->ip, mod->flags,mod->prev);
				mod->currentSelectedGizmo = 0;
				}

			EnableWindow(GetDlgItem(hWnd,IDC_ADD),FALSE);
			EnableWindow(GetDlgItem(hWnd,IDC_REMOVE),FALSE);


			UpdateCopyPasteButtons();
			break;
			}
		case WM_COMMAND:

			switch (LOWORD(wParam)) 
				{
				case IDC_ADD:
					mod->AddGizmo();
					macroRecorder->FunctionCall(_T("skinOps.buttonAddGizmo"), 1,0, mr_reftarg, mod );
					UpdateCopyPasteButtons();
					break;
				case IDC_REMOVE:
					mod->RemoveGizmo();
					macroRecorder->FunctionCall(_T("skinOps.buttonRemoveGizmo"), 1,0, mr_reftarg, mod );
					UpdateCopyPasteButtons();
					break;
				case IDC_COPY:
					mod->CopyGizmoBuffer();
					macroRecorder->FunctionCall(_T("skinOps.buttonCopyGizmo"), 1,0, mr_reftarg, mod );
					UpdateCopyPasteButtons();
					break;
				case IDC_PASTE:
					mod->PasteGizmoBuffer();
					macroRecorder->FunctionCall(_T("skinOps.buttonPasteGizmo"), 1,0, mr_reftarg, mod );
					break;

				case IDC_DEFORMERS:
					{
					if (HIWORD(wParam)==CBN_SELCHANGE)
						{
						int fsel;
						fsel = SendMessage(
							GetDlgItem(hWnd,IDC_DEFORMERS),
							CB_GETCURSEL,0,0)+1;	
						macroRecorder->FunctionCall(_T("skinOps.selectGizmoType"), 2,0, mr_reftarg, mod, mr_int,fsel );

						}
					break;
					}

				case IDC_LIST1:
					{
					if (HIWORD(wParam)==LBN_SELCHANGE) {
						int fsel;
						fsel = SendMessage(
							GetDlgItem(hWnd,IDC_LIST1),
							LB_GETCURSEL,0,0);	

// PW: macro-recorder
						if (fsel != mod->currentSelectedGizmo)
							{
							theHold.Begin();
							theHold.Put(new SelectGizmoRestore(mod));

							ReferenceTarget *ref;
							if (mod->currentSelectedGizmo != -1)
								{
								ref = mod->pblock_gizmos->GetReferenceTarget(skin_gizmos_list,0,mod->currentSelectedGizmo);
								GizmoClass *gizmo = (GizmoClass *)ref;
								if (gizmo) gizmo->EndEditParams(mod->ip, END_EDIT_REMOVEUI,NULL);
								}

							mod->currentSelectedGizmo = fsel;
							ref = mod->pblock_gizmos->GetReferenceTarget(skin_gizmos_list,0,mod->currentSelectedGizmo);
							GizmoClass *gizmo = (GizmoClass *)ref;
							gizmo->BeginEditParams(mod->ip, mod->flags,NULL);
							mod->NotifyDependents(FOREVER,PART_DISPLAY,REFMSG_CHANGE);
//							mod->ip->RedrawViews(mod->ip->GetTime());
							macroRecorder->FunctionCall(_T("skinOps.selectGizmo"), 2,0, mr_reftarg, mod,mr_int, fsel+1);
							
							theHold.Accept(GetString(IDS_PW_SELECT));

							}
						UpdateCopyPasteButtons();
						}
					break;
					}


				}
			break;


		}
	return FALSE;
	}


static INT_PTR CALLBACK DeleteDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	BonesDefMod *mod = (BonesDefMod*)GetWindowLongPtr(hWnd,GWLP_USERDATA);

	switch (msg) {
		case WM_INITDIALOG:
			{
			mod = (BonesDefMod*)lParam;
			SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);

			for (int i=0; i < mod->BoneData.Count(); i++)
				{
				TCHAR *temp;
				temp = mod->GetBoneName(i);
				if (temp)
					{
					TCHAR title[200];

					_tcscpy(title,temp);

					SendMessage(GetDlgItem(hWnd,IDC_LIST1),
						LB_ADDSTRING,0,(LPARAM)(TCHAR*)title);



					}

				}

			break;

			}

		case WM_COMMAND:
			switch (LOWORD(wParam)) 
				{
				case IDOK:
					{
					int listCt = SendMessage(GetDlgItem(hWnd,IDC_LIST1),
						LB_GETCOUNT,0,0);
					int selCt =  SendMessage(GetDlgItem(hWnd,IDC_LIST1),
						LB_GETSELCOUNT ,0,0);
					int *selList;
					selList = new int[selCt];

					SendMessage(GetDlgItem(hWnd,IDC_LIST1),
						LB_GETSELITEMS  ,(WPARAM) selCt,(LPARAM) selList);
					mod->pasteList.SetCount(selCt);
					for (int i=0; i < selCt; i++)
						{
						mod->pasteList[i] = selList[i];
						}

					delete [] selList;

					EndDialog(hWnd,1);
					break;
					}
				case IDCANCEL:
					mod->pasteList.ZeroCount();
					EndDialog(hWnd,0);
					break;
				}
			break;

		
		case WM_CLOSE:
			mod->pasteList.ZeroCount();
			EndDialog(hWnd, 0);
//			DestroyWindow(hWnd);			
			break;

	
		default:
			return FALSE;
		}
	return TRUE;
	}

