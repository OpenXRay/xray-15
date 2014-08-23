#include "SolidifyPW.h"


Point3 BuildMeshInfo::GetOutlineDir(Point3 a, Point3 b)
{
	Point3 vec(1.0f,0.0f,0.0f);
	Point3 xVec = Normalize(b-a);
	Point3 zVec(0.0f,0.0f,1.0f);
	vec = Normalize(CrossProd(xVec,zVec));
	return vec;

}

void BuildMeshInfo::BuildShapeData(PolyLine *shape)
{
	shapeData.SetCount(shape->numPts);

	Point3 vec = (shape->pts[shape->numPts-1].p - shape->pts[0].p);
	float uLen = (vec.y);


	for (int i = 0; i < shapeData.Count(); i++)
	{
		Point3 p = shape->pts[i].p;
//DebugPrint("Point %d  %f %f %f\n",i,p.x,p.y,p.z);
		if (fabs(uLen) <= 0.0001f)
		{
			shapeData[i].uPer = 0.0f;
			shapeData[i].vPer = 0.0f;
		}
		else
		{
			shapeData[i].uPer = (shape->pts[i].p.y-shape->pts[0].p.y)/uLen;
			shapeData[i].vPer = (shape->pts[i].p.x-shape->pts[0].p.x)/uLen * -1.0f;
		}
	}

}

void BuildMeshInfo::BuildData(Mesh *mesh,PolyLine *shape, BOOL fixupEdges)
{
	if (shape)
		{
		BuildShapeData(shape);
		}

	mesh->RemoveDegenerateFaces() ;

	
	AdjEdgeList ae(*mesh);

	Face *of = mesh->faces;


	for (int j = 0; j < ae.edges.Count(); j++)
		{
		int a,b;
		a = ae.edges[j].f[0];
		b = ae.edges[j].f[1];
		if ((a == -1) && (b==-1))
			{
			}
		else if ((a==-1) || (b==-1))
			{
			int va, vb;
			va = ae.edges[j].v[0];
			vb = ae.edges[j].v[1];
			SolidifyEdge e;
			e.a = va;
			e.b = vb;
			if (a == -1)
				e.ownerFace = b;
			else e.ownerFace = a;

			e.length = Length(mesh->verts[va]-mesh->verts[vb]);

			e.flip = FALSE;

			Face f = of[e.ownerFace];

			if ((of[e.ownerFace].v[0] == e.a) &&  (of[e.ownerFace].v[1] == e.b))
				{
				e.a = vb;
				e.b = va;
				e.ithVertA = 0;
				e.ithVertB = 1;

				}
			else if ((of[e.ownerFace].v[1] == e.a) &&  (of[e.ownerFace].v[2] == e.b))
				{
				e.a = vb;
				e.b = va;
				e.ithVertA = 1;
				e.ithVertB = 2;
				}
			else if ((of[e.ownerFace].v[2] == e.a) &&  (of[e.ownerFace].v[0] == e.b))
				{
				e.a = vb;
				e.b = va;
				e.ithVertA = 2;
				e.ithVertB = 0;
				}
			else if ((of[e.ownerFace].v[1] == e.a) &&  (of[e.ownerFace].v[0] == e.b))
				{
				e.ithVertA = 0;
				e.ithVertB = 1;
				}
			else if ((of[e.ownerFace].v[2] == e.a) &&  (of[e.ownerFace].v[1] == e.b))
				{
				e.ithVertA = 1;
				e.ithVertB = 2;
				}
			else if ((of[e.ownerFace].v[0] == e.a) &&  (of[e.ownerFace].v[2] == e.b))
				{
				e.ithVertA = 2;
				e.ithVertB = 0;
				}



			sEdges.Append(1,&e,500);
			}
		}

	int initialNumberFaces, initialNumberVertices;
	initialNumberFaces = mesh->numFaces;
	initialNumberVertices= mesh->numVerts;

	//compute our vertex normals
	
	//our bevel normals
//	Tab<Point3> bnorms;

	if (shape)
	{
		bevelData.SetCount(mesh->numVerts);
		for (j = 0; j < bevelData.Count(); j++)
			bevelData[j].ct = 0;

		for (j = 0; j < ae.edges.Count(); j++)
		{
			int a,b;
			a = ae.edges[j].v[0];
			b = ae.edges[j].v[1];
			int fa,fb;
			fa = ae.edges[j].f[0];
			fb = ae.edges[j].f[1];

			if ((fa == -1) || (fb == -1))
			{

				int faceID = fa;
				if (faceID == -1)
					faceID = fb;

				int ithA;
				Face f = mesh->faces[faceID];
				for (int k = 0; k < 3; k++)
				{
					int index = f.v[k];
					if (index == a)
						ithA = k;
				}
				int ithB = ithA+1;
				if (ithB >= 3) ithB = 0;

				BOOL flip = FALSE;
				if (mesh->faces[faceID].v[ithB] == b)
				{
				}
				else
				{
					flip = TRUE;
					int temp;
					temp = a;
					a = b;
					b = temp;
				}



				Point3 pa,pb;
				pa = mesh->verts[a];
				pb = mesh->verts[b];
				Point3 vecAtoB = pb-pa;
				Point3 vecBtoA = pb-pa;

				if (flip)
				{
//					vecAtoB = pa-pb;
//					vecBtoA = pa-pb;

				}

				bevelData[a].p = pa;
				bevelData[a].vec[bevelData[a].ct] = Normalize(vecAtoB);
				bevelData[a].ct++;

				bevelData[b].p = pb;
				bevelData[b].vec[bevelData[b].ct] = Normalize(vecBtoA);
				bevelData[b].ct++;
			}
		}

	}



	
	realLengths.SetCount(mesh->numVerts);
	for (j = 0; j < mesh->numVerts; j++)
		realLengths[j] = 1.0f;

	Tab<WeightedNormals> cornerAngles;

	cornerAngles.SetCount(mesh->numFaces);

	fnorms.SetCount(mesh->numFaces);

	Tab<int> vcount;
	vnorms.SetCount(mesh->numVerts);
	op.SetCount(mesh->numVerts);
	vcount.SetCount(mesh->numVerts);
	for (j = 0; j < mesh->numVerts; j++)
		{
		vnorms[j] = Point3(0.0f,0.0f,0.0f);
		vcount[j] = 0;
		op[j] = mesh->verts[j];
		}

	Tab<int> closestFaceNorm;
	Tab<float> closestFaceAngle;

	closestFaceNorm.SetCount(mesh->numVerts);
	closestFaceAngle.SetCount(mesh->numVerts);


	for (j = 0; j < mesh->numVerts; j++)
		{
		closestFaceNorm[j] = -1;
		closestFaceAngle[j] = PI*3;
		}

	Tab<int> degFaces;
//compute our face norms
	for (j = 0; j < mesh->numFaces; j++)
		{
		Point3 norm(0.0f,0.0f,0.0f);
		Point3 vecA,vecB,vecC;
		Point3 a,b,c;
		int ia,ib,ic;
		ia =mesh->faces[j].v[0];
		ib =mesh->faces[j].v[1];
		ic =mesh->faces[j].v[2];
		a = mesh->verts[ia];
		b = mesh->verts[ib];
		c = mesh->verts[ic];
		vecA = Normalize(a - b);
		vecB = Normalize(c - b);
		vecC = Normalize(c - a);


			norm = Normalize(CrossProd(vecA,vecB));

			fnorms[j] = norm;

			Point3 vecAB = vecA * -1.0f;
			Point3 vecBA = vecA;
			Point3 vecBC = vecB;
			Point3 vecCB = vecB * -1.0f;
			Point3 vecCA = vecC * -1.0f;
			Point3 vecAC = vecC;

			float dA = DotProd(vecAB,vecAC);
			float dB = DotProd(vecBA,vecBC);
			float dC = DotProd(vecCA,vecCB);

			float aA = acos(dA);
			float aB = acos(dB);
			float aC = acos(dC);

			if ((aA == 0.0f) || (aB == 0.0f) || (aC == 0.0f))
				{
				degFaces.Append(1,&j);
				fnorms[j] = Point3(0.0f,0.0f,0.0f);

				}
/*
			else
				{

				cornerAngles[j].a = aA;//acos(DotProd(vecAB,vecAC));
				cornerAngles[j].b = aB;//acos(DotProd(vecBA,vecBC));
				cornerAngles[j].c = aC;//acos(DotProd(vecCA,vecCB));


				vnorms[ia] += norm * cornerAngles[j].a/(PI*2);
				vnorms[ib] += norm * cornerAngles[j].b/(PI*2);
				vnorms[ic] += norm * cornerAngles[j].c/(PI*2);
				}

			}
	*/



		}

	if (degFaces.Count())
		{
		//we are dorked since we have sliver faces
		AdjFaceList adjFace(*mesh,ae);	

		BOOL done = FALSE;
		int numberDegFaces = degFaces.Count();
		while (!done)
			{

			for (j = 0; j < degFaces.Count(); j++)
				{
				Point3 norm(0.0f,0.0f,0.0f);
				int normCT = 0;
				int faceIndex = degFaces[j];

				for (int k = 0; k < 3; k++)
					{
					int neighbor = adjFace.list[faceIndex].f[k];
					if (neighbor != -1)
						{
						if (fnorms[neighbor] != Point3(0.0f,0.0f,0.0f))
							{
							normCT++;
							norm += fnorms[neighbor];
							}	
						}
					}	
				if (normCT>0)
					{
					Point3 n = Normalize(norm/(float)normCT);
					fnorms[faceIndex] = n;
					degFaces.Delete(j,1);
					j--;
					}
				}
			if (numberDegFaces == degFaces.Count())
				done = TRUE;
			numberDegFaces = degFaces.Count();
			}
		}

	for (j = 0; j < mesh->numFaces; j++)
		{
		Point3 vecA,vecB,vecC;
		Point3 a,b,c;
		int ia,ib,ic;
		ia =mesh->faces[j].v[0];
		ib =mesh->faces[j].v[1];
		ic =mesh->faces[j].v[2];
		a = mesh->verts[ia];
		b = mesh->verts[ib];
		c = mesh->verts[ic];
		vecA = Normalize(a - b);
		vecB = Normalize(c - b);
		vecC = Normalize(c - a);



		Point3 vecAB = vecA * -1.0f;
		Point3 vecBA = vecA;
		Point3 vecBC = vecB;
		Point3 vecCB = vecB * -1.0f;
		Point3 vecCA = vecC * -1.0f;
		Point3 vecAC = vecC;

		float dA = DotProd(vecAB,vecAC);
		float dB = DotProd(vecBA,vecBC);
		float dC = DotProd(vecCA,vecCB);

		float aA = acos(dA);
		float aB = acos(dB);
		float aC = acos(dC);


		cornerAngles[j].a = aA;//acos(DotProd(vecAB,vecAC));
		cornerAngles[j].b = aB;//acos(DotProd(vecBA,vecBC));
		cornerAngles[j].c = aC;//acos(DotProd(vecCA,vecCB));

		Point3 norm = fnorms[j];

		vnorms[ia] += norm * cornerAngles[j].a/(PI*2);
		vnorms[ib] += norm * cornerAngles[j].b/(PI*2);
		vnorms[ic] += norm * cornerAngles[j].c/(PI*2);

		}


	for (j = 0; j < mesh->numVerts; j++)
		{
		vnorms[j] = Normalize(vnorms[j]);
		}

	for (j = 0; j < mesh->numFaces; j++)
		{

		Point3 fnorm = fnorms[j];
		for (int k = 0; k < 3; k++)
			{
			int vindex = mesh->faces[j].v[k];
			Point3 vnorm = vnorms[vindex];
			float angle = acos(DotProd(fnorm,vnorm));
			if (fabs(angle) < closestFaceAngle[vindex])
				{
				closestFaceAngle[vindex] = fabs(angle);
				closestFaceNorm[vindex] = j;
				}
			}


		}

	for (j = 0; j < mesh->numVerts; j++)
		{
		vnorms[j] = vnorms[j];
		}

	


	if (fixupEdges)
		{

		vPlanes.SetCount(mesh->numVerts);
		for (j = 0; j < mesh->numVerts; j++)
			{
			vPlanes[j].ct = 0;
			vPlanes[j].isEdgeVert = FALSE;
			vPlanes[j].display = FALSE;
			vPlanes[j].vnorm = vnorms[j] * -1.0f;
			}
//loop through our faces
		for (j = 0; j < mesh->numFaces; j++)
			{
			Point3 a,b,c;
			a = mesh->verts[mesh->faces[j].v[0]];
			b = mesh->verts[mesh->faces[j].v[1]];
			c = mesh->verts[mesh->faces[j].v[2]];
			for (int k = 0; k < 3; k++)
				{
				int vindex = mesh->faces[j].v[k];
				//get the face norm
				Point3 faceNorm = fnorms[j]*-1.0f;
				//add it to our vertplances if it does not exist
				Point3 p = mesh->verts[vindex]+faceNorm;
//				if (vindex == 4)
//					DebugPrint("wait\n");
				vPlanes[vindex].AddPlane(p,faceNorm,a,b,c);
				vPlanes[vindex].op = mesh->verts[vindex];				
				//get 
				}
			}

		AdjEdgeList e(*mesh);
		for (j = 0; j < e.edges.Count(); j++)
			{
			int faceA, faceB;
			faceA = e.edges[j].f[0];
			faceB = e.edges[j].f[1];
			int vertA, vertB;
			vertA = e.edges[j].v[0];
			vertB = e.edges[j].v[1];

//			faceA = 0;
//			faceB = 0;

			if ((faceA ==-1) || (faceB == -1))
				{
				vPlanes[vertA].isEdgeVert = TRUE;
				vPlanes[vertB].isEdgeVert = TRUE;
				vPlanes[vertA].ct = 0;
				vPlanes[vertB].ct = 0;
				}
			}
		for (j = 0; j < e.edges.Count(); j++)
			{
			int faceA, faceB;
			faceA = e.edges[j].f[0];
			faceB = e.edges[j].f[1];
			int vertA, vertB;
			vertA = e.edges[j].v[0];
			vertB = e.edges[j].v[1];

//			faceA = 0;
//			faceB = 0;

			if (faceA ==-1) 
				{
				int faceIndex = faceB;

				Point3 a,b,c;
				a = mesh->verts[vertA];
				b = mesh->verts[vertB];
				c = mesh->verts[mesh->faces[faceIndex].v[2]];

				Point3 faceNorm = fnorms[faceIndex]*-1.0f;
				//add it to our vertplances if it does not exist
				Point3 p = mesh->verts[vertA]+faceNorm;
				vPlanes[vertA].AddPlane(p,faceNorm,a,b,c);

				p = mesh->verts[vertB]+faceNorm;
				vPlanes[vertB].AddPlane(p,faceNorm,a,b,c);
				}
			if (faceB ==-1) 
				{
				int faceIndex = faceA;

				Point3 a,b,c;
				a = mesh->verts[vertA];
				b = mesh->verts[vertB];
				c = mesh->verts[mesh->faces[faceIndex].v[2]];

				Point3 faceNorm = fnorms[faceIndex]*-1.0f;
				//add it to our vertplances if it does not exist
				Point3 p = mesh->verts[vertA]+faceNorm;
				vPlanes[vertA].AddPlane(p,faceNorm,a,b,c);

				p = mesh->verts[vertB]+faceNorm;
				vPlanes[vertB].AddPlane(p,faceNorm,a,b,c);
				}

			}


//solve our planes
		for (j = 0; j < mesh->numVerts; j++)
			{

			vPlanes[j].BuildPlaneEquations();
			if ((vPlanes[j].ct == 3) || 
				((vPlanes[j].ct == 2) && (vPlanes[j].isEdgeVert)))
				vnorms[j] = vPlanes[j].vnorm;
			}

			



		//get an edge
		//get the 2 faces of that edge
		//get the normals

		BitArray processedVerts;
		processedVerts.SetSize(mesh->numVerts);
		processedVerts.ClearAll();

		cdata.SetCount(mesh->numVerts);
		
		for (j = 0; j < mesh->numFaces; j++)
			{
			for (int k = 0; k < 3; k++)
				{
				int vindex = mesh->faces[j].v[k];
				if (!processedVerts[vindex])
					{
					
					Point3 norm, hyp;

					hyp = vnorms[vindex];

					norm = vPlanes[vindex].planes[0].vec*-1.0f;

//					norm = fnorms[faceID]*-1.0f;
//					norm = fnorms[j]*-1.0f;

					cdata[vindex].p = mesh->verts[vindex];




					float dot = DotProd(hyp,norm);

					realLengths[vindex] = 1.0f;
					
					float d = fabs(1.0f+dot);
					if ((fabs(1.0f-dot) <= 0.001f) || (fabs(dot) < 0.001f))
						{
						}
					else if (d <= 0.0001f)
						{
 						realLengths[vindex] = -1.0f;
						}
					else
						{
		 				float angle = acos(dot);
		 				if (!_isnan(angle))
		 					{
							if (angle == 0.0f)
								realLengths[vindex] = 1.0f;
							else 
								{
								float len = 1.0f/cos(angle);
								realLengths[vindex] = len;
								}
							}
						}

					cdata[vindex].norm = norm ;
					cdata[vindex].hyp = hyp  * realLengths[vindex];


					processedVerts.Set(vindex);
					}
				}
			}
		}
//build our bevel normals
	if (shape)
	{
		for (j = 0; j < bevelData.Count(); j++)
		{
		//get our vnorm
			if (bevelData[j].ct == 2)
			{
				Point3 norm = Normalize(vnorms[j]* realLengths[j]);
			//get our vecs
				Point3 vecA = bevelData[j].vec[0];
				Point3 vecB = bevelData[j].vec[1];
				vecA = Normalize(CrossProd(norm,vecA));
				vecB = Normalize(CrossProd(norm,vecB));
				bevelData[j].cross[0] = vecA;
				bevelData[j].cross[1] = vecB;
			//crossprod them

			//avergae them to get our bnorm
				Point3 bnorm = (vecA+vecB);
				bnorm *=0.5f;
//2.0f;
				bevelData[j].norm = Normalize(bnorm);
				bevelData[j].vnorm = vnorms[j];

				bevelData[j].realLength = 1.0f;

			 	float dot = DotProd(bevelData[j].norm,vecA);

				if ((fabs(1.0f-dot) <= 0.0001f) || (fabs(dot) < 0.00001f))
				{
				}
				else
				{
	 				float angle = acos(dot);
					if (angle == 0.0f)
						bevelData[j].realLength = 1.0f;
					else 
					{
						float len = 1.0f/cos(angle);
						bevelData[j].realLength = len;
					}
				}



			}
		}
	}

}


void BuildMeshInfo::MakeSolid(Mesh *mesh, int segments, float amount, float oamount, int matid, int sg, int edgemap, float tvOffset,int innerMatID,int outerMatID,
					  BOOL selEdges, BOOL selFaces,BOOL selOuterFaces, 
					  BOOL fixupEdges,
					  BOOL autoSmooth, float autoSmoothAngle,PolyLine *shape)
{

	if (shape)
		segments = shape->numPts-1;
		
	if (sg != 0)
		{
		sg--;		
		sg = 1 << sg;
		}

	BuildData(mesh,shape,fixupEdges);

	int initialNumberFaces, initialNumberVertices;
	initialNumberFaces = mesh->numFaces;
	initialNumberVertices= mesh->numVerts;


	//compute the number of vertices	
	//2*numberofVertices;
	mesh->setNumVerts(initialNumberVertices*2 + (segments-1)*(sEdges.Count()), TRUE);

	Point3 *p = mesh->verts;
	//copy our vertices and shift them along the normals neg value
	int startVert = initialNumberVertices;

	Tab<float> dist;
	dist.SetCount(initialNumberVertices);

	Tab<Point3> innerList;
	Tab<Point3> outerList;
	innerList.SetCount(initialNumberVertices);
	outerList.SetCount(initialNumberVertices);
	for (int j = 0; j < initialNumberVertices; j++)
		{
			p[startVert] = p[j] + vnorms[j] * (amount) * realLengths[j];
			p[j] = p[j] + vnorms[j] * (oamount*-1.0f) * realLengths[j];
			innerList[j] = p[startVert];
			outerList[j] = p[j];
			dist[j] = Length(p[startVert] - p[j]);
			startVert++;
			
		}

	Tab <int> lookup;
	lookup.SetCount(mesh->numVerts);
	for (j = 0; j < mesh->numVerts; j++)
		lookup[j] = -1;

	//this does our edge vertices
	for (int k = 0; k < (segments-1); k++)
		{
		float per = (float) (k+1)/(segments);
		for (j = 0; j < sEdges.Count(); j++)
			{


			SolidifyEdge *se = sEdges.Addr(j);
			int index = sEdges[j].a;
			se->aIndexToEList = j;
			lookup[index] = j;

			float d= dist[index];

			if (shape)
			{
				int shapeIndex = (k+1);
				float uPer = shapeData[shapeIndex].uPer;
				float vPer = shapeData[shapeIndex].vPer;
				Point3 vVec = bevelData[index].norm;
				float edgedist = amount + oamount;
				vVec = vVec * edgedist/*Length((outerList[index]-innerList[index]))*/*vPer * bevelData[index].realLength;

				p[startVert] = outerList[index] - (outerList[index]-innerList[index]) * uPer;
				p[startVert] += vVec;

			}
		 	else p[startVert] = outerList[index] - (outerList[index]-innerList[index]) * per;

//		 	p[startVert] = p[index] + vnorms[index] * ((amount+oamount) * realLengths[j]) * per;

			startVert++;
			}

		}

	if (segments > 1)
		{
		for (j = 0; j < sEdges.Count(); j++)
			{
			SolidifyEdge *se = sEdges.Addr(j);
			
			int index = se->b;
			if (index == -1)
				DebugPrint("error\n");
			else se->bIndexToEList = lookup[index];

			}

		}

	
	//compute the number of faces
	//2*numberofVertices + number of open edges *2;

//	segments = 1;

	initialNumberFaces = mesh->numFaces;
	mesh->setNumFaces(initialNumberFaces*2 + sEdges.Count()*2*segments, TRUE);

	int startFace = initialNumberFaces;
	Face *f = mesh->faces;
	for (j = 0; j < initialNumberFaces; j++)
		{
		f[startFace] = f[j];
		f[startFace].v[0]  = f[j].v[2]+initialNumberVertices;
		f[startFace].v[1]  = f[j].v[1]+initialNumberVertices;
		f[startFace].v[2]  = f[j].v[0]+initialNumberVertices;

		if (f[j].getEdgeVis(0))
			f[startFace].setEdgeVis(1,EDGE_VIS);
		else f[startFace].setEdgeVis(1,EDGE_INVIS);

		if (f[j].getEdgeVis(1))
			f[startFace].setEdgeVis(0,EDGE_VIS);
		else f[startFace].setEdgeVis(0,EDGE_INVIS);

		if (f[j].getEdgeVis(2))
			f[startFace].setEdgeVis(2,EDGE_VIS);
		else f[startFace].setEdgeVis(2,EDGE_INVIS);

		if (innerMatID >= 0) 
			f[startFace].setMatID(innerMatID);




		startFace++;
		}

//do our edge faces now
	startFace = initialNumberFaces*2;
	for (j = 0; j < sEdges.Count(); j++)
		{
		for (int k = 0; k < segments; k++)
			{

			int a,b;
			int a2,b2;

			if (k == 0)
				{
				a = sEdges[j].a ;
				b = sEdges[j].b ;
				}
			else
				{
				a = (initialNumberVertices*2) + ((k-1) * sEdges.Count()) + sEdges[j].aIndexToEList;
				b = (initialNumberVertices*2) + ((k-1) * sEdges.Count()) + sEdges[j].bIndexToEList;
				}

			if (k == (segments -1))
				{
				a2 = sEdges[j].a + initialNumberVertices ;
				b2 = sEdges[j].b + initialNumberVertices ;
				}
			else
				{
				a2 = (initialNumberVertices*2) + (k * sEdges.Count()) + sEdges[j].aIndexToEList;
				b2 = (initialNumberVertices*2) + (k * sEdges.Count())+ sEdges[j].bIndexToEList;

				}
			if (a >= mesh->numVerts)
				DebugPrint("stop\n");
			if (a2 >= mesh->numVerts)
				DebugPrint("stop\n");

			if (b >= mesh->numVerts)
				DebugPrint("stop\n");
			if (b2 >= mesh->numVerts)
				DebugPrint("stop\n");

			f[startFace] = f[sEdges[j].ownerFace];
			f[startFace].v[0] = a;
			f[startFace].v[1] = b;
			f[startFace].v[2] = b2;
			
			f[startFace].setEdgeVis(0,EDGE_VIS);
			f[startFace].setEdgeVis(1,EDGE_VIS);
			f[startFace].setEdgeVis(2,EDGE_INVIS);
			if (matid != -1)
				f[startFace].setMatID(matid);
			if (sg != -1)
				f[startFace].setSmGroup(sg);

			if (autoSmooth)
				f[startFace].setSmGroup(32);

			sEdges[j].tvFaceA = startFace; 

			startFace++;


			f[startFace] = f[sEdges[j].ownerFace];
			f[startFace].v[0] = b2;
			f[startFace].v[1] = a2;
			f[startFace].v[2] = a;
			
			f[startFace].setEdgeVis(0,EDGE_VIS);
			f[startFace].setEdgeVis(1,EDGE_VIS);
			f[startFace].setEdgeVis(2,EDGE_INVIS);
			if (matid != -1)
				f[startFace].setMatID(matid);
			if (sg != -1)
				f[startFace].setSmGroup(sg);

			if (autoSmooth)
				f[startFace].setSmGroup(32);


			sEdges[j].tvFaceB = startFace; 

			startFace++;
			}

		}


	for (j = 0; j < initialNumberFaces; j++)
		{
		if (outerMatID >= 0) 
			f[j].setMatID(outerMatID);
		}


	int numMaps = mesh->getNumMaps();


	for (int mp = 0; mp < numMaps; mp++)
		{

		if (!mesh->mapSupport(mp)) continue;

		if (edgemap == 0) //copy
			{
//now do all our textures
			int initialNumberTVVerts = mesh->getNumMapVerts(mp);

			if (initialNumberTVVerts > 0)
				{
 				mesh->setNumMapVerts(mp,initialNumberTVVerts,TRUE);

//do our tverts
				int startVert = initialNumberTVVerts;
				Point3 *uvw = mesh->mapVerts(mp);
			
				if (uvw)
					{
					for ( j = 0; j < initialNumberTVVerts; j++)
						{
//						uvw[startVert] = uvw[j];
//						startVert++;
						}
					}
				}

//do our tfaces
			int initialNumberTVFaces = initialNumberFaces;

			if (initialNumberTVFaces > 0)
				{

				int startFace = initialNumberTVFaces;
				TVFace *uvwFace = mesh->mapFaces(mp);
				Point3 *uvw = mesh->mapVerts(mp);

				if (uvwFace)
					{
					for ( j = 0; j < initialNumberTVFaces; j++)
						{
						uvwFace[startFace].t[0] = uvwFace[j].t[2];
						uvwFace[startFace].t[1] = uvwFace[j].t[1];
						uvwFace[startFace].t[2] = uvwFace[j].t[0];
						startFace++;
						}
				
					startFace = initialNumberFaces*2;


					for (j = 0; j < sEdges.Count(); j++)
						{			

						for (int k = 0; k < segments; k++)
							{
							int ownerFace,ithA,ithB;

							ithA = sEdges[j].ithVertA;
							ithB = sEdges[j].ithVertB;
							ownerFace = sEdges[j].ownerFace;

							int b = uvwFace[ownerFace].t[ithA];
							int a = uvwFace[ownerFace].t[ithB];

							uvwFace[startFace].t[0] = a;
							uvwFace[startFace].t[1] = b;
							uvwFace[startFace].t[2] = b;
							startFace++;

							uvwFace[startFace].t[0] = b;
							uvwFace[startFace].t[1] = a;
							uvwFace[startFace].t[2] = a;
							startFace++;
							}
						}

					}


				}

			}

		else if (edgemap == 1) //no edge mapping just copy and blow
			{
//now do all our textures
			int initialNumberTVVerts = mesh->getNumMapVerts(mp);

			if (initialNumberTVVerts > 0)
				{
				mesh->setNumMapVerts(mp,initialNumberTVVerts*2,TRUE);

//do our tverts
				int startVert = initialNumberTVVerts;
				Point3 *uvw = mesh->mapVerts(mp);
			
				if (uvw)
					{
					for ( j = 0; j < initialNumberTVVerts; j++)
						{
						uvw[startVert] = uvw[j];
						startVert++;
						}
					}
				}

//do our tfaces
			int initialNumberTVFaces = initialNumberFaces;

			if (initialNumberTVFaces > 0)
				{
//				mesh->setNumMapVerts(mp,initialNumberFaces*2 + sEdges.Count()*2,TRUE);

				int startFace = initialNumberTVFaces;
				TVFace *uvwFace = mesh->mapFaces(mp);

				if (uvwFace)
					{
					for ( j = 0; j < initialNumberTVFaces; j++)
						{
						uvwFace[startFace].t[0] = uvwFace[j].t[2];
						uvwFace[startFace].t[1] = uvwFace[j].t[1];
						uvwFace[startFace].t[2] = uvwFace[j].t[0];
						startFace++;
						}
				
					startFace = initialNumberFaces*2;
					
					for (j = 0; j < sEdges.Count(); j++)
						{			
						for (int k = 0; k < segments; k++)
							{
							uvwFace[startFace].t[0] = 0;
							uvwFace[startFace].t[1] = 0;
							uvwFace[startFace].t[2] = 0;
							startFace++;

							uvwFace[startFace].t[0] = 0;
							uvwFace[startFace].t[1] = 0;
							uvwFace[startFace].t[2] = 0;
							startFace++;
							}
						}

					}


				}

			}
		else if (edgemap == 2)  //strip edge mapping 
			{
			//build our loops
			Tab<EdgeLoop*> edgeLoops;
			BitArray usedEdges;
			usedEdges.SetSize(sEdges.Count());
			usedEdges.ClearAll();

		
			BOOL done = FALSE;
			int startEdge = 0;
			int currentEdge = 0;


			while ((!done) && (sEdges.Count() > 0))
				{
				BOOL lookingForEdge = TRUE;
				
				EdgeLoop *el = new EdgeLoop();
				el->loop.Append(1,&startEdge,50);
				usedEdges.Set(startEdge);
				
				while (lookingForEdge)
					{
					BOOL hit = FALSE;
					for (j = 0; j < sEdges.Count(); j++)
						{
						if (!usedEdges[j])
							{
							int a,b;
							a = sEdges[j].a;
							b = sEdges[j].b;
							if ( (a == sEdges[currentEdge].a) || (a == sEdges[currentEdge].b) ||
								 (b == sEdges[currentEdge].a) || (b == sEdges[currentEdge].b) )
								{
								if (j != startEdge)
									{
									currentEdge = j;
									usedEdges.Set(j);
									el->loop.Append(1,&j,50);
									j = sEdges.Count();
									hit = TRUE;
									
									}
								}
							}
						}
					if (!hit) 
						lookingForEdge = FALSE;
					}	
				
				edgeLoops.Append(1,&el);

				if (usedEdges.NumberSet() == sEdges.Count())//see if we have any edges left
					done = TRUE;
				else //look for a new start edge
					{
					for (j = 0; j < usedEdges.GetSize(); j++)
						{
						if (!usedEdges[j])
							{
							startEdge = j;
							currentEdge = j;
							
							j = usedEdges.GetSize();
							}
						}
					}
				}

//now do all our textures
			int initialNumberTVVerts = mesh->getNumMapVerts(mp);

			if (initialNumberTVVerts > 0)
				{
				int additionalVerts =0;

				for (j = 0; j < edgeLoops.Count(); j++)
					{
//					additionalVerts += edgeLoops[j]->loop.Count()*4;
					additionalVerts += edgeLoops[j]->loop.Count()+1;
					edgeLoops[j]->totalLength = 0.0f;
					for (int i = 0; i < edgeLoops[j]->loop.Count(); i++)
						{
						int eIndex = edgeLoops[j]->loop[i];
						edgeLoops[j]->totalLength += sEdges[eIndex].length;
						}
					}

//				mesh->setNumMapVerts(mp,initialNumberTVVerts*2+additionalVerts ,TRUE);
				mesh->setNumMapVerts(mp,initialNumberTVVerts*2+additionalVerts*2 *segments ,TRUE);

//do our tverts
				int startVert = initialNumberTVVerts;
				Point3 *uvw = mesh->mapVerts(mp);
			
				if (uvw)
					{
					for ( j = 0; j < initialNumberTVVerts; j++)
						{
						uvw[startVert] = uvw[j];
						startVert++;
						}
					}
				}

//do our tfaces
			int initialNumberTVFaces = initialNumberFaces;

			if (initialNumberTVFaces > 0)
				{
//				mesh->setNumMapVerts(mp,initialNumberFaces*2 + sEdges.Count()*2,TRUE);

				int startFace = initialNumberTVFaces;
				TVFace *uvwFace = mesh->mapFaces(mp);

				if (uvwFace)
					{
					for ( j = 0; j < initialNumberTVFaces; j++)
						{
						uvwFace[startFace].t[0] = uvwFace[j].t[2];
						uvwFace[startFace].t[1] = uvwFace[j].t[1];
						uvwFace[startFace].t[2] = uvwFace[j].t[0];
						startFace++;
						}
				// now build our other edge data

					//build vertex data for this loop
					int startVertex = initialNumberTVVerts*2;					
					Point3 *uvw = mesh->mapVerts(mp);


					
					for (j = 0; j < edgeLoops.Count(); j++)
						{

						int groupStart = startVertex;

						float currentLength = 0.0f;
						

						float u = 0.0f;
						
						for (k = 0; k < segments+1; k++)
							{
							float v = (float)k/(float)segments * tvOffset;
							Point3 p;
							p.x = u;
							p.y = v;
							p.z = 0.0f;

							uvw[startVertex] = p;
							startVertex++;
							}

						for (int i = 0; i < edgeLoops[j]->loop.Count(); i++)
							{

							int eIndex = edgeLoops[j]->loop[i];
							
							float u = 0.f;
							
							currentLength += sEdges[eIndex].length;

							u = currentLength/edgeLoops[j]->totalLength;							

							
							for (k = 0; k < segments+1; k++)
								{
								float v = (float)k/(float)segments * tvOffset;
								Point3 p;
								p.x = u;
								p.y = v;
								p.z = 0.0f;

								uvw[startVertex] = p;
								startVertex++;
								}
							}


						int ia,ib,ia2,ib2;
						
						int firstEdge = edgeLoops[j]->loop[0];
						int secondEdge = edgeLoops[j]->loop[1];
						BOOL isBackwards = TRUE;

						
						if (sEdges[firstEdge].b ==  sEdges[secondEdge].a)
							isBackwards = FALSE;


						if (isBackwards)
							{
							int starti = 0;
							for (i = (edgeLoops[j]->loop.Count()-1); i >= 0; i--)
								{
								int eIndex = edgeLoops[j]->loop[i];


								int faceIndex = startFace +eIndex * 2 * segments ;
								for (k = 0; k < segments; k++)
									{
									ia = groupStart + (starti*(segments+1)) + k;
									ib = groupStart + (starti*(segments+1)) + k + segments+1;
									ia2 = ia+1;
									ib2 = ib+1;
									uvwFace[faceIndex].t[0] = ia;
									uvwFace[faceIndex].t[1] = ib;
									uvwFace[faceIndex].t[2] = ib2;
									faceIndex++;

									uvwFace[faceIndex].t[0] = ib2;
									uvwFace[faceIndex].t[1] = ia2;
									uvwFace[faceIndex].t[2] = ia;
									faceIndex++;
									}
								starti++;
								}
							}
						else
							{
							for (i = 0; i < edgeLoops[j]->loop.Count(); i++)
								{
								int eIndex = edgeLoops[j]->loop[i];


								int faceIndex = startFace +eIndex * 2 * segments ;
								for (k = 0; k < segments; k++)
									{
									ia = groupStart + (i*(segments+1)) + k;
									ib = groupStart + (i*(segments+1)) + k + segments+1;
									ia2 = ia+1;
									ib2 = ib+1;
									uvwFace[faceIndex].t[0] = ia;
									uvwFace[faceIndex].t[1] = ib;
									uvwFace[faceIndex].t[2] = ib2;
									faceIndex++;

									uvwFace[faceIndex].t[0] = ib2;
									uvwFace[faceIndex].t[1] = ia2;
									uvwFace[faceIndex].t[2] = ia;
									faceIndex++;
									}
								}
							}


							
						}


					}


				}
			for (j = 0; j < edgeLoops.Count(); j++)
				delete edgeLoops[j];



			}
		else if (edgemap == 3)  //interpolate edge mapping 
			{
//now do all our textures
			int initialNumberTVVerts = mesh->getNumMapVerts(mp);

			if (initialNumberTVVerts > 0)
				{
				mesh->setNumMapVerts(mp,initialNumberTVVerts*2+sEdges.Count()*2*segments,TRUE);

//do our tverts
				int startVert = initialNumberTVVerts;
				Point3 *uvw = mesh->mapVerts(mp);
			
				if (uvw)
					{
					for ( j = 0; j < initialNumberTVVerts; j++)
						{
						uvw[startVert] = uvw[j];
						startVert++;
						}
					}
				}

//do our tfaces
			int initialNumberTVFaces = initialNumberFaces;

			if (initialNumberTVFaces > 0)
				{

				int startFace = initialNumberTVFaces;
				TVFace *uvwFace = mesh->mapFaces(mp);
				Point3 *uvw = mesh->mapVerts(mp);

				if (uvwFace)
					{
					for ( j = 0; j < initialNumberTVFaces; j++)
						{
						uvwFace[startFace].t[0] = uvwFace[j].t[2]+initialNumberTVVerts;
						uvwFace[startFace].t[1] = uvwFace[j].t[1]+initialNumberTVVerts;
						uvwFace[startFace].t[2] = uvwFace[j].t[0]+initialNumberTVVerts;
						startFace++;
						}
				
					startFace = initialNumberFaces*2;
					int startVert = initialNumberTVVerts*2;

					Tab<AverageDirection*> averageList;

					averageList.SetCount(initialNumberTVVerts*2+sEdges.Count()*2*segments);

					for (j = 0; j < averageList.Count(); j++)
						{
						averageList[j] = new AverageDirection();						
						averageList[j]->a.SetCount(segments);
						for (int k = 0; k < segments; k++)
							{
							averageList[j]->a[k].a = -1;
							averageList[j]->a[k].b = -1;
							}

						}

					for (j = 0; j < sEdges.Count(); j++)
						{			

						int ownerFace,ithA,ithB;

						ithA = sEdges[j].ithVertA;
						ithB = sEdges[j].ithVertB;
						ownerFace = sEdges[j].ownerFace;

						int b = uvwFace[ownerFace].t[ithA];
						int a = uvwFace[ownerFace].t[ithB];

						int ob = b;
						int oa = a;

						Point3 vec;
						Point3 pa,pb,pa2,pb2;

						pa = uvw[a];
						pb = uvw[b];

						averageList[oa]->owner = oa;
						averageList[ob]->owner = ob;

						for (k = 0; k < segments; k++)
							{

							float per = (float)(k+1)/(float)segments;

							
							vec = GetOutlineDir(pa,pb)*-1.0f;
							pa2 = pa + vec *tvOffset*per;
							pb2 = pb + vec *tvOffset*per;

							uvw[startVert] = pb2;
							int b2 = startVert;
							startVert++;


							uvwFace[startFace].t[0] = a;
							uvwFace[startFace].t[1] = b;
							uvwFace[startFace].t[2] = b2;
							startFace++;
							

							uvw[startVert] = pa2;
							int a2 = startVert;
							startVert++;

							uvwFace[startFace].t[0] = b2;
							uvwFace[startFace].t[1] = a2;
							uvwFace[startFace].t[2] = a;
							startFace++;


							if (averageList[oa]->a[k].a == -1)
								averageList[oa]->a[k].a = a2;
							else averageList[oa]->a[k].b = a2;

							if (averageList[ob]->a[k].a == -1)
								averageList[ob]->a[k].a = b2;
							else averageList[ob]->a[k].b = b2;

							a = a2;
							b = b2;
							}

						}
					for (j = 0; j < averageList.Count(); j++)
						{
						if (averageList[j]->owner != -1)
							{
							Point3 a,b, owner;
							owner = uvw[j];

							for (int k = 0; k < segments; k++)
								{
								if ( (averageList[j]->a[k].a != -1) && (averageList[j]->a[k].b != -1) )
									{
									int ia,ib;
									ia = averageList[j]->a[k].a;
									ib = averageList[j]->a[k].b;
									a = uvw[ia];
									b = uvw[ib];
									Point3 p = (a+b) *0.5f;

									float len = 0.0f;
									float angle = 0.0f;
									Point3 np = Normalize(p - owner);

									len = Length(a - owner);
									Point3 pa = Normalize(a - owner);
									float dot = DotProd(pa,np);
									if ((fabs(1.0f-dot) <= 0.0001f))
										{
//										len = tvOffset;
										}
									else
										{
						 				angle = acos(dot);
										if (angle == 0.0f)
											len = tvOffset;
										else len = len/cos(angle);
										}

		//							len = 0.05f;


									

									p = Normalize(p-owner) * len ;
									p += owner;
									uvw[ia] = p;
									uvw[ib] = p;
									//look for any points to weld
									for (int k = (initialNumberFaces*2); k < startFace;k++)
										{
										for (int m = 0; m < 3; m++)
											{
											if (uvwFace[k].t[m] == ib)
												uvwFace[k].t[m] = ia;

											}
										}
									}
								}
							}
						delete averageList[j];

						}

					}


				}


			}

		}


	if (autoSmooth)
		{
		float sangle = autoSmoothAngle * PI/180.0f;

		//find 

		mesh->faceSel.ClearAll();

		BitArray sel;
		sel.SetSize(mesh->numFaces);
		sel.ClearAll();

		for (int i = initialNumberFaces*2; i < initialNumberFaces*2+sEdges.Count()*2*segments; i++)
			sel.Set(i);

		AdjEdgeList e(*mesh);
		AdjFaceList f(*mesh,e);

		Tab<Point3> fnorms;
		fnorms.SetCount(mesh->numFaces);

		for (j = 0; j < mesh->numFaces; j++)
			{
			Point3 norm(0.0f,0.0f,0.0f);
			Point3 vecA,vecB,vecC;
			Point3 a,b,c;
			int ia,ib,ic;
			ia =mesh->faces[j].v[0];
			ib =mesh->faces[j].v[1];
			ic =mesh->faces[j].v[2];
			a = mesh->verts[ia];
			b = mesh->verts[ib];
			c = mesh->verts[ic];
			vecA = Normalize(a - b);
			vecB = Normalize(c - b);
			vecC = Normalize(c - a);
			norm = Normalize(CrossProd(vecA,vecB));
			fnorms[j] = norm;
			}



		int numberSet = sel.NumberSet();
		while (sel.NumberSet() != 0)
			{
			for (i = 0 ; i < e.edges.Count(); i++)
				{
				int faceA, faceB;
				faceA = e.edges[i].f[0];
				faceB = e.edges[i].f[1];
				if ((faceA != -1) && (faceB != -1))
					{
					int selFace, unselFace;
					BOOL isSeed = FALSE;
					if (sel[faceA] && !sel[faceB])
						{
						selFace = faceA;
						unselFace = faceB;
						isSeed = TRUE;
						}
					else if (!sel[faceA] && sel[faceB])
						{
						selFace = faceB;
						unselFace = faceA;
						isSeed = TRUE;
						}
					if (isSeed)
						{
						DWORD usedSG = mesh->faces[unselFace].getSmGroup();

						BitArray processed;
						processed.SetSize(mesh->numFaces);
						processed.ClearAll();
						processed.Set(selFace);

						BOOL done = FALSE;
						int numberSet = 1;
						Tab<int> cluster;
						cluster.Append(1,&selFace);
						processed.Set(selFace);


						while (!done)
							{
							Tab<int> newCluster;
							for (int k = 0; k < cluster.Count(); k++)
								{
								int faceIndex = cluster[k];

								for (int m = 0; m < 3; m++)
									{								
									int adjFace = f.list[faceIndex].f[m];

									BOOL isEdge = FALSE;
									//unselected is edge
									if (!sel[adjFace]) isEdge = TRUE;
									float dot = DotProd(fnorms[faceIndex],fnorms[adjFace]);
									float angle = 0.0f;
									if (dot == 0.0f)
										angle = PI/2.0f;
									else if (fabs(1.0f-dot) <= 0.0001f)
										angle = 0.0f;
									else if (dot == -1.0f)
										angle = PI;
									else angle = acos(dot);
									//if angle > threshold is edge
									if (angle > sangle)
										isEdge = TRUE;

									if (isEdge)
										{
										usedSG |= mesh->faces[adjFace].getSmGroup();
										
										}
									else
										{
										if (!processed[adjFace])
											newCluster.Append(1,&adjFace,500);
										processed.Set(adjFace);	
										sel.Set(adjFace,FALSE);
										}								
									}
								
								}
							cluster = newCluster;

							if (numberSet == processed.NumberSet())
								done = TRUE;
							numberSet = processed.NumberSet();
							}
						//get an unused sg
						DWORD sg = 1;
						for (int k = 0; k < 32; k++)
							{
							if (!(sg & usedSG))
								k = 32;
							else sg = sg << 1;
							}
//DebugPrint("Cluster %d",sg);
						for (k = 0; k < processed.GetSize(); k++)
							{

							if (processed[k])
								{
//DebugPrint("%d, ",k);

								mesh->faces[k].setSmGroup(sg);
//								mesh->faceSel.Set(k);
								}
							}					
//DebugPrint("\n");
						}
					}
				}
			if (numberSet == sel.NumberSet())
				sel.ClearAll();
			numberSet = sel.NumberSet();


			}


		}

//	if (0)
	if (selEdges || selFaces || selOuterFaces) 
		{
		BitArray sel;
		sel.SetSize(mesh->numFaces);
		sel.ClearAll();

		if (selFaces) 
			{
			for (int i = initialNumberFaces; i < initialNumberFaces*2; i++)
				sel.Set(i);
			}

		if (selOuterFaces) 
			{
			for (int i = 0; i < initialNumberFaces; i++)
				sel.Set(i);
			}
		


		if (selEdges) 
			{
			for (int i = initialNumberFaces*2; i < initialNumberFaces*2+sEdges.Count()*2*segments; i++)
				sel.Set(i);
			}
		mesh->faceSel = sel;

		}




}

//bevel
//detach the selected faces



void BuildMeshInfo::BevelBySpline(Mesh *mesh,PolyLine *shape,int eMatID, int oMatID,
					  int sg,
					  BOOL autoSmooth, float autoSmoothAngle,
					  int map,
					  BOOL selEgdes, BOOL selTop)
{
	int initialNumberFaces, initialNumberVertices;
	initialNumberFaces = mesh->numFaces;
	initialNumberVertices = mesh->numVerts;

	BuildData(mesh,shape,TRUE);

	BitArray copyFaces(mesh->faceSel);
	//if no selection select all
	if ((mesh->selLevel != MESH_FACE) || (mesh->faceSel.NumberSet() ==0))
	{
	//save our selection
		copyFaces.SetAll();
	}
		
	

	//clone those vertices
	BitArray copyVerts;
	Tab<int> clonedVerts;

	copyVerts.SetSize(initialNumberVertices);
	copyVerts.ClearAll();
	clonedVerts.SetCount(initialNumberVertices);

	for (int j = 0; j < initialNumberVertices; j++)
		clonedVerts[j] = -1;

	for (j = 0; j < initialNumberFaces; j++)
	{
		if (copyFaces[j])
		{
			for (int i = 0; j < 3; i++)
			{
				int index = mesh->faces[j].v[i];
				copyVerts.Set(index);

			}
		}
	}

	int ct = initialNumberVertices;

	int numberSegs = shape->numPts-1;

	mesh->setNumFaces(initialNumberVertices + copyVerts.NumberSet(),TRUE);
	for (j = 0; j < initialNumberVertices; j++)
	{
		if (copyVerts[j])
		{
			mesh->verts[ct] = mesh->verts[j];
			clonedVerts[j] = ct;
			ct++;
		}
	}


	mesh->setNumFaces(initialNumberFaces + copyFaces.NumberSet(),TRUE);
	//clone all the selected faces


	//get a list of all the edge vertices
	//extrude all those edges
	//build those faces

	//bevel it


}
