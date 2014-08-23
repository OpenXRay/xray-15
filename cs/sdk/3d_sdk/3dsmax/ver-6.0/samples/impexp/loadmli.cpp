/*******************************************************************
 *
 *    DESCRIPTION: LOADMLI.CPP: 3dsr4 material loader
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

#include "dummy.h"
#define xxDBGLDMLI

int dbgldmli=0;

SMtl *loadmtl,inmtl;

extern UWORD file_in_degamtab[];


static void MtlError() {
	// handy place to put a breakpoint
	}

int put_lib_mtl(SMtl *loadmtl) {
	return 1;
	}


//===========================================================
int load_app_data(FILE *stream,void **pdata, int size) {
	ULONG *plong;
	if (*pdata!=NULL) 
		XMFreeAndZero(pdata);
	*pdata = (void *)XMAlloc(size+4);
	if (*pdata==NULL) {
		MtlError();
		return(0);	
		}		
	plong = (ULONG *)(*pdata);
	plong[0] = size;
	RDERR((void *)&plong[1],size);
	return(1);
	}

extern int bad_reason,antiquated,lastchunk;
int just_name=0,strlimit;

/* Percentage work areas */

short ipct;
float fpct;

/* Incoming color work areas */

Color_24 C24,LC24;
Color_f Cf,LCf;

/* Error codes */

#define INVALID_FILE 1
#define PARTIAL_READ 2
#define EXCEEDS_SETUP 3


int
get_mapdata_chunk(FILE *stream, int tag, MapData *md) {
	switch (tag) {
		case MAT_MAPNAME:
			strlimit=13;
			if (!get_mtlchunk(stream,md->name)) {
				return(0);
				}
#ifdef DBGLDMLI
		if (dbgldmli)
				printf("  get_map : mapname: %s \n",md->name);
#endif
			return(1);
		case MAT_MAP_TILING: 
			return(get_mtlchunk(stream,&md->p.tex.texflags));
		case MAT_MAP_TILINGOLD: 
			if (!get_mtlchunk(stream,&md->p.tex.texflags)) {
				return(0);
				}
			if (md->p.tex.texflags&TEX_DECAL)
				md->p.tex.texflags|=TEX_NOWRAP;				
			return(1);
		case MAT_MAP_TEXBLUR: 
			return(get_mtlchunk(stream,&md->p.tex.texblur));
		case MAT_MAP_TEXBLUR_OLD: 
			return(get_mtlchunk(stream,&md->p.tex.texblur));
		case MAT_MAP_USCALE: 
			return(get_mtlchunk(stream,&md->p.tex.uscale));
		case MAT_MAP_VSCALE: 
			return(get_mtlchunk(stream,&md->p.tex.vscale));
		case MAT_MAP_UOFFSET: 
			return(get_mtlchunk(stream,&md->p.tex.uoffset));
		case MAT_MAP_VOFFSET: 
			return(get_mtlchunk(stream,&md->p.tex.voffset));
		case MAT_MAP_ANG: 
			return(get_mtlchunk(stream,&md->p.tex));
		case MAT_MAP_COL1: 
			return(get_mtlchunk(stream,&md->p.tex.col1));
		case MAT_MAP_COL2: 
			return(get_mtlchunk(stream,&md->p.tex.col2));
		case MAT_MAP_RCOL: 
			return(get_mtlchunk(stream,&md->p.tex.rcol));
		case MAT_MAP_GCOL: 
			return(get_mtlchunk(stream,&md->p.tex.gcol));
		case MAT_MAP_BCOL: 
			return(get_mtlchunk(stream,&md->p.tex.bcol));
		default:
			return(skip_chunk(stream));
		}
	}

int get_map(FILE *stream, int n, int size) {
	uint nexttag;
	Chunk_hdr nxt;
	Mapping *m = loadmtl->map[n];
#ifdef DBGLDMLI
	if (dbgldmli)
		printf(" GET_MAP: n = %d\n",n);
#endif
	if (m==NULL) {
		m = NewMapping(n,0);
		if (m==NULL) {
			MtlError();
			return(0);
			}
		loadmtl->map[n] = m;
		}
	m->use = 1;
	while(size) {
		if(get_next_chunk(stream,&nxt)==0)
			return(0);
		nexttag = nxt.tag;
		switch(nexttag) {
			case INT_PERCENTAGE:
			case FLOAT_PERCENTAGE:
				if(get_mtlchunk(stream,NULL)==0)
					return(0);
				m->amt.pct = ipct; 
				if (n==Nbump) 
					m->amt.pct = (ipct<=20)?4*ipct:100;  /* for old files */
			
#ifdef DBGLDMLI
				if (dbgldmli)
					printf("  get_map %d : pct: %d \n",n,ipct);
#endif
				break;
			case MAT_BUMP_PERCENT:
				if(get_mtlchunk(stream,NULL)==0)
					return(0);
				m->amt.pct = ipct; /* for version 3+ files */
				break;
			default:
				if (!get_mapdata_chunk(stream,nexttag,&m->map)) {
#ifdef DBGLDMLI
					if (dbgldmli)
						printf(" Error in get_mapdata_chunk, nexttag = %X\n",nexttag);
#endif
					MtlError();
					return(0);
					}
				break;
			}
		size-=nxt.size;
		}

#ifdef DBGLDMLI
	if (dbgldmli)
		printf(" EXIT GET_MAP: n = %d\n",n);
#endif
	return(1);
	}

int get_mask(FILE *stream, int n, int size) {
	uint nexttag;
	Chunk_hdr nxt;
	Mapping *m = loadmtl->map[n];
	if (m==NULL) {
		m = NewMapping(n,0);
		if (m==NULL) {
			MtlError();
			return(0);
			}
		loadmtl->map[n] = m;
		}
	m->use = 1;
	while(size) {
		if(get_next_chunk(stream,&nxt)==0)
			return(0);
		nexttag = nxt.tag;
#ifdef DBGLDMLI
		if (dbgldmli)
			printf("  get_MASK loop: nexttag = %X \n",nexttag);
#endif
		if (!get_mapdata_chunk(stream,nexttag,&m->mask)) return(0);

		size-=nxt.size;
		}
	return(1);
	}

int get_sxp_data(FILE *stream, int n, int size, int isMask) {
	ULONG **pp,*ptr;
	Mapping *m = loadmtl->map[n];
#ifdef DBGLDMLI
	if (dbgldmli)
		printf("GET_SXP_DATA: n=%d, size = %d, isMask = %d, m=%X\n",
			n,size,isMask,m);
#endif
	if (size==0) return(1);
	if (m==NULL) {
		m = NewMapping(n,0);
		if (m==NULL) {
			MtlError();
			return(0);
			}
		loadmtl->map[n] = m;
		}
	if (isMask)
		pp = (ULONG **)&m->mask.p.tex.sxp_data;
	else 
		pp = (ULONG **)&m->map.p.tex.sxp_data;
	if (*pp!=NULL) XMFreeAndZero((void **)pp);
	ptr = (ULONG *)XMAlloc(size+4);
	if (ptr==NULL)  {
		MtlError();
		return(0);   
		}
	ptr[0] = size;

#ifdef DBGLDMLI
	if (dbgldmli)
		printf("  sxp_data: addr = %X,  size = %X\n", ptr, ptr[0]);
#endif
	RDERR(&ptr[1],size);
	*pp = ptr;
	return(1);
	}       

#define GET_SXP(num,type) if (!get_sxp_data(stream,num,chunk.size,type)) return(0); chunk.size=0;      
#define GET_MAP(num) if (!get_map(stream,num,chunk.size)) return(0); chunk.size=0;      
#define GET_MASK(num) if (!get_mask(stream,num,chunk.size)) return(0); chunk.size=0;      

int
get_mtlchunk(FILE *stream,void *data)
	{
	/* Grab a chunk from the file and process all its subsets */
	Chunk_hdr chunk;
	Chunk_hdr nxt;
	uint thistag,nexttag;
	char *string;
	short *pshort;
	
	RDERR(&chunk,6);
	
	thistag=chunk.tag;
	
#ifdef DBGLDMLI
	if (dbgldmli)
		printf("          get_mtlchunk: tag=%X, size=%d \n",thistag,chunk.size);
#endif
	
	/* Update chunk size to account for header */
	
	chunk.size-=6L;
	
	/* Find chunk type and go process it */
	
	switch(thistag)
		{
		case MLIBMAGIC:
			while(chunk.size)
				{
				if(get_next_chunk(stream,&nxt)==0)
					return(0);
				nexttag=nxt.tag;
				switch(nexttag)
					{
					case MAT_ENTRY:
						loadmtl= &inmtl;
						/* Zero out data structure first */
						init_mtl_struct(loadmtl);
	
						if(get_mtlchunk(stream,NULL)==0)
							return(0);
						if(put_lib_mtl(loadmtl)==0)
							return(0);
						break;     
					default:
						if(skip_chunk(stream)==0){
							MtlError();
							return(0);
							}
						break;
					}
				chunk.size-=nxt.size;
				}
			break;
		case MAT_ENTRY:
		case MATMAGIC:
			while(chunk.size)
				{
				if(get_next_chunk(stream,&nxt)==0)
					return(0);
				nexttag=nxt.tag;
				switch(nexttag)
					{
					case MAT_NAME:
						strlimit=17;
						if(get_mtlchunk(stream,loadmtl->name)==0)
							return(0);
#ifdef DBGLDMLI
						if (dbgldmli)
							printf(" **** Loading material : %s \n", loadmtl->name);
#endif
						if(just_name) /* If all we need is the name, return */
							return(1);
						break;     
					case MAT_AMBIENT:
						if(get_mtlchunk(stream,&loadmtl->amb)==0)
							return(0);
						break;     
					case MAT_DIFFUSE:
						if(get_mtlchunk(stream,&loadmtl->diff)==0)
							return(0);
						break;     
					case MAT_SPECULAR:
						if(get_mtlchunk(stream,&loadmtl->spec)==0)
							return(0);
						break;     
					case MAT_ACUBIC:
						{
						Mapping *m = loadmtl->map[Nrefl];
						if (m==NULL) goto	skip_mtl_chunk;
						if (get_mtlchunk(stream,&m->map.p.ref.acb)==0)
							return(0);
						}
						break;     
	
					case MAT_SXP_TEXT_DATA:       
					case MAT_SXP_TEXT2_DATA: 
					case MAT_SXP_OPAC_DATA:  
					case MAT_SXP_BUMP_DATA:  
					case MAT_SXP_SPEC_DATA:  
					case MAT_SXP_SELFI_DATA: 
					case MAT_SXP_SHIN_DATA:  
				
					case MAT_SXP_TEXT_MASKDATA:  
					case MAT_SXP_TEXT2_MASKDATA: 
					case MAT_SXP_OPAC_MASKDATA:  
					case MAT_SXP_BUMP_MASKDATA:  
					case MAT_SXP_SPEC_MASKDATA:  
					case MAT_SXP_SELFI_MASKDATA: 
					case MAT_SXP_SHIN_MASKDATA:  
					case MAT_SXP_REFL_MASKDATA:  
						if(get_mtlchunk(stream,NULL)==0)
							return(0);
						break;
				
					case MAT_TEXMAP:   
					case MAT_TEX2MAP:  
					case MAT_OPACMAP:  
					case MAT_BUMPMAP:  
					case MAT_SPECMAP:  
					case MAT_SHINMAP:  
					case MAT_SELFIMAP: 
					case MAT_REFLMAP:  
	
					case MAT_TEXMASK:   
					case MAT_TEX2MASK:  
					case MAT_OPACMASK:  
					case MAT_BUMPMASK:  
					case MAT_SPECMASK:  
					case MAT_SHINMASK:  
					case MAT_SELFIMASK: 
					case MAT_REFLMASK:  
						if(get_mtlchunk(stream,NULL)==0)
							return(0);
						break;
	
					case MAT_SHININESS:
					case MAT_SHIN2PCT:
					case MAT_TRANSPARENCY:
					case MAT_XPFALL:
					case MAT_REFBLUR:
					case MAT_SELF_ILPCT:
					case MAT_SHADING:
					case MAT_TWO_SIDE:
					case MAT_SUPERSMP:
					case MAT_SELF_ILLUM:
					case MAT_DECAL:
					case MAT_ADDITIVE:
					case MAT_WIRE:
					case MAT_FACEMAP:
					case MAT_XPFALLIN:
					case MAT_PHONGSOFT:
					case MAT_WIREABS:
					case MAT_USE_XPFALL:
					case MAT_USE_REFBLUR:
					case MAT_WIRESIZE:
						if(get_mtlchunk(stream,NULL)==0)
							return(0);
						break;
					case APP_DATA:
						if(get_mtlchunk(stream,&loadmtl->appdata)==0)
							return(0);
						break;
					default:
						skip_mtl_chunk:
						if(skip_chunk(stream)==0) {
							MtlError();
							return(0);
							}
						break;
					}
				chunk.size-=nxt.size;
				}
						
#ifdef DBGLDMLI
			if (dbgldmli)
					printf("  finished loading mtl %s, flags = %X\n",
						loadmtl->name, loadmtl->flags);
#endif
			/* convert old data formats to new */
			if (loadmtl->shading==REND_WIRE) {
				loadmtl->shading = REND_FLAT;
				loadmtl->flags |= MF_WIRE;
				loadmtl->flags |= MF_TWOSIDE;
				loadmtl->shininess=0;
				loadmtl->shin2pct=0;
				loadmtl->transparency=0;
				}

			if (loadmtl->xpfall<0.0) {
				loadmtl->flags|= MF_XPFALLIN;
				loadmtl->xpfall = -loadmtl->xpfall;
				}
			if (loadmtl->flags&MF_DECAL) {
				set_mtl_decal(loadmtl);
				loadmtl->flags &= ~MF_DECAL;
				}
			if (loadmtl->shin2pct==255) {
				float shin = (((float)(loadmtl->shininess))/100.0f);
				float atten = (float)sin(1.5707*shin);
				loadmtl->shin2pct = (int)((atten)*100.0f+0.5f);
				}

			break;

		case MAT_SXP_TEXT_DATA:  GET_SXP(Ntex,0); break;     
		case MAT_SXP_TEXT2_DATA: GET_SXP(Ntex2,0);  break;     
		case MAT_SXP_OPAC_DATA:  GET_SXP(Nopac,0);  break;
		case MAT_SXP_BUMP_DATA:  GET_SXP(Nbump,0);  break; 
		case MAT_SXP_SPEC_DATA:  GET_SXP(Nspec,0);  break;
		case MAT_SXP_SELFI_DATA: GET_SXP(Nselfi,0);  break;
		case MAT_SXP_SHIN_DATA:  GET_SXP(Nshin,0);  break;
	
		case MAT_SXP_TEXT_MASKDATA:  GET_SXP(Ntex,1);  break;     
		case MAT_SXP_TEXT2_MASKDATA: GET_SXP(Ntex2,1);  break;     
		case MAT_SXP_OPAC_MASKDATA:  GET_SXP(Nopac,1);  break;
		case MAT_SXP_BUMP_MASKDATA:  GET_SXP(Nbump,1);  break; 
		case MAT_SXP_SPEC_MASKDATA:  GET_SXP(Nspec,1);  break;
		case MAT_SXP_SELFI_MASKDATA: GET_SXP(Nselfi,1);  break;
		case MAT_SXP_SHIN_MASKDATA:  GET_SXP(Nshin,1);  break;
		case MAT_SXP_REFL_MASKDATA:  GET_SXP(Nrefl,1);  break;

		case MAT_TEXMAP:   GET_MAP(Ntex);  break;     
		case MAT_TEX2MAP:  GET_MAP(Ntex2);  break;     
		case MAT_OPACMAP:  GET_MAP(Nopac);  break;     
		case MAT_BUMPMAP:  GET_MAP(Nbump);  break;     
		case MAT_SPECMAP:  GET_MAP(Nspec);  break;     
		case MAT_SHINMAP:  GET_MAP(Nshin);  break;     
		case MAT_SELFIMAP: GET_MAP(Nselfi);  break;     
		case MAT_REFLMAP:  GET_MAP(Nrefl);  break;     

		case MAT_TEXMASK:   GET_MASK(Ntex);  break;     
		case MAT_TEX2MASK:  GET_MASK(Ntex2);  break;     
		case MAT_OPACMASK:  GET_MASK(Nopac);  break;     
		case MAT_BUMPMASK:  GET_MASK(Nbump);  break;     
		case MAT_SPECMASK:  GET_MASK(Nspec);  break;     
		case MAT_SHINMASK:  GET_MASK(Nshin);  break;     
		case MAT_SELFIMASK: GET_MASK(Nselfi);  break;     
		case MAT_REFLMASK:  GET_MASK(Nrefl);  break;     

		case MAT_AMBIENT:
		case MAT_DIFFUSE:
		case MAT_SPECULAR:
			{
			int got_lin,got_gam;
			got_lin = got_gam = 0;
			while(chunk.size)
				{
				if(get_next_chunk(stream,&nxt)==0)
					return(0);
				nexttag=nxt.tag;
				switch(nexttag)	{
					case COLOR_F:
					case COLOR_24:
						got_gam = 1;
						if(get_mtlchunk(stream,NULL)==0) return(0);
						break;     
					case LIN_COLOR_24:
						got_lin = 1;
						if(get_mtlchunk(stream,NULL)==0)	return(0);
						break;     
					default:
						if(skip_chunk(stream)==0) {
							MtlError();
							return(0);
							}
						break;
					}
				chunk.size-=nxt.size;
				}
			if (got_lin) {
				memcpy((char *)data,(char *)&LC24,sizeof(Color_24));
				}
			else { 
				if (!got_gam) {
					MtlError();
					return(0);
					}
				if (gammaMgr.enable) {
					Color_24 gc;
					gc.r = gammaMgr.file_in_degamtab[C24.r]>>8;
					gc.g = gammaMgr.file_in_degamtab[C24.g]>>8;
					gc.b = gammaMgr.file_in_degamtab[C24.b]>>8;
					memcpy((char *)data,(char *)&gc,sizeof(Color_24));
					}
				else { 
					memcpy((char *)data,(char *)&C24,sizeof(Color_24));
					}
				}
			}
			break;
		case MAT_SELF_ILLUM:	
				loadmtl->flags |= MF_SELF;	  
				loadmtl->selfipct = 100;				
				break;
		case MAT_TWO_SIDE:	loadmtl->flags |= MF_TWOSIDE;	break;
		case MAT_SUPERSMP:	loadmtl->flags |= MF_SUPERSMP;	break;
		case MAT_ADDITIVE:	loadmtl->flags |= MF_ADDITIVE;break;
		case MAT_DECAL:   loadmtl->flags |= MF_DECAL;  	break;
		case MAT_WIRE:	loadmtl->flags |= MF_WIRE; break;
		case MAT_FACEMAP:	loadmtl->flags |= MF_FACEMAP; break;
		case MAT_XPFALLIN:	loadmtl->flags |= MF_XPFALLIN; break;
		case MAT_PHONGSOFT:	loadmtl->flags |= MF_PHONGSOFT; break;
		case MAT_WIREABS:	loadmtl->flags |= MF_WIREABS; break;
		case MAT_USE_XPFALL:	loadmtl->use |= MATUSE_XPFALL; break;
		case MAT_USE_REFBLUR:	loadmtl->use |= MATUSE_REFBLUR;	break;

		case MAT_SHININESS:
			pshort = &loadmtl->shininess;
			goto get_int_float_chunk;
	
		case MAT_SHIN2PCT:
			pshort = &loadmtl->shin2pct;
			goto get_int_float_chunk;
	
		case MAT_TRANSPARENCY:
			pshort = &loadmtl->transparency;
			goto get_int_float_chunk;
	
		case MAT_XPFALL:
			pshort = &loadmtl->xpfall;
			goto get_int_float_chunk;
	
		case MAT_REFBLUR:
			pshort = &loadmtl->refblur;
			goto get_int_float_chunk;
	
		case MAT_SELF_ILPCT:
			pshort = &loadmtl->selfipct;
			goto get_int_float_chunk;
	
			get_int_float_chunk:
			while(chunk.size)
				{
				if(get_next_chunk(stream,&nxt)==0)
					return(0);
				nexttag=nxt.tag;
				switch(nexttag)
					{
					case INT_PERCENTAGE:
					case FLOAT_PERCENTAGE:
						if(get_mtlchunk(stream,NULL)==0)
							return(0);
#ifdef DBGLDMLI
						if (dbgldmli)
							printf(" get_int_float_chunk: val = %d \n",ipct);
#endif 

						*pshort=ipct;
						break;
					default:
						if(skip_chunk(stream)==0) {
							MtlError();
							return(0);
							}
						break;
					}
				chunk.size-=nxt.size;
				}
			break;
		case MAT_SHADING:
			RDERR(&loadmtl->shading,2);
			chunk.size-=2L;
			goto skiprest;
		case MAT_MAP_TILING: 
		case MAT_MAP_TILINGOLD: 
			RDERR(data,2);
			chunk.size-=2L;
			goto skiprest;
		case MAT_MAP_TEXBLUR_OLD: {
			float foo;
			RDERR(&foo,sizeof(float));
			foo = (foo-1.0f)/7.0f;
			if (foo<0.0f) foo = 0.0f;
			if (foo>1.0f) foo = 1.0f;
			memmove(data,&foo,4);
			chunk.size -=4L;
			}
			goto skiprest;
		case MAT_MAP_TEXBLUR: 
		case MAT_MAP_USCALE: 
		case MAT_MAP_VSCALE: 
		case MAT_MAP_UOFFSET: 
		case MAT_MAP_VOFFSET: 
			RDERR(data,sizeof(float));
			chunk.size -=4L;
			goto skiprest;
		case MAT_MAP_ANG: {
			MapParams *mp = (MapParams *)data;
			float ang;
			RDERR(&ang,4);
			ang = DegToRad(ang);
			mp->ang_sin = (float)sin(ang);
			mp->ang_cos = (float)cos(ang);
			chunk.size -=4L;
			goto skiprest;
			}

		case MAT_MAP_COL1: 
		case MAT_MAP_COL2: 
		case MAT_MAP_RCOL: 
		case MAT_MAP_GCOL: 
		case MAT_MAP_BCOL: 
			{
			Color_24 *pcol = (Color_24 *)data;
			RDERR(pcol,3);
        	}
			chunk.size-=3L;
			goto skiprest;
		case COLOR_F:
			RDERR(&Cf,12);
			chunk.size-=12L;
			C24.r=(uchar)(Cf.r*255.0f);
			C24.g=(uchar)(Cf.g*255.0f);
			C24.b=(uchar)(Cf.b*255.0f);
			goto skiprest;
		case COLOR_24:
			RDERR(&C24,3);
			Cf.r=(float)C24.r/255.0f;
			Cf.g=(float)C24.g/255.0f;
			Cf.b=(float)C24.b/255.0f;
			chunk.size-=3L;
			goto skiprest;
		case LIN_COLOR_24:
			RDERR(&LC24,3);
			LCf.r=(float)LC24.r/255.0f;
			LCf.g=(float)LC24.g/255.0f;
			LCf.b=(float)LC24.b/255.0f;
			chunk.size-=3L;
			goto skiprest;
		case MAT_NAME:  /* Simple strings */
		case MAT_MAPNAME:
			string=(char *)data;
			if(read_string(string,stream,strlimit)==0) {
				MtlError();
				return(0);
				}
			chunk.size-=(long)(strlen(string)+1);
			goto skiprest;


		case MAT_ACUBIC: 
			{
			AutoCubicParams *ac = (AutoCubicParams *)data;
			RDERR(&ac->shade,1);
			RDERR(&ac->aalevel,1);
			RDERR(&ac->flags,2);
			RDERR(&ac->size,4);
			RDERR(&ac->nth,4);
			chunk.size-=12;
			}
			goto skiprest;
		case INT_PERCENTAGE:
			RDERR(&ipct,2);
			fpct=(float)ipct/100.0f;
			chunk.size-=2L;
			goto skiprest;
		case MAT_WIRESIZE:
			RDERR(&loadmtl->wiresize,4);
			chunk.size-=4L;
			goto skiprest;
		case MAT_BUMP_PERCENT:
			RDERR(&ipct,2);
			fpct=(float)ipct/100.0f;
			chunk.size-=2L;
			goto skiprest;
		case FLOAT_PERCENTAGE:
			RDERR(&fpct,4);
			ipct=(short)(fpct*100.0f);
			chunk.size-=4;
			goto skiprest;
		case APP_DATA:
//			if (!load_app_data(stream, (void **)data, chunk.size))
//				return(0);
//			chunk.size = 0;
			goto skiprest;
			
	/* The following routine is used to dump any sub-chunks */
	/* in the current chunk.    */
			skiprest:
			while(chunk.size)
				{
				if(get_next_chunk(stream,&nxt)==0)
					return(0);
				if(skip_chunk(stream)==0) {
					MtlError();
					return(0);
					}
				chunk.size-=nxt.size;
				}  
			break;
		}
	//done:
	return(1);
	}

void end_loadmli(){}
