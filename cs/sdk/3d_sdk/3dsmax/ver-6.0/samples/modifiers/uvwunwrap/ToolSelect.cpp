#include "unwrap.h"


void UnwrapMod::RebuildDistCache()
{
	
if (iStr==NULL) return;

float str = iStr->GetFVal();
falloffStr = str;
float sstr = str*str;
if ((str == 0.0f) || (enableSoftSelection == FALSE))
	{
	for (int i = 0; i<TVMaps.v.Count(); i++)
		{
		if (!(TVMaps.v[i].flags & FLAG_WEIGHTMODIFIED))
			TVMaps.v[i].influence = 0.0f;
		}
	return;
	}
Tab<int> Selected;

for (int i = 0; i<TVMaps.v.Count(); i++)
	{
	if (vsel[i])
		Selected.Append(1,&i,1);
	}
if (falloffSpace == 0)
	BuildObjectPoints();

BitArray useableVerts;
useableVerts.SetSize(TVMaps.v.Count());



if (limitSoftSel)
	{
	int oldMode = fnGetTVSubMode();
	fnSetTVSubMode(TVVERTMODE);
	useableVerts.ClearAll();
	BitArray holdSel;
	holdSel.SetSize(vsel.GetSize());
	holdSel = vsel;
	for (i=0; i < limitSoftSelRange; i++)
		{
		ExpandSelection(0,FALSE,FALSE);
		}
	useableVerts = vsel;
	vsel = holdSel;
	fnSetTVSubMode(oldMode);
	}
else useableVerts.SetAll();

for (i = 0; i<TVMaps.v.Count(); i++)
	{
	if (!(TVMaps.v[i].flags & FLAG_WEIGHTMODIFIED))
		TVMaps.v[i].influence = 0.0f;
	}


for (i = 0; i<TVMaps.v.Count(); i++)
	{
	if ( (vsel[i] == 0) && (useableVerts[i]) && (!(TVMaps.v[i].flags & FLAG_WEIGHTMODIFIED)))
		{
		float closest_dist = BIGFLOAT;
		for (int j= 0; j < Selected.Count();j++)
			{
//use XY	Z space values
			if (falloffSpace == 0)
				{
				Point3 sp = GetObjectPoint(ip->GetTime(),i);
				Point3 rp = GetObjectPoint(ip->GetTime(),Selected[j]);
				float d = LengthSquared(sp-rp);
				if (d < closest_dist) closest_dist = d;

				}
			else
//use UVW space values
				{
				Point3 sp = GetPoint(ip->GetTime(),Selected[j]);
				Point3 rp = GetPoint(ip->GetTime(),i);
				float d = LengthSquared(sp-rp);
				if (d < closest_dist) closest_dist = d;
				}
			}
		if (closest_dist < sstr)
			{
			closest_dist = (float) sqrt(closest_dist);
			TVMaps.v[i].influence = 1.0f - closest_dist/str;
			ComputeFalloff(TVMaps.v[i].influence,falloff);
			}
		else TVMaps.v[i].influence = 0.0f;
		}
	}	

}


void UnwrapMod::ExpandSelection(int dir, BOOL rebuildCache, BOOL hold)

{
if (!theHold.Holding() && hold)
	theHold.Begin();

if (hold)
	HoldSelection();

//convert our sub selection type to vertex selection
TransferSelectionStart();

BitArray faceHasSelectedVert;

faceHasSelectedVert.SetSize(TVMaps.f.Count());
faceHasSelectedVert.ClearAll();

for (int i = 0; i < TVMaps.f.Count();i++)
	{
	if (!(TVMaps.f[i]->flags & FLAG_DEAD))
		{
		int pcount = 3;
		pcount = TVMaps.f[i]->count;
		int totalSelected = 0;
		for (int k = 0; k < pcount; k++)
			{
			int index = TVMaps.f[i]->t[k];
			if (vsel[index])
				{
				totalSelected++;
				}
			}

		if ( (totalSelected != pcount) && (totalSelected!= 0))
			{
			faceHasSelectedVert.Set(i);
			}

		}
	}
for (i = 0; i < TVMaps.f.Count();i++)
	{
	if (faceHasSelectedVert[i])
		{
		int pcount = 3;
		pcount = TVMaps.f[i]->count;
		int totalSelected = 0;
		for (int k = 0; k < pcount; k++)
			{
			int index = TVMaps.f[i]->t[k];
			if (dir == 0)
				vsel.Set(index,1);
			else vsel.Set(index,0);
			}

		}
	}




SelectHandles(dir);
if (rebuildCache)
	RebuildDistCache();
//convert our sub selection back

TransferSelectionEnd(FALSE,TRUE);


if (hold)
	theHold.Accept(GetString(IDS_DS_SELECT));					
}

void	UnwrapMod::fnSelectElement()
	{
	if (!theHold.Holding())
		theHold.Begin();
	HoldSelection();
	SelectElement();
	if (fnGetSyncSelectionMode()) fnSyncGeomSelection();
	RebuildDistCache();
	theHold.Accept(GetString(IDS_DS_SELECT));					
	}

void UnwrapMod::SelectElement()

{

TransferSelectionStart();

BitArray faceHasSelectedVert;

faceHasSelectedVert.SetSize(TVMaps.f.Count());
faceHasSelectedVert.ClearAll();

int count = -1;

while (count != vsel.NumberSet())
	{
	count = vsel.NumberSet();
	for (int i = 0; i < TVMaps.f.Count();i++)
		{
		if (!(TVMaps.f[i]->flags & FLAG_DEAD))
			{
			int pcount = 3;
			pcount = TVMaps.f[i]->count;
			int totalSelected = 0;
			for (int k = 0; k < pcount; k++)
				{
				int index = TVMaps.f[i]->t[k];
				if (vsel[index])
					{
					totalSelected++;
					}
				}

			if ( (totalSelected != pcount) && (totalSelected!= 0))
				{
				faceHasSelectedVert.Set(i);
				}

			}
		}
	for (i = 0; i < TVMaps.f.Count();i++)
		{
		if (faceHasSelectedVert[i])
			{
			int pcount = 3;
			pcount = TVMaps.f[i]->count;
			int totalSelected = 0;
			for (int k = 0; k < pcount; k++)
				{
				int index = TVMaps.f[i]->t[k];
				vsel.Set(index,1);
				}
			}

		}
	}

SelectHandles(0);

TransferSelectionEnd(FALSE,TRUE);

}


void UnwrapMod::SelectHandles(int dir)

{
//if face is selected select me
for (int j = 0; j < TVMaps.f.Count();j++)
	{
	if ( (!(TVMaps.f[j]->flags & FLAG_DEAD)) &&
	     (TVMaps.f[j]->flags & FLAG_CURVEDMAPPING) &&
		 (TVMaps.f[j]->vecs) 
	   )

		{
		int pcount = 3;
		pcount = TVMaps.f[j]->count;
		for (int k = 0; k < pcount; k++)
			{
			int id = TVMaps.f[j]->t[k];
			if (dir ==0)
				{
				if ((vsel[id]) && (!wasSelected[id]))
					{
					int vid1 = TVMaps.f[j]->vecs->handles[k*2];
					int vid2 = 0;
					if (k ==0)
						vid2 = TVMaps.f[j]->vecs->handles[pcount*2-1];
					else vid2 = TVMaps.f[j]->vecs->handles[k*2-1];
	
					if ( (IsVertVisible(vid1)) &&  (!(TVMaps.v[vid1].flags & FLAG_FROZEN)) )
						vsel.Set(vid1,1);
					if ( (IsVertVisible(vid2)) &&  (!(TVMaps.v[vid2].flags & FLAG_FROZEN)) )
						vsel.Set(vid2,1);

					if (TVMaps.f[j]->flags & FLAG_INTERIOR)
						{
						int ivid1 = TVMaps.f[j]->vecs->interiors[k];
						if ( (ivid1 >= 0) && (IsVertVisible(ivid1)) &&  (!(TVMaps.v[ivid1].flags & FLAG_FROZEN)) )
							vsel.Set(ivid1,1);
						}
					}
				}
			else
				{
				if (!vsel[id])
					{
					int vid1 = TVMaps.f[j]->vecs->handles[k*2];
					int vid2 = 0;
					if (k ==0)
						vid2 = TVMaps.f[j]->vecs->handles[pcount*2-1];
					else vid2 = TVMaps.f[j]->vecs->handles[k*2-1];
	
					if ( (IsVertVisible(vid1)) &&  (!(TVMaps.v[vid1].flags & FLAG_FROZEN)) )
						vsel.Set(vid1,0);
					if ( (IsVertVisible(vid2)) &&  (!(TVMaps.v[vid2].flags & FLAG_FROZEN)) )
						vsel.Set(vid2,0);

					if (TVMaps.f[j]->flags & FLAG_INTERIOR)
						{
						int ivid1 = TVMaps.f[j]->vecs->interiors[k];
						if ( (ivid1 >= 0) && (IsVertVisible(ivid1)) &&  (!(TVMaps.v[ivid1].flags & FLAG_FROZEN)) )
							vsel.Set(ivid1,0);
						}

					}

				}

			}
		}
	}

}





void UnwrapMod::SelectFacesByNormals( MeshTopoData *md,BitArray &sel,Point3 norm, float angle, Tab<Point3> &normList)
	{

	if (normList.Count() == 0)
		BuildNormals(md,normList);
	norm = Normalize(norm);
	angle = angle * PI/180.0f;
	if (md->mesh)
		{
//check for contigous faces
		double newAngle;
		if (sel.GetSize() != md->mesh->numFaces)
			sel.SetSize(md->mesh->numFaces);
		sel.ClearAll();
		for (int i =0; i < md->mesh->numFaces; i++)
			{
			Point3 debugNorm = Normalize(normList[i]);
			float dot = DotProd(debugNorm,norm);
			newAngle = (acos(dot));




			if ((dot == 1.0f) || (newAngle <= angle))
				sel.Set(i);
			else
				{
				sel.Set(i,0);
				}
			}
		}
	else if (md->mnMesh)
		{
//check for contigous faces
		double newAngle;
		if (sel.GetSize() != md->mnMesh->numf)
			sel.SetSize(md->mnMesh->numf);
		for (int i =0; i < md->mnMesh->numf; i++)
			{
			Point3 debugNorm = normList[i];
			float dot = DotProd(normList[i],norm);
			newAngle = (acos(dot));			
			if ((dot == 1.0f) || (newAngle <= angle))
				sel.Set(i);
			else
				{
				sel.Set(i,0);
				}
			}
		}
	else if (md->patch)
		{
//check for contigous faces
		double newAngle;
		if (sel.GetSize() != md->patch->numPatches)
			sel.SetSize(md->patch->numPatches);
		for (int i =0; i < md->patch->numPatches; i++)
			{
			Point3 debugNorm = normList[i];
			float dot = DotProd(normList[i],norm);
			newAngle = (acos(dot));
			if ((dot == 1.0f) || (newAngle <= angle))
				sel.Set(i);
			else
				{
				sel.Set(i,0);
				}
			}
		}


	}


void UnwrapMod::SelectFacesByGroup( MeshTopoData *md,BitArray &sel,int seedFaceIndex, Point3 norm, float angle, BOOL relative,Tab<Point3> &normList,
								   Tab<BorderClass> &borderData,
								   AdjEdgeList *medges)
	{
	//check for type

	if (normList.Count() == 0)
		BuildNormals(md,normList);

	int ct = 0;

	angle = angle * PI/180.0f;

	sel.ClearAll();

	

	if (md->mesh)
		{
//get seed face
		if (seedFaceIndex < md->mesh->numFaces)
			{
			
			if (sel.GetSize() != md->mesh->numFaces)
				sel.SetSize(md->mesh->numFaces);
//select it
			sel.Set(seedFaceIndex);
			
//build egde list of all edges that have only one edge selected
			AdjEdgeList *edges;
			BOOL deleteEdges = FALSE;
			if (medges == NULL)
				{
				edges = new AdjEdgeList(*md->mesh);
				deleteEdges = TRUE;
				}
			else edges = medges;

			borderData.ZeroCount();

			
			int numberWorkableEdges = 1;
			while (numberWorkableEdges > 0)
				{
				numberWorkableEdges = 0;
				for (int i = 0; i < edges->edges.Count(); i++)
					{
//					if (!blockedEdges[i])
						{
						int a = edges->edges[i].f[0];
						int b = edges->edges[i].f[1];
						if ( (a>=0) && (b>=0) )
							{
							if (sel[a] && (!sel[b]))
								{
								float newAngle;
								if (!relative)
									newAngle = (acos(DotProd(normList[b],norm)));
								else newAngle = (acos(DotProd(normList[b],normList[a])));
								if (newAngle <= angle)
									{
									sel.Set(b);
									numberWorkableEdges++;
									
									}
								else
									{
									BorderClass tempData;
									tempData.edge = i;
									tempData.innerFace = a;
									tempData.outerFace = b;
									borderData.Append(1,&tempData);
									}
								}
							else if (sel[b] && (!sel[a]))
								{
								float newAngle;
								if (!relative)
									newAngle = (acos(DotProd(normList[a],norm)));
								else newAngle = (acos(DotProd(normList[a],normList[b])));
								if (newAngle <= angle)
									{
									sel.Set(a);
									numberWorkableEdges++;
									
									}
								else
									{
									BorderClass tempData;
									tempData.edge = i;
									tempData.innerFace = b;
									tempData.outerFace = a;
									borderData.Append(1,&tempData);

									}

								}
							}
						}
					}
				}
			if (deleteEdges) delete edges;
			}

		}
	else if (md->patch)
		{
		if (seedFaceIndex < md->patch->numPatches)
			{
//select it
			if (sel.GetSize() != md->patch->numPatches)
				sel.SetSize(md->patch->numPatches);

			sel.Set(seedFaceIndex);

			borderData.ZeroCount();

//build egde list of all edges that have only one edge selected
			PatchEdge *edges = md->patch->edges;
				
			int numberWorkableEdges = 1;
			while (numberWorkableEdges > 0)
				{
				numberWorkableEdges = 0;
				for (int i = 0; i < md->patch->numEdges; i++)
					{
					if (edges[i].patches.Count() ==2 )
						{
						int a = edges[i].patches[0];
						int b = edges[i].patches[1];
						if ( (a>=0) && (b>=0) )
							{
							if (sel[a] && (!sel[b]))
								{
								float newAngle;
								if (!relative)
									newAngle = (acos(DotProd(normList[b],norm)));
								else newAngle = (acos(DotProd(normList[b],normList[a])));
								if (newAngle <= angle)
									{
									sel.Set(b);
									numberWorkableEdges++;
									
									}
								else
									{
									BorderClass tempData;
									tempData.edge = i;
									tempData.innerFace = a;
									tempData.outerFace = b;
									borderData.Append(1,&tempData);

									}
								}
							else if (sel[b] && (!sel[a]))
								{
								float newAngle;
								if (!relative)
									newAngle = (acos(DotProd(normList[a],norm)));
								else newAngle = (acos(DotProd(normList[a],normList[b])));
								if (newAngle <= angle)
									{
									sel.Set(a);
									numberWorkableEdges++;
									
									}
								else
									{
									BorderClass tempData;
									tempData.edge = i;
									tempData.innerFace = b;
									tempData.outerFace = a;
									borderData.Append(1,&tempData);

									}
								}

							}
						}
					}
				}
			}

		}
	else if (md->mnMesh)
		{
//select it
		if (seedFaceIndex < md->mnMesh->numf)
			{
			if (sel.GetSize() != md->mnMesh->numf)
				sel.SetSize(md->mnMesh->numf);
			sel.Set(seedFaceIndex);
			
			borderData.ZeroCount();

//build egde list of all edges that have only one edge selected
			MNEdge *edges = md->mnMesh->E(0);
			int numberWorkableEdges = 1;
			while (numberWorkableEdges > 0)
				{
				numberWorkableEdges = 0;
				for (int i = 0; i < md->mnMesh->nume; i++)
					{
					int a = edges[i].f1;
					int b = edges[i].f2;
					if ( (a>=0) && (b>=0) )
						{
						if (sel[a] && (!sel[b]))
							{
							float newAngle;
							if (!relative)
								newAngle = (acos(DotProd(normList[b],norm)));
							else newAngle = (acos(DotProd(normList[b],normList[a])));
							if (newAngle <= angle)
								{
								sel.Set(b);
								numberWorkableEdges++;
								
								}
							else
								{
								BorderClass tempData;
								tempData.edge = i;
								tempData.innerFace = a;
								tempData.outerFace = b;
								borderData.Append(1,&tempData);

								}
							}
						else if (sel[b] && (!sel[a]))
							{
							float newAngle;
							if (!relative)
								newAngle = (acos(DotProd(normList[a],norm)));
							else newAngle = (acos(DotProd(normList[a],normList[b])));
							if (newAngle <= angle)
								{
								sel.Set(a);
								numberWorkableEdges++;
								
								}
							else
								{

								BorderClass tempData;
								tempData.edge = i;
								tempData.innerFace = b;
								tempData.outerFace = a;
								borderData.Append(1,&tempData);

								}
							}

						}
					}
				}
			}
		}


		

	}


void UnwrapMod::fnSelectPolygonsUpdate(BitArray *sel, BOOL update)
	{
	ModContextList mcList;		
	INodeTab nodes;

	MeshTopoData *md =NULL;

	if (ip) 
		{
		ip->GetModContexts(mcList,nodes);

		int objects = mcList.Count();
		md = (MeshTopoData*)mcList[0]->localData;
		}
	else md = GetModData();

	int ct = 0;
	if (md)
		{
//		MeshTopoData *md = (MeshTopoData*)mcList[0]->localData;

		if (md == NULL) 
			{
			return;
			}


		md->faceSel.ClearAll();
		for (int i =0; i < md->faceSel.GetSize(); i++)
			{
			if (i < sel->GetSize())
				{
				if ((*sel)[i]) md->faceSel.Set(i);
				}
			}
		UpdateFaceSelection(md->faceSel);
		if (update)
			{
			NotifyDependents(FOREVER,PART_SELECT,REFMSG_CHANGE);
			InvalidateView();
			}
		else
			{
			if (md->GetMesh())
				MeshUpdateGData(md->GetMesh(), md->faceSel);
			if (md->GetPatch())
				PatchUpdateGData(md->GetPatch(), md->faceSel);
			if (md->GetMNMesh())
				PolyUpdateGData(md->GetMNMesh(), md->faceSel);
			}
		}

	}

void	UnwrapMod::fnSelectFacesByNormal(Point3 normal, float angleThreshold, BOOL update)
	{
	//check for type
	ModContextList mcList;		
	INodeTab nodes;

	if (!ip) return;
	ip->GetModContexts(mcList,nodes);

	int objects = mcList.Count();


	if (objects != 0)
		{
		MeshTopoData *md = (MeshTopoData*)mcList[0]->localData;

		if (md == NULL) 
			{
			return;
			}


		BitArray sel;
		Tab<Point3> normList;
		normList.ZeroCount();
		SelectFacesByNormals( md,sel, normal, angleThreshold,normList);
		fnSelectPolygonsUpdate(&sel,update);
		}
	}

void	UnwrapMod::fnSelectClusterByNormal( float angleThreshold, int seedIndex, BOOL relative, BOOL update)
	{
	//check for type
	ModContextList mcList;		
	INodeTab nodes;

	if (!ip) return;
	ip->GetModContexts(mcList,nodes);

	int objects = mcList.Count();
	seedIndex--;


	if (objects != 0)
		{
		MeshTopoData *md = (MeshTopoData*)mcList[0]->localData;

		if (md == NULL) 
			{
			return;
			}

		BitArray sel;

		Tab<BorderClass> dummy;
		Tab<Point3> normList;
		normList.ZeroCount();
		BuildNormals(md, normList);
		if ((seedIndex >= 0) && (seedIndex <normList.Count()))
			{
			Point3 normal = normList[seedIndex];
			SelectFacesByGroup( md,sel,seedIndex, normal, angleThreshold, relative,normList,dummy);
			fnSelectPolygonsUpdate(&sel,update);
			}
		}
	}



BOOL	UnwrapMod::fnGetLimitSoftSel()
	{
	return limitSoftSel;
	}

void	UnwrapMod::fnSetLimitSoftSel(BOOL limit)
	{
	limitSoftSel = limit;
	RebuildDistCache();
	NotifyDependents(FOREVER,PART_SELECT,REFMSG_CHANGE);
	InvalidateView();

	}

int		UnwrapMod::fnGetLimitSoftSelRange()
	{
	return limitSoftSelRange;
	}

void	UnwrapMod::fnSetLimitSoftSelRange(int range)
	{
	limitSoftSelRange = range;
	RebuildDistCache();
	NotifyDependents(FOREVER,PART_SELECT,REFMSG_CHANGE);
	InvalidateView();
	}


float	UnwrapMod::fnGetVertexWeight(int index)
	{
	float v = 0.0f;
	index--;

	if ((index >=0) && (index <TVMaps.v.Count()))
		v = TVMaps.v[index].influence;
	return v;
	}
void	UnwrapMod::fnSetVertexWeight(int index,float weight)
	{
	index--;
	if ((index >=0) && (index <TVMaps.v.Count()))
		{
		TVMaps.v[index].influence = weight;
		TVMaps.v[index].flags  |= FLAG_WEIGHTMODIFIED;
		NotifyDependents(FOREVER,PART_SELECT,REFMSG_CHANGE);
		InvalidateView();
		}

	}

BOOL	UnwrapMod::fnIsWeightModified(int index)
	{
	index--;
	BOOL mod = FALSE;
	if ((index >=0) && (index <TVMaps.v.Count()))
		{

		mod = (TVMaps.v[index].flags & FLAG_WEIGHTMODIFIED);
		}
	
	return mod;

	}
void	UnwrapMod::fnModifyWeight(int index, BOOL modified)
	{
	index--;
	if ((index >=0) && (index <TVMaps.v.Count()))
		{
		if (modified)
			TVMaps.v[index].flags |= FLAG_WEIGHTMODIFIED;
		else TVMaps.v[index].flags &= ~FLAG_WEIGHTMODIFIED;
		NotifyDependents(FOREVER,PART_SELECT,REFMSG_CHANGE);
		InvalidateView();
		}


	}

BOOL	UnwrapMod::fnGetGeomElemMode()
	{
	return geomElemMode;
	}

void	UnwrapMod::fnSetGeomElemMode(BOOL elem)
	{
	geomElemMode = elem;
	CheckDlgButton(hSelRollup,IDC_SELECTELEMENT_CHECK,geomElemMode);
	}

void	UnwrapMod::SelectGeomElement(MeshTopoData* tmd)
	{

	Tab<BorderClass> dummy;
	Tab<Point3> normList;
	normList.ZeroCount();
	BuildNormals(tmd, normList);
	BitArray tempSel;
	tempSel.SetSize(tmd->faceSel.GetSize());

	tempSel = tmd->faceSel;

	for (int i =0; i < tmd->faceSel.GetSize(); i++)
		{
		if ((tempSel[i]) && (i >= 0) && (i <normList.Count()))
			{
			BitArray sel;
			Point3 normal = normList[i];
			SelectFacesByGroup( tmd,sel,i, normal, 180.0f, FALSE,normList,dummy);
			tmd->faceSel |= sel;
			for (int j = 0; j < tempSel.GetSize(); j++)
				{
				if (sel[j]) tempSel.Set(j,FALSE);
				}
			}
		}

	}


void	UnwrapMod::SelectGeomFacesByAngle(MeshTopoData* tmd)
	{


	Tab<BorderClass> dummy;
	Tab<Point3> normList;
	normList.ZeroCount();
	BuildNormals(tmd, normList);
	BitArray tempSel;
	tempSel.SetSize(tmd->faceSel.GetSize());

	tempSel = tmd->faceSel;

	for (int i =0; i < tmd->faceSel.GetSize(); i++)
		{
		if ((tempSel[i]) && (i >= 0) && (i <normList.Count()))
			{
			BitArray sel;
			Point3 normal = normList[i];
			SelectFacesByGroup( tmd,sel,i, normal, planarThreshold, TRUE,normList,dummy);
			tmd->faceSel |= sel;
			for (int j = 0; j < tempSel.GetSize(); j++)
				{
				if (sel[j]) tempSel.Set(j,FALSE);
				}
			}
		}
	UpdateFaceSelection(tmd->faceSel);


	}

BOOL	UnwrapMod::fnGetGeomPlanarMode()
	{
	return planarMode ;
	}

void	UnwrapMod::fnSetGeomPlanarMode(BOOL planar)
	{
	planarMode = planar;
//update UI
	CheckDlgButton(hSelRollup,IDC_PLANARANGLE_CHECK,planarMode);
	}

float	UnwrapMod::fnGetGeomPlanarModeThreshold()
	{
	return planarThreshold;
	}

void	UnwrapMod::fnSetGeomPlanarModeThreshold(float threshold)
	{
	planarThreshold = threshold;
//update UI
	if (iPlanarThreshold)
		iPlanarThreshold->SetValue(fnGetGeomPlanarModeThreshold(),TRUE);
	}


void	UnwrapMod::fnSelectByMatID(int matID)
	{
	//check for type
	ModContextList mcList;		
	INodeTab nodes;

	if (!ip) return;
	ip->GetModContexts(mcList,nodes);

	int objects = mcList.Count();

	matID--;
	if (objects != 0)
		{

		MeshTopoData *tmd = (MeshTopoData*)mcList[0]->localData;

		if (tmd == NULL) 
			{
			return;
			}


		if (!theHold.Holding() )
			theHold.Begin();


		if (theHold.Holding()) theHold.Put (new UnwrapSelRestore (this,tmd));

		tmd->faceSel.ClearAll();
		for (int i = 0; i < TVMaps.f.Count(); i++)
			{
			if ( (TVMaps.f[i]->MatID == matID) && (i < tmd->faceSel.GetSize()))
				tmd->faceSel.Set(i);
			}
		UpdateFaceSelection(tmd->faceSel);

		if (fnGetSyncSelectionMode()) fnSyncTVSelection();

		theHold.Accept (GetString (IDS_DS_SELECT));
		ComputeSelectedFaceData();
		NotifyDependents(FOREVER,PART_SELECT,REFMSG_CHANGE);
		InvalidateView();
		ip->RedrawViews(ip->GetTime());

		}



	}

void	UnwrapMod::fnSelectBySG(int sg)
	{

	//check for type
	ModContextList mcList;		
	INodeTab nodes;

	if (!ip) return;
	ip->GetModContexts(mcList,nodes);

	int objects = mcList.Count();

	sg--;

	if (objects != 0)
		{

		MeshTopoData *tmd = (MeshTopoData*)mcList[0]->localData;

		if (tmd == NULL) 
			{
			return;
			}


		if (!theHold.Holding() )
			theHold.Begin();


		if (theHold.Holding()) theHold.Put (new UnwrapSelRestore (this,tmd));

		tmd->faceSel.ClearAll();

		if ((objType == IS_MESH) && (!tmd->mesh))
			{
			updateCache = TRUE;
			NotifyDependents(FOREVER,PART_SELECT,REFMSG_CHANGE);
			}
		else if ((objType == IS_PATCH) && (!tmd->patch))
			{
			updateCache = TRUE;
			NotifyDependents(FOREVER,PART_SELECT,REFMSG_CHANGE);
			}
		else if ((objType == IS_MNMESH) && (!tmd->mnMesh))
			{
			updateCache = TRUE;
			NotifyDependents(FOREVER,PART_SELECT,REFMSG_CHANGE);
			}

		sg = 1 << sg;
		if(tmd->mesh)
			{
			for (int i =0; i < tmd->mesh->numFaces; i++)
				{
				if ( (tmd->mesh->faces[i].getSmGroup() & sg) && (i < tmd->faceSel.GetSize()) )
					tmd->faceSel.Set(i);
				}
			}
		else if (tmd->mnMesh)
			{
			for (int i =0; i < tmd->mnMesh->numf; i++)
				{
				if ( (tmd->mnMesh->f[i].smGroup & sg) && (i < tmd->faceSel.GetSize()) )
					tmd->faceSel.Set(i);
				}
			}
		else if (tmd->patch)
			{
			for (int i =0; i < tmd->patch->numPatches; i++)
				{
				if ( (tmd->patch->patches[i].smGroup & sg) && (i < tmd->faceSel.GetSize()) )
					tmd->faceSel.Set(i);
				}
			}
		


		UpdateFaceSelection(tmd->faceSel);

		if (fnGetSyncSelectionMode()) fnSyncTVSelection();
		
		theHold.Accept (GetString (IDS_DS_SELECT));
		ComputeSelectedFaceData();
		NotifyDependents(FOREVER,PART_SELECT,REFMSG_CHANGE);
		InvalidateView();
		ip->RedrawViews(ip->GetTime());
		}


	}



void	UnwrapMod::GeomExpandFaceSel(MeshTopoData* tmd)
	{

	BitArray selectedVerts;
	
	if ((objType == IS_MESH) && (tmd->mesh))
		{
		selectedVerts.SetSize(tmd->mesh->getNumVerts());
		selectedVerts.ClearAll();
		for (int i =0; i < tmd->mesh->getNumFaces(); i++)
			{
			if (tmd->faceSel[i])
				{
				for (int j = 0;j < 3; j++)
					{
					int index = tmd->mesh->faces[i].v[j];
					selectedVerts.Set(index);
					}
				}
			}
		for (i =0; i < tmd->mesh->getNumFaces(); i++)
			{
			if (!tmd->faceSel[i])
				{
				for (int j = 0;j < 3; j++)
					{
					int index = tmd->mesh->faces[i].v[j];
					if (selectedVerts[index])
						tmd->faceSel.Set(i);

					}
				}
			}

		}
	else if ((objType == IS_PATCH) && (tmd->patch))
		{
		selectedVerts.SetSize(tmd->patch->getNumVerts());
		selectedVerts.ClearAll();
		for (int i =0; i < tmd->patch->getNumPatches(); i++)
			{
			if (tmd->faceSel[i])
				{
				int ct = 4;
				if (tmd->patch->patches[i].type == PATCH_TRI) ct = 3;
				for (int j = 0;j < ct; j++)
					{
					int index = tmd->patch->patches[i].v[j];
					selectedVerts.Set(index);
					}
				}
			}
		for (i =0; i < tmd->patch->getNumPatches(); i++)
			{
			if (!tmd->faceSel[i])
				{
				int ct = 4;
				if (tmd->patch->patches[i].type == PATCH_TRI) ct = 3;
				for (int j = 0;j < ct; j++)
					{
					int index = tmd->patch->patches[i].v[j];
					if (selectedVerts[index])
						tmd->faceSel.Set(i);

					}
				}
			}
		}
	else if ((objType == IS_MNMESH) && (tmd->mnMesh))
		{
		selectedVerts.SetSize(tmd->mnMesh->numv);
		selectedVerts.ClearAll();
		for (int i =0; i < tmd->mnMesh->numf; i++)
			{
			if (tmd->faceSel[i])
				{
				int ct = tmd->mnMesh->f[i].deg;
				for (int j = 0;j < ct; j++)
					{
					int index = tmd->mnMesh->f[i].vtx[j];
					selectedVerts.Set(index);
					}
				}
			}
		for (i =0; i < tmd->mnMesh->numf; i++)
			{
			if (!tmd->faceSel[i])
				{
				int ct = tmd->mnMesh->f[i].deg;
				for (int j = 0;j < ct; j++)
					{
					int index = tmd->mnMesh->f[i].vtx[j];
					if (selectedVerts[index])
						tmd->faceSel.Set(i);

					}
				}
			}
		}

	}
void	UnwrapMod::GeomContractFaceSel(MeshTopoData* tmd)
	{
	BitArray unselectedVerts;
	
	if ((objType == IS_MESH) && (tmd->mesh))
		{
		unselectedVerts.SetSize(tmd->mesh->getNumVerts());
		unselectedVerts.ClearAll();
		for (int i =0; i < tmd->mesh->getNumFaces(); i++)
			{
			if (!tmd->faceSel[i])
				{
				for (int j = 0;j < 3; j++)
					{
					int index = tmd->mesh->faces[i].v[j];
					unselectedVerts.Set(index);
					}
				}
			}
		for (i =0; i < tmd->mesh->getNumFaces(); i++)
			{
			if (tmd->faceSel[i])
				{
				for (int j = 0;j < 3; j++)
					{
					int index = tmd->mesh->faces[i].v[j];
					if (unselectedVerts[index])
						tmd->faceSel.Set(i,FALSE);

					}
				}
			}

		}
	else if ((objType == IS_PATCH) && (tmd->patch))
		{
		unselectedVerts.SetSize(tmd->patch->getNumVerts());
		unselectedVerts.ClearAll();
		for (int i =0; i < tmd->patch->getNumPatches(); i++)
			{
			if (!tmd->faceSel[i])
				{
				int ct = 4;
				if (tmd->patch->patches[i].type == PATCH_TRI) ct = 3;
				for (int j = 0;j < ct; j++)
					{
					int index = tmd->patch->patches[i].v[j];
					unselectedVerts.Set(index);
					}
				}
			}
		for (i =0; i < tmd->patch->getNumPatches(); i++)
			{
			if (tmd->faceSel[i])
				{
				int ct = 4;
				if (tmd->patch->patches[i].type == PATCH_TRI) ct = 3;
				for (int j = 0;j < ct; j++)
					{
					int index = tmd->patch->patches[i].v[j];
					if (unselectedVerts[index])
						tmd->faceSel.Set(i,FALSE);

					}
				}
			}
		}
	else if ((objType == IS_MNMESH) && (tmd->mnMesh))
		{
		unselectedVerts.SetSize(tmd->mnMesh->numv);
		unselectedVerts.ClearAll();
		for (int i =0; i < tmd->mnMesh->numf; i++)
			{
			if (!tmd->faceSel[i])
				{
				int ct = tmd->mnMesh->f[i].deg;
				for (int j = 0;j < ct; j++)
					{
					int index = tmd->mnMesh->f[i].vtx[j];
					unselectedVerts.Set(index);
					}
				}
			}
		for (i =0; i < tmd->mnMesh->numf; i++)
			{
			if (tmd->faceSel[i])
				{
				int ct = tmd->mnMesh->f[i].deg;
				for (int j = 0;j < ct; j++)
					{
					int index = tmd->mnMesh->f[i].vtx[j];
					if (unselectedVerts[index])
						tmd->faceSel.Set(i,FALSE);

					}
				}
			}
		}

	}
void	UnwrapMod::fnGeomExpandFaceSel()
	{
	//check for type
	ModContextList mcList;		
	INodeTab nodes;

	if (!ip) return;
	ip->GetModContexts(mcList,nodes);

	int objects = mcList.Count();

	if (objects != 0)
		{

		MeshTopoData *tmd = (MeshTopoData*)mcList[0]->localData;

		if (tmd == NULL) 
			{
			return;
			}


		if (!theHold.Holding() )
			theHold.Begin();


		if (theHold.Holding()) theHold.Put (new UnwrapSelRestore (this,tmd));


		if ((objType == IS_MESH) && (!tmd->mesh))
			{
			updateCache = TRUE;
			NotifyDependents(FOREVER,PART_SELECT,REFMSG_CHANGE);
			}
		else if ((objType == IS_PATCH) && (!tmd->patch))
			{
			updateCache = TRUE;
			NotifyDependents(FOREVER,PART_SELECT,REFMSG_CHANGE);
			}
		else if ((objType == IS_MNMESH) && (!tmd->mnMesh))
			{
			updateCache = TRUE;
			NotifyDependents(FOREVER,PART_SELECT,REFMSG_CHANGE);
			}

		GeomExpandFaceSel(tmd);

		if (fnGetSyncSelectionMode()) fnSyncTVSelection();

		UpdateFaceSelection(tmd->faceSel);

		theHold.Accept (GetString (IDS_DS_SELECT));
		ComputeSelectedFaceData();
		NotifyDependents(FOREVER,PART_SELECT,REFMSG_CHANGE);
		InvalidateView();
		ip->RedrawViews(ip->GetTime());
		}
	}
void	UnwrapMod::fnGeomContractFaceSel()
	{
	//check for type
	ModContextList mcList;		
	INodeTab nodes;

	if (!ip) return;
	ip->GetModContexts(mcList,nodes);

	int objects = mcList.Count();

	if (objects != 0)
		{

		MeshTopoData *tmd = (MeshTopoData*)mcList[0]->localData;

		if (tmd == NULL) 
			{
			return;
			}


		if (!theHold.Holding() )
			theHold.Begin();


		if (theHold.Holding()) theHold.Put (new UnwrapSelRestore (this,tmd));


		if ((objType == IS_MESH) && (!tmd->mesh))
			{
			updateCache = TRUE;
			NotifyDependents(FOREVER,PART_SELECT,REFMSG_CHANGE);
			}
		else if ((objType == IS_PATCH) && (!tmd->patch))
			{
			updateCache = TRUE;
			NotifyDependents(FOREVER,PART_SELECT,REFMSG_CHANGE);
			}
		else if ((objType == IS_MNMESH) && (!tmd->mnMesh))
			{
			updateCache = TRUE;
			NotifyDependents(FOREVER,PART_SELECT,REFMSG_CHANGE);
			}

		GeomContractFaceSel(tmd);
	
		if (fnGetSyncSelectionMode()) fnSyncTVSelection();

		UpdateFaceSelection(tmd->faceSel);

		theHold.Accept (GetString (IDS_DS_SELECT));
		ComputeSelectedFaceData();
		NotifyDependents(FOREVER,PART_SELECT,REFMSG_CHANGE);
		InvalidateView();
		ip->RedrawViews(ip->GetTime());
		}
	}



BitArray* UnwrapMod::fnGetSelectedVerts()
//int fnGetSelectedVerts()
	{
	return &vsel;
	}

void UnwrapMod::fnSelectVerts(BitArray *sel)
	{
	vsel.ClearAll();
	for (int i =0; i < vsel.GetSize(); i++)
		{
		if (i < sel->GetSize())
			{
			if ((*sel)[i]) vsel.Set(i);
			}
		}

	if (fnGetSyncSelectionMode()) fnSyncGeomSelection();

	InvalidateView();
	}

BOOL UnwrapMod::fnIsVertexSelected(int index)
	{
	index--;
	if (index < vsel.GetSize())
		return vsel[index];

	return FALSE;
	}



BitArray* UnwrapMod::fnGetSelectedFaces()
	{
	return &fsel;
	}

void	UnwrapMod::fnSelectFaces(BitArray *sel)
	{
	fsel.ClearAll();
	for (int i =0; i < fsel.GetSize(); i++)
		{
		if (i < sel->GetSize())
			{
			if ((*sel)[i]) fsel.Set(i);
			}
		}
	if (fnGetSyncSelectionMode()) fnSyncGeomSelection();
	InvalidateView();
	}

BOOL	UnwrapMod::fnIsFaceSelected(int index)
	{
	index--;
	if (index < fsel.GetSize())
		return fsel[index];

	return FALSE;
	}

void    UnwrapMod::GetVertSelFromFace(BitArray &sel)
	{
	sel.SetSize(TVMaps.v.Count());
	sel.ClearAll();
	for (int i = 0; i < TVMaps.f.Count(); i++)
		{
		if (fsel[i])
			{
			int pcount = 3;
			pcount = TVMaps.f[i]->count;
			for (int k = 0; k < pcount; k++)
				{
				int index = TVMaps.f[i]->t[k];
				sel.Set(index);
				if ((TVMaps.f[i]->flags & FLAG_CURVEDMAPPING) && (TVMaps.f[i]->vecs))
					{
					index = TVMaps.f[i]->vecs->handles[k*2];
					if ((index >= 0) && (index < sel.GetSize()))
						sel.Set(index);

					index = TVMaps.f[i]->vecs->handles[k*2+1];
					if ((index >= 0) && (index < sel.GetSize()))
						sel.Set(index);
	
					if (TVMaps.f[i]->flags & FLAG_INTERIOR) 
						{
						index = TVMaps.f[i]->vecs->interiors[k];
						if ((index >= 0) && (index < sel.GetSize()))
							sel.Set(index);
						}
					}

				}
			}

		}
	}

void    UnwrapMod::GetFaceSelFromVert(BitArray &sel, BOOL partialSelect)
	{
	sel.SetSize(TVMaps.f.Count());
	sel.ClearAll();	
	for (int i = 0; i < TVMaps.f.Count(); i++)
		{
		int pcount = 3;
		pcount = TVMaps.f[i]->count;
		int total = 0;
		for (int k = 0; k < pcount; k++)
			{
			int index = TVMaps.f[i]->t[k];
			if (vsel[index]) total++;
			if ((TVMaps.f[i]->flags & FLAG_CURVEDMAPPING) && (TVMaps.f[i]->vecs))
				{
				index = TVMaps.f[i]->vecs->handles[k*2];
				if ((index >= 0) && (index < sel.GetSize()))
					{
					if (vsel[index]) total++;
					}

				index = TVMaps.f[i]->vecs->handles[k*2+1];
				if ((index >= 0) && (index < sel.GetSize()))
					{
					if (vsel[index]) total++;
					}
	
				if (TVMaps.f[i]->flags & FLAG_INTERIOR) 
					{
					index = TVMaps.f[i]->vecs->interiors[k];
					if ((index >= 0) && (index < sel.GetSize()))
						{
						if (vsel[index]) 
							total++;
						}
					}

				}
			}
		if ((partialSelect) && (total))
			sel.Set(i);
		else if ((!partialSelect) && (total >= pcount))
			sel.Set(i);

		}
	}




void	UnwrapMod::TransferSelectionStart()
	{
	
	originalSel.SetSize(vsel.GetSize());
	originalSel = vsel;

	holdFSel.SetSize(fsel.GetSize());
	holdFSel = fsel;

	holdESel.SetSize(esel.GetSize());
	holdESel = esel;


	if (fnGetTVSubMode() == TVVERTMODE)
		{
		}
	else if (fnGetTVSubMode() == TVEDGEMODE)
		{
		GetVertSelFromEdge(vsel);
		}
	else if (fnGetTVSubMode() == TVFACEMODE)
		{
		GetVertSelFromFace(vsel);
		}
	}	

		//this transfer our vertex selection into our curren selection
void	UnwrapMod::TransferSelectionEnd(BOOL partial,BOOL recomputeSelection)
	{


	if (fnGetTVSubMode() == TVVERTMODE)
		{
		//is vertex selection do not need to do anything
		}
	else if (fnGetTVSubMode() == TVEDGEMODE) //face mode
		{
		//need to convert our vertex selection to face
		if ( (recomputeSelection)  || (holdESel.GetSize() != TVMaps.ePtrList.Count()))
			GetEdgeSelFromVert(esel,partial);
		else esel = holdESel;
		//now we need to restore the vertex selection
		vsel = originalSel;
		}
	else if (fnGetTVSubMode() == TVFACEMODE) //face mode
		{
		//need to convert our vertex selection to face
		if (recomputeSelection) GetFaceSelFromVert(fsel,partial);
		else fsel = holdFSel;
		//now we need to restore the vertex selection as long as we have not changed topo
		if (vsel.GetSize() == originalSel.GetSize())
			vsel = originalSel;
		}


	}


BitArray* UnwrapMod::fnGetSelectedEdges()
	{
	return &esel;
	}

void	UnwrapMod::fnSelectEdges(BitArray *sel)
	{
	esel.ClearAll();
	for (int i =0; i < esel.GetSize(); i++)
		{
		if (i < sel->GetSize())
			{
			if ((*sel)[i]) esel.Set(i);
			}
		}
	if (fnGetSyncSelectionMode()) fnSyncGeomSelection();
	InvalidateView();
	}

BOOL	UnwrapMod::fnIsEdgeSelected(int index)
	{
	index--;
	if (index < esel.GetSize())
		return esel[index];

	return FALSE;
	}


		//Converts the edge selection into a vertex selection set
void    UnwrapMod::GetVertSelFromEdge(BitArray &sel)
	{
	sel.SetSize(TVMaps.v.Count());
	sel.ClearAll();
	for (int i = 0; i < TVMaps.ePtrList.Count(); i++)
		{
		if (esel[i])
			{
			int index = TVMaps.ePtrList[i]->a;
			if ((index >= 0) && (index < sel.GetSize()))
				sel.Set(index);

			index = TVMaps.ePtrList[i]->b;
			if ((index >= 0) && (index < sel.GetSize()))
				sel.Set(index);

			index = TVMaps.ePtrList[i]->avec;
			if ((index >= 0) && (index < sel.GetSize()))
				sel.Set(index);

			index = TVMaps.ePtrList[i]->bvec;
			if ((index >= 0) && (index < sel.GetSize()))
				sel.Set(index);


			}

		}
	}
		//Converts the vertex selection into a edge selection set
		//PartialSelect determines whether all the vertices of a edge need to be selected for that edge to be selected
void    UnwrapMod::GetEdgeSelFromVert(BitArray &sel,BOOL partialSelect)
	{
	sel.SetSize(esel.GetSize());
	sel.ClearAll();
	for (int i =0; i < TVMaps.ePtrList.Count(); i++)
		{
		int a,b;
		a = TVMaps.ePtrList[i]->a;
		b = TVMaps.ePtrList[i]->b;
		if (partialSelect)
			{
			if (vsel[a] || vsel[b])
				sel.Set(i);
			}
		else
			{
			if (vsel[a] && vsel[b])
				sel.Set(i);
			}

		}
	}


BOOL	UnwrapMod::fnGetUVEdgeMode()
	{
	return uvEdgeMode;
	}
void	UnwrapMod::fnSetUVEdgeMode(BOOL uvmode)	
	{
	if (uvmode)
		{
		tvElementMode = FALSE;
		openEdgeMode = FALSE;
		}
	 uvEdgeMode = uvmode;
	}

BOOL	UnwrapMod::fnGetTVElementMode()
	{
	return tvElementMode;
	}
void	UnwrapMod::fnSetTVElementMode(BOOL mode)
	{
	if (mode)
		{
		uvEdgeMode = FALSE;
		openEdgeMode = FALSE;
		}
	tvElementMode =mode;
	}

void	UnwrapMod::SelectUVEdge()
	{		
	BitArray originalESel;
	originalESel.SetSize(esel.GetSize());
	originalESel = esel;

//look for open edges first
	int edgeCount = esel.GetSize();
	
		
	int eselSet = 0;

	while (eselSet!= esel.NumberSet())
		{
		eselSet =  esel.NumberSet();
		GrowSelectUVEdge();
				//get connecting a edge
		}
	}


void	UnwrapMod::SelectOpenEdge()
	{		
	BitArray originalESel;
	originalESel.SetSize(esel.GetSize());
	originalESel = esel;

//look for open edges first
	int edgeCount = esel.GetSize();
	
		
	int eselSet = 0;

	while (eselSet!= esel.NumberSet())
		{
		eselSet =  esel.NumberSet();
		GrowSelectOpenEdge();
				//get connecting a edge
		}
	}


void	UnwrapMod::GrowSelectOpenEdge()
	{

	int edgeCount = esel.GetSize();

	Tab<int> edgeCounts;
	edgeCounts.SetCount(TVMaps.v.Count());
	for (int i = 0; i < TVMaps.v.Count(); i++)
		edgeCounts[i] = 0;
	for (i = 0; i < edgeCount; i++)
		{
		int a = TVMaps.ePtrList[i]->a;
		int b = TVMaps.ePtrList[i]->b;
		if (esel[i])
			{
			edgeCounts[a]++;
			edgeCounts[b]++;
			}
		}
	for (i = 0; i < edgeCount; i++)
		{
		if ( (!esel[i]) && (TVMaps.ePtrList[i]->faceList.Count() == 1) )
			{
			int a = TVMaps.ePtrList[i]->a;
			int b = TVMaps.ePtrList[i]->b;
			int aCount = edgeCounts[a];
			int bCount = edgeCounts[b];
			if ( ( ( aCount == 0) && (bCount >= 1) ) ||
				 ( (bCount == 0) && (aCount >= 1) ) ||
				 ( (bCount >= 1) && (aCount >= 1) ) )
				esel.Set(i,TRUE);
			}
		}
	}

void	UnwrapMod::ShrinkSelectOpenEdge()
	{


	int edgeCount = esel.GetSize();

	Tab<int> edgeCounts;
	edgeCounts.SetCount(TVMaps.v.Count());
	for (int i = 0; i < TVMaps.v.Count(); i++)
		edgeCounts[i] = 0;
	for (i = 0; i < edgeCount; i++)
		{
		int a = TVMaps.ePtrList[i]->a;
		int b = TVMaps.ePtrList[i]->b;
		if (esel[i])
			{
			edgeCounts[a]++;
			edgeCounts[b]++;
			}
		}
	for (i = 0; i < edgeCount; i++)
		{
		if ( (esel[i])  && (TVMaps.ePtrList[i]->faceList.Count() == 1) )
			{
			int a = TVMaps.ePtrList[i]->a;
			int b = TVMaps.ePtrList[i]->b;
			if ( (edgeCounts[a] == 1) || (edgeCounts[b] == 1) )
				esel.Set(i,FALSE);
			}
		}
	}


void	UnwrapMod::GrowSelectUVEdge()
 {
// get current edgesel
	Tab<int> edgeConnectedCount;
	Tab<int> numberEdgesAtVert;
	int edgeCount = esel.GetSize();
	int vertCount = vsel.GetSize();  edgeConnectedCount.SetCount(vertCount);
	numberEdgesAtVert.SetCount(vertCount);

	for (int i = 0; i < vertCount; i++)
		{
		edgeConnectedCount[i] = 0;
		numberEdgesAtVert[i] = 0;
		}

//find all the vertices that touch a selected edge
// and keep a count of the number of selected edges that touch that //vertex  
	for (i = 0; i < edgeCount; i++)
		{
		int a = TVMaps.ePtrList[i]->a;
		int b = TVMaps.ePtrList[i]->b;
		if (a!=b)
			{

			if (!(TVMaps.ePtrList[i]->flags & FLAG_HIDDENEDGEA))
				{
				numberEdgesAtVert[a]++;
				numberEdgesAtVert[b]++;
				}
			if (esel[i])
				{
				edgeConnectedCount[a]++;
				edgeConnectedCount[b]++;
				}
			}
		}


	BitArray edgesToExpand;
	edgesToExpand.SetSize(edgeCount);
	edgesToExpand.ClearAll();

//tag any edge that has only one vertex count since that will be an end edge  
	for (i = 0; i < edgeCount; i++)
		{
		int a = TVMaps.ePtrList[i]->a;
		int b = TVMaps.ePtrList[i]->b;
		if (a!=b)
			{
			if (esel[i])
				{
				if ((edgeConnectedCount[a] == 1) || (edgeConnectedCount[b] == 1))
					edgesToExpand.Set(i,TRUE);
				}
			}
		}

	for (i = 0; i < edgeCount; i++)
		{
		if (edgesToExpand[i])
			{
//make sure we have an even number of edges at this vert
//if odd then we can not go further
//   if ((numberEdgesAtVert[i] % 2) == 0)
//now need to find our most opposing edge
//find all edges connected to the vertex

		    for (int k = 0; k < 2; k++)
				{
			    int a = 0;
			    if (k==0) a = TVMaps.ePtrList[i]->a;
				else a = TVMaps.ePtrList[i]->b;
		
				if ( (edgeConnectedCount[a] == 1) && ((numberEdgesAtVert[a] % 2) == 0))
					{
					int centerVert = a;
				    Tab<int> edgesConnectedToVert;
					for (int j = 0; j < edgeCount; j++)
						{
						if (j!=i)  //make sure we dont add our selected vertex
							{
							int a = TVMaps.ePtrList[j]->a;
							int b = TVMaps.ePtrList[j]->b;

							if (a!=b)
								{
								if ((a == centerVert) || (b==centerVert))
									{
									edgesConnectedToVert.Append(1,&j);
									}
								}
							}

						}
//get a face connected to our oririnal egd
					int faceIndex = TVMaps.ePtrList[i]->faceList[0];
				    int count = numberEdgesAtVert[centerVert]/2;
					int tally = 0;
					BOOL done = FALSE;
					while (!done)
						{
						int lastEdge = -1;


						for (int m = 0; m < edgesConnectedToVert.Count(); m++)
							{
							int edgeIndex = edgesConnectedToVert[m];
							for (int n = 0; n < TVMaps.ePtrList[edgeIndex]->faceList.Count(); n++)
								{
								if (faceIndex == TVMaps.ePtrList[edgeIndex]->faceList[n])
									{
									for (int p = 0; p < TVMaps.ePtrList[edgeIndex]->faceList.Count(); p++)
										{
										if (faceIndex != TVMaps.ePtrList[edgeIndex]->faceList[p])
											{
											faceIndex = TVMaps.ePtrList[edgeIndex]->faceList[p];
											p = TVMaps.ePtrList[edgeIndex]->faceList.Count();
											}
										}
									if (!(TVMaps.ePtrList[edgeIndex]->flags& FLAG_HIDDENEDGEA))
										tally++;
									edgesConnectedToVert.Delete(m,1);
									m = edgesConnectedToVert.Count();
									n = TVMaps.ePtrList[edgeIndex]->faceList.Count();

									lastEdge = edgeIndex;
									}
								}
							}
						if (lastEdge == -1)
							{
//					        assert(0);
					        done = TRUE;
							}
						if (tally >= count)
							{
							done = TRUE;
							if (lastEdge != -1)
								esel.Set(lastEdge,TRUE);
							}

						}
					}
				}
			}
		}	
	}
  


void	UnwrapMod::ShrinkSelectUVEdge()
	{


	int edgeCount = esel.GetSize();

	Tab<int> edgeCounts;
	edgeCounts.SetCount(TVMaps.v.Count());
	for (int i = 0; i < TVMaps.v.Count(); i++)
		edgeCounts[i] = 0;
	for (i = 0; i < edgeCount; i++)
		{
		int a = TVMaps.ePtrList[i]->a;
		int b = TVMaps.ePtrList[i]->b;
		if (esel[i])
			{
			edgeCounts[a]++;
			edgeCounts[b]++;
			}
		}
	for (i = 0; i < edgeCount; i++)
		{
		if ( (esel[i])  && (TVMaps.ePtrList[i]->faceList.Count() == 1) )
			{
			int a = TVMaps.ePtrList[i]->a;
			int b = TVMaps.ePtrList[i]->b;
			if ( (edgeCounts[a] == 1) || (edgeCounts[b] == 1) )
				esel.Set(i,FALSE);
			}
		}
	}


BOOL	UnwrapMod::fnGetOpenEdgeMode()
	{
	return openEdgeMode;
	}

void	UnwrapMod::fnSetOpenEdgeMode(BOOL mode)
	{
	if (mode)
		{
		uvEdgeMode = FALSE;
		tvElementMode =FALSE;
		}

	openEdgeMode = mode;
	}

void	UnwrapMod::fnUVEdgeSelect()
	{
	if (!theHold.Holding())
		theHold.Begin();

	HoldSelection();
	SelectUVEdge();
	if (fnGetSyncSelectionMode()) fnSyncGeomSelection();
	theHold.Accept(GetString(IDS_DS_SELECT));	
	InvalidateView();			
	}

void	UnwrapMod::fnOpenEdgeSelect()
	{
	if (!theHold.Holding())
		theHold.Begin();

	HoldSelection();
	SelectOpenEdge();
	if (fnGetSyncSelectionMode()) fnSyncGeomSelection();
	theHold.Accept(GetString(IDS_DS_SELECT));					
	InvalidateView();
	}


void	UnwrapMod::fnVertToEdgeSelect()
	{
	if (!theHold.Holding())
		theHold.Begin();
	HoldSelection();

	GetEdgeSelFromVert(esel,FALSE);
	if (fnGetSyncSelectionMode()) fnSyncGeomSelection();
	theHold.Accept(GetString(IDS_DS_SELECT));	
	InvalidateView();
	}
void	UnwrapMod::fnVertToFaceSelect()
	{
	if (!theHold.Holding())
		theHold.Begin();
	HoldSelection();

	GetFaceSelFromVert(fsel,FALSE);

	if (fnGetSyncSelectionMode()) fnSyncGeomSelection();

	theHold.Accept(GetString(IDS_DS_SELECT));	
	InvalidateView();
	}

void	UnwrapMod::fnEdgeToVertSelect()
	{
	if (!theHold.Holding())
		theHold.Begin();
	HoldSelection();

	GetVertSelFromEdge(vsel);

	if (fnGetSyncSelectionMode()) fnSyncGeomSelection();

	theHold.Accept(GetString(IDS_DS_SELECT));	
	InvalidateView();	
	}
void	UnwrapMod::fnEdgeToFaceSelect()
	{
	if (!theHold.Holding())
		theHold.Begin();
	HoldSelection();

	BitArray tempSel;
	tempSel.SetSize(vsel.GetSize());
	tempSel = vsel;
	GetVertSelFromEdge(vsel);
	GetFaceSelFromVert(fsel,FALSE);

	vsel = tempSel;

	if (fnGetSyncSelectionMode()) fnSyncGeomSelection();

	theHold.Accept(GetString(IDS_DS_SELECT));	
	InvalidateView();
	}

void	UnwrapMod::fnFaceToVertSelect()
	{
	if (!theHold.Holding())
		theHold.Begin();
	HoldSelection();

	GetVertSelFromFace(vsel);

	if (fnGetSyncSelectionMode()) fnSyncGeomSelection();

	theHold.Accept(GetString(IDS_DS_SELECT));	
	InvalidateView();
	}
void	UnwrapMod::fnFaceToEdgeSelect()
	{
	if (!theHold.Holding())
		theHold.Begin();
	HoldSelection();

	ConvertFaceToEdgeSel();
/*
	BitArray tempSel;
	tempSel.SetSize(vsel.GetSize());
	tempSel = vsel;
	GetVertSelFromFace(vsel);
	GetEdgeSelFromVert(esel,FALSE);

	vsel = tempSel;
*/
	if (fnGetSyncSelectionMode()) fnSyncGeomSelection();

	theHold.Accept(GetString(IDS_DS_SELECT));	
	InvalidateView();
	}


void UnwrapMod::InitReverseSoftData()
	{
	RebuildDistCache();
	BitArray originalVSel(vsel);

	sketchBelongsToList.SetCount(TVMaps.v.Count());
	originalPos.SetCount(TVMaps.v.Count());

	for (int i = 0; i < TVMaps.v.Count(); i++)
		{
		sketchBelongsToList[i] = -1;
		originalPos[i] = TVMaps.v[i].p;
		}
	
	for (i = 0; i < TVMaps.v.Count(); i++)
		{
		if ((!originalVSel[i]) && (TVMaps.v[i].influence > 0.0f))
			{
			int closest = -1;
			float closestDist = 0.0f;
			Point3 a = TVMaps.v[i].p;
			for (int j = 0; j < TVMaps.v.Count(); j++)
				{
				if (vsel[j])
					{
					Point3 b = TVMaps.v[j].p;
					float dist = Length(a-b);
					if ((dist < closestDist) || (closest == -1))
						{
						closest = j;
						closestDist = dist;
						}
					}
				}
			if (closest != -1)
				{
				sketchBelongsToList[i] = closest;
				}
			}
		}
	vsel = originalVSel;

	}
void UnwrapMod::ApplyReverseSoftData()
	{
	for (int i = 0; i < TVMaps.v.Count(); i++)
		{
		if (sketchBelongsToList[i] >= 0)
			{
			Point3 accumVec(0.0f,0.0f,0.0f);
			int index = sketchBelongsToList[i];
			Point3 vec = TVMaps.v[index].p - originalPos[index];
			accumVec += vec * TVMaps.v[i].influence;
	
			Point3 p = TVMaps.v[i].p + accumVec;
			TVMaps.v[i].p = p;
			if (TVMaps.cont[i]) TVMaps.cont[i]->SetValue(0,&TVMaps.v[i].p);
			}
		}
	}


int		UnwrapMod::fnGetHitSize()
	{
	return hitSize;
	}
void	UnwrapMod::fnSetHitSize(int size)
	{
	hitSize = size;
	}


BOOL	UnwrapMod::fnGetPolyMode()
	{
	return polyMode;
	}
void	UnwrapMod::fnSetPolyMode(BOOL pmode)
	{
	polyMode = pmode;
	}

void	UnwrapMod::ConvertFaceToEdgeSel()
	{
	esel.ClearAll();
	for (int i = 0; i < TVMaps.ePtrList.Count();i++)
		{
		for (int j = 0; j < TVMaps.ePtrList[i]->faceList.Count();j++)
			{
			int index = TVMaps.ePtrList[i]->faceList[j];
			if (fsel[index])
				{
				esel.Set(i);
				j = TVMaps.ePtrList[i]->faceList.Count();
				}
			}
		
		}

	}

void	UnwrapMod::fnPolySelect()
	{

	BitArray originalESel(esel);
	BitArray originalVSel(vsel);
	
	BitArray tempSel;
//convert to edge sel
	ConvertFaceToEdgeSel();

//repeat until selection  not done
	int selCount = fsel.NumberSet();
	BOOL done= FALSE;
	int eSelCount = esel.GetSize();
	while (!done)
		{
		for (int i =0; i < eSelCount; i++)
			{
			if ( (esel[i])  && (TVMaps.ePtrList[i]->flags&FLAG_HIDDENEDGEA))
				{
				int ct = TVMaps.ePtrList[i]->faceList.Count();
				for (int j = 0; j < ct; j++)
					{
					int index = TVMaps.ePtrList[i]->faceList[j];
					fsel.Set(index);				
					}
				}
			}

		if (selCount == fsel.NumberSet()) 
			done = TRUE;
		else
			{
			selCount = fsel.NumberSet();
			ConvertFaceToEdgeSel();

			}
		}

	esel = originalESel;
	vsel = originalVSel;

	if (fnGetSyncSelectionMode()) fnSyncGeomSelection();

	InvalidateView();
	
	}


BOOL	UnwrapMod::fnGetSyncSelectionMode()
	{
	return syncSelection;
	}

void	UnwrapMod::fnSetSyncSelectionMode(BOOL sync)
	{
	syncSelection = sync;
	}


void	UnwrapMod::SyncGeomToTVSelection(MeshTopoData *md)
	
	{
//get our geom face sel
	if (fnGetTVSubMode() == TVVERTMODE)
		{
		GetFaceSelFromVert(md->faceSel,FALSE);
		}
	else if (fnGetTVSubMode() == TVEDGEMODE)
		{
		BitArray holdFaceSel(fsel);
		fsel = md->faceSel;

		BitArray tempSel(vsel);
		GetVertSelFromEdge(vsel);
		GetFaceSelFromVert(fsel,FALSE);

		vsel = tempSel;
		md->faceSel = fsel;
		fsel = holdFaceSel;
		}
	else if (fnGetTVSubMode() == TVFACEMODE)
		{
		md->faceSel = fsel;
		}
	}

void	UnwrapMod::SyncTVToGeomSelection(MeshTopoData *md)
	{
//get our geom face sel
	if (fnGetTVSubMode() == TVVERTMODE)
		{
		BitArray holdFaceSel(fsel);
		fsel = md->faceSel;
		GetVertSelFromFace(vsel);
		fsel = holdFaceSel;
		}
	else if (fnGetTVSubMode() == TVEDGEMODE)
		{
		BitArray holdFaceSel(fsel);
		fsel = md->faceSel;
		ConvertFaceToEdgeSel();
		fsel = holdFaceSel;
		}
	else if (fnGetTVSubMode() == TVFACEMODE)
		{
		fsel = md->faceSel;
		}

	}


void	UnwrapMod::fnSyncTVSelection()
	{
	if (!ip) return;

	if (!theHold.Holding() )
		theHold.Begin();

	HoldSelection();



		//check for type
	ModContextList mcList;		
	INodeTab nodes;

	ip->GetModContexts(mcList,nodes);

	int objects = mcList.Count();


	if (objects != 0)
		{
		MeshTopoData *md = (MeshTopoData*)mcList[0]->localData;

		if (md == NULL) 
			{
			theHold.Cancel();
			return;
			}


		SyncTVToGeomSelection(md);

		theHold.Accept(GetString(IDS_DS_SELECT));					
		InvalidateView();
		}
	else theHold.Cancel();


	}

void	UnwrapMod::fnSyncGeomSelection()
	{

	if (!ip) return;

	if (!theHold.Holding())
		theHold.Begin();
	HoldSelection();
		//check for type
	ModContextList mcList;		
	INodeTab nodes;

	ip->GetModContexts(mcList,nodes);

	int objects = mcList.Count();


	if (objects != 0)
		{
		MeshTopoData *md = (MeshTopoData*)mcList[0]->localData;

		if (md)
			{
			SyncGeomToTVSelection(md);

			theHold.Accept(GetString(IDS_DS_SELECT));					
			InvalidateView();
			NotifyDependents(FOREVER,PART_SELECT,REFMSG_CHANGE);
			if (ip)	ip->RedrawViews(ip->GetTime());
			}
		else theHold.Cancel();
		}
	else theHold.Cancel();

	}


BOOL	UnwrapMod::fnGetPaintMode()
	{
	if (mode==ID_PAINTSELECTMODE)
		return TRUE;
	else return FALSE;
	}
void	UnwrapMod::fnSetPaintMode(BOOL paint)
	{
	if (paint)
		{
		if (ip) ip->ReplacePrompt( GetString(IDS_PW_PAINTSELECTPROMPT));
		SetMode(ID_PAINTSELECTMODE);
		}
	}


int		UnwrapMod::fnGetPaintSize()
	{
	return paintSize ;
	}

void	UnwrapMod::fnSetPaintSize(int size)
	{
	
	paintSize = size;

	if (paintSize < 1) paintSize = 1;
	if (paintSize > 15) paintSize = 15;

	}

void	UnwrapMod::fnIncPaintSize()
	{
	paintSize++;
	if (paintSize < 1) paintSize = 1;
	if (paintSize > 15) paintSize = 15;
	}
void	UnwrapMod::fnDecPaintSize()
	{
	paintSize--;
	if (paintSize < 1) paintSize = 1;
	if (paintSize > 15) paintSize = 15;
	}