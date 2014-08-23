#include "unwrap.h"

void	UnwrapMod::GetEdgeCluster(BitArray &cluster)
	{
	BitArray tempArray;
//next check to make sure we only have one cluster and if not element the smaller ones

	tempArray.SetSize(TVMaps.v.Count());
	tempArray.ClearAll();
	tempArray = vsel;


	int seedVert=-1;
	int seedSize = -1;
	BitArray oppoProcessedElement;
	oppoProcessedElement.SetSize(TVMaps.v.Count());
	oppoProcessedElement.ClearAll();


//5.1.04  remove any verts that share common geo verts
	Tab<int> geoIDs;
	geoIDs.SetCount(TVMaps.v.Count());
	for (int i = 0; i < TVMaps.f.Count(); i++)
		{
		UVW_TVFaceClass *face = TVMaps.f[i];
		for (int j = 0; j < face->count; j++)
			{
			int gid = face->v[j];
			int vid = face->t[j];
			if ((gid >= 0) && (gid < geoIDs.Count()))
				geoIDs[vid] = gid;
			}
		}
	for (i = 0; i < TVMaps.v.Count(); i++)
		{
		if (cluster[i])
			{
			//tag all shared geo verts
			int gid = geoIDs[i];
			BitArray sharedVerts;
			sharedVerts.SetSize(TVMaps.v.Count());
			sharedVerts.ClearAll();
			for (int j = 0; j < TVMaps.v.Count(); j++)
				{
				if ((geoIDs[j] == gid) && cluster[j])
					sharedVerts.Set(j);
				}
			//now look to see any of these
			if (sharedVerts.NumberSet() > 1)
				{
				int sharedEdge = -1;
				for (j = 0; j < TVMaps.v.Count(); j++)
					{
					if (sharedVerts[j])
						{
						//loop through our edges and see if this one touches any of our vertices
						for (int k = 0; k < TVMaps.ePtrList.Count(); k++)
							{
							int a = TVMaps.ePtrList[k]->a;
							int b = TVMaps.ePtrList[k]->b;
							if (a == j)
								{
								if (cluster[b]) sharedEdge = j;
								}
							else if (b == j)
								{
								if (cluster[a]) sharedEdge = j;
								}

							}
						}
					}
				if (sharedEdge == -1)
					{
					BOOL first = TRUE;
					for (j = 0; j < TVMaps.v.Count(); j++)
						{
						if (sharedVerts[j])
							{
							if (!first)
								cluster.Set(j,FALSE);
							first = FALSE;
							}
						}
					}
				else
					{
					for (j = 0; j < TVMaps.v.Count(); j++)
						{
						if (sharedVerts[j])
							{
							cluster.Set(j,FALSE);
							}
						}
					cluster.Set(sharedEdge,TRUE);

					}
				}

			}
		}


	for (i = 0; i < TVMaps.v.Count(); i++)
		{
		if ((!oppoProcessedElement[i])  && (cluster[i]))
			{
			vsel.ClearAll();
			vsel.Set(i);
			SelectElement();

			oppoProcessedElement |= vsel;

			vsel = vsel & cluster;
			int ct = vsel.NumberSet();
			if (ct > seedSize)
				{
				seedSize = ct;
				seedVert = i;
				}
		
			}
		}
	if (seedVert != -1)
		{
		vsel.ClearAll();
		vsel.Set(seedVert);
		SelectElement();
		cluster = vsel & cluster;
		}

	vsel = tempArray;

	}

void	UnwrapMod::fnStitchVerts(BOOL bAlign, float fBias)
	{
//build my connection list first

	TransferSelectionStart();
	int oldSubObjectMode = fnGetTVSubMode();
	fnSetTVSubMode(TVVERTMODE);

	BuildVertexClusterList();

//clean up selection
//remove any verts not on edge
	for (int i =0; i < TVMaps.v.Count(); i++)
		{
//only process selected vertices that have more than one connected verts
//and that have not been already processed (tprocessedVerts)
		if ((vertexClusterListCounts[i] <= 1) && (vsel[i]))
			{
			vsel.Set(i,FALSE);
			}
		}
//if multiple edges selected set it to the largest
	GetEdgeCluster(vsel);

//this builds a list of verts connected to the selection
	BitArray oppositeVerts;
	oppositeVerts.SetSize(TVMaps.v.Count());
	oppositeVerts.ClearAll();


	for (i =0; i < TVMaps.v.Count(); i++)
		{
//only process selected vertices that have more than one connected verts
//and that have not been already processed (tprocessedVerts)
		if ((vertexClusterListCounts[i] > 1) && (vsel[i]))
			{
			int clusterId = vertexClusterList[i];
			//now loop through all the
			for (int j =0; j < TVMaps.v.Count(); j++)
				{
				if ((clusterId == vertexClusterList[j]) && (j!=i))
					{
					if (!vsel[j])
						oppositeVerts.Set(j);
					}
				}

			}
		}


	GetEdgeCluster(oppositeVerts);

	BitArray elem,tempArray;
	tempArray.SetSize(TVMaps.v.Count());
	tempArray.ClearAll();

//clean up vsel and rmeove any that are not part of the opposite set
	for (i =0; i < TVMaps.v.Count(); i++)
		{
		if (oppositeVerts[i])
			{
			int clusterId = vertexClusterList[i];
			for (int j =0; j < TVMaps.v.Count(); j++)
				{
				if ((clusterId == vertexClusterList[j]) && (j!=i) && (vsel[j]))
					tempArray.Set(j);
				}

			}
		}
	vsel = vsel & tempArray;


	tempArray.SetSize(TVMaps.v.Count());
	tempArray.ClearAll();
	tempArray = vsel;

//this builds a list of the two elements clusters
//basically all the verts that are connected to the selection and the opposing verts
	int seedA=-1, seedB=-1;
	for (i =0; i < TVMaps.v.Count(); i++)
		{
		if ((vsel[i]) && (seedA == -1))
			seedA = i;
		if ((oppositeVerts[i]) && (seedB == -1))
			seedB = i;
		}
	
	if (seedA == -1) return;
	if (seedB == -1) return;


	
	elem.SetSize(TVMaps.v.Count());
	elem.SetSize(TVMaps.v.Count());

	tempArray.SetSize(TVMaps.v.Count());
	tempArray.ClearAll();
	tempArray = vsel;

	vsel.ClearAll();
	vsel.Set(seedA);
	SelectElement();
	elem = vsel;


	vsel.ClearAll();
	vsel.Set(seedB);
	SelectElement();
	elem |= vsel;

	vsel = tempArray;



//align the clusters if they are seperate 
	if (bAlign)
		{
//check to make sure there are only 2 elements
		int numberOfElements = 2;

		BitArray elem1,elem2, holdArray;

//build element 1
		elem1.SetSize(TVMaps.v.Count());
		elem2.SetSize(TVMaps.v.Count());
		elem1.ClearAll();
		elem2.ClearAll();


		holdArray.SetSize(vsel.GetSize());
		holdArray = vsel;

		SelectElement();
		elem1 = vsel;

		vsel = oppositeVerts;
		elem2 = oppositeVerts;
		SelectElement();



		for (i = 0; i < TVMaps.v.Count(); i++)
			{
			if (vsel[i] && elem1[i])
				{
				numberOfElements = 0;
				break;
				}
			}
		

	
		if (numberOfElements == 2)
			{
//build a line from selection of element 1
			elem1 = holdArray;

			sourcePoints[0] = -1;
			sourcePoints[1] = -1;
			targetPoints[0] = -1;
			targetPoints[1] = -1;

			int ct = 0;
			Tab<int> connectedCount;
			connectedCount.SetCount(TVMaps.v.Count());

			for (i = 0; i < TVMaps.v.Count(); i++)
				connectedCount[i] = 0;

//find a center point and then the farthest point from it
			Point3 centerP(0.0f,0.0f,0.0f);
			Box3 bounds;
			bounds.Init();
			for (i = 0; i < TVMaps.v.Count(); i++)
				{
				if ((holdArray[i])  && (vertexClusterListCounts[i] > 1))
					{
					bounds += TVMaps.v[i].p;
					}
				}
			centerP = bounds.Center();
			int closestIndex = -1;
			float closestDist = 0.0f;
			centerP.z = 0.0f;

			for (i = 0; i < TVMaps.v.Count(); i++)
				{
				if ((holdArray[i])  && (vertexClusterListCounts[i] ==2 ) )//watjeFIX
					{
					float dist;
					Point3 p = TVMaps.v[i].p;
					p.z = 0.0f;
					dist = LengthSquared(p-centerP);
					if ((dist > closestDist) || (closestIndex == -1))
						{
						closestDist = dist;
						closestIndex = i;
						}
					}
				}
			sourcePoints[0] = closestIndex;

			closestIndex = -1;
			closestDist = 0.0f;
			for (i = 0; i < TVMaps.v.Count(); i++)
				{
				if ((holdArray[i])  && (vertexClusterListCounts[i] == 2))//watjeFIX
					{
					float dist;
					Point3 p = TVMaps.v[i].p;
					p.z = 0.0f;
					dist = LengthSquared(p-TVMaps.v[sourcePoints[0]].p);
					if (((dist > closestDist) || (closestIndex == -1)) && (i!=sourcePoints[0])) //fix 5.1.04
						{
						closestDist = dist;
						closestIndex = i;
						}
					}
				}
			sourcePoints[1] = closestIndex;

//fix 5.1.04 if our guess fails  pick 2 at random
			if ( (sourcePoints[0] == -1) || (sourcePoints[1] == -1) )
				{
				int ct = 0;
				for (i = 0; i < TVMaps.v.Count(); i++)
					{
					if ((holdArray[i])  && (vertexClusterListCounts[i] >= 2 ) )
						{
						sourcePoints[ct] = i;
						ct++;
						if (ct == 2) break;

						}
					}

				}


// find the matching corresponding points on the opposite edge
//by matching the 
			ct = 0;
			int clusterID = vertexClusterList[sourcePoints[0]];
			for (i = 0; i < TVMaps.v.Count(); i++)
				{
				if (elem2[i])
					{
					if ( (sourcePoints[ct] != i) && (clusterID == vertexClusterList[i]))
						{
						targetPoints[ct] = i;
						ct++;
						break;
						}
					}
				}

			clusterID = vertexClusterList[sourcePoints[1]];
			for (i = 0; i < TVMaps.v.Count(); i++)
				{
				if (elem2[i])
					{
					if ( (sourcePoints[ct] != i) && (clusterID == vertexClusterList[i]) && (targetPoints[0]!=i) ) //watjeFIX
						{
						targetPoints[ct] = i;
						ct++;
						break;
						}
					}
				}

			if ( (targetPoints[0] != -1) && (targetPoints[1] != -1) &&
				 (sourcePoints[0] != -1) && (sourcePoints[1] != -1) )
				{
//build a line from selection of element 1
				Point3 centerTarget,centerSource,vecTarget,vecSource;

				vecSource = Normalize(TVMaps.v[sourcePoints[1]].p-TVMaps.v[sourcePoints[0]].p);
				vecSource.z = 0.0f;
				vecSource = Normalize(vecSource);

				//build a line from selection of element 2
				vecTarget = Normalize(TVMaps.v[targetPoints[1]].p-TVMaps.v[targetPoints[0]].p);
				vecTarget.z = 0.0f;
				vecTarget = Normalize(vecTarget);

				centerSource = (TVMaps.v[sourcePoints[0]].p + TVMaps.v[sourcePoints[1]].p) / 2.0f; 
				centerTarget = (TVMaps.v[targetPoints[0]].p + TVMaps.v[targetPoints[1]].p) / 2.0f; 

				float angle = 0.0f;
				float dist = Length(vecSource-vecTarget*-1.0f);
				if ( dist < 0.001f) angle = PI;
				else if (Length(vecSource-vecTarget) < 0.001f)  //fixes 5.1.01 fixes a stitch error
					angle = 0.0f;
				else angle = acos(DotProd( vecSource, vecTarget));

				float x,y,x1,y1;
				float tx = vecTarget.x;
				float ty = vecTarget.y;

				float sx = vecSource.x;
				float sy = vecSource.y;

				Point3 sp,spn;
				sp = vecTarget;
				spn = vecTarget;
				Matrix3 tm(1);
				tm.SetRotateZ(angle);
				sp = sp * tm;

				tm.IdentityMatrix();
				tm.SetRotateZ(-angle);
				spn = spn * tm;

				x = sp.x;
				y = sp.y;

				x1 = spn.x;
				y1 = spn.y;

				if ( (fabs(x-vecSource.x) > 0.001f) || (fabs(y-vecSource.y) > 0.001f))
					{
					angle = -angle;
					}
//now align the elem2
				Point3 delta = centerSource - centerTarget;

				for (i = 0; i < TVMaps.v.Count(); i++)
					{
					if (vsel[i])
						{
						Point3 p = TVMaps.v[i].p;
						p = p - centerTarget;
						float tx = p.x;
						float ty = p.y;
						p.x = (tx * cos(angle)) - (ty * sin(angle));
						p.y = (tx * sin(angle)) + (ty * cos(angle));
						p += centerSource;

						TVMaps.v[i].p = p;
						if (TVMaps.cont[i]) TVMaps.cont[i]->SetValue(0,&TVMaps.v[i].p);

						}
					}
//find an offset so the elements do not intersect
				}

//align element 2 to element 1

			}
		 vsel  = holdArray;

		}
//now loop through the vertex list and process only those that are selected.
	BitArray processedVerts;
	processedVerts.SetSize(TVMaps.v.Count());
	processedVerts.ClearAll();



	Tab<Point3> movePoints;
	movePoints.SetCount(TVMaps.v.Count());
	for ( i =0; i < TVMaps.v.Count(); i++)
		movePoints[i]= Point3(0.0f,0.0f,0.0f);



	
	for (i =0; i < TVMaps.v.Count(); i++)
		{
//only process selected vertices that have more than one connected verts
//and that have not been already processed (tprocessedVerts)

		if ((vertexClusterListCounts[i] > 1) && (!oppositeVerts[i]) && (vsel[i]) && (!processedVerts[i]))
			{
			int clusterId = vertexClusterList[i];
			//now loop through all the
			Tab<int> slaveVerts;
			processedVerts.Set(i);
			for (int j =0; j < TVMaps.v.Count(); j++)
				{

				if ((clusterId == vertexClusterList[j]) && (j!=i) && (oppositeVerts[j]))
					{
					slaveVerts.Append(1,&j);

//					processedVerts.Set(j);
					}
				}
			Point3 center;
			center = TVMaps.v[i].p;


			if (slaveVerts.Count()>=2) //watjeFIX
				{
				// find the selected then find the closest
				int selected = 0;
//				for (j = 0; j < slaveVerts.Count(); j++)
//					{
//					processedVerts.Set(slaveVerts[j],FALSE);
//					}
				float dist = -1.0f;
				int closest = 0;
				for (j = 0; j < slaveVerts.Count(); j++)
					{
					float ldist = Length(TVMaps.v[slaveVerts[j]].p-TVMaps.v[i].p);
					if ((dist < 0.0f) || (ldist<dist))
						{
						dist = ldist;
						closest = slaveVerts[j];
						}
					}
				
				processedVerts.Set(closest);
				slaveVerts.SetCount(1);
				slaveVerts[0] = closest;


				}
			else if (slaveVerts.Count()==1) processedVerts.Set(slaveVerts[0]);


			Tab<Point3> averages;
			averages.SetCount(slaveVerts.Count());

			for (j = 0; j < slaveVerts.Count(); j++)
				{
				int slaveIndex = slaveVerts[j];
				averages[j] = (TVMaps.v[slaveIndex].p - center) * fBias;
				}
			
			Point3 mid(0.0f,0.0f,0.0f);
			for (j = 0; j < slaveVerts.Count(); j++)
				{
				mid += averages[j];
				}
			mid = mid/(float)slaveVerts.Count();

			center += mid;

			for (j = 0; j < slaveVerts.Count(); j++)
				{
				int slaveIndex = slaveVerts[j];
				movePoints[slaveIndex]= center;
				}


			movePoints[i]= center;
			

			}
		}


	BitArray oldSel;
	oldSel.SetSize(vsel.GetSize());
	oldSel = vsel;

	vsel = processedVerts;
	RebuildDistCache();

	for (i =0; i < TVMaps.v.Count(); i++)
		{
		float influ = TVMaps.v[i].influence;
		if ((influ != 0.0f) && (!processedVerts[i]) && (!(TVMaps.v[i].flags & FLAG_DEAD)) && (elem[i]) )
			{
//find closest processed vert and use that delta
			int closestIndex = -1;
			float closestDist = 0.0f;
			for (int j =0; j < TVMaps.v.Count(); j++)
				{
				if (processedVerts[j])
					{
					int vertexCT = vertexClusterListCounts[j];
					if (( vertexCT > 1) && (!(TVMaps.v[j].flags& FLAG_DEAD)))
						{
						float tdist = LengthSquared(TVMaps.v[i].p - TVMaps.v[j].p);
						if ((closestIndex == -1) || (tdist < closestDist))
							{
							closestDist = tdist;
							closestIndex = j;
							}
						}
					}
				}
			if (closestIndex == -1)
				{
				if (gDebugLevel >= 1)
					{
					ScriptPrint("Error stitch soft sel vert %d\n",i); 
					}
				}
			else
				{		
				if (gDebugLevel >= 3)
					{
					ScriptPrint("Moving stitch soft sel vert %d closest edge %d dist %f\n",i,closestIndex, closestDist); 
					}				
				Point3 delta = (movePoints[closestIndex] - TVMaps.v[closestIndex].p) * influ;
				TVMaps.v[i].p += delta;
				if (TVMaps.cont[i]) TVMaps.cont[i]->SetValue(0,&TVMaps.v[i].p);
				}
			}

		}

	for (i =0; i < TVMaps.v.Count(); i++)
		{
		if ( processedVerts[i])
			{
			TVMaps.v[i].p = movePoints[i];
			if (TVMaps.cont[i]) TVMaps.cont[i]->SetValue(0,&TVMaps.v[i].p);
			}
		}


//now weld the verts


	float tempWeld = weldThreshold;
	weldThreshold = 0.001f;
//	theHold.Suspend();
	WeldSelected(FALSE);
//	theHold.Resume();
	weldThreshold = tempWeld;


	CleanUpDeadVertices();
	vsel = oldSel;

	fnSetTVSubMode(oldSubObjectMode);
	TransferSelectionEnd(FALSE,TRUE);

	RebuildDistCache();
	RebuildEdges();
	NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
	if (ip) ip->RedrawViews(currentTime);
	}

void	UnwrapMod::fnStitchVertsNoParams()
	{
	if (!theHold.Holding())
		theHold.Begin();
	HoldPointsAndFaces();	
	fnStitchVerts(bStitchAlign,fStitchBias);
	theHold.Accept(_T(GetString(IDS_PW_STITCH)));
	InvalidateView();

	}
void	UnwrapMod::fnStitchVertsDialog()
	{
//bring up the dialog
	DialogBoxParam(	hInstance,
							MAKEINTRESOURCE(IDD_STICTHDIALOG),
							GetCOREInterface()->GetMAXHWnd(),
//							hWnd,
							UnwrapStitchFloaterDlgProc,
							(LPARAM)this );


	}

void	UnwrapMod::SetStitchDialogPos()
	{
	if (stitchWindowPos.length != 0) 
		SetWindowPlacement(stitchHWND,&stitchWindowPos);
	}

void	UnwrapMod::SaveStitchDialogPos()
	{
	stitchWindowPos.length = sizeof(WINDOWPLACEMENT); 
	GetWindowPlacement(stitchHWND,&stitchWindowPos);
	}




INT_PTR CALLBACK UnwrapStitchFloaterDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	UnwrapMod *mod = (UnwrapMod*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
	//POINTS p = MAKEPOINTS(lParam);	commented out by sca 10/7/98 -- causing warning since unused.
	static ISpinnerControl *iBias = NULL;
	static BOOL bAlign = TRUE;
	static float fBias= 0.0f;
	static float fSoftSel = 0.0f;
	static BitArray sel;
	static syncGeom = TRUE;
	switch (msg) {
		case WM_INITDIALOG:


			mod = (UnwrapMod*)lParam;
			mod->stitchHWND = hWnd;

			SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);

			syncGeom = mod->fnGetSyncSelectionMode();
			mod->fnSetSyncSelectionMode(FALSE);

//create bias spinner and set value
			iBias = SetupFloatSpinner(
				hWnd,IDC_UNWRAP_BIASSPIN,IDC_UNWRAP_BIAS,
				0.0f,1.0f,mod->fStitchBias);	
			iBias->SetScale(0.01f);
//set align cluster
			CheckDlgButton(hWnd,IDC_ALIGN_CHECK,mod->bStitchAlign);
			bAlign = mod->bStitchAlign;
			fBias = mod->fStitchBias;
			
			sel.SetSize(mod->vsel.GetSize());
			sel = mod->vsel;
//restore window pos
			mod->SetStitchDialogPos();
//start the hold begin
			if (!theHold.Holding())
				{
				theHold.SuperBegin();
				theHold.Begin();
				}
//hold the points and faces
			mod->HoldPointsAndFaces();
//stitch initial selection
			mod->fnStitchVerts(bAlign,fBias);
			mod->InvalidateView();

			break;
		case CC_SPINNER_BUTTONDOWN:
			if (LOWORD(wParam) == IDC_UNWRAP_BIASSPIN) 
				{
				}
			break;


		case CC_SPINNER_CHANGE:
			if (LOWORD(wParam) == IDC_UNWRAP_BIASSPIN) 
				{
//get align
				fBias = iBias->GetFVal();
//revert hold
				theHold.Restore();
				mod->vsel = sel;
//call stitch again
				mod->fnStitchVerts(bAlign,fBias);
				mod->InvalidateView();
				}
			break;

		case WM_CUSTEDIT_ENTER:
		case CC_SPINNER_BUTTONUP:
			if ( (LOWORD(wParam) == IDC_UNWRAP_BIAS) || (LOWORD(wParam) == IDC_UNWRAP_BIASSPIN) )
				{
				}
			break;


		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_APPLY:
					{
					theHold.Accept(_T(GetString(IDS_PW_STITCH)));
					theHold.SuperAccept(_T(GetString(IDS_PW_STITCH)));
					mod->SaveStitchDialogPos();

					ReleaseISpinner(iBias);
					iBias = NULL;

					 
					mod->fnSetSyncSelectionMode(syncGeom);


					EndDialog(hWnd,1);
					
					break;
					}
				case IDC_REVERT:
					{
					theHold.Restore();
					theHold.Cancel();
					theHold.SuperCancel();
					mod->vsel = sel;
				
					mod->SaveStitchDialogPos();
					ReleaseISpinner(iBias);
					iBias = NULL;

					mod->fnSetSyncSelectionMode(syncGeom);

					EndDialog(hWnd,0);

					break;
					}
				case IDC_DEFAULT:
					{
//get bias
					fBias = iBias->GetFVal();
					mod->fStitchBias = fBias;

//get align
					bAlign = IsDlgButtonChecked(hWnd,IDC_ALIGN_CHECK);
					mod->bStitchAlign = bAlign;
//set as defaults
					break;
					}
				case IDC_ALIGN_CHECK:
					{
//get align
					bAlign = IsDlgButtonChecked(hWnd,IDC_ALIGN_CHECK);
//revert hold
					theHold.Restore();
					mod->vsel = sel;

//call stitch again
					mod->fnStitchVerts(bAlign,fBias);
					mod->InvalidateView();
					break;
					}

				}
			break;

		default:
			return FALSE;
		}
	return TRUE;
	}



