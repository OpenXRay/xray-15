#include "unwrap.h"

void	UnwrapMod::fnFlattenMapDialog()
	{
//bring up the dialog
		DialogBoxParam(	hInstance,
							MAKEINTRESOURCE(IDD_FLATTENDIALOG),
							GetCOREInterface()->GetMAXHWnd(),
//							hWnd,
							UnwrapFlattenFloaterDlgProc,
							(LPARAM)this );


	}

void	UnwrapMod::SetFlattenDialogPos()
	{
	if (flattenWindowPos.length != 0) 
		SetWindowPlacement(flattenHWND,&flattenWindowPos);
	}

void	UnwrapMod::SaveFlattenDialogPos()
	{
	flattenWindowPos.length = sizeof(WINDOWPLACEMENT); 
	GetWindowPlacement(flattenHWND,&flattenWindowPos);
	}

INT_PTR CALLBACK UnwrapFlattenFloaterDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	UnwrapMod *mod = (UnwrapMod*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
	//POINTS p = MAKEPOINTS(lParam);	commented out by sca 10/7/98 -- causing warning since unused.
	static ISpinnerControl *iAngle = NULL;
	static ISpinnerControl *iSpacing = NULL;

	switch (msg) {
		case WM_INITDIALOG:


			mod = (UnwrapMod*)lParam;
			mod->flattenHWND = hWnd;

			SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);

//create spinners and set value
			iAngle = SetupFloatSpinner(
				hWnd,IDC_UNWRAP_ANGLESPIN,IDC_UNWRAP_ANGLE,
				0.0f,180.0f,mod->flattenAngleThreshold);	
			iSpacing = SetupFloatSpinner(
				hWnd,IDC_UNWRAP_SPACINGSPIN,IDC_UNWRAP_SPACING,
				0.0f,1.0f,mod->flattenSpacing);	

//set align cluster
			CheckDlgButton(hWnd,IDC_NORMALIZE_CHECK,mod->flattenNormalize);
			CheckDlgButton(hWnd,IDC_ROTATE_CHECK,mod->flattenRotate);
			CheckDlgButton(hWnd,IDC_COLLAPSE_CHECK,mod->flattenCollapse);
			mod->SetFlattenDialogPos();

			break;


		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_OK:
					{
					mod->SaveFlattenDialogPos();


					float tempSpacing, tempAngle;
					BOOL tempNormalize, tempRotate, tempCollapse;
					tempSpacing = mod->flattenSpacing;
					tempAngle = mod->flattenAngleThreshold;
					tempNormalize = mod->flattenNormalize;
					tempRotate = mod->flattenRotate;
					tempCollapse = mod->flattenCollapse;


					mod->flattenSpacing = iSpacing->GetFVal()*0.5f;
					mod->flattenAngleThreshold = iAngle->GetFVal();

					mod->flattenNormalize = IsDlgButtonChecked(hWnd,IDC_NORMALIZE_CHECK);
					mod->flattenRotate = IsDlgButtonChecked(hWnd,IDC_ROTATE_CHECK);
					mod->flattenCollapse = IsDlgButtonChecked(hWnd,IDC_COLLAPSE_CHECK); 

					mod->fnFlattenMapNoParams();

					mod->flattenSpacing = tempSpacing;
					mod->flattenAngleThreshold = tempAngle;
					mod->flattenNormalize = tempNormalize;
					mod->flattenRotate = tempRotate;
					mod->flattenCollapse= tempCollapse;


					ReleaseISpinner(iAngle);
					iAngle = NULL;
					ReleaseISpinner(iSpacing);
					iSpacing = NULL;

					EndDialog(hWnd,1);
					
					break;
					}
				case IDC_CANCEL:
					{
				
					mod->SaveFlattenDialogPos();
					ReleaseISpinner(iAngle);
					iAngle = NULL;
					ReleaseISpinner(iSpacing);
					iSpacing = NULL;

					EndDialog(hWnd,0);

					break;
					}
				case IDC_DEFAULT:
					{
//get bias
					mod->flattenSpacing = iSpacing->GetFVal();
					mod->flattenAngleThreshold = iAngle->GetFVal();

//get align
					mod->flattenNormalize = IsDlgButtonChecked(hWnd,IDC_NORMALIZE_CHECK);
					mod->flattenRotate = IsDlgButtonChecked(hWnd,IDC_ROTATE_CHECK);
					mod->flattenCollapse = IsDlgButtonChecked(hWnd,IDC_COLLAPSE_CHECK); 
					
//set as defaults
					break;
					}

				}
			break;

		default:
			return FALSE;
		}
	return TRUE;
	}


void	UnwrapMod::fnFlattenMapNoParams()
	{
	Tab<Point3*> normList;
	normList.SetCount(6);

	normList[0] = new Point3(1.0f,0.0f,0.0f);
	normList[1] = new Point3(-1.0f,0.0f,0.0f);
	normList[2] = new Point3(0.0f,1.0f,0.0f);
	normList[3] = new Point3(0.0f,-1.0f,0.0f);
	normList[4] = new Point3(0.0f,0.0f,1.0f);
	normList[5] = new Point3(0.0f,0.0f,-1.0f);


	fnFlattenMap(flattenAngleThreshold, &normList, flattenSpacing, flattenNormalize, 2, flattenRotate, flattenCollapse);
	for (int i = 0; i < 6; i++)
		delete normList[i];
	}

void UnwrapMod::fnFlattenMap(float angleThreshold, Tab<Point3*> *normalList, float spacing, BOOL normalize, int layoutType, BOOL rotateClusters, BOOL fillHoles)

	{
	
	BailStart();
	if (preventFlattening) return;
	
	if (TVMaps.f.Count() == 0) return;

	if (!theHold.Holding())
		{
		theHold.SuperBegin();
		theHold.Begin();
		}

	HoldPointsAndFaces();	


	BitArray *polySel = fnGetSelectedPolygons();
	if (polySel == NULL) 
		{
		theHold.Cancel();
		theHold.SuperCancel();
		return;
		}
	BitArray holdPolySel;
	holdPolySel.SetSize(polySel->GetSize());
	holdPolySel = *polySel;




	Point3 normal(0.0f,0.0f,1.0f);

	Tab<Point3> mapNormal;
	mapNormal.SetCount(normalList->Count());
	for (int i =0; i < mapNormal.Count(); i++)
		{
		mapNormal[i] = *(*normalList)[i];
		}

	TSTR statusMessage;
	BOOL bContinue = BuildCluster( mapNormal, angleThreshold, TRUE, TRUE);
	BitArray sel;
	sel.SetSize(TVMaps.f.Count());

	gBArea = 0.0f;

	int initialCluster = clusterList.Count();

	if (bContinue)
		{
		for (i =0; i < clusterList.Count(); i++)
			{
			sel.ClearAll();
			for (int j = 0; j < clusterList[i]->faces.Count();j++)
				sel.Set(clusterList[i]->faces[j]);
			fnSelectPolygonsUpdate(&sel, FALSE);
			PlanarMapNoScale(clusterList[i]->normal);

			int per = (i * 100)/clusterList.Count();
			statusMessage.printf("%s %d%%.",GetString(IDS_PW_STATUS_MAPPING),per);
			if (Bail(ip,statusMessage))
				{
				i = clusterList.Count();
				bContinue =  FALSE;
				}

			}

//		if (0)
		if (bContinue)
			{

			if (layoutType == 1)
				bContinue = LayoutClusters( spacing, rotateClusters, TRUE, fillHoles);
			else bContinue = LayoutClusters2( spacing, rotateClusters, fillHoles);

//normalize map to 0,0 to 1,1
			if ((bContinue) && (normalize))
				{
				NormalizeCluster(spacing);
				}

			}

		}

	CleanUpDeadVertices();
	if (bContinue)
		{
		theHold.Accept(_T(GetString(IDS_PW_FLATTEN)));
		theHold.SuperAccept(_T(GetString(IDS_PW_FLATTEN)));
//		sel.ClearAll();
		fnSelectPolygonsUpdate(&holdPolySel, FALSE);
		theHold.Suspend();
		fnSyncTVSelection();
		theHold.Resume();

		}
	else
		{
		theHold.Cancel();
		theHold.SuperCancel();
		}

	RebuildEdges();

	if (bContinue)
		fnSelectPolygonsUpdate(&holdPolySel, FALSE);
	theHold.Suspend();
	fnSyncTVSelection();
	fnSyncGeomSelection();
	theHold.Resume();


	NotifyDependents(FOREVER,PART_SELECT,REFMSG_CHANGE);
	InvalidateView();


#ifdef DEBUGMODE 
	if (gDebugLevel >= 1)
		{
		int finalCluster = clusterList.Count();
		gEdgeHeight = 0.0f;
		gEdgeWidth = 0.0f;
		for (i =0; i < clusterList.Count(); i++)
			{
			gEdgeHeight += clusterList[i]->h;
			gEdgeWidth += clusterList[i]->w;
				
			}

		ScriptPrint("Surface Area %f bounds area %f  per used %f\n",gSArea,gBArea,gSArea/gBArea); 
		ScriptPrint("Edge Height %f Edge Width %f\n",gEdgeHeight,gEdgeWidth); 
		ScriptPrint("Initial Clusters %d finalClusters %d\n",initialCluster,finalCluster); 
		}

#endif

	FreeClusterList();

	statusMessage.printf("Done, area coverage %3.2f",(gSArea/gBArea)*100.f);
	Bail(ip,statusMessage,0);


	}

