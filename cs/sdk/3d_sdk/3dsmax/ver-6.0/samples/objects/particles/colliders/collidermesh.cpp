#include "ICollision.h"
#include "Colliders.h"


#define PBLOCK_REF_NO 0

#define USE_OLD_COLLISION 0



void CollisionVNormal::AddNormal(Point3 &n,DWORD s)
	{
	if (!(s&smooth) && init) {
		if (next) next->AddNormal(n,s);
		else {
			next = new CollisionVNormal(n,s);
			}
	} else {
		norm   += n;
		smooth |= s;
		init    = TRUE;
		}
	}

Point3 &CollisionVNormal::GetNormal(DWORD s)
	{
	if (smooth&s || !next) return norm;
	else return next->GetNormal(s);	
	}

void CollisionVNormal::Normalize()
	{
	CollisionVNormal *ptr = next, *prev = this;
	while (ptr) {
		if (ptr->smooth&smooth) {
			norm += ptr->norm;			
			prev->next = ptr->next;
			delete ptr;
			ptr = prev->next;
		} else {
			prev = ptr;
			ptr  = ptr->next;
			}
		}
	norm = ::Normalize(norm);
	if (next) next->Normalize();
	}

TriObject *IsUseable(Object *pobj,TimeValue t)
{ 
	if (pobj->SuperClassID()==GEOMOBJECT_CLASS_ID)
		{	
		if (pobj->IsSubClassOf(triObjectClassID)) 
			return (TriObject*)pobj;
		else 
			{ 
			if (pobj->CanConvertToType(triObjectClassID)) 
	  		return (TriObject*)pobj->ConvertToType(t,triObjectClassID);			
			}
		}
	return NULL;
}

void AddMesh(CollisionMesh *obj,TriObject *triOb,Matrix3 tm,BOOL nottop)
{ int lastv=obj->nv,lastf=obj->nf;
  obj->nv+=triOb->GetMesh().getNumVerts();
  obj->nf+=triOb->GetMesh().getNumFaces();
  if (!nottop)
    obj->dmesh->DeepCopy(&triOb->GetMesh(),PART_GEOM|PART_TOPO);
  else
  {obj->dmesh->setNumFaces(obj->nf,obj->dmesh->getNumFaces());
   obj->dmesh->setNumVerts(obj->nv,obj->dmesh->getNumVerts());
   tm=tm*obj->invtm;
   for (int vc=0;vc<triOb->GetMesh().getNumFaces();vc++)
   { obj->dmesh->faces[lastf]=triOb->GetMesh().faces[vc];
     for (int vs=0;vs<3;vs++) 
	   obj->dmesh->faces[lastf].v[vs]+=lastv;
     lastf++;}
  }
   for (int vc=0;vc<triOb->GetMesh().getNumVerts();vc++)
   { if (nottop) obj->dmesh->verts[lastv]=triOb->GetMesh().verts[vc]*tm;
	 else obj->dmesh->verts[lastv]=triOb->GetMesh().verts[vc];
     lastv++;}
}  
//watje
/*
void AddMeshPrev(CollisionMesh *obj,TriObject *triOb,Matrix3 tm,BOOL nottop)
{ int lastv=0,lastf=0;
  if (!nottop)
    obj->dmeshPrev->DeepCopy(&triOb->GetMesh(),PART_GEOM|PART_TOPO);
  else
  {obj->dmeshPrev->setNumFaces(obj->nf,obj->dmesh->getNumFaces());
   obj->dmeshPrev->setNumVerts(obj->nv,obj->dmesh->getNumVerts());
   tm=tm*obj->invtmPrev;
   for (int vc=0;vc<triOb->GetMesh().getNumFaces();vc++)
		{ 
		obj->dmeshPrev->faces[lastf]=triOb->GetMesh().faces[vc];
		for (int vs=0;vs<3;vs++) 
		   obj->dmeshPrev->faces[lastf].v[vs]+=lastv;
		lastf++;
		}

	}
   for (int vc=0;vc<triOb->GetMesh().getNumVerts();vc++)
   { if (nottop) 
		obj->dmeshPrev->verts[lastv]=triOb->GetMesh().verts[vc]*tm;
	 else obj->dmeshPrev->verts[lastv]=triOb->GetMesh().verts[vc];
     lastv++;}
}  
*/

void GetVFLst(Mesh* dmesh,CollisionVNormal* vnorms,Point3* fnorms)	 
{ int nv=dmesh->getNumVerts();	
  int nf=dmesh->getNumFaces();	
  Face *face = dmesh->faces;
  for (int i=0; i<nv; i++) 
    vnorms[i] = CollisionVNormal();
  Point3 v0, v1, v2;
  for (i=0; i<nf; i++,face++) 
  {	// Calculate the surface normal
	v0 = dmesh->verts[face->v[0]];
	v1 = dmesh->verts[face->v[1]];
	v2 = dmesh->verts[face->v[2]];
	fnorms[i] = (v1-v0)^(v2-v1);
	for (int j=0; j<3; j++) 
	   vnorms[face->v[j]].AddNormal(fnorms[i],face->smGroup);
    fnorms[i] = Normalize(fnorms[i]);
  }
  for (i=0; i<nv; i++) 
	vnorms[i].Normalize();
}


/*  Mesh Collision Code  */




class CollisionMeshClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() {return 0;}
	void *			Create(BOOL loading = FALSE) { return new CollisionMesh;}
	const TCHAR *	ClassName() {return GetString(IDS_PW_MESHCOLLISION_CLASS);}
	SClass_ID		SuperClassID() {return REF_MAKER_CLASS_ID; }
	Class_ID		ClassID() {return MESH_COLLISION_ID;}
	const TCHAR* 	Category() {return GetString(IDS_PW_COLLISIONCLASS);}

	// Hardwired name, used by MAX Script as unique identifier
	const TCHAR*	InternalName()				{ return _T("MeshCollision"); }
	HINSTANCE		HInstance()					{ return hInstance; }
	};

static CollisionMeshClassDesc collisionMeshDesc;
ClassDesc* GetCollisionMeshDesc() {return &collisionMeshDesc;}


//this is the data need to solve for the mesh collision

// per instance geosphere block
static ParamBlockDesc2 collisionmesh_param_blk ( collisionmesh_params, _T("CollisionMeshParameters"),  0, &collisionMeshDesc, P_AUTO_CONSTRUCT, 0, 
	// params
	collisionmesh_hit_face_index,  _T("hit_face_index"), 			TYPE_INT, 	0, 	IDS_PW_HITFACEINDEX,   
		end, 
	collisionmesh_hit_bary,  _T("hit_bary"), 			TYPE_POINT3, 	0, 	IDS_PW_HITBARY,   
		end, 
	collisionmesh_node,  _T("node"), 			TYPE_INODE, 	0, 	IDS_PW_NODE,   
		end, 


	end
	);

CollisionMesh::CollisionMesh()
	{
	pblock = NULL;
//instaniates the pblock
	collisionMeshDesc.MakeAutoParamBlocks(this);
	assert(pblock);

	dmesh=NULL;
	vnorms=NULL;
	fnorms=NULL;

//watje
//	dmeshPrev=NULL;
//	vnormsPrev=NULL;
//	fnormsPrev=NULL;
	}

CollisionMesh::~CollisionMesh()
{
	DeleteAllRefsFromMe();

	if (vnorms) delete[] vnorms;
	if (fnorms) delete[] fnorms;
	if (dmesh) delete dmesh;

//watje
//   if (vnormsPrev) delete[] vnormsPrev;
//	if (fnormsPrev) delete[] fnormsPrev;
  // if (dmeshPrev) delete dmeshPrev;
}

// Bayboro: should be as a member of the CollisionMesh class,
// but will work for now (3/6/2001)
static TimeValue preFrameDT;

//this call alls you to do some computation before the frame is solved
void CollisionMesh::PreFrame(TimeValue t, TimeValue dt) 
{
	Interval iv;
	INode *tempNode;
	pblock->GetValue(collisionmesh_node,t,tempNode,iv);
	if (tempNode != NULL) node = tempNode;

	if (node == NULL) return;

	preFrameDT = dt;
	validity = FOREVER;
	tm = node->GetObjectTM(t,&validity);
	invtm= Inverse(tm);

	tmPrev=node->GetObjectTM(t-dt,&iv);
	invtmPrev=Inverse(tmPrev);
	if (dmesh) 
		delete dmesh;
	dmesh=new Mesh;
	dmesh->setNumFaces(0);
	if (vnorms) 
	{	
		delete[] vnorms;
		vnorms=NULL;
	}
	if (fnorms) 
	{
		delete[] fnorms;
		fnorms=NULL;
	}
	nv=nf=0;
	Interval tmpValid=FOREVER;
	Object *pobj; 
	pobj = node->EvalWorldState(t).obj;
	TriObject *triOb=NULL;
	if ((triOb=IsUseable(pobj,t))!=NULL) 
		AddMesh(this,triOb,tm,TRUE);
	if ((triOb)&&(triOb!=pobj)) 
		triOb->DeleteThis();
	if (node->IsGroupHead())
		{	
		for (int ch=0;ch<node->NumberOfChildren();ch++)
			{	
			INode *cnode=node->GetChildNode(ch);
			if (cnode->IsGroupMember())
				{	
				pobj = cnode->EvalWorldState(t).obj;
				if ((triOb=IsUseable(pobj,t))!=NULL)
					{	
					Matrix3 tm=cnode->GetObjectTM(t,&tmpValid);
					AddMesh(this,triOb,tm,TRUE);
					}
				}
				if ((triOb)&&(triOb!=pobj)) 
					triOb->DeleteThis();
			}
		}
	if (nf>0)
	{	vnorms=new CollisionVNormal[nv];
		fnorms=new Point3[nf];
		GetVFLst(dmesh,vnorms,fnorms);
	}
}


	
#define EPSILON	0.0001f

BOOL RayIntersect(Ray& ray, float& at, Point3& norm,Mesh *amesh,CollisionVNormal* vnorms,Point3 *fnorms,
				 DWORD &faceId, Point3 &outBary)
{
	DWORD fi;
	Point3 bary;
	Face *face = amesh->faces;	
	Point3 v0, v1, v2;
	Point3 n, sum, p, bry;
	float d, rn, a;
	Matrix3 vTM(1);
	BOOL first = FALSE;
	fi = 0xFFFFFFFF;

	for (int i=0; i<amesh->getNumFaces(); i++,face++) {
		n = fnorms[i];
		
		// See if the ray intersects the plane (backfaced)
		rn = DotProd(ray.dir,n);
//		if (rn > -EPSILON) continue;
		if (rn > 0.0f) continue;
		
		// Use a point on the plane to find d
		d = DotProd(amesh->verts[face->v[0]],n);

		// Find the point on the ray that intersects the plane
		a = (d - DotProd(ray.p,n)) / rn;

		// Must be positive...
		if (a < 0.0f) continue;

		// Must be closer than the closest at so far
		if (first) {
			if (a > at) continue;
			}

		// The point on the ray and in the plane.
		p = ray.p + a*ray.dir;

		// Compute barycentric coords.
		bry = amesh->BaryCoords(i,p);

		// barycentric coordinates must sum to 1 and each component must
		// be in the range 0-1
		// if (bry.x<0.0f || bry.x>1.0f || bry.y<0.0f || bry.y>1.0f || bry.z<0.0f || bry.z>1.0f) continue;
		if (bry.x<-EPSILON || bry.x>1.0f+EPSILON || bry.y<-EPSILON || bry.y>1.0f+EPSILON || bry.z<-EPSILON || bry.z>1.0f+EPSILON) continue;
		// if (fabs(bry.x + bry.y + bry.z - 1.0f) >= 0.0001f) continue;
		if (fabs(bry.x + bry.y + bry.z - 1.0f) >= EPSILON) continue;

		// Hit!
		first = TRUE;		
		at    = a;
		fi    = (DWORD)i;		
//		bary.x  = bry.z;
//		bary.y  = bry.x;
//		bary.z  = bry.y;
		bary  = bry;	// DS 3/8/97
		
		// Use interpolated normal instead.
		if (!face->smGroup) {
			norm  = n;
		} else {
			norm = 
				vnorms[face->v[0]].GetNormal(face->smGroup) * bary.x +
				vnorms[face->v[1]].GetNormal(face->smGroup) * bary.y +
				vnorms[face->v[2]].GetNormal(face->smGroup) * bary.z;
			norm = Normalize(norm);
			}
		}
	faceId = fi;
	outBary = bary;
	return first;
}


static float const K1 = 0.01f;
static float const K3 = 0.999f;

//this is the actual point to mesh collision test
//returns the time of the hit, the point that was hit, the bounce velpocity vector component
//friction velocity vector component, and the amount of velocity the particle inherits from the 
//motion of the deflector

BOOL CollisionMesh::CheckCollision (TimeValue t,Point3 pos, Point3 vel, float dt, 
									 float &finalAt, Point3 &hitPoint, Point3 &bounceVec, Point3 &frictionVec, Point3 &inheritedVel 
									 )
{
//check if particle collides with the end frame object
	if (!dmesh) return FALSE;
	if (dt == 0.0f) return FALSE;

	Point3 startPos, endPos, relativeVel, relativeVelNorm;
	Point3 norm, hitPointColliderSpace;
	float velLength, at, beforeAt;
	Ray ray;
	int kfound;
	Interval ival;

//----------------------------------------------------------------//
//			static collider: simplifies overall					  //
//----------------------------------------------------------------//
	if (validity.Start() <= t-dt)
	{
		startPos = pos*invtm;
		relativeVel = dt*VectorTransform(invtm, vel);
		velLength = Length(relativeVel);
		if (velLength == 0.0f) return FALSE; // particle is static
		relativeVelNorm = relativeVel/velLength;

		ray.p = startPos;
		ray.dir = relativeVelNorm;
		kfound = RayIntersect(ray, at, norm, dmesh, vnorms, fnorms, outFi, outBary);

		if (!kfound) return FALSE; // no points of intersection
		beforeAt = at*K3;
		if (beforeAt > velLength) return FALSE; // intersection is far away
		
		// back it up a tiny bit so we are not exactly on the object
		hitPointColliderSpace = startPos + beforeAt*relativeVelNorm + K1*norm;
		hitPoint = hitPointColliderSpace * tm;

		bounceVec = -DotProd(relativeVel, norm) * norm;
		frictionVec = relativeVel + bounceVec;
		bounceVec = VectorTransform(tm, bounceVec)/dt;
		frictionVec = VectorTransform(tm, frictionVec)/dt;

		finalAt = beforeAt*dt/velLength;

		inheritedVel = Point3::Origin;

		ival = validity;
		pblock->SetValue(collisionmesh_hit_face_index,t,(int)outFi);
		pblock->SetValue(collisionmesh_hit_bary,t,outBary);
		validity = ival;

		return TRUE;
	} // end of static collider

//----------------------------------------------------------------//
//			moving collider										  //
//----------------------------------------------------------------//
	startPos = ((int)dt == preFrameDT) ? pos*invtmPrev : pos*invtm;
	endPos = (pos + vel*dt)*invtm;
	relativeVel = endPos - startPos;
	velLength = Length(relativeVel);
	if (velLength == 0.0f) return FALSE; // particle is static
	relativeVelNorm = relativeVel/velLength;

	ray.p = startPos;
	ray.dir = relativeVelNorm;
	kfound = RayIntersect(ray, at, norm, dmesh, vnorms, fnorms, outFi, outBary);

	if (!kfound) return FALSE; // no points of intersection
	beforeAt = at*K3;
	if (beforeAt > velLength) return FALSE; // intersection is far away

	//back it up a tiny bit so we are not exactly on the object
	hitPointColliderSpace = startPos + beforeAt*relativeVelNorm + K1*norm;
	hitPoint = hitPointColliderSpace * tm;
		
	float inherit = inheritedVel.x;
	inheritedVel = Point3::Origin;
	if ((int)dt == preFrameDT) // first collision in the current frame
	{
		inheritedVel = (hitPoint*invtmPrev*tm - hitPoint)/preFrameDT;
//		if (DotProd(norm, inheritedVel) < 0)
//			inheritedVel = Point3::Origin;
		ival = validity;
		pblock->SetValue(collisionmesh_hit_face_index,t,(int)outFi);
		pblock->SetValue(collisionmesh_hit_bary,t,outBary);
		validity = ival;
	}

	if (inherit < 1.0f)
		relativeVel += (1.0f-inherit)*VectorTransform(invtm, dt*inheritedVel);

	bounceVec = -DotProd(relativeVel, norm) * norm;
	frictionVec = relativeVel + bounceVec;
	bounceVec = VectorTransform(tm,bounceVec)/dt;
	frictionVec = VectorTransform(tm,frictionVec)/dt;

	finalAt = beforeAt*dt/velLength;

	return TRUE;
}
