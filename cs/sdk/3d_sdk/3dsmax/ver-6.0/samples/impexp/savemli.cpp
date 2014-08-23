/* Native binary file writer */

#include "Max.h"
#include <stdio.h>
#include <math.h>
#include <commdlg.h>
#include "splshape.h"
#include "3dsires.h"
#include "dummy.h"
#include "imtl.h"
#include "3dsimp.h"
#include "mtldef.h"
#include "gamma.h"

extern int dump_mtlchunk(ushort tag,FILE *stream,void *data);

#define WRITEF(ptr,size) fwrite((char *)ptr,1,size,stream)
#define WERR(ptr,sz) {if (WRITEF(ptr,(sz))!=(sz)) return(0);}
#define WRTERR(ptr,sz) {if (WRITEF(ptr,(sz))!=(sz)) return(0);}
#define WRFLOAT(ptr) WERR(ptr,sizeof(FLOAT))
#define WR3FLOAT(ptr) WERR(ptr,3*sizeof(FLOAT))
#define WRLONG(ptr) WERR(ptr,sizeof(LONG))
#define WRSHORT(ptr) WERR(ptr,sizeof(SHORT))
#define WRSTRING(ptr) WERR(ptr,strlen(ptr)+1)

//extern Mliblist *mlib;
Mmtllist *mmtl;
SMtl *savemtl,*savemlist;
int savecount;


static ULONG map_chunks[8] = {
	MAT_TEXMAP,MAT_TEX2MAP,MAT_OPACMAP,MAT_BUMPMAP,
	MAT_SPECMAP,MAT_SHINMAP,MAT_SELFIMAP,MAT_REFLMAP
	};
static ULONG mask_chunks[8] = {
	MAT_TEXMASK,MAT_TEX2MASK,MAT_OPACMASK,MAT_BUMPMASK,
	MAT_SPECMASK,MAT_SHINMASK,MAT_SELFIMASK,MAT_REFLMASK
	};
static ULONG map_sxp_chunks[8] = {
	MAT_SXP_TEXT_DATA,MAT_SXP_TEXT2_DATA,MAT_SXP_OPAC_DATA,MAT_SXP_BUMP_DATA,
	MAT_SXP_SPEC_DATA,MAT_SXP_SHIN_DATA,MAT_SXP_SELFI_DATA,0	
	};
static ULONG mask_sxp_chunks[8] = {
	MAT_SXP_TEXT_MASKDATA,MAT_SXP_TEXT2_MASKDATA,MAT_SXP_OPAC_MASKDATA,MAT_SXP_BUMP_MASKDATA,
	MAT_SXP_SPEC_MASKDATA,MAT_SXP_SHIN_MASKDATA,MAT_SXP_SELFI_MASKDATA,MAT_SXP_REFL_MASKDATA	
	};

int libtype;

#ifdef LATER
/* Save materials (0=lib, 1=current) to file */
int
save_mli(char *fname,int type)
	{
	FILE *stream;
	int error;
	
	libtype=type;
	
	if((stream=GOpen(fname,"wb"))==NULL)
		{
		cant_create();
		return(0);
		}
	
	error=0;
	
	if(dump_mtlchunk(MLIBMAGIC,stream,NULL)==0)
		error=1;
	
	if(GClose(stream))
		error=1;
	
	if(error) {
		write_err();
		remove(fname);
		}
	
	if(debug)
		{
		getchar();
		redraw(0,0,320,200);
		}
	
	if(error)
		return(0);
	return(1);
	}

#endif  //LATER

/* Main routine for writing a file! */
/* Recursive chunk writer -- keeps track of each chunk */

#define DMP_MATCHUNK(id,data)  { if(dump_mtlchunk((ushort)id,stream,data)==0) return(0);}

#define DMP_IFNOT_NULL(id,ptr)  if (ptr) DMP_MATCHUNK(id,&ptr)


isSXPname(char *name) {
	char ext[5];
	strncpy(ext, name+strlen(name)-4, 5);
	strupr(ext);
	return(strcmp(".SXP",ext)==0);
	}


int dmp_map_params(FILE *stream,MapParams *mp) {
	DMP_MATCHUNK(MAT_MAP_TILING,&mp->texflags);
	DMP_MATCHUNK(MAT_MAP_TEXBLUR,&mp->texblur);
	if (mp->uscale!=1.0)
		DMP_MATCHUNK(MAT_MAP_USCALE,&mp->uscale);
	if (mp->vscale!=1.0)
		DMP_MATCHUNK(MAT_MAP_VSCALE,&mp->vscale);
	if (mp->uoffset!=0.0)
		DMP_MATCHUNK(MAT_MAP_UOFFSET,&mp->uoffset);
	if (mp->voffset!=0.0)
		DMP_MATCHUNK(MAT_MAP_VOFFSET,&mp->voffset);
	if (mp->ang_sin!=0.0) {
		DMP_MATCHUNK(MAT_MAP_ANG,mp);
		}
	if (mp->col1.r!=0||mp->col1.g!=0||mp->col1.b!=0) 
		DMP_MATCHUNK(MAT_MAP_COL1,&mp->col1);
	if (mp->col2.r!=255||mp->col2.g!=255||mp->col2.b!=255) 
		DMP_MATCHUNK(MAT_MAP_COL2,&mp->col2);
	if (mp->rcol.r!=255||mp->rcol.g!=0  ||mp->rcol.b!=0  ) 
		DMP_MATCHUNK(MAT_MAP_RCOL,&mp->rcol);
	if (mp->gcol.r!=0  ||mp->gcol.g!=255||mp->gcol.b!=0  ) 
		DMP_MATCHUNK(MAT_MAP_GCOL,&mp->gcol);
	if (mp->bcol.r!=0  ||mp->bcol.g!=0  ||mp->bcol.b!=255) 
		DMP_MATCHUNK(MAT_MAP_BCOL,&mp->bcol);
	return(1);
	}

int dmp_map(FILE *stream, int i) {
	Mapping *m = savemtl->map[i];
	if (i==Nbump) {
		int t = m->amt.pct/4;      /* for old versions */
		DMP_MATCHUNK(INT_PERCENTAGE,&t);
		DMP_MATCHUNK(MAT_MAPNAME,m->map.name);
		DMP_MATCHUNK(MAT_BUMP_PERCENT,&m->amt.pct); /* new version of bump percent*/
		}
	else {
		DMP_MATCHUNK(INT_PERCENTAGE,&m->amt.pct);
		DMP_MATCHUNK(MAT_MAPNAME,m->map.name);
		}
	if (m->map.kind==0) 
		return(dmp_map_params(stream,&m->map.p.tex));
	else 
		return(1);
	}


int dmp_mask(FILE *stream, int i) {
	Mapping *m = savemtl->map[i];
	DMP_MATCHUNK(MAT_MAPNAME,m->mask.name);
	if (m->mask.kind==0) 
		return(dmp_map_params(stream,&m->mask.p.tex));
	else 
		return(1);
	}

int isSXPMap(MapData *md) {
	return(md->kind==0&&md->p.tex.sxp_data!=NULL);
	} 

int
dump_mtlchunk(ushort tag,FILE *stream,void *data)
	{
	long chunkptr,chunkbase,curpos,chunksize;
	Color_f *cf;
	Color_24 *c24;
	char *string;
	Mmtllist *ml;
//	Mliblist *mlb;
	
	chunkbase=ftell(stream);
	WRTERR(&tag,2);
	chunkptr=ftell(stream);  /* Save file ptr for chunk size */
	WRTERR(&chunkptr,4);
	switch(tag)
		{
		case MLIBMAGIC:
			switch(libtype)
				{
				case 0:
//					if(mlib)
//						{
//						mlb=mlib;
//						while(mlb)
//							{
//							savemtl= &mlb->material;
//							if(dump_mtlchunk(MAT_ENTRY,stream,NULL)==0)
//								return(0);
//							mlb=mlb->next;
//							}
//						}
					break;
				case 1:
					if(mmtl)
						{
//						tag_used_mmtls(); // for our purposes, they're ALL used
						ml=mmtl;
						while(ml)
							{
//							if(ml->flags & MMTLUSED)
								{
								savemtl= &ml->material;
								if(dump_mtlchunk(MAT_ENTRY,stream,NULL)==0)
									return(0);
								}
							ml=ml->next;
							}
						}
					break;
				}
			break;
		case MAT_ENTRY:
		case MATMAGIC:
			{
			if(dump_mtlchunk(MAT_NAME,stream,savemtl->name)==0) return(0);
			if(dump_mtlchunk(MAT_AMBIENT,stream,&savemtl->amb)==0) return(0);
			if(dump_mtlchunk(MAT_DIFFUSE,stream,&savemtl->diff)==0) return(0);
			if(dump_mtlchunk(MAT_SPECULAR,stream,&savemtl->spec)==0)return(0);
			if(dump_mtlchunk(MAT_SHININESS,stream,NULL)==0)	return(0);
			if(dump_mtlchunk(MAT_SHIN2PCT,stream,NULL)==0) return(0);
			if(dump_mtlchunk(MAT_TRANSPARENCY,stream,NULL)==0)	return(0);
			if(dump_mtlchunk(MAT_XPFALL,stream,NULL)==0)	return(0);
			if(dump_mtlchunk(MAT_REFBLUR,stream,NULL)==0) return(0);
			if(dump_mtlchunk(MAT_SHADING,stream,NULL)==0) return(0);
			if(dump_mtlchunk(MAT_SELF_ILPCT,stream,NULL)==0) return(0);
	
			if(savemtl->use&MATUSE_XPFALL)  {
				if(dump_mtlchunk(MAT_USE_XPFALL,stream,NULL)==0)
					return(0);
				}
			if(savemtl->use&MATUSE_REFBLUR)  {
				if(dump_mtlchunk(MAT_USE_REFBLUR,stream,NULL)==0)
					return(0);
				}
			if(savemtl->flags & MF_TWOSIDE)  {
				if(dump_mtlchunk(MAT_TWO_SIDE,stream,NULL)==0)
					return(0);
				}
			if(savemtl->flags & MF_ADDITIVE) {
				if(dump_mtlchunk(MAT_ADDITIVE,stream,NULL)==0)
					return(0);
				}
			if(savemtl->flags & MF_WIRE) {
				if(dump_mtlchunk(MAT_WIRE,stream,NULL)==0)
					return(0);
				}
			if(savemtl->flags & MF_FACEMAP) {
				if(dump_mtlchunk(MAT_FACEMAP,stream,NULL)==0)
					return(0);
				}

			if(savemtl->flags & MF_XPFALLIN) {
				if(dump_mtlchunk(MAT_XPFALLIN,stream,NULL)==0)
					return(0);
				}

			if(savemtl->flags & MF_PHONGSOFT) {
				if(dump_mtlchunk(MAT_PHONGSOFT,stream,NULL)==0)
					return(0);
				}

			if(savemtl->flags & MF_WIREABS) {
				if(dump_mtlchunk(MAT_WIREABS,stream,NULL)==0)
					return(0);
				}

			if(dump_mtlchunk(MAT_WIRESIZE,stream,&savemtl->wiresize)==0)
					return(0);

			/* Save out any texture maps, masks, sxp's */
			for (int k=0; k<NMAPTYPES; k++) {
				Mapping *mp = savemtl->map[k];
				if (mp==NULL) continue;
				if (mp->use) {
					DMP_MATCHUNK(map_chunks[k],NULL);
					if (k!=Nrefl) {
						if (isSXPMap(&mp->map)) 
							DMP_MATCHUNK(map_sxp_chunks[k],mp->map.p.tex.sxp_data);
						}
					if (mp->mask.name[0]!=0) {
						DMP_MATCHUNK(mask_chunks[k],NULL);
						if (isSXPMap(&mp->mask)) 
							DMP_MATCHUNK(mask_sxp_chunks[k],mp->mask.p.tex.sxp_data);
						}
					}
				}
			/* dump auto-cubic chunk */
				{
				Mapping *rm = savemtl->map[Nrefl];
				if (rm&&rm->use&&rm->map.p.ref.acb.flags & AC_ON) DMP_MATCHUNK(MAT_ACUBIC,&rm->map.p.ref.acb);
				}
			if (savemtl->appdata) 
				if(dump_mtlchunk(APP_DATA,stream,savemtl->appdata)==0)
						return(0);
			}				
			break;
	
		case APP_DATA:
			{
			ULONG *plong = (ULONG *)data;
			WRTERR(&plong[1],plong[0]);
			}
			break;
		case MAT_AMBIENT:
		case MAT_DIFFUSE:
		case MAT_SPECULAR:
			if (gammaMgr.enable) {
				Color_24 gc;
				c24 = (Color_24 *)data;
				gc.r = gammaMgr.file_in_gamtab[c24->r];
				gc.g = gammaMgr.file_in_gamtab[c24->g];
				gc.b = gammaMgr.file_in_gamtab[c24->b];
				if(dump_mtlchunk(COLOR_24,stream,&gc)==0)	return(0);
				if(dump_mtlchunk(LIN_COLOR_24,stream,data)==0)return(0);
				}
			else {
				if(dump_mtlchunk(COLOR_24,stream,data)==0) return(0);
				}
			break;
		case MAT_SHININESS:
			if(dump_mtlchunk(INT_PERCENTAGE,stream,&savemtl->shininess)==0)
				return(0);
			break;
		case MAT_SHIN2PCT:
			if(dump_mtlchunk(INT_PERCENTAGE,stream,&savemtl->shin2pct)==0)
				return(0);
			break;
		case MAT_SHIN3PCT:
			if(dump_mtlchunk(INT_PERCENTAGE,stream,&savemtl->shin3pct)==0)
				return(0);
			break;
		case MAT_TRANSPARENCY:
			if(dump_mtlchunk(INT_PERCENTAGE,stream,&savemtl->transparency)==0)
				return(0);
			break;
		case MAT_XPFALL:
			if(dump_mtlchunk(INT_PERCENTAGE,stream,&savemtl->xpfall)==0)
				return(0);
			break;
		case MAT_REFBLUR:
			if(dump_mtlchunk(INT_PERCENTAGE,stream,&savemtl->refblur)==0)
				return(0);
			break;
		case MAT_SELF_ILPCT:
			if(dump_mtlchunk(INT_PERCENTAGE,stream,&savemtl->selfipct)==0)
				return(0);
			break;
		case MAT_SHADING:
			WRTERR(&savemtl->shading,2);
			break;
	
		case MAT_TEXMAP:   if (!dmp_map(stream,Ntex)) return(0); break;
		case MAT_TEX2MAP:  if (!dmp_map(stream,Ntex2)) return(0); break;
		case MAT_OPACMAP:  if (!dmp_map(stream,Nopac)) return(0); break;
		case MAT_BUMPMAP:  if (!dmp_map(stream,Nbump)) return(0); break;
		case MAT_SPECMAP:  if (!dmp_map(stream,Nspec)) return(0); break;
		case MAT_SHINMAP:  if (!dmp_map(stream,Nshin)) return(0); break;
		case MAT_SELFIMAP: if (!dmp_map(stream,Nselfi)) return(0); break;
		case MAT_REFLMAP:  if (!dmp_map(stream,Nrefl)) return(0); break;
	
		case MAT_TEXMASK:  if (!dmp_mask(stream,Ntex)) return(0); break;
		case MAT_TEX2MASK: if (!dmp_mask(stream,Ntex2)) return(0); break;
		case MAT_OPACMASK: if (!dmp_mask(stream,Nopac)) return(0); break;
		case MAT_BUMPMASK: if (!dmp_mask(stream,Nbump)) return(0); break;
		case MAT_SPECMASK: if (!dmp_mask(stream,Nspec)) return(0); break;
		case MAT_SHINMASK: if (!dmp_mask(stream,Nshin)) return(0); break;
		case MAT_SELFIMASK:if (!dmp_mask(stream,Nselfi)) return(0); break;
		case MAT_REFLMASK: if (!dmp_mask(stream,Nrefl)) return(0); break;
	
		case MAT_MAP_TILING: 
			WRTERR(data,2);
			break;
		case MAT_MAP_TEXBLUR: 
		case MAT_MAP_USCALE: 
		case MAT_MAP_VSCALE: 
		case MAT_MAP_UOFFSET: 
		case MAT_MAP_VOFFSET: 
			WRTERR(data,4);
			break;
		case MAT_MAP_COL1:
		case MAT_MAP_COL2:
		case MAT_MAP_RCOL:
		case MAT_MAP_GCOL:
		case MAT_MAP_BCOL:
			c24=(Color_24 *)data;
			WRTERR(c24,3);
			break;
		case MAT_MAP_ANG:
 			{
			MapParams *mp = (MapParams *)data;
			float ang,dang;
 			ang = (float)atan2(mp->ang_sin,mp->ang_cos);
			dang = RadToDeg(ang);
#if 0
			printf("Saving MAT_MAP_ANG sin = %.4f , cos = %.4f, ang = %.4f \n",
				mp->ang_sin, mp->ang_cos, ang);
#endif
			WRTERR(&dang,4);
			}
			break;
	
		case COLOR_F:
			cf=(Color_f *)data;
			WRTERR(cf,12);
			break;
		case COLOR_24:
			c24=(Color_24 *)data;
			WRTERR(c24,3);
			break;
		case LIN_COLOR_24:
			c24 = (Color_24 *)data;
			WRTERR(c24,3);
			break;
		case MAT_NAME:  /* Simple strings */
		case MAT_MAPNAME:
			string=(char *)data;
			WRTERR(string,strlen(string)+1);
			break;
		case MAT_BUMP_PERCENT:
		case INT_PERCENTAGE:
			WRTERR(data,2);
			break;
		case MAT_WIRESIZE:
			WRTERR(data,4);
			break;
		case MAT_TWO_SIDE:
		case MAT_SUPERSMP:
		case MAT_ADDITIVE:
		case MAT_WIRE:
		case MAT_FACEMAP:
		case MAT_XPFALLIN:
		case MAT_USE_XPFALL:
		case MAT_USE_REFBLUR:
		case MAT_PHONGSOFT:
		case MAT_WIREABS:
		case DUMMY:
			break;
		case MAT_ACUBIC: {
				AutoCubicParams *ac = (AutoCubicParams *)data;
				WRTERR(&ac->shade,1);
				WRTERR(&ac->aalevel,1);
				WRTERR(&ac->flags,2);
				WRTERR(&ac->size,4);
				WRTERR(&ac->nth,4);
				}
			break;
		case MAT_SXP_TEXT_DATA:
		case MAT_SXP_TEXT2_DATA:
		case MAT_SXP_OPAC_DATA:
		case MAT_SXP_BUMP_DATA:
		case MAT_SXP_SPEC_DATA:
		case MAT_SXP_SHIN_DATA:
		case MAT_SXP_SELFI_DATA:

		case MAT_SXP_TEXT_MASKDATA:
		case MAT_SXP_TEXT2_MASKDATA:
		case MAT_SXP_OPAC_MASKDATA:
		case MAT_SXP_BUMP_MASKDATA:
		case MAT_SXP_SPEC_MASKDATA:
		case MAT_SXP_SHIN_MASKDATA:
		case MAT_SXP_SELFI_MASKDATA:
		case MAT_SXP_REFL_MASKDATA:
			{
			ULONG *plong = (ULONG *)data;
			if (plong!=NULL)
				WRTERR(&plong[1],plong[0]);
			}
			break;
		}
	
	/* Save file ptr */
	
	curpos=ftell(stream);
	
	/* Point back to chunk size location */
	
	fseek(stream,chunkptr,SEEK_SET);
	
	/* Calc & write chunk size */
	
	chunksize=curpos-chunkbase;
	WRTERR(&chunksize,4);
	
	/* Point back to file end */
	
	fseek(stream,curpos,SEEK_SET);
	return(1);
	}

