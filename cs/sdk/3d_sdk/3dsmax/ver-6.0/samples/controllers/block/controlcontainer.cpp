
/**********************************************************************
 *<
	FILE: controlContainer.cpp

	DESCRIPTION:  a simple object that just holds a list of sub animas 
				  so I can save and load them
				  

	CREATED BY: Peter Watje
				

	HISTORY: 11/27/1998

 *>	Copyright (c) 1998, All Rights Reserved.
 **********************************************************************/
#include "block.h"
#include "masterblock.h"
#include "Simpobj.h"
#include "iparamm2.h"

//--- ClassDescriptor and class vars ---------------------------------
ClassDesc* GetControlContainerDesc();
static ControlContainerClassDesc controlContainerDesc;
ClassDesc* GetControlContainerDesc() { return &controlContainerDesc; }

IObjParam* ControlContainerObject::ip = NULL;




// JBW: Here follows the new parameter block descriptors.  There are now 3, 
//      two new STATIC ones to hold the old class var parameters, one for the main
//		per-instance parameters.  Apart from all the extra 
//      metadata you see in the definitions, one important new idea is the
//      folding of ParamMap description info into the parameter descriptor and
//      providing a semi-automatic rollout desipaly mechanism.
//      

// Parameter Block definitions

// JBW: First come the position and version independent IDs for each
//      of the blocks and the parameters in each block.  These IDs are used
//	    in subsequent Get/SetValue() parameter access, etc. and for version-independent
//      load and save


// JBW: here are the two static block descriptors.  This form of 
//      descriptor declaration uses a static NParamBlockDesc instance whose constructor
//      uses a varargs technique to walk through all the param definitions.
//      It has the advantage of supporting optional and variable type definitions, 
//      but may generate a tad more code than a simple struct template.  I'd
//      be interested in opinions about this.

//      I'll briefly describe the first definition so you can figure the others.  Note
//      that in certain places where strings are expected, you supply a string resource ID rather than
//      a string at it does the lookup for you as needed.
//
//		line 1: block ID, internal name, local (subanim) name, flags
//																 AUTO_UI here means the rollout will
//																 be automatically created (see BeginEditParams for details)
//      line 2: since AUTO_UI was set, this line gives: 
//				dialog resource ID, rollout title, flag test, appendRollout flags
//		line 3: required info for a parameter:
//				ID, internal name, type, flags, local (subanim) name
//		lines 4-6: optional parameter declaration info.  each line starts with a tag saying what
//              kind of spec it is, in this case default value, value range, and UI info as would
//              normally be in a ParamUIDesc less range & dimension
//	    the param lines are repeated as needed for the number of parameters defined.

// JBW: this descriptor defines the main per-instance parameter block.  It is flagged as AUTO_CONSTRUCT which
//      means that the CreateInstance() will automatically create one of these blocks and set it to the reference
//      number given (0 in this case, as seen at the end of the line).

// per instance geosphere block
static ParamBlockDesc2 container_param_blk ( container_params, _T("ContainerParameters"),  0, &controlContainerDesc, P_AUTO_CONSTRUCT, 0, 
	//rollout
//	0, 0, 0, 0, NULL,
	// params
	container_refs,  _T("refs"), TYPE_REFTARG_TAB,0 , 0, 	IDS_PW_REFS, 
		end, 
	container_names,  _T("names"), TYPE_STRING_TAB,0, 0, 	IDS_PW_REFS_NAMES, 
		end, 
	container_color,  _T("color"), TYPE_POINT3, 0, 	IDS_PW_COLOR, 
		end, 
	container_start,  _T("start"), TYPE_TIMEVALUE , 0, 	IDS_PW_START, 
		end, 
	container_end,  _T("end"), TYPE_TIMEVALUE , 0, 	IDS_PW_END, 
		end, 
	container_blockname,  _T("name"), TYPE_STRING, 0, 	IDS_PW_REFS_NAMES, 
		end, 

	end
	);




//--- GSphere methods -------------------------------
// JBW: the GeoSphere constructor has gone.  The paramblock creation and wiring and
//		the intial value setting is automatic now.
// JBW: BeginEditParams() becomes much simpler with automatic UI param blocks.
//      you redirect the BeginEditParams() to the ClassDesc instance and it
//      throws up the appropriate rollouts.

ControlContainerObject::ControlContainerObject() 
	{ 
	GetControlContainerDesc()->MakeAutoParamBlocks(this); 
	assert(pblock2);
	}



void ControlContainerObject::BeginEditParams(IObjParam *ip, ULONG flags, Animatable *prev)
{
	GeomObject::BeginEditParams(ip, flags, prev);
	this->ip = ip;
	// throw up all the appropriate auto-rollouts
	controlContainerDesc.BeginEditParams(ip, this, flags, prev);
}
		
// JBW: similarly for EndEditParams and you also don't have to snapshot
//		current parameter values as initial values for next object as
//		this is automatic for the new ParamBlock params unless disabled.

void ControlContainerObject::EndEditParams(IObjParam *ip, ULONG flags, Animatable *next)
{		
	GeomObject::EndEditParams(ip, flags, next);
	this->ip = NULL;
	// tear down the appropriate auto-rollouts
	controlContainerDesc.EndEditParams(ip, this, flags, next);
}

// CONSTRUCTING THE MESH:

// To construct a geodesic sphere, we take a tetrahedron, subdivide each face into
// segs^2 faces, and project the vertices onto the sphere of the correct radius.

// This subdivision produces 3 kinds of vertices: 4 "corner" vertices, which are the
// original tetrahedral vertices; "edge" vertices, those that lie on the tetrahedron's
// edges, and "face" vertices.  There are 6 edges with (segs-1) verts on each, and
// 4 faces with (segs-1)*(segs-2)/2 verts.

// We construct these vertices in this order: the first four are the corner vertices.
// Then we use spherical interpolation to place edge vertices along each edge.
// Finally, we use the same interpolation to produce face vertices between the edge
// vertices.


// Assumed in the following function: the vertices have the same radius, or
// distance from the origin, and they have nontrivial cross product.



BOOL ControlContainerObject::HasUVW() { 
	return 0; 
	}

void ControlContainerObject::SetGenUVW(BOOL sw) {  
	}


// Now put it all together sensibly
#define EPSILON 1e-5f
void ControlContainerObject::BuildMesh(TimeValue t)
	{
	
	mesh.setNumVerts(0);
	mesh.setNumFaces(0);
	mesh.setNumTVerts (0);
	mesh.setNumTVFaces (0);

	mesh.InvalidateTopologyCache();
}

class GSphereObjCreateCallBack : public CreateMouseCallBack {
	IPoint2 sp0;
	ControlContainerObject *ob;
	Point3 p0;
public:
	int proc( ViewExp *vpt, int msg, int point, int flags, IPoint2 m, Matrix3& mat);
	void SetObj(ControlContainerObject *obj) {ob = obj;}
};

int GSphereObjCreateCallBack::proc(ViewExp *vpt, int msg, int point, int flags, IPoint2 m, Matrix3& mat ) {
	float r;
	Point3 p1, center;

	if (msg == MOUSE_FREEMOVE)
	{
		vpt->SnapPreview(m, m, NULL, SNAP_IN_3D);
	}


	if (msg==MOUSE_POINT||msg==MOUSE_MOVE) {
		switch(point) {
		case 0:  // only happens with MOUSE_POINT msg
			ob->suspendSnap = TRUE;				
			sp0 = m;
			p0 = vpt->SnapPoint(m, m, NULL, SNAP_IN_3D);
			mat.SetTrans(p0);
			break;
		case 1:
			mat.IdentityMatrix();
			p1 = vpt->SnapPoint(m, m, NULL, SNAP_IN_3D);
			center = (p0+p1)/float(2);
			mat.SetTrans(center);
			r = Length(center-p0);
			mat.SetTrans(center);
			container_param_blk.InvalidateUI();

			if (flags&MOUSE_CTRL) {
				float ang = (float)atan2(p1.y-p0.y, p1.x-p0.x);					
				mat.PreRotateZ(ob->ip->SnapAngle(ang));
			}

			if (msg==MOUSE_POINT) {
				ob->suspendSnap = FALSE;
				return (Length(m-sp0)<3)?CREATE_ABORT:CREATE_STOP;
			}
			break;					   
		}
	} else {
		if (msg == MOUSE_ABORT) return CREATE_ABORT;
	}

	return TRUE;
}

static GSphereObjCreateCallBack gsphereCreateCB;

CreateMouseCallBack* ControlContainerObject::GetCreateMouseCallBack() 
	{
	gsphereCreateCB.SetObj(this);
	return(&gsphereCreateCB);
	}


BOOL ControlContainerObject::OKtoDisplay(TimeValue t) 
	{
	return FALSE;
	}


void ControlContainerObject::InvalidateUI() 
{
	container_param_blk.InvalidateUI();
}

RefTargetHandle ControlContainerObject::Clone(RemapDir& remap) 
{
	ControlContainerObject* newob = new ControlContainerObject();	
	newob->ReplaceReference(0, pblock2->Clone(remap));
	newob->ivalid.SetEmpty();	
	BaseClone(this, newob, remap);
	return(newob);
}


int ControlContainerObject::CanConvertToType(Class_ID obtype)
	{
	return GeomObject::CanConvertToType(obtype);
	}

Object* ControlContainerObject::ConvertToType(TimeValue t, Class_ID obtype)
	{
	return SimpleObject::ConvertToType(t, obtype);
	}


void ControlContainerObject::GetCollapseTypes(Tab<Class_ID> &clist, Tab<TSTR*> &nlist)
{
    Object::GetCollapseTypes(clist, nlist);
}

