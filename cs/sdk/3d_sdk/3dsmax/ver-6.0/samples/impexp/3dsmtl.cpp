/*******************************************************************
 *
 *    DESCRIPTION: 3DSMTL.CPP: 3dsr4 material utilities
 *
 *    AUTHOR:
 *
 *    HISTORY:    
 *
 *******************************************************************/
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
#include "gamma.h"


void XMFree(void *p) { if (p) free(p); }

void XMFreeAndZero(void **p) {
	if (p) {
		if (*p) {
			free(*p);
			*p = NULL;
			}
		}
	}

void *XMAlloc(int size) { return malloc(size); }

void *XMAllocZero(int size) { 
	void *p = malloc(size);
	memset(p,0,size);
	return p;
	}


static Color_24 blackcol = {0,0,0};
static Color_24 whitecol = {255,255,255};

int TexBlurToPct(float tb) {
	return((int)(tb*100.0+.5));	
	}

float PctToTexBlur(int p) {
	return((float)p/100.0f);	
	}

void ResetMapData(MapData *md, int n, int ismask) {
	memset(md,0,sizeof(MapData));
	if ((!ismask)&&(n==Nrefl)) {
		md->kind = 1;
		md->p.ref.acb.shade = REND_METAL;
		md->p.ref.acb.aalevel = 0;
		md->p.ref.acb.flags = 0;
		md->p.ref.acb.size = 100;
		md->p.ref.acb.nth = 1;
		}
	else {
		md->p.tex.texblur = PctToTexBlur(10 /*P.texture_blur_default*/);
		md->p.tex.uscale = 1.0f;
		md->p.tex.vscale = 1.0f;
		md->p.tex.uoffset = 0.0f;
		md->p.tex.voffset = 0.0f;
		md->p.tex.ang_sin = 0.0f;
		md->p.tex.ang_cos = 1.0f;
		md->p.tex.col1 = blackcol;
		md->p.tex.col2 = whitecol;
		md->p.tex.rcol = 	md->p.tex.gcol = 	md->p.tex.bcol = blackcol;
		md->p.tex.rcol.r = 255;
		md->p.tex.gcol.g = 255;
		md->p.tex.bcol.b = 255;
		}
	}

void InitMappingValues(Mapping *m, int n, int isRmtl) {
	if (isRmtl) m->amt.f = 1.0f;
	else m->amt.pct = 100;
	ResetMapData(&m->map,n,0);
	ResetMapData(&m->mask,n,1);
	}


void FreeMapDataRefs(MapData *md) {
//	Cubmap *cm,*nextcm;
	switch(md->kind) {
		case 0:
			XMFreeAndZero(&md->p.tex.sxp_data);
			break;
		case 1:
#if 0
			if (md->p.ref.acb.flags&AC_ON) {
				for (cm =(Cubmap *)md->p.ref.bm; cm!=NULL; cm=nextcm) {
					nextcm = cm->next;
					XMFree(cm);
					}
				}
			else{
				char ext[5];
				split_fext(md->name,NULL,ext);
				if(stricmp(ext,".CUB")==0) {
					if (md->p.ref .bm!=NULL)
						XMFree(md->p.ref.bm);
					}
				}
#endif
			break;
		}	
	}

void FreeMatRefs(SMtl *m) {
	int k;
	if (m->appdata) XMFreeAndZero(&m->appdata);
	for (k=0; k<NMAPTYPES; k++) {
		if (m->map[k]) {
			FreeMapDataRefs(&m->map[k]->map);
			FreeMapDataRefs(&m->map[k]->mask);
			XMFreeAndZero((void **)&m->map[k]);
			}
		}
	}


void ResetMapping(Mapping *m, int n, int isRmtl) {
	FreeMapDataRefs(&m->map);
	FreeMapDataRefs(&m->mask);
	memset(m,0,sizeof(Mapping));
	InitMappingValues(m,n,isRmtl);
	}

Mapping *NewMapping(int n,int isRmtl) {
	Mapping *m;
	m = (Mapping*)XMAllocZero(sizeof(Mapping));
	if (m==NULL) return(NULL);
	InitMappingValues(m,n,isRmtl);
	return(m);
	}


void init_mtl_struct(SMtl *mtl) {
	memset(mtl,0,sizeof(SMtl));
	mtl->shininess=50;
	mtl->shin2pct = 255; /* undefined */
	mtl->transparency=0;
	mtl->shading=3;
	mtl->wiresize = 1.0f;
	}

void set_mtl_decal(SMtl *mtl) {
	int i;
	Mapping *m;
	for (i=0; i<NMAPTYPES; i++) {
		if ((m=mtl->map[i])!=NULL) {
			if (m->map.kind==0)
				m->map.p.tex.texflags |= TEX_DECAL|TEX_NOWRAP;
			if (m->mask.kind==0)
				m->mask.p.tex.texflags |= TEX_DECAL|TEX_NOWRAP;
			}
		}
	}
