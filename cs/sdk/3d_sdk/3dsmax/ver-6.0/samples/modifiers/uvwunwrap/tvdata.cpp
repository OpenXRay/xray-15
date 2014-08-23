#include "unwrap.h"



UVW_TVFaceClass* UVW_TVFaceClass::Clone()
	{
	UVW_TVFaceClass *f = new UVW_TVFaceClass;
	f->FaceIndex = FaceIndex;
	f->MatID = MatID;
	f->flags = flags;
	f->count = count;
	f->t = new int[count];
	f->v = new int[count];
	for (int i =0; i < count; i++)
		{
		f->t[i] = t[i];
		f->v[i] = v[i];
		}
	f->vecs = NULL;
	if (vecs)
		{
		UVW_TVVectorClass *vect = new UVW_TVVectorClass;
		f->vecs = vect;

		for (int i =0; i < count; i++)
			{
			vect->interiors[i] =  vecs->interiors[i];
			vect->handles[i*2] =  vecs->handles[i*2];
			vect->handles[i*2+1] =  vecs->handles[i*2+1];

			vect->vinteriors[i] =  vecs->vinteriors[i];
			vect->vhandles[i*2] =  vecs->vhandles[i*2];
			vect->vhandles[i*2+1] =  vecs->vhandles[i*2+1];
			}
		}
	return f;
	}

void UVW_TVFaceClass::DeleteVec()
	{
	if (vecs) delete vecs;
	vecs = NULL;
	}

ULONG UVW_TVFaceClass::SaveFace(ISave *isave)
	{
	ULONG nb = 0;
	isave->Write(&count, sizeof(int), &nb);
	isave->Write(t, sizeof(int)*count, &nb);
	isave->Write(&FaceIndex, sizeof(int), &nb);
	isave->Write(&MatID, sizeof(int), &nb);
	isave->Write(&flags, sizeof(int), &nb);
	isave->Write(v, sizeof(int)*count, &nb);
	BOOL useVecs = TRUE;
	if ( (vecs) && (flags & FLAG_CURVEDMAPPING))
		{
		isave->Write(vecs->handles, sizeof(int)*count*2, &nb);
		isave->Write(vecs->interiors, sizeof(int)*count, &nb);
		isave->Write(vecs->vhandles, sizeof(int)*count*2, &nb);
		isave->Write(vecs->vinteriors, sizeof(int)*count, &nb);

		}
	return nb;
	}

ULONG UVW_TVFaceClass::SaveFace(FILE *file)
	{
	ULONG nb = 1;
	fwrite(&count, sizeof(int), 1,file);
	fwrite(t, sizeof(int)*count, 1,file);
	fwrite(&FaceIndex, sizeof(int), 1,file);
	fwrite(&MatID, sizeof(int), 1,file);
	fwrite(&flags, sizeof(int), 1,file);
	fwrite(v, sizeof(int)*count, 1,file);
	BOOL useVecs = TRUE;
	if ( (vecs) && (flags & FLAG_CURVEDMAPPING))
		{
		fwrite(vecs->handles, sizeof(int)*count*2, 1,file);
		fwrite(vecs->interiors, sizeof(int)*count, 1,file);
		fwrite(vecs->vhandles, sizeof(int)*count*2, 1,file);
		fwrite(vecs->vinteriors, sizeof(int)*count, 1,file);
		}
	return nb;
	}


ULONG UVW_TVFaceClass::LoadFace(ILoad *iload)
	{
	ULONG nb = 0;
	iload->Read(&count, sizeof(int), &nb);

	if (t==NULL)
		t = new int[count];
	else
		{
		delete [] t;
		t = new int[count];
		}

	if (v==NULL)
		v = new int[count];
	else
		{
		delete [] v;
		v = new int[count];
		}


	iload->Read(t, sizeof(int)*count, &nb);
	iload->Read(&FaceIndex, sizeof(int), &nb);
	iload->Read(&MatID, sizeof(int), &nb);
	iload->Read(&flags, sizeof(int), &nb);
	iload->Read(v, sizeof(int)*count, &nb);
	vecs = NULL;

	if (flags & FLAG_CURVEDMAPPING)
		{
		vecs = new UVW_TVVectorClass;
		iload->Read(vecs->handles, sizeof(int)*count*2, &nb);
		iload->Read(vecs->interiors, sizeof(int)*count, &nb);
		iload->Read(vecs->vhandles, sizeof(int)*count*2, &nb);
		iload->Read(vecs->vinteriors, sizeof(int)*count, &nb);
		}
	return nb;
	}

ULONG UVW_TVFaceClass::LoadFace(FILE *file)
	{
	ULONG nb = 0;
	fread(&count, sizeof(int), 1, file);

	if (t==NULL)
		t = new int[count];
	else
		{
		delete [] t;
		t = new int[count];
		}

	if (v==NULL)
		v = new int[count];
	else
		{
		delete [] v;
		v = new int[count];
		}

	fread(t, sizeof(int)*count, 1, file);
	fread(&FaceIndex, sizeof(int), 1, file);
	fread(&MatID, sizeof(int), 1, file);
	fread(&flags, sizeof(int), 1, file);
	fread(v, sizeof(int)*count, 1, file);
	vecs = NULL;

	if (flags & FLAG_CURVEDMAPPING)
		{
		vecs = new UVW_TVVectorClass;
		fread(vecs->handles, sizeof(int)*count*2, 1, file);
		fread(vecs->interiors, sizeof(int)*count, 1, file);
		fread(vecs->vhandles, sizeof(int)*count*2, 1, file);
		fread(vecs->vinteriors, sizeof(int)*count, 1, file);
		}
	return nb;
	}




ULONG UVW_ChannelClass::LoadFaces(ILoad *iload)
	{
	ULONG nb;
	for (int i=0; i < f.Count(); i++)
		{
		nb = f[i]->LoadFace(iload);
		}
	return nb;
	}

ULONG UVW_ChannelClass::LoadFaces(FILE *file)
	{
	ULONG nb;
	for (int i=0; i < f.Count(); i++)
		{
		nb = f[i]->LoadFace(file);
		}
	return nb;
	}


ULONG UVW_ChannelClass::SaveFaces(ISave *isave)
	{
	ULONG nb;
	for (int i=0; i < f.Count(); i++)
		{
		nb = f[i]->SaveFace(isave);
		}
	return nb;
	}

ULONG UVW_ChannelClass::SaveFaces(FILE *file)
	{
	ULONG nb;
	for (int i=0; i < f.Count(); i++)
		{
		nb = f[i]->SaveFace(file);
		}
	return nb;
	}


void UVW_ChannelClass::SetCountFaces(int newct)
	{
//delete existing data
	int ct = f.Count();
	for (int i =0; i < ct; i++)
		{
		if (f[i]->vecs) delete f[i]->vecs;
		if (f[i]->t) delete [] f[i]->t;
		if (f[i]->v) delete [] f[i]->v;
		f[i]->vecs = NULL;
		f[i]->t = NULL;
		f[i]->v = NULL;

		delete f[i];
		f[i] = NULL;
		}

	f.SetCount(newct);
	for (i =0; i < newct; i++)
		{
		f[i] = new UVW_TVFaceClass;
		f[i]->vecs = NULL;
		f[i]->t = NULL;
		f[i]->v = NULL;
		}


	}

void UVW_ChannelClass::CloneFaces(Tab<UVW_TVFaceClass*> &t)
	{
	int ct = f.Count();
	t.SetCount(ct);
	for (int i =0; i < ct; i++)
		t[i] = f[i]->Clone();
	}

void UVW_ChannelClass::AssignFaces(Tab<UVW_TVFaceClass*> &t)
	{
	//nuke old data and cassign clone of new
	int ct = f.Count();
	for (int i =0; i < ct; i++)
		{
		if (f[i]->vecs) delete f[i]->vecs;
		if (f[i]->t) delete [] f[i]->t;
		if (f[i]->v) delete [] f[i]->v;
		f[i]->vecs = NULL;
		f[i]->t = NULL;
		f[i]->v = NULL;

		delete f[i];
		f[i] = NULL;
		}
	ct = t.Count();
	f.SetCount(ct);
	for (i =0; i < ct; i++)
		f[i] = t[i]->Clone();
	}

void UVW_ChannelClass::FreeFaces()
	{
	int ct = f.Count();
	for (int i =0; i < ct; i++)
		{
		if (f[i]->vecs) delete f[i]->vecs;
		if (f[i]->t) delete [] f[i]->t;
		if (f[i]->v) delete [] f[i]->v;
		f[i]->vecs = NULL;
		f[i]->t = NULL;
		f[i]->v = NULL;

		delete f[i];
		f[i] = NULL;
		}

	}

void UVW_ChannelClass::Dump()
	{
	for (int i = 0; i < v.Count(); i++)
		{
		DebugPrint("Vert %d pt %f %f %f\n",i,v[i].p.x,v[i].p.y,v[i].p.z);
		}
	for (i = 0; i < f.Count(); i++)
		{
		DebugPrint("face %d t %d %d %d %d\n",i,f[i]->t[0],f[i]->t[1],f[i]->t[2],f[i]->t[3]);

		if (f[i]->vecs)
			DebugPrint("     int  %d %d %d %d\n",f[i]->vecs->interiors[0],f[i]->vecs->interiors[1],
										  f[i]->vecs->interiors[2],f[i]->vecs->interiors[3]
				   );
		
		if (f[i]->vecs)
			DebugPrint("     vec  %d,%d %d,%d %d,%d %d,%d\n",
										  f[i]->vecs->handles[0],f[i]->vecs->handles[1],
										  f[i]->vecs->handles[2],f[i]->vecs->handles[3],
										  f[i]->vecs->handles[4],f[i]->vecs->handles[5],
										  f[i]->vecs->handles[6],f[i]->vecs->handles[7]
				   );
		}
	}	

void UVW_ChannelClass::MarkDeadVertices()
	{
	BitArray usedVerts;
	usedVerts.SetSize(v.Count());
	usedVerts.ClearAll();

	for (int i =0; i < f.Count(); i++)
		{
		if (!(f[i]->flags & FLAG_DEAD))
			{
			for (int j=0; j < f[i]->count; j++)
				{
				int id = f[i]->t[j];
				if (id < usedVerts.GetSize()) usedVerts.Set(id);
				if ((f[i]->flags & FLAG_CURVEDMAPPING) && (f[i]->vecs))
					{
					id = f[i]->vecs->handles[j*2];
					if (id < usedVerts.GetSize()) usedVerts.Set(id);
					id = f[i]->vecs->handles[j*2+1];
					if (id < usedVerts.GetSize()) usedVerts.Set(id);
					if (f[i]->flags & FLAG_INTERIOR)
						{
						id = f[i]->vecs->interiors[j];
						if (id < usedVerts.GetSize()) usedVerts.Set(id);
						}
					}
	
				}
			}
		}

	for (i =0; i < v.Count(); i++)
		{
		if (i < usedVerts.GetSize())
			{
			if (!usedVerts[i])
				{
				v[i].flags |= FLAG_DEAD;
				}
			}
		
		}
	

	}






void VertexLookUpListClass::addPoint(int a_index, Point3 a)
	{	
	BOOL found = FALSE;
	if (sel[a_index]) found = TRUE;
	if (!found)
		{
		VertexLookUpDataClass t;
		t.index = a_index;
		t.newindex = a_index;
		t.p = a;
		d[a_index] = t;
		sel.Set(a_index);
		}
	};


void UVW_ChannelClass::FreeEdges()
	{
	for (int i =0; i < e.Count(); i++)
		{
		for (int j =0; j < e[i]->data.Count(); j++)
			delete e[i]->data[j];
		delete e[i];
		}
	e.ZeroCount();
	ePtrList.ZeroCount();
	}

void UVW_ChannelClass::BuildEdges()
	{
	FreeEdges();
	if (v.Count() != 0)
		edgesValid = TRUE;
	e.SetCount(v.Count());
	


	for (int i = 0; i < v.Count();i++)
		{
		e[i] = new UVW_TVEdgeClass();
		}

	for (i = 0; i < f.Count();i++)
		{
		if (!(f[i]->flags & FLAG_DEAD))
			{
			int pcount = 3;
			pcount = f[i]->count;
			int totalSelected = 0;
			for (int k = 0; k < pcount; k++)
				{
				
				int index1 = f[i]->t[k];
 
				int index2 = 0;
				if (k == (pcount-1))
					index2 = f[i]->t[0];
				else index2 = f[i]->t[k+1];
				int vec1=-1, vec2=-1;
				if ((f[i]->flags & FLAG_CURVEDMAPPING) &&(f[i]->vecs) )
					{
					vec1 = f[i]->vecs->handles[k*2];

					vec2 = f[i]->vecs->handles[k*2+1];
					}
				if (index2 < index1) 
					{
					int temp = index1;
					index1 = index2;
					index2 = temp;
					temp = vec1;
					vec1 = vec2;
					vec2 = temp;
					}
				BOOL hidden = FALSE;
				if (k==0)
					{
					if (f[i]->flags & FLAG_HIDDENEDGEA)
						hidden = TRUE;
					}
				else if (k==1)
					{
					if (f[i]->flags & FLAG_HIDDENEDGEB)
						hidden = TRUE;
					}
				else if (k==2)
					{
					if (f[i]->flags & FLAG_HIDDENEDGEC)
						hidden = TRUE;

					}
				AppendEdge(index1,vec1,index2,vec2,i,hidden);
				}	

			}
		}
	int ePtrCount = 0;
	for (i =0; i < e.Count(); i++)
		{
		ePtrCount += e[i]->data.Count();
		}
	ePtrList.SetCount(ePtrCount);
	int ct = 0;
	for (i =0; i < e.Count(); i++)
		{
		for (int j =0; j < e[i]->data.Count(); j++)
			ePtrList[ct++] = e[i]->data[j];
		}

	}

void UVW_ChannelClass::AppendEdge(int index1,int vec1, int index2,int vec2, int face, BOOL hidden)
	{
	if (e[index1]->data.Count() == 0)
		{
		UVW_TVEdgeDataClass *temp = new UVW_TVEdgeDataClass();
		temp->a = index1;
		temp->avec = vec1;
		temp->b = index2;
		temp->bvec = vec2;
		temp->flags = 0;
		if (hidden)
			temp->flags |= FLAG_HIDDENEDGEA;
		temp->faceList.Append(1,&face,4);
		e[index1]->data.Append(1,&temp,4);


		}
	else
		{
		BOOL found = FALSE;
		for (int i =0; i < e[index1]->data.Count(); i++)
			{
			if ( (e[index1]->data[i]->b == index2) && (e[index1]->data[i]->bvec == vec2))
				{
				found = TRUE;
				e[index1]->data[i]->faceList.Append(1,&face,4);

				if ((!hidden) && (e[index1]->data[i]->flags & FLAG_HIDDENEDGEA) )
					e[index1]->data[i]->flags &= ~FLAG_HIDDENEDGEA;

				i = e[index1]->data.Count();
				}
			}
		if (!found)
			{
			UVW_TVEdgeDataClass *temp = new UVW_TVEdgeDataClass();
			temp->a = index1;
			temp->avec = vec1;
			temp->b = index2;
			temp->bvec = vec2;
			temp->flags = 0;
			if (hidden)
				temp->flags |= FLAG_HIDDENEDGEA;
			temp->faceList.Append(1,&face,4);
			e[index1]->data.Append(1,&temp,4);
			}

		}

	}


float UVW_ChannelClass::LineToPoint(Point3 p1, Point3 l1, Point3 l2)
{
Point3 VectorA,VectorB,VectorC;
double Angle;
double dist = 0.0f;
VectorA = l2-l1;
VectorB = p1-l1;
float dot = DotProd(Normalize(VectorA),Normalize(VectorB));
if (dot == 1.0f) dot = 0.99f;
Angle =  acos(dot);
if (Angle > (3.14/2.0))
	{
	dist = Length(p1-l1);
	}
else
	{
	VectorA = l1-l2;
	VectorB = p1-l2;
	dot = DotProd(Normalize(VectorA),Normalize(VectorB));
	if (dot == 1.0f) dot = 0.99f;
	Angle = acos(dot);
	if (Angle > (3.14/2.0))
		{
		dist = Length(p1-l2);
		}
	else
		{
		double hyp;
		hyp = Length(VectorB);
		dist =  sin(Angle) * hyp;

		}

	}

return (float) dist;

}


int UVW_ChannelClass::EdgeIntersect(Point3 p, float threshold, int i1,int i2)
{

 static int startEdge = 0;
 if (startEdge >= ePtrList.Count()) startEdge = 0;
 if (ePtrList.Count() == 0) return -1;

 int ct = 0;
 BOOL done = FALSE;


 int hitEdge = -1;
 while (!done) 
	{
 //check bounding volumes

	Box3 bounds;
	bounds.Init();

	int index1 = ePtrList[startEdge]->a;
	int index2 = ePtrList[startEdge]->b;
	if ((v[index1].flags & FLAG_HIDDEN) &&  (v[index2].flags & FLAG_HIDDEN))
		{
		}
	else if ((v[index1].flags & FLAG_FROZEN) &&  (v[index1].flags & FLAG_FROZEN)) 
		{
		}
	else
		{
		Point3 p1(0.0f,0.0f,0.0f);
		p1[i1] = v[index1].p[i1];
		p1[i2] = v[index1].p[i2];
//		p1.z = 0.0f;
		bounds += p1;
		Point3 p2(0.0f,0.0f,0.0f);
		p2[i1] = v[index2].p[i1];
		p2[i2] = v[index2].p[i2];
//		p2.z = 0.0f;
		bounds += p2;
		bounds.EnlargeBy(threshold);
		if (bounds.Contains(p))
			{
//check edge distance
			if (LineToPoint(p, p1, p2) < threshold)
				{
				hitEdge = startEdge;
				done = TRUE;
//				LineToPoint(p, p1, p2);
				}
			}
		}
 	ct++;
	startEdge++;
	if (ct == ePtrList.Count()) done = TRUE;
	if (startEdge >= ePtrList.Count()) startEdge = 0;
	}
 
 return hitEdge;
}




void	UnwrapMod::RebuildEdges()
	{
	if (mode == ID_SKETCHMODE)
		SetMode(ID_UNWRAP_MOVE);

	BitArray holdVSel(vsel);

	BOOL holdSyncMode = fnGetSyncSelectionMode();
	fnSetSyncSelectionMode(FALSE);

	if (esel.GetSize() > 0)
		{
		theHold.Suspend();
		fnEdgeToVertSelect();
		theHold.Resume();
		}

	TVMaps.BuildEdges(  );
	if (esel.GetSize() != TVMaps.ePtrList.Count())
		{
		esel.SetSize(TVMaps.ePtrList.Count());
		esel.ClearAll();
		}

	if (esel.GetSize() > 0)
		{
		theHold.Suspend();
		fnVertToEdgeSelect();
		theHold.Resume();
		}


	vsel = holdVSel;

	fnSetSyncSelectionMode(holdSyncMode);

	usedVertices.SetSize(TVMaps.v.Count());
	usedVertices.ClearAll();

	for (int i = 0; i < TVMaps.f.Count(); i++)
		{
		int faceIndex = i;
		for (int k = 0; k < TVMaps.f[faceIndex]->count; k++)
			{
			if (!(TVMaps.f[faceIndex]->flags & FLAG_DEAD))
				{
				int vertIndex = TVMaps.f[faceIndex]->t[k];
				usedVertices.Set(vertIndex);
				if (objType == IS_PATCH)
					{
					if ((TVMaps.f[faceIndex]->flags & FLAG_CURVEDMAPPING) && (TVMaps.f[faceIndex]->vecs))
						{
						if (TVMaps.f[faceIndex]->flags & FLAG_INTERIOR)
							{
							vertIndex = TVMaps.f[faceIndex]->vecs->interiors[k];
							if ((vertIndex >=0) && (vertIndex < usedVertices.GetSize()))
								usedVertices.Set(vertIndex);
							}
						vertIndex = TVMaps.f[faceIndex]->vecs->handles[k*2];
						if ((vertIndex >=0) && (vertIndex < usedVertices.GetSize()))
							usedVertices.Set(vertIndex);
						vertIndex = TVMaps.f[faceIndex]->vecs->handles[k*2+1];
						if ((vertIndex >=0) && (vertIndex < usedVertices.GetSize()))
							usedVertices.Set(vertIndex);
						}
					}
				}
			}
		}



	}
