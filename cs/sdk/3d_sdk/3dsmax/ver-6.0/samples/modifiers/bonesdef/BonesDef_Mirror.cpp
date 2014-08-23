
/**************************************************


***************************************************/

#include "mods.h"
#include "bonesdef.h"
#include "macrorec.h"


MirrorVertexData::MirrorVertexData()
{
	this->bmd = NULL;
}


MirrorData::MirrorData()
{

	Tab<MirrorVertexData*> localMirrorData;
	mod = NULL;

	

}


MirrorData::~MirrorData()
{
	Free();
}

void 
MirrorData::Free()
{
	
	for (int i = 0; i < localMirrorData.Count(); i++)
		{
		delete localMirrorData[i];
		}

	localMirrorData.ZeroCount();
	bonesMirrorList.ZeroCount();
}

BOOL MirrorData::Enabled()
{
	BOOL enabled = FALSE;
	if (mod)
		{
		mod->pblock_mirror->GetValue(skin_mirrorenabled,0,enabled,FOREVER);
		}
	return enabled;
}


void 
MirrorData::DrawBounds(GraphicsWindow *gw, Box3 bounds)
{

	Point3 plist[3];
	
	plist[0] = bounds[0];
	plist[1] = bounds[1];
	gw->polyline(2, plist, NULL, NULL, 1,NULL);

	plist[0] = bounds[1];
	plist[1] = bounds[3];
	gw->polyline(2, plist, NULL, NULL, 1,NULL);

	plist[0] = bounds[3];
	plist[1] = bounds[2];
	gw->polyline(2, plist, NULL, NULL, 1,NULL);

	plist[0] = bounds[2];
	plist[1] = bounds[0];
	gw->polyline(2, plist, NULL, NULL, 1,NULL);


	plist[0] = bounds[0+4];
	plist[1] = bounds[1+4];
	gw->polyline(2, plist, NULL, NULL, 1,NULL);

	plist[0] = bounds[1+4];
	plist[1] = bounds[3+4];
	gw->polyline(2, plist, NULL, NULL, 1,NULL);

	plist[0] = bounds[3+4];
	plist[1] = bounds[2+4];
	gw->polyline(2, plist, NULL, NULL, 1,NULL);

	plist[0] = bounds[2+4];
	plist[1] = bounds[0+4];
	gw->polyline(2, plist, NULL, NULL, 1,NULL);
					

	plist[0] = bounds[0];
	plist[1] = bounds[0+4];
	gw->polyline(2, plist, NULL, NULL, 1,NULL);

	plist[0] = bounds[1];
	plist[1] = bounds[1+4];
	gw->polyline(2, plist, NULL, NULL, 1,NULL);

	plist[0] = bounds[3];
	plist[1] = bounds[3+4];
	gw->polyline(2, plist, NULL, NULL, 1,NULL);

	plist[0] = bounds[2];
	plist[1] = bounds[2+4];
	gw->polyline(2, plist, NULL, NULL, 1,NULL);


}
//Draw display overrid
int 
MirrorData::DisplayMirrorData(GraphicsWindow *gw)
{

	int savedLimits;
	gw->setRndLimits((savedLimits = gw->getRndLimits()) & ~GW_ILLUM);


	if (!Enabled()) return 0;

	gw->setRndLimits(savedLimits & ~GW_ILLUM & ~GW_Z_BUFFER);

	Point3 gizmoColor = GetUIColor(COLOR_GIZMOS);
	Point3 selGizmoColor = GetUIColor(COLOR_SEL_GIZMOS);

	Point3 greyColor(0.3f,0.3f,0.3f);
	Point3 redColor(0.65f,0.0f,0.0f);
	Point3 greenColor(0.0f,0.65f,0.0f);
	Point3 blueColor(0.0f,0.0f,0.65f);

	Point3 bredColor(1.f,0.0f,0.0f);
	Point3 bgreenColor(0.f,1.0f,0.0f);
	Point3 bblueColor(0.f,0.0f,1.0f);


	//set our transform
	//draw our mirror plane
	gw->setColor(LINE_COLOR,gizmoColor);
	Matrix3 tm(1);
	int mirrorPlaneDir;

	GetMirrorTM(tm, mirrorPlaneDir);

	gw->setTransform(Inverse(tm));

	
	Box3 mirrorPlane = mirrorBounds;


	float offset;
	mod->pblock_mirror->GetValue(skin_mirroroffset,0,offset,FOREVER);


	mirrorPlane = mirrorPlane * localMirrorData[0]->bmd->BaseTM * tm;
	mirrorPlane.pmin[mirrorPlaneDir] = 0.0f;
	mirrorPlane.pmax[mirrorPlaneDir] = 0.0f;


	DrawBounds(gw, mirrorPlane);



	TimeValue t = GetCOREInterface()->GetTime();

	//draw the mirror bones
	for (int i = 0; i < bonesMirrorList.Count(); i++)
		{
		INode *node = mod->BoneData[i].Node;


		if (node)
			{
			Matrix3 tm = node->GetObjectTM(t);
			gw->setTransform(tm);
			Point3 plist[3];

	//	if not mirrored draw red
			if (bonesMirrorList[i].flags == NOT_MIRRORED)
				{
				gw->setColor(LINE_COLOR,bredColor);
				}
			else if (bonesMirrorList[i].selected)
				{
				gw->setColor(LINE_COLOR,selGizmoColor);				
				}
			else if (bonesMirrorList[i].flags == POS_MIRRORED)
				{
				int mirrorIndex = bonesMirrorList[i].index;
				if ((mirrorIndex != -1) && (bonesMirrorList[mirrorIndex].selected))
					gw->setColor(LINE_COLOR,bblueColor);
				else gw->setColor(LINE_COLOR,blueColor);
				}
			else if (bonesMirrorList[i].flags == NEG_MIRRORED)
				{
				int mirrorIndex = bonesMirrorList[i].index;
				if ((mirrorIndex != -1) && (bonesMirrorList[mirrorIndex].selected))
					gw->setColor(LINE_COLOR,bgreenColor);
				else gw->setColor(LINE_COLOR,greenColor);
				}


			Point3 pa,pb;
			mod->BoneData[i].EndPoint1Control->GetValue(t,&pa,FOREVER);
			mod->BoneData[i].EndPoint2Control->GetValue(t,&pb,FOREVER);

			plist[0] = pa;
			plist[1] = pb;

			Point3 vec = (plist[1] - plist[0]) * 0.1f;
			plist[0] += vec;
			plist[1] -= vec;



			gw->polyline(2, plist, NULL, NULL, 1,NULL);

			Point3 cent = (plist[0] + plist[1])*0.5f;


			gw->marker(&cent,DOT_MRKR);
			gw->marker(&plist[0],CIRCLE_MRKR);
			gw->marker(&plist[1],CIRCLE_MRKR);
			}
		}


	
	//draw mirror vertices
	int projection;
	mod->pblock_mirror->GetValue(skin_mirrorprojection,0,projection,FOREVER);

	if (projection == 3) //no display
		{
		gw->setRndLimits(savedLimits);
		return 1;
		}


	for (i = 0; i < localMirrorData.Count(); i++)
		{

		BoneModData *bmd = localMirrorData[i]->bmd;

		if (projection == 0) //normal projection
			{
			Matrix3 tm = bmd->BaseTM;
			gw->setTransform(tm);
			int numberOfVertices = bmd->VertexData.Count();

			gw->startMarkers();

			VertexListClass **vertexData = bmd->VertexData.Addr(0);
			VMirrorData *vertexMirrorList = localMirrorData[i]->vertexMirrorList.Addr(0);
			for (int j = 0; j < numberOfVertices; j++)
				{
				Point3 p = vertexData[j]->LocalPosPostDeform;

		//	if not mirrored draw red
				BOOL sel = FALSE;
				if (vertexMirrorList[j].flags == NOT_MIRRORED)
					{
					gw->setColor(LINE_COLOR,bredColor);
					}
				else if (bmd->selected[j])
					{
					gw->setColor(LINE_COLOR,selGizmoColor);				
					gw->marker(&p,SM_HOLLOW_BOX_MRKR);
					}

			//if selected draw blue
				else if (vertexMirrorList[j].flags == POS_MIRRORED)
					{
					int mirrorIndex = vertexMirrorList[j].index;
					
					if ((mirrorIndex != -1) && (bmd->selected[mirrorIndex]))
						{
						gw->setColor(LINE_COLOR,bblueColor);
						sel = TRUE;
						}
					else gw->setColor(LINE_COLOR,blueColor);
					}
	//if mirror then green
				else if (vertexMirrorList[j].flags == NEG_MIRRORED)
					{
					int mirrorIndex = vertexMirrorList[j].index;
					if ((mirrorIndex != -1) && (bmd->selected[mirrorIndex]))
						{
						gw->setColor(LINE_COLOR,bgreenColor);
						sel = TRUE;
						}
					else gw->setColor(LINE_COLOR,greenColor);
					}
				if (sel)
					gw->marker(&p,SM_HOLLOW_BOX_MRKR);
				else gw->marker(&p,POINT_MRKR);
				}
			gw->endMarkers();

			}
		else
			{
			//set up our tm in mirror space
			Matrix3 mtm;
			int mirrorPlaneDir;

			GetMirrorTM(mtm, mirrorPlaneDir);

			Matrix3 fromLocalSpaceToMirrorSpace;

			fromLocalSpaceToMirrorSpace = bmd->BaseTM * mtm;


			Matrix3 tm = Inverse(mtm);

			gw->setTransform(tm );

			gw->startMarkers();

			VertexListClass **vertexData = bmd->VertexData.Addr(0);
//			Point3 *localPosPostDeform = bmd->VertexData[j]->LocalPosPostDeform.Addr(0);
			
			for (int j = 0; j < bmd->VertexData.Count(); j++)
				{
				Point3 p = vertexData[j]->LocalPosPostDeform * fromLocalSpaceToMirrorSpace;
				//first draw our select

				if (bmd->selected[j])
					{
					gw->setColor(LINE_COLOR,selGizmoColor);				
					gw->marker(&p,SM_HOLLOW_BOX_MRKR);
					}

				if (projection == 1)
					{
					if (p[mirrorPlaneDir] > 0.0f)
//					if (localMirrorData[i]->vertexMirrorList[j].flags == POS_MIRRORED)
						{
						gw->setColor(LINE_COLOR,bblueColor);
						gw->marker(&p,POINT_MRKR);
						p[mirrorPlaneDir] *= -1.0f;
						gw->marker(&p,PLUS_SIGN_MRKR);
						}
					}
				else
					{
					if (p[mirrorPlaneDir] < 0.0f)
//					if (localMirrorData[i]->vertexMirrorList[j].flags == NEG_MIRRORED)
						{
						gw->setColor(LINE_COLOR,bgreenColor);
						gw->marker(&p,POINT_MRKR);
						p[mirrorPlaneDir] *= -1.0f;
						gw->marker(&p,PLUS_SIGN_MRKR);
						}

					}
				}
			gw->endMarkers();

			}
		}


	gw->setRndLimits(savedLimits);

	return 1;
}
//Hittest override


  //this returns the mirror tm and direction of the mirror plane
void MirrorData::GetMirrorTM(Matrix3 &tm, int &dir)
{
	//create mirror tm
	Matrix3 mtm = initialTM;
	Point3 p(0.0f,0.0f,0.0f);
	
	int mirrorPlaneDir;
	mod->pblock_mirror->GetValue(skin_mirrorplane,0,mirrorPlaneDir,FOREVER);

	float offset;
	mod->pblock_mirror->GetValue(skin_mirroroffset,0,offset,FOREVER);


	Point3 vec = Normalize(mtm.GetRow(mirrorPlaneDir));
	p = mtm.GetRow(3);
	p += vec * offset;
	mtm.SetRow(3,p);


	tm = Inverse(mtm);
	dir = mirrorPlaneDir;

}

//Builds our mirror bone connections
void MirrorData::BuildBonesMirrorData()
{
	if (!Enabled()) return;

	Matrix3 mtm;
	int mirrorPlaneDir;

	GetMirrorTM(mtm, mirrorPlaneDir);

	float threshold;
	mod->pblock_mirror->GetValue(skin_mirrorthreshold,0,threshold,FOREVER);

	
	for (int i = 0; i < bonesMirrorList.Count(); i++)
		{
		bonesMirrorList[i].index = -1;
		bonesMirrorList[i].flags = NOT_MIRRORED;
		}

	
	for (i = 0; i < bonesMirrorList.Count(); i++)
		{
		if (bonesMirrorList[i].index == -1)
			{
			int closest = -1;
			float closestDist = -1.0f;
			Point3 sa;
			sa = bonesMirrorList[i].initialBounds.Center() * mtm;
			for (int j = 0; j < bonesMirrorList.Count(); j++)
				{
				if (i!=j)
					{
					float dist = 0.0f;
					Point3 da,db;
					
					da = bonesMirrorList[j].initialBounds.Center() * mtm;
					da[mirrorPlaneDir] *= -1.0f;

					dist = Length(da-sa);

					if ((dist < closestDist) || (closest == -1))
						{
						 closestDist = dist ;
						 closest = j;
						}

					}
				}
			if ((closest != -1) && (closestDist <= threshold))
				{
				int iflag,jflag;
				if (sa[mirrorPlaneDir]  >= 0.0f)
					{
					iflag = POS_MIRRORED;
					jflag = NEG_MIRRORED;
					}
				else
					{
					iflag = NEG_MIRRORED;
					jflag = POS_MIRRORED;
					}
					

				if (bonesMirrorList[i].index == -1)
					{
					bonesMirrorList[i].index = closest;
					bonesMirrorList[i].flags = iflag;
					}
				if (bonesMirrorList[closest].index == -1)
					{
					bonesMirrorList[closest].index = i;
					bonesMirrorList[closest].flags = jflag;
					}

				}


			}
		}



}
//Builds our vertex mirror data
void MirrorData::BuildVertexMirrorData()
{
	if (!Enabled()) return;

	
	mod->pblock_mirror->GetValue(skin_mirrorfast,0,fastEngine,FOREVER);

	Matrix3 mtm;
	int mirrorPlaneDir;

	GetMirrorTM(mtm, mirrorPlaneDir);

	int gridDir;
	if (mirrorPlaneDir == 0)
		gridDir = 1;
	else if (mirrorPlaneDir == 1)
		gridDir = 0;
	else if (mirrorPlaneDir == 2)
		gridDir = 1;


	float threshold;
	mod->pblock_mirror->GetValue(skin_mirrorthreshold,0,threshold,FOREVER);

	
	for (int i = 0; i < localMirrorData.Count(); i++)
		{
		BoneModData *bmd = localMirrorData[i]->bmd;
		localMirrorData[i]->vertexMirrorList.SetCount(bmd->VertexData.Count());

		VMirrorData *vertexMirrorList = localMirrorData[i]->vertexMirrorList.Addr(0);

		for (int j = 0; j < localMirrorData[i]->vertexMirrorList.Count(); j++)
			{
			vertexMirrorList[j].index = -1;
			vertexMirrorList[j].flags = NOT_MIRRORED;
			}

		Matrix3 tm = bmd->BaseTM * mtm;

		Tab<Point3> mirrorPos;
		mirrorPos.SetCount(localMirrorData[i]->vertexMirrorList.Count());

		Tab<Point3> mirrorPosFlip;
		mirrorPosFlip.SetCount(localMirrorData[i]->vertexMirrorList.Count());

		
		Tab<int> posList,negList;
		Tab<Point3> posListP,negListP;

		Tab<Box3> boundsList;
		boundsList.SetCount(localMirrorData[i]->vertexMirrorList.Count());
		float threshSquared = threshold*threshold;

		int numberOfVertices = localMirrorData[i]->vertexMirrorList.Count();



		for (j = 0; j < numberOfVertices; j++)
			{
			mirrorPos[j] = bmd->VertexData[j]->LocalPos * tm;
			mirrorPosFlip[j] = mirrorPos[j];
			mirrorPosFlip[j][mirrorPlaneDir] *= -1.0f;


			boundsList[j].Init();
			boundsList[j] += mirrorPos[j];
			boundsList[j].EnlargeBy(threshSquared);

			if (mirrorPos[j][mirrorPlaneDir] >= 0.0f)
				{
				posListP.Append(1,&mirrorPosFlip[j],500);
				posList.Append(1,&j,500);
				}
			else
				{
				negListP.Append(1,&mirrorPosFlip[j],500);
				negList.Append(1,&j,500);
				}
			}


		if (fastEngine)
			{
			ngrid.InitializeGrid(500,gridDir);
			pgrid.InitializeGrid(500,gridDir);

			if (negListP.Count() >0)
				ngrid.LoadPoints(negListP.Addr(0),negListP.Count() );

			if (posList.Count() >0)
				pgrid.LoadPoints(posListP.Addr(0),posListP.Count() );
			}


		for (j = 0; j < numberOfVertices; j++)
			{
			if (vertexMirrorList[j].index == -1)
				{
				int closest = -1;
				float closestDist = -1.0f;
				Point3 sa;


				if (fastEngine)
					{
					sa = mirrorPos[j];
					Point3 da = mirrorPosFlip[j];

					if (sa[mirrorPlaneDir] >= 0.0f)					
						{
						ngrid.ClosestPoint(sa,threshold,closest,closestDist);
						if (closest != -1)
							closest = negList[closest];
						}
					else
						{
						pgrid.ClosestPoint(sa,threshold,closest,closestDist);
						if (closest != -1)
							closest = posList[closest];
						}
					}

				else
					{

					sa = mirrorPos[j];
					int arrayLength;
					int *pointList;

					if (sa[mirrorPlaneDir] >= 0.0f)
						{
						arrayLength = negList.Count();
						if (arrayLength > 0)
							pointList = negList.Addr(0);
						}
					else
						{
						arrayLength = posList.Count();
						if (arrayLength > 0)
							pointList = posList.Addr(0);

						}

					for (int k2 = 0; k2 < arrayLength; k2++)
						{
						int k = pointList[k2];

						if (j != k)
							{
							float dist = 0.0f;
							Point3 da;
							
							da = mirrorPosFlip[k];
	//						da[mirrorPlaneDir] *= -1.0f;

							if (boundsList[j].Contains(da))
								{

								dist = LengthSquared(da-sa);

								if ((dist < closestDist) || (closest == -1))
									{
									 closestDist = dist ;
									 closest = k;
									}
								}

							}
						}
					}

				if ((closest != -1) && (closest != j) && (closestDist <= threshSquared))
					{
					int jflag,kflag;
					if (sa[mirrorPlaneDir]  >= 0.0f)
						{
						jflag = POS_MIRRORED;
						kflag = NEG_MIRRORED;
						}
					else
						{
						jflag = NEG_MIRRORED;
						kflag = POS_MIRRORED;
						}
						

					if (vertexMirrorList[j].index == -1)
						{
						vertexMirrorList[j].index = closest;
						vertexMirrorList[j].flags = jflag;
						}

					if (localMirrorData[i]->vertexMirrorList[closest].index == -1)
						{
						vertexMirrorList[closest].index = j;
						vertexMirrorList[closest].flags = kflag;

						}


					}




				}
			}


		ngrid.FreeGrid();
		pgrid.FreeGrid();


		}


}

//Initialize all our connections to skin
void MirrorData::InitializeData(BonesDefMod *mod)
{

	//build our initial bonedata list
	this->mod = mod;

	int numberOfBones = mod->BoneData.Count();

	bonesMirrorList.SetCount(numberOfBones);


	TimeValue t = GetCOREInterface()->GetTime();


	for (int i = 0; i < numberOfBones; i++)
		{
		if (mod->BoneData[i].Node)
			{
			bonesMirrorList[i].flags = NOT_MIRRORED;
			bonesMirrorList[i].node = mod->BoneData[i].Node;

			Object *obj;
			obj = (bonesMirrorList[i].node->EvalWorldState(t)).obj;

			Box3 currentBounds;
			obj->GetDeformBBox(t,currentBounds);


			int largestAxis = 0;
			float adist = currentBounds.pmax.x - currentBounds.pmin.x;

			float d = currentBounds.pmax.y - currentBounds.pmin.y;
			if (d > adist)
				{
				largestAxis = 1;
				adist = d;
				}

			d = currentBounds.pmax.z - currentBounds.pmin.z;
			if (d > adist)
				{
				largestAxis = 2;
				adist = d;
				}


			Point3 pa = currentBounds.pmin;
			Point3 pb = currentBounds.pmax;


			if (largestAxis == 0)
				{
				pa.y = (pb.y + pa.y)*0.5f;
				pa.z = (pb.z + pa.z)*0.5f;

				pb.y = pa.y;
				pb.z = pa.z;
				}

			if (largestAxis == 1)
				{
				pa.x = (pb.x + pa.x)*0.5f;
				pa.z = (pb.z + pa.z)*0.5f;

				pb.x = pa.x;
				pb.z = pa.z;
				}

			if (largestAxis == 2)
				{
				pa.x = (pb.x + pa.x)*0.5f;
				pa.y = (pb.y + pa.y)*0.5f;

				pb.x = pa.x;
				pb.y = pa.y;
				}

			bonesMirrorList[i].pa = pa;
			bonesMirrorList[i].pb = pb;


			bonesMirrorList[i].selected = FALSE;

			bonesMirrorList[i].initialBounds = currentBounds * mod->BoneData[i].InitObjectTM;
			bonesMirrorList[i].currentBounds = bonesMirrorList[i].initialBounds;
			}
		}

	//get the number of instances
	//get the local data list
	//build our initial vertex list
	MyEnumProc dep;              
	mod->EnumDependents(&dep);

//number of nodes instanced to this one modifier
	int nodeCount = 0; 
	for (i = 0; i < dep.Nodes.Count(); i++)
		{
		BoneModData *bmd = mod->GetBMD(dep.Nodes[i]);
		if (bmd) 
			{

//buid our initial matrix which is based on the first node
			Tab<Point3> mpointList;
			
			if (nodeCount == 0)
				{
				initialTM = bmd->BaseTM;

				mirrorBounds.Init();
			
				mpointList.SetCount(bmd->VertexData.Count());

				for (int j = 0; j < bmd->VertexData.Count(); j++)
					{
					mpointList[j] = bmd->VertexData[j]->LocalPosPostDeform;
					mirrorBounds += bmd->VertexData[j]->LocalPosPostDeform;
					}


				}

			MirrorVertexData *data = new MirrorVertexData();
			data->bmd = bmd;


			localMirrorData.Append(1,&data);
			nodeCount++;
			}
		}

	
}


void MirrorData::PasteAllBones(BOOL posDir)
{
	//select all the approp bones
	for (int i = 0; i < bonesMirrorList.Count(); i++)
		{
		INode *node = mod->BoneData[i].Node;
		if (node)
			{

	//	if not mirrored draw red
			if (bonesMirrorList[i].flags == NOT_MIRRORED)
				{
				SelectBone(i,FALSE);
				}
			else if (posDir)
				{
				if (bonesMirrorList[i].flags == POS_MIRRORED)
					SelectBone(i,TRUE);
				else if (bonesMirrorList[i].flags == NEG_MIRRORED)
					SelectBone(i,FALSE);

				}
			else if (!posDir)
				{
				if (bonesMirrorList[i].flags == POS_MIRRORED)
					SelectBone(i,FALSE);
				else if (bonesMirrorList[i].flags == NEG_MIRRORED)
					SelectBone(i,TRUE);

				}
			}
		}

	//deselect all vertices
	for ( i = 0; i < localMirrorData.Count(); i++ ) 
		{
		BoneModData *bmd = localMirrorData[i]->bmd;

		mod->ClearVertexSelections(bmd);
		}
	

	//paste them
	Paste();

	mod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());

}
void MirrorData::PasteAllVertices(BOOL posDir)
{
//unselect all the bones
	ClearBoneSelection();
//deselect all vertices
	for (int i = 0; i < localMirrorData.Count(); i++ ) 
		{
		BoneModData *bmd = localMirrorData[i]->bmd;

		mod->ClearVertexSelections(bmd);
		}

	for ( i = 0; i < localMirrorData.Count(); i++ ) 
		{
		BoneModData *bmd = localMirrorData[i]->bmd;
		for (int j = 0; j < localMirrorData[i]->vertexMirrorList.Count(); j++)
			{
			if (localMirrorData[i]->vertexMirrorList[j].flags == POS_MIRRORED)
				{
				if (posDir) 
					bmd->selected.Set(j);
				}
			else if (localMirrorData[i]->vertexMirrorList[j].flags == NEG_MIRRORED)
				{
				if (!posDir) 
					bmd->selected.Set(j);

				}					

			}

		}
	//paste them
	Paste();

	mod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());


}


void MirrorData::PasteBones(TimeValue t, Matrix3 mtm, int mirrorPlaneDir, int sourceBone, int destBone)

{
	
//put end points in world space
	Point3 pa,pb;

	if ((sourceBone < 0) || (sourceBone >= mod->BoneData.Count()) )  return;
	if ((destBone < 0) || (destBone >= mod->BoneData.Count()) )  return;
	if (mod->BoneData[sourceBone].Node == NULL) return;
	if (mod->BoneData[destBone].Node == NULL) return;

	Matrix3 imtm = Inverse(mtm);

	
	
	mod->BoneData[sourceBone].EndPoint1Control->GetValue(t,&pa,FOREVER);
	mod->BoneData[sourceBone].EndPoint2Control->GetValue(t,&pb,FOREVER);

					
	pa = pa * mod->BoneData[sourceBone].InitObjectTM;
	pb = pb * mod->BoneData[sourceBone].InitObjectTM;

//mirror them
	pa = pa * mtm;
	pb = pb * mtm; //put em in mirror space

	pa[mirrorPlaneDir] *= -1.0f;	//mirror em
	pb[mirrorPlaneDir] *= -1.0f;

	pa = pa * imtm;	//put em back in world space
	pb = pb * imtm;

	pa = pa * Inverse(mod->BoneData[destBone].InitObjectTM);
	pb = pb * Inverse(mod->BoneData[destBone].InitObjectTM);

					
	SuspendAnimate();
	AnimateOff();

	mod->BoneData[destBone].EndPoint1Control->SetValue(t,pa);
	mod->BoneData[destBone].EndPoint2Control->SetValue(t,pb);

	ResumeAnimate();

	//copy the attributes also
	mod->BoneData[destBone].FalloffType = mod->BoneData[sourceBone].FalloffType;

	if (mod->BoneData[sourceBone].flags&BONE_ABSOLUTE_FLAG)
		mod->BoneData[destBone].flags |= BONE_ABSOLUTE_FLAG;
	else mod->BoneData[destBone].flags &= ~BONE_ABSOLUTE_FLAG;

	if (mod->BoneData[sourceBone].flags&BONE_DRAW_ENVELOPE_FLAG)
		mod->BoneData[destBone].flags |= BONE_DRAW_ENVELOPE_FLAG;
	else mod->BoneData[destBone].flags &= ~BONE_DRAW_ENVELOPE_FLAG;

//copy the cross sections
	int sourceCount, destCount;

	sourceCount = mod->BoneData[sourceBone].CrossSectionList.Count();
	destCount = mod->BoneData[destBone].CrossSectionList.Count();

	//if different make them the same
	//set them the same
	//copy the cross section sizes

	for (int j =(destCount-1); j >= 0 ; j--)
		mod->RemoveCrossSection(destBone, j);

	for (j =0; j < sourceCount ; j++)
		{
		float u, inner, outer;
		u = mod->BoneData[sourceBone].CrossSectionList[j].u;
		mod->BoneData[sourceBone].CrossSectionList[j].InnerControl->GetValue(t,&inner,FOREVER);
		mod->BoneData[sourceBone].CrossSectionList[j].OuterControl->GetValue(t,&outer,FOREVER);
		
		SuspendAnimate();
		AnimateOff();

		mod->AddCrossSection(destBone,u,inner,outer);
		ResumeAnimate();
		}

}

//the actual pasting of the mirror data
void MirrorData::Paste()
{
	if (!Enabled()) return;

	theHold.Begin();
	//hold all our bones
	theHold.Put(new PasteToAllRestore(mod));
	//hold our weights

	for ( int i = 0; i < localMirrorData.Count(); i++ ) 
		{
		BoneModData *bmd = localMirrorData[i]->bmd;
		bmd->reevaluate = TRUE;

		theHold.Put(new WeightRestore(mod,bmd));


		}

	Matrix3 mtm;
	int mirrorPlaneDir;

	GetMirrorTM(mtm, mirrorPlaneDir);

//	Matrix3 imtm = Inverse(mtm);


	//mirror the bones
	///loop through the bones
	TimeValue t = GetCOREInterface()->GetTime();
	int numberOfBones = mod->BoneData.Count();
	for ( i = 0; i < numberOfBones; i++)
		{
		if (mod->BoneData[i].Node)
			{
	//see if selected
			if (bonesMirrorList[i].selected)
				{
				int index = bonesMirrorList[i].index;
			//if so see if it has a mirror
				if (( index != -1) && (!bonesMirrorList[index].selected) && (mod->BoneData[index].Node) ) //make sure we have a valid mirror and that mirror is not also selected
					{
					int sourceBone = i;
					int destBone = index;

					PasteBones(t, mtm, mirrorPlaneDir, sourceBone, destBone);
					
					}
				}
			}
		}

	//now do weights

	for (i = 0; i < localMirrorData.Count(); i++ ) 
		{
		BoneModData *bmd = localMirrorData[i]->bmd;
		bmd->reevaluate = TRUE;
	
		for (int j = 0; j < localMirrorData[i]->vertexMirrorList.Count(); j++ ) 
			{
			if (bmd->selected[j])
				{
				int sourceVert = j;
				int destVert = localMirrorData[i]->vertexMirrorList[j].index;
				if ((destVert != -1) && (localMirrorData[i]->vertexMirrorList[j].flags != NOT_MIRRORED))
					{
					if (!bmd->selected[destVert])
						{
						bmd->VertexData[destVert]->flags = bmd->VertexData[sourceVert]->flags;
						int numberOfWeights = bmd->VertexData[j]->d.Count();
						bmd->VertexData[destVert]->d.SetCount(numberOfWeights);
						for (int k = 0; k < numberOfWeights; k++)
							{
							int sourceBone = bmd->VertexData[sourceVert]->d[k].Bones;
							int destBone = bonesMirrorList[sourceBone].index;

							if (destBone == -1) destBone = bmd->VertexData[sourceVert]->d[k].Bones;

							bmd->VertexData[destVert]->d[k].Bones = destBone;

							bmd->VertexData[destVert]->d[k].Influences = bmd->VertexData[sourceVert]->d[k].Influences;
							bmd->VertexData[destVert]->d[k].normalizedInfluences = bmd->VertexData[sourceVert]->d[k].normalizedInfluences;
							}
						}
					}
				}
			}
		}



	//then exclusions lists
	for (i = 0; i < localMirrorData.Count(); i++ ) 
		{
		BoneModData *bmd = localMirrorData[i]->bmd;

		for (int j = 0; j < bmd->exclusionList.Count(); j++)
			{
			ExclusionListClass *exclusionData = bmd->exclusionList[j];
			if (exclusionData)
				{
				int numberOfExcludedVerts = exclusionData->Count();

				int sourceBone = j;
				int destBone = bonesMirrorList[sourceBone].index;

				if (destBone == -1) destBone = sourceBone;
				
				BOOL isSelected;

				Tab<int> exclude;
				for (int k = 0; k < numberOfExcludedVerts; k++)
					{
					int sourceIndex = exclusionData->Vertex(k);
					if (bmd->selected[sourceIndex])
						{
						int destIndex = localMirrorData[i]->vertexMirrorList[sourceIndex].index;
						if (destIndex != -1)
							{
							isSelected = TRUE;
							exclude.Append(1,&destIndex);
							}
						}
					}

				if (isSelected)
					{
					theHold.Put(new ExclusionListRestore(mod,bmd,destBone)); 
					bmd->ExcludeVerts(destBone,exclude);
					}
				
				}
			}
		}

	theHold.Accept(GetString(IDS_MIRRORPASTE));

	mod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
}


void MirrorData::EnableUIButton(BOOL enable)
{
	EnableWindow(GetDlgItem(hWnd,IDC_MIRRORPASTE),enable);	
	EnableWindow(GetDlgItem(hWnd,IDC_MIRRORCOMBO),enable);	
	EnableWindow(GetDlgItem(hWnd,IDC_USEINTIALPOSE_CHECK),enable);	
	EnableWindow(GetDlgItem(hWnd,IDC_PROJECTIONCOMBO),enable);	

	EnableWindow(GetDlgItem(hWnd,IDC_GTOB_BONEPASTE),enable);	
	EnableWindow(GetDlgItem(hWnd,IDC_BTOG_BONEPASTE),enable);	

	EnableWindow(GetDlgItem(hWnd,IDC_GTOB_VERTSPASTE),enable);	
	EnableWindow(GetDlgItem(hWnd,IDC_BTOG_VERTSPASTE),enable);	

	if (enable)
		{
		SpinnerOn(hWnd,IDC_MIRROROFFSETSPIN,IDC_MIRROROFFSET);
		SpinnerOn(hWnd,IDC_MIRRORTHRESHOLDSPIN,IDC_MIRRORTHRESHOLD);
		}
	else 
		{
		SpinnerOff(hWnd,IDC_MIRROROFFSETSPIN,IDC_MIRROROFFSET);
		SpinnerOff(hWnd,IDC_MIRRORTHRESHOLDSPIN,IDC_MIRRORTHRESHOLD);
		}
}

void MirrorData::EnableMirrorButton(BOOL enable)
{

	ICustButton *iBut = GetICustButton(GetDlgItem(hWnd, IDC_MIRRORMODE));
	if (iBut)
		{
		iBut->Enable(enable);
		ReleaseICustButton(iBut);
		}

}



void MirrorData::ClearBoneSelection()
{
	for (int i = 0; i < bonesMirrorList.Count(); i++)
		{		
		bonesMirrorList[i].selected = FALSE;
		}

}
void MirrorData::SelectBone(int bone, BOOL  sel)
{
	if ((bone < 0) || (bone >= bonesMirrorList.Count()))
		return;

	bonesMirrorList[bone].selected = sel;

}

void MirrorData::SelectBones(BitArray sel)
{
	for (int i = 0; i < sel.GetSize(); i++)
		{
		if (i < bonesMirrorList.Count())
			{
			bonesMirrorList[i].selected = sel[i];
			}
		}
}

void MirrorData::EmitBoneSelectionScript()
{
	BitArray sel;
	sel.SetSize(bonesMirrorList.Count());
	for (int i = 0; i < sel.GetSize(); i++)
		{
		sel.Set(i,bonesMirrorList[i].selected);
		}

	macroRecorder->FunctionCall(_T("skinOps.selectMirrorBones"), 2, 0, mr_reftarg, mod,mr_bitarray, &sel);
}


BOOL MirrorMapDlgProc::DlgProc(
		TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
			
	{


	switch (msg) {
		case WM_INITDIALOG:
			{

			mod->mirrorData.hWnd = hWnd;
			mod->pblock_mirror->SetValue(skin_mirrorenabled,0,FALSE);

			ICustButton*   iMirrorButton;
			iMirrorButton = GetICustButton(GetDlgItem(hWnd, IDC_MIRRORMODE));
			iMirrorButton->SetType(CBT_CHECK);
			iMirrorButton->SetHighlightColor(GREEN_WASH);
			ReleaseICustButton(iMirrorButton);

			break;
			}
		case WM_COMMAND:

			switch (LOWORD(wParam)) 
				{
				case IDC_UPDATE:
					{
					mod->mirrorData.BuildBonesMirrorData();
					mod->mirrorData.BuildVertexMirrorData();
					macroRecorder->FunctionCall(_T("skinOps.updateMirror"), 1, 0, mr_reftarg, mod);

					mod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
					GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());

					break;
					}
				case IDC_GTOB_BONEPASTE:
					{
					mod->mirrorData.PasteAllBones(FALSE);					
					macroRecorder->FunctionCall(_T("skinOps.pasteAllBones"), 2, 0, mr_reftarg, mod,mr_bool,FALSE);
					break;
					}
				case IDC_BTOG_BONEPASTE:
					{
					mod->mirrorData.PasteAllBones(TRUE);					
					macroRecorder->FunctionCall(_T("skinOps.pasteAllBones"), 2, 0, mr_reftarg, mod,mr_bool,TRUE);
					break;

					}
				case IDC_GTOB_VERTSPASTE:
					{
					mod->mirrorData.PasteAllVertices(FALSE);					
					macroRecorder->FunctionCall(_T("skinOps.pasteAllVerts"), 2, 0, mr_reftarg, mod,mr_bool,FALSE);

					break;
					}
				case IDC_BTOG_VERTSPASTE:
					{
					mod->mirrorData.PasteAllVertices(TRUE);					
					macroRecorder->FunctionCall(_T("skinOps.pasteAllVerts"), 2, 0, mr_reftarg, mod,mr_bool,TRUE);

					break;
					}

				case IDC_MIRRORPASTE:
					{
					mod->mirrorData.Paste();
					macroRecorder->FunctionCall(_T("skinOps.mirrorPaste"), 1, 0, mr_reftarg, mod);
					break;
					}

	

				}
			break;


		}
	return FALSE;
	}
