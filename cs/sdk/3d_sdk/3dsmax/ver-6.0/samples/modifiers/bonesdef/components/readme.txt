Readme.txt : Interface description of the interfaces exposed from MAXComponents 
History : 
07/06/99 : Readme.txt created with interface description for IID_IFlexEngine and IID_IBezierSurfEngine
08/30/99 : IID_ISkinEngine description added


MAXComponents is an inproc COM server that implements 3 interfaces:

IID_IFlexEngine 
IID_IBezierSurfEngine
IID_ISkinEngine

A recompile of the MAXComponents dll will automatically register the COM server in the registry. In order to register the 
dll manually please call "regsvr32 MAXComponents.dll" from the command line.

In order to create the COM server, developers have to call CoInitialize followed by CoCreateInstance with the specific
class id :

	IFlexEngine *pFME = NULL;
	
	CoInitialize(NULL);
	// This is two steps in one, CreateInstance IUnknown + QureyInterface IID_IFlexEngine
	HRESULT hr = CoCreateInstance( CLSID_FlexEngine, NULL, CLSCTX_INPROC_SERVER,IID_IFlexEngine,(void **)&pFME);


The interfaces are defined/implemented in the following files. These files have to be included 
in the clients project.

"MAXComponents\MAXComponents.h"
"MAXComponents\MAXComponents_i.c"

The MAXComponents dll uses some geometry classes such as Point3 and Matrix3 of the MAX SDK and thus links to the geom.dll and maxutil.dll.
These dll's have to be present in the path, or same directory as the MAXComponents.dll. Note, that if you compile with Debug Multithreaded you
have to use the geom.dll und maxutil.dll provided with the debug SDK. It is assumed, that game developers would replace the vector and matrix 
multiplication code with their own optimized libraries. It is not allowed to ship geom.dll and maxutil.dll with a game.

Samples for all interfaces can be found in the following sample modifiers:

COMFlex (IID_IFlexEngine) : Functionality identical to Flex modifier, but uses FlexEngine
SimpleFlex (IID_IFlexEngine) : World space modifier, that is much simpler than the flex modifier. 
							   It doesn't support external forces (wind etc.), no vertex weights (it does support soft selections though) and 
							   no animated meshes in conjunction with network rendering (or whenever you go backwards in time)

PatchTess (IID_IBezierSurfEngine) : Tesselates a Patchmesh. Supports with or without vertex binding.

COM Skin (IID_ISkinEngine) : Functionality almost identical to Skin modifier. The Skin modifier BonesDef.dlm has to be moved from the 
stdplugs directory when this modifier is used, since it has the same ClassID. COM Skin uses the Node TM's to affect the mesh unlike the original Skin 
modifier, which uses the Object TM.

Interface description:

FlexEngine (IID_IFlexEngine) :

This interface provides the functionality to produce spring-based animation. Since it generally works on 
positions in 3D it can be used for different geometry types like mesh vertices, Bezier surface CV's, Particles etc. 
The algorithm uses Euler integration, to er the differential equation. External forces such as wind, gravity 
etc. are also supported.

For samples on how to use this COM server

The algorithm is split into 4 sections:

1) Set the engine parameters
2) Evaluate the engine for the current time
3) Provide requested data in callbacks (connection points)
4) Read out the new 3D coordinates

Here are the methods involved with the different sections:

1) Set the Engine parameters:

// All parameters can be animated, meaning they can change from frame to frame, although it does not make much sense to
// change the number of points over time, since a realistic simulation is impossible, because no animation data 
// is available for the newly added points. Finding out which points have been added is not possible, due to 
// topology changes

SetNumPoints(int numpoints)		// This tells the FlexEngine the number of points it is working on, to allocate 
								// the memory for caches etc. this method must be called first.

SetFalloff(float falloff)		// This is the flex parameter
SetStrength(float strength)		// This is the strength parameter
SetDampening(float dampening)	// This is sway parameter
SetWeight(int idx, float weight)// This can be used to support soft selections. idx is the index of the position
								// and weight is a value between 0 and 1. See SDK help file for more info on soft selections

SetReferenceTime(int time)		// In case the flex animation is absolute (in contrast to relative)
								// the reference time must be set. If the client requests a time that is smaller 
								// than the last time the engine was evaluated (values in the cache) the engine
								// will recalculate the timeline starting from the reference time.

2) Evaluate the engine for the current time:

// Developers have to decide, if they are using an absolute, or relative evaluation model. Typically developers
// would call either or one of the following methods to let the engine calculate the new positions.

EvalEngineAbsolute(int curTime, int ticksPerSecond, int stepSize) 

// This method will be used for absolute time evaluation. Absolute time evaluation means, that the animation 
// happens in a finite time interval where it is allowed to evaluate the engine at any point in time. Note, that 
// if subsequent calls go back in time, a performance hit can be experienced, since the animation has to be 
// recalculated starting from the reference time (see paragraph 3. below).

// curTime is the time where the engine should be evaluated in ticks
// ticksPerSecond tells the engine the units of the curTime value
// stepSize tells the engine in which time steps the Euler integration should be done. 

// It is totally up to the developer to determine the resolution of the Euler integration. If the developer wants 
// to set the step size to the frame rate he would simply provide the frame rate (ticks per frame) in this argument. 
// The engine will call the client back at specific (sometimes multiple) times depending on the step size and the 
// last valid cache time.(See paragraph 3.below).


EvalEngineRelative(int deltaT, int ticksPerSecond)

// This method will be used for relative time evaluation. Relative time evaluation means, that the
// animation happens on an infinite time range, where the engine can never be evaluated at an earlier time than 
// the last evaluation. There is no stepSize, since the stepSize is determined by deltaT. The client will only be
// called back once per evaluation with the current deltaT. This method is likely to be used from inside a game,
// where the time range is infinite.


3) Provide requested data in callbacks (connection points)

// The client (your application, or plug-in) has to provide a callback class implemented as connection point and
// register this callback class with the FlexEngine. It does so, by implementing a class derived from 
// _IFlexEngineEvents and implementing the two methods GetPoints and GetForces as well as the IUnknown methods
// AddRef(), Release() and QueryInterface(REFIID iid, void** ppv). Samples can be found in COMFlex and SimpleFlex.
// Depending on the evaluation model used (absolute or relative) the client can be called back multiple times or
// once per evaluation. The client has to provide the requested data for the engine :

GetPoints( int time, int size, float *points)


// The client has to fill the points array with the positions at the specified time. The points array is an 
// array of floats triplets (x,y,z) with the given size ( float points[size][3] ). The positions have to be 
// in world space.


// - Absolute evaluation method:
//   Depending on the internal cache of the FlexEngine (last evaluated time) and the stepSize, this method can be 
//   called for different times than the time passed in EvalEngineAbsolute.

// - Relative evaluation method:
//   This method will only be called once per evaluation. The time argument will be the deltaT passed to EvalEngineRelative.


GetForces( int time, int num, DWORD p_stride, float *pPos, DWORD v_stride, float *pVel, float *pForces)

// The client has to fill the pFroces array with the external forces at the specified time. The pForces array is an 
// array of floats triplets (x,y,z) with the given size ( float pForces[size][3] ). The forces have to be 
// in world space. The client has access to the positions and the velocities of all points through the parameters
// pPos and pVel. The pPos and pVel arrays point to arrays of a structure (or class) that consists of more data than only 
// 3 floats (x,y,z). In order to be able to iterate over this structure, developers get additional arguments passed
// p_stride and v_stride, which define the offset between the positions or velocities. This offset is defined in bytes.
// In order to iterate over the array, developers can use the following code :

	BYTE *pPosPtr = (BYTE *) pPos;
	BYTE *pVelPtr = (BYTE *) pVel;

	for(int idx = 0 ; idx < num ; idx++, pPosPtr+= p_stride, pVelPtr+= v_stride)
	{
		pForces[idx] = GetForce(time, *(Point3 *)pPosPtr, *(Point3*)pVelPtr, ObjTM);
	}

// It is also possible to use the template StrideArray provided in Utilities.h. Developer would use it like that :

StrideArray<Point3> PosArray = StrideArray<Point3>((Point3 *) pPos, p_stride);
StrideArray<Point3> VelArray = StrideArray<Point3>((Point3 *) pVel, v_stride);

for(int i = 0 ; i < num ; i++)
{
	pForces[idx] = GetForce(time, PosArray[i], VelArray[i], ObjTM);
}


4) Read out the new 3D coordinates

GetPoint(int i, float *point)


// After the engine has been evaluated the points can be read out with GetPoint(int i, float *point).

// i is the index of the point
// point is the address of a x,y,z float triplet (float point[3]), that will be filled with the x,y,z position from the engine.

ResetCache()

// This method can be called to flush all the cache data in the engine. The next time the engine will be evaluated
// all velocities will be 0.


BezierSurfEngine (IID_IBezierSurfEngine) :

// This interface provides the functionality to provide the vertex positions on a Bezier surface, given control vertices 
// and control vectors. These positions can be tessellated into a mesh structure. On more information about mesh tessellation
// have a look at the Patch Tess modifier.

// In order to tessellate a Bezier surface a client application would iterate over all patches of a PatchMesh and sample the
// patches over the respective u,v ( & w for TriPatches) coordinates.

// Here's how the sampling algorithm would look like :

	
	PatchMesh *pm = &pobj->patch;
	IBezierSurfEngine *pBSE;

	pBSE->InitializePatchMesh(pm->numVerts,sizeof(PatchVert),(float *) pm->verts,pm->numVecs, sizeof(PatchVec),(float *) pm->vecs); 
	for(int i = 0; i < pm->numPatches; i++) {
		Patch &p = pm->patches[px];
		pBSE->InitializePatch(p.type,p.v, p.vec, false, (float *) p.aux, p.interior);
		float df = 1.0f/(steps-1);

		switch(p.type) {
			case PATCH_TRI: {
				float fu, fv;
				for(int u = 0; u < steps; u++) {
					fu = df*(float)u;
					for(int v = 0; v < work; v++) {
						float fv = fv = df*(float)v;
						float fw = 1.0f - fu - fv;		// Barycentric validity guaranteed!
						pBSE->Interpolate( fu, fv, fw, pout);
						mesh->setVert(vert++, pout); 
						}
					}
				}
				break;
			case PATCH_QUAD: {
				float fv, fu;
				for(int u = 0; u < steps ; u++) {
					fu = df*(float)u;
					for(int v = 0; v < steps; v++) {		
						fv = df*(float)v;
						pBSE->Interpolate( fu, fv, 0.0f, pout);
						mesh->setVert(vert++, pout); 
						}
					}
				}
			}
		}


InitializePatchMesh(int numvert, DWORD vert_stride, float *pVert, int numvec, DWORD vec_stride, float *pVec)

// This method has to be called for a PatchMesh to provide the control vertex and control vector arrays for the engine.
// It is unimportant how the structure of the vertices and vectors look like. The client simply to provides a pointer to the 
// first vertex/vector and the offset in bytes between the control vertices/vectors, so the engine can iterate over the array.

// This is how a developer would use this method:

	PatchMesh *pm = &pobj->patch;
	IBezierSurfEngine *pBSE;	

	pBSE->InitializePatchMesh(pm->numVerts,sizeof(PatchVert),(float *) pm->verts,pm->numVecs, sizeof(PatchVec),(float *) pm->vecs); 

InitializePatch(int type, int *pvert, int *pvec, int iUpdateInteriors, float *pAux, int *pInterior)

// This method has to be called for each Patch to provide the engine with the following data:

// type  : PATCH_TRI (3) or PATCH_QUAD (4)
// pvert : pointer to an integer array, consisting of 4 integers, describing the indices of the control vertices into the pVert array, 
//		   passed to InitializePatchMesh for TriPatches only the first 3 integers are used.
// pvec  : pointer to an integer array, consisting of 8 integers, describing the indices of the control vectors into the pVec array, 
//         passed to InitializePatchMesh for TriPatches only the first 6 integers are used.
//
// iUpdateInteriors : pass true, in case the vertices or vectors have moved since the last evaluation. This is the case when you have 
//				      animated patches. The engine has to recalculate the interior vertices and/or the auxiliary points.
//
// pAux  : pointer to an array of 9 (x,y,z) float triplets (27 float values), describing auxiliary points needed by the interpolation algorithm.
//
// pInterior : pointer to an integer array, consisting of 4 integers, describing the indices of the interior vertices into the pVert array, 
			   passed to InitializePatchMesh for TriPatches only the first 3 integers are used.
//


Interpolate(float u, float v, float w, float *pout)
 
// This method is used to sample the patch at a given u,v coordinate (w for TriPatches). The point on the surface is returned in pout, which
// is the address of a float triplet (float pout[3])


Skin Engine (IID_ISkinEngine) :

The main difference between the COM Skin modifier and the old Skin modifier is, that COM Skin uses the Node TM's and not the Object TM's. In order to 
do that, the Node TM's are reconstructed from the Object TM at load time. 

IMPORTANT : It is assumed, that the Object Offset TM didn't change between bone assignment and save. If that is the case, the bones have to be reassigned.

All of the data required for the Skin interface can be retrieved through the Skin API, or the MAX SDK.

Skin interface :

The Skin interface can be divided into initialization and runtime. Normally the initialization has to be done only once and the runtime methods have to 
be called on every frame.

Initialization :

SetNumBones(int numBones)					// This method sets the number of bones, that the object is assigned to. this method has to be called *before*
											// any of the below methods.

SetNumPoints(int numPoints)					// This sets the number of points of the object. The meaning of points is defined by the object, the Skin modifier 
											// is assigned to. they can be vertices in case of meshes, or vertices and vectors in the case of Patches, or CV's
											// for NURBS.

SetBoneFlags(int boneIdx, DWORD flags)		// The flags are currently only used to determine, if the bone is a spline that the object is linked to or not.
											// In case it is a spline, the client has to support spline animation by implementing the connection point for 
											// _ISkinEngineEvents (see Skin project).

SetInitTM(float *InitTM)					// This method sets the Node TM of the Object at initialization time.

SetInitBoneTM(int boneIdx, float *InitTM)	// This method sets the Node TM of the bone at initialization time. In case the bone is a spline it is actually
											// the Object TM, since spline animation is dependent on the Object TM, not the bone TM.

SetPointData(int pointIdx, int numData,		// This method sets the vertex-bone assignments for the Object. It is important, that the indices in the game engine
											// relate to the ones in MAX.
											// Developers can use the convenience class StrideArray to access the following arrays. See GetForces for a more
											// detailed description of StrideArray. All arrays have numData elements
	DWORD b_stride, int *BoneIndexArray,	// This describes the index of bone that this point is assigned to.
	DWORD w_stride, float *WeightArray,		// This describes the weight with which the point is assigned to the bone.
	
	// The following methods are only used, in case spline animation is supported. If not, all parameters can be passed as NULL.
	
	DWORD sci_stride, int *SubCurveIdxArray, // Type integer. This describes which SubCurve the point is assigned to
	DWORD ssi_stride, int *SubSegIdxArray,   // Type integer. This describes which segment in the SubCurve the point is assigned to
	DWORD ssd_stride, float *SubSegDistArray,// Type float. This describes the distance from the beginning of the SubSegment to the intersection of the normal on the spline
											 // that is defined by the point (vertex).
	DWORD t_stride, float *TangentsArray,    // Type float[3] (x,y,z). This describes the tangent vector at the intersection point.
	DWORD op_stride, float *OPointsArray)	 // Type float[3] (x,y,z). This describes the intersection point itself.


Runtime :


SetBoneTM(int boneIdx, float *currentTM)	// This method sets the new TM of the individual bones, in case they changed from the last frame.
MapPoint(int idx, float *pin, float *pout)	// This method returns the point that was modified by Skin with the given index.

