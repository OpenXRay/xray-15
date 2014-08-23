#include "mods.h"
#include "iparamm.h"
#include "shape.h"
#include "spline3d.h"
#include "splshape.h"
#include "linshape.h"

// This uses the linked-list class templates
#include "linklist.h"
#include "bonesdef.h"


int	  BonesDefMod::GetMirrorBone(Point3 center, int axis)
	{
	//get current bone
	INode *node = BoneData[ModeBoneIndex].Node;
	if (node)
		{
	//get tm
		Matrix3 iMirrorTM(1);
		iMirrorTM.SetRow(3,center);
		iMirrorTM = Inverse(iMirrorTM);
		TimeValue t = GetCOREInterface()->GetTime();
		Matrix3 tm = node->GetNodeTM(t);
		Point3 mirrorPoint = Point3(0.0f,0.0f,0.0f) * tm;
		mirrorPoint = mirrorPoint * iMirrorTM;
	//mirror its location
		mirrorPoint[axis] *= -1.0f;




	//loop through all the bones
		int closestBone = -1;
		float closestDist= 0.0f;
		Box3 bounds;
		bounds.Init();
		for (int i = 0; i < BoneData.Count(); i++)
			{
			node = BoneData[i].Node;
	//get there tm
			if ((node) && (i!=ModeBoneIndex))
				{	
	//find closest bone
				tm = node->GetNodeTM(t);
				Point3 bonePoint = Point3(0.0f,0.0f,0.0f) * tm;
				bonePoint = bonePoint * iMirrorTM;
				bounds += bonePoint;
				float dist = Length(bonePoint-mirrorPoint);
				if ( (closestBone == -1) || (dist < closestDist) )
					{
					closestDist = dist;
					closestBone = i;
					}
				}
			}
		float threshold = Length(bounds.pmax-bounds.pmin)/100.0f;
		if (closestDist < threshold)
			return closestBone;

		}
	
	return -1;
	}

void  BonesDefMod::SetMirrorBone()
{
	mirrorIndex = -1;
	if (pPainterInterface->GetMirrorEnable())
		{
		Point3 center = pPainterInterface->GetMirrorPlaneCenter();
		int dir = pPainterInterface->GetMirrorAxis();

		float offset = pPainterInterface->GetMirrorOffset();
		center[dir] += offset;

		mirrorIndex = GetMirrorBone(center, dir);
		}

	if (mirrorIndex == -1)
		mirrorIndex = ModeBoneIndex;

}

void BonesDefMod::ApplyPaintWeights(BOOL alt, INode *incNode)
	{
//5.1.03
	BOOL blendMode = FALSE;
	pblock_param->GetValue(skin_paintblendmode,0,blendMode,FOREVER);

	INode *node = NULL;
	for (int i =0;i < painterData.Count(); i++)
		{
		painterData[i].alt = alt;
		if (incNode == NULL)
			node = painterData[i].node;
		else node = incNode;
		if (node == painterData[i].node)
			{
			BoneModData *bmd = painterData[i].bmd;
			int count;
			float *str = pPainterInterface->RetrievePointGatherStr(node, count);
			int *isMirror = pPainterInterface->RetrievePointGatherIsMirror(node, count);
			for (int j =0; j < count; j++)
				{
				if (str[j] != 0.0f)
					{
//get original amount
					float amount = 0.0f;
					int boneID = ModeBoneIndex;
					if (isMirror[j] == MIRRRORED) boneID = mirrorIndex;

					if(boneID != -1)
						{
//add to it str
						for (int k =0; k < bmd->VertexData[j]->d.Count(); k++)
							{
							if (bmd->VertexData[j]->d[k].Bones == boneID)
								{
								amount = bmd->VertexData[j]->d[k].normalizedInfluences;			
								k = bmd->VertexData[j]->d.Count();
								}
							}
//5.1.03
						if (blendMode)
							{
							float blurAmount = bmd->blurredWeights[j];
				 		    amount += (bmd->blurredWeights[j] - amount) * str[j];
				 		    }
				 		 else
				 			{
							if (alt)
								amount -= str[j];
							else 
								amount += str[j];
							if (amount > 1.0f) amount = 1.0f;
							if (amount < 0.0f) amount = 0.0f;
							}

						
//set it back
						SetVertex(bmd, j, boneID, amount);
						}
					}
				}

			}
		}
	}

BOOL  BonesDefMod::StartStroke()
	{
	theHold.Begin();
	int vertCount = 0;
	
//5.1.05
	BOOL blendMode = FALSE;
	pblock_param->GetValue(skin_paintblendmode,0,blendMode,FOREVER);


	
//this puts back the original state of the node vc mods and shade state
	for (int  i = 0; i < painterData.Count(); i++)
		{
		BoneModData *bmd = painterData[i].bmd;
		if (bmd)
			{
//5.1.05
			if (blendMode)
				bmd->BuildBlurData(ModeBoneIndex);
			
			theHold.Put(new WeightRestore(this,bmd,FALSE));
			vertCount += bmd->VertexData.Count();
			} 
		}
	lagRate = pPainterInterface->GetLagRate();

	lagHit = 1;
	for (i =0;i < painterData.Count(); i++)
		{
		painterData[i].node->FlagForeground(GetCOREInterface()->GetTime());
		}

//get mirror nodeindex
	SetMirrorBone();
	return TRUE;
	}
BOOL  BonesDefMod::PaintStroke(
						  BOOL hit,
						  IPoint2 mousePos, 
						  Point3 worldPoint, Point3 worldNormal,
						  Point3 localPoint, Point3 localNormal,
						  Point3 bary,  int index,
						  BOOL shift, BOOL ctrl, BOOL alt, 
						  float radius, float str,
						  float pressure, INode *node,
						  BOOL mirrorOn,
						  Point3 worldMirrorPoint, Point3 worldMirrorNormal,
						  Point3 localMirrorPoint, Point3 localMirrorNormal
						  ) 
	{
	theHold.Restore();

	int selNodeCount = GetCOREInterface()->GetSelNodeCount();  //this in case of instance modifiers
														  //theHold.Restore will wipe both instances
														  //so we need to compute weights for all nodes
														  // not just the one that got hit
	for (int i = 0; i < selNodeCount; i++)
		{
		ApplyPaintWeights(alt, GetCOREInterface()->GetSelNode(i));
		}

	lagHit++;
	if (  (lagRate == 0) ||  ((lagHit%lagRate) == 0)) NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);;
	
	return TRUE;
	}

BOOL  BonesDefMod::EndStroke(int ct, BOOL *hits, IPoint2 *mousePos, 
						  Point3 *worldPoint, Point3 *worldNormal,
						  Point3 *localPoint, Point3 *localNormal,
						  Point3 *bary,  int *index,
						  BOOL *shift, BOOL *ctrl, BOOL *alt, 
						  float *radius, float *str,
						  float *pressure, INode **node,
						  BOOL mirrorOn,
						  Point3 *worldMirrorPoint, Point3 *worldMirrorNormal,
						  Point3 *localMirrorPoint, Point3 *localMirrorNormal	)
	{
	theHold.Restore();
	ApplyPaintWeights(alt[ct-1], NULL);


	theHold.Accept(GetString(IDS_PW_WEIGHTCHANGE));
	NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
	RebuildPaintNodes();
	return TRUE;
	}

BOOL  BonesDefMod::EndStroke()
	{
	theHold.Restore();
	ApplyPaintWeights(painterData[0].alt, NULL);
	theHold.Accept(GetString(IDS_PW_WEIGHTCHANGE));
	NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
	RebuildPaintNodes();
	return TRUE;
	}
BOOL  BonesDefMod::CancelStroke()
	{
	theHold.Cancel();
	NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);;
	return TRUE;
	}
BOOL  BonesDefMod::SystemEndPaintSession()
	{
	//Unhook us from the painter interface
	//lets move this to the start session since some on may bump us off by starting another paint session and we wont know about it
	if (pPainterInterface) pPainterInterface->InitializeCallback(NULL);
	if (iPaintButton!=NULL) iPaintButton->SetCheck(FALSE);
	return TRUE;
	}

void BonesDefMod::RebuildPaintNodes()
	{
	//this sends all our dependant nodes to the painter
	MyEnumProc dep;              
	EnumDependents(&dep);
	Tab<INode *> nodes;
	for (int i = 0; i < nodes.Count(); i++)
		{

		ObjectState os = nodes[i]->EvalWorldState(GetCOREInterface()->GetTime());
						
		if ( (os.obj->NumPoints() != painterData[i].bmd->VertexData.Count()) ||
			  (painterData[i].bmd->isPatch) || (painterData[i].bmd->inputObjectIsNURBS) )
			{
			int ct = painterData[i].bmd->VertexData.Count();
			Tab<Point3> pointList;
			pointList.SetCount(ct);
			Matrix3 tm = nodes[i]->GetObjectTM(GetCOREInterface()->GetTime());
			for (int j =0; j < ct; j++)
				{
				pointList[j] = painterData[i].bmd->VertexData[j]->LocalPosPostDeform*tm;
				}
			pPainterInterface->LoadCustomPointGather(ct, pointList.Addr(0), nodes[i]);
			}
		}
	pPainterInterface->UpdateMeshes(TRUE);
	}

void BonesDefMod::CanvasStartPaint()
{
	if (iPaintButton!=NULL) iPaintButton->SetCheck(TRUE);
	
	pPainterInterface->SetBuildNormalData(FALSE);
	pPainterInterface->SetEnablePointGather(TRUE);

	//this sends all our dependant nodes to the painter
	MyEnumProc dep;              
	EnumDependents(&dep);
	Tab<INode *> nodes;
	painterData.ZeroCount();
	for (int  i = 0; i < dep.Nodes.Count(); i++)
		{
		BoneModData *bmd = GetBMD(dep.Nodes[i]);

		if (bmd)
			{
			nodes.Append(1,&dep.Nodes[i]);
			ObjectState os;
			PainterSaveData temp;
			temp.node = dep.Nodes[i];
			temp.bmd = bmd;
			painterData.Append(1,&temp);
			}
		}
	pPainterInterface->InitializeNodes(0, nodes);
	BOOL updateMesh = FALSE;
	for (i = 0; i < nodes.Count(); i++)
		{

		ObjectState os = nodes[i]->EvalWorldState(GetCOREInterface()->GetTime());
				
				
		if ( (os.obj->NumPoints() != painterData[i].bmd->VertexData.Count()) ||
			  (painterData[i].bmd->isPatch) || (painterData[i].bmd->inputObjectIsNURBS) )
			{
			int ct = painterData[i].bmd->VertexData.Count();
			Tab<Point3> pointList;
			pointList.SetCount(ct);
			Matrix3 tm = nodes[i]->GetObjectTM(GetCOREInterface()->GetTime());
			for (int j =0; j < ct; j++)
				{
				pointList[j] = painterData[i].bmd->VertexData[j]->LocalPosPostDeform*tm;
				}
			pPainterInterface->LoadCustomPointGather(ct, pointList.Addr(0), nodes[i]);
			updateMesh = TRUE;
			}
		}

	if (updateMesh)
		pPainterInterface->UpdateMeshes(TRUE);

//get mirror nodeindex
	SetMirrorBone();

	for (i = 0; i < nodes.Count(); i++)
		{
	
		painterData[i].node->FlagForeground(GetCOREInterface()->GetTime());
		}
		
//5.1.03
	for (i = 0; i < painterData.Count(); i++)
		{
		BoneModData *bmd = painterData[i].bmd;
//5.1.05
		bmd->BuildEdgeList();
		}		

}
void BonesDefMod::CanvasEndPaint()
{
//	pPainterInterface->InitializeCallback(NULL);
	if (iPaintButton!=NULL) iPaintButton->SetCheck(FALSE);
	
//5.1.03
	for (int  i = 0; i < painterData.Count(); i++)
		{
		BoneModData *bmd = painterData[i].bmd;
//5.1.05
		if (bmd)
			bmd->FreeEdgeList();
		}	
}

void BonesDefMod::StartPaintMode()
	{
	//hook us up to the painter interface
	//lets move this to the start session since some on may bump us off by starting another paint session and we wont know about it
	if (pPainterInterface)
		{
		pPainterInterface->InitializeCallback((ReferenceTarget *) this);
		pPainterInterface->StartPaintSession();  //all we need to do is call startpaintsession
												//with the 5.1 interface the painterinterface will
												//then call either CanvaseStartPaint or CanvaseEndPaint
												//this lets the painter turn off and on the paint mode when it command mode gets popped
		}
	}
void BonesDefMod::PaintOptions()
	{
	if (pPainterInterface) pPainterInterface->BringUpOptions();
	}