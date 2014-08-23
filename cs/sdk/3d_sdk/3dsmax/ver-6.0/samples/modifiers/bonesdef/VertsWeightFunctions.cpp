 /**********************************************************************
 
	FILE: VertsWeightFunctions.cpp

	DESCRIPTION:  Contains functions that set vertex weights

	CREATED BY: Peter Watje

	HISTORY: 12/12/01


 *>	Copyright (c) 1998, All Rights Reserved.


 **********************************************************************/
#include "max.h"
#include "mods.h"
#include "iparamm.h"
#include "shape.h"
#include "spline3d.h"
#include "splshape.h"
#include "linshape.h"
#include "surf_api.h"
#include "polyobj.h"

// This uses the linked-list class templates
#include "linklist.h"
#include "bonesdef.h"
#include "macrorec.h"
#include "modstack.h"
#include "ISkin.h"
#include "MaxIcon.h"



#ifdef _DEBUG
	#undef _DEBUG
	#include <atlbase.h>
	#define _DEBUG
#else
	#include <atlbase.h>
#endif

void WeightTableWindow::UpdateSpinner()
{
		for (int i = 0; i < modDataList.Count(); i++)
			{
			mod->BuildNormalizedWeight(modDataList[i].bmd);
			mod->UpdateEffectSpinner(modDataList[i].bmd);
			}

}

void WeightTableWindow::SetWeight(int currentVert, int currentBone, float value, BOOL update)
	{

	if ((currentBone < 0) || (currentBone >= mod->BoneData.Count()))
		return;

	if (currentVert == -1)
		{
		if (mod) mod->weightTableWindow.SetAllWeights( currentBone,value,update);
		}
	else
		{
//get weight 
	
//get index
		BOOL alreadyInList = FALSE;
		int boneID;

		if (currentVert >= vertexPtrList.Count()) return;

		VertexListClass *vd = vertexPtrList[currentVert].vertexData;

//check if modified
		if (!vd->IsModified())
			{
//normalize out the influence list
			vd->Modified (TRUE);
		//rebalance rest
			vd->NormalizeWeights();
			}

		if (vd->d.Count()==0)
			{
			VertexInfluenceListClass tempV;
			tempV.Bones = currentBone;
			tempV.Influences = value;

			if (mod->BoneData[currentBone].flags & BONE_SPLINE_FLAG)
				{
				Interval valid;
				Matrix3 ntm = mod->BoneData[currentBone].Node->GetObjTMBeforeWSM(mod->RefFrame,&valid);
				ntm = vertexPtrList[currentVert].bmd->BaseTM * Inverse(ntm);

				float garbage = mod->SplineToPoint(vd->LocalPos,
										&mod->BoneData[currentBone].referenceSpline,
			                            tempV.u,
										tempV.OPoints,tempV.Tangents,
										tempV.SubCurveIds,tempV.SubSegIds,
										ntm);
				}

			vd->d.Append(1,&tempV);
			}
		else
			{
			for (int i =0; i < vd->d.Count();i++)
				{
				if (vd->d[i].Bones == currentBone)
					{
					alreadyInList = TRUE;
					boneID = i;
					}
				}
//if normalize reset rest
//for right now we will always normalize until we get per verts attributes in
			BOOL normalize = TRUE;
			if (vd->IsUnNormalized())
				{
				if (alreadyInList)
					vd->d[boneID].Influences = value;
				else
					{
					VertexInfluenceListClass tempV;
					tempV.Bones = currentBone;
					tempV.Influences = value;

					if (mod->BoneData[currentBone].flags & BONE_SPLINE_FLAG)
						{
						Interval valid;
						Matrix3 ntm = mod->BoneData[currentBone].Node->GetObjTMBeforeWSM(mod->RefFrame,&valid);
						ntm = vertexPtrList[currentVert].bmd->BaseTM * Inverse(ntm);

						float garbage = mod->SplineToPoint(vd->LocalPos,
										&mod->BoneData[currentBone].referenceSpline,
			                            tempV.u,
										tempV.OPoints,tempV.Tangents,
										tempV.SubCurveIds,tempV.SubSegIds,
										ntm);
						}

					vd->d.Append(1,&tempV);
					}

				}
			else
				{
				if (value >= 1.0f) value = 1.0f;
				if (value < 0.0f) value = 0.0f;
				if (value == 1.0f)
					{
					for (int i=0; i < vd->d.Count(); i++)
						{
						vd->d[i].Influences = 0.0f;
						}
					if (alreadyInList)
						vd->d[boneID].Influences = 1.0f;
					else 
						{
						VertexInfluenceListClass tempV;
						tempV.Bones = currentBone;
						tempV.Influences = 1.0f;

						if (mod->BoneData[currentBone].flags & BONE_SPLINE_FLAG)
							{
							Interval valid;
							Matrix3 ntm = mod->BoneData[currentBone].Node->GetObjTMBeforeWSM(mod->RefFrame,&valid);
							ntm = vertexPtrList[currentVert].bmd->BaseTM * Inverse(ntm);

							float garbage = mod->SplineToPoint(vd->LocalPos,
										&mod->BoneData[currentBone].referenceSpline,
			                            tempV.u,
										tempV.OPoints,tempV.Tangents,
										tempV.SubCurveIds,tempV.SubSegIds,
										ntm);
							}

						vd->d.Append(1,&tempV);
						}
					}
				else if (value == 0.0f)
					{
					if (alreadyInList)
						{
						vd->d[boneID].Influences = 0.0f;
//rebalance rest
					
						float sum = 0.0f;
						for (int i=0; i < vd->d.Count(); i++)
							sum += vd->d[i].Influences;

						if (sum != 0.0f)
							{
							for (i=0; i < vd->d.Count(); i++)
								vd->d[i].Influences = vd->d[i].Influences/sum;

							}
						
						
						}
					}
				else
					{
					float remainder = 1.0f - value;
					float sum = 0.f;
	
					if (alreadyInList)
						{
						vd->d[boneID].Influences = 0.0f;
						for (int i=0; i < vd->d.Count(); i++)
							sum += vd->d[i].Influences;
						if (sum == 0.0f)
							vd->d[boneID].Influences = 1.0f;
						else
							{
							float per = remainder/sum;
							for (i=0; i < vd->d.Count(); i++)
								vd->d[i].Influences = vd->d[i].Influences * per;

							vd->d[boneID].Influences = value;
							}

						}
					else
						{
						for (int i=0; i < vd->d.Count(); i++)
							sum += vd->d[i].Influences;
						if (sum == 0.0f)
							{
							value = 1.0f;
							VertexInfluenceListClass tempV;
							tempV.Bones = currentBone;
							tempV.Influences = value;

							if (mod->BoneData[currentBone].flags & BONE_SPLINE_FLAG)
								{
								Interval valid;
								Matrix3 ntm = mod->BoneData[currentBone].Node->GetObjTMBeforeWSM(mod->RefFrame,&valid);
								ntm = vertexPtrList[currentVert].bmd->BaseTM * Inverse(ntm);

								float garbage = mod->SplineToPoint(vd->LocalPos,
										&mod->BoneData[currentBone].referenceSpline,
			                            tempV.u,
										tempV.OPoints,tempV.Tangents,
										tempV.SubCurveIds,tempV.SubSegIds,
										ntm);
								}

							vd->d.Append(1,&tempV);

							}
						else
							{
							float per = remainder/sum;
							for (i=0; i < vd->d.Count(); i++)
								vd->d[i].Influences = vd->d[i].Influences * per;
	
							VertexInfluenceListClass tempV;
							tempV.Bones = currentBone;
							tempV.Influences = value;

							if (mod->BoneData[currentBone].flags & BONE_SPLINE_FLAG)
								{
								Interval valid;
								Matrix3 ntm = mod->BoneData[currentBone].Node->GetObjTMBeforeWSM(mod->RefFrame,&valid);
								ntm = vertexPtrList[currentVert].bmd->BaseTM * Inverse(ntm);

								float garbage = mod->SplineToPoint(vd->LocalPos,
										&mod->BoneData[currentBone].referenceSpline,
			                            tempV.u,
										tempV.OPoints,tempV.Tangents,
										tempV.SubCurveIds,tempV.SubSegIds,
										ntm);
								}

							vd->d.Append(1,&tempV);
							}
						}	
					}
				}

			}

		}


	if (update)
		{
		if (mod) mod->weightTableWindow.UpdateSpinner();

		if (mod) mod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
		GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());	
		}
	BringDownEditField();

	}

void WeightTableWindow::SetAllWeights(int currentBone, float value, BOOL update)
	{
//	int currentBone = GetCurrentBone(x,y);
//get weight 
	
//get index
	for (int vIndex = 0; vIndex < vertexPtrList.Count(); vIndex++)
		{
		BOOL alreadyInList = FALSE;
		int boneID;

		VertexListClass *vd = vertexPtrList[vIndex].vertexData;

		BOOL process = TRUE;

		if (GetAffectSelectedOnly())
			{
			if (!vertexPtrList[vIndex].IsSelected())
				process = FALSE;
			}

		if (process)
			{
//check if modified
			if (!vd->IsModified())
				{
//normalize out the influence list
				vd->Modified (TRUE);
		//rebalance rest
				float sum = 0.0f;
				for (int i=0; i < vd->d.Count(); i++)
				sum += vd->d[i].Influences;
				for (i=0; i < vd->d.Count(); i++)
					vd->d[i].Influences = vd->d[i].Influences/sum;
				}

			for (int i =0; i < vd->d.Count();i++)
				{
				if (vd->d[i].Bones == currentBone)
					{
					alreadyInList = TRUE;
					boneID = i;
					}
				}
//if normalize reset rest
//for right now we will always normalize until we get per verts attributes in
			if (vd->IsUnNormalized())
				{
				if (alreadyInList)
					vd->d[boneID].Influences = value;
				else
					{
					VertexInfluenceListClass tempV;
					tempV.Bones = currentBone;
					tempV.Influences = value;

					if (mod->BoneData[currentBone].flags & BONE_SPLINE_FLAG)
						{
						Interval valid;
						Matrix3 ntm = mod->BoneData[currentBone].Node->GetObjTMBeforeWSM(mod->RefFrame,&valid);
						ntm = vertexPtrList[vIndex].bmd->BaseTM * Inverse(ntm);

						float garbage = mod->SplineToPoint(vd->LocalPos,
										&mod->BoneData[currentBone].referenceSpline,
			                            tempV.u,
										tempV.OPoints,tempV.Tangents,
										tempV.SubCurveIds,tempV.SubSegIds,
										ntm);
						}

					vd->d.Append(1,&tempV);
					}
				}
			else
				{
				if (value >= 1.0f) value = 1.0f;
				if (value < 0.0f) value = 0.0f;
				if (value == 1.0f)
					{
					for (int i=0; i < vd->d.Count(); i++)
						{
						vd->d[i].Influences = 0.0f;
						}
					if (alreadyInList)
						vd->d[boneID].Influences = 1.0f;
					else 
						{
						VertexInfluenceListClass tempV;
						tempV.Bones = currentBone;
						tempV.Influences = 1.0f;

						if (mod->BoneData[currentBone].flags & BONE_SPLINE_FLAG)
							{
							Interval valid;
							Matrix3 ntm = mod->BoneData[currentBone].Node->GetObjTMBeforeWSM(mod->RefFrame,&valid);
							ntm = vertexPtrList[vIndex].bmd->BaseTM * Inverse(ntm);

							float garbage = mod->SplineToPoint(vd->LocalPos,
										&mod->BoneData[currentBone].referenceSpline,
			                            tempV.u,
										tempV.OPoints,tempV.Tangents,
										tempV.SubCurveIds,tempV.SubSegIds,
										ntm);
							}

						vd->d.Append(1,&tempV);
						}
					}
				else if (value == 0.0f)
					{
					if (alreadyInList)
						{
						vd->d[boneID].Influences = 0.0f;
//rebalance rest
						float sum = 0.0f;
						for (int i=0; i < vd->d.Count(); i++)
							sum += vd->d[i].Influences;

						if (sum != 0.0f)
							{
							for (i=0; i < vd->d.Count(); i++)
								vd->d[i].Influences = vd->d[i].Influences/sum;

							}
						
						}
					}
				else
					{
					float remainder = 1.0f - value;
					float sum = 0.f;
	
					if (alreadyInList)
						{
						vd->d[boneID].Influences = 0.0f;
						for (int i=0; i < vd->d.Count(); i++)
							sum += vd->d[i].Influences;
						if (sum == 0.0f)
							vd->d[boneID].Influences = 1.0f;
						else
							{
							float per = remainder/sum;
							for (i=0; i < vd->d.Count(); i++)
								vd->d[i].Influences = vd->d[i].Influences * per;

							vd->d[boneID].Influences = value;
							}

						}
					else
						{
						for (int i=0; i < vd->d.Count(); i++)
						sum += vd->d[i].Influences;
						if (sum == 0.0f)
							{
							value = 1.0f;
							VertexInfluenceListClass tempV;
							tempV.Bones = currentBone;
							tempV.Influences = value;

							if (mod->BoneData[currentBone].flags & BONE_SPLINE_FLAG)
								{
								Interval valid;
								Matrix3 ntm = mod->BoneData[currentBone].Node->GetObjTMBeforeWSM(mod->RefFrame,&valid);
								ntm = vertexPtrList[vIndex].bmd->BaseTM * Inverse(ntm);

								float garbage = mod->SplineToPoint(vd->LocalPos,
										&mod->BoneData[currentBone].referenceSpline,
			                            tempV.u,
										tempV.OPoints,tempV.Tangents,
										tempV.SubCurveIds,tempV.SubSegIds,
										ntm);
								}

							vd->d.Append(1,&tempV);

							}
						else
							{
							float per = remainder/sum;
							for (i=0; i < vd->d.Count(); i++)
								vd->d[i].Influences = vd->d[i].Influences * per;

							VertexInfluenceListClass tempV;
							tempV.Bones = currentBone;
							tempV.Influences = value;

							if (mod->BoneData[currentBone].flags & BONE_SPLINE_FLAG)
								{
								Interval valid;
								Matrix3 ntm = mod->BoneData[currentBone].Node->GetObjTMBeforeWSM(mod->RefFrame,&valid);
								ntm = vertexPtrList[vIndex].bmd->BaseTM * Inverse(ntm);

								float garbage = mod->SplineToPoint(vd->LocalPos,
										&mod->BoneData[currentBone].referenceSpline,
			                            tempV.u,
										tempV.OPoints,tempV.Tangents,
										tempV.SubCurveIds,tempV.SubSegIds,
										ntm);
								}

							vd->d.Append(1,&tempV);
							}
						}
					}

				}
			}
		}
	if (update)
		{
		if (mod) mod->weightTableWindow.UpdateSpinner();

		if (mod) mod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
		GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());	
		}
	BringDownEditField();

	}


void BonesDefMod::SetVertex(BoneModData *bmd,int vertID, int BoneID, float amount)

	{

	if (BoneData[BoneID].flags & BONE_LOCK_FLAG)
		return;

	ObjectState os;
	ShapeObject *pathOb = NULL;

	if (BoneData[BoneID].flags & BONE_SPLINE_FLAG)
		{
		os = BoneData[BoneID].Node->EvalWorldState(ip->GetTime());
		pathOb = (ShapeObject*)os.obj;
		}


	float effect,originaleffect;
		
	int found = 0;
	int id;

//	int k = vsel[i];
	int k = vertID;
	for (int j =0; j<bmd->VertexData[k]->d.Count();j++)
		{
		if ( (bmd->VertexData[k]->d[j].Bones == BoneID)&& (!(BoneData[bmd->VertexData[k]->d[j].Bones].flags & BONE_LOCK_FLAG)))

			{
			originaleffect = bmd->VertexData[k]->d[j].normalizedInfluences;
			bmd->VertexData[k]->d[j].normalizedInfluences = amount;
			found = 1;
			id =j;
			effect = bmd->VertexData[k]->d[j].normalizedInfluences;
			j = bmd->VertexData[k]->d.Count();

			}
		}

	if (bmd->VertexData[k]->IsUnNormalized())
		{
		if (found == 0)
			{
			VertexInfluenceListClass td;
			td.Bones = BoneID;
			td.normalizedInfluences = amount;
			bmd->VertexData[k]->d.Append(1,&td,1);
			}
		else
			{
			bmd->VertexData[k]->d[id].normalizedInfluences = amount;
			bmd->VertexData[k]->d[id].Influences = amount;
			}
		return;
		}

	if ((found == 0) && (amount > 0.0f))
		{

		VertexInfluenceListClass td;
		td.Bones = BoneID;
		td.normalizedInfluences = amount;
//check if spline if so add approriate spline data info also
// find closest spline
		
		if (BoneData[BoneID].flags & BONE_SPLINE_FLAG)
			{
			Interval valid;
			Matrix3 ntm = BoneData[BoneID].Node->GetObjTMBeforeWSM(RefFrame,&valid);
			ntm = bmd->BaseTM * Inverse(ntm);

			float garbage = SplineToPoint(bmd->VertexData[k]->LocalPos,
										&BoneData[BoneID].referenceSpline,
			                            td.u,
										td.OPoints,td.Tangents,
										td.SubCurveIds,td.SubSegIds,
										ntm);
			}


		bmd->VertexData[k]->d.Append(1,&td,1);
		effect = amount;
		originaleffect = 0.0f;
		found = 1;
		}

	if (found == 1)
		{
		int bc = bmd->VertexData[k]->d.Count();




//rebalance rest
		float remainder = 1.0f - effect;
		originaleffect = 1.0f - originaleffect;
		if (bmd->VertexData[k]->d.Count() > 1)
			{
			for (j=0;j<bmd->VertexData[k]->d.Count();j++)
				{
	
				if (!(BoneData[bmd->VertexData[k]->d[j].Bones].flags & BONE_LOCK_FLAG))
					{
					if (bmd->VertexData[k]->d[j].Bones!=BoneID)
						{
						float per = 0.0f;
						if (remainder == 0.0f)
							{
							bmd->VertexData[k]->d[j].normalizedInfluences = 0.0f;
							}
						else if (originaleffect == 0.0f)
							{
							bmd->VertexData[k]->d[j].normalizedInfluences = remainder/(bmd->VertexData[k]->d.Count()-1.0f);
							}
						else
							{
							per = bmd->VertexData[k]->d[j].normalizedInfluences/originaleffect;
							bmd->VertexData[k]->d[j].normalizedInfluences = remainder*per;

							}
						}
					}
				}
			for (j=0;j<bmd->VertexData[k]->d.Count();j++)
				bmd->VertexData[k]->d[j].Influences = bmd->VertexData[k]->d[j].normalizedInfluences;
			}
		else
			{
			bmd->VertexData[k]->d[0].Influences = amount;
			bmd->VertexData[k]->d[0].normalizedInfluences = amount;
			}



		bmd->VertexData[k]->Modified (TRUE);

		}
	
//WEIGHTTABLE
	PaintAttribList();
	}



void BonesDefMod::SetVertices(BoneModData *bmd,int vertID, Tab<int> BoneIDList, Tab<float> amountList)

{
//Tab<int> vsel;


ObjectState os;
ShapeObject *pathOb = NULL;



float effect,originaleffect;
int k = vertID;

for (int i = 0; i < amountList.Count();i++)
	{
	int found = 0;

	float amount = amountList[i];
	int BoneID = BoneIDList[i];
	if (BoneData[BoneID].flags & BONE_SPLINE_FLAG)
		{
		os = BoneData[BoneID].Node->EvalWorldState(ip->GetTime());
		pathOb = (ShapeObject*)os.obj;
		}
	int id;
	for (int j =0; j<bmd->VertexData[k]->d.Count();j++)
		{
		if ( (bmd->VertexData[k]->d[j].Bones == BoneID)&& (!(BoneData[bmd->VertexData[k]->d[j].Bones].flags & BONE_LOCK_FLAG)))

			{
			originaleffect = bmd->VertexData[k]->d[j].Influences;
			bmd->VertexData[k]->d[j].Influences = amount;
			found = 1;
			id = j;
			effect = bmd->VertexData[k]->d[j].Influences;
			j = bmd->VertexData[k]->d.Count();

			}
		}

	if (bmd->VertexData[k]->IsUnNormalized())
		{
		if (found == 0)
			{
			VertexInfluenceListClass td;
			td.Bones = BoneID;
			td.normalizedInfluences = amount;
			bmd->VertexData[k]->d.Append(1,&td,1);
			}
		else
			{
			bmd->VertexData[k]->d[id].normalizedInfluences = amount;
			bmd->VertexData[k]->d[id].Influences = amount;
			}
	
		}
	else
		{
		if ((found == 0) && (amount > 0.0f))
			{
	
			VertexInfluenceListClass td;
			td.Bones = BoneID;
			td.Influences = amount;
//check if spline if so add approriate spline data info also
// find closest spline
		
			if (BoneData[BoneID].flags & BONE_SPLINE_FLAG)
				{
				Interval valid;
				Matrix3 ntm = BoneData[BoneID].Node->GetObjTMBeforeWSM(RefFrame,&valid);
				ntm = bmd->BaseTM * Inverse(ntm);

				float garbage = SplineToPoint(bmd->VertexData[k]->LocalPos,
										&BoneData[BoneID].referenceSpline,
			                            td.u,
										td.OPoints,td.Tangents,
										td.SubCurveIds,td.SubSegIds,
										ntm);
				}


			bmd->VertexData[k]->d.Append(1,&td,1);
			effect = amount;
			originaleffect = 0.0f;
			found = 1;
			}

		if (found == 1)
			{
			int bc = bmd->VertexData[k]->d.Count();

//remove 0 influence bones otherwise they skew the reweigthing
			for (j=0;j<bmd->VertexData[k]->d.Count();j++)
				{
				if (bmd->VertexData[k]->d[j].Influences==0.0f)
					{
					bmd->VertexData[k]->d.Delete(j,1);
					j--;
					}
				}	


//rebalance rest

			}
		}
	}
if (!bmd->VertexData[k]->IsUnNormalized())
	{
	bmd->VertexData[k]->Modified (TRUE);
	if (bmd->VertexData[k]->d.Count() > 1)
		{
		float totalSum = 0.0f;
		for (int j=0;j<bmd->VertexData[k]->d.Count();j++)
			{
			if (bmd->VertexData[k]->d[j].Influences==0.0f)
				{
				bmd->VertexData[k]->d.Delete(j,1);
				j--;
				}	
			}

		for (j=0;j<bmd->VertexData[k]->d.Count();j++)
			totalSum += bmd->VertexData[k]->d[j].Influences;
		for (j=0;j<bmd->VertexData[k]->d.Count();j++)
			{
			 bmd->VertexData[k]->d[j].Influences = bmd->VertexData[k]->d[j].Influences/totalSum;
			}
		}
	}

//WEIGHTTABLE
PaintAttribList();

}



  
void BonesDefMod::SetSelectedVertices(BoneModData *bmd, int BoneID, float amount)

{
Tab<int> vsel;

if (BoneData[BoneID].flags & BONE_LOCK_FLAG)
	return;

ObjectState os;
ShapeObject *pathOb = NULL;

if (BoneData[BoneID].flags & BONE_SPLINE_FLAG)
	{
	os = BoneData[BoneID].Node->EvalWorldState(ip->GetTime());
	pathOb = (ShapeObject*)os.obj;
	}


int selcount = bmd->selected.GetSize();

for (int i = 0 ; i <bmd->VertexData.Count();i++)
	{
	if (bmd->selected[i]) vsel.Append(1,&i,1);
	}

//float effect,originaleffect;
for ( i = 0; i < vsel.Count();i++)
	{
	int found = 0;

	int k = vsel[i];
	SetVertex(bmd,k, BoneID, amount);
/*
	for (int j =0; j<bmd->VertexData[k]->d.Count();j++)
		{
		if ( (bmd->VertexData[k]->d[j].Bones == BoneID)&& (!(BoneData[bmd->VertexData[k]->d[j].Bones].flags & BONE_LOCK_FLAG)))

			{
			originaleffect = bmd->VertexData[k]->d[j].normalizedInfluences;
			bmd->VertexData[k]->d[j].normalizedInfluences = amount;
			found = 1;
			effect = bmd->VertexData[k]->d[j].normalizedInfluences;
			j = bmd->VertexData[k]->d.Count();

			}
		}

	if ((found == 0) && (amount > 0.0f))
		{

		VertexInfluenceListClass td;
		td.Bones = BoneID;
		td.normalizedInfluences = amount;
//check if spline if so add approriate spline data info also
// find closest spline
		
		if (BoneData[BoneID].flags & BONE_SPLINE_FLAG)
			{
			Interval valid;
			Matrix3 ntm = BoneData[BoneID].Node->GetObjTMBeforeWSM(RefFrame,&valid);
			ntm = bmd->BaseTM * Inverse(ntm);

			float garbage = SplineToPoint(bmd->VertexData[k]->LocalPos,
										&BoneData[BoneID].referenceSpline,
			                            td.u,
										td.OPoints,td.Tangents,
										td.SubCurveIds,td.SubSegIds,
										ntm);
			}


		bmd->VertexData[k]->d.Append(1,&td,1);
		effect = amount;
		originaleffect = 0.0f;
		found = 1;
		}

	if (found == 1)
		{
		int bc = bmd->VertexData[k]->d.Count();



//rebalance rest
		float remainder = 1.0f - effect;
		originaleffect = 1.0f - originaleffect;
		if (bmd->VertexData[k]->d.Count() > 1)
			{
			for (j=0;j<bmd->VertexData[k]->d.Count();j++)
				{
	
				if (!(BoneData[bmd->VertexData[k]->d[j].Bones].flags & BONE_LOCK_FLAG))
					{
					if (bmd->VertexData[k]->d[j].Bones!=BoneID)
						{
						float per = 0.0f;
						if (remainder == 0.0f)
							{
							bmd->VertexData[k]->d[j].normalizedInfluences = 0.0f;
							}
						else if (originaleffect == 0.0f)
							{
							bmd->VertexData[k]->d[j].normalizedInfluences = remainder/(bmd->VertexData[k]->d.Count()-1.0f);
							}
						else
							{
							per = bmd->VertexData[k]->d[j].normalizedInfluences/originaleffect;
							bmd->VertexData[k]->d[j].normalizedInfluences = remainder*per;

							}

						}
					}
				}
			for (j=0;j<bmd->VertexData[k]->d.Count();j++)
				bmd->VertexData[k]->d[j].Influences = bmd->VertexData[k]->d[j].normalizedInfluences;
			}
		else
			{
			bmd->VertexData[k]->d[0].Influences = amount;
			bmd->VertexData[k]->d[0].normalizedInfluences = amount;
			}

		bmd->VertexData[k]->Modified (TRUE);

		}
*/
	}
//WEIGHTTABLE
PaintAttribList();
}



void BonesDefMod::NormalizeWeight(BoneModData *bmd,int vertID, BOOL unNormalize)
	{

	//find vert
	if ((vertID >= 0) && (vertID < bmd->VertexData.Count()))
		{
//only normalize the data if
		if ((!bmd->VertexData[vertID]->IsModified()) || (!unNormalize))
			{
			float sum = 0.0f;
			for (int j=0;j<bmd->VertexData[vertID]->d.Count();j++)
				{
	//get sum
				sum += bmd->VertexData[vertID]->d[j].Influences;
				}
	//normalize everything
			for (j=0;j<bmd->VertexData[vertID]->d.Count();j++)
				{
				if (sum == 0.0f)
					bmd->VertexData[vertID]->d[j].Influences = 0.0f;
				else bmd->VertexData[vertID]->d[j].Influences = bmd->VertexData[vertID]->d[j].Influences/sum;
				bmd->VertexData[vertID]->d[j].normalizedInfluences = bmd->VertexData[vertID]->d[j].Influences;
				}
			}
		bmd->VertexData[vertID]->UnNormalized(unNormalize);
		PaintAttribList();
		}
	}



float BonesDefMod::RetrieveNormalizedWeight(BoneModData *bmd, int vid, int bid)
{
return bmd->VertexData[vid]->d[bid].normalizedInfluences;
}


static int InfluSort( const void *elem1, const void *elem2 ) {
	VertexInfluenceListClass *a = (VertexInfluenceListClass *)elem1;
	VertexInfluenceListClass *b = (VertexInfluenceListClass *)elem2;
	if ( a->Influences == b->Influences)
		return 0;
	if ( a->Influences > b->Influences)
		return -1;
	else
		return 1;

}

void BonesDefMod::BuildNormalizedWeight(BoneModData *bmd)
{
//need to reqeight based on remainder
//right now if the UI is up redo all verts else uses the cache
//if ((bmd->nukeValidCache == TRUE) || (ip) /* this can be taken out when we work on the cache*/ || (bmd->validVerts.GetSize() != bmd->VertexData.Count()))
if(1) // nedd to fix the cache when i get time
	{
	bmd->validVerts.SetSize(bmd->VertexData.Count());
	bmd->validVerts.ClearAll();
	bmd->nukeValidCache = FALSE;
	}

for (int vid = 0; vid < bmd->VertexData.Count(); vid++)
	{
	float tempdist=0.0f;
	float w;

	if (!bmd->validVerts[vid])
		{
//		bmd->validVerts.Set(vid,TRUE);

		if (bmd->VertexData[vid]->d.Count() > boneLimit)
			{
			bmd->VertexData[vid]->d.Sort(InfluSort);
			if (bmd->VertexData[vid]->d.Count() > boneLimit) 
				{
				bmd->VertexData[vid]->d.SetCount(boneLimit);
/*			for (int j = (boneLimit); j < bmd->VertexData[vid]->d.Count(); j++)
				{
				bmd->VertexData[vid]->d[j].Influences = 0.0f;
				}
*/
//				bmd->VertexData[vid]->NormalizeWeights();
				}

			}

		if (bmd->VertexData[vid]->IsRigid())
			{
//find largest influence set it to 1 and the rest to 0
			float largest = -1.0f;
			int id = -1;
			for (int j =0; j < bmd->VertexData[vid]->d.Count(); j++)
				{
				if (bmd->VertexData[vid]->d[j].Influences > largest)
					{
					id = j;
					largest = bmd->VertexData[vid]->d[j].Influences;
					}
				}
			if (id != -1)
				{
				for (int j =0; j < bmd->VertexData[vid]->d.Count(); j++)
					bmd->VertexData[vid]->d[j].normalizedInfluences = 0.0f;

				 bmd->VertexData[vid]->d[id].normalizedInfluences = 1.0f;
				}

			}
		else if (bmd->VertexData[vid]->IsUnNormalized())
			{
			for (int j =0; j < bmd->VertexData[vid]->d.Count(); j++)
				bmd->VertexData[vid]->d[j].normalizedInfluences  = bmd->VertexData[vid]->d[j].Influences;
			}
		else if (bmd->VertexData[vid]->d.Count() == 1) 
			{
//if only one bone and absolute control set to it to max control
			if ( (BoneData[bmd->VertexData[vid]->d[0].Bones].flags & BONE_ABSOLUTE_FLAG) )
				{
				if (bmd->VertexData[vid]->IsModified())
					{
					if (bmd->VertexData[vid]->d[0].Influences > 0.5f) 
						w = 1.0f;
					else w = 0.0f;
					}
				else
					{
					if (bmd->VertexData[vid]->d[0].Influences > 0.0f) 
						w = 1.0f;
					else w = 0.0f;
					}
				}
			else w = bmd->VertexData[vid]->d[0].Influences;
			
			bmd->VertexData[vid]->d[0].normalizedInfluences  = w;
			}
		else if (bmd->VertexData[vid]->d.Count() >= 1) 
			{
			tempdist = 0.0f;
			for (int j =0; j < bmd->VertexData[vid]->d.Count(); j++)
				{
				int bd = bmd->VertexData[vid]->d[j].Bones;

				if (BoneData[bd].Node == NULL)
					{
					bmd->VertexData[vid]->d[j].normalizedInfluences = 0.0f;
					bmd->VertexData[vid]->d[j].Influences = 0.0f;
					}
				else
					{
					float infl = bmd->VertexData[vid]->d[j].Influences;
					int bone=bmd->VertexData[vid]->d[j].Bones;
					tempdist += infl;
					}
				}
			for (j=0; j<bmd->VertexData[vid]->d.Count(); j++) 
				{
				w = (bmd->VertexData[vid]->d[j].Influences)/tempdist;
				bmd->VertexData[vid]->d[j].normalizedInfluences  = (bmd->VertexData[vid]->d[j].Influences)/tempdist;
				}


			}

		}

	}

if (bmd->isPatch)
	{
	for (vid = 0; vid < bmd->VertexData.Count(); vid++)
		{

		if (!bmd->validVerts[vid])
			{
			if ((bmd->VertexData[vid]->IsRigidHandle()) && (bmd->vecOwnerList[vid]!=-1))
				{
				int ownerID = bmd->vecOwnerList[vid];
				bmd->VertexData[vid]->d = bmd->VertexData[ownerID]->d;
				}
			}

	}


}

bmd->validVerts.SetAll();
}


void BonesDefMod::NormalizeSelected(BOOL norm)
	{
	ModContextList mcList;		
	INodeTab nodes;


	if (ip)
		{
		ip->GetModContexts(mcList,nodes);
		int objects = mcList.Count();

		for (int i =0; i < objects; i++)
			{
			BoneModData *bmd = (BoneModData*)mcList[0]->localData;
			for (int j =0; j < bmd->VertexData.Count(); j++)
				{
				if (bmd->selected[j])
					{
					bmd->VertexData[j]->UnNormalized(!norm);
					
					}
				}	
			}
		}

	}

void BonesDefMod::RigidSelected(BOOL rigid)
	{
	ModContextList mcList;		
	INodeTab nodes;


	if (ip)
		{
		ip->GetModContexts(mcList,nodes);
		int objects = mcList.Count();

		for (int i =0; i < objects; i++)
			{
			BoneModData *bmd = (BoneModData*)mcList[0]->localData;
			for (int j =0; j < bmd->VertexData.Count(); j++)
				{
				if (bmd->selected[j])
					{
					bmd->VertexData[j]->Rigid(rigid);
					bmd->validVerts.Set(j,FALSE);
					
					}
				}	
			}
		}

	}


void BonesDefMod::RigidHandleSelected(BOOL rigid)
	{
	ModContextList mcList;		
	INodeTab nodes;


	if (ip)
		{
		ip->GetModContexts(mcList,nodes);
		int objects = mcList.Count();

		for (int i =0; i < objects; i++)
			{
			BoneModData *bmd = (BoneModData*)mcList[0]->localData;
			for (int j =0; j < bmd->VertexData.Count(); j++)
				{
				if (bmd->selected[j])
					{
					bmd->VertexData[j]->RigidHandle(rigid);
					bmd->validVerts.Set(j,FALSE);
					
					}
				}	
			}
		}

	}

void BonesDefMod::BakeSelectedVertices()
	{
	ModContextList mcList;		
	INodeTab nodes;


	if (ip)
		{
		if (!theHold.Holding())
			theHold.Begin();

		ip->GetModContexts(mcList,nodes);
		int objects = mcList.Count();

		for (int i =0; i < objects; i++)
			{
			BoneModData *bmd = (BoneModData*)mcList[0]->localData;

			theHold.Put(new WeightRestore(this,bmd));

			for (int j =0; j < bmd->VertexData.Count(); j++)
				{
				if ((bmd->selected[j]) && (!bmd->VertexData[j]->IsModified()))
					{
					bmd->VertexData[j]->Modified(TRUE);
					bmd->validVerts.Set(j,FALSE);				
					}
				}	
			}
		theHold.Accept(GetString(IDS_PW_WEIGHTCHANGE));
		}

	}

void BonesDefMod::InvalidateWeightCache()
	{
	ModContextList mcList;		
	INodeTab nodes;


	if (ip)
		{

		ip->GetModContexts(mcList,nodes);
		int objects = mcList.Count();

		for (int i =0; i < objects; i++)
			{
			BoneModData *bmd = (BoneModData*)mcList[0]->localData;
			bmd->nukeValidCache = TRUE;
			}
		}


	}


void BonesDefMod::GetClosestPoints(Point3 p, Tab<Point3> &pointList,float threshold,int count,Tab<int> &hitList, Tab<float> &distList)

	{
	hitList.ZeroCount();
	distList.ZeroCount();

//copy any that are exactly on and mark those

	for (int j =0; j < pointList.Count(); j++)
		{
		Point3 oldPos = pointList[j];
		float dist = Length(oldPos-p);
		if (dist < 0.00001f)
			{
			hitList.Append(1,&j);
			float v = 1.0f;
			distList.Append(1,&v);
			return;
			}
		}

	//now look for any that are within a threshold
	for (int i =0; i < pointList.Count(); i++)
		{
		Point3 pos = pointList[i];
		float dist = Length(p-pos);
		if (distList.Count() < count)
			{
			if (dist < threshold)
				{
				hitList.Append(1,&i);
				distList.Append(1,&dist);
				}
			}
		else
			{	
			int id = i;			
			for (int k =0; k < count; k++)
				{
				if (dist < distList[k])
					{
					int tempid = hitList[k];
					float tempdist = distList[k];
					distList[k] = dist;
					hitList[k] = id;
					id = tempid;
					dist = tempdist;
					}
				}			
			}
		}
	}

void BonesDefMod::RemapVertData(BoneModData *bmd, float threshold, int loadID, Object *obj)
	{
//if obj then use that point data
	if (obj)
		{
		Tab<VertexListClass*> tempData;
		Tab<Point3> posList;
		posList.SetCount(bmd->VertexData.Count());

	//copy the old vertex data
		tempData.SetCount(bmd->VertexData.Count());
	 	for (int i =0; i < bmd->VertexData.Count(); i++)
			{
			tempData[i]  = new(VertexListClass);
			*tempData[i] = *bmd->VertexData[i];
			posList[i] = bmd->VertexData[i]->LocalPos;
			delete bmd->VertexData[i];
			}


		bmd->VertexData.SetCount(obj->NumPoints());
	//reset the bmd vertex data to empty
		for ( i =0; i < bmd->VertexData.Count(); i++)
			{
			bmd->VertexData[i] = new VertexListClass();
			}
		BitArray processedVerts;
		processedVerts.SetSize(bmd->VertexData.Count());
		processedVerts.ClearAll();


	//copy any that are exactly on and mark those
		Tab<int> hitList;
		Tab<float> distList;

		for ( i =0; i < bmd->VertexData.Count(); i++)
			{

			Point3 pos = obj->GetPoint(i);
			GetClosestPoints(pos, posList,threshold,4,hitList, distList);

			if (hitList.Count() == 1)
				{
				int index = hitList[0];
				*bmd->VertexData[i] = *tempData[index];
				bmd->VertexData[i]->LocalPos = pos;
				processedVerts.Set(i);
				}
			else
				{
				int modCount = 0;
				for (int j = 0; j < hitList.Count(); j++)
					{
					int index = hitList[j];
					if (tempData[index]->IsModified()) modCount++;
					}
				if ( (hitList.Count() > 0) && (modCount>=2))
					{
					bmd->VertexData[i]->Modified(TRUE);
					float sum = 0.0f;
					for (j =0; j < hitList.Count(); j++)
						{
						if (tempData[hitList[j]]->IsModified())
							sum += distList[j];
						}
					for (j =0; j < hitList.Count(); j++)
						{
						int hitID = hitList[j];
						if (tempData[hitID]->IsModified())
							{
							int tweightCount = tempData[hitID]->d.Count();
							int oweightCount = bmd->VertexData[i]->d.Count();

	//if multiples then average
							for (int k =0; k < tweightCount; k++)
								{
								int id = tempData[hitID]->d[k].Bones;
								float weight = tempData[hitID]->d[k].normalizedInfluences;
								float per = 1.0f - (distList[j]/sum);
								weight *= per;
								BOOL found = FALSE;
								for (int m =0; m < oweightCount; m++)
									{
									if (bmd->VertexData[i]->d[m].Bones == id)
										{
										found = TRUE;
										bmd->VertexData[i]->d[m].Influences += weight;
										}

									}
								if (!found)
									{
									VertexInfluenceListClass temp;
									temp.Bones = id;
									temp.Influences = weight;
									bmd->VertexData[i]->d.Append(1,&temp);
									}
								}
							}

						}
					bmd->VertexData[i]->NormalizeWeights();

					}

				}
			}
		}
	else if ((loadID >=0) && (loadID < loadBaseNodeData.Count()) && (loadBaseNodeData[loadID].matchData))
//else use the load data
		{
		LoadVertexDataClass *matchData = loadBaseNodeData[loadID].matchData;

		BitArray processedVerts;
		processedVerts.SetSize(bmd->VertexData.Count());
		processedVerts.ClearAll();

//build a remap list instead

		Tab<Point3> posList;
		posList.SetCount(matchData->vertexData.Count());
		for (int i =0; i < matchData->vertexData.Count(); i++)
			{
			Point3 p = matchData->vertexData[i]->LocalPos;
			
			posList[i] = p;
			}


	//copy any that are exactly on and mark those
		Tab<int> hitList;
		Tab<float> distList;

	//copy any that are exactly on and mark those
		for (i =0; i < bmd->VertexData.Count(); i++)
			{
			Point3 pos = bmd->VertexData[i]->LocalPos;
//5.1.02
			if (this->loadByIndex)
				{
				if (i < posList.Count())
					{
					hitList.SetCount(1);
					hitList[0] = i;
					}
				else hitList.ZeroCount();
				}	
			else GetClosestPoints(pos, posList,threshold,4,hitList, distList);

			if (hitList.Count() == 1)
				{
				int index = hitList[0];
				*bmd->VertexData[i] = *matchData->vertexData[index];
				bmd->VertexData[i]->LocalPos = pos;
				processedVerts.Set(i);				
				}
			else
				{
				int modCount = 0;
				for (int j = 0; j < hitList.Count(); j++)
					{
					int index = hitList[j];
					if (matchData->vertexData[index]->IsModified()) modCount++;
					}
				if ( (hitList.Count() > 0) && (modCount>=2))
					{
					bmd->VertexData[i]->Modified(TRUE);
					bmd->VertexData[i]->d.ZeroCount();
					float sum = 0.0f;
					for (j =0; j < hitList.Count(); j++)
						{
						if (matchData->vertexData[hitList[j]]->IsModified())
							sum += distList[j];
						}
					for (j =0; j < hitList.Count(); j++)
						{
						
						int hitID = hitList[j];
						if (matchData->vertexData[hitID]->IsModified())
							{
							int tweightCount = matchData->vertexData[hitID]->d.Count();
							int oweightCount = bmd->VertexData[i]->d.Count();

	//if multiples then average
							for (int k =0; k < tweightCount; k++)
								{
								int id = matchData->vertexData[hitID]->d[k].Bones;
								float weight = matchData->vertexData[hitID]->d[k].normalizedInfluences;

								float per = 1.0f - (distList[j]/sum);
								weight *= per;
								BOOL found = FALSE;
								for (int m =0; m < oweightCount; m++)
									{
									if (bmd->VertexData[i]->d[m].Bones == id)
										{
										found = TRUE;
										bmd->VertexData[i]->d[m].Influences += weight;
										}

									}
								if (!found)
									{
									VertexInfluenceListClass temp;
									temp.Bones = id;
									temp.Influences = weight;
									bmd->VertexData[i]->d.Append(1,&temp);
									}
								}

							}

						}
					bmd->VertexData[i]->NormalizeWeights();

					}
				}
			}
		}


	}



void BonesDefMod::RemapExclusionData(BoneModData *bmd, float threshold, int loadID, Object *obj)
	{
	Tab<int> hitList;
	Tab<float> distList;

	//if obj then use that point data
	if (obj)
		{

		Tab<Point3> posList;
		posList.SetCount(bmd->VertexData.Count());

	//copy the old vertex data
	 	for (int i =0; i < bmd->VertexData.Count(); i++)
			{
			posList[i] = bmd->VertexData[i]->LocalPos;
			}


//need to clone exclusions
		Tab<ExclusionListClass*> holdExclusionList;
		holdExclusionList.SetCount(bmd->exclusionList.Count());
		for (i = 0; i < bmd->exclusionList.Count(); i++)
			{
			if (bmd->exclusionList[i])
				{
				holdExclusionList[i] = new ExclusionListClass();
				*holdExclusionList[i] = *bmd->exclusionList[i];
				}
			else holdExclusionList[i]=NULL;
			delete bmd->exclusionList[i];
			bmd->exclusionList[i] = NULL;
			}

		for (int j=0; j< obj->NumPoints(); j++)
			{
			Point3 pos = obj->GetPoint(j);
						//copy any that are exactly on and mark those
			GetClosestPoints(pos, posList,threshold,4,hitList, distList);
			
			for (i = 0; i < holdExclusionList.Count(); i++)
				{
				if (holdExclusionList[i])
					{

					
					if (hitList.Count() == 1)
						{
						int index = hitList[0];
						Tab<int> tempList;
						tempList.Append(1,&j);
						int where;
						if (holdExclusionList[i]->isInList(index,where))
							{
							bmd->ExcludeVerts(i, tempList,FALSE);
							}
						}
					else
						{
						int exCount=0;
						for (int k = 0; k < hitList.Count();k++)
							{
							int index = hitList[k];
							int where;
							if (holdExclusionList[i]->isInList(index,where))
								exCount++;

							}
						if (exCount >=2)
							{	
							Tab<int> tempList;
							tempList.Append(1,&j);
							bmd->ExcludeVerts(i, tempList,FALSE);

							}

						}
					}
				}
			}

		for (i = 0; i < holdExclusionList.Count(); i++)
			{
			if (holdExclusionList[i])
				delete holdExclusionList[i];
			}
			
		}

	else if ((loadID >=0) && (loadID < loadBaseNodeData.Count()) && (loadBaseNodeData[loadID].matchData))
		{
//else use the load data
		Tab<Point3> posList;
		posList.SetCount(loadBaseNodeData[loadID].matchData->vertexData.Count());

	//copy the old vertex data
	 	for (int i =0; i < loadBaseNodeData[loadID].matchData->vertexData.Count(); i++)
			{
			posList[i] = loadBaseNodeData[loadID].matchData->vertexData[i]->LocalPos;
			}


//need to clone exclusions
		Tab<ExclusionListClass*> holdExclusionList;
		holdExclusionList.SetCount(loadBaseNodeData[loadID].matchData->exclusionList.Count());
		for (i = 0; i < loadBaseNodeData[loadID].matchData->exclusionList.Count(); i++)
			{
			if (loadBaseNodeData[loadID].matchData->exclusionList[i])
				{
				holdExclusionList[i] = new ExclusionListClass();
				*holdExclusionList[i] = *loadBaseNodeData[loadID].matchData->exclusionList[i];
				}
			else holdExclusionList[i]=NULL;
			delete loadBaseNodeData[loadID].matchData->exclusionList[i]; 
//			delete bmd->exclusionList[i]; 
//			bmd->exclusionList[i] = NULL;
			}
//nuke old exclusion list
		for (i = 0; i < bmd->exclusionList.Count(); i++)
			{
			if (bmd->exclusionList[i])
				{
				delete bmd->exclusionList[i]; 
				bmd->exclusionList[i] = NULL;
				}
			}


		for (int j=0; j< bmd->VertexData.Count(); j++)
			{
			Point3 pos = bmd->VertexData[j]->LocalPos;
						//copy any that are exactly on and mark those
//5.1.02
			if (this->loadByIndex)
				{
				if (i < posList.Count())
					{
					hitList.SetCount(1);
					hitList[0] = j;
					}
				else hitList.ZeroCount();
				}	
			else GetClosestPoints(pos, posList,threshold,4,hitList, distList);
			
			
			for (i = 0; i < holdExclusionList.Count(); i++)
				{
				if (holdExclusionList[i])
					{

					
					if (hitList.Count() == 1)
						{
						int index = hitList[0];
						Tab<int> tempList;
						tempList.Append(1,&j);
						int where;
						if (holdExclusionList[i]->isInList(index,where))
							{
							bmd->ExcludeVerts(i, tempList,FALSE);
							}
						}
					else
						{
						int exCount=0;
						for (int k = 0; k < hitList.Count();k++)
							{
							int index = hitList[k];
							int where;
							if (holdExclusionList[i]->isInList(index,where))
								exCount++;

							}
						if (exCount >=2)
							{	
							Tab<int> tempList;
							tempList.Append(1,&j);
							bmd->ExcludeVerts(i, tempList,FALSE);

							}

						}
					}
				}
			}

		for (i = 0; i < holdExclusionList.Count(); i++)
			{
			if (holdExclusionList[i])
				delete holdExclusionList[i];
			}
		}


	}


void BonesDefMod::RemapLocalGimzoData(BoneModData *bmd,  float threshold, int loadID ,Object *obj)
	{
	Tab<int> hitList;
	Tab<float> distList;

	//if obj then use that point data
	if (obj)
		{

		Tab<Point3> posList;
		posList.SetCount(obj->NumPoints());

	//copy the old vertex data
	 	for (int i =0; i < obj->NumPoints(); i++)
			{
			posList[i] = obj->GetPoint(i);
			}



		for (int j=0; j< bmd->gizmoData.Count(); j++)
			{
			if (bmd->gizmoData[j])
				{
				for (i = 0; i < bmd->gizmoData[j]->Count(); i++)
					{
					int index = bmd->gizmoData[j]->GetVert(i);
					if ((index >=0) && (index < bmd->VertexData.Count()))
						{
						Point3 pos = bmd->VertexData[index]->LocalPos;
						//copy any that are exactly on and mark those
						GetClosestPoints(pos, posList,threshold,1,hitList, distList);
						if (hitList.Count() == 1)
							{
							bmd->gizmoData[j]->Replace(i,hitList[0]);
							}
						else bmd->gizmoData[j]->Replace(i,-1);
						
						}
	
					}
				}
			}
		}

	else if ((loadID >=0) && (loadID < loadBaseNodeData.Count()) && (loadBaseNodeData[loadID].matchData))
		{
		}

	}