/**********************************************************************
 *<
	FILE: 3dsimp.cpp

	DESCRIPTION:  .3DS/.SHP file import module

	CREATED BY: Tom Hudson

	HISTORY: created 26 December 1994

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "Max.h"
#include <stdio.h>
#include <direct.h>
#include <commdlg.h>
#include "splshape.h"
#include "3dsires.h"
#include "imtl.h"
#include "dummy.h"
#include "3dsimp.h"
#include "mtldef.h"
#include "istdplug.h"
#include "stdmat.h"
#include "gamma.h"
#include "helpsys.h"
#include "buildver.h"

//#define PRINTCHUNKID

HINSTANCE hInstance;

static BOOL showPrompts;

TCHAR *GetString(int id)
	{
	static TCHAR buf[256];
	if (hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
	}


static int MessageBox(int s1, int s2, int option = MB_OK) {
	TSTR str1(GetString(s1));
	TSTR str2(GetString(s2));
	return MessageBox(GetActiveWindow(), str1, str2, option);
	}

static int MessageBox2(int s1, int s2, int option = MB_OK) {
	TSTR str1(GetString(s1));
	TSTR str2(GetString(s2));
	return MessageBox(GetActiveWindow(), str1, str2, option);
	}

static int MessageBox(TCHAR *s, int s2, int option = MB_OK) {
	TSTR str1(s);
	TSTR str2(GetString(s2));
	return MessageBox(GetActiveWindow(), str1, str2, option);
	}


// 3DS-to-MAX time constant multiplier
#define TIME_CONSTANT GetTicksPerFrame()

//#define DBGPRINT


// The file stream

static FILE *stream;

// The debugging dump stream

static FILE *dStream;

// Some stuff we're defining (for now)

static int merging = 0;
static int lastchunk;
#define OBJ_NAME_LEN 10
static char obname[32];
static int nodeLoadNumber = -1;
static short cur_node_id,nodeflags,nodeflags2;
static int nodetag;
static int skipNode = 0;
static ImpNode *obnode;
static short readVers;
static int skipped_nodes = 0;
static int got_mat_chunk;
static BGdata BG;
static BOOL replaceScene =0;
static BOOL autoConv = 1;
static BOOL importShapes = TRUE;
static BOOL needShapeImportOptions = FALSE;
static BOOL shapeImportAbort = FALSE;

// Some handy macros
static float msc_wk;

void
split_fn(char *path,char *file,char *pf)
	{
	int ix,jx,bs_loc,fn_loc;
	if(strlen(pf)==0) {
		if(path) *path=0;
		if(file) *file=0;
		return;
		}
	bs_loc=strlen(pf);
	for(ix=bs_loc-1; ix>=0; --ix) {
		if(pf[ix]=='\\')  {
			bs_loc=ix;
			fn_loc=ix+1;
			goto do_split;
			}
		if(pf[ix]==':') {
			bs_loc=ix+1;
			fn_loc=ix+1;
			goto do_split;
			}
		}
	bs_loc= -1;
	fn_loc=0;

	do_split:
	if(file)
		strcpy(file,&pf[fn_loc]);
	if(path) {
		if(bs_loc>0)  {
			for(jx=0; jx<bs_loc; ++jx)
				path[jx]=pf[jx];
			path[jx]=0;
			}
		else  path[0]=0;
		}
	}


void fin_degammify(Color *col, Color *gamcol) {
	if (gammaMgr.enable) {
		col->r = deGammaCorrect(gamcol->r, gammaMgr.fileInGamma);
		col->g = deGammaCorrect(gamcol->g, gammaMgr.fileInGamma);
		col->b = deGammaCorrect(gamcol->b, gammaMgr.fileInGamma);
		}
	else *col = *gamcol;
	}

static Color ColorFrom24(Color_24 c) {
	Color a;
	a.r = (float)c.r/255.0f;
	a.g = (float)c.g/255.0f;
	a.b = (float)c.b/255.0f;
	return a;
	}

//==========================================================================
// Setup Environment
//==========================================================================
int ObjWorker::SetupEnvironment() {
	if (!replaceScene) 
		return 1;
	i->SetAmbient(0,BG.amb_light);	
	i->SetBackGround(0,BG.bkgd_solid);	
	i->SetUseMap(FALSE);
	switch(BG.bgType) {
		case BG_SOLID: 
			break;
		case BG_GRADIENT: 
			{
#ifndef NO_MAPTYPE_GRADIENT // orb 01-07-2001
			GradTex *gt = NewDefaultGradTex();
			gt->SetColor(0, BG.bkgd_gradient.topcolor);
			gt->SetColor(1, BG.bkgd_gradient.midcolor);
			gt->SetColor(2, BG.bkgd_gradient.botcolor);
			gt->SetMidPoint(1.0f-BG.bkgd_gradient.midpct);
			gt->GetUVGen()->SetCoordMapping(UVMAP_SCREEN_ENV);
			i->SetEnvironmentMap(gt);
			i->SetUseMap(TRUE);
#endif // NO_MAPTYPE_GRADIENT
			}
			break;
		case BG_BITMAP: 
			if (strlen(BG.bkgd_map)>0) {
				BitmapTex *bmt = NewDefaultBitmapTex();
				bmt->SetMapName(TSTR(BG.bkgd_map));
				bmt->GetUVGen()->SetCoordMapping(UVMAP_SCREEN_ENV);
				i->SetEnvironmentMap(bmt);
				i->SetUseMap(TRUE);
				}
			break;
		}	
	switch(BG.envType) {
		case ENV_DISTCUE:
			//	BG.distance_cue.nearplane; ??
			//	BG.distance_cue.farplane;  ??
			{
			StdFog *fog = NewDefaultStdFog();
			fog->SetType(0);  
			fog->SetColor(Color(0.0f,0.0f,0.0f),0);  
			fog->SetNear(BG.distance_cue.neardim/100.0f,0);  
			fog->SetFar(BG.distance_cue.fardim/100.0f,0);  
			fog->SetFogBackground(BG.dim_bg);
			i->AddAtmosphere(fog);			
			}				
			break;
		case ENV_FOG:{
			StdFog *fog = NewDefaultStdFog();
			fog->SetType(0);  
			fog->SetColor(BG.fog_data.color,0);  
			fog->SetNear(BG.fog_data.neardens/100.0f,0);  
			fog->SetFar(BG.fog_data.fardens/100.0f,0);  
			fog->SetFogBackground(BG.fog_bg);
			i->AddAtmosphere(fog);			
			}
			break;
		case ENV_LAYFOG:{
			StdFog *fog = NewDefaultStdFog();
			fog->SetType(1);  
			int ftype;
			switch(BG.lfog_data.type) {
				case 1:  ftype = FALLOFF_BOTTOM; break;
				case 2:  ftype = FALLOFF_TOP; break;
				default: ftype = FALLOFF_NONE; break;
				}
			fog->SetFalloffType(ftype);  
			fog->SetColor(BG.lfog_data.color,0);  
			fog->SetDensity(BG.lfog_data.density*100.0f,0);  
			fog->SetTop(BG.lfog_data.zmax,0);  
			fog->SetBottom(BG.lfog_data.zmin,0);  
			fog->SetFogBackground(BG.lfog_data.fog_bg);
			i->AddAtmosphere(fog);			
			}
			break;
		}
	i->SetAmbient(0,BG.amb_light);
	return 1;
	}

// DS  8/31/00: this converts NameTab'sto NodeIDTab's after the scene has been loaded.
int ObjWorker::FixupExclusionLists() {
	for (int i=0; i<exclSaver.Count(); i++) {	
		ExclListSaver *s = exclSaver[i];
		ExclList excl;
		GetCOREInterface()->ConvertNameTabToExclList(&s->nametab, &excl);
		s->lt->SetExclusionList(excl);
        delete s;
		}
	return 1;
	}

static int MAXMapIndex(int i) {
	switch(i) {
		case Ntex:  return ID_DI;
		case Ntex2: return ID_DI;
		case Nopac: return ID_OP;
		case Nbump: return ID_BU;
		case Nspec: return ID_SP;
		case Nshin: return ID_SS;
		case Nselfi:return ID_SI; 
		case Nrefl: return ID_RL;
		default:    return ID_DI;
		}
	}

//=========================================================
// Match 3DStudio's "Default" material
static StdMat *New3DSDefaultMtl() {
	StdMat *m = NewDefaultStdMat();
	m->SetName(_T("Default"));
	m->SetAmbient(Color(.7f,.7f,.7f),0);
	m->SetDiffuse(Color(.7f,.7f,.7f),0);
	m->SetSpecular(Color(1.0f,1.0f,1.0f),0);
	m->SetShininess(0.5f,0);
	m->SetShinStr(.707f,0);
	return m;
	}

static BOOL IsSXPName(char *name) {
	char fname[30];
	char ext[5];
	_splitpath(name, NULL, NULL, fname, ext );
	return stricmp(ext,".sxp")==0?1:0;
	}

static Texmap* MakeTex(MapData& map, SMtl *smtl, BOOL &wasSXP) {
	Texmap *txm; 
	wasSXP = FALSE;
	if (map.kind==0) { 
		// Texture Map
//		if (IsSXPName(map.name)&&(map.p.tex.sxp_data!=NULL)) {
		if (IsSXPName(map.name)) {	 // DS - 6/11/96
			Tex3D *t3d = GetSXPReaderClass(map.name);
			if (t3d) {
				if (map.p.tex.sxp_data) {
					ULONG *p = (ULONG *)map.p.tex.sxp_data;
					t3d->ReadSXPData(map.name, (void *)(p+1));
					wasSXP = TRUE;
					}
				}
			txm = t3d;							
			}
		else {
			BitmapTex *bmt = NewDefaultBitmapTex();
			bmt->SetMapName(TSTR(map.name));
			MapParams &par = map.p.tex;
			bmt->SetAlphaAsMono((par.texflags&TEX_ALPHA_SOURCE)?1:0);
			bmt->SetAlphaSource((par.texflags&TEX_DONT_USE_ALPHA)?ALPHA_NONE:ALPHA_FILE);
			bmt->SetFilterType((par.texflags&TEX_SAT)?FILTER_SAT:FILTER_PYR);
			StdUVGen *uv = bmt->GetUVGen();
			uv->SetUOffs(par.uoffset,0);
			uv->SetVOffs(-par.voffset,0);
			uv->SetUScl(par.uscale,0);
			uv->SetVScl(par.vscale,0);
			uv->SetAng(-((float)atan2(par.ang_sin, par.ang_cos)),0);
			uv->SetBlur(par.texblur+1.0f,0);
			int tile=0;
			if (par.texflags&TEX_MIRROR) tile|= U_MIRROR|V_MIRROR;
			else {
				if (0==(par.texflags&TEX_NOWRAP)) tile|= U_WRAP|V_WRAP;
				}
			uv->SetTextureTiling(tile);
			TextureOutput *txout = bmt->GetTexout();
			txout->SetInvert(par.texflags&TEX_INVERT?1:0);
			txm = bmt;
			}
		if (map.p.tex.texflags&TEX_TINT) {
			// map.p.tex.col1, col2	: stuff into Mix	
			MultiTex* mix = NewDefaultMixTex();
			mix->SetColor(0,ColorFrom24(map.p.tex.col1));
			mix->SetColor(1,ColorFrom24(map.p.tex.col2));
			mix->SetSubTexmap(2, txm);
			txm = mix;			
			}
		else if (map.p.tex.texflags&TEX_RGB_TINT) {
			// map.p.tex.rcol,gcol,bcol : stuf into tint
#ifndef NO_MAPTYPE_RGBTINT // orb 01-07-2001
			MultiTex* mix = NewDefaultTintTex();
			mix->SetColor(0,ColorFrom24(map.p.tex.rcol));
			mix->SetColor(1,ColorFrom24(map.p.tex.gcol));
			mix->SetColor(2,ColorFrom24(map.p.tex.bcol));
			mix->SetSubTexmap(0, txm);
			txm = mix;			
#endif // NO_MAPTYPE_RGBTINT
			}
		}
	else {  
		// kind == 1 :  Reflection Map
		BitmapTex *bmt = NewDefaultBitmapTex();
		bmt->SetMapName(TSTR(map.name));
		StdUVGen *uv = bmt->GetUVGen();

		// TBD: REFLECTION BLUR SETTING:
		uv->SetBlurOffs((float)smtl->refblur/400.0f+.001f,0);
		bmt->InitSlotType(MAPSLOT_ENVIRON);
		txm = bmt;
		}


	return txm;
	}

//==========================================================================
// Convert mesh mtl to max standard matl.
//==========================================================================
void ObjWorker::AddMeshMtl(SMtl *smtl) {
	StdMat *m;
	Mesh *mesh = NULL;
	if (smtl==NULL) {
		m = New3DSDefaultMtl();
		loadMtls.AddMtl(m);			
		loadMtlMesh.Append(1,&mesh,10);
		return;
		}
	m = NewDefaultStdMat();
	m->SetName(TSTR(smtl->name));
	int shade;
	switch(smtl->shading) {
		case REND_FLAT:  shade = SHADE_CONST; break;
		case REND_METAL: shade = SHADE_METAL; break;
		default:		 shade = SHADE_PHONG; break;
		}
	m->SetShading(shade);
	m->SetAmbient(ColorFrom24(smtl->amb),0);
	m->SetDiffuse(ColorFrom24(smtl->diff),0);
	m->SetFilter(ColorFrom24(smtl->diff),0);
	m->SetSpecular(ColorFrom24(smtl->spec),0);
	m->SetShininess((float)smtl->shininess/100.0f,0);
	m->SetShinStr((float)smtl->shin2pct/100.0f,0);
	m->SetOpacity(1.0f-(float)smtl->transparency/100.0f,0);
	m->SetOpacFalloff((float)smtl->xpfall/100.0f, 0);		
	m->SetFalloffOut(smtl->flags&MF_XPFALLIN?0:1);  
	m->SetSelfIllum((float)smtl->selfipct/100.0f,0);
	m->SetWireSize(smtl->wiresize,0);
	m->SetFaceMap(smtl->flags&MF_FACEMAP?1:0);
	m->SetSoften(smtl->flags&MF_PHONGSOFT?1:0);
	m->SetWire(smtl->flags&MF_WIRE?1:0);
	m->SetTwoSided(smtl->flags&MF_TWOSIDE?1:0);
	m->SetTransparencyType(smtl->flags&MF_ADDITIVE ? TRANSP_ADDITIVE : TRANSP_FILTER);
	m->SetWireUnits(smtl->flags&MF_WIREABS?1:0);


	if (smtl->map) {
		Texmap *txm;
		float amt,amt0;
		BOOL gotTex=0;
		BOOL dum;
		for (int i=0; i<8; i++) {
			if (smtl->map[i]==NULL) 
				continue;
			Mapping &mp = *(smtl->map[i]);
			int n = MAXMapIndex(i);
			if (i==Nrefl) {
				amt = (float)mp.amt.pct/100.0f;
				RMapParams &par = mp.map.p.ref;
				if (par.acb.flags&AC_ON) {	
					// Mirror or Auto-cubic
					if (par.acb.flags&AC_MIRROR) {
#ifndef NO_MAPTYPE_FLATMIRROR // orb 01-07-2001
						StdMirror *mir = NewDefaultStdMirror();
						txm = (Texmap *)mir;
						mir->SetDoNth(par.acb.flags&AC_FIRSTONLY?0:1);
						mir->SetNth(par.acb.nth);
#endif // NO_MAPTYPE_FLATMIRROR
						}
					else {
#ifndef NO_MAPTYPE_REFLECTREFRACT // orb 01-07-2001
						StdCubic *cub = NewDefaultStdCubic();
						txm = (Texmap *)cub;
						cub->SetSize(par.acb.size,0);
						cub->SetDoNth(par.acb.flags&AC_FIRSTONLY?0:1);
						cub->SetNth(par.acb.nth);
#endif // NO_MAPTYPE_REFLECTREFRACT
						}
					}
				else {	
					// Environment map
					txm = MakeTex(mp.map,smtl,dum);
					}

				if (strlen(mp.mask.name)>0) {
					// make a Mask texmap.
					Texmap *masktex = (Texmap *)CreateInstance(TEXMAP_CLASS_ID, Class_ID(MASK_CLASS_ID,0));
					masktex->SetSubTexmap(1,MakeTex(mp.mask,smtl,dum));
					masktex->SetSubTexmap(0,txm);
					txm = masktex;
					}

				m->SetSubTexmap(n,txm);
				amt = (float)mp.amt.pct/100.0f;
				m->SetTexmapAmt(n, amt, 0);
				}
			else {
					
				// non-reflection maps
				amt = (float)mp.amt.pct/100.0f;

				// DS: 4/30/97 correct for new interpretation of the
				// amount sliders.
				switch(i) {
					case Nopac:	 
						if (amt<1.0f) 
							m->SetOpacity(0.0f,0); 
						break;
					case Nselfi: 
						if (amt<1.0f) 
							m->SetSelfIllum(0.0f,0); 
						break;
					case Nshin:  
						// Shininess mapping in 3DS was really shininess strength mapping
						amt*= (float)smtl->shin2pct/100.0f;
						m->SetShinStr(0.0f,0);
						break;
					}

				BOOL wasSXP;
				txm = MakeTex(mp.map,smtl,wasSXP);
				if (n==ID_BU&&!wasSXP) amt *= 10.0f;
				m->SetTexmapAmt(n, amt, 0);
				if (strlen(mp.mask.name)>0) {
					// make a Mask texmap.
					Texmap *masktex = (Texmap *)CreateInstance(TEXMAP_CLASS_ID, Class_ID(MASK_CLASS_ID,0));
					masktex->SetSubTexmap(1,MakeTex(mp.mask,smtl,dum));
					masktex->SetSubTexmap(0,txm);
					txm = masktex;
					}
				if (i==Ntex2) {
					if (gotTex) {
						// Make a Composite texmap
						MultiTex *comp = NewDefaultCompositeTex();
						comp->SetNumSubTexmaps(2);
						Texmap *tm0 = m->GetSubTexmap(ID_DI);
						comp->SetSubTexmap(0,tm0);
						comp->SetSubTexmap(1,txm);
						m->SetSubTexmap(ID_DI,comp);						
						if (tm0) 
							tm0->SetOutputLevel(0,amt0);
						if (txm)
							txm->SetOutputLevel(0,amt);
						m->SetTexmapAmt(ID_DI,1.0f,0);
						}
					else {
						m->SetSubTexmap(ID_DI,txm);						
						m->SetTexmapAmt(n,amt,0);
						}
					} 
				else 
					m->SetSubTexmap(n,txm);
				if (i==Ntex&&txm) {
					gotTex=1;
					amt0 = amt;
					}
				}
			m->EnableMap(n,mp.use);
			}
		}
	loadMtls.AddMtl(m);			
	loadMtlMesh.Append(1,&mesh,10);
	}

Mtl *ObjWorker::GetMaxMtl(int i) {
	if (i<0) return NULL;
	if (i>=loadMtls.Count()) return NULL;
	return loadMtls[i];
	}

int ObjWorker::GetMatNum(char *name) {
	TSTR s(name);
	for (int i=0; i<loadMtls.Count(); i++) {
		if (_tcscmp(s,loadMtls[i]->GetName())==0)
			return i;
		}
	assert(0);
	return 0;
	}

void ObjWorker::AssignMtl(INode *theINode, Mesh *mesh) {
	short used[256],cused[256],remap[256];
	int i;

	// First check if another instance of this mesh has already
	// had a material assigned to it.
	for (i=0; i<loadMtlMesh.Count(); i++) {
		if (loadMtlMesh[i]==mesh) {
			theINode->SetMtl(loadMtls[i]);
			return ;
			}
		}
	for (i=0; i<256; i++) used[i] = 0;
	// See if a multi-mtl is required.
	int nmtl,numMtls=0;
	for (i =0; i<mesh->numFaces; i++) {
		nmtl = mesh->faces[i].getMatID();
		assert(nmtl<256);
		if (!used[nmtl]) {
			used[nmtl] = 1;
			remap[nmtl] = numMtls;
			cused[numMtls++] = nmtl;
			}
		}
	if (numMtls>1) { 
		// Need a Multi-mtl
		// scrunch the numbers down to be local to this multi-mtl
		for (i =0; i<mesh->numFaces; i++) 	{
			Face &f = mesh->faces[i];
			int id = f.getMatID();
			f.setMatID(remap[id]);
			}
		// create a new multi with numMtls, and set them
		// to GetMaxMtl(cused[i]), i==0..numMtls-1
		MultiMtl *newmat = NewDefaultMultiMtl();
		newmat->SetNumSubMtls(numMtls);
		for (i=0; i<numMtls; i++) 
			newmat->SetSubMtl(i,GetMaxMtl(cused[i]));
		theINode->SetMtl(newmat);
		loadMtls.AddMtl(newmat);			
		loadMtlMesh.Append(1,&mesh,10);
		}
	else {
		if (mesh->getNumFaces()) {
			nmtl = mesh->faces[0].getMatID();
			for (i =0; i<mesh->numFaces; i++) 
				mesh->faces[i].setMatID(0);
			theINode->SetMtl(GetMaxMtl(nmtl));
			loadMtlMesh[nmtl] = mesh;
			}
		}
	}


int CountRefs(ReferenceTarget *rt) {
	DependentIterator di(rt);
	int nrefs = 0;
	RefMakerHandle rm;
	while (NULL!=(rm=di.Next())) 
		nrefs++;
	return nrefs;			
	}

void ObjWorker::FreeUnusedMtls() {
	for (int i=0; i<loadMtls.Count(); i++) {
		if (CountRefs(loadMtls[i])==0) {
			loadMtls[i]->DeleteThis();
			}			
		}
	loadMtls.SetCount(0);
	}



//===========================================================


// Handy pointers to importers
StudioShapeImport *theShapeImport = NULL;
StudioImport *theImport = NULL;

// ObjWorker class

ObjWorker::ObjWorker(ImpInterface *iptr,Interface *ip) {
	okay=1;
	i=iptr;
	shape=NULL;
	objects=NULL;
	nodes=NULL;
	dummy=NULL;
	isDummy=0;
	Reset();
	lengthSet = FALSE;
	segmentSet = FALSE;
	appdata = NULL;
	appdataLen = 0;
	gotM3DMAGIC=FALSE;
	this->ip = ip;
	for (int i=0;i<256;i++) {
		sceneMtls[i] = NULL;
		mtlNames[i].s[0] = 0;
		}
	hook_x = hook_y = 0.0f;
	}

void
ObjWorker::Reset() {
	mode=WORKER_IDLE;
	tm.IdentityMatrix();
	gotverts=gottverts=gotfaces=verts=faces=0;
	id = -1;
	object=NULL;
	light=NULL;
	camera=NULL;
	dummy = NULL;
	isDummy = 0;
	mesh=NULL;
	shape=NULL;
	spline=NULL;
	splShape = NULL;
	parentNode = thisNode = NULL;
	// NEW INITS
	workNode = NULL;	
	pivot = Point3(0,0,0);
	cstShad = 1;
	rcvShad = 1;
	newTV.SetCount(0);
	}

int
ObjWorker::StartMesh(const char *iname) {
	if(!FinishUp())
		return 0;
//DebugPrint("Starting mesh %s\n",iname);
	if(dStream) {
		fprintf(dStream,"Starting mesh:%s\n",iname);
		fflush(dStream);
		}
	tm.IdentityMatrix();
	name = TSTR(iname);
	object = CreateNewTriObject();
	if(!object)
		return 0;
	mesh = &object->GetMesh();
	mode = WORKER_MESH;
	newTV.SetCount(0);
	return 1;
	}

int
ObjWorker::StartLight(const char *iname) {
	if(!FinishUp())
		return 0;
	if(dStream) {
		fprintf(dStream,"Starting mesh:%s\n",iname);
		fflush(dStream);
		}
	tm.IdentityMatrix();
	name = TSTR(iname);
	light = NULL;
	mode = WORKER_LIGHT;	
	return 1;
	}

int
ObjWorker::CreateLight(int type) {
	assert(mode==WORKER_LIGHT);
	light = i->CreateLightObject(type);
	if(!light) {
		DebugPrint("Light type %d create failure\n",type);
		return 0;
		}
//DebugPrint("Created light obj %d/%p\n",type,light);
	lightType = (type == OMNI_LIGHT) ? OBJ_OMNILIGHT : OBJ_SPOTLIGHT;

	SuspendAnimate();
	AnimateOff();

	Point3 col;
	col[0] = studioLt.color[0];
	col[1] = studioLt.color[1];
	col[2] = studioLt.color[2];
	light->SetRGBColor(0,col);
	light->SetUseLight(studioLt.flags&NO_LT_ON);
	light->SetSpotShape(studioLt.flags&NO_LT_RECT ? RECT_LIGHT : CIRCLE_LIGHT);
	light->SetConeDisplay(studioLt.flags&NO_LT_CONE ? 1 : 0, FALSE);
	light->SetUseAtten(studioLt.flags&NO_LT_ATTEN ? 1 : 0);
	light->SetAttenDisplay(0);
	light->SetUseGlobal(studioLt.flags&NO_LT_LOCAL ? 0 : 1);
	light->SetShadow(studioLt.flags&NO_LT_SHAD ? 1 : 0);
	light->SetOvershoot(studioLt.flags&NO_LT_OVER ? 1 : 0);
	light->SetShadowType(studioLt.flags&NO_LT_RAYTR ? 1 : 0);
	light->SetAtten(0, ATTEN_START, studioLt.in_range);
	light->SetAtten(0, ATTEN_END, studioLt.out_range);
	light->SetIntensity(0, studioLt.mult);
	float aspect = 1.0f/studioLt.aspect;	
	light->SetAspect(0, aspect);	
	light->SetMapBias(0, studioLt.lo_bias);
	light->SetMapRange(0, studioLt.shadfilter);
	light->SetMapSize(0, studioLt.shadsize);
	light->SetRayBias(0, studioLt.ray_bias);
	// ? light->SetAbsMapBias(int a);
	ExclList tmpList;

	// DS 8/31/00 COMMENTING THIS OUT.  THE exclList must be converted
	// to a NodeIDTab AFTER the scene is loaded.		
//	light->SetExclusionList(studioLt.excList);

	// Save light and associated list
	ExclListSaver* sv = new ExclListSaver();
	sv->lt = light;
	sv->nametab = studioLt.excList;
	exclSaver.Append(1,&sv);

	light->SetHotspot(0,studioLt.hotsize);
	light->SetFallsize(0,studioLt.fallsize);
	studioLt.excList.SetSize(0);

	ResumeAnimate();
	return 1;
	}

int
ObjWorker::StartCamera(const char *iname) {
	if(!FinishUp())
		return 0;
	if(dStream) {
		fprintf(dStream,"Starting mesh:%s\n",iname);
		fflush(dStream);
		}
	tm.IdentityMatrix();
	name = TSTR(iname);
	camera = NULL;
	mode = WORKER_CAMERA;
	return 1;
	}

int
ObjWorker::CreateCamera(int type) {
	assert(mode==WORKER_CAMERA);
	camera = i->CreateCameraObject(type);
	camera->SetEnvRange(0, 0, studioCam.nearplane);
	camera->SetEnvRange(0, 1, studioCam.farplane);
	camera->SetFOV(0, DegToRad(2400.0/studioCam.focal));

	if(!camera) {
		DebugPrint("Camera type %d create failure\n",type);
		return 0;
		}
//DebugPrint("Created camera obj %d/%p\n",type,camera);
	return 1;
	}

int
ObjWorker::StartKF(ImpNode *node) {
	if(!FinishUp())
		return 0;
	if(dStream) {
		fprintf(dStream,"Starting KF node\n");
		fflush(dStream);
		}
	thisNode = node;
	parentNode = NULL;
	mode = WORKER_KF;
	pivot = Point3(0,0,0);	// Default pivot
	return 1;
	}

int
ObjWorker::StartShape() {
	if(!FinishUp())
		return 0;
	if(dStream) {
		fprintf(dStream,"Starting shape\n");
		fflush(dStream);
		}
	if(!object) {
		tm.IdentityMatrix();
		splShape = new SplineShape;
		if(!splShape)
			return 0;
		shape = &splShape->GetShape();
		shape->NewShape();
		}
	mode = WORKER_SHAPE;
	return 1;
	}

int
ObjWorker::StartSpline() {
	// If there's a shape already, and we're outputting multiple shapes, output it!
	if(shape && theShapeImport && theShapeImport->importType == MULTIPLE_SHAPES) {
		FinishShape();
		FinishUp();
		}

	// If no shape yet, start one!
	if(!shape) {
		if(!StartShape())
			return 0;
		}
	spline = shape->NewSpline();
	if(!spline)
		return 0;
	return 1;
	}

int
ObjWorker::AddShapePoint(Shppt *p) {
	if(!spline)
		return 0;
	int type = KTYPE_BEZIER_CORNER;	// Default to corner
	// See if this is a smooth bezier
	Point3 invec(p->inx,p->iny,p->inz);
	float inlen = Length(invec);
	Point3 outvec(p->outx,p->outy,p->outz);
	float outlen = Length(outvec);
	if(inlen != 0.0f && outlen != 0.0f) {
		Point3 normin = Normalize(invec);
		Point3 normout = Normalize(outvec);
		Point3 lowtest = -normin * 0.98f;
		Point3 hightest = -normin * 1.02f;
		if(((normout.x >= lowtest.x && normout.x <= hightest.x) || (normout.x <= lowtest.x && normout.x >= hightest.x)) && 
		   ((normout.y >= lowtest.y && normout.y <= hightest.y) || (normout.y <= lowtest.y && normout.y >= hightest.y)) && 
		   ((normout.z >= lowtest.z && normout.z <= hightest.z) || (normout.z <= lowtest.z && normout.z >= hightest.z)))
		   	type = KTYPE_BEZIER;
		}	
	spline->AddKnot(SplineKnot(type,LTYPE_CURVE,Point3(p->x,p->y,p->z),
		Point3(p->x+p->inx,p->y+p->iny,p->z+p->inz),Point3(p->x+p->outx,p->y+p->outy,p->z+p->outz)));
	return 1;
	}

int
ObjWorker::CloseSpline() {
	if(!spline)
		return 0;
	spline->SetClosed();
	return 1;
	}

int
ObjWorker::FinishShape() {
	if(!shape)
		return 0;
	shape->UpdateSels();	// Make sure it readies the selection set info
	shape->InvalidateGeomCache();
	return 1;
	}



#define NextChunk(hdr)  ((Chunk_hdr *)(((char  *)hdr)+hdr->size))
#define SubChunk(hdr)  ((Chunk_hdr *)(((char  *)hdr)+6))

typedef unsigned short ushort;

typedef struct {
	ushort id;
	ulong length;
	char data[100];
	} ChunkHdr;

void whoa(){}

static Object *CreateObjectFromAppData(
		TriObject *tobj, void *data, DWORD len)
	{
	Chunk_hdr *sub, *hdr = (Chunk_hdr*)data;
	Object *obj;
	char idstring[100];
	int noff, nbytes = (int)len;	

	noff = 0;

	// DS 6/7/96: put in check for hdr->size<0 to catch bad appdata
	// chunks.
	while (noff<nbytes) {
		if (hdr->tag != XDATA_ENTRY) goto next; // if it aint XDATA_ENTRY we don't want it
		sub = SubChunk(hdr);
		if (sub->tag!= XDATA_APPNAME) goto next;
		
		// Try to convert it to an object
		GetIDStr((char*)hdr,idstring);
		obj = ObjectFromAppData(tobj, idstring, (void*)sub, hdr->size-6);
		if (obj) return obj;
		
	  next:
		if (hdr->size<0) break; 
		noff += hdr->size;
		hdr = NextChunk(hdr);
		}

	return NULL;
	}


int
ObjWorker::FinishUp() {
	switch(mode) {
		case WORKER_IDLE:
			return 1;	// Nothing to do, return success!
		case WORKER_MESH: {
			
			// Must have defined verts!
			if(gotverts) {

				// Make this an inverse of the mesh matrix.  This sets up a transform which will
				// be used to transform the mesh from the 3DS Editor-space to the neutral space
				// used by the keyframer.
#ifdef DBGPRINT
DebugPrint("WORKER_MESH TM:\n");
for(int i = 0; i < 4; ++i) {
	Point3 p = tm.GetRow(i);
	DebugPrint("%.4f %.4f %.4f\n",p.x,p.y,p.z);
	}
#endif	
				Matrix3 itm = Inverse(tm);
#ifdef DBGPRINT
DebugPrint("WORKER_MESH inverse TM:\n");
for(i = 0; i < 4; ++i) {
	Point3 p = tm.GetRow(i);
	DebugPrint("%.4f %.4f %.4f\n",p.x,p.y,p.z);
	}
#endif	
				int ix;	
		
				// Transform verts through the inverted mesh transform
				for(ix=0; ix<verts; ++ix) {
					Point3 &p = mesh->getVert(ix);
					p = p * itm;
					mesh->setVert(ix,p);
					}

				/* Check for objects that have been flipped: their 3D
					"parity" will be off */    
				Point3 cp = CrossProd(tm.GetRow(0),tm.GetRow(1));
				if (DotProd(cp,tm.GetRow(2))<0) {
					Matrix3 tmFlipX(1);
					Point3 row0 = tmFlipX.GetRow(0);
					row0.x = -1.0f;
					tmFlipX.SetRow(0,row0);
					// Transform verts through the mirror transform
					for(ix=0; ix<verts; ++ix) {
						Point3 &p = mesh->getVert(ix);
						p = p * tmFlipX;
						mesh->setVert(ix,p);
						}
					}

				mesh->buildNormals();
				mesh->EnableEdgeList(1);				
				}

			if(gottverts) {
				int ntv;
				if ((ntv=newTV.Count())>0) {
					int oldn = mesh->numTVerts;
					mesh->setNumTVerts(oldn+ntv, TRUE); 
					for (int i=0; i<ntv; i++)
						mesh->tVerts[oldn+i] = newTV[i];
					}
				}

			if (appdata) {
				// See if we can create an object from the appdata
				Object *obj = 
					CreateObjectFromAppData(object,appdata,appdataLen);
				if (obj) {
					obj->AddAppDataChunk(
						triObjectClassID, 
						GEOMOBJECT_CLASS_ID, 
						0, appdataLen, appdata);
					// first need to save away its mtl num
					int mnum = -1;
					if (mesh&&mesh->numFaces)
						mnum = mesh->faces[0].getMatID();
					// Then can toss it
					object->DeleteThis(); // don't need tri object
					object = NULL;
					AddObject(obj,OBJ_OTHER,name,NULL,mnum);
					appdata = NULL;  //DS 3/27/96
					Reset();
					return 1;
					}

				// Stick app data in the object
				object->AddAppDataChunk(
					triObjectClassID, 
					GEOMOBJECT_CLASS_ID, 
					0, appdataLen, appdata);
				appdata = NULL;
				}

			AddObject(object,OBJ_MESH,name,&tm);
			if(dStream) {
				fprintf(dStream,"Adding object %s\n",CStr(name));
				fflush(dStream);
				}
			Reset();
			return 1;
			}

		case WORKER_KF: {
			
			if (appdata) {				
				// Stick app data in the node
				thisNode->GetINode()->AddAppDataChunk(
					thisNode->GetINode()->ClassID(), 
					thisNode->GetINode()->SuperClassID(), 
					0, appdataLen, appdata);
				appdata = NULL;
				}

			if(dStream) {
				fprintf(dStream,"Finalizing %s\n",CStr(NodeName(thisNode)));
				fflush(dStream);
				}

			int type = FindTypeFromNode(thisNode, &mesh);

			if(parentNode) {
				if(dStream)
					{
					fprintf(dStream,"Attaching %s to %s\n",CStr(NodeName(thisNode)),CStr(NodeName(parentNode)));
					fflush(dStream);
					}
				parentNode->GetINode()->AttachChild(thisNode->GetINode());
				}

 			switch(type) {
				case OBJ_MESH:
				case OBJ_DUMMY:
				case OBJ_TARGET:
				case OBJ_OTHER:
					thisNode->SetPivot(-pivot);
					break;
				}
			Reset();
			return 1;
			}
		case WORKER_SHAPE:

			// Must have defined shape object!
			if(splShape) {
				ImpNode *node = i->CreateNode();
				if(!node) {
					Reset();
					return 0;
					}
				node->Reference(splShape);
				tm.IdentityMatrix();		// Reset initial matrix to identity
				tm.SetTrans(Point3(hook_x, hook_y, 0.0f));	// TH 3/30/96
				node->SetTransform(0,tm);
				i->AddNodeToScene(node);
				TSTR name;
				name.printf(GetString(IDS_DB_SHAPE_NUM),theShapeImport->shapeNumber++);
				node->SetName(name);
				Reset();
				return 1;
				}

			if(!shape && showPrompts)
				MessageBox(name,IDS_DB_MISSING_SHAPE);

			shape = NULL;
			Reset();
			return 1; 
		case WORKER_LIGHT: {
			if(dStream) {
				fprintf(dStream,"Finalizing light %s\n",CStr(NodeName(thisNode)));
				fflush(dStream);
				}

			// Must have a light
			if(light) {
//DebugPrint("Light calling AddObject %p (%d) %s\n",light,lightType,name);
				AddObject(light, lightType, name, &tm);				
				objects->targPos = 
					Point3(studioLt.tx,studioLt.ty,studioLt.tz);
				objects->srcPos = 
					Point3(studioLt.x,studioLt.y,studioLt.z);
				Reset();
				return 1;
				}

			if(showPrompts)
				MessageBox(name,IDS_DB_LIGHT_ERROR);
			
			delete light;
			Reset();
			return 1;
			}
		case WORKER_CAMERA:
			if(dStream) {
				fprintf(dStream,"Finalizing camera %s\n",CStr(NodeName(thisNode)));
				fflush(dStream);
				}

			// Must have a camera
			if(camera) {
				AddObject(camera, OBJ_CAMERA, name, &tm);
				objects->targPos = 
					Point3(studioCam.tx,studioCam.ty,studioCam.tz);
				objects->srcPos = 
					Point3(studioCam.x,studioCam.y,studioCam.z);
				Reset();
				return 1;
				}

			if(showPrompts)
				MessageBox(name,IDS_DB_CAMERA_ERROR);
			
			delete camera;
			Reset();
			return 1;
		}

	// Undefined state!!!
	return 0;
	}


int ObjWorker::LoadAppData(FILE *stream,DWORD chunkSize)
	{
	appdata = malloc(chunkSize);
	if (!appdata) return 0;
	appdataLen = chunkSize;
	RDERR(appdata,appdataLen);
	return 1;
	}



int
ObjWorker::SetDummyBounds(Point3& dMin,Point3& dMax) {
   	if(!isDummy)
		return 0;

	dummy->SetBox(Box3(dMin,dMax));	

#if 0
   	float dummyVertX[] =
		{(float)dMin.x,(float)dMax.x,(float)dMax.x,(float)dMin.x,(float)dMin.x,(float)dMax.x,(float)dMax.x,(float)dMin.x};
	float dummyVertY[] =
		{(float)dMin.y,(float)dMin.y,(float)dMax.y,(float)dMax.y,(float)dMin.y,(float)dMin.y,(float)dMax.y,(float)dMax.y};
	float dummyVertZ[] =
		{(float)dMin.z,(float)dMin.z,(float)dMin.z,(float)dMin.z,(float)dMax.z,(float)dMax.z,(float)dMax.z,(float)dMax.z};

	int ix;

	Mesh *mesh = &dummy->Mesh();

	// Stuff verts
	for(ix=0; ix<8; ++ix)
		mesh->setVert(ix,Point3(dummyVertX[ix],dummyVertY[ix],dummyVertZ[ix]));

	mesh->buildNormals();
#endif
	return 1;
	}

void
ObjWorker::SetInstanceName(ImpNode *node, const TCHAR *iname) {
	TSTR instancename(iname);
	if(!nodename.Length() && instancename.Length())
		node->SetName(iname);
	else
	if(!instancename.Length() && nodename.Length())
		node->SetName(nodename);
	else {
		TSTR fullname = nodename + TSTR(_T(".")) + iname;
		node->SetName(fullname);
		}
	}

int
ObjWorker::ReadyDummy() {
	if(dummy)
		return 1;		// Already exists!

	dummy = new DummyObject();
	dummy->SetBox(Box3(
		-Point3(0.5f,0.5f,0.5f),
		 Point3(0.5f,0.5f,0.5f)));

#if 0
	static int dummyFaceA[] =
		{0,2,0,5,1,6,2,7,3,4,4,6};
	static int dummyFaceB[] =
		{3,1,1,4,2,5,3,6,0,7,5,7};
	static int dummyFaceC[] =
		{2,0,5,0,6,1,7,2,4,3,6,4};

	// Create a triobj (for now)
	TriObject *object = CreateNewTriObject();
	if(!object)
		return 0;
	mesh = &object->Mesh();

	if(!mesh->setNumVerts(8)) {
		delete object;
		return 0;
		}
	if(!mesh->setNumFaces(12)) {
		delete object;
		return 0;
		}

	dummy = object;

	int ix;

	// Stuff faces
	for(ix=0; ix<12; ++ix) {
		Face *fp = &mesh->faces[ix];
		fp->setVerts(dummyFaceA[ix],dummyFaceB[ix],dummyFaceC[ix]);
		fp->setEdgeVisFlags(1,1,0);
		fp->setSmGroup(1 << (ix/2));
		}

	// Stuff verts
	SetDummyBounds(Point3((float)-0.5,(float)-0.5,(float)-0.5),Point3((float)0.5,(float)0.5,(float)0.5));

 	mesh->EnableEdgeList(1);
#endif
	return 1;
	}

ImpNode *
ObjWorker::MakeDummy(const TCHAR *name) {
	if(!ReadyDummy())
		return NULL;
	ImpNode *node = i->CreateNode();
	if(!node || !dummy) {
		return NULL;
		}
	node->Reference(dummy);
	tm.IdentityMatrix();		// Reset initial matrix to identity
	node->SetTransform(0,tm);
	i->AddNodeToScene(node);
	node->SetName(name);
	AddNode(node,name,OBJ_DUMMY,mesh,"");
	return node;
	}

ImpNode *
ObjWorker::MakeANode(const TCHAR *name, BOOL target, char *owner) {
	int type, cstShad, rcvShad, mtlNum;
	Object *obj;
	// For now, it MUST have an object unless it's a target!
	if(!target) {
		// RB: changed cou
		if(!(obj = (Object *)FindObject((TCHAR*)name, type, cstShad, rcvShad, mtlNum))) {
			if(showPrompts)
				MessageBox((TCHAR *)name,IDS_DB_NO_OBJECT);
			return NULL;
			}
		UseObject((TCHAR*)name);
		}
	else {
		type = OBJ_TARGET;
		obj = i->CreateTargetObject();
		}
	if(type==OBJ_MESH)
		mesh = &(((TriObject *)obj)->GetMesh());
	else
		mesh = NULL;
	ImpNode *node = i->CreateNode();
	if(!node)
		return NULL;
	node->Reference((ObjectHandle)obj);
	tm.IdentityMatrix();		// Reset initial matrix to identity
	node->SetTransform(0,tm);
	node->GetINode()->SetCastShadows(cstShad);
	node->GetINode()->SetRcvShadows(rcvShad);
	i->AddNodeToScene(node);
	node->SetName(name);
	AddNode(node,name,type,mesh,owner,mtlNum);
	return node;
	}

int
ObjWorker::SetVerts(int count) {
	if(gotverts) {
		if(showPrompts)
			MessageBox(name,IDS_DB_HAS_VERTS);
		return 0;
		}
	if(!mesh->setNumVerts(count)) {
		if(showPrompts)
			MessageBox(name,IDS_DB_NUMVERTS_FAIL);
		return 0;
		}
	verts = count;
	gotverts = 1;
	return 1;
	}

int
ObjWorker::SetTVerts(int count) {
	if(gottverts) {
		if(showPrompts)
			MessageBox(name,IDS_DB_HAS_TVERTS);
		return 0;
		}
	if(!mesh->setNumTVerts(count)) {
		if(showPrompts)
			MessageBox(name,IDS_DB_NUMVERTS_FAIL);
		return 0;
		}
//DebugPrint("Set %d tverts\n",count);
	tverts = count;
	gottverts = 1;
	return 1;
	}

int
ObjWorker::SetFaces(int count) {
	if(gotfaces) {
		if(showPrompts)
			MessageBox(name,IDS_DB_HAS_FACES);
		return 0;
		}
	if(!mesh->setNumFaces(count)) {
		if(showPrompts)
			MessageBox(name,IDS_DB_NUMFACES_FAIL);
		return 0;
		}
	// If got texture vertices, set up an equal number of texture faces
	if(gottverts) {
		if(!mesh->setNumTVFaces(count)) {
			if(showPrompts)
				MessageBox(name,IDS_DB_NUMTVFACES_FAIL);
			return 0;
			}
		}
	faces = count;
	gotfaces = 1;
	return 1;
	}

int
ObjWorker::PutVertex(int index,Verts *v) {
	if(!gotverts) {
		if(showPrompts)
			MessageBox(IDS_DB_PUT_NO_VERTS,IDS_DB_3DSIMP);
		return 0;
		}
	if(index<0 || index>=verts) {
		if(showPrompts)
			MessageBox(IDS_DB_VERTS_OR,IDS_DB_3DSIMP);
		return 0;
		}
	mesh->setVert(index,v->x,v->y,v->z);
	return 1;
	}

int
ObjWorker::PutTVertex(int index,UVVert *v) {
	if(!gottverts) {
		if(showPrompts)
			MessageBox(IDS_DB_PUT_NO_TVERTS,IDS_DB_3DSIMP);
		return 0;
		}
	if(index<0 || index>=tverts) {
		if(showPrompts)
			MessageBox(IDS_DB_TVERTS_OR,IDS_DB_3DSIMP);
		return 0;
		}
	mesh->setTVert(index,*v);
	return 1;
	}

static void check_for_wrap(UVVert *tv, int flags) {
	float d;
	if (flags&UWRAP) {
		float maxu,minu;
		maxu = minu = tv[0].x;
		if (tv[1].x>maxu) maxu = tv[1].x;
		else if (tv[1].x<minu) minu = tv[1].x;
		if (tv[2].x>maxu) maxu = tv[2].x;
		else if (tv[2].x<minu) minu = tv[2].x;
		if ((maxu-minu)>0.8f) {
			d = (float)ceil(maxu-minu);
			if (tv[0].x<.5f)  tv[0].x += d; 
			if (tv[1].x<.5f)  tv[1].x += d; 
			if (tv[2].x<.5f)  tv[2].x += d; 
			}
		}
	if (flags&VWRAP) {
		float maxv,minv;
		maxv = minv = tv[0].y;
		if (tv[1].y>maxv) maxv = tv[1].y;
		else if (tv[1].y<minv) minv = tv[1].y;
		if (tv[2].y>maxv) maxv = tv[2].y;
		else if (tv[2].y<minv) minv = tv[2].y;
		if ((maxv-minv)>0.8f) {
			d = (float)ceil(maxv-minv);
			if (tv[0].y<.5f)  tv[0].y += d; 
			if (tv[1].y<.5f)  tv[1].y += d; 
			if (tv[2].y<.5f)  tv[2].y += d; 
			}
		}
	}

DWORD ObjWorker::AddNewTVert(UVVert p) {
	// see if already have it in the new verts.
	int ntv = newTV.Count();
	for (int i=0; i<ntv; i++)
		if (p == newTV[i]) return i + mesh->numVerts;
	// otherwise, add it.
	return 	(DWORD)newTV.Append(1,&p,10) + mesh->numVerts;
	}


void 
ObjWorker::SetTVerts(int nf, Faces *f) {
	UVVert uv[3], uvnew[3]; 
	DWORD ntv[3];
	uvnew[0] = uv[0] = mesh->tVerts[f->a];
	uvnew[1] = uv[1] = mesh->tVerts[f->b];
	uvnew[2] = uv[2] = mesh->tVerts[f->c];
	check_for_wrap(uvnew, f->flags);
	ntv[0] = (uvnew[0]==uv[0]) ? f->a: AddNewTVert(uvnew[0]);
	ntv[1] = (uvnew[1]==uv[1]) ? f->b: AddNewTVert(uvnew[1]);
	ntv[2] = (uvnew[2]==uv[2]) ? f->c: AddNewTVert(uvnew[2]);
	TVFace *tf = &mesh->tvFace[nf];
	tf->setTVerts(ntv[0], ntv[1], ntv[2]);
	}

int
ObjWorker::PutFace(int index,Faces *f) {
	Face *fp = &mesh->faces[index];
	if(!gotfaces) {
		if(showPrompts)
			MessageBox(IDS_DB_PUT_NO_FACES,IDS_DB_3DSIMP);
		return 0;
		}
	if(index<0 || index>=faces)	{
		if(showPrompts)
			MessageBox(IDS_DB_FACES_OR,IDS_DB_3DSIMP);
		return 0;
		}
	// If we've got texture vertices, also put out a texture face
	if(gottverts) {
		// DS 6/6/96
		// check for wrap.
// fix 990920  --prs.
        int q = mesh->getNumTVerts();
        if (f->a < 0 || f->a >= q || f->b < 0 || f->b >= q ||
            f->c < 0 || f->c >= q) {
            if(showPrompts)
                MessageBox(IDS_PRS_TVERT_OR,IDS_DB_3DSIMP);
            return 0;
	        }
//        
		SetTVerts(index,f);
//		TVFace *tf = &mesh->tvFace[index];
//		tf->setTVerts((DWORD)f->a, (DWORD)f->b, (DWORD)f->c);
		}
// fix 990920  --prs.
    int qq = mesh->getNumVerts();
    if (f->a < 0 || f->a >= qq || f->b < 0 || f->b >= qq ||
        f->c < 0 || f->c >= qq) {
        if(showPrompts)
            MessageBox(IDS_PRS_VERT_OR,IDS_DB_3DSIMP);
        return 0;
	    }
//
	fp->setVerts((int)f->a,(int)f->b,(int)f->c);
	fp->setEdgeVisFlags(f->flags & ABLINE,f->flags & BCLINE,f->flags & CALINE);
	fp->setSmGroup(f->sm_group);
	return 1;
	}


int ObjWorker::PutFaceMtl(int index, int imtl) {
	if (index<mesh->numFaces) {
		mesh->faces[index].setMatID(imtl);
		return 1;
		}
	else 
		return 0;
	}

int
ObjWorker::PutSmooth(int index,unsigned long smooth) {
	Face *fp = &mesh->faces[index];
	if(!gotfaces) {
		if(showPrompts)
			MessageBox(IDS_DB_SMOOTH_NO_FACES,IDS_DB_3DSIMP);
		return 0;
		}
	if(index<0 || index>=faces)	{
		if(showPrompts)
			MessageBox(IDS_DB_SMFACE_OR,IDS_DB_3DSIMP);
		return 0;
		}
	fp->setSmGroup(smooth);
	return 1;
	}

void
ObjWorker::Abandon() {
	okay = 0;
	switch(mode) {
		case WORKER_MESH:
			delete object;
			break;
		case WORKER_KF:
			break;
		case WORKER_SHAPE:
			delete splShape;
			break;
		}
	Reset();
	}

void
ObjWorker::FreeObjList() {
	WkObjList *ptr = objects;
	while(ptr) {
		WkObjList *next = (WkObjList *)ptr->next;
		free(ptr);
		ptr = next;
		}
	objects = NULL;
	}

void
ObjWorker::FreeNodeList() {
	WkNodeList *ptr = nodes;
	while(ptr) {
		WkNodeList *next = (WkNodeList *)ptr->next;
		FreeKeyList(&ptr->posList);
		FreeKeyList(&ptr->rotList);
		FreeKeyList(&ptr->scList);
		FreeKeyList(&ptr->colList);
		FreeKeyList(&ptr->hotList);
		FreeKeyList(&ptr->fallList);
		FreeKeyList(&ptr->fovList);
		FreeKeyList(&ptr->rollList);
		free(ptr);
		ptr = next;
		}
	}

int
ObjWorker::AddObject(Object *obj, int type, const TCHAR *name, Matrix3 *tm, int mtlNum) {
	WkObjList *ptr = new WkObjList;
	if(!ptr)
		return 0;
	ptr->object = obj;
	ptr->name = name;
	ptr->type = type;
	ptr->used = 0;
	ptr->cstShad = cstShad;
	ptr->rcvShad = rcvShad;
	ptr->next = objects;
	ptr->mtln = mtlNum;
	if (tm) ptr->tm = *tm;
	else ptr->tm.IdentityMatrix();
	objects = ptr;
	return 1;
	}

int
ObjWorker::AddNode(ImpNode *node,const TCHAR *name,int type,Mesh *msh,char *owner, int mtlNum) {
//DebugPrint("Adding node %s, type %d, mesh:%p\n",name,type,msh);
	WkNodeList *ptr = new WkNodeList;
	if(!ptr)
		return 0;
	ptr->node = node;
	ptr->id = -1;
	ptr->type = type;
	ptr->name = name;
	ptr->mesh = msh;
	ptr->mnum = mtlNum;
	ptr->next = nodes;
	ptr->parent = NULL;
	ptr->posList = ptr->rotList = ptr->scList = ptr->colList = ptr->hotList = ptr->fallList = ptr->fovList = ptr->rollList = NULL;
	if(type==OBJ_TARGET)
		ptr->owner = TSTR(owner);
	workNode = nodes = ptr;
	thisNode = node;
	return 1;
	}

/*TriObject*/ void *
ObjWorker::FindObject(char *name, int &type, int &cstShad, int &rcvShad, int &mtlNum) {
	TSTR wname(name);
	WkObjList *ptr = objects;
	while(ptr) {
		if(_tcscmp(ptr->name,wname)==0) {
			type = ptr->type;
			cstShad = ptr->cstShad;
			rcvShad = ptr->rcvShad;
			mtlNum = ptr->mtln;
			return ptr->object;
			}
		ptr = (WkObjList *)ptr->next;
		}
	return NULL;
	}

int
ObjWorker::UseObject(char *name) {
	TSTR wname(name);
	WkObjList *ptr = objects;
	while(ptr) {
		if(_tcscmp(ptr->name,wname)==0) {
			ptr->used = 1;
			return 1;
			}
		ptr = (WkObjList *)ptr->next;
		}
	return 0;
	}

void ObjWorker::MakeControlsTCB(Control *tmCont,SHORT *tflags)
	{
	Control *c;
	DWORD flags=INHERIT_ALL;

	// Setup inheritance flags.
	if (tflags[POS_TRACK_INDEX] & NO_LNK_X) flags &= ~INHERIT_POS_X;
	if (tflags[POS_TRACK_INDEX] & NO_LNK_Y) flags &= ~INHERIT_POS_Y;
	if (tflags[POS_TRACK_INDEX] & NO_LNK_Z) flags &= ~INHERIT_POS_Z;

	if (tflags[ROT_TRACK_INDEX] & NO_LNK_X) flags &= ~INHERIT_ROT_X;
	if (tflags[ROT_TRACK_INDEX] & NO_LNK_Y) flags &= ~INHERIT_ROT_Y;
	if (tflags[ROT_TRACK_INDEX] & NO_LNK_Z) flags &= ~INHERIT_ROT_Z;

	if (tflags[SCL_TRACK_INDEX] & NO_LNK_X) flags &= ~INHERIT_SCL_X;
	if (tflags[SCL_TRACK_INDEX] & NO_LNK_Y) flags &= ~INHERIT_SCL_Y;
	if (tflags[SCL_TRACK_INDEX] & NO_LNK_Z) flags &= ~INHERIT_SCL_Z;

	tmCont->SetInheritanceFlags(flags,FALSE);

	c = tmCont->GetPositionController();
	if (c && c->ClassID()!=Class_ID(TCBINTERP_POSITION_CLASS_ID,0)) {
		Control *tcb = (Control*)ip->CreateInstance(
			CTRL_POSITION_CLASS_ID,
			Class_ID(TCBINTERP_POSITION_CLASS_ID,0));
		if (!tmCont->SetPositionController(tcb)) {
			tcb->DeleteThis();
			}
		}

	c = tmCont->GetRotationController();
	if (c && c->ClassID()!=Class_ID(TCBINTERP_ROTATION_CLASS_ID,0)) {
		Control *tcb = (Control*)ip->CreateInstance(
			CTRL_ROTATION_CLASS_ID,
			Class_ID(TCBINTERP_ROTATION_CLASS_ID,0));
		if (!tmCont->SetRotationController(tcb)) {
			tcb->DeleteThis();
			}
		}

	c = tmCont->GetRollController();
	if (c && c->ClassID()!=Class_ID(TCBINTERP_FLOAT_CLASS_ID,0)) {
		Control *tcb = (Control*)ip->CreateInstance(
			CTRL_FLOAT_CLASS_ID,
			Class_ID(TCBINTERP_FLOAT_CLASS_ID,0));
		if (!tmCont->SetRollController(tcb)) {
			tcb->DeleteThis();
			}
		}

	c = tmCont->GetScaleController();
	if (c && c->ClassID()!=Class_ID(TCBINTERP_SCALE_CLASS_ID,0)) {
		Control *tcb = (Control*)ip->CreateInstance(
			CTRL_SCALE_CLASS_ID,
			Class_ID(TCBINTERP_SCALE_CLASS_ID,0));
		if (!tmCont->SetScaleController(tcb)) {
			tcb->DeleteThis();
			}
		}
	}


// Note about aspect ratios for rectangular spotlights:
// MAX handles these differently than 3DS.
// Fs = FOV/2 for 3DS
// Fm = FOV/2 for MAX
// As = aspect ratio for 3DS
// Am = aspect ratio for MAX
//
// For 3DS:
//
// w = tan(Fs)
// h = tan(Fs) * As
//
// For MAX:
//
// w = tan(Fm) * sqrt(Am)
// h = tan(Fm) / sqrt(Am)
//
// so solving for stuff gives:
//
// Am = 1 / As
// Fm = atan( tan(Fs) / sqrt(Am) )
//
// Note that FOV is really refering to hotspot or falloff
// So when importing, the hotspot and falloff tracks need
// to be modified by the above formulas.
//
// If the 'aspect' parameter to this function is >0 then
// we assume the float track is either a falloff or hotspot
// track and perform the conversion.

void ObjWorker::SetControllerKeys(
		Control *cont,KeyList *keys,int type,float f,float aspect)
	{
	KeyList *ptr = keys;
	int ct=0;
	ITCBFloatKey fkey;
	ITCBPoint3Key pkey;
	ITCBRotKey rkey;
	ITCBScaleKey skey;
	ITCBKey *k;

	// Set up 'k' to point at the right derived class
	switch (type) {
		case KEY_FLOAT: k = &fkey; break;
		case KEY_POS: k = &pkey; break;
		case KEY_ROT: k = &rkey; break;
		case KEY_SCL: k = &skey; break;
		case KEY_COLOR: k = &pkey; break;
		default: return;
		}

	// Count how many keys we got.
	while (ptr) {
		ct++;
		ptr = (KeyList*)ptr->next;
		}

	// Special case where there is only one key at frame 0.
	// Just set the controller's value
	if (ct==1 && keys->key.key.time==0) {
		switch (type) {
			case KEY_FLOAT: {
				float flt;
				if (aspect>0) {
					float tn = (float)tan(DegToRad(keys->key.sc.e[0].p/2.0f));
					flt = (float)RadToDeg(atan(tn/sqrt(aspect)))*2.0f;
				} else {
					flt = keys->key.sc.e[0].p * f;
					}
				cont->SetValue(0,&flt);
				break;
				}

			case KEY_COLOR: {
				Point3 p(
					keys->key.col.c[0],
					keys->key.col.c[1],
					keys->key.col.c[2]);
				cont->SetValue(0,&p);
				break;
				}

			case KEY_POS: {
				Point3 p(
					keys->key.pos.e[0].p,
					keys->key.pos.e[1].p,
					keys->key.pos.e[2].p);
				cont->SetValue(0,&p);
				break;
				}
			case KEY_ROT: {
				Point3 axis(
					keys->key.rot.axis[0],
					keys->key.rot.axis[1],
					keys->key.rot.axis[2]);
				Quat q = QFromAngAxis(keys->key.rot.angle, axis);
				cont->SetValue(0,&q);
				break;
				}
			case KEY_SCL: {
				ScaleValue s(
					Point3(
					keys->key.pos.e[0].p,
					keys->key.pos.e[1].p,
					keys->key.pos.e[2].p));
				cont->SetValue(0,&s);
				break;
				}
			}
		return;
		}

	// Make sure we can get the interface and have some keys
	IKeyControl *ikeys = GetKeyControlInterface(cont);
	if (!ct || !ikeys) return;
	
	// Allocate the keys in the table
	ikeys->SetNumKeys(ct);

	ptr = keys;
	ct  = 0;
	while (ptr) {
		// Set the common values
		k->time    = ptr->key.key.time * GetTicksPerFrame();
		k->tens    = ptr->key.key.tens;
		k->cont    = ptr->key.key.cont;
		k->bias    = ptr->key.key.bias;
		k->easeIn  = ptr->key.key.easeTo;
		k->easeOut = ptr->key.key.easeFrom;
		
		// Set the key type specific values
		switch (type) {
			case KEY_FLOAT:
				if (aspect>0) {
					float tn = (float)tan(DegToRad(ptr->key.sc.e[0].p/2.0f));
					fkey.val = (float)RadToDeg(atan(tn/sqrt(aspect)))*2.0f;
				} else {
					fkey.val = ptr->key.sc.e[0].p * f;
					}
				break;

			case KEY_POS:	
				pkey.val[0] = ptr->key.pos.e[0].p; 
				pkey.val[1] = ptr->key.pos.e[1].p; 
				pkey.val[2] = ptr->key.pos.e[2].p; 
				break;

			case KEY_ROT:	
				rkey.val.angle   = ptr->key.rot.angle; 
				rkey.val.axis[0] = ptr->key.rot.axis[0];
				rkey.val.axis[1] = ptr->key.rot.axis[1];
				rkey.val.axis[2] = ptr->key.rot.axis[2];
				break;

			case KEY_SCL:	
				skey.val.s[0] = ptr->key.pos.e[0].p; 
				skey.val.s[1] = ptr->key.pos.e[1].p; 
				skey.val.s[2] = ptr->key.pos.e[2].p;
				break;

			case KEY_COLOR:	
				pkey.val[0] = ptr->key.col.c[0]; 
				pkey.val[1] = ptr->key.col.c[1]; 
				pkey.val[2] = ptr->key.col.c[2]; 
				break;
			}
		
		// Set the key in the table
		ikeys->SetKey(ct++,k);		
		ptr = (KeyList*)ptr->next;
		}
	
	// This will ensure that the keys are sorted by time at the
	// track is invalidated so the tangents will be recomputed.
	ikeys->SortKeys();
	}



// 3DS R4 IK joint data
typedef struct {
	int		freeJoints[6];
	float	jointMins[6];
	float	jointMaxs[6];
	float	precedence[6];
	float	damping[6];
	int		limited[6];
	int		ease[6];
	int		lastModified;
} JointData3DSR4;

void ObjWorker::ParseIKData(INode *node)
	{
	AppDataChunk *ad = 
		node->GetAppDataChunk(node->ClassID(),node->SuperClassID(),0);
    if (!ad) return;
	
	// Get the IK data chunk
	Chunk_hdr *hdr = (Chunk_hdr*)
		GetAppDataChunk(ad->data,ad->length,"IK KXPv1 62094j39dlj3i3h42");
	if (!hdr) return;

	// Get the joint data sub chunk
	Chunk_hdr *nhdr = hdr+1;
	DWORD len = hdr->size - nhdr->size - 6;
	void *ikdata = (void*)(((char*)nhdr)+nhdr->size);
	hdr = (Chunk_hdr*)GetAppDataChunk(ikdata,len,"JOINTDATA");
	if (!hdr) return;

	// The first 4 bytes is the version number
	nhdr = hdr+1;
	int *version = (int*)(((char*)nhdr)+nhdr->size);
	if (*version!=3) return; // gotta be version 3

	// Then the joint data
	JointData3DSR4 *jd = (JointData3DSR4*)(version+1);
	
	// Copy it into our data structures
	InitJointData posData, rotData;
	for (int i=0; i<3; i++) {
		posData.active[i]  = jd->freeJoints[i+3];
		posData.limit[i]   = jd->limited[i+3];
		posData.ease[i]    = jd->ease[i+3];
		posData.min[i]     = jd->jointMins[i+3];
		posData.max[i]     = jd->jointMaxs[i+3];
		posData.damping[i] = jd->damping[i+3];

		rotData.active[i]  = jd->freeJoints[i];
		rotData.limit[i]   = jd->limited[i];
		rotData.ease[i]    = jd->ease[i];
		rotData.min[i]     = jd->jointMins[i];
		rotData.max[i]     = jd->jointMaxs[i];
		rotData.damping[i] = jd->damping[i];
		}

	// Give the data to the TM controller
	node->GetTMController()->InitIKJoints(&posData,&rotData);
	}
	
int
ObjWorker::CompleteScene() {
	FinishUp();
	// If we need to expand time, ask the user!
	if(lengthSet) {
		Interval cur = i->GetAnimRange();
		Interval nrange = Interval(0,length * TIME_CONSTANT);
		if (!(cur==nrange)) {
			if (replaceScene||(!showPrompts)||MessageBox2(IDS_MATCHANIMLENGTH, IDS_3DSIMP, MB_YESNO)==IDYES) {
				i->SetAnimRange(nrange);
				}
			SetCursor(LoadCursor(NULL, IDC_WAIT));
			}
		
		/*		
		if((length * TIME_CONSTANT) > cur.End()) {
			if(MessageBox( IDS_TH_EXPANDANIMLENGTH, IDS_RB_3DSIMP, MB_YESNO) == IDYES) {
				cur.SetEnd(length * TIME_CONSTANT);
				i->SetAnimRange(cur);
				}
			}
		*/
		}
	WkNodeList *nptr = nodes;
	while(nptr) {
		if(nptr->type==OBJ_TARGET && nptr->node) {
			WkNodeList *nptr2 = nodes;
			while(nptr2) {
				if((nptr2->type==OBJ_CAMERA || nptr2->type==OBJ_SPOTLIGHT) && nptr->owner==nptr2->name && nptr2->node) {
					i->BindToTarget(nptr2->node,nptr->node);
					goto next_target;			
					}
				nptr2 = (WkNodeList *)nptr2->next;
				}
			}
		next_target:
		nptr = (WkNodeList *)nptr->next;
		}
	nptr = nodes;
	while(nptr) {
		Control *control;
		if(nptr->node) {
			ImpNode *theNode = nptr->node;
			INode *theINode = theNode->GetINode();
			switch(nptr->type) {
				case OBJ_MESH:
					// Do materials
					AssignMtl(theINode, nptr->mesh);

				case OBJ_DUMMY:
					// Unload all key info into the Jaguar node
					control = theINode->GetTMController();
					MakeControlsTCB(control,nptr->trackFlags);
					if(control) {
						SuspendAnimate();
						AnimateOn();
						if(nptr->posList) {
							Control *posControl = control->GetPositionController();
							if(posControl) {								
								SetControllerKeys(
									posControl,
									nptr->posList,
									KEY_POS);
								/*
								KeyList *k = nptr->posList;
								while(k) {
									PosKey *p = &k->key.pos;
									Point3 trans(p->e[0].p,p->e[1].p,p->e[2].p);
									posControl->SetValue(p->time * GetTicksPerFrame(), &trans);							
									k = (KeyList *)(k->next);
									}
								*/
								}
							}
						if(nptr->rotList) {
							Control *rotControl = control->GetRotationController();
							if(rotControl) {
								SetControllerKeys(
									rotControl,
									nptr->rotList,
									KEY_ROT);
								/*
								KeyList *k = nptr->rotList;
								BOOL firstkey = TRUE;
								Quat accum;
								while(k) {
//									RotKey *r = &k->key.rot;
//									Point3 axis = Point3(r->axis[0], r->axis[1], r->axis[2]);
//									float angle = r->angle;
//									Quat q = QFromAngAxis(angle, axis);
//									AngAxis aa(axis, angle);
//									if(firstkey) {
//										rotControl->SetValue(r->time * TIME_CONSTANT, &q);
//										firstkey = FALSE;
//										}
//									else
//										rotControl->SetValue(r->time * TIME_CONSTANT, &aa, 1, CTRL_RELATIVE);
//									k = (KeyList *)(k->next);
									RotKey *r = &k->key.rot;
									Point3 axis = Point3(r->axis[0], r->axis[1], r->axis[2]);
									float angle = r->angle;
									Quat q = QFromAngAxis(angle, axis);
									if(firstkey)
										accum = q;
									else
										accum *= q;
									rotControl->SetValue(r->time * TIME_CONSTANT, &accum);
									firstkey = FALSE;
									k = (KeyList *)(k->next);
									}
								*/
								}
							}
						if(nptr->scList) {
							Control *scControl = control->GetScaleController();
							if(scControl) {
								SetControllerKeys(
									scControl,
									nptr->scList,
									KEY_SCL);
								/*
								KeyList *k = nptr->scList;
								while(k) {
									PosKey *s = &k->key.pos;
									Point3 scale(s->e[0].p,s->e[1].p,s->e[2].p);
									scControl->SetValue(s->time * TIME_CONSTANT, &scale);							
									k = (KeyList *)(k->next);
									}
								*/
								}
							}
						ResumeAnimate();
						}
					break;
				case OBJ_OMNILIGHT: {
					GenLight *lt = (GenLight *)FindObjFromNode(theNode);
					if(!lt) {
						assert(0);
						break;
						}
					// Unload all key info into the Jaguar node
					SuspendAnimate();
					AnimateOn();
					control = theINode->GetTMController();
					MakeControlsTCB(control,nptr->trackFlags);
					if(control) {
						if(nptr->posList) {
							Control *posControl = control->GetPositionController();
							if(posControl) {
								SetControllerKeys(
									posControl,
									nptr->posList,
									KEY_POS);
								/*
								KeyList *k = nptr->posList;
								while(k) {
									PosKey *p = &k->key.pos;
									Point3 trans(p->e[0].p,p->e[1].p,p->e[2].p);
									posControl->SetValue(p->time * TIME_CONSTANT, &trans);							
									k = (KeyList *)(k->next);
									}
								*/
								}
							}
						}
					if(nptr->colList) {
						Control *cont = (Control*)ip->CreateInstance(
							CTRL_POINT3_CLASS_ID,
							Class_ID(TCBINTERP_POINT3_CLASS_ID,0));
						lt->SetColorControl(cont);
						SetControllerKeys(
							cont,
							nptr->colList,
							KEY_COLOR);
						/*
						KeyList *k = nptr->colList;
						while(k) {
							ColorKey *c = &k->key.col;
							lt->SetRGBColor(c->time * TIME_CONSTANT,Point3(c->c[0],c->c[1],c->c[2]));
							k = (KeyList *)(k->next);
							}
						*/
						}
					lt->Enable(TRUE);
					ResumeAnimate();
					}
					break;
				case OBJ_SPOTLIGHT: {
					GenLight *lt = (GenLight *)FindObjFromNode(theNode);
					float aspect = lt->GetAspect(0);
					if(!lt) {
						assert(0);
						break;
						}
					// Unload all key info into the Jaguar node
					SuspendAnimate();
					AnimateOn();
					control = theINode->GetTMController();
					MakeControlsTCB(control,nptr->trackFlags);
					if(control) {
						if(nptr->posList) {
							Control *posControl = control->GetPositionController();
							if(posControl) {
								SetControllerKeys(
									posControl,
									nptr->posList,
									KEY_POS);
								/*
								KeyList *k = nptr->posList;
								while(k) {
									PosKey *p = &k->key.pos;
									Point3 trans(p->e[0].p,p->e[1].p,p->e[2].p);
									posControl->SetValue(p->time * TIME_CONSTANT, &trans);							
									k = (KeyList *)(k->next);
									}
								*/
								}
							}
						}
					if(nptr->colList) {
						Control *cont = (Control*)ip->CreateInstance(
							CTRL_POINT3_CLASS_ID,
							Class_ID(TCBINTERP_POINT3_CLASS_ID,0));
						lt->SetColorControl(cont);
						SetControllerKeys(
							cont,
							nptr->colList,
							KEY_COLOR);
						/*
						KeyList *k = nptr->colList;
						while(k) {
							ColorKey *c = &k->key.col;
							lt->SetRGBColor(c->time * TIME_CONSTANT,Point3(c->c[0],c->c[1],c->c[2]));
							k = (KeyList *)(k->next);
							}
						*/
						}
					if(nptr->hotList) {
						Control *cont = (Control*)ip->CreateInstance(
							CTRL_FLOAT_CLASS_ID,
							Class_ID(TCBINTERP_FLOAT_CLASS_ID,0));
						lt->SetHotSpotControl(cont);
						SetControllerKeys(
							cont,
							nptr->hotList,
							KEY_FLOAT,1.0f,aspect);
						/*
						KeyList *k = nptr->hotList;
						while(k) {
							ScalarKey *h = &k->key.sc;
							lt->SetHotspot(h->time * TIME_CONSTANT, h->e[0].p);
							k = (KeyList *)(k->next);
							}
						*/
						}
					if(nptr->fallList) {
						Control *cont = (Control*)ip->CreateInstance(
							CTRL_FLOAT_CLASS_ID,
							Class_ID(TCBINTERP_FLOAT_CLASS_ID,0));
						lt->SetFalloffControl(cont);
						SetControllerKeys(
							cont,
							nptr->fallList,
							KEY_FLOAT,1.0f,aspect);
						/*
						KeyList *k = nptr->fallList;
						while(k) {
							ScalarKey *f = &k->key.sc;
							lt->SetFallsize(f->time * TIME_CONSTANT, f->e[0].p);
							k = (KeyList *)(k->next);
							}
						*/
						}
					control = theINode->GetTMController();
					if(control) {
						if(nptr->rollList) {
							Control *rollControl = control->GetRollController();
							if(rollControl) {
								SetControllerKeys(
									rollControl,
									nptr->rollList,
									KEY_FLOAT,-DEG_TO_RAD);
								/*
								KeyList *k = nptr->rollList;
								while(k) {
									ScalarKey *r = &k->key.sc;
									float roll = r->e[0].p * DEG_TO_RAD;
									rollControl->SetValue(r->time * TIME_CONSTANT, &roll);							
									k = (KeyList *)(k->next);
									}
								*/
								}
							}
						}
					lt->Enable(TRUE);
					ResumeAnimate();
					}
					break;
				case OBJ_CAMERA: {
					GenCamera *cam = (GenCamera *)FindObjFromNode(theNode);
					if(!cam) {
						assert(0);
						break;
						}
					// Unload all key info into the Jaguar node
					SuspendAnimate();
					AnimateOn();
					control = theINode->GetTMController();
					MakeControlsTCB(control,nptr->trackFlags);
					if(control) {
						if(nptr->posList) {
							Control *posControl = control->GetPositionController();
							if(posControl) {
								SetControllerKeys(
									posControl,
									nptr->posList,
									KEY_POS);
								/*
								KeyList *k = nptr->posList;
								while(k) {
									PosKey *p = &k->key.pos;
									Point3 trans(p->e[0].p,p->e[1].p,p->e[2].p);
									posControl->SetValue(p->time * TIME_CONSTANT, &trans);							
									k = (KeyList *)(k->next);
									}
								*/
								}
							}
						}
					if(nptr->fovList) {
						Control *cont = (Control*)ip->CreateInstance(
							CTRL_FLOAT_CLASS_ID,
							Class_ID(TCBINTERP_FLOAT_CLASS_ID,0));
						cam->SetFOVControl(cont);
						SetControllerKeys(
							cont,
							nptr->fovList,
							KEY_FLOAT,DEG_TO_RAD);
						/*
						KeyList *k = nptr->fovList;
						while(k) {
							ScalarKey *f = &k->key.sc;
							cam->SetFOV(f->time * TIME_CONSTANT, f->e[0].p * DEG_TO_RAD);
							k = (KeyList *)(k->next);
							}
						*/
						}
					control = theINode->GetTMController();
					if(control) {
						if(nptr->rollList) {
							Control *rollControl = control->GetRollController();
							if(rollControl) {
								SetControllerKeys(
									rollControl,
									nptr->rollList,
									KEY_FLOAT,-DEG_TO_RAD);
								/*
								KeyList *k = nptr->rollList;
								while(k) {
									ScalarKey *r = &k->key.sc;
									float roll = r->e[0].p * DEG_TO_RAD;
									rollControl->SetValue(r->time * TIME_CONSTANT, &roll);							
									k = (KeyList *)(k->next);
									}
								*/
								}
							}
						}
					cam->Enable(TRUE);
					ResumeAnimate();
					}
					break;
				case OBJ_OTHER:
					theINode->SetMtl(GetMaxMtl(nptr->mnum));
					
				case OBJ_TARGET:

					// Unload all key info into the Jaguar node
					control = theINode->GetTMController();
					MakeControlsTCB(control,nptr->trackFlags);
					if(control) {
						SuspendAnimate();
						AnimateOn();
						if(nptr->posList) {
							Control *posControl = control->GetPositionController();
							if(posControl) {
								SetControllerKeys(
									posControl,
									nptr->posList,
									KEY_POS);
								/*
								KeyList *k = nptr->posList;
								while(k) {
									PosKey *p = &k->key.pos;
									Point3 trans(p->e[0].p,p->e[1].p,p->e[2].p);
									posControl->SetValue(k->key.key.time * TIME_CONSTANT, &trans);							
									k = (KeyList *)(k->next);
									}
								*/
								}
							}
						ResumeAnimate();
						}
					break;
				default:
					assert(0);
					break;
				}

			ParseIKData(theINode);
			}
		nptr = (WkNodeList *)nptr->next;
		}
	WkObjList *ptr = objects;
	while(ptr) {
		if(dStream) {
			fprintf(dStream,"Found object %s, used:%d\n",CStr(ptr->name),ptr->used);
			fflush(dStream);
			}
		if(!ptr->used) {
			ImpNode *node1, *node2;
			node1 = MakeANode(ptr->name, FALSE, "");

			// DS: 4/30/97
			// This fixes import of files created in 3DS-DOS with the
			// Save Selected command, which don't have keyframe data.
			if (!gotM3DMAGIC) 
				node1->SetTransform(0,ptr->tm);

			if (ptr->type==OBJ_MESH) {
				INode *inode = node1->GetINode();
				Mesh *mesh = &(((TriObject *)inode->GetObjectRef())->GetMesh());
				AssignMtl(inode, mesh);
				}

			if (ptr->type==OBJ_SPOTLIGHT || ptr->type==OBJ_CAMERA) {
				Matrix3 tm(1);
				
				if (ptr->type==OBJ_SPOTLIGHT) {
					GenLight *lt = 
						(GenLight *)FindObjFromNode(node1);
					lt->Enable(TRUE);
				} else {
					GenCamera *cam = 
						(GenCamera *)FindObjFromNode(node1);
					cam->Enable(TRUE);
					}

				// Create a target
				TSTR name = ptr->name + TSTR(_T(".Target"));
				node2 = MakeANode(name, TRUE, "");								
				tm.SetTrans(ptr->targPos);
				node2->SetTransform(0,tm);
				i->BindToTarget(node1,node2);

				// Set the position of the light or camera.
				tm.SetTrans(ptr->srcPos);
				node1->SetTransform(0,tm);
				}
			if (ptr->type==OBJ_OMNILIGHT) {
				GenLight *lt = 
					(GenLight *)FindObjFromNode(node1);
				lt->Enable(TRUE);
				}
			}

		ptr = (WkObjList *)ptr->next;
		}
	return 1;
	}

ImpNode *
ObjWorker::FindNode(char *name) {
	TSTR wname(name);
	WkNodeList *ptr = nodes;
	while(ptr) {
		if(_tcscmp(ptr->name,wname)==0)
			return ptr->node;
		ptr = (WkNodeList *)ptr->next;
		}
	return NULL;
	}

TCHAR *
ObjWorker::NodeName(ImpNode *node) {
	WkNodeList *ptr = nodes;
	while(ptr) {
		if(node == ptr->node)
			return ptr->name;
		ptr = (WkNodeList *)ptr->next;
		}
	return NULL;
	}

int
ObjWorker::SetNodesParent(ImpNode *node,ImpNode *parent) {
	WkNodeList *ptr = nodes;
	while(ptr) {
		if(node == ptr->node) {
			ptr->parent = parent;
			return 1;
			}
		ptr = (WkNodeList *)ptr->next;
		}
	return 0;
	}

ImpNode *
ObjWorker::FindNodeFromId(short id) {
	WkNodeList *ptr = nodes;
	while(ptr) {
		if(ptr->id == id)
			return ptr->node;
		ptr = (WkNodeList *)ptr->next;
		}
	return NULL;
	}

WkNodeList *
ObjWorker::FindEntry(char *name) {
	TSTR wname(name);
	WkNodeList *ptr = nodes;
	while(ptr) {
		if(_tcscmp(ptr->name,wname)==0)
			return ptr;
		ptr = (WkNodeList *)ptr->next;
		}
	return NULL;
	}

WkNodeList *
ObjWorker::FindEntryFromId(short id) {
	WkNodeList *ptr = nodes;
	while(ptr) {
		if(ptr->id == id)
			return ptr;
		ptr = (WkNodeList *)ptr->next;
		}
	return NULL;
	}

WkNodeList *
ObjWorker::FindNodeListEntry(ImpNode *node) {
	WkNodeList *ptr = nodes;
	while(ptr) {
		if(ptr->node == node)
			return ptr;
		ptr = (WkNodeList *)ptr->next;
		}
	return NULL;
	}

WkObjList *	
ObjWorker::FindObjListEntry(TSTR &name) {
	WkObjList *ptr = objects;
	while(ptr) {
		if(ptr->name == name)
			return ptr;
		ptr = (WkObjList *)ptr->next;
		}
	return NULL;
	}

int
ObjWorker::FindTypeFromNode(ImpNode *node, Mesh **mesh) {
	WkNodeList *ptr = nodes;
	while(ptr) {
		if(ptr->node == node) {
			*mesh =  ptr->mesh;
			return ptr->type;
			}
		ptr = (WkNodeList *)ptr->next;
		}
	return -1;
	}

void *
ObjWorker::FindObjFromNode(ImpNode *node) {
	WkNodeList *ptr = nodes;
	while(ptr) {
		if(ptr->node == node) {
			WkObjList *optr = objects;
			while(optr) {
				if(optr->name == ptr->name)
					return optr->object;
				optr = (WkObjList *)optr->next;
				}
			assert(0);
			return NULL;
			}
		ptr = (WkNodeList *)ptr->next;
		}
	return NULL;
	}

int
ObjWorker::SetNodeId(ImpNode *node,short id) {
	WkNodeList *ptr = nodes;
	while(ptr) {
		if(ptr->node == node) {
			ptr->id = id;
			if(dStream) {
				fprintf(dStream,"Setting node ID to %d\n",id);
				fflush(dStream);
				}
			return 1;
			}
		ptr = (WkNodeList *)ptr->next;
		}
	this->id = id;
	return 0;	
	}

int
ObjWorker::AddKey(KeyList **list,Key *key) {
	if(mode != WORKER_KF) {
//		MessageBox(NULL,_T("Key add: Not in KF Worker mode"),_T("3DSIMP"),MB_OK);
		return 1;	// Keep it from bailing out -- Will need to return 0 eventually
		}
	KeyList *ptr = new KeyList;
	if(!ptr)
		return 0;
	KeyList *work = *list,*last = NULL;
	while(work) {
		last = work;
		work = (KeyList *)work->next;
		}
	if(last)
		last->next = ptr;
	else
		*list = ptr;
	ptr->key.key = *key;
	ptr->next = NULL;
	return 1;
	}

void
ObjWorker::FreeKeyList(KeyList **list) {
	KeyList *ptr = *list;
	while(ptr) {
		KeyList *next = (KeyList *)ptr->next;
		free(ptr);
		ptr = next;
		}
	*list = NULL;
	}

int
ObjWorker::SetTransform(ImpNode *node,Matrix3& m) {
	WkNodeList *ptr = nodes;
	while(ptr) {
		if(ptr->node == node) {
			ptr->tm = m;
			return 1;
			}
		ptr = (WkNodeList *)ptr->next;
		}
	return 0;
	}

Matrix3
ObjWorker::GetTransform(ImpNode *node) {
	WkNodeList *ptr = nodes;
	while(ptr) {
		if(ptr->node == node)
			return ptr->tm;
		ptr = (WkNodeList *)ptr->next;
		}
	Matrix3 dummy;
	dummy.IdentityMatrix();
	return dummy;
	}

// Pointer to the ObjWorker object

ObjWorker *theWorker;

// Jaguar interface code

int controlsInit = FALSE;

BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved) {
	hInstance = hinstDLL;

	if ( !controlsInit ) {
		controlsInit = TRUE;
		
		// jaguar controls
		InitCustomControls(hInstance);

		// initialize Chicago controls
		InitCommonControls();
		}
	switch(fdwReason) {
		case DLL_PROCESS_ATTACH:
			//MessageBox(NULL,_T("3DSIMP.DLL: DllMain"),_T("3DSIMP"),MB_OK);
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
			break;
		}
	return(TRUE);
	}


//------------------------------------------------------

class StudioClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new StudioImport; }
	const TCHAR *	ClassName() { return GetString(IDS_TH_3DSTUDIO); }
	SClass_ID		SuperClassID() { return SCENE_IMPORT_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(2,0); }
	const TCHAR* 	Category() { return GetString(IDS_TH_SCENEIMPORT);  }
	};

static StudioClassDesc StudioDesc;

class StudioShapeClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new StudioShapeImport; }
	const TCHAR *	ClassName() { return GetString(IDS_TH_3DSTUDIOSHAPE); }
	SClass_ID		SuperClassID() { return SCENE_IMPORT_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(2,1); }
	const TCHAR* 	Category() { return GetString(IDS_TH_SCENEIMPORT); }
	};

static StudioShapeClassDesc StudioShapeDesc;

// Statics

int	StudioShapeImport::importType = SINGLE_SHAPE;

//------------------------------------------------------
// This is the interface to Jaguar:
//------------------------------------------------------

__declspec( dllexport ) const TCHAR *
LibDescription() { return GetString(IDS_TH_3DSIMPORTDLL); }

__declspec( dllexport ) int
LibNumberClasses() { return 2; }

__declspec( dllexport ) ClassDesc *
LibClassDesc(int i) {
	switch(i) {
		case 0: return &StudioDesc; break;
		case 1: return &StudioShapeDesc; break;
		default: return 0; break;
		}

	}

// Return version so can detect obsolete DLLs
__declspec( dllexport ) ULONG 
LibVersion() { return VERSION_3DSMAX; }

// Let the plug-in register itself for deferred loading
__declspec( dllexport ) ULONG CanAutoDefer()
{
	return 1;
}

//
// .3DS import module functions follow:
//

StudioImport::StudioImport() {
	}

StudioImport::~StudioImport() {
	}

int
StudioImport::ExtCount() {
	return 2;
	}

const TCHAR *
StudioImport::Ext(int n) {		// Extensions supported for import/export modules
	switch(n) {
		case 0:
			return _T("3DS");
		case 1:
			return _T("PRJ");
		}
	return _T("");
	}

const TCHAR *
StudioImport::LongDesc() {			// Long ASCII description (i.e. "Targa 2.0 Image File")
	return GetString(IDS_TH_3DSSCENEFILE);
	}
	
const TCHAR *
StudioImport::ShortDesc() {			// Short ASCII description (i.e. "Targa")
	return GetString(IDS_TH_3DSMESH);
	}

const TCHAR *
StudioImport::AuthorName() {			// ASCII Author name
	return GetString(IDS_TH_TOM_HUDSON);
	}

const TCHAR *
StudioImport::CopyrightMessage() {	// ASCII Copyright message
	return GetString(IDS_TH_COPYRIGHT_YOST_GROUP);
	}

const TCHAR *
StudioImport::OtherMessage1() {		// Other message #1
	return _T("");
	}

const TCHAR *
StudioImport::OtherMessage2() {		// Other message #2
	return _T("");
	}

unsigned int
StudioImport::Version() {				// Version number * 100 (i.e. v3.01 = 301)
	return 100;
	}

void
StudioImport::ShowAbout(HWND hWnd) {			// Optional
 	}
//
// .SHP import module functions follow:
//

StudioShapeImport::StudioShapeImport() {
	shapeNumber = 1;		// For name index
 	}

StudioShapeImport::~StudioShapeImport() {
	}

int
StudioShapeImport::ExtCount() {
	return 1;
	}

const TCHAR *
StudioShapeImport::Ext(int n) {		// Extensions supported for import/export modules
	switch(n) {
		case 0:
			return _T("SHP");
		}
	return _T("");
	}

const TCHAR *
StudioShapeImport::LongDesc() {			// Long ASCII description (i.e. "Targa 2.0 Image File")
	return GetString(IDS_TH_3DSSHAPEFILE);
	}
	
const TCHAR *
StudioShapeImport::ShortDesc() {			// Short ASCII description (i.e. "Targa")
	return GetString(IDS_TH_3DSTUDIOSHAPE);
	}

const TCHAR *
StudioShapeImport::AuthorName() {			// ASCII Author name
	return GetString(IDS_TH_TOM_HUDSON);
	}

const TCHAR *
StudioShapeImport::CopyrightMessage() {	// ASCII Copyright message
	return GetString(IDS_TH_COPYRIGHT_YOST_GROUP);
	}

const TCHAR *
StudioShapeImport::OtherMessage1() {		// Other message #1
	return _T("");
	}

const TCHAR *
StudioShapeImport::OtherMessage2() {		// Other message #2
	return _T("");
	}

unsigned int
StudioShapeImport::Version() {				// Version number * 100 (i.e. v3.01 = 301)
	return 100;
	}

void
StudioShapeImport::ShowAbout(HWND hWnd) {			// Optional
 	}

/* Bypass an unknown chunk type */

int
skip_chunk(FILE *stream)
{
Chunk_hdr chunk;

RDERR(&chunk,sizeof(Chunk_hdr));

chunk.size-=6L;
return(1-fseek(stream,chunk.size,SEEK_CUR));
}

/* Skip the next n bytes in the file */

int
SkipRead(FILE *stream,long bytes)
{
return fseek(stream,bytes,SEEK_CUR);
}

/* Get next chunk ID and return it, repositioning file to old pos */

int
get_next_chunk(FILE *stream,Chunk_hdr *hdr)
{
long curpos;

curpos=ftell(stream);
RDERR(hdr,sizeof(Chunk_hdr));
fseek(stream,curpos,SEEK_SET);

#ifdef DUMPING
{
TCHAR buf[256];
_stprintf(buf,_T("Next chunk: %X, size:%d"),hdr->tag,hdr->size);
MessageBox(NULL,buf,_T("3DSIMP"),MB_OK);
}
#endif // DUMPING

return(1);
}

/* Get a null-terminated string from the file */

int
read_string(char *string,FILE *stream,int maxsize)
{
while(maxsize--)
 {
 RDERR(string,1);
 if(*(string++)==0)
  return(1);
 }
return(0);	/* Too long */
}

// The main 3DS reader!
int
get_mchunk(void *data) {
	int ix,skipping;
	float fdum;
	Verts v;
	UVVert tv;
	Faces f;
	Dirlight *d;
	Camera3DS *c;
	Bkgrad *gd;
	Chunk_hdr chunk;
	Chunk_hdr next;
	int obnbr;
	unsigned short wk_count;
	struct fc_wrt
	{
	unsigned short a;
	unsigned short b;
	unsigned short c;
	unsigned short flags;
	} Fc_wrt;

	RDERR(&chunk,6);

	/* Update chunk size to account for header */

	chunk.size-=6L;

	/* Find chunk type and go process it */

	lastchunk=chunk.tag;

#ifdef PRINTCHUNKID
DebugPrint("Chunk:%X\n",chunk.tag);
#endif

#ifdef DUMPING
{
TCHAR buf[256];
_stprintf(buf,_T("Chunk: %X, size:%d"),chunk.tag,chunk.size);
MessageBox(NULL,buf,_T("3DSIMP"),MB_OK);
}
#endif // DUMPING

	/* printf("get_mchunk: chunk.tag:%X\n",lastchunk); */

	switch(chunk.tag)
		{
		case CMAGIC:
			while(chunk.size)
				{
				if(get_next_chunk(stream,&next)==0)
					return(0);
				switch(next.tag)
					{
					case SMAGIC:
//						if(merging) goto skipit;
//						inst_shape(&Infoshp);
//						shapeok=1;
						msc_wk=1.0f;
						if(get_mchunk(NULL)==0)
							return(0);
//						uninst_shape(&Infoshp);
						break;
//					case LMAGIC:
//						if(merging) goto skipit;
//						path_only=0;
//						msc_wk=1.0;
//						if(get_lchunk(stream,NULL)==0)
//							return(0);
//						calc_pathpts();
//						shp_poffsets();
//						break;
//#ifdef LOADING_MLI
//					case MLIBMAGIC:
//						if(merging) goto skipit;
//						new_mlib();
//						just_name=0;
//						if (get_mtlchunk(stream,NULL)==0)
//							return(0);
//						break;
//#endif
					case MMAGIC:
//						hiddens=0;
//						if(!merging)
							msc_wk=1.0f;
//						mesh_version=0;
						if(get_mchunk(NULL)==0)
							return(0);
						break;
					case KFDATA:
//						if(merging==2)
//							goto skipit;
//						if(merging &&(mg_keys==0))
//							goto skipit;
						/*merging = 0;*/
						if(get_mchunk(NULL)==0)
							return(0);
						break;

					default:
//						skipit:
						if(skip_chunk(stream)==0)
							return(0);
						break;
					}
				chunk.size-=next.size;
				}
			break;
		case KFDATA:
			while(chunk.size) {
				if(!get_next_chunk(stream,&next))
					return(0);
				switch(next.tag)   	{
					case OBJECT_NODE_TAG:
					case CAMERA_NODE_TAG: 
					case TARGET_NODE_TAG: 	
					case L_TARGET_NODE_TAG: 	
					case LIGHT_NODE_TAG:
					case SPOTLIGHT_NODE_TAG:
						nodeLoadNumber++;
						if(dStream) {
							fprintf(dStream,"nodeLoadNumber:%d\n",nodeLoadNumber);
							fflush(dStream);
							}

					case KFHDR:
					case KFSEG:
//						nodetag = next.tag;
						if(!get_mchunk(NULL))	return(0);
						break;
					case AMBIENT_NODE_TAG:
					default:
						if(!skip_chunk(stream))	return(0);
						break;
					}
				chunk.size -= next.size;
				}
			break;
		case KFHDR: {	 /* version, mesh name anim length ... */
			char dumstr[10];
			LONG length;
			RDSHORT(&readVers);
			if (!read_string(dumstr,stream,9)) return(0);
			RDLONG(&length); 
			theWorker->SetAnimLength((TimeValue)length);
			chunk.size = 0;
			}
			break;
		case KFSEG: {
			long segStart,segEnd;
			RDLONG(&segStart);
			RDLONG(&segEnd);
			theWorker->SetSegment(Interval((TimeValue)segStart,(TimeValue)segEnd));
			chunk.size = 0;
			}
			break;
		case OBJECT_NODE_TAG:
			theWorker->FinishUp();	// Finish anything that we started earlier 
			skipNode = 0;
			cur_node_id = -32000;	
			while(chunk.size) {
				if(!get_next_chunk(stream,&next)) return(0);
				switch (next.tag) {
					case NODE_ID:
					case NODE_HDR:
					case APP_DATA:
					case BOUNDBOX: 
					case PIVOT:
					case INSTANCE_NAME:
					case POS_TRACK_TAG: 
					case ROT_TRACK_TAG: 
					case SCL_TRACK_TAG:
						if(!get_mchunk(NULL)) return(0);
						break;
//					case MORPH_SMOOTH:
//						if(!get_mchunk(onode)) return(0);
//						break;
#ifdef LATER
					case MORPH_TRACK_TAG:
						if(!get_mchunk(&onode->mtrack)) return(0);
						break;
					case HIDE_TRACK_TAG:
						if(!get_mchunk(&onode->htrack)) return(0);
						break;
#endif
					default:
						if(!skip_chunk(stream))	return(0);
						break;
					}
				chunk.size -= next.size;
				}
			break;
		case CAMERA_NODE_TAG: 
			theWorker->FinishUp();	// Finish anything that we started earlier 
			skipNode = 0;
			cur_node_id = -32000;	
			while(chunk.size) {
				if(!get_next_chunk(stream,&next)) return(0);
				if (skipNode) {
					if(!skip_chunk(stream)) return(0);
					}
				else switch (next.tag) {
					case NODE_HDR:	case NODE_ID:
					case APP_DATA:
					case POS_TRACK_TAG: 
					case ROLL_TRACK_TAG: 
					case FOV_TRACK_TAG: 
						if(!get_mchunk(NULL)) return(0);
						break;
					default:
						if(!skip_chunk(stream))	return(0);
						break;
					}
				chunk.size -= next.size;
				}
			break;
		case L_TARGET_NODE_TAG: 
		case TARGET_NODE_TAG: 
			theWorker->FinishUp();	// Finish anything that we started earlier 
			skipNode = 0;
			cur_node_id = -32000;	
			while(chunk.size) {
				if(!get_next_chunk(stream,&next)) return(0);
				if (skipNode) { 
					if(!skip_chunk(stream)) return(0);
					}
				else switch (next.tag) {
					case NODE_ID: case NODE_HDR:
						if(!get_mchunk(".target")) return(0);
						break;
					case APP_DATA:
					case POS_TRACK_TAG: 
						if(!get_mchunk(NULL)) return(0);
						break;
					default:
						if(!skip_chunk(stream))	return(0);
						break;
					}
				chunk.size -= next.size;
				}
			break;
		case LIGHT_NODE_TAG: 
			theWorker->FinishUp();	// Finish anything that we started earlier 
			skipNode = 0;
			cur_node_id = -32000;	
			while(chunk.size) {
				if(!get_next_chunk(stream,&next)) return(0);
				if (skipNode) {
					if(!skip_chunk(stream)) return(0);
					}
				else switch (next.tag) {
					case NODE_ID: case NODE_HDR:
					case APP_DATA:
					case POS_TRACK_TAG: 
					case COL_TRACK_TAG: 
						if(!get_mchunk(NULL)) return(0);
						break;
					default:
						if(!skip_chunk(stream))	return(0);
						break;
					}
				chunk.size -= next.size;
				}
			break;
		case SPOTLIGHT_NODE_TAG: 
			theWorker->FinishUp();	// Finish anything that we started earlier 
			skipNode = 0;
			cur_node_id = -32000;	
			while(chunk.size) {
				if(!get_next_chunk(stream,&next)) return(0);
				if (skipNode) {if(!skip_chunk(stream)) return(0);}
				else switch (next.tag) {
					case NODE_ID: case NODE_HDR:
					case APP_DATA:
					case POS_TRACK_TAG: 
					case COL_TRACK_TAG: 
					case HOT_TRACK_TAG: 
					case FALL_TRACK_TAG: 
					case ROLL_TRACK_TAG: 
						if(!get_mchunk(NULL)) return(0);
						break;
					default:
						if(!skip_chunk(stream))	return(0);
						break;
					}
				chunk.size -= next.size;
				}
			break;
		case NODE_ID:
//			MessageBox(NULL,_T("Reading node id"),_T("3dsimp"),MB_OK); 
			RDSHORT(&cur_node_id);	
			if(dStream) {
				fprintf(dStream,"Got Node ID:%d\n",cur_node_id);
				fflush(dStream);
				}
			break;
		case NODE_HDR: {
//			MessageBox(NULL,_T("Reading node hdr"),_T("3dsimp"),MB_OK); 
			USHORT npar;
			ImpNode *pnode;
			// If we have a suffix, this is a target object
			char *suffix = (char *)data;
						
			if (!read_string(obname,stream,20)) return(0);
			chunk.size -= (long)(strlen(obname)+1);
			obname[10] = 0;
			char prefix[32];
			if(suffix) {
				strcpy(prefix,obname);				
				strcat(obname,suffix);
				}
			else
				prefix[0] = 0;

//DebugPrint("Got node %s\n",obname);

			TSTR Wname(obname);

			if (strcmp(obname,"$_$_$_$")==0) {
//				MessageBox(NULL,_T("Skipping $_$_$_$ object"),_T("3DSIMP"),MB_OK);
				goto skip_node;
				}

			theWorker->SetDummy(0);

			// Check for dummies -- If it is a dummy, we create a node now because
			// none was created for it in the mesh section
			if (strcmp(obname,"DUMMY")==0||strcmp(obname,"$$$DUMMY")==0) {
				if(!(obnode = theWorker->MakeDummy(_T(""))))
					return 0;
				theWorker->SetDummy(1);
				theWorker->SetNodeName(_T(""));
				}
			else {
				TSTR wname(obname);
				if(!(obnode = theWorker->MakeANode(wname,suffix ? TRUE : FALSE, prefix)))
					return 0;
				theWorker->SetNodeName(wname);
				}

			theWorker->StartKF(obnode);	// Start creating a KF node
							
			if (cur_node_id!=-32000)
				theWorker->SetNodeId(obnode,cur_node_id);
			else 
				theWorker->SetNodeId(obnode,nodeLoadNumber);

			if (!theWorker->IsDummy()) { 
#ifdef LATER
				if (merging) {
					if (!merge_this(objname)) goto skip_node;
					MaybeRemapName(objname);
					}
#endif
				}

			RDSHORT(&nodeflags);	chunk.size -= 2;

			if (readVers<4) nodeflags &= 7;
			/* read second flag word */
			RDSHORT(&nodeflags2);	chunk.size -= 2;
			/* get parent node if any */
			RDSHORT(&npar);	chunk.size -= 2;
			
			pnode = NULL;
			if(npar != NO_PARENT) {
				if(dStream) {
					fprintf(dStream,"Parent's node id:%d\n",npar);
					fflush(dStream);
					}
				WkNodeList *ptr = theWorker->FindEntryFromId(npar);
				if(!ptr) {
					if(showPrompts)
						MessageBox(Wname.data(),IDS_DB_NOT_LINKED);
					}
				else {
					pnode = ptr->node;
					theWorker->SetNodesParent(obnode,pnode);
					}
				}

			theWorker->SetParentNode(pnode);
				
//			InsertNodeById(node, pnode);
			break;
			}

	      skip_node:
			skipped_nodes = 1;
//		  	FreeNode(node);
			if (SkipRead(stream, chunk.size)) return(0);
			chunk.size = 0;
			skipNode = 1;
			break;

		case PIVOT:	{
				float pivot[3];
				RD3FLOAT(pivot);
				if(msc_wk!=1.0)
					{
					pivot[0]*=msc_wk;
					pivot[1]*=msc_wk;
					pivot[2]*=msc_wk;
					}
				theWorker->SetPivot(Point3(pivot[0],pivot[1],pivot[2]));
				chunk.size = 0;
				}
			break;
		case BOUNDBOX:	{
				Point3 min,max;
				RD3FLOAT(&min.x);
				RD3FLOAT(&max.x);
				theWorker->SetDummyBounds(min,max);
				if(dStream) {
					fprintf(dStream,"Got dummy bounds\n");
					fflush(dStream);
					}
				chunk.size = 0;
				}
			break;
		case INSTANCE_NAME:	{
				char iname[20];
				if (!read_string(iname,stream,20)) return(0);
				iname[10] = 0;
				theWorker->SetInstanceName(obnode, TSTR(iname));
				chunk.size = 0;
				}
			break;
		case POS_TRACK_TAG:
		case ROT_TRACK_TAG:
		case SCL_TRACK_TAG:
		case FOV_TRACK_TAG:
		case ROLL_TRACK_TAG:
		case COL_TRACK_TAG:
		case HOT_TRACK_TAG:
		case FALL_TRACK_TAG:
#ifdef LATER
		case MORPH_TRACK_TAG:
		case HIDE_TRACK_TAG:
#endif
			LONG nkeys;
			SHORT rflags;
			Key key;
			short trflags;
			long trtmin,trtmax;
			RDSHORT(&trflags);
			RDLONG(&trtmin);
			RDLONG(&trtmax);
			RDLONG(&nkeys);
//			if (!InitKeyTab(tr)) return(0);
			for (ix=0; ix<nkeys; ix++) {  
				memset(&key,0,sizeof(Key));
				RDLONG(&key.time); 
				RDSHORT(&rflags);
 				if (rflags&W_TENS) RDFLOAT(&key.tens);
 				if (rflags&W_CONT) RDFLOAT(&key.cont);
 				if (rflags&W_BIAS) RDFLOAT(&key.bias);
 				if (rflags&W_EASETO) RDFLOAT(&key.easeTo);
 				if (rflags&W_EASEFROM)RDFLOAT(&key.easeFrom);
				switch (chunk.tag) {
					case POS_TRACK_TAG:	
						{
						PosKey pkey;
						memset(&pkey,0,sizeof(PosKey));
						memcpy(&pkey,&key,sizeof(PosKey));
						RDFLOAT(&pkey.e[0].p);
						RDFLOAT(&pkey.e[1].p);
						RDFLOAT(&pkey.e[2].p);

						/* Tom's scale fix 2/12/92 */
						if(msc_wk!=1.0)
							{
							pkey.e[0].p *= msc_wk;
							pkey.e[1].p *= msc_wk;
							pkey.e[2].p *= msc_wk;
							}
						//theWorker->trackFlags[POS_TRACK_INDEX] = trflags;
						theWorker->workNode->trackFlags[POS_TRACK_INDEX] = trflags;

						if(!theWorker->AddPositionKey(&pkey))
							return 0;
						}
						break;
					case SCL_TRACK_TAG:
						{
						PosKey skey;
						memset(&skey,0,sizeof(PosKey));
						memcpy(&skey,&key,sizeof(PosKey));
						RDFLOAT(&skey.e[0].p);
						RDFLOAT(&skey.e[1].p);
						RDFLOAT(&skey.e[2].p);
						//theWorker->trackFlags[SCL_TRACK_INDEX] = trflags;
						theWorker->workNode->trackFlags[SCL_TRACK_INDEX] = trflags;						

						if(!theWorker->AddScaleKey(&skey))
							return 0;
						}
						break;
					case COL_TRACK_TAG:
						{
						ColorKey ckey;
						memset(&ckey,0,sizeof(ColorKey));
						memcpy(&ckey,&key,sizeof(ColorKey));
						RDFLOAT(&ckey.c[0]);
						RDFLOAT(&ckey.c[1]);
						RDFLOAT(&ckey.c[2]);
						//theWorker->trackFlags[COL_TRACK_INDEX] = trflags;
						theWorker->workNode->trackFlags[COL_TRACK_INDEX] = trflags;						

						if(!theWorker->AddColorKey(&ckey))
							return 0;
						}
						break;
					case ROT_TRACK_TAG: {
						RotKey rkey;
						memset(&rkey,0,sizeof(RotKey));
						memcpy(&rkey,&key,sizeof(RotKey));
						RDFLOAT(&rkey.angle);
						RD3FLOAT(rkey.axis);
						//theWorker->trackFlags[ROT_TRACK_INDEX] = trflags;
						theWorker->workNode->trackFlags[ROT_TRACK_INDEX] = trflags;
						
						if(!theWorker->AddRotationKey(&rkey))
							return 0;
						}
						break;
					case HOT_TRACK_TAG:
						{
						ScalarKey sckey;
						memset(&sckey,0,sizeof(ScalarKey));
						memcpy(&sckey,&key,sizeof(ScalarKey));
						RDFLOAT(&sckey.e[0].p);
#ifdef DBGKFB
						if(dbgio) 
						   printf("  scalar = (%.3f)\n", sckey.e[0].p);
#endif
						//theWorker->trackFlags[HOT_TRACK_INDEX] = trflags;
						theWorker->workNode->trackFlags[HOT_TRACK_INDEX] = trflags;
						
						if(!theWorker->AddHotKey(&sckey))
							return 0;
						}
						break;
					case FALL_TRACK_TAG:
						{
						ScalarKey sckey;
						memset(&sckey,0,sizeof(ScalarKey));
						memcpy(&sckey,&key,sizeof(ScalarKey));
						RDFLOAT(&sckey.e[0].p);
#ifdef DBGKFB
						if(dbgio) 
						   printf("  scalar = (%.3f)\n", sckey.e[0].p);
#endif
						//theWorker->trackFlags[FALL_TRACK_INDEX] = trflags;
						theWorker->workNode->trackFlags[FALL_TRACK_INDEX] = trflags;
						
						if(!theWorker->AddFallKey(&sckey))
							return 0;
						}
						break;
					case FOV_TRACK_TAG:
						{
						ScalarKey sckey;
						memset(&sckey,0,sizeof(ScalarKey));
						memcpy(&sckey,&key,sizeof(ScalarKey));
						RDFLOAT(&sckey.e[0].p);
#ifdef DBGKFB
						if(dbgio) 
						   printf("  scalar = (%.3f)\n", sckey.e[0].p);
#endif
						//theWorker->trackFlags[FOV_TRACK_INDEX] = trflags;
						theWorker->workNode->trackFlags[FOV_TRACK_INDEX] = trflags;
						
						if(!theWorker->AddFOVKey(&sckey))
							return 0;
						}
						break;
					case ROLL_TRACK_TAG:
						{
						ScalarKey sckey;
						memset(&sckey,0,sizeof(ScalarKey));
						memcpy(&sckey,&key,sizeof(ScalarKey));
						RDFLOAT(&sckey.e[0].p);
#ifdef DBGKFB
						if(dbgio) 
						   printf("  scalar = (%.3f)\n", sckey.e[0].p);
#endif
						//theWorker->trackFlags[ROLL_TRACK_INDEX] = trflags;
						theWorker->workNode->trackFlags[ROLL_TRACK_INDEX] = trflags;
						
						if(!theWorker->AddRollKey(&sckey))
							return 0;
						}
						break;
#ifdef LATER
					case MORPH_TRACK_TAG: {
							MorphKey mkey;
							char mobname[12];
							int num;
							memset(&mkey,0,sizeof(MorphKey));
							memcpy(&mkey,&key,sizeof(MorphKey));
							if (!getstring(rstream,mobname,11)) return(0);
							if (merging) MaybeRemapName(mobname);
							num = nobj_number(mobname);
							if (num>=0) {
								mkey.object = get_named_ptr(num);
								APPEND_ELEMENT(&mkey,&tr->keytab);
								}
							}
						break;
					case HIDE_TRACK_TAG: {
							HideKey hkey;
							int num;
							memset(&hkey,0,sizeof(HideKey));
							memcpy(&hkey,&key,sizeof(key));
							APPEND_ELEMENT(&hkey,&tr->keytab);
							}
						break;
#endif
					}  /*switch */
				}  /* for */	
			chunk.size = 0;
			break;
		case M3DMAGIC:
			theWorker->gotM3DMAGIC = TRUE;
		case MMAGIC:
			while(chunk.size)
				{
				if(get_next_chunk(stream,&next)==0)
					return(0);

				switch(next.tag)
					{
					case MMAGIC:
					case NAMED_OBJECT:
					case KFDATA:
						if(get_mchunk(NULL)==0)
							return(0);
						break;
					case MASTER_SCALE:
						if(get_mchunk(NULL)==0)
							return(0);
						break;
#ifdef LATER
					case VIEWPORT_LAYOUT:
						if(merging)
							goto no_merge;
						if(get_vuechunk(stream,(void *)MODEEDT)==0)
							return(0);
						break;
					case MESH_VERSION:
					case O_CONSTS:
					case RAY_BIAS:
					case LO_SHADOW_BIAS:
					case SHADOW_MAP_SIZE:
					case SHADOW_FILTER:
					case SHADOW_RANGE:
					case RAY_SHADOWS:
#endif

					case AMBIENT_LIGHT:
					case BIT_MAP:
					case USE_BIT_MAP:
					case SOLID_BGND:
					case USE_SOLID_BGND:
					case USE_V_GRADIENT:
					case FOG:
					case USE_FOG:
					case LAYER_FOG:
					case USE_LAYER_FOG:
					case DISTANCE_CUE:
					case USE_DISTANCE_CUE:
					case V_GRADIENT:
						if(merging)
							goto no_merge;
						if(get_mchunk(&BG.bkgd_gradient)==0)
							return(0);
						break;
#ifdef LATER
					case APP_DATA:
						if(merging)
							goto no_merge;
						if(get_mchunk(&MSHappdata)==0)
							return(0);
						break;					
#endif
					case MAT_ENTRY:
						got_mat_chunk=1;
						loadmtl = &inmtl;
						/* Zero out data structure first */
						init_mtl_struct(loadmtl);
						if(get_mtlchunk(stream,NULL)==0)
							return(0);
						if(merging==0||(merging==1 /*&& mg_meshes*/ ))
							theWorker->AddMeshMtl(loadmtl);
						FreeMatRefs(loadmtl);
						break;     
					default:
						no_merge:
						if(skip_chunk(stream)==0)
							return(0);
						break;
					}
				chunk.size-=next.size;
				}
			break;
		case SMAGIC:
			while(chunk.size) {
				if(get_next_chunk(stream,&next)==0)
			    	return(0);

				switch(next.tag) {
					case POLY_2D:
						if(needShapeImportOptions) {
							needShapeImportOptions = FALSE;
							// Put up the options dialog!
							int result = DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_SHAPEIMPORTOPTIONS), theWorker->ip->GetMAXHWnd(), ShapeImportOptionsDlgProc, (LPARAM)theShapeImport);
							if(result <= 0) {
								shapeImportAbort = TRUE;
								return 0;
								}
							}
						if(!importShapes)
							goto skipit2;
						// Intentional fall-thru
				    case SHAPE_HOOK:
//					case SHAPE_OK:
//					case SHAPE_NOT_OK:
				    case MASTER_SCALE:
						if(get_mchunk(NULL)==0)
							return(0);
						break;
					default:
						skipit2:
						if(skip_chunk(stream)==0)
							return(0);
						break;
					}
			   chunk.size-=next.size;
			   }
			theWorker->FinishShape();
			break;
		 
		case SHAPE_HOOK:
			RDERR(&theWorker->hook_x,4);
			theWorker->hook_x*=msc_wk;
			RDERR(&theWorker->hook_y,4);
			theWorker->hook_y*=msc_wk;
			chunk.size-=8L;
			goto skiprest;

		case POLY_2D:
			if(!theWorker->StartSpline())
				return 0;

			RDERR(&wk_count,2);
			chunk.size-=2L;

			for(ix=0; ix<wk_count; ++ix)
				{
				Shppt point;
				Shppt *p= &point;
				RDERR(&point,sizeof(Shppt));

				if(msc_wk!=1.0) {
					p->x=p->x*msc_wk;	/* Adjust all pts to appropriate scale */
					p->y=p->y*msc_wk;
					p->z=0.0f;
					p->inx*=msc_wk;
					p->iny*=msc_wk;
					p->inz*=msc_wk;
					p->outx*=msc_wk;
					p->outy*=msc_wk;
					p->outz*=msc_wk;
					}
				p->x=p->x - theWorker->hook_x;	/* Adjust all pts to appropriate hook */
				p->y=p->y - theWorker->hook_y;
				p->z=0.0f;	// DB 2/29

				if(!theWorker->AddShapePoint(p))
					return 0;
				if(point.flags&POLYEND) {
					if(point.flags&POLYCLOSED)
						theWorker->CloseSpline();
					}
				chunk.size-=(long)sizeof(Shppt);
				}
			goto skiprest;

	#ifdef LATER
		case MESH_VERSION:
			RDERR(&mesh_version,4);
			chunk.size-=4L;
			goto skiprest;
	#endif
		case LIN_COLOR_F:
		case COLOR_F: {
			Color_f *cf=(Color_f *)data;
			RDERR(cf,12);
			chunk.size-=12L;
			goto skiprest;
			}
		case COLOR_24: {
			Color_24 c24;
			Color_f *cf=(Color_f *)data;
			RDERR(&c24,3);
			cf->r=(float)c24.r/255.0f;
			cf->g=(float)c24.g/255.0f;
			cf->b=(float)c24.b/255.0f;
			chunk.size-=3L;
			goto skiprest;
			}
		case MASTER_SCALE: {
			RDERR(&msc_wk,4);
			chunk.size-=4L;
			int type;
			float scale;

			GetMasterUnitInfo(&type, &scale);
			float msc_factor = (float)GetMasterScale(autoConv ? UNITS_INCHES : type);
			msc_wk = (float)((double)msc_wk / msc_factor);		/* Turn into a mult factor */
			goto skiprest;
			}
//	#ifdef LATER
		case SOLID_BGND:
			{
			int got_gam=0,got_lin=0;
			Color gamcol,lincol;
			while (chunk.size)
				{
				if (get_next_chunk(stream,&next)==0) return(0);
				switch(next.tag) {
					case COLOR_F:	case COLOR_24:
						got_gam = 1;
						if (get_mchunk(&gamcol)==0) return(0);
						break;
					case LIN_COLOR_F:
						got_lin = 1;
						if (get_mchunk(&lincol)==0) return(0);
						break;
					default:
						if (skip_chunk(stream)==0)
							return(0);
						break;
					}
				chunk.size-=next.size;
				}
			if (gammaMgr.enable) {
				if (got_lin) 
					BG.bkgd_solid = lincol;
				else {
					if (!got_gam) return(0);
					fin_degammify(&BG.bkgd_solid, &gamcol);
					}
				}
			else { 
				if (!got_gam) return(0);
				BG.bkgd_solid = gamcol;
				}
			}
			break;
		case USE_SOLID_BGND:
			BG.bgType = BG_SOLID;
			goto skiprest;
		case BIT_MAP: {
			char buf[82];
			if(read_string(buf,stream,81)==0)
				return(0);
			chunk.size-=(long)(strlen(buf)+1);
			split_fn(NULL,BG.bkgd_map,buf);
			if(stricmp(BG.bkgd_map,"none")==0)
				BG.bkgd_map[0]=0;
			}
	/*	DebugPrint(BG.bkgd_map,"%s\\%s",P.mapdrawer,gp_buffer);*/
			goto skiprest;
		case USE_BIT_MAP:
			BG.bgType = BG_BITMAP;
			goto skiprest;
		case V_GRADIENT:
			{
			Color cols[3],lcols[3];
			int lcolors,colors;
			gd=(Bkgrad *)data;
			RDERR(&gd->midpct,sizeof(float));
			chunk.size-=(long)sizeof(float);

		/* Make defaults black */
			gd->botcolor.r = gd->botcolor.g = gd->botcolor.b = 0.0f;
			gd->midcolor = gd->topcolor = gd->botcolor;

		/* Now read up to 3 colors */

			lcolors = colors = 0;
			while(chunk.size)
				{
				if(get_next_chunk(stream,&next)==0)	return(0);
				switch(next.tag)
					{
					case COLOR_F:
					case COLOR_24:
						if(get_mchunk(&cols[colors])==0) return(0);
						colors++;
						break;
					case LIN_COLOR_F:
						if(get_mchunk(&lcols[lcolors])==0) return(0);
						lcolors++;
						break;
					default:
						if(skip_chunk(stream)==0)	return(0);
						break;
					}
				chunk.size-=next.size;
				}
			if (gammaMgr.enable) {
				if (lcolors) {
					if (lcolors>0) gd->botcolor = lcols[0];
					if (lcolors>1) gd->midcolor = lcols[1];
					if (lcolors>2) gd->topcolor = lcols[2];
					}
				else {
					if (colors>0) fin_degammify(&gd->botcolor, &cols[0]);
					if (colors>1) fin_degammify(&gd->midcolor, &cols[1]);
					if (colors>2) fin_degammify(&gd->topcolor, &cols[2]);
					}
				}
			else {
				if (colors>0) gd->botcolor = cols[0];
				if (colors>1) gd->midcolor = cols[1];
				if (colors>2) gd->topcolor = cols[2];
				}
			}
			break;
		case USE_V_GRADIENT:
			BG.bgType = BG_GRADIENT;
			goto skiprest;
		case AMBIENT_LIGHT:
			while(chunk.size)
				{
				if(get_next_chunk(stream,&next)==0)
					return(0);
				switch(next.tag)
					{
					case COLOR_F:
					case COLOR_24:
						if(get_mchunk(&BG.amb_light)==0)
							return(0);
//						load_ambient_light();
						break;
					default:
						if(skip_chunk(stream)==0)
							return(0);
						break;
					}
				chunk.size-=next.size;
				}
			break;
		case FOG:
			RDERR(&BG.fog_data,sizeof(float)*4);
			BG.fog_data.nearplane *= msc_wk;
			BG.fog_data.farplane *= msc_wk;
			BG.distance_cue.nearplane=BG.fog_data.nearplane;
			BG.distance_cue.farplane=BG.fog_data.farplane;
			chunk.size-=(long)(4*sizeof(float));
			BG.fog_bg=0;
			while(chunk.size)
				{
				if(get_next_chunk(stream,&next)==0)
					return(0);
				switch(next.tag)
					{
					case FOG_BGND:
						if(get_mchunk(NULL)==0)
							return(0);
						break;
					case COLOR_F:
					case COLOR_24:
						if(get_mchunk(&BG.fog_data.color)==0)
							return(0);
						break;
					default:
						if(skip_chunk(stream)==0)
							return(0);
						break;
					}
				chunk.size-=next.size;
				}
			break;
		case LAYER_FOG:
			RDERR(&BG.lfog_data,sizeof(float)*3);
			RDERR(&BG.lfog_data.type,sizeof(int));
			BG.lfog_data.zmin *= msc_wk;
			BG.lfog_data.zmax *= msc_wk;
			chunk.size-=(long)(4*sizeof(float));
			while(chunk.size)
				{
				if(get_next_chunk(stream,&next)==0)
					return(0);
				switch(next.tag)
					{
					case COLOR_F:
					case COLOR_24:
						if(get_mchunk(&BG.lfog_data.color)==0)
							return(0);
						break;
					default:
						if(skip_chunk(stream)==0)
							return(0);
						break;
					}
				chunk.size-=next.size;
				}
			break;
		case USE_FOG:
			BG.envType = ENV_FOG;
			goto skiprest;
		case FOG_BGND:
			BG.fog_bg=1;
			goto skiprest;
		case DISTANCE_CUE:
			RDERR(&BG.distance_cue,sizeof(float)*4);
			BG.distance_cue.nearplane *= msc_wk;
			BG.distance_cue.farplane *= msc_wk;
			BG.fog_data.nearplane=BG.distance_cue.nearplane;
			BG.fog_data.farplane=BG.distance_cue.farplane;
			chunk.size-=(long)(4*sizeof(float));
			BG.dim_bg=0;
			while(chunk.size)
				{
				if(get_next_chunk(stream,&next)==0)
					return(0);
				switch(next.tag)
					{
					case DCUE_BGND:
						if(get_mchunk(NULL)==0)
							return(0);
						break;
					default:
						if(skip_chunk(stream)==0)
							return(0);
						break;
					}
				chunk.size-=next.size;
				}
			break;
		case USE_LAYER_FOG:
			BG.envType = ENV_LAYFOG;
			goto skiprest;
		case USE_DISTANCE_CUE:
			BG.envType = ENV_DISTCUE;
			goto skiprest;
		case DCUE_BGND:
			BG.dim_bg=1;
			goto skiprest;
#ifdef LATER
		case O_CONSTS:
			RDERR(&const_x,sizeof(float)*3);
			chunk.size-=(long)(sizeof(float)*3);
			goto skiprest;
		case RAY_SHADOWS:
			P.ray_shadows = 1;
			goto skiprest;
		case SHADOW_MAP_SIZE:
			RDERR(&wk_count,2);
			P.shadsize=wk_count;
			chunk.size-=2L;
			goto skiprest;
		case RAY_BIAS:
			RDERR(&P.ray_bias,sizeof(float));
			chunk.size-=sizeof(float);
			goto skiprest;
		case LO_SHADOW_BIAS:
			RDERR(&P.lo_bias,sizeof(float));
			chunk.size-=sizeof(float);
			goto skiprest;
		case SHADOW_RANGE:
			RDERR(&wk_count,2);
			P.shadfilter=(float)wk_count;
			chunk.size-=2L;
			goto skiprest;
		case SHADOW_FILTER:
			RDERR(&P.shadfilter,sizeof(float));
			chunk.size-=sizeof(float);
			goto skiprest;
#endif
		case DL_RANGE:
		case DL_OUTER_RANGE:
		case DL_INNER_RANGE: {
			float *fptr=(float *)data;
			RDERR(fptr,sizeof(float));
			*fptr *= msc_wk;
			chunk.size-=sizeof(float);
			goto skiprest;
			}
		case DL_MULTIPLIER:
			d=(Dirlight *)data;
			RDERR(&d->mult,sizeof(float));
			chunk.size-=sizeof(float);
			goto skiprest;
		case NAMED_OBJECT:
			if(read_string(obname,stream,OBJ_NAME_LEN+1)==0)
				return(0);
			chunk.size-=(long)(strlen(obname)+1);

	#ifdef LATER
			if(merging==1 && merge_this(obname)==0)
				skipping=1;
			else
				skipping=0;
	#else
			skipping = 0;
	#endif // LATER
			obnbr= -1;
			while(chunk.size)
				{
				if(get_next_chunk(stream,&next)==0)
					return(0);
				if(skipping) {
					if(skip_chunk(stream)==0) // skip obj
						return(0);
                }
				switch(next.tag)
					{
					case N_TRI_OBJECT:
						if(!theWorker->StartMesh(obname))
							return 0;
						if(get_mchunk(theWorker)==0)
							return 0;
						break;
					case N_D_L_OLD:
					case N_DIRECT_LIGHT: {
						Dirlight &d = theWorker->studioLt;
						// Init the direct light to an omni
						d.x=d.y=d.z=d.tx=d.ty=d.tz=0.0f;
						d.flags=NO_LT_ON;
						d.color.r=d.color.g=d.color.b=1.0f;
						d.hotsize=d.fallsize=360.0f;
						d.lo_bias=3.0f;	// TO DO: USE GLOBAL?
//						d.exclude=NULL;
						d.shadsize=256;	// TO DO: USE GLOBAL?
						d.in_range=10.0f;
						d.out_range=100.0f;
						d.shadfilter=5.0f;	// TO DO: USE GLOBAL?
						strcpy(d.imgfile,"");
						d.ray_bias = 0.2f;	// TO DO: USE GLOBAL?
						d.bank=0.0f;
						d.aspect=1.0f;
						d.mult=1.0f;
						d.appdata = NULL;
						if(!theWorker->StartLight(obname))
							return 0;
						if(get_mchunk(&d)==0)
							return 0;
						int type = d.hotsize==360.0f ? OMNI_LIGHT : TSPOT_LIGHT;
						if(!theWorker->CreateLight(type))
							return 0;
						}
						break;
					case N_CAM_OLD:
					case N_CAMERA: {
						Camera3DS &c = theWorker->studioCam;
						// Init the camera struct
						c.x=c.y=c.z=c.tx=c.ty=c.tz=c.bank=0.0f;
						c.focal=50.0f;
						c.flags=0;
						c.nearplane=10.0f;
						c.farplane=1000.0f;
						c.appdata = NULL;
						if(!theWorker->StartCamera(obname))
							return 0;
						if(get_mchunk(&c)==0)
							return 0;
						if(!theWorker->CreateCamera(TARGETED_CAMERA))
							return 0;
						}
						break;

	#ifdef FIXTHIS
						/*----  Keyframer code */
						int dupres=0;
						Namedobj *dupob;
						char origobname[OBJ_NAME_LEN+1];
						strcpy(origobname,obname);
						if (do_replace_msh) {
							if (obj_unique(obname,-1)) {
								skipping = 1;
								if(skip_chunk(stream)==0)  // skip obj
									return(0);
							}
							else {
								/*replace the object with the same name */
								dupob = get_named_ptr(nobj_number(obname));
								if (dupob->type!=ObTypeFromTag(next.tag)) {
									if(skip_chunk(stream)==0)  // skip obj
										return(0);
                                }
								dont_del_ob_refs = 1;
								kill_named(obname,0);
								dont_del_ob_refs = 0;
								dupres = 2;
								}
							}
						else {
							if (merging)
								while(obj_unique(obname,-1)==0)	{
									dupob = get_named_ptr(nobj_number(obname));
									check_dup_type = 1;
									merge_nob_type = next.tag;
									dupres = dup_name(obname,progstr(STR0057),progstr(STR0546));
									check_dup_type = 0;
									if (dupres==0)  {  /* SKIPPING  object */
										if (dupob->type!=N_TRI_OBJECT)  
											RegisterObNameChg(origobname,"$_$_$_$");
										else {
											/* turn off '*' for this name in the merge list */
											dont_merge_this(origobname);
											}
										skipping=1;
										if(skip_chunk(stream)==0)  // skip obj
											return(0);
									}
									if (dupres==2) {
										break; 
										}
									}
							}
						if((obnbr=create_named_obj(obname,next.tag,&n)<0))	{
							bad_reason=PARTIAL_READ;
							return(0);
							}
						if (merging) {
							if (dupres==2) /* Delete OLD */
								RemapNodeObRefs(dupob,n);
							else if (dupres==1)  /* Rename NEW object */
								RegisterObNameChg(origobname,obname);								
							}
						if (do_replace_msh) 
							RemapNodeObRefs(dupob,n);
				   	}  /* end of merge_it block */
						n->flags |= NO_NEED_DRAW;
						if(get_mchunk(stream,n->dstruct)==0)
							{
							kill_object(obnbr);
							return(0);
							}
						if(P.detail==0)
							set_ob_sels(OBJECTS-1,OBJECTS);
#else
						if(get_mchunk(NULL)==0)
							return 0;
#endif
						break;
					
					case OBJ_DOESNT_CAST:
					case OBJ_DONT_RCVSHADOW:
						if (get_mchunk(NULL)==0)
							return(0);
						break;
#ifdef LATER
					case OBJ_HIDDEN:
					case OBJ_VIS_LOFTER:
					case OBJ_FAST:
					case OBJ_FROZEN:
					case OBJ_DOESNT_CAST:
					case OBJ_MATTE:
					case OBJ_DONT_RCVSHADOW:
					case OBJ_PROCEDURAL:
						if(merging && mg_meshes==0) {
							if(skip_chunk(stream)==0)  // skip obj
								return(0);
                        }
						if(obnbr<0)
							return(0);
						/* n = get_named_ptr(obnbr); */
						if (get_mchunk(stream,n)==0)
							return(0);
						break;
	#endif
					default:
						if(skip_chunk(stream)==0) // skip obj
							return(0);
						break;
					}
				chunk.size-=next.size;
				}
			break;
				
		case OBJ_DOESNT_CAST:
			theWorker->cstShad = 0;
			break;

		case OBJ_DONT_RCVSHADOW:			
			theWorker->rcvShad = 0;
			break;

	#ifdef LATER
		case OBJ_HIDDEN:
			hiddens=1;
			n=(Namedobj *)data;
			n->flags |= NO_HIDDEN;
			goto skiprest;
		case OBJ_DOESNT_CAST:
			n=(Namedobj *)data;
			n->flags |= NO_DOESNT_CAST;
			goto skiprest;
		case OBJ_MATTE:
			n=(Namedobj *)data;
			n->flags |= NO_MATTE;
			goto skiprest;
	   case OBJ_DONT_RCVSHADOW:
			n=(Namedobj *)data;
			n->flags |= NO_DONT_RCVSHADOW;
			goto skiprest;
		case OBJ_PROCEDURAL:
			n=(Namedobj *)data;
			n->flags |= NO_PROCEDURAL;
			goto skiprest;
		case OBJ_FAST:										 
			n=(Namedobj *)data;
			n->flags |= NO_FAST;
			goto skiprest;
		case OBJ_FROZEN:
			n=(Namedobj *)data;
			n->flags |= NO_FROZEN;
			goto skiprest;
		case OBJ_VIS_LOFTER:
			n=(Namedobj *)data;
			n->flags |= NO_SEE_LOFT;
			goto skiprest;
#endif
		case APP_DATA:			
			theWorker->LoadAppData(stream,chunk.size);
			break;
// TO DO: Actually load & store appdata
//			if (!load_app_data(stream, data, chunk.size))
//				return(0);
//			chunk.size = 0;
//			goto skiprest;
		

		case N_TRI_OBJECT:
	#ifdef FIXTHIS
			t=(Tri_obj *)data;
			t->color=0;	/* Default to good ol' white */
	#endif
			while(chunk.size)
				{
				if(get_next_chunk(stream,&next)==0)
					return(0);
				switch(next.tag)
					{
					case POINT_ARRAY:
					case FACE_ARRAY:
					case MESH_MATRIX:
					case TEX_VERTS:
						if(get_mchunk(data)==0)
							return(0);
						break;

					case APP_DATA:
						if(get_mchunk(data)==0)
							return(0);
						break;
	#ifdef LATER
					case POINT_FLAG_ARRAY:
					case MESH_TEXTURE_INFO:
					case PROC_NAME:
					case PROC_DATA:
					case MESH_COLOR:
						if(get_mchunk(stream,data)==0)
							return(0);
						break;
					case APP_DATA:
						if(get_mchunk(stream,&t->appdata)==0)
							return(0);
						break;
	#endif
					default:
						if(skip_chunk(stream)==0)
							return(0);
						break;
					}
				chunk.size-=next.size;
				}
	#ifdef LATER
			updt_ob_box(data); 
	#endif
			break;
		case POINT_ARRAY:
			RDERR(&wk_count,2);
			chunk.size-=2L;
			if(!theWorker->SetVerts((int)wk_count))
				return 0;
			for(ix=0; ix<wk_count; ++ix)
				{
				RDERR(&v,sizeof(float)*3);
				if(msc_wk!=1.0)
					{
					v.x*=msc_wk;
					v.y*=msc_wk;
					v.z*=msc_wk;
					}
				v.flags=0;
				if(!theWorker->PutVertex(ix,&v))
					return 0;
				chunk.size-=(long)(sizeof(float)*3);
				}
			goto skiprest;
		case TEX_VERTS:
			RDERR(&wk_count,2);
			chunk.size-=2L;
			if(!theWorker->SetTVerts((int)wk_count))
				return 0;
			tv.z = 0.0f;
			for(ix=0; ix<wk_count; ++ix)
				{
				RDERR(&tv,sizeof(float)*2);
				// filter out bogus texture coord values: old versions of 
				// 3ds could have 0xfefefefe as the value.
				// This should catch those and other bad values 
				// derived from them.  --DS 3/16/96
				if (tv.x>1.0e10) tv.x = 0.0f;
				if (tv.x<-1.0e10) tv.x = 0.0f;
				if (tv.y>1.0e10) tv.y = 0.0f;
				if (tv.y<-1.0e10) tv.y = 0.0f;
				if(!theWorker->PutTVertex(ix,&tv))
					return 0;
				chunk.size-=(long)(sizeof(float)*2);
				}
			goto skiprest;
	#ifdef LATER
		case PROC_NAME:
			t=(Tri_obj *)data;
			if(read_string(t->proc_name,stream,9)==0)
				return(0);
			chunk.size-=(long)(strlen(t->proc_name)+1);
			goto skiprest;
		case PROC_DATA: 
			{
			ULONG *plong;
			t=(Tri_obj *)data;
			if (t->flags&TRI_HAS_PROCDATA) {
				XMFreeAndZero(&t->proc_data);
				t->flags &= ~TRI_HAS_PROCDATA;
				}
			t->proc_data = (void *)XMAlloc(chunk.size+4);
			if (t->proc_data==NULL) 
				return(0);			
			plong = (ULONG *)t->proc_data;
			plong[0] = chunk.size;
			RDERR(&plong[1],chunk.size);
			t->flags |= TRI_HAS_PROCDATA;
			chunk.size = 0;
			goto skiprest;
			}
		case MESH_TEXTURE_INFO:
			t=(Tri_obj *)data;
			RDERR(&Mapinfo,sizeof(Mapinfo));
			chunk.size-=sizeof(Mapinfo);
			t->maptype=Mapinfo.maptype;
			t->tile_x=Mapinfo.tile_x;
			t->tile_y=Mapinfo.tile_y;
			t->map_x=Mapinfo.map_x;
			t->map_y=Mapinfo.map_y;
			t->map_z=Mapinfo.map_z;
			t->map_scale=Mapinfo.map_scale;
			memcpy(t->map_matrix,Mapinfo.map_matrix,sizeof(float)*12);
			t->map_pw=Mapinfo.map_pw;
			t->map_ph=Mapinfo.map_ph;
			t->map_ch=Mapinfo.map_ch;
			goto skiprest;
	#endif
		case POINT_FLAG_ARRAY:
	#ifdef FIXTHIS
			t=(Tri_obj *)data;
			RDERR(&wk_count,2);
			chunk.size-=2L;
			for(ix=0; ix<t->verts; ++ix)
				{
				get_vert(t,ix,&v);
				RDERR(&v.flags,sizeof(short));
				put_vert(t,ix,&v);
				chunk.size-=(long)sizeof(short);
				}
	#endif
			goto skiprest;
		case FACE_ARRAY:
			RDERR(&wk_count,2);
			chunk.size-=2L;
			if(!theWorker->SetFaces((int)wk_count))
				return 0;
			for(ix=0; ix<wk_count; ++ix)
				{
				RDERR(&Fc_wrt,8);
/*
				if (mesh_version<2)
					Fc_wrt.flags &= ~VWRAP;
*/
				f.a=Fc_wrt.a;
				f.b=Fc_wrt.b;
				f.c=Fc_wrt.c;
				f.material = 0;		/* None! */
				f.sm_group = 0;		/* None! */
				f.flags=Fc_wrt.flags;
				if(!theWorker->PutFace(ix,&f))
					return 0;
				chunk.size-=8L;
				}

			while(chunk.size)
				{
				if(get_next_chunk(stream,&next)==0)
					return(0);
				switch(next.tag)
					{
					case MSH_MAT_GROUP:
	#ifdef LATER
					case OLD_MAT_GROUP:
					case MSH_BOXMAP:
	#endif
					case SMOOTH_GROUP:
						if(get_mchunk(NULL)==0)
							return(0);     
						break;
					default:
						if(skip_chunk(stream)==0)
							return(0);
						break;
					}
				chunk.size-=next.size;
				}
			break;
		case MESH_MATRIX:
			{
			Matrix3 tm;
			RDERR(tm.GetAddr(),sizeof(float)*12);
			tm.SetNotIdent();
			if(msc_wk!=1.0)
				{
				tm.SetTrans(tm.GetTrans()*msc_wk);
				}
			theWorker->SetTm(&tm);
			chunk.size-=48L;
			goto skiprest;
			}
		case MESH_COLOR:
	#ifdef FIXTHIS
			t=(Tri_obj *)data;
			RDERR(&t->color,sizeof(uchar));
			if(t->color>=OBJ_COLOR_MAX)
				t->color=0;
	#else
	fseek(stream,1,SEEK_CUR);
	#endif
			chunk.size-=1;
			goto skiprest;

		case MSH_MAT_GROUP:	{
			char mtlname[40];
			int mtlnum;
			unsigned short wkface;
//			if(got_mat_chunk==0)
//			 	antiquated=1;
			if(read_string(mtlname,stream,17)==0)
				return(0);
			chunk.size-=(long)(strlen(mtlname)+1);

			mtlnum = theWorker->GetMatNum(mtlname);
			
			RDERR(&wk_count,2);
			chunk.size-=2L;
			for(ix=0; ix<wk_count; ++ix)
				{
				RDERR(&wkface,2);
				chunk.size -= 2L;   				
				if(!theWorker->PutFaceMtl(wkface, mtlnum))
					return 0;				
				}
			}
			goto skiprest;

	#ifdef LATER
		case OLD_MAT_GROUP:
			antiquated=1;
		case MSH_BOXMAP:
			t=(Tri_obj *)data;
			t->flags |= TRI_BOX_MAP;
			for(ix=0; ix<6; ++ix) {
				if(read_string(mtlname,stream,17)==0)
					return(0);
	 			chunk.size-=(long)(strlen(mtlname)+1);
				if (strcmp(mtlname,"DEFAULT")==0) 
					t->boxmtl[ix]=255;
				else
					t->boxmtl[ix]=inst_material(mtlname,0);
	#ifdef STAND_ALONE
				printf(" Box material %s , # %d \n",mtlname,t->boxmtl[ix]);
	#endif
				}
			goto skiprest;
	#endif
		case SMOOTH_GROUP:
			{
			int faces = theWorker->GetFaces();
			unsigned long smgroup;
			for(ix=0; ix<faces; ++ix)
				{
				RDERR(&smgroup,sizeof(unsigned long));
				chunk.size-=sizeof(unsigned long);
				if(!theWorker->PutSmooth(ix,smgroup))
					return 0;
				}
			goto skiprest;
			}
		case N_D_L_OLD: {
			d = (Dirlight *)data;
			RDERR(&d->x,sizeof(float));
			RDERR(&d->y,sizeof(float));
			RDERR(&d->z,sizeof(float));
			if(msc_wk != 1.0)
				{
				d->x *= msc_wk;
				d->y *= msc_wk;
				d->z *= msc_wk;
				}
			RDERR(&fdum,sizeof(float));
			RDERR(&fdum,sizeof(float));
			RDERR(&d->hotsize,sizeof(float));
			d->fallsize=d->hotsize;
			RDERR(&d->flags,2);
		
			chunk.size-=26L;
			while(chunk.size)
				{
				if(get_next_chunk(stream,&next)==0)
					return(0);
				switch(next.tag)
					{
					case COLOR_F:
					case COLOR_24:
						if(get_mchunk(&d->color)==0)
							return(0);
						break;
					default:
						if(skip_chunk(stream)==0)
							return(0);
						break;
					}
				chunk.size-=next.size;
				}
			}
			break;
		case N_CAM_OLD: {
			Camera3DS *c = (Camera3DS *)data;
			RDERR(&c->x,sizeof(float));
			RDERR(&c->y,sizeof(float));
			RDERR(&c->z,sizeof(float));
			if(msc_wk != 1.0)
				{
				c->x *= msc_wk;
				c->y *= msc_wk;
				c->z *= msc_wk;
				}
			RDERR(&fdum,sizeof(float));
			RDERR(&fdum,sizeof(float));
			RDERR(&fdum,sizeof(float));
			chunk.size-=24L;
			goto skiprest;
			}
		case N_DIRECT_LIGHT: {
			d = (Dirlight *)data;
			RDERR(&d->x,sizeof(float));
			RDERR(&d->y,sizeof(float));
			RDERR(&d->z,sizeof(float));
			if(msc_wk != 1.0)
				{
				d->x *= msc_wk;
				d->y *= msc_wk;
				d->z *= msc_wk;
				}
				
			chunk.size-=12L;
			while(chunk.size)
				{
				if(get_next_chunk(stream,&next)==0)
					return(0);
				switch(next.tag)
					{
					case COLOR_F:
					case COLOR_24:
						if(get_mchunk(&d->color)==0)
							return(0);
						break;
					case DL_RANGE:	/* Old style chunk */
						if(get_mchunk(&d->out_range)==0)
							return(0);
						d->in_range=d->out_range/10.0f;
						if(d->out_range==d->in_range)
							d->out_range+=1.0f;
						break;
					case DL_INNER_RANGE:
						if(get_mchunk(&d->in_range)==0)
							return(0);
						break;
					case DL_OUTER_RANGE:
						if(get_mchunk(&d->out_range)==0)
							return(0);
						break;
					case DL_EXCLUDE:
					case DL_SPOTLIGHT:
					case DL_OFF:
					case DL_ATTENUATE:
					case DL_MULTIPLIER:
						if(get_mchunk(d)==0)
							return(0);
						break;
					case APP_DATA:
						if(get_mchunk(&d->appdata)==0)
							return(0);
						break;
					default:
						if(skip_chunk(stream)==0)
							return(0);
						break;
					}
				chunk.size-=next.size;
				}
			}
			break;
		case DL_SPOTLIGHT: {
			d = (Dirlight *)data;
			RDERR(&d->tx,sizeof(float));
			RDERR(&d->ty,sizeof(float));
			RDERR(&d->tz,sizeof(float));
			if(msc_wk != 1.0)
				{
				d->tx *= msc_wk;
				d->ty *= msc_wk;
				d->tz *= msc_wk;
				}
			RDERR(&d->hotsize,sizeof(float));
			RDERR(&d->fallsize,sizeof(float));

			/* Enforce keep .5 degree gap for antialiasing */
			if (d->hotsize>d->fallsize-.5&&d->fallsize>=.5f) 
				d->hotsize = d->fallsize-.5f;
			
			chunk.size-=20L;
			while(chunk.size)
				{
				if(get_next_chunk(stream,&next)==0)
					return(0);
				switch(next.tag)
					{
					case DL_SHADOWED:
					case DL_LOCAL_SHADOW:
					case DL_LOCAL_SHADOW2:
					case DL_SEE_CONE:
					case DL_SPOT_RECTANGULAR:
					case DL_SPOT_OVERSHOOT:
					case DL_SPOT_PROJECTOR:
					case DL_SPOT_ROLL:
					case DL_SPOT_ASPECT:
					case DL_RAY_BIAS:
					case DL_RAYSHAD:
						if(get_mchunk(d)==0)
							return(0);
						break;
					default:
						if(skip_chunk(stream)==0)
							return(0);
						break;
					}
				chunk.size-=next.size;
				}
			}
			break;
//#ifdef LATER
		case DL_EXCLUDE: {
			d=(Dirlight *)data;
			TCHAR oname[12];
			if(read_string(oname,stream,11)==0)
				return(0);
			chunk.size-=(long)(strlen(oname)+1);
			//if(add_exclude_list(oname,d)<=0)
			//	return(0);
			d->excList.AddName(oname);
			goto skiprest;
			}

//#endif // LATER
		case DL_SPOT_ROLL:
			d=(Dirlight *)data;
			RDERR(&d->bank,sizeof(float));
			chunk.size-=(long)(sizeof(float));
			goto skiprest;
		case DL_SPOT_ASPECT:
			d=(Dirlight *)data;
			RDERR(&d->aspect,sizeof(float));
			chunk.size-=(long)(sizeof(float));
			goto skiprest;
		case DL_RAY_BIAS:
			d=(Dirlight *)data;
			RDERR(&d->ray_bias,sizeof(float));
			chunk.size-=(long)(sizeof(float));
			goto skiprest;
		case DL_OFF:
			d=(Dirlight *)data;
			d->flags &= NO_LT_OFF;
			goto skiprest;
		case DL_ATTENUATE:
			d=(Dirlight *)data;
			d->flags |= NO_LT_ATTEN;
			goto skiprest;
		case DL_RAYSHAD:
			d=(Dirlight *)data;
			d->flags |= NO_LT_RAYTR;
			goto skiprest;
		case DL_SHADOWED:
			d=(Dirlight *)data;
			d->flags |= NO_LT_SHAD;
			goto skiprest;
		case DL_SEE_CONE:
			d=(Dirlight *)data;
			d->flags |= NO_LT_CONE;
			goto skiprest;
		case DL_SPOT_RECTANGULAR:
			d=(Dirlight *)data;
			d->flags |= NO_LT_RECT;
			goto skiprest;
		case DL_SPOT_OVERSHOOT:
			d=(Dirlight *)data;
			d->flags |= NO_LT_OVER;
			goto skiprest;
		case DL_SPOT_PROJECTOR:
			d=(Dirlight *)data;
			d->flags |= NO_LT_PROJ;
			RDERR(d->imgfile,13);
			chunk.size-=13;
			goto skiprest;
		case DL_LOCAL_SHADOW: {
			Locshad Loc_shadwrt;
			d=(Dirlight *)data;
			d->flags |= NO_LT_LOCAL;
			RDERR(&Loc_shadwrt,sizeof(Locshad));
			chunk.size-=sizeof(Locshad);
			d->lo_bias=Loc_shadwrt.lo_bias;
			d->shadsize=Loc_shadwrt.shadsize;
			d->shadfilter=(float)Loc_shadwrt.shadrange;
			goto skiprest;
			}
		case DL_LOCAL_SHADOW2:
			{
			LocShad2 locShad2;
			d=(Dirlight *)data;
			d->flags |= NO_LT_LOCAL;
			RDERR(&locShad2,sizeof(LocShad2));
			chunk.size-=sizeof(LocShad2);
			d->lo_bias=locShad2.bias;
			d->shadsize=locShad2.shadsize;
			d->shadfilter=locShad2.shadfilter;
			}
			goto skiprest;
		case N_CAMERA: {
			Camera3DS *c = (Camera3DS *)data;
			RDERR(c,32);
			if(msc_wk != 1.0)
				{
				c->x *= msc_wk;
				c->y *= msc_wk;
				c->z *= msc_wk;
				c->tx *= msc_wk;
				c->ty *= msc_wk;
				c->tz *= msc_wk;
				}
			chunk.size-=32L;
			while(chunk.size)
				{
				if(get_next_chunk(stream,&next)==0)
					return(0);
				switch(next.tag)
					{
					case APP_DATA:
						if(get_mchunk(&c->appdata)==0)
							return(0);
						break;
					case CAM_SEE_CONE:
					case CAM_RANGES:
						if(get_mchunk(c)==0)
							return(0);
						break;
					default:
						if(skip_chunk(stream)==0)
							return(0);
						break;
					}
				chunk.size-=next.size;
				}
			}
			break;
		case CAM_SEE_CONE:
			c=(Camera3DS *)data;
			c->flags |= NO_CAM_CONE;
			goto skiprest;
		case CAM_RANGES:
			c=(Camera3DS *)data;
			RDERR(&c->nearplane,sizeof(float));
			RDERR(&c->farplane,sizeof(float));
			c->nearplane *= msc_wk;
			c->farplane *= msc_wk;
			chunk.size-=(sizeof(float)*2);
			goto skiprest;

	/* The following routine is used to dump any sub-chunks	*/
	/* in the current chunk.				*/

			skiprest:
			while(chunk.size)
				{
				if(get_next_chunk(stream,&next)==0)
					return(0);
				if(skip_chunk(stream)==0)
					return(0);
				chunk.size-=next.size;
				}  
			break;
		default:
			assert(0);
			break;
		}
	return(1);
	}


static INT_PTR CALLBACK
ImportDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	switch(message) {
		case WM_INITDIALOG:
			SetWindowContextHelpId(hDlg, idh_3dsimp_import);
			CheckRadioButton( hDlg, IDC_3DS_MERGE, IDC_3DS_REPLACE, replaceScene?IDC_3DS_REPLACE:IDC_3DS_MERGE );
			CheckDlgButton( hDlg, IDC_3DS_CONVERT, autoConv );
			CenterWindow(hDlg,GetParent(hDlg));
			SetFocus(hDlg); // For some reason this was necessary.  DS-3/4/96
			return FALSE;
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDOK: {
	            	replaceScene = IsDlgButtonChecked(hDlg,IDC_3DS_REPLACE);
					autoConv = IsDlgButtonChecked(hDlg, IDC_3DS_CONVERT);
					EndDialog(hDlg, 1);
					}
					return TRUE;
				case IDCANCEL:
					EndDialog(hDlg, 0);
					return TRUE;
			}
		case WM_SYSCOMMAND:
			if ((wParam & 0xfff0) == SC_CONTEXTHELP) {
				DoHelp(HELP_CONTEXT, idh_3dsexp_export);
				return FALSE;
			}
		}
	return FALSE;
	}

int
StudioImport::DoImport(const TCHAR *filename,ImpInterface *i,Interface *gi, BOOL suppressPrompts) {
	// Set a global prompt display switch
	showPrompts = suppressPrompts ? FALSE : TRUE;

	WorkFile theFile(filename,_T("rb"));
	ObjWorker W(i,gi);
	theWorker = &W;

	if(suppressPrompts) {
		}
	else {
		if (!DialogBox(hInstance, MAKEINTRESOURCE(IDD_MERGEORREPL), gi->GetMAXHWnd(), ImportDlgProc))
			return IMPEXP_CANCEL;
		}

	dStream = i->DumpFile();

	if(!(stream = theFile.Stream())) {
		if(showPrompts)
			MessageBox(IDS_TH_ERR_OPENING_FILE, IDS_TH_3DSIMP);
		return IMPEXP_FAIL;						// Didn't open!
		}

	// Find out what kind of file it is...
	unsigned short header;
	if(!(fread(&header,2,1,stream))) {
		if(showPrompts)
			MessageBox(IDS_TH_3DSREADERROR, IDS_TH_3DSIMP);
		return IMPEXP_FAIL;						// No data!
		}
	if(header != MMAGIC && header != M3DMAGIC && header != CMAGIC) {
		if(showPrompts)
			MessageBox(IDS_TH_INVALIDFILE, IDS_TH_3DSIMP);
		return IMPEXP_FAIL;						// Wrong header!
		}
	fseek(stream,0L,SEEK_SET);			// Seek back to the beginning

	BOOL project = (header == CMAGIC) ? TRUE : FALSE;	
	StudioShapeImport shapeImport;
	theShapeImport = &shapeImport;

	// If it's a project file, give 'em shape options
	if(project)
		needShapeImportOptions = TRUE;
	else
		needShapeImportOptions = FALSE;
	shapeImportAbort = FALSE;

	if (replaceScene) {
		if (!i->NewScene())
			return IMPEXP_CANCEL;
		}

	SetCursor(LoadCursor(NULL, IDC_WAIT));
	theImport = this;

	// Init misc items (for now)
	memset(&BG,0,sizeof(BG));
	msc_wk = (float)1.0;
	nodeLoadNumber=-1;
	theWorker->AddMeshMtl(NULL); // Create "Default" mtl
	got_mat_chunk = 0;

	if(!get_mchunk(NULL)) {
		theWorker->Abandon();		// Abandon any pending object construction
		SetCursor(LoadCursor(NULL, IDC_ARROW));
		if(shapeImportAbort)		// If aborted by shape dialog, fake an OK return
			return IMPEXP_CANCEL;
		return IMPEXP_FAIL;
		}
	SetCursor(LoadCursor(NULL, IDC_ARROW));

	// Make sure any objects without nodes are used!
	if(!theWorker->CompleteScene())
		return IMPEXP_FAIL;

	if(!theWorker->SetupEnvironment()) {
		return IMPEXP_FAIL;
		}

	// convert NameTab's to NodeIDTab's
	if (!theWorker->FixupExclusionLists()) {
		return IMPEXP_FAIL;
		}

	return IMPEXP_SUCCESS;
	}

static int shapeButtons[] = { IDC_SINGLEOBJECT, IDC_MULTIPLEOBJECTS };
#define NUM_SHAPEBUTTONS 2

static void MaybeEnableOptions(HWND hDlg) {
	EnableWindow(GetDlgItem(hDlg, IDC_SINGLEOBJECT), importShapes);
	EnableWindow(GetDlgItem(hDlg, IDC_MULTIPLEOBJECTS), importShapes);
	}

static INT_PTR CALLBACK
ShapeImportOptionsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	static StudioShapeImport *imp;

	switch(message) {
		case WM_INITDIALOG:
			imp = (StudioShapeImport *)lParam;
			CheckDlgButton( hDlg, IDC_IMPORT_SHAPES, importShapes);
			CheckDlgButton( hDlg, shapeButtons[imp->importType], TRUE);
			MaybeEnableOptions(hDlg);
			CenterWindow(hDlg,GetParent(hDlg));
			return TRUE;
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDC_IMPORT_SHAPES:
					if(HIWORD(wParam) == BN_CLICKED) {
						importShapes = IsDlgButtonChecked(hDlg, IDC_IMPORT_SHAPES);
						MaybeEnableOptions(hDlg);
						}
					break;
				case IDOK: {
					// Unload values into StudioShapeImport statics
					for(int i = 0; i < NUM_SHAPEBUTTONS; ++i) {
						if(IsDlgButtonChecked(hDlg, shapeButtons[i])) {
							imp->importType = i;
							break;
							}
						}
					EndDialog(hDlg, 1);
					}
					return TRUE;
				case IDCANCEL:
					EndDialog(hDlg, 0);
					return TRUE;
			}
		}
	return FALSE;
	}

int
StudioShapeImport::DoImport(const TCHAR *filename,ImpInterface *i,Interface *gi, BOOL suppressPrompts) {
	// Set a global prompt display switch
	showPrompts = suppressPrompts ? FALSE : TRUE;

	WorkFile theFile(filename,_T("rb"));
	ObjWorker W(i,gi);
	theWorker = &W;

	if(suppressPrompts) {
		}
	else {
		if (!DialogBox(hInstance, MAKEINTRESOURCE(IDD_MERGEORREPL),  gi->GetMAXHWnd(), ImportDlgProc))
			return IMPEXP_CANCEL;
		}

	dStream = i->DumpFile();

	if(!(stream = theFile.Stream())) {
		if(showPrompts)
			MessageBox(IDS_TH_ERR_OPENING_FILE, IDS_TH_3DSIMP);
		return 0;						// Didn't open!
		}

	// Got the file -- Now put up the options dialog!
	if(suppressPrompts) { // Set default parameters here
		importShapes = TRUE;
		importType = MULTIPLE_SHAPES;
		}
	else {
		int result = DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_SHAPEIMPORTOPTIONS), gi->GetMAXHWnd(), ShapeImportOptionsDlgProc, (LPARAM)this);
		if(result <= 0)
			return IMPEXP_CANCEL;
		}

	if (replaceScene) {
		if (!i->NewScene())
			return IMPEXP_CANCEL;
		}

	theShapeImport = this;

	unsigned short header;
	if(!(fread(&header,2,1,stream))) {
		if(showPrompts)
			MessageBox(IDS_TH_3DSREADERROR, IDS_TH_3DSIMP);
		return 0;						// No data!
		}
	if(header != MMAGIC && header != SMAGIC) {
		if(showPrompts)
			MessageBox(IDS_TH_INVALIDFILE, IDS_TH_3DSIMP);
		return 0;						// Wrong header!
		}
	fseek(stream,0L,SEEK_SET);			// Seek back to the beginning
	
	// Init misc items (for now)
	msc_wk = (float)1.0;
	nodeLoadNumber=-1;
	theWorker->AddMeshMtl(NULL); // Create "Default" mtl
	got_mat_chunk = 0;
	
	int res = 0;

	// Load the file contents --
	if(get_mchunk(NULL)) {
		res = 1; 
		}
	else 
		theWorker->Abandon();		// Abandon any pending object construction

	theWorker->FreeUnusedMtls();
	return res;
	}

