//***************************************************************************
// CJRender - [cjrender.cpp] Sample Plugin Renderer for 3D Studio MAX.
// 
// By Christer Janson - Kinetix
//
// Description:
// Implementation of main render class
//
//***************************************************************************

#include "maxincl.h"
#include "cjrender.h"
#include "rendutil.h"
#include "refenum.h"

//===========================================================================
//
// Class CJRenderer
//
//===========================================================================

//***************************************************************************
// This is called on File/Reset and we should 
// reset the class variables here.
//***************************************************************************

CJRenderer::CJRenderer()
{
	rendParams.renderer = this;
}

//***************************************************************************
// This is called on File/Reset and we should 
// reset the class variables here.
//***************************************************************************

void CJRenderer::ResetParams()
{
	DebugPrint("**** Resetting parameters.\n");

	rendParams.	SetDefaults();
}

//***************************************************************************
// Standard Animatable method
//***************************************************************************

void CJRenderer::DeleteThis()
{
	delete this;
}

//***************************************************************************
// Standard Animatable method
//***************************************************************************

Class_ID CJRenderer::ClassID()
{
	return CCJREND_CLASS_ID;
}

//***************************************************************************
// Standard Animatable method
//***************************************************************************

void CJRenderer::GetClassName(TSTR& s)
{
	s = RENDERNAME;
}

//***************************************************************************
// Open the renderer.
// This is called when the rendering is first initiated.
// We should do a couple of things here:
// * Grab the parameters passed in and set our own representation
//   of these parameters
// * Enumerate the scene to create a list of all nodes and lights
// * Get hold of the atmospheric effects used
// * Get hold of all materials used.
// * Call RenderBegin() on all objects
//***************************************************************************

int CJRenderer::Open(INode *scene,INode *vnode,ViewParams* viewPar,RendParams& rpar,HWND hwnd,DefaultLight* defaultLights,int numDefLights)
{
	int idx;

	// Important!! This has to be done here in MAX Release 2!
	// Also enable it again in Renderer::Close()
	GetCOREInterface()->DisableSceneRedraw();

	if (!rpar.inMtlEdit) {
		BroadcastNotification(NOTIFY_PRE_RENDER, (void*)(RendParams*)&rpar);	// skk
		}

	// Get options from RenderParams
	// These are the options that are common to all renderers
	rendParams.bVideoColorCheck = rpar.colorCheck;
	rendParams.bForce2Sided = rpar.force2Side;
	rendParams.bRenderHidden = rpar.rendHidden;
	rendParams.bSuperBlack = rpar.superBlack;
	rendParams.bRenderFields = rpar.fieldRender;
	rendParams.bNetRender = rpar.isNetRender;
	rendParams.rendType = rpar.rendType;

	// Default lights
	rendParams.nNumDefLights = numDefLights;
	rendParams.pDefaultLights = defaultLights;

	// Support Render effects
	rendParams.effect = rpar.effect;

	rendParams.gbufReader = NULL;
	rendParams.gbufWriter = NULL;

	// Flag we use when reporting errors
	bFirstFrame = TRUE;
	bUvMessageDone = FALSE;

	// Initialize node counter
	nCurNodeID = -1;  //skk

	// Couldn't hurt to initalize these tables...
	ilist = NULL;
	instTab.ZeroCount();
	instTab.Shrink();
	lightTab.ZeroCount();
	lightTab.Shrink();
	mtls.ZeroCount();
	mtls.Shrink();

	// Get the root of the node hierarchy
	pScene = scene;

	// Viewnode is given if our view is a camera or a light
	pViewNode = vnode;
	theView.pRendParams = &rendParams;

	// Viewpar is there if we render a viewport
	if (viewPar) 
		view = *viewPar;


	// Enumerate the nodes in the scene
	// nodeEnum will fill in the rNodeTab for us.
	// Please note that the scene itself is not a true node, it is a
	// place holder whose children are the top level nodes..
	DebugPrint("**** Scanning for nodes.\n");
	for (idx = 0; idx < scene->NumberOfChildren(); idx++) {
		NodeEnum(scene->GetChildNode(idx));
	}

	DebugPrint("\tFound %d nodes\n", instTab.Count());
	DebugPrint("\tFound %d lights\n", lightTab.Count());

	// If there are no lights in the scene
	// we should use the default lights if we got them.
	if (lightTab.Count() == 0 && defaultLights) {
		for (idx = 0; idx < numDefLights; idx++) {
			RenderLight* rl = new RenderLight(&defaultLights[idx]);
			lightTab.Append(1, &rl);
		}
		DebugPrint("\tUsing %d default lights\n", lightTab.Count());
	}

	// The "top atmospheric" called RenderEnvironment is passed as a member
	// of RendParams.
	// All atmospheric effects are referenced by the RenderEnvironment so we
	// only need to evaluate this "RenderEnvironment" in order to catch all
	// Atmospherics.
	rendParams.atmos = rpar.atmos;

	// The environment Map is a pointer to a Texmap
	rendParams.envMap = rpar.envMap;

	// Add any texture maps in the atmosphere or environment to the mtls list
	GetMaps getmaps(&mtls);

	if (rendParams.atmos)

		EnumRefs(rendParams.atmos,getmaps);

	if (rendParams.envMap)
		EnumRefs(rendParams.envMap,getmaps);

	BeginThings();				// Call RenderBegin() on all objects

	// Indicate that we have been opened.
	// A developer can initialize the renderer from the interface class
	// and we want to make sure that he has called Open() first.
	bOpen = TRUE;

	return 1; 	
}

//***************************************************************************
// Close is the last thing called after rendering all frames.
// Things to do here includes:
// * Deleting anything we have allocated.
// * Call RenderEnd() on all objects
//***************************************************************************

void CJRenderer::Close(HWND hwnd)
{
	int idx;
	DebugPrint("**** Renderer going down.\n");

	EndThings();				// Call RenderEnd() on all objects

	for (idx=0; idx<instTab.Count(); idx++) {
		delete instTab[idx];
	}
	instTab.ZeroCount();
	instTab.Shrink();

	for (idx=0; idx<lightTab.Count(); idx++) {
		delete lightTab[idx];
	}
	lightTab.ZeroCount();
	lightTab.Shrink();

	// Shouldn't delete materials. The Dummy materials are already deleted,
	// and the rest of them aren't allocated/created by us.
	mtls.ZeroCount();
	mtls.Shrink();

	if (!rendParams.inMtlEdit) {
		BroadcastNotification(NOTIFY_POST_RENDER);	// skk
		}

	// Important!! Don't forget to enable screen redraws when the renderer closes.
	GetCOREInterface()->EnableSceneRedraw();
	bOpen = FALSE;
}

//***************************************************************************
// Call RenderBegin() on all objects.
// We need to call this on each object in the reference hierarchy.
// This needs to be done in order to let the object prepare itself
// for rendering.
// Particle systems for example will change to the number of
// particles used to rendering (instead of for viewport),
// and the optimize modifier have different options for 
// viewport and rendering as well.
//***************************************************************************

void CJRenderer::BeginThings()
{
	int idx;

	ClearFlags clearFlags;
	BeginEnum beginEnum(rendParams.time);

	// First clear the A_WORK1 flag for each object
	for (idx = 0; idx < instTab.Count(); idx++) {
		ReferenceMaker* rm = instTab[idx]->GetINode();
		EnumRefs(rm, clearFlags);
	}

	// Clear reference hierarchy from Atmospherics
	if (rendParams.atmos)
		EnumRefs(rendParams.atmos, clearFlags);

	// Clear reference hierarchy from Environment map
	if (rendParams.envMap)
		EnumRefs(rendParams.envMap, clearFlags);


	// Call RenderBegin() and set the A_WORK1 flag on each object.
	// We need to set the flag so we don't call RenderBegin on the
	// same object twice.
	for (idx = 0; idx < instTab.Count(); idx++) {
		ReferenceMaker* rm = instTab[idx]->GetINode();
		EnumRefs(rm, beginEnum);
	}

	// reference hierarchy from Atmospherics
	if (rendParams.atmos)
		EnumRefs(rendParams.atmos, beginEnum);

	// reference hierarchy from Environment map
	if (rendParams.envMap)
		EnumRefs(rendParams.envMap, beginEnum);
}

//***************************************************************************
// Call RenderEnd() on all objects. See above how we called
// RenderBegin() for information.
//***************************************************************************

void CJRenderer::EndThings()
{
	int idx;

	ClearFlags clearFlags;
	EndEnum endEnum(rendParams.time);

	for (idx = 0; idx < instTab.Count(); idx++) {
		ReferenceMaker* rm = instTab[idx]->GetINode();
		EnumRefs(rm, clearFlags);
	}

	if (rendParams.atmos)
		EnumRefs(rendParams.atmos, clearFlags);

	if (rendParams.envMap)
		EnumRefs(rendParams.envMap, clearFlags);


	for (idx = 0; idx < instTab.Count(); idx++) {
		ReferenceMaker* rm = instTab[idx]->GetINode();
		EnumRefs(rm, endEnum);
	}

	if (rendParams.atmos)
		EnumRefs(rendParams.atmos, endEnum);

	if (rendParams.envMap)
		EnumRefs(rendParams.envMap, endEnum);
}

//***************************************************************************
// This is used as a callback for aborting a potentially lengthy operation.
//***************************************************************************

class MyCallback: public CheckAbortCallback {
	CJRenderer *cjr;
	public:
	MyCallback(CJRenderer *r) { cjr = r; }
	virtual BOOL Check() { return cjr->CheckAbort(0,0); }
	virtual BOOL Progress(int done, int total ) { return cjr->CheckAbort(done,total); }
	virtual void SetTitle(const TCHAR *title) { cjr->SetProgTitle(title); }
	};

int CJRenderer::CheckAbort(int done, int total) {
	if (rendParams.progCallback) if (rendParams.progCallback->Progress(done,total)==RENDPROG_ABORT) {
		return 1;
		}
	return 0;
	}

void CJRenderer::SetProgTitle(const TCHAR *title) {
	if (rendParams.progCallback) rendParams.progCallback->SetTitle(title);
	}

//***************************************************************************
// Render() is called by MAX to render each frame.
// We get a time, an output bitmap, some parameters and a progress
// callback passed into us here.
//***************************************************************************

int CJRenderer::Render(TimeValue t, Bitmap* tobm, FrameRendParams &frp, HWND hwnd, RendProgressCallback* prog, ViewParams* viewPar)
{
	int i;
	int nExitStatus = 1;

	if (!tobm || !bOpen) {
		return 0; // No output bitmap, not much we can do.
	}
	
	DebugPrint("**** Rendering frame. TimeValue: %d.\n", t);

	// Update progress window
	if (prog) {
		prog->SetTitle("Preparing to render...");
	}

	// Render progress callback
	rendParams.progCallback = prog;	

	// Setup ViewParams:
	rendParams.devWidth = tobm->Width();
	rendParams.devHeight = tobm->Height();
	rendParams.devAspect = tobm->Aspect();

	// These are moved from rendparams to FrameRendParams for R3
	rendParams.nRegxmin = frp.regxmin;
	rendParams.nRegymin = frp.regymin;
	rendParams.nRegxmax = frp.regxmax;
	rendParams.nRegymax = frp.regymax;

	// Get the frame
	rendParams.time = t;
	// Get the FrameRenderParams. These are parameters that can be animated
	// so they are different every frame
	rendParams.pFrp = &frp;

	// Viewpar is there if we render a viewport
	if (viewPar) 
		view = *viewPar;

	// Setup the view parameters
	if (pViewNode)
		GetViewParams(pViewNode, view, t);

	rendParams.ComputeViewParams(view);

	// Setup G-Buffer channels
	// The channels we need to supply are present in the output bitmap
	// so we ask the bitmap for the channels we need to fill in
	// Use a BitArray for the channels
	rendParams.gbufChan.SetSize(NUMGBCHAN);
	rendParams.gbufChan.ClearAll();

	rendParams.gbufReader = NULL;
	rendParams.gbufWriter = NULL;

	// Original channels
	ULONG origGBufChannels = tobm->ChannelsPresent();

	if (rendParams.effect&&rendParams.effect->Active(t) ) {
		DWORD needChannels = rendParams.effect->GBufferChannelsRequired(t);
		DWORD curChannels = tobm->ChannelsPresent();
		if (needChannels& ~curChannels) {
			tobm->CreateChannels(needChannels&~curChannels);
			if (!((tobm->ChannelsPresent()&needChannels)== needChannels)) {
				//return 0;	- Bail here?
				}
			}
		}

	// Get GBUffers ready:
	GBuffer *gb = tobm->GetGBuffer();
	if (gb) {
		gb->InitBuffer();
		rendParams.gbufWriter = gb->CreateWriter();	// Need one per thread
		rendParams.gbufReader = gb->CreateReader();
		}

	ULONG chan = tobm->ChannelsPresent();

	ULONG ctype;
	pGbufZ = NULL;
	pGbufMtlID = NULL;
	pGbufNodeID = NULL;
	pGbufUV = NULL;
	pGbufNormal = NULL;
	pGbufRealPix = NULL;
	pGbufCov = NULL;
	pGbufBg = NULL;
	pGbufNodeIndex = NULL;
	pGbufRealPix = NULL;
	pGbufColor = NULL;
	pGbufTransp = NULL;
	pGbufVeloc = NULL;


	// Get hold of the specific channel
	if (chan & BMM_CHAN_Z) {
		DebugPrint("G-Buffer requests Z buffer\n");
		rendParams.gbufChan.Set(GBUF_Z);
		pGbufZ = (float*)tobm->GetChannel(BMM_CHAN_Z, ctype);
	}
	if (chan & BMM_CHAN_MTL_ID) {
		DebugPrint("G-Buffer requests Material ID\n");
		rendParams.gbufChan.Set(GBUF_MTLID);
		pGbufMtlID = (UBYTE*)tobm->GetChannel(BMM_CHAN_MTL_ID, ctype);
	}
	if (chan & BMM_CHAN_NODE_ID) {
		DebugPrint("G-Buffer requests Node ID\n");
		rendParams.gbufChan.Set(GBUF_NODEID);
		pGbufNodeID = (UWORD*)tobm->GetChannel(BMM_CHAN_NODE_ID, ctype);
	}
	if (chan & BMM_CHAN_UV) {
		DebugPrint("G-Buffer requests UV Channel\n");
		rendParams.gbufChan.Set(GBUF_UV);
		pGbufUV = (Point2*)tobm->GetChannel(BMM_CHAN_UV, ctype);
	}
	if (chan & BMM_CHAN_NORMAL) {
		DebugPrint("G-Buffer requests Normals\n");
		rendParams.gbufChan.Set(GBUF_NORMAL);
		pGbufNormal = (ULONG*)tobm->GetChannel(BMM_CHAN_NORMAL, ctype);
	}
	if (chan & BMM_CHAN_REALPIX) {
		DebugPrint("G-Buffer requests RealPixel\n");
		rendParams.gbufChan.Set(GBUF_REALPIX);
		pGbufRealPix = (RealPixel*)tobm->GetChannel(BMM_CHAN_REALPIX, ctype);
	}
	if (chan & BMM_CHAN_COVERAGE) {
		DebugPrint("G-Buffer requests Pixel Coverage\n");
		rendParams.gbufChan.Set(GBUF_COVER);
		pGbufCov = (UBYTE*)tobm->GetChannel(BMM_CHAN_COVERAGE, ctype);
	}

	if (chan & BMM_CHAN_BG) {
		DebugPrint("G-Buffer requests background\n");
		rendParams.gbufChan.Set(GBUF_BG);
		pGbufBg = (Color24*)tobm->GetChannel(BMM_CHAN_BG, ctype);
	}

	if (chan & BMM_CHAN_NODE_RENDER_ID) {
		DebugPrint("G-Buffer requests node render id\n");
		rendParams.gbufChan.Set(GBUF_NODE_RENDER_ID);
		pGbufNodeIndex = (UWORD*)tobm->GetChannel(BMM_CHAN_NODE_RENDER_ID, ctype);
	}

	if (chan & BMM_CHAN_COLOR) {
		DebugPrint("G-Buffer requests color\n");
		rendParams.gbufChan.Set(GBUF_COLOR);
		pGbufColor = (Color24*)tobm->GetChannel(BMM_CHAN_COLOR, ctype);
	}
	if (chan & BMM_CHAN_TRANSP) {
		DebugPrint("G-Buffer requests transparency\n");
		rendParams.gbufChan.Set(GBUF_TRANSP);
		pGbufTransp = (Color24*)tobm->GetChannel(BMM_CHAN_TRANSP, ctype);
	}
	if (chan & BMM_CHAN_VELOC) {
		DebugPrint("G-Buffer requests velocity buffer\n");
		rendParams.gbufChan.Set(GBUF_VELOC);
		pGbufVeloc = (Point2*)tobm->GetChannel(BMM_CHAN_VELOC, ctype);
	}


	// This renderer supports NORMAL or REGION rendering.
	// You should also support SELECTED, and BLOWUP
	if (rendParams.rendType == RENDTYPE_REGION) {
		if (rendParams.nRegxmin<0) rendParams.nRegxmin = 0;
		if (rendParams.nRegymin<0) rendParams.nRegymin = 0;
		if (rendParams.nRegxmax>rendParams.devWidth)
			rendParams.nRegxmax = rendParams.devWidth;
		if (rendParams.nRegymax>rendParams.devHeight)
			rendParams.nRegymax = rendParams.devHeight;
		rendParams.nMinx = rendParams.nRegxmin;
		rendParams.nMiny = rendParams.nRegymin;
		rendParams.nMaxx = rendParams.nRegxmax;
		rendParams.nMaxy = rendParams.nRegymax;
	}
	else {
		rendParams.nMinx = 0;
		rendParams.nMiny = 0;
		rendParams.nMaxx = rendParams.devWidth;
		rendParams.nMaxy = rendParams.devHeight;
	}

	// We need to scan and manually load each map in the system.
	// We will only report any errors on the first frame so we pass in
	// a flag indicating if this is the first frame
	// If some maps are missing this method will ask the user if
	// rendering should continue or be aborted.
	if (!LoadMapFiles(t, hwnd, bFirstFrame))	{
		return 0;
	}

	// Update the Atmospheric effects
	if (rendParams.atmos)
		rendParams.atmos->Update(t, FOREVER);

	// Update the environment map
	if (rendParams.envMap)
		rendParams.envMap->Update(t, FOREVER);


	// Update the Materials.
	// Only top-level materials need to be updated.
	// Also we get the mesh for all nodes and count the number
	// of faces (to give status information to the user)
	nNumFaces = 0;
	for (i = 0; i < instTab.Count(); i++) {
		instTab[i]->mtl->Update(t, FOREVER);
		instTab[i]->Update(t, theView, this);
	}

	// Update the Lights for every frame
	// The light descriptor updates its internal transformation here so
	// if you transform the light nodes into camera space in your renderer
	// you need to call Update() on the light while it is still in 
	// world space.
	// In this renderer we don't transform lights into camera space so
	// this is not a problem here
	for (i = 0; i < lightTab.Count(); i++) {
		lightTab[i]->Update(t, this);
		lightTab[i]->UpdateViewDepParams(rendParams.worldToCam);
	}

	// Transform geometry from object space to Camera (view) space	
	for (i = 0; i < instTab.Count(); i++) {
		instTab[i]->UpdateViewTM(view.affineTM);
		instTab[i]->mesh->buildRenderNormals();
		instTab[i]->TransformGeometry(instTab[i]->objToCam, instTab[i]->normalObjToCam);
	}

	BroadcastNotification(NOTIFY_PRE_RENDERFRAME, (void*)(RenderGlobalContext*)&rendParams);

	if (!BuildMapFiles(t))	{
		//return 0;
	}

	// Display something informative in the progress bar
	if (prog) {
		prog->SetTitle("Rendering...");
		prog->SetSceneStats(lightTab.Count(), 0, 0, instTab.Count(), nNumFaces);
	}

	// And now the moment we've all been waiting for (drumroll please)....

	nExitStatus = RenderImage(rendParams, t, tobm, prog);

	BroadcastNotification(NOTIFY_POST_RENDERFRAME, (void*)(RenderGlobalContext*)&rendParams);

	// Transform geometry back from camera space	
	for (i = 0; i < instTab.Count(); i++) {
		instTab[i]->TransformGeometry(instTab[i]->camToObj, instTab[i]->normalCamToObj);
	}


	// Update the Virtual Frame Buffer.
	Rect r;
	r.top = 0;
	r.bottom = tobm->Height();
	r.left = 0;
	r.right = tobm->Width();
	tobm->RefreshWindow(&r);

	// Write RendInfo to output bitmap
	RenderInfo* ri = tobm->AllocRenderInfo();
	if (ri) {
		ri->projType = rendParams.projType?ProjParallel:ProjPerspective;
		ri->kx = rendParams.xscale;
		ri->ky = rendParams.yscale;
		ri->xc = (float)rendParams.xc;
		ri->yc = (float)rendParams.xc;
		ri->fieldRender = FALSE;	// We don't support field rendering...
		ri->fieldOdd = FALSE;		// We don't support field rendering...
		ri->renderTime[0] = rendParams.time;
		ri->worldToCam[0] = rendParams.worldToCam;
		ri->camToWorld[0] = rendParams.camToWorld;
	}

	bool needFinalRefresh = false;

	// To display the z channel in the VFB properly requires the min and max values:
	// Update these values, and cause the VFB to be refreshed.
	if (tobm->GetGBuffer()) {
		GBuffer *gb = tobm->GetGBuffer();
		gb->UpdateChannelMinMax();
		needFinalRefresh = (origGBufChannels&BMM_CHAN_Z)?1:0;
		}


	// Do render effects here
	if (nExitStatus && rendParams.effect&&rendParams.effect->Active(t))  {
		MyCallback cb(this);
		rendParams.effect->Apply(t, tobm, &rendParams, &cb);
		tobm->RefreshWindow(NULL);
		if (tobm->GetWindow())
			UpdateWindow(tobm->GetWindow());
		needFinalRefresh = FALSE;
		}

	// Delete any gbuffer channels created just for the render.
	chan = tobm->ChannelsPresent()&(~origGBufChannels);
	if (chan) 
		tobm->DeleteChannels(chan);

	if (gb) {
		if (rendParams.gbufReader) {
			gb->DestroyReader(rendParams.gbufReader);
			rendParams.gbufReader = NULL;
			}
		if (rendParams.gbufWriter) {
			gb->DestroyWriter(rendParams.gbufWriter);
			rendParams.gbufWriter = NULL;
			}
		}

	if (needFinalRefresh) {
		tobm->RefreshWindow(NULL);
		if (tobm->GetWindow()) {
			UpdateWindow(tobm->GetWindow());
			}
		}

	// Now it's not the first frame anymore
	bFirstFrame = FALSE;

	// Display something else in the progress bar
	if (prog) {
		prog->SetTitle("Done.");
	}

	return nExitStatus;
}

//***************************************************************************
// Get the parameters for the view
//***************************************************************************

void CJRenderer::GetViewParams(INode* vnode, ViewParams& vp, TimeValue t)
{
	Interval iv;
	const ObjectState& os = vnode->EvalWorldState(t);
	switch (os.obj->SuperClassID()) {
		case CAMERA_CLASS_ID: {
			// compute camera transform
			CameraState cs;
			CameraObject *cam = (CameraObject *)os.obj;
			iv.SetInfinite();

			// Grab the Camera transform from the node.
			Matrix3 camtm = vnode->GetObjTMAfterWSM(t,&iv);

			RemoveScaling(camtm);
			vp.affineTM = Inverse(camtm);
			cam->EvalCameraState(t,iv,&cs);
			if (cs.manualClip) {
				vp.hither = cs.hither;
				vp.yon = cs.yon;
			}
			else {
			    vp.hither	= 0.1f;
		    	vp.yon	  	= -BIGFLOAT;
			}
			vp.projType = PROJ_PERSPECTIVE;
			vp.fov = cs.fov;
			rendParams.nearRange = cs.nearRange;
			rendParams.farRange = cs.farRange;
			}
			break;
		case LIGHT_CLASS_ID: {

			iv.SetInfinite();
			Matrix3 ltm = vnode->GetObjTMAfterWSM(t,&iv);
			vp.affineTM = Inverse(ltm);
			
			LightState ls;
			LightObject *ltob = (LightObject *)os.obj;
			ltob->EvalLightState(t,iv,&ls);

			float aspect = ls.shape?1.0f:ls.aspect;
			switch(ls.type) {
				case SPOT_LGT:			
					vp.projType = PROJ_PERSPECTIVE;      
					vp.fov = DegToRad(ls.fallsize);  
					vp.fov = 2.0f* (float)atan(tan(vp.fov*0.5f)*sqrt(aspect));
					rendParams.devAspect = (float(rendParams.devHeight)/float(rendParams.devWidth))*aspect;
					break;
				case DIRECT_LGT:
					vp.projType = PROJ_PARALLEL; 
					rendParams.devAspect = (float(rendParams.devHeight)/float(rendParams.devWidth))*aspect;
					break;
			}
		    vp.hither	= 0.1f;
	    	vp.yon	  	= -BIGFLOAT;  // so  it doesn't get used

			rendParams.nearRange = 0.0f;
			rendParams.farRange = 500.0f;
			}
			break;
		default:
			rendParams.nearRange = 0.0f;
			rendParams.farRange = 500.0f;
			break;
	}	
}

//***************************************************************************
// This method enumerates the external maps needed by the objects
// and materials.
// If a map is missing its name is appended to a nameTab and we
// report the problem.
// If a map is found it is appended to a map list and
// when done, we manually load each map into the system.
// See the rendutil module for the enumerators.
//***************************************************************************

// LAM - 5/22/03 - added  FILE_ENUM_SKIP_VPRENDER_ONLY flag
#define ENUMMISSING FILE_ENUM_MISSING_ONLY|FILE_ENUM_1STSUB_MISSING|FILE_ENUM_SKIP_VPRENDER_ONLY

int CJRenderer::LoadMapFiles(TimeValue t, HWND hWnd, BOOL firstFrame)
{
	NameTab mapFiles;
	CheckFileNames checkNames(&mapFiles);

	int i;

	// Check the nodes
	for (i = 0; i < instTab.Count(); i++) {
		instTab[i]->GetINode()->EnumAuxFiles(checkNames, ENUMMISSING);
	}

	// Check the lights
	for (i = 0; i < lightTab.Count(); i++) {
		if (lightTab[i]->pLight != NULL) {
			lightTab[i]->pLight->EnumAuxFiles(checkNames, ENUMMISSING);
		}
	}

	// Check atmospherics and environment.
	if (rendParams.envMap) rendParams.envMap->EnumAuxFiles(checkNames, ENUMMISSING );
	if (rendParams.atmos) rendParams.atmos->EnumAuxFiles(checkNames, ENUMMISSING);

	// If we have any missing maps we report it to the user.
	// We should only report these errors on the first frame.
	// Also, if we are network rendering, we do not prompt the user,
	// instead we are writing the status to the log file.

	if (mapFiles.Count() && firstFrame) {
		// TBD: Use new log system
		// Updated to new logging system - GG: 01/29/99
		if (rendParams.bNetRender) {
			// Write out error report to file.
			Interface *ci = GetCOREInterface();
			for ( i=0; i<mapFiles.Count(); i++ )
				ci->Log()->LogEntry(SYSLOG_ERROR,NO_DIALOG,NULL,"Missing Map: %d",mapFiles[i]);
			return 0;
		}
		else {
			if (MessageBox(hWnd, "There are missing maps.\nDo you want to render anyway?", "Warning!", MB_YESNO) != IDYES) {
				return 0;
			}
		}

	}

	// Load the maps
	MapLoadEnum mapload(t);
	for (i=0; i<mtls.Count(); i++) {
		EnumMaps(mtls[i],-1, mapload);
	}

	return 1;
}

//***************************************************************************
// This method enumerates the external maps needed by the objects
// and materials.
// If a map is missing its name is appended to a nameTab and we
// report the problem.
// If a map is found it is appended to a map list and
// when done, we manually load each map into the system.
// See the rendutil module for the enumerators.
//***************************************************************************

int CJRenderer::BuildMapFiles(TimeValue t)
{
	Instance* inst;

	for (inst = ilist; inst!=NULL; inst = inst->next) {
		// Load the maps
		MapSetupEnum mapsetup(t, this, inst);
		EnumMtlTree(inst->mtl, -1, mapsetup);
	}

	return 1;
}



//***************************************************************************
// Cloning the renderer
// This method is required for 3D Studio MAX 2.0
//***************************************************************************

RefTargetHandle CJRenderer::Clone(RemapDir &remap)
{
	CJRenderer* newRend = new CJRenderer();

	// Clone the local options
	newRend->rendParams.nMaxDepth = rendParams.nMaxDepth;
	newRend->rendParams.nAntiAliasLevel = rendParams.nAntiAliasLevel;

	BaseClone(this, newRend, remap);
	return newRend;
}

void CJRenderer::AddInstance(INode* node)
{
	nCurNodeID++;
	Instance* pInst = new Instance(node, &mtls, nCurNodeID);

	pInst->next = ilist;
	ilist = pInst;
	// TBD - initialize stuff here

	instTab.Append(1, &pInst, 25);
}

//***************************************************************************
// A node enumerator.
// These classes enumerates the nodes in the scene for access at render time.
// When enumerated, we create an object of class RenderNode for each node.
// This RenderNode contains the node, assigned materials etc.
// We store a list of the RenderNodes in a list CJRenderer::rNodeTab
// We also have another list CJRenderer::lightTab where we store all
// the lights in the scene.
//***************************************************************************

void CJRenderer::NodeEnum(INode* node)
{
	// For each child of this node, we recurse into ourselves 
	// until no more children are found.
	for (int c = 0; c < node->NumberOfChildren(); c++) {
		NodeEnum(node->GetChildNode(c));
	}

	// Is the node hidden?
	BOOL nodeHidden = node->IsNodeHidden(TRUE);
	
	// Get the ObjectState.
	// The ObjectState is the structure that flows up the pipeline.
	// It contains a matrix, a material index, some flags for channels,
	// and a pointer to the object in the pipeline.
	ObjectState ostate = node->EvalWorldState(0);
	if (ostate.obj==NULL) 
		return;

	// Examine the superclass ID in order to figure out what kind
	// of object we are dealing with.
	switch (ostate.obj->SuperClassID()) {

		// It's a light.
		case LIGHT_CLASS_ID: { 

			// Get the light object from the ObjectState
			LightObject *light = (LightObject*)ostate.obj;

			// Is this light turned on?
			if (light->GetUseLight()) {
				switch (light->GetShadowMethod()) {
					case LIGHTSHADOW_MAPPED:
						// Mapped shadows
						break;
					case LIGHTSHADOW_RAYTRACED:
						// Ratraced shadows
						break;
					}
				}
				RenderLight* rl = new RenderLight(node, &mtls);
				// Create a RenderLight and append it to our list of lights
				lightTab.Append(1, &rl);
			}
			break;
		case SHAPE_CLASS_ID:	// To support renderable shapes
		case GEOMOBJECT_CLASS_ID: {
			// This is an object in the scene

			// If we are not rendering hidden objects, return now
			if (nodeHidden && !rendParams.bRenderHidden)
				return;

			// If this object cannot render, skip it
			if (!ostate.obj->IsRenderable()) 
				return;
			if (!node->Renderable()) 
				return;

			// Handle motion blur etc...
			
			// Add the node to our list
			AddInstance(node);
			break;
		}
	}
}

//***************************************************************************
// Loading and Saving render parameters
// Chunk ID's for loading and saving render data
//***************************************************************************

#define RAYDEPTH_CHUNK		0x120
#define ANTIALIAS_CHUNK		0x130

//***************************************************************************
//	Save the render options in the scene
//***************************************************************************

IOResult CJRenderer::Save(ISave *isave)
{
	ULONG nb;

	isave->BeginChunk(RAYDEPTH_CHUNK);
	isave->Write(&rendParams.nMaxDepth,sizeof(int),&nb);
	isave->EndChunk();

	isave->BeginChunk(ANTIALIAS_CHUNK);
	isave->Write(&rendParams.nAntiAliasLevel,sizeof(int),&nb);
	isave->EndChunk();

	return IO_OK; 
}


//***************************************************************************
//	Load the render options from the scene
//***************************************************************************

IOResult CJRenderer::Load(ILoad *iload)
{
	ULONG nb;
	int id;
	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(id = iload->CurChunkID())  {
			case RAYDEPTH_CHUNK:
				res = iload->Read(&rendParams.nMaxDepth,sizeof(int), &nb);
				break;
			case ANTIALIAS_CHUNK:
				res = iload->Read(&rendParams.nAntiAliasLevel,sizeof(int), &nb);
				break;
		}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
	}
	return IO_OK;
}

//===========================================================================
//
// Class CJRenderParams : public RenderGlobalContext
//
//===========================================================================

//***************************************************************************
// Initialize our custom options.
//***************************************************************************

CJRenderParams::CJRenderParams()
{
	SetDefaults();

	envMap = NULL;
	atmos = NULL;
	rendType = RENDTYPE_NORMAL;
	nMinx = 0;
	nMiny = 0;
	nMaxx = 0;
	nMaxy = 0;
	nNumDefLights = 0;
	nRegxmin = 0;
	nRegxmax = 0;
	nRegymin = 0;
	nRegymax = 0;
	scrDUV = Point2(0.0f, 0.0f);
	pDefaultLights = NULL;
	pFrp = NULL;
	bVideoColorCheck = 0;
	bForce2Sided = FALSE;
	bRenderHidden = FALSE;
	bSuperBlack = FALSE;
	bRenderFields = FALSE;
	bNetRender = FALSE;

	renderer = NULL;
	projType = PROJ_PERSPECTIVE;
	devWidth = 0;
	devHeight = 0;
	xscale = 0;
	yscale = 0;
	xc = 0;
	yc = 0;
	antialias = FALSE;
	nearRange = 0;
	farRange = 0;
	devAspect = 0;
	frameDur = 0;
	time = 0;
	wireMode = FALSE;
	inMtlEdit = FALSE;
	fieldRender = FALSE;
	first_field = FALSE;
	field_order = FALSE;
	objMotBlur = FALSE;
	nBlurFrames = 0;
}

void CJRenderParams::SetDefaults()
{
	nMaxDepth = 0;
	nAntiAliasLevel = AA_NONE;
	bReflectEnv = FALSE;
}

//***************************************************************************
// These values can be assumed to be correct.
// See the SDK help for class ViewParams for an explanation.
//***************************************************************************

#define VIEW_DEFAULT_WIDTH ((float)400.0)

void CJRenderParams::ComputeViewParams(const ViewParams&vp)
{
	worldToCam = vp.affineTM;
	camToWorld = Inverse(worldToCam);

	xc = devWidth / 2.0f;
	yc = devHeight / 2.0f;

	scrDUV.x = 1.0f/(float)devWidth;
	scrDUV.y = 1.0f/(float)devHeight;

	projType = vp.projType;

	if (projType == PROJ_PERSPECTIVE) {
		float fac =  -(float)(1.0 / tan(0.5*(double)vp.fov));
		xscale =  fac*xc;
		yscale = -devAspect*xscale;
	}
	else {
		xscale = (float)devWidth/(VIEW_DEFAULT_WIDTH*vp.zoom);
		yscale = -devAspect*xscale;
	}

	// TBD: Do Blowup calculation here.
}

//***************************************************************************
// Calculate the direction of a ray going through pixels sx, sy
//***************************************************************************

Point3 CJRenderParams::RayDirection(float sx, float sy)
{
	Point3 p;
	p.x = -(sx-xc)/xscale; 
	p.y = -(sy-yc)/yscale; 
	p.z = -1.0f;
	return Normalize(p);
}

//***************************************************************************
// Render Instances (from RenderGlobalContext)
//***************************************************************************

int CJRenderParams::NumRenderInstances()
{
	return ((CJRenderer*)renderer)->instTab.Count();
}

RenderInstance* CJRenderParams::GetRenderInstance(int i)
{
	if (i<NumRenderInstances()) {
		return ((CJRenderer*)renderer)->instTab[i];
	}

	return NULL;
}


//===========================================================================
//
// Class MyView
//
//===========================================================================

//***************************************************************************
// View to screen implementation for the view class
//***************************************************************************

Point2 MyView::ViewToScreen(Point3 p)
{
	return pRendParams->MapToScreen(p);
}



