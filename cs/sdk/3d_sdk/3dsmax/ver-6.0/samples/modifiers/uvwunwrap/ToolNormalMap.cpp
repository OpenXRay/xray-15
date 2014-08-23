#include "unwrap.h"

void	UnwrapMod::fnNormalMapDialog()
	{
//bring up the dialog
	DialogBoxParam(	hInstance,
							MAKEINTRESOURCE(IDD_NORMALMAPDIALOG),
							GetCOREInterface()->GetMAXHWnd(),
//							hWnd,
							UnwrapNormalFloaterDlgProc,
							(LPARAM)this );


	}

void	UnwrapMod::SetNormalDialogPos()
	{
	if (normalWindowPos.length != 0) 
		SetWindowPlacement(normalHWND,&normalWindowPos);
	}

void	UnwrapMod::SaveNormalDialogPos()
	{
	normalWindowPos.length = sizeof(WINDOWPLACEMENT); 
	GetWindowPlacement(normalHWND,&normalWindowPos);
	}

INT_PTR CALLBACK UnwrapNormalFloaterDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	UnwrapMod *mod = (UnwrapMod*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
	//POINTS p = MAKEPOINTS(lParam);	commented out by sca 10/7/98 -- causing warning since unused.
	static ISpinnerControl *iSpacing = NULL;

	switch (msg) {
		case WM_INITDIALOG:

			{
			mod = (UnwrapMod*)lParam;
			mod->normalHWND = hWnd;

			SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);

			HWND hMethod = GetDlgItem(hWnd,IDC_METHOD_COMBO);
			SendMessage(hMethod, CB_RESETCONTENT, 0, 0);

			SendMessage(hMethod, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR) GetString(IDS_BACKFRONT));
			SendMessage(hMethod, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR) GetString(IDS_LEFTRIGHT));
			SendMessage(hMethod, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR) GetString(IDS_TOPBOTTOM));
			SendMessage(hMethod, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR) GetString(IDS_BOXNOTOP));
			SendMessage(hMethod, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR) GetString(IDS_BOX));
			SendMessage(hMethod, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR) GetString(IDS_DIAMOND));

			SendMessage(hMethod, CB_SETCURSEL, mod->normalMethod, 0L);


//create spinners and set value
			iSpacing = SetupFloatSpinner(
				hWnd,IDC_UNWRAP_SPACINGSPIN,IDC_UNWRAP_SPACING,
				0.0f,1.0f,mod->normalSpacing);	

//set align cluster
			CheckDlgButton(hWnd,IDC_NORMALIZE_CHECK,mod->normalNormalize);
			CheckDlgButton(hWnd,IDC_ROTATE_CHECK,mod->normalRotate);
			CheckDlgButton(hWnd,IDC_ALIGNWIDTH_CHECK,mod->normalAlignWidth);
			mod->SetNormalDialogPos();

			break;
			}


		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_OK:
					{
					mod->SaveNormalDialogPos();


					float tempSpacing;
					BOOL tempNormalize, tempRotate, tempAlignWidth;
					int tempMethod;
					tempSpacing = mod->normalSpacing;
					tempNormalize = mod->normalNormalize;
					tempRotate = mod->normalRotate;
					tempAlignWidth = mod->normalAlignWidth;
					tempMethod = mod->normalMethod;


					mod->normalSpacing = iSpacing->GetFVal();

					mod->normalNormalize = IsDlgButtonChecked(hWnd,IDC_NORMALIZE_CHECK);
					mod->normalRotate = IsDlgButtonChecked(hWnd,IDC_ROTATE_CHECK);
					mod->normalAlignWidth = IsDlgButtonChecked(hWnd,IDC_ALIGNWIDTH_CHECK); 
		
					HWND hMethod = GetDlgItem(hWnd,IDC_METHOD_COMBO);
					mod->normalMethod = SendMessage(hMethod, CB_GETCURSEL, 0, 0L);

					mod->fnNormalMapNoParams();

					mod->normalSpacing = tempSpacing;
					mod->normalNormalize = tempNormalize;
					mod->normalRotate = tempRotate;
					mod->normalAlignWidth= tempAlignWidth;
					mod->normalMethod = tempMethod;


					ReleaseISpinner(iSpacing);
					iSpacing = NULL;
					
					

					EndDialog(hWnd,1);
					
					break;
					}
				case IDC_CANCEL:
					{
				
					mod->SaveNormalDialogPos();
					ReleaseISpinner(iSpacing);
					iSpacing = NULL;

					EndDialog(hWnd,0);

					break;
					}
				case IDC_DEFAULT:
					{
//get bias
					mod->normalSpacing = iSpacing->GetFVal();

//get align
					mod->normalNormalize = IsDlgButtonChecked(hWnd,IDC_NORMALIZE_CHECK);
					mod->normalRotate = IsDlgButtonChecked(hWnd,IDC_ROTATE_CHECK);
					mod->normalAlignWidth = IsDlgButtonChecked(hWnd,IDC_ALIGNWIDTH_CHECK); 

					HWND hMethod = GetDlgItem(hWnd,IDC_METHOD_COMBO);
					mod->normalMethod = SendMessage(hMethod, CB_GETCURSEL, 0, 0L);
					
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


void	UnwrapMod::fnNormalMapNoParams()
	{
	Tab<Point3*> normList;

	if (normalMethod==0)  //front/back
		{
		normList.SetCount(2);

		normList[0] = new Point3(0.0f,1.0f,0.0f);
		normList[1] = new Point3(0.0f,-1.0f,0.0f);

		}
	else if (normalMethod==1)  //left/right
		{
		normList.SetCount(2);

		normList[0] = new Point3(1.0f,0.0f,0.0f);
		normList[1] = new Point3(-1.0f,0.0f,0.0f);

		}
	else if (normalMethod==2)  //top/bottom
		{
		normList.SetCount(2);

		normList[0] = new Point3(0.0f,0.0f,1.0f);
		normList[1] = new Point3(0.0f,0.0f,-1.0f);

		}

	else if (normalMethod==3)//box no top
		{
		normList.SetCount(4);

		normList[0] = new Point3(1.0f,0.0f,0.0f);
		normList[1] = new Point3(-1.0f,0.0f,0.0f);
		normList[2] = new Point3(0.0f,1.0f,0.0f);
		normList[3] = new Point3(0.0f,-1.0f,0.0f);
//		normList[4] = new Point3(0.0f,0.0f,1.0f);
//		normList[5] = new Point3(0.0f,0.0f,-1.0f);
		}

	else if (normalMethod==4)//box
		{
		normList.SetCount(6);

		normList[0] = new Point3(1.0f,0.0f,0.0f);
		normList[1] = new Point3(-1.0f,0.0f,0.0f);
		normList[2] = new Point3(0.0f,1.0f,0.0f);
		normList[3] = new Point3(0.0f,-1.0f,0.0f);
		normList[4] = new Point3(0.0f,0.0f,1.0f);
		normList[5] = new Point3(0.0f,0.0f,-1.0f);
		}
	else if (normalMethod==5)//box
		{
		normList.SetCount(8);

		normList[0] = new Point3(0.57735f, 0.57735f, 0.57735f);
		normList[1] = new Point3(-0.57735f, -0.57735f, 0.57735f);
		normList[2] = new Point3(0.57735f, -0.57735f, 0.57735f);
		normList[3] = new Point3(-0.57735f, 0.57735f, 0.57735f);
		normList[4] = new Point3(-0.57735f, 0.57735f, -0.57735f);
		normList[5] = new Point3(-0.57735f, -0.57735f, -0.57735f);
		normList[6] = new Point3(0.57735f, -0.57735f, -0.57735f);
		normList[7] = new Point3(0.57735f, 0.57735f, -0.57735f);
		}
	else return;


	fnNormalMap( &normList, normalSpacing, normalNormalize, 1, normalRotate, normalAlignWidth);
	for (int i = 0; i < normList.Count(); i++)
		delete normList[i];
	}


void UnwrapMod::fnNormalMap(Tab<Point3*> *normalList, float spacing, BOOL normalize, int layoutType, BOOL rotateClusters, BOOL alignWidth)
	{

	if (TVMaps.f.Count() == 0) return;

	BailStart();

	if (!theHold.Holding())
		{
		theHold.SuperBegin();
		theHold.Begin();
		}

	BitArray *polySel = fnGetSelectedPolygons();
	BitArray holdPolySel;

	if (polySel == NULL) 
		{
		theHold.Cancel();
		theHold.SuperCancel();
		return;
		}

	holdPolySel.SetSize(polySel->GetSize());
	holdPolySel = *polySel;

	HoldPointsAndFaces();	

	Point3 normal(0.0f,0.0f,1.0f);


	FreeClusterList();

	Tab<Point3> mapNormal;
	mapNormal.SetCount(normalList->Count());
	for (int i =0; i < mapNormal.Count(); i++)
		{
//		Point3 *p = (*normalList)[i];
		mapNormal[i] = *(*normalList)[i];
		ClusterClass *cluster = new ClusterClass();
		cluster->normal = mapNormal[i];
		clusterList.Append(1,&cluster);
		}


		//check for type
	ModContextList mcList;		
	INodeTab nodes;

	if (!ip) return;
	ip->GetModContexts(mcList,nodes);

	int objects = mcList.Count();

	BOOL bContinue = TRUE;
	TSTR statusMessage;

	if (objects != 0)
		{
		MeshTopoData *md = (MeshTopoData*)mcList[0]->localData;

		if (md == NULL) 
			{
			theHold.Cancel();
			return;
			}

		Tab<Point3> objNormList;			
		BuildNormals(md,objNormList);


		if (objNormList.Count() == 0) return;

		BitArray skipFace;
		skipFace.SetSize(md->faceSel.GetSize());
		skipFace.ClearAll();
		if (md->faceSel.NumberSet() != 0)
			{
			for (int i = 0; i < md->faceSel.GetSize(); i++)
				{
				if (!md->faceSel[i])
					skipFace.Set(i);
				}
			}


		for (i =0; i < objNormList.Count(); i++)
			{
			int index = -1;
			float angle = 0.0f;
			if (skipFace[i] == FALSE)
				{
				for (int j =0; j < clusterList.Count(); j++)
					{
					Point3 debugNorm = objNormList[i];
					float dot = DotProd(debugNorm,mapNormal[j]);
					float newAngle = (acos(dot));

					if ((dot == 1.0f) || (newAngle <= angle) || (index == -1))
						{
						index = j;
						angle = newAngle;
						}
					}
				if (index != -1)
					clusterList[index]->faces.Append(1,&i);
				}
			}


		BitArray sel;
		sel.SetSize(TVMaps.f.Count());
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
		if (bContinue)
			{	
			if (layoutType == 1)
				bContinue = LayoutClusters( spacing, rotateClusters, alignWidth, FALSE);
			else bContinue = LayoutClusters2( spacing, rotateClusters, FALSE);

			if (bContinue)
				{
				BitArray processedVerts;
				processedVerts.SetSize(TVMaps.v.Count());
				processedVerts.ClearAll();
				for (i =0; i < clusterList.Count(); i++)
					{
					for (int j = 0; j < clusterList[i]->faces.Count();j++)
						{
						int faceIndex = clusterList[i]->faces[j];
						for (int k = 0; k < TVMaps.f[faceIndex]->count; k++)
							{
							int vertIndex = TVMaps.f[faceIndex]->t[k];
							processedVerts.Set(vertIndex);
							if (objType == IS_PATCH)
								{
								if ((TVMaps.f[faceIndex]->flags & FLAG_CURVEDMAPPING) && (TVMaps.f[faceIndex]->vecs))
									{
									if (TVMaps.f[faceIndex]->flags & FLAG_INTERIOR)
										{
										vertIndex = TVMaps.f[faceIndex]->vecs->interiors[k];
										if ((vertIndex >=0) && (vertIndex < processedVerts.GetSize()))
											processedVerts.Set(vertIndex);
										}
									vertIndex = TVMaps.f[faceIndex]->vecs->handles[k*2];
									if ((vertIndex >=0) && (vertIndex < processedVerts.GetSize()))
										processedVerts.Set(vertIndex);
									vertIndex = TVMaps.f[faceIndex]->vecs->handles[k*2+1];
									if ((vertIndex >=0) && (vertIndex < processedVerts.GetSize()))
										processedVerts.Set(vertIndex);
									}
	
								}
							}
						}
					}
	


				float minx = FLT_MAX,miny = FLT_MAX;
				float maxx = FLT_MIN,maxy = FLT_MIN;
				for (i = 0; i < TVMaps.v.Count(); i++)
					{
					if (processedVerts[i])
						{
						Point3 p = TVMaps.v[i].p;
						if (p.x<minx) minx = p.x;
						if (p.y<miny) miny = p.y;
						if (p.x>maxx) maxx = p.x;
						if (p.y>maxy) maxy = p.y;
						}
					}
				float w,h;
				w = maxx-minx;
				h = maxy-miny;

				gBArea = w * h;

//normalize map to 0,0 to 1,1
				if (normalize)
					{
					for (i = 0; i < TVMaps.v.Count(); i++)
						{
						if (processedVerts[i])
							{
							TVMaps.v[i].p.x -= minx;
							TVMaps.v[i].p.y -= miny;
							}
						}

					for (i = 0; i < TVMaps.v.Count(); i++)
						{
						float amount = h;
						if (w > h) 
							amount = w;
	
						if (processedVerts[i])
							{
							TVMaps.v[i].p.x /= amount;
							TVMaps.v[i].p.y /= amount;
							if (TVMaps.cont[i]) TVMaps.cont[i]->SetValue(0,&TVMaps.v[i].p);
				
							}
						}
					}
				}
			}
		}

	if (bContinue)
		{
		theHold.Accept(_T(GetString(IDS_PW_PLANARMAP)));
		theHold.SuperAccept(_T(GetString(IDS_PW_PLANARMAP)));

		fnSelectPolygonsUpdate(&holdPolySel, FALSE);
		theHold.Suspend();
		fnSyncTVSelection();
		theHold.Resume();
//		BitArray sel;
//		sel.SetSize(TVMaps.f.Count());
//		sel.ClearAll();
//		fnSelectPolygonsUpdate(&sel, FALSE);
		}
	else
		{
		theHold.Cancel();
		theHold.SuperCancel();
		}

	CleanUpDeadVertices();
	RebuildEdges();

	theHold.Suspend();
	fnSyncGeomSelection();
	theHold.Resume();


	NotifyDependents(FOREVER,PART_SELECT,REFMSG_CHANGE);
	InvalidateView();

#ifdef DEBUGMODE 
		gEdgeHeight = 0.0f;
		gEdgeWidth = 0.0f;
		for (i =0; i < clusterList.Count(); i++)
			{
			gEdgeHeight += clusterList[i]->h;
			gEdgeWidth += clusterList[i]->w;
			
			}
		ScriptPrint("Layout Type %d Rotate Cluster %d  Align Width %d\n",layoutType, rotateClusters, alignWidth); 

		ScriptPrint("Surface Area %f bounds area %f  per used %f\n",gSArea,gBArea,gSArea/gBArea); 
		ScriptPrint("Edge Height %f Edge Width %f\n",gEdgeHeight,gEdgeWidth); 
#endif

	FreeClusterList();

	}


