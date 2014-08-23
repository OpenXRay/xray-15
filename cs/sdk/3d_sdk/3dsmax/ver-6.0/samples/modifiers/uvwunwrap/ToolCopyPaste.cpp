#include "unwrap.h"
/***************************************************************
****************************************************************/
//CopyPasteBuffer    UnwrapMod::copyPasteBuffer;


CopyPasteBuffer::~CopyPasteBuffer()
	{
	for (int i =0; i < faceData.Count(); i++)
		delete faceData[i];
	}


BOOL CopyPasteBuffer::CanPaste()
	{
	if (faceData.Count() == 0) return FALSE;
		return TRUE;
	}
BOOL CopyPasteBuffer::CanPasteInstance(UnwrapMod *mod)
	{
	if (faceData.Count() == 0) return FALSE;
	if (this->mod != mod) return FALSE;
	return TRUE;
	}

void	UnwrapMod::fnCopy()
	{
	//check for type
	ModContextList mcList;		
	INodeTab nodes;

	if (!ip) return;
	ip->GetModContexts(mcList,nodes);

	int objects = mcList.Count();

	copyPasteBuffer.iRotate = 0;

	
	copyPasteBuffer.copyType = 2;

	if (objects != 0)
		{

		copyPasteBuffer.mod = this;
//copy the vertex list over
		copyPasteBuffer.tVertData.SetCount(TVMaps.v.Count());
		for (int i =0; i < TVMaps.v.Count(); i++)
			{
			copyPasteBuffer.tVertData[i] = TVMaps.v[i].p;
			}	

		
		MeshTopoData *md = (MeshTopoData*)mcList[0]->localData;//copy the face data over that is selected
		copyPasteBuffer.lmd = md;

		if (md == NULL) return;

		for (i = 0; i < copyPasteBuffer.faceData.Count(); i++)
			{
			delete copyPasteBuffer.faceData[i];
			}

		int ct = md->faceSel.NumberSet();
		if (md->faceSel.NumberSet() == TVMaps.f.Count())
			copyPasteBuffer.copyType = 1;
		if (md->faceSel.NumberSet() == 1)
			copyPasteBuffer.copyType = 0;

		copyPasteBuffer.faceData.SetCount(ct);


		int faceIndex = 0;
		for (i = 0; i < TVMaps.f.Count(); i++)
			{
			if (md->faceSel[i])
				{
				UVW_TVFaceClass *f = TVMaps.f[i]->Clone();
				copyPasteBuffer.faceData[faceIndex] = f;
				faceIndex++;
				}
			}

		}
	}


void	UnwrapMod::fnPaste(BOOL rotate)
	{
	//check for type
	ModContextList mcList;		
	INodeTab nodes;

	if (!ip) return;
	ip->GetModContexts(mcList,nodes);

	int objects = mcList.Count();


	if (objects != 0)
		{
		MeshTopoData *md = (MeshTopoData*)mcList[0]->localData;//copy the face data over that is selected

		if (md == NULL) return;


//check faces if just one selected normal paste with rotates
//or if all faces where selected
//or if first paste
		if (!theHold.Holding())
			{
			theHold.Begin();
			theHold.SuperBegin();
			}
//hold the points and faces
		HoldPointsAndFaces();
		theHold.Accept(_T(GetString(IDS_PW_PASTE)));
		theHold.SuperAccept(_T(GetString(IDS_PW_PASTE)));

		BitArray holdFaceSel(md->faceSel);

		BitArray subFaceSel;
		if ( ip && (ip->GetSubObjectLevel() == 0) )
			{
//convert our current selection into faces			
			if (fnGetTVSubMode() == TVVERTMODE)
				GetFaceSelFromVert(subFaceSel,FALSE);
			else if (fnGetTVSubMode() == TVEDGEMODE)
				{
				BitArray tempVSel;
				GetVertSelFromEdge(tempVSel);
				BitArray holdVSel(vsel);
				vsel = tempVSel;
				GetFaceSelFromVert(subFaceSel,FALSE);
				vsel = holdVSel;
				}
			else 
				{
				subFaceSel.SetSize(fsel.GetSize());
				subFaceSel = fsel;
				}
			}
		else
			{
			if (fnGetTVSubMode() == TVFACEMODE)
				{
				subFaceSel.SetSize(fsel.GetSize());
				subFaceSel = fsel;
				}

			}

		if ( (copyPasteBuffer.copyType == 0) || (copyPasteBuffer.copyType == 1) || (copyPasteBuffer.iRotate==0))
			{
			int copyIndex = 0;
			Tab<int> vertexLookUpList;

			vertexLookUpList.SetCount(copyPasteBuffer.tVertData.Count());


			for (int i =0; i < vertexLookUpList.Count(); i++)
				vertexLookUpList[i] = -1;
			if (copyPasteBuffer.copyType == 1)
				copyPasteBuffer.iRotate = 0;
			else 
				{
				if (copyPasteBuffer.lastSel.GetSize() == md->faceSel.GetSize())
					{
					if (copyPasteBuffer.lastSel == md->faceSel) 
						{

						if (rotate)
							{
							copyPasteBuffer.iRotate++;
							}
						else copyPasteBuffer.iRotate = 0;

						
						}
					}
				}
			if (copyPasteBuffer.copyType == 2) copyPasteBuffer.iRotate = 0;
			copyPasteBuffer.lastSel = md->faceSel;

//loop through selected faces
			for (i =0; i < md->faceSel.GetSize(); i++)
				{
				if (md->faceSel[i])
					{
//make sure selected faces count = buffer face
					if (( i < TVMaps.f.Count()) && (copyIndex < copyPasteBuffer.faceData.Count()))
						{
						if (TVMaps.f[i]->count == copyPasteBuffer.faceData[copyIndex]->count)
							{
//if so	set the face data indices as the same
							for (int j = 0; j < TVMaps.f[i]->count; j++)
								{
							//index into the texture vertlist
							
								int vid = (j + copyPasteBuffer.iRotate)%TVMaps.f[i]->count;
								int vertexIndex = copyPasteBuffer.faceData[copyIndex]->t[vid];

								if (vertexLookUpList[vertexIndex] == -1)
									{
									Point3 p = copyPasteBuffer.tVertData[vertexIndex];
									AddPoint(p, i, j,FALSE);
									vertexLookUpList[vertexIndex] = TVMaps.f[i]->t[j];
									}
								else TVMaps.f[i]->t[j] = vertexLookUpList[vertexIndex];

								if ((TVMaps.f[i]->vecs) && (copyPasteBuffer.faceData[copyIndex]->vecs) && (j < 4))
									{
									int hid = (j*2 + (copyPasteBuffer.iRotate*2))%(TVMaps.f[i]->count*2);
									int handleIndex = copyPasteBuffer.faceData[copyIndex]->vecs->handles[hid];
									if ((handleIndex >= 0) && (vertexLookUpList[handleIndex] == -1))
										{
										Point3 p = copyPasteBuffer.tVertData[handleIndex];
										AddHandle(p, i, j*2,FALSE);
										vertexLookUpList[handleIndex] = TVMaps.f[i]->vecs->handles[j*2];
										}
									else TVMaps.f[i]->vecs->handles[j*2] = vertexLookUpList[handleIndex];
	
									hid = (j*2 + (copyPasteBuffer.iRotate*2))%(TVMaps.f[i]->count*2)+1;
									handleIndex = copyPasteBuffer.faceData[copyIndex]->vecs->handles[hid];
									if ((handleIndex >= 0) && (vertexLookUpList[handleIndex] == -1))
										{
										Point3 p = copyPasteBuffer.tVertData[handleIndex];
										AddHandle(p, i, j*2+1,FALSE);
										vertexLookUpList[handleIndex] = TVMaps.f[i]->vecs->handles[j*2+1];
										}
									else TVMaps.f[i]->vecs->handles[j*2+1] = vertexLookUpList[handleIndex];
	
									int iid = (j + (copyPasteBuffer.iRotate))%(TVMaps.f[i]->count);
									int interiorIndex = copyPasteBuffer.faceData[copyIndex]->vecs->interiors[iid];
									if ((interiorIndex >= 0) && (vertexLookUpList[interiorIndex] == -1))
										{
										Point3 p = copyPasteBuffer.tVertData[interiorIndex];
										AddInterior(p, i, j,FALSE);
										vertexLookUpList[interiorIndex] = TVMaps.f[i]->vecs->handles[j];
										}
									else TVMaps.f[i]->vecs->interiors[j] = vertexLookUpList[interiorIndex];


									}
								}
							copyIndex++;
							if (copyIndex >= copyPasteBuffer.faceData.Count()) copyIndex = 0;
							}
						}	
					}

				}

			}
		CleanUpDeadVertices();
		RebuildEdges();

		if ( ip && (ip->GetSubObjectLevel() == 0) )
			{
			if (fnGetTVSubMode() == TVVERTMODE)
				{
				BitArray holdFSel(fsel);
				fsel = subFaceSel;
				GetVertSelFromFace(vsel);
				fsel = holdFSel;
				}
			else if (fnGetTVSubMode() == TVEDGEMODE)
				{
				BitArray holdFSel(fsel);
				fsel = subFaceSel;
				GetVertSelFromFace(fsel);
				GetEdgeSelFromVert(esel,FALSE);
				fsel = holdFSel;
				}
			else 
				{				
				fsel = subFaceSel;
				}

			}
		else
			{
			md->faceSel = holdFaceSel;
			if (fnGetTVSubMode() == TVFACEMODE)
				{
				fsel = subFaceSel;
				}
			}

		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
		InvalidateView();
		GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());		
		}

	}

void	UnwrapMod::fnPasteInstance()
	{
//make sure mods are the same
	if (this == copyPasteBuffer.mod)
		{
	//check for type
		ModContextList mcList;		
		INodeTab nodes;

		if (!ip) return;
		ip->GetModContexts(mcList,nodes);

		int objects = mcList.Count();



	


		if (objects != 0)
			{
			MeshTopoData *md = (MeshTopoData*)mcList[0]->localData;//copy the face data over that is selected

			if (md == NULL) return;

			if (!theHold.Holding())
				{
				theHold.Begin();
				}
//hold the points and faces
			HoldPointsAndFaces();

//loop through selected faces
			int copyIndex = 0;
			for (int i =0; i < md->faceSel.GetSize(); i++)
				{
				if (md->faceSel[i])
					{
//make sure selected faces count = buffer face
					if (( i < TVMaps.f.Count()) && (copyIndex < copyPasteBuffer.faceData.Count()))
						{
						if (TVMaps.f[i]->count == copyPasteBuffer.faceData[copyIndex]->count)
							{
//if so set the face data indices as the same
							for (int j = 0; j < TVMaps.f[i]->count; j++)
								{
								//index into the texture vertlist
								TVMaps.f[i]->t[j] = copyPasteBuffer.faceData[copyIndex]->t[j];
//index into the geometric vertlist
								if ((TVMaps.f[i]->vecs) && (j < 4))
									{
									TVMaps.f[i]->vecs->interiors[j] = copyPasteBuffer.faceData[copyIndex]->vecs->interiors[j];

									TVMaps.f[i]->vecs->handles[j*2] = copyPasteBuffer.faceData[copyIndex]->vecs->handles[j*2];
									TVMaps.f[i]->vecs->handles[j*2+1] = copyPasteBuffer.faceData[copyIndex]->vecs->handles[j*2+1];

									}
								}
							copyIndex++;
							}
						}
					}
				}
			CleanUpDeadVertices();
			theHold.Accept(_T(GetString(IDS_PW_PASTE)));
			RebuildEdges();
			NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
			InvalidateView();
			GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());

			}
		
		}
	}



void	UnwrapMod::CleanUpDeadVertices()
	{
	BitArray usedList;



	usedList.SetSize(TVMaps.v.Count());
	usedList.ClearAll();

	for (int i = 0; i < TVMaps.f.Count(); i++)
		{
		for (int j = 0; j < TVMaps.f[i]->count; j++)
			{
			int vertIndex = TVMaps.f[i]->t[j];
			usedList.Set(vertIndex);

//index into the geometric vertlist
			if ((TVMaps.f[i]->vecs) && (j < 4))
				{
				vertIndex = TVMaps.f[i]->vecs->interiors[j];
				if ((vertIndex>=0) && (vertIndex < usedList.GetSize()))
					usedList.Set(vertIndex);

				vertIndex = TVMaps.f[i]->vecs->handles[j*2];
				if ((vertIndex>=0) && (vertIndex < usedList.GetSize()))
					usedList.Set(vertIndex);
				vertIndex = TVMaps.f[i]->vecs->handles[j*2+1];
				if ((vertIndex>=0) && (vertIndex < usedList.GetSize()))
					usedList.Set(vertIndex);
				}

			}
		}
	int vTotalCount = TVMaps.v.Count();
	int vInitalDeadCount = 0;
	int vFinalDeadCount = 0;

	for (i =0; i < TVMaps.v.Count(); i++)
		{
		if (TVMaps.v[i].flags & FLAG_DEAD)
			vInitalDeadCount++;
		}

	for (i =0; i < usedList.GetSize(); i++)
		{
		if (!usedList[i])
			{
			TVMaps.v[i].flags |= FLAG_DEAD;
			}
		}

	for (i =0; i < TVMaps.v.Count(); i++)
		{
		if (TVMaps.v[i].flags & FLAG_DEAD)
			vFinalDeadCount++;
		}


#ifdef DEBUGMODE 

		if (gDebugLevel >= 3)
			ScriptPrint("Cleaning Dead Verts Total Verts %d Initial Dead Verts %d Final Dead Verts %d \n",vTotalCount,vInitalDeadCount,vFinalDeadCount); 

#endif
	}



