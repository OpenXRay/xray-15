// odb2vrml

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <lwtypes.h>
#include <lwhost.h>
#include <lwmeshes.h>
#include <lwtxtr.h>
#include <lwimage.h>
#include <lwmath.h>

#include "objectdb.h"
#include "sctags.h"
#include "xvrml.h"

extern LWItemInfo			*itemInfo;
extern LWObjectInfo		*objInfo;
extern LWImageList			*imageList;
extern LWTextureFuncs		*texFuncs;
extern LWSurfaceFuncs		*surfFuncs;
extern GlobalFunc			*GlobalGlobal;
extern LWObjectFuncs		*objFuncs;


void newext(char *s, char *x);
void killext(char *s);
int swapchars(char *str,char o, char n);
char *filepart(char *str);
void str_tolower(char *str);
void GetDate(char *datebuf,int siz);

void fixVRMLName(char *str, int len);


char *sep1="Separator { ",*sep2 = "Group { ",*sep;
static char *endline="       \n";
static char *endline15="               \n";



double boundsCenter(void *fukd, LWDVector bbox, LWDVector cent)
{
	ObjectDB	*obj = (ObjectDB*)fukd;
	double		bounds[6] = {0.,0.,0.,0.,0.,0.},rmax = 0.0, r;
	int			i;
	DBPoint		*pt;

	pt = &(obj->pt[0]);
	bounds[0] = bounds[1] = pt->pos[0][0];
	bounds[2] = bounds[3] = pt->pos[0][1];
	bounds[4] = bounds[5] = pt->pos[0][2];
	rmax = VDOT(pt->pos[0],pt->pos[0]);
	for(i=1; i<obj->npoints; i++)
	{
		pt = &(obj->pt[i]);
		if(pt->pos[0][0] > bounds[1])
			bounds[1] = pt->pos[0][0];
		else if(pt->pos[0][0] < bounds[0])
			bounds[0] = pt->pos[0][0];
		if(pt->pos[0][1] > bounds[3])
			bounds[3] = pt->pos[0][1];
		else if(pt->pos[0][1] < bounds[2])
			bounds[2] = pt->pos[0][1];
		if(pt->pos[0][2] > bounds[5])
			bounds[5] = pt->pos[0][2];
		else if(pt->pos[0][2] < bounds[4])
			bounds[4] = pt->pos[0][2];
		r = VDOT(pt->pos[0],pt->pos[0]);
		if(r>rmax)
			rmax = r;
	}
	
	cent[0]=(float) (bounds[0]+bounds[1])/2;
	cent[1]=(float)(bounds[2]+bounds[3])/2;
	cent[2]=(float)RIGHT_HANDZ((bounds[4]+bounds[5])/2);
	bbox[0]=(float)(bounds[1]-bounds[0]);
	bbox[1]=(float)(bounds[3]-bounds[2]);
	bbox[2]=(float)(bounds[5]-bounds[4]);
	return sqrt(rmax);
}


LWTLayerID imgLayer(LWTextureID tex)
{
	LWTLayerID	layr=NULL;
	if(tex)
		for( layr=texFuncs->firstLayer(tex); layr; layr=texFuncs->nextLayer(tex, layr) )
				if(TLT_IMAGE==texFuncs->layerType(layr))
					break;
	return layr;
}



int VRMLPointArrayWrite(VRMLData *dat, void *fukd)
{
	ObjectDB	*odb = (ObjectDB*)fukd;
	int 	i;
	fprintf(dat->vr->file,"\tpoint [\n");
	INDENT(dat->vr);
	for(i=0; i<odb->npoints; i++)
	{
		if(i&1) 
			fprintf(dat->vr->file,"%f %f %f,\n",
					odb->pt[i].pos[0][0],
					odb->pt[i].pos[0][1],
					RIGHT_HANDZ(odb->pt[i].pos[0][2]) );
		else 
			fprintf(dat->vr->file,"%s%f %f %f, ",dat->vr->indent,
					odb->pt[i].pos[0][0],
					odb->pt[i].pos[0][1],
					RIGHT_HANDZ(odb->pt[i].pos[0][2]) );

	}
	UNINDENT(dat->vr);
	fprintf(dat->vr->file,"%s\t]",dat->vr->indent);
	return(i);
}

static float defVertRGB = 0.5f;

static int VRML97_VertexColorsWrite(VRMLData *dat, ObjectDB *odb, DBSurface *surf)
{
	int 	i;
	float	rgba[8];
	double  v,*scl=NULL, *colr;
	char	tag[TAG_SIZE], *vmname = NULL;
	void	*vmid;
	LWID type = LWID_('R','G','B',' ');	//	, typeRGBA = LWID_('R','G','B','A');

	LWMeshInfo	*mesh;

	if(surf)
	{
		vmname = (char*)surfFuncs->getColorVMap(surf->id);
		scl = surfFuncs->getFlt(surf->id, SURF_VCOL);
		colr = surfFuncs->getFlt(surf->id, SURF_COLR);
	}

	if(!vmname)
		if(!getItemTag(itemInfo,odb->id,VTAG_VERTEXRGB"=", tag,TAG_SIZE))
			return 0;
		else
			vmname = strtok(tag," \n");
	if(!vmname)
		return 0;

	mesh = objInfo->meshInfo(odb->id,1);
	if(!mesh)
		return 0;

	if( (vmid = mesh->pntVLookup(mesh, type, vmname)) )
		mesh->pntVSelect(mesh, vmid);
	else
		return 0;

	fprintf(dat->vr->file,"%scolor \tColor { \tcolor [\n", dat->vr->indent);
	INDENT(dat->vr);
	for(i=0; i<odb->npoints; i++)
	{
		VSET(rgba,defVertRGB);
		//VCLR(rgba);
		rgba[3] = 0.0f;//defVertRGB;
		if( mesh->pntVGet(mesh, odb->pt[i].id, rgba) )
		{
			if(colr && scl)
			{
				v = 1.0 - *scl; // scl is alpha blend
				rgba[0] = *scl*rgba[0] + colr[0]*v;
				rgba[1]	= *scl*rgba[1] + colr[1]*v;
				rgba[2] = *scl*rgba[2] + colr[2]*v;
				VSCL(rgba, 0.5);
			} 
			v = VLEN(rgba); // Must normalize color, not CLAMP to keep hue/direction
			if(v>1.7320508080) 
			{
				v = 1.7320508080/v; // 1.732050808root3 -> white at 1,1,1
				VSCL(rgba,((float)v));
			} 
			rgba[0] = CLAMP(rgba[0],0.0f,1.0f);
			rgba[1] = CLAMP(rgba[1],0.0f,1.0f);
			rgba[2] = CLAMP(rgba[2],0.0f,1.0f); 
		}
		else if(colr)
			VCPY(rgba,colr);
		else 
			VSET(rgba,1.0);
		if(i&1) 
			fprintf(dat->vr->file,"%f %f %f,\n", rgba[0], rgba[1], rgba[2] );
		else 
			fprintf(dat->vr->file,"%s%f %f %f, ", dat->vr->indent, rgba[0], rgba[1], rgba[2] );

	}
	UNINDENT(dat->vr);
	fprintf(dat->vr->file,"%s\t] }\n",dat->vr->indent);
	return(i);
}

static int VRML2_PointsWrite(VRMLData *dat, ObjectDB* lwo, char *name)
{
	int	i;
	fprintf(dat->vr->file,"%sShape { geometry  IndexedFaceSet { coord DEF  %s"VDEF_COORDS_DEF" \tCoordinate { ",dat->vr->indent,name);
	i = VRMLPointArrayWrite(dat,lwo);
	fprintf(dat->vr->file," }\n%s} }\n",dat->vr->indent);
	return(i);
}

static int VRML2_NormalsWrite(VRMLData *dat, ObjectDB* lwo, DBSurface* surf)
{
	int i;
	DBPolygon*pol; 

	fprintf(dat->vr->file,"%sNormal {\tvector [\n",dat->vr->indent);
	INDENT(dat->vr);
	for(i=0; i<lwo->npolygons; i++)
	{
		if( (pol=&(lwo->pol[i])) )
			if(&(lwo->surf[pol->sindex])==surf)
			{
				fprintf(dat->vr->file,"%s%f %f %f,\n",dat->vr->indent,
					RIGHT_HANDZ(pol->norm[0][0]),
					RIGHT_HANDZ(pol->norm[0][1]),
					RIGHT_HANDZ(pol->norm[0][2]));
			}
	}
	UNINDENT(dat->vr);
	fprintf(dat->vr->file,"%s] }\n\n",dat->vr->indent);
	return(i);
}



// write 	Appearance  { material []  [ texture url] } 
static void VRML2_AppearanceWrite(VRMLData *dat, ObjectDB* lwo, DBSurface* sur)
{
	LWImageID		img;
	LWTLayerID		lay;
	LWTextureID		tex;

	if(dat->vr->vrprefs&VRPREF_COMMENTS)
		fprintf(dat->vr->file,"%s# Named Surface %s\n",dat->vr->indent,sur->name);
	fprintf(dat->vr->file,"%sappearance Appearance  { \n",dat->vr->indent);
	INDENT(dat->vr);
	if(	sur)
	{
		//		if(img = surfFuncs->getImg(sur->id, SURF_COLR)) 
		img = NULL;
		if(tex = surfFuncs->getTex(sur->id, SURF_COLR)) 
			if(lay = imgLayer(tex))
				texFuncs->getParam(lay,TXTAG_IMAGE,&img);

		if( (sur->diff!=0.0f || sur->lumi!=0.0f || img==NULL )  )// no material makes image 100% luminous??
		{
			fprintf(dat->vr->file,"%smaterial  Material  { ",dat->vr->indent);
			INDENT(dat->vr);
				fprintf(dat->vr->file,"\n%sdiffuseColor   %.4f %.4f %.4f\n",dat->vr->indent,
								sur->diff*sur->colr[0], 
								sur->diff*sur->colr[1], 
								sur->diff*sur->colr[2] );
						//	sur->colr[0], sur->colr[1], sur->colr[2]);

				fprintf(dat->vr->file,"%sspecularColor   %.4f %.4f %.4f\n",dat->vr->indent,
						sur->spec*(sur->colr[0]*sur->clrh + (1.0-sur->clrh)), 
						sur->spec*(sur->colr[1]*sur->clrh + (1.0-sur->clrh)), 
						sur->spec*(sur->colr[2]*sur->clrh + (1.0-sur->clrh)) );

				if(sur->lumi>0.0f) 
					fprintf(dat->vr->file,"\n%semissiveColor   %.4f %.4f %.4f\n",dat->vr->indent,
							sur->lumi*sur->colr[0], 
							sur->lumi*sur->colr[1], 
							sur->lumi*sur->colr[2] );

			//	fprintf(dat->vr->file,"%sshininess      %.4f\n",dat->vr->indent,sur->spec);
				fprintf(dat->vr->file,"%sshininess      %.4f\n",dat->vr->indent,sur->glos);

				if(sur->tran>0.0f) 
					fprintf(dat->vr->file,"%stransparency   %.4f\n",dat->vr->indent,sur->tran);

			UNINDENT(dat->vr);
			fprintf(dat->vr->file,"%s}\n",dat->vr->indent);	 
		}
		else
			fprintf(dat->vr->file,"%s# Material node skipped for unshaded texture!\n",dat->vr->indent);	 


		if(img)
		{
			char *n,imgfile[MAX_STR+1];
			fprintf(dat->vr->file,"%stexture  ImageTexture  {\n",dat->vr->indent);	
			// name = makeURL(srf->su_image);	
			n = (char*)imageList->filename(img,0);
			if(n)
				strncpy(imgfile,n,MAX_STR);
#ifndef _XGL
			swapchars(imgfile,FILE_MARK,'/');
#else
			swapchars(imgfile,'\\','/');
#endif 

			if(dat->vr->vrprefs&VRPREF_LOWERCASE)
				str_tolower(imgfile);

			INDENT(dat->vr);
			if(*dat->texURN)
			{	char *fpi = filepart(imgfile);
				fprintf(dat->vr->file,"%surl [ \"%s\", \"%s%s\", \"%s\" ]\n",dat->vr->indent,fpi,dat->texURN,fpi,imgfile);	
			}
			else
				fprintf(dat->vr->file,"%surl [ \"%s\", \"%s\" ]\n",dat->vr->indent,filepart(imgfile),imgfile);	
			UNINDENT(dat->vr);
			fprintf(dat->vr->file,"%s}\n",dat->vr->indent);	 
		}

	}
	else 
	{
		fprintf(dat->vr->file,"%smaterial  Material  {  # using default color :( \n",dat->vr->indent);
		fprintf(dat->vr->file," %s\tdiffuseColor   0.9 0.9 0.9\n",dat->vr->indent);
		UNINDENT(dat->vr);
		fprintf(dat->vr->file,"%s}\n",dat->vr->indent);
	}
	UNINDENT(dat->vr);
	fprintf(dat->vr->file,"%s}\n",dat->vr->indent);	 
}


// write 	Appearance  { material USE x [ texture url] } for all polys w/ surf
static int VRML2_AppearanceListWrite(VRMLData *dat, ObjectDB* lwo)
{
	int n=0,i;
	DBSurface	*sur;
	LWImageID img;
	LWTLayerID		lay;
	LWTextureID		tex;
	char nbuf[300]="";

	for(i=0; i<lwo->nsurfaces;i++)
	{
		if( (sur=&(lwo->surf[i])) )
		{
			strncpy(nbuf,sur->name,299);
			fixVRMLName(nbuf,299);
			//swapchars(nbuf,' ','_');	  /* VRML Parse HATES spaces in surface names! */
			fprintf(dat->vr->file,"%sDEF %s_Stuff Appearance  { \n",dat->vr->indent,nbuf);
			INDENT(dat->vr);
			if(dat->vr->vrprefs&VRPREF_COMMENTS)
				fprintf(dat->vr->file,"%s# Attributes for surface named %s\n",dat->vr->indent,sur->name);

			n++;

			img = NULL;
			if(tex = surfFuncs->getTex(sur->id, SURF_COLR)) 
				if(lay = imgLayer(tex))
					texFuncs->getParam(lay,TXTAG_IMAGE,&img);

			if( (sur->diff!=0.0f || sur->lumi!=0.0f || img==NULL ) )// no material makes image 100% luminous??
			{
				fprintf(dat->vr->file,"%smaterial  Material  { ",dat->vr->indent);
				INDENT(dat->vr);
					fprintf(dat->vr->file,"\n%sdiffuseColor   %.4f %.4f %.4f\n",dat->vr->indent,
								sur->diff*sur->colr[0], 
								sur->diff*sur->colr[1], 
								sur->diff*sur->colr[2] );
						//	sur->colr[0], sur->colr[1], sur->colr[2]);

					fprintf(dat->vr->file,"%sspecularColor   %.4f %.4f %.4f\n",dat->vr->indent,
							sur->spec*(sur->colr[0]*sur->clrh + (1.0-sur->clrh)), 
							sur->spec*(sur->colr[1]*sur->clrh + (1.0-sur->clrh)), 
							sur->spec*(sur->colr[2]*sur->clrh + (1.0-sur->clrh)) );

					if(sur->lumi>0.0f) 
						fprintf(dat->vr->file,"\n%semissiveColor   %.4f %.4f %.4f\n",dat->vr->indent,
								sur->lumi*sur->colr[0], 
								sur->lumi*sur->colr[1], 
								sur->lumi*sur->colr[2] );

				//	fprintf(dat->vr->file,"%sshininess      %.4f\n",dat->vr->indent,sur->spec);
					fprintf(dat->vr->file,"%sshininess      %.4f\n",dat->vr->indent,sur->glos);

					if(sur->tran>0.0f) 
						fprintf(dat->vr->file,"%stransparency   %.4f\n",dat->vr->indent,sur->tran);

				UNINDENT(dat->vr);
				fprintf(dat->vr->file,"%s}\n",dat->vr->indent);	 
			}
			if(img)
			{
				char imgfile[MAX_STR+1];
				fprintf(dat->vr->file,"%stexture  ImageTexture  {\n",dat->vr->indent);	
				strncpy(imgfile,imageList->filename(img,0),MAX_STR);
#ifndef _XGL
				swapchars(imgfile,FILE_MARK,'/');
#else
				swapchars(imgfile,'\\','/');
#endif 

				if(dat->vr->vrprefs&VRPREF_LOWERCASE)
					str_tolower(imgfile);

				INDENT(dat->vr);
				fprintf(dat->vr->file,"%stexture  ImageTexture  {\n	",dat->vr->indent);	
				fprintf(dat->vr->file,"%s\turl  \"%s\"\n",dat->vr->indent,imgfile);	
				UNINDENT(dat->vr);
			} 

			UNINDENT(dat->vr);
		}
	}
	return(n);
}

static int polPointUV(LWTLayerID lay, DBPolygon *pol, DBPoint *pnt, int vxnum,  double *uv)
{
	LWDVector	tuv = {0.0f,0.0f}, wpos, opos;
	int		proj, waxis, oaxis;

	texFuncs->getParam(lay,TXTAG_PROJ,&proj);
	if(proj==TXPRJ_UVMAP)
	{
	}
	else
	{
		VCPY(opos,(pnt->pos[0]));
		VCPY(wpos,opos);

		texFuncs->evaluateUV(lay,waxis,oaxis,opos,wpos,uv);
	}
	return proj;
}


// write 			TextureCoordinate  { point [] } for all polys w/ surf
static float polyUVs[132];

static int VRML2_TexCoordsWrite(VRMLData *dat, ObjectDB* lwo, DBSurface *surf, LWTextureID tex)
{
	int p,txIndx=0;
	int			 proj,i;
	LWTLayerID		lay;
	DBPolygon	*pol; 
	void		*vmapid;
	LWMeshInfo	*mesh;
	double u=0.0,v=0.0;
	DBPoint		*pnt;
	LWDVector	uv = {0.0f,0.0f},tuv = {0.0f,0.0f}, wpos, opos;

	lay = imgLayer(tex);
	if(!lay)
		return 0;

	texFuncs->getParam(lay,TXTAG_PROJ,&proj);
	mesh = objInfo->meshInfo(lwo->id,1);
	fprintf(dat->vr->file,"%stexCoord  TextureCoordinate {\tpoint [\n",dat->vr->indent);
	INDENT(dat->vr);
	for(i=0; i<lwo->npolygons; i++)
	{
		if( (pol=&(lwo->pol[i])) )
			if(&(lwo->surf[pol->sindex])==surf)
			{
				char	*tag = NULL;
				int		puvs = 0;
				fprintf(dat->vr->file,dat->vr->indent);

				if(proj==TXPRJ_UVMAP)
					texFuncs->getParam(lay,TXTAG_VMAP,&vmapid);

#ifndef CCW_POLYS
				for(p=0; p<pol->nverts; p++)
#else
				for(p=pol->nverts-1; p>=0; p--)
#endif
				{			 
					pnt = &(lwo->pt[pol->v[p].index]);
					if(proj==TXPRJ_UVMAP)
					{
						float fuv[]={0.0f,0.0f,0.0f};
						if(tag && puvs)
						{
							uv[0] = (double)polyUVs[2*p];
							uv[1] = (double)polyUVs[(2*p) + 1];
						}
						else if(mesh && vmapid)
							if(2==mesh->pntVSelect(mesh,vmapid))
								if(mesh->pntVPGet(mesh, pnt->id, pol->id, fuv))
								{
									uv[0] = (double)fuv[0];
									uv[1] = (double)fuv[1];
								}
								else if(mesh->pntVGet(mesh, pnt->id, fuv))
								{
									uv[0] = (double)fuv[0];
									uv[1] = (double)fuv[1];
								}
					}
					else
					{
						VCPY(opos,(pnt->pos[0]));
						VCPY(wpos,opos);
						texFuncs->evaluateUV(lay,0,0,opos,wpos,uv);
					}
					fprintf(dat->vr->file,"%f %f, %c",uv[0],uv[1],endline[p&7]);					
				}	
				fprintf(dat->vr->file,"\n");	
			}
	}
	UNINDENT(dat->vr);
	fprintf(dat->vr->file,"%s]  }\n",dat->vr->indent);
	return(p);
}

static int VRML2_ShapeWrite(VRMLData *dat, ObjectDB* lwo, int srfIdx,char *name)//, int Proto)
{
	int p,n,linz=0,cvex=1,dotz=0,facz=0;
	int	i;
	DBPolygon*	pol;  
	DBSurface *surf;
//	LWSurfaceID	sur=surf->id;
	if(srfIdx<lwo->nsurfaces)
		surf = &(lwo->surf[srfIdx]);
	if(surf)  
		fprintf(dat->vr->file,"%s# Polygons with surface named %s \n",dat->vr->indent,surf->name);
	else return 0;

	for(i=0; i<lwo->npolygons; i++)
	{
		if( pol=&(lwo->pol[i]) )
			if(pol->sindex==srfIdx)
				if(pol->nverts>=3)
					facz++;
				else if(pol->nverts==2)
					linz++;
				else if(pol->nverts==1)
					dotz++;
	}


	if(facz)  // skip the IFS if only lines 
	{
		fprintf(dat->vr->file,"%sShape {\n",dat->vr->indent);
		INDENT(dat->vr);

		fprintf(dat->vr->file,"%sgeometry  IndexedFaceSet {\n",dat->vr->indent);
		INDENT(dat->vr);
	#ifndef CCW_POLYS
		fprintf(dat->vr->file,"%sccw FALSE\n",dat->vr->indent);
	#else
		fprintf(dat->vr->file,"%sccw TRUE\n",dat->vr->indent);
	#endif
		if(dat->vr->vrprefs&VRPREF_PROTO_OBJS)
			fprintf(dat->vr->file,"%scoord Coordinate { point IS point }\n",dat->vr->indent);
		else
			fprintf(dat->vr->file,"%scoord USE %s"VDEF_COORDS_DEF"\n",dat->vr->indent,name);
		fprintf(dat->vr->file,"%scoordIndex [\n",dat->vr->indent);
		INDENT(dat->vr);
		for(i=0; i<lwo->npolygons; i++)
		{
			if( pol=&(lwo->pol[i]) )
				if(pol->sindex==srfIdx)
					if(pol->nverts>=3)
					{
						if(pol->nverts>3) 
							cvex=0;
						fprintf(dat->vr->file,dat->vr->indent);
						n=pol->nverts;
		#ifndef CCW_POLYS
						for(p=0; p<n; p++)
							fprintf(dat->vr->file,"%d,%c",pol->v[p].index,endline15[p&15]); //endline[p&7]);
		#else
						for(p=n; p; p--)
							fprintf(dat->vr->file,"%d,%c",pol->v[p-1].index,endline15[p&15]); //endline[p&7]);
		#endif
						fprintf(dat->vr->file,"%s-1,\n",(p&15) ? " ":dat->vr->indent );	
					}
		}
		UNINDENT(dat->vr);
 		fprintf(dat->vr->file,"%s]\n",dat->vr->indent);
		if(cvex)
			fprintf(dat->vr->file,"%sconvex TRUE\n",dat->vr->indent);
		else
			fprintf(dat->vr->file,"%sconvex FALSE\n",dat->vr->indent);
		if(VRML97_VertexColorsWrite(dat, lwo, surf))
			fprintf(dat->vr->file,"%scolorPerVertex TRUE\n",dat->vr->indent);
		if(surf) 
		{
			LWTextureID		tex;
			if(surf->sman>0.0f) 
				fprintf(dat->vr->file,"%screaseAngle %04f\n",dat->vr->indent,(surf->sman));	// already radians
//			if(img = surfFuncs->getImage(surf->id, SURF_DIFF)) 
			if(tex = surfFuncs->getTex(surf->id, SURF_COLR)) 
			{
				int txIndx = 0;
				// tex coord index first
				fprintf(dat->vr->file,"%stexCoordIndex [\n",dat->vr->indent);
				INDENT(dat->vr);
				for(i=0; i<lwo->npolygons; i++)
				{
					if( pol=&(lwo->pol[i]) )
						if(pol->sindex==srfIdx)
						{
							fprintf(dat->vr->file,dat->vr->indent);
							for(p=0; p<pol->nverts; p++, txIndx++)
								fprintf(dat->vr->file,"%d,%c",txIndx,endline15[p&15]); //endline[p&7]);
							fprintf(dat->vr->file,"%s-1,\n",(p&7) ? " ":dat->vr->indent );	
						}
				}
				UNINDENT(dat->vr);
				fprintf(dat->vr->file,"%s]\n ",dat->vr->indent);
				VRML2_TexCoordsWrite(dat,lwo, surf, tex);
			} 

			if(dat->vr->vrprefs&VRPREF_NORMALS)
			{
				int normIndx = 0;
				fprintf(dat->vr->file,"%snormalPerVertex FALSE\n",dat->vr->indent);
				fprintf(dat->vr->file,"%snormalIndex [\n",dat->vr->indent);
				INDENT(dat->vr);
				for(i=0; i<lwo->npolygons; i++)
				{
					if( pol=&(lwo->pol[i]) )
						if(pol->sindex==srfIdx)
						{
							fprintf(dat->vr->file,dat->vr->indent);
							fprintf(dat->vr->file,"%d,%c",normIndx,endline15[normIndx&15]); //endline[normIndx&7]);
							normIndx++;
						}
				}
				fprintf(dat->vr->file,"%snormal ",dat->vr->indent);
				VRML2_NormalsWrite(dat,lwo,surf);
			}
			UNINDENT(dat->vr);
			fprintf(dat->vr->file,"%s}\n",dat->vr->indent);
			VRML2_AppearanceWrite(dat, lwo, surf );
		}
		UNINDENT(dat->vr);
		fprintf(dat->vr->file,"%s}\n",dat->vr->indent);
	}


	if(linz)
	{
		fprintf(dat->vr->file,",\n%sShape {\n",dat->vr->indent);
		INDENT(dat->vr);
		fprintf(dat->vr->file,"%sgeometry  IndexedLineSet {\n",dat->vr->indent);
		INDENT(dat->vr);
		if(dat->vr->vrprefs&VRPREF_PROTO_OBJS)
			fprintf(dat->vr->file,"%scoord Coordinate { point IS point }\n",dat->vr->indent);
		else
			fprintf(dat->vr->file,"%scoord USE %s"VDEF_COORDS_DEF"\n",dat->vr->indent,name);
		fprintf(dat->vr->file,"%scoordIndex [\n",dat->vr->indent);
		INDENT(dat->vr);
		for(i=0; i<lwo->npolygons; i++)
		{
			if( pol=&(lwo->pol[i]) )
				if(pol->sindex==srfIdx)
					if(pol->nverts==2)
					{
						fprintf(dat->vr->file,dat->vr->indent);
						fprintf(dat->vr->file,"%d, %d, -1", pol->v[0].index, pol->v[1].index, endline15[p&15]); //endline[p&7]);
						fprintf(dat->vr->file,"%s-1,\n",(p&7) ? " ":dat->vr->indent );	
					}
		}
		UNINDENT(dat->vr);
 		fprintf(dat->vr->file,"%s]\n%scolorPerVertex FALSE\n",dat->vr->indent,dat->vr->indent);
		UNINDENT(dat->vr);
		fprintf(dat->vr->file,"%s}\n",dat->vr->indent);
		if(surf) 
			VRML2_AppearanceWrite(dat, lwo, surf);
		UNINDENT(dat->vr);
		fprintf(dat->vr->file,"%s}\n",dat->vr->indent);
	} 
	if(dotz)
	{
		int t=0;
		DBPoint *pnt;

		fprintf(dat->vr->file,",\n%sShape {\n",dat->vr->indent);
		INDENT(dat->vr);
		fprintf(dat->vr->file,"%sgeometry  PointSet {\n",dat->vr->indent);
		INDENT(dat->vr);
		if(dotz==lwo->npolygons)  // just use existing Node, if all 1 point polys!
		{
			if(dat->vr->vrprefs&VRPREF_PROTO_OBJS)
				fprintf(dat->vr->file,"%scoord Coordinate { point IS point }\n",dat->vr->indent);
			else
				fprintf(dat->vr->file,"%scoord USE %s"VDEF_COORDS_DEF"\n",dat->vr->indent,name);
		}
		else
		{
			fprintf(dat->vr->file,"%scoord Coordinate { point [\n",dat->vr->indent);
			INDENT(dat->vr);
			for(i=0; i<lwo->npolygons; i++)
			{
				if( pol=&(lwo->pol[i]) )
					if(pol->sindex==srfIdx)
						if(pol->nverts==1)
						{
							pnt = &(lwo->pt[pol->v[0].index]);
							fprintf(dat->vr->file,dat->vr->indent);
							if(t&1) 
								fprintf(dat->vr->file,"%f %f %f,\n",
									pnt->pos[0][0],
									pnt->pos[0][1],
									RIGHT_HANDZ(pnt->pos[0][2]));
							else 
								fprintf(dat->vr->file,"%s%f %f %f, ",dat->vr->indent,
									pnt->pos[0][0],
									pnt->pos[0][1],
									RIGHT_HANDZ(pnt->pos[0][2]));
							t++;
						}
			}
			UNINDENT(dat->vr);
 			fprintf(dat->vr->file,"%s] }\n",dat->vr->indent);
		}
		UNINDENT(dat->vr);
		fprintf(dat->vr->file,"%s}\n",dat->vr->indent);  // close pointset
		if(surf) 
		{
			fprintf(dat->vr->file,"%sappearance Appearance {\n\t%smaterial Material {\n",dat->vr->indent,dat->vr->indent);
			fprintf(dat->vr->file,"%s\t\temissiveColor   %.4f %.4f %.4f } }\n",dat->vr->indent,
				surf->colr[0], surf->colr[1], surf->colr[2]);
		}
		UNINDENT(dat->vr);
		fprintf(dat->vr->file,"%s}\n",dat->vr->indent); // close shape
	} 

	return(n);
}


static char uvfile[MAX_STR+1]="";

int VRML2_ObjWrite(VRMLData *dat, void *fukd, char *url, char *comment)
{
	ObjectDB	*lwo = (ObjectDB*)fukd;
	int i=0,srfs=0, sid;
	char *fname;
	char *name, *argv[]={NULL,"Output by Lightwave3D from NewTek:  http://www.newtek.com",NULL};	  
	LWDVector bb,cnt;

	if(lwo )
	{
		fname = lwo->filename;
		strncpy(uvfile,fname,MAX_STR);
		killext(uvfile);
		strncat(uvfile,".uv",MAX_STR);
//		dat->uvFile = openUVFile(uvfile,&(dat->poly_pos));
		name = filepart(fname);
		strncpy(uvfile,name,MAX_STR);
		fixVRMLName(uvfile,MAX_STR);
		name = uvfile;
		srfs=lwo->nsurfaces;
		if( !(dat->vr->vrprefs&VRPREF_INCLUDE_OBJS) )
	 	{
			argv[0] = comment;  
			VRML2_WorldInfoWrite(dat,name,2,argv);
		}
		if(dat->vr->vrprefs&VRPREF_COMMENTS)
			fprintf(dat->vr->file,"#\tObject: %s \n",name);
		killext(name);
		strncpy(uvfile,name,MAX_STR);
		fixVRMLName(uvfile,MAX_STR);
		name = uvfile;
		VRML2_PointsWrite(dat,lwo,name);

	
		if( (url && (*url!=0)) || !(dat->vr->vrprefs&VRPREF_INCLUDE_OBJS) )  // for easier parsing at Atari (?)
		{
			fprintf(dat->vr->file,"DEF %s ",name);		
			boundsCenter(lwo, bb, cnt);
			VRML2_AnchorGroupOpen(dat, bb, cnt, url);
		}
	//	if( !(dat->vr->vrprefs&VRPREF_INCLUDE_OBJS) )  // for easier parsing at Atari (?)
			VRML2_TransformOpen(dat,name); // needed for multiple surfaces!!!

		for(sid=0; sid<lwo->nsurfaces-1; sid++)
		{
			VRML2_ShapeWrite(dat,lwo,sid,name);//,0);
			fprintf(dat->vr->file,"%s,\n",dat->vr->indent);
		}
		VRML2_ShapeWrite(dat,lwo,sid,name);//,0);	  // write last one w/out a comma!
		if( (dat->vr->vrprefs&VRPREF_VIEWS)	&& !(dat->vr->vrprefs&VRPREF_INCLUDE_OBJS) )
			VRML97_ViewsWrite(dat->vr,0, bb, cnt, name);
//		if( !(dat->vr->vrprefs&VRPREF_INCLUDE_OBJS) )  // for easier parsing at Atari (?)
//		{
			fprintf(dat->vr->file,"## %sDEF %s_Sense SphereSensor { enabled TRUE }\n",dat->vr->indent,name);
			VRML2_TransformClose(dat);
//		}
		if( (url && (*url!=0)) || !(dat->vr->vrprefs&VRPREF_INCLUDE_OBJS) )  // for easier parsing at Atari (?)
			VRML2_GroupClose(dat);
		if( !(dat->vr->vrprefs&VRPREF_INCLUDE_OBJS) )  // for easier parsing at Atari (?)
			fprintf(dat->vr->file,"#%sROUTE %s_Sense.rotation_changed TO %s"VDEF_OBJXFORM".set_rotation\n",dat->vr->indent,name,name);
	/*	if(dat->uvFile)
		{
			fclose(dat->uvFile);
			dat->uvFile = NULL;
		} */
	}
	return srfs;
}

int VRML2_ProtoObjWrite(VRMLData *dat, void *fukd, char *url, char *comment)
{
	ObjectDB	*lwo = (ObjectDB*)fukd;
	int i=0,srfs=0,surf;
	char *fname;
	char *name, *argv[]={NULL,"Output by Lightwave3D from NewTek:  http://www.newtek.com",NULL};	  
	LWDVector bb,cnt;

	if(lwo)
	{
		char oname[MAX_STR+1];
		fname = lwo->filename;
		strncpy(oname,fname,MAX_STR);
		strncpy(uvfile,fname,MAX_STR);
		killext(uvfile);
		strncat(uvfile,".uv",MAX_STR);
	//	dat->uvFile = openUVFile(uvfile,&(dat->poly_pos));
		name = filepart(oname);
		srfs=lwo->nsurfaces;
		if( !(dat->vr->vrprefs&VRPREF_INCLUDE_OBJS) )
	 	{
			argv[0] = comment;  
			VRML2_WorldInfoWrite(dat,name,2,argv);
		}
		if(dat->vr->vrprefs&VRPREF_COMMENTS)
			fprintf(dat->vr->file,"#\tObject: %s \n",name);
		killext(name);

 // PROTO header
		fprintf(dat->vr->file,"%sPROTO %s"VDEF_OBJ_PROTO"  [\n",dat->vr->indent,name);		
		INDENT(dat->vr);
		fprintf(dat->vr->file,"%s exposedField MFVec3f ",dat->vr->indent);
		VRMLPointArrayWrite(dat,lwo);
 		boundsCenter(lwo, bb, cnt);
		fprintf(dat->vr->file,"\n%s field SFVec3f bboxCenter %.4f  %.4f  %.4f\n",dat->vr->indent,cnt[0],cnt[1],cnt[2]);
  		fprintf(dat->vr->file,"%s field SFVec3f bboxSize   %.4f  %.4f  %.4f\n",dat->vr->indent,bb[0],bb[1],bb[2]);
   		fprintf(dat->vr->file,"%s exposedField SFBool	turnable FALSE\n",dat->vr->indent);
		UNINDENT(dat->vr);
		fprintf(dat->vr->file,"%s]\n",dat->vr->indent);

 // PROTO body
		fprintf(dat->vr->file,"%s{ # PROTO body \n",dat->vr->indent,name);		
		INDENT(dat->vr);

//		VRML2_AnchorGroupOpen(dat, bb, cnt, url); 
		if(url && *url)
		{
			fprintf(dat->vr->file,"%sAnchor {\n",dat->vr->indent);
			fprintf(dat->vr->file,"%s\turl  [ \"%s\" ]\n",dat->vr->indent,url);
			INDENT(dat->vr);
			fprintf(dat->vr->file,"%sbboxCenter IS bboxCenter\n",dat->vr->indent);
			fprintf(dat->vr->file,"%sbboxSize IS bboxSize\n",dat->vr->indent);
			fprintf(dat->vr->file,"%schildren  [ \n",dat->vr->indent);
			INDENT(dat->vr);
		}
		else if( !(dat->vr->vrprefs&VRPREF_INCLUDE_OBJS) )  // for easier parsing at Atari (?)
		{
			fprintf(dat->vr->file,"%sGroup {\n",dat->vr->indent);
			INDENT(dat->vr);
			fprintf(dat->vr->file,"%sbboxCenter IS bboxCenter\n",dat->vr->indent);
			fprintf(dat->vr->file,"%sbboxSize IS bboxSize\n",dat->vr->indent);
			fprintf(dat->vr->file,"%schildren  [ \n",dat->vr->indent);
			INDENT(dat->vr);
		}
		VRML2_TransformOpen(dat,name); // needed for multiple surfaces!!!

		

		for(surf=0; surf<srfs-1; surf++)
		{
			VRML2_ShapeWrite(dat,lwo,surf,name);
			fprintf(dat->vr->file,"%s,\n",dat->vr->indent);
		}
		VRML2_ShapeWrite(dat,lwo,surf,name);	  // write last one w/out a comma!
//		if( !(dat->vr->vrprefs&VRPREF_INCLUDE_OBJS) )  // for easier parsing at Atari (?)
//		{
			fprintf(dat->vr->file,"## %sDEF %s_Sense SphereSensor { enabled IS turnable }\n",dat->vr->indent,name);
			VRML2_TransformClose(dat);
//		}
		if( (url && *url) || !(dat->vr->vrprefs&VRPREF_INCLUDE_OBJS))
		{
//			VRML2_GroupClose(dat);
			UNINDENT(dat->vr);
			fprintf(dat->vr->file,"%s]\n",dat->vr->indent);
			UNINDENT(dat->vr);
			fprintf(dat->vr->file,"%s}\n",dat->vr->indent);
		}
		if( !(dat->vr->vrprefs&VRPREF_INCLUDE_OBJS) )  // for easier parsing at Atari (?)
			fprintf(dat->vr->file,"## %sROUTE %s_Sense.rotation_changed TO %s"VDEF_OBJXFORM".set_rotation\n",dat->vr->indent,name,name);
		UNINDENT(dat->vr);
		fprintf(dat->vr->file,"%s} # End PROTO body \n",dat->vr->indent,name);	
			
 // PROTO instantiation
		if( !(dat->vr->vrprefs&VRPREF_INCLUDE_OBJS) )	//if( !(dat->vr->vrprefs&VRPREF_NOINSTANCE) )
			fprintf(dat->vr->file,"%s %s"VDEF_OBJ_PROTO" { } # Default instance, for viewing this file directly, or inlining\n",dat->vr->indent,name);		
		if( (dat->vr->vrprefs&VRPREF_VIEWS)	&& !(dat->vr->vrprefs&VRPREF_INCLUDE_OBJS) )
			VRML97_ViewsWrite(dat->vr,0, bb, cnt, name);

/*		if(dat->uvFile)
		{
			fclose(dat->uvFile);
			dat->uvFile = NULL;
		} */
	
	}
	return srfs;
}





int VRML97_ProtoShapeWrite(VRMLData *dat, void *fukd, int srfIdx,char *name)
{
	ObjectDB	*lwo = (ObjectDB*)fukd;
	int p,n,linz=0,cvex=1,dotz=0,facz=0;
	int	i;
	static int	vtxSurf;
	DBPolygon*	pol;  
	DBSurface *surf;
//	LWSurfaceID	sur=surf->id;
	if(!lwo->id)
		return 0;
	if(srfIdx<lwo->nsurfaces)
		surf = &(lwo->surf[srfIdx]);
	if(surf)  
		fprintf(dat->vr->file,"%s# Polygons with surface named %s \n",dat->vr->indent,surf->name);
	else return 0;
	if(srfIdx==0)
		vtxSurf = 0;
	for(i=0; i<lwo->npolygons; i++)
	{
		if( pol=&(lwo->pol[i]) )
			if(pol->sindex==srfIdx)
				if(pol->nverts>=3)
					facz++;
				else if(pol->nverts==2)
					linz++;
				else if(pol->nverts==1)
					dotz++;
	}


	if(facz)  // skip the IFS if only lines 
	{
		fprintf(dat->vr->file,"%sShape {\n",dat->vr->indent);
		INDENT(dat->vr);

		fprintf(dat->vr->file,"%sgeometry  IndexedFaceSet {\n",dat->vr->indent);
		INDENT(dat->vr);
	#ifndef CCW_POLYS
		fprintf(dat->vr->file,"%sccw FALSE\n",dat->vr->indent);
	#else
		fprintf(dat->vr->file,"%sccw TRUE\n",dat->vr->indent);
	#endif
		if(vtxSurf)
			fprintf(dat->vr->file,"%scoord USE "VDEF_VERTS"\n",dat->vr->indent);
		else // define coords on first surface, instead of empty shape
		{
			fprintf(dat->vr->file,"%scoord DEF "VDEF_VERTS" Coordinate { ",dat->vr->indent);
			VRMLPointArrayWrite(dat,lwo);
			fprintf(dat->vr->file,"}\n",dat->vr->indent);
			vtxSurf = 1;
		}
		fprintf(dat->vr->file,"%scoordIndex [\n",dat->vr->indent);
		INDENT(dat->vr);
		for(i=0; i<lwo->npolygons; i++)
		{
			if( pol=&(lwo->pol[i]) )
				if(pol->sindex==srfIdx)
					if(pol->nverts>=3)
					{
						if(pol->nverts>3) 
							cvex=0;
						fprintf(dat->vr->file,dat->vr->indent);
						n=pol->nverts;
		#ifndef CCW_POLYS
						for(p=0; p<n; p++)
							fprintf(dat->vr->file,"%d,%c",pol->v[p].index,endline15[p&15]); //endline[p&7]);
		#else
						for(p=n; p; p--)
							fprintf(dat->vr->file,"%d,%c",pol->v[p-1].index,endline15[p&15]); //endline[p&7]);
		#endif
						fprintf(dat->vr->file,"%s-1,\n",(p&15) ? " ":dat->vr->indent );	
					}
		}
		UNINDENT(dat->vr);
 		fprintf(dat->vr->file,"%s]\n",dat->vr->indent);
		if(cvex)
			fprintf(dat->vr->file,"%sconvex TRUE\n",dat->vr->indent);
		else
			fprintf(dat->vr->file,"%sconvex FALSE\n",dat->vr->indent);
		if(VRML97_VertexColorsWrite(dat, lwo, surf))
			fprintf(dat->vr->file,"%scolorPerVertex TRUE\n",dat->vr->indent);
		if(surf) 
		{
			LWTextureID		tex;
			// 6/7/97
			//srf.su_tx.zTextureCenter *= RIGHT_HANDZ(1);
			if(surf->sman>0.0f) 
				fprintf(dat->vr->file,"%screaseAngle %04f\n",dat->vr->indent,(surf->sman));	// already radians
			if(tex = surfFuncs->getTex(surf->id, SURF_COLR)) 
			{
				int txIndx = 0;
				// tex coord index first
				fprintf(dat->vr->file,"%stexCoordIndex [\n",dat->vr->indent);
				INDENT(dat->vr);
				for(i=0; i<lwo->npolygons; i++)
				{
					if( pol=&(lwo->pol[i]) )
						if(pol->sindex==srfIdx)
						{
							fprintf(dat->vr->file,dat->vr->indent);
							for(p=0; p<pol->nverts; p++, txIndx++)
								fprintf(dat->vr->file,"%d,%c",txIndx,endline15[p&15]); //endline[p&7]);
							fprintf(dat->vr->file,"%s-1,\n",(p&7) ? " ":dat->vr->indent );	
						}
				}
				UNINDENT(dat->vr);
				fprintf(dat->vr->file,"%s]\n ",dat->vr->indent);
				VRML2_TexCoordsWrite(dat,lwo, surf, tex);
			} 

			if(dat->vr->vrprefs&VRPREF_NORMALS)
			{
				int normIndx = 0;
				fprintf(dat->vr->file,"%snormalPerVertex FALSE\n",dat->vr->indent);
				fprintf(dat->vr->file,"%snormalIndex [\n",dat->vr->indent);
				INDENT(dat->vr);
				for(i=0; i<lwo->npolygons; i++)
				{
					if( pol=&(lwo->pol[i]) )
						if(pol->sindex==srfIdx)
						{
							fprintf(dat->vr->file,dat->vr->indent);
							fprintf(dat->vr->file,"%d,%c",normIndx,endline15[normIndx&15]); //endline[normIndx&7]);
							normIndx++;
						}
				}
				fprintf(dat->vr->file,"%snormal ",dat->vr->indent);
				VRML2_NormalsWrite(dat,lwo,surf);
			}
			UNINDENT(dat->vr);
			fprintf(dat->vr->file,"%s}\n",dat->vr->indent);
			VRML2_AppearanceWrite(dat, lwo, surf );
		}
		UNINDENT(dat->vr);
		fprintf(dat->vr->file,"%s}\n",dat->vr->indent);
	}


	if(linz)
	{
		fprintf(dat->vr->file,",\n%sShape {\n",dat->vr->indent);
		INDENT(dat->vr);
		fprintf(dat->vr->file,"%sgeometry  IndexedLineSet {\n",dat->vr->indent);
		INDENT(dat->vr);
		if(vtxSurf)
			fprintf(dat->vr->file,"%scoord USE "VDEF_VERTS"\n",dat->vr->indent);
		else // define coords on first surface, instead of empty shape
		{
			fprintf(dat->vr->file,"%scoord DEF "VDEF_VERTS" Coordinate { ",dat->vr->indent);
			VRMLPointArrayWrite(dat,lwo);
			fprintf(dat->vr->file,"}\n",dat->vr->indent);
			vtxSurf = 1;
		}
		fprintf(dat->vr->file,"%scoordIndex [\n",dat->vr->indent);
		INDENT(dat->vr);
		for(i=0; i<lwo->npolygons; i++)
		{
			if( pol=&(lwo->pol[i]) )
				if(pol->sindex==srfIdx)
					if(pol->nverts==2)
					{
						fprintf(dat->vr->file,dat->vr->indent);
						fprintf(dat->vr->file,"%d, %d, -1", pol->v[0].index, pol->v[1].index, endline15[p&15]); //endline[p&7]);
						fprintf(dat->vr->file,"%s-1,\n",(p&7) ? " ":dat->vr->indent );	
					}
		}
		UNINDENT(dat->vr);
 		fprintf(dat->vr->file,"%s]\n%scolorPerVertex FALSE\n",dat->vr->indent,dat->vr->indent);
		UNINDENT(dat->vr);
		fprintf(dat->vr->file,"%s}\n",dat->vr->indent);
		if(surf) 
			VRML2_AppearanceWrite(dat, lwo, surf);
		UNINDENT(dat->vr);
		fprintf(dat->vr->file,"%s}\n",dat->vr->indent);
	} 
	if(dotz)
	{
		int t=0;
		DBPoint *pnt;

		fprintf(dat->vr->file,",\n%sShape {\n",dat->vr->indent);
		INDENT(dat->vr);
		fprintf(dat->vr->file,"%sgeometry  PointSet {\n",dat->vr->indent);
		INDENT(dat->vr);
		if(dotz==lwo->npolygons)  // just use existing Node, if all 1 point polys!
		{
			fprintf(dat->vr->file,"%scoord DEF "VDEF_VERTS" Coordinate { ",dat->vr->indent);
			VRMLPointArrayWrite(dat,lwo);
			fprintf(dat->vr->file,"}\n",dat->vr->indent);
			vtxSurf = 1;
		}
		else
		{
			fprintf(dat->vr->file,"%scoord Coordinate { point [\n",dat->vr->indent);
			INDENT(dat->vr);
			for(i=0; i<lwo->npolygons; i++)
			{
				if( pol=&(lwo->pol[i]) )
					if(pol->sindex==srfIdx)
						if(pol->nverts==1)
						{
							pnt = &(lwo->pt[pol->v[0].index]);
							fprintf(dat->vr->file,dat->vr->indent);
							if(t&1) 
								fprintf(dat->vr->file,"%f %f %f,\n",
									pnt->pos[0][0],
									pnt->pos[0][1],
									RIGHT_HANDZ(pnt->pos[0][2]));
							else 
								fprintf(dat->vr->file,"%s%f %f %f, ",dat->vr->indent,
									pnt->pos[0][0],
									pnt->pos[0][1],
									RIGHT_HANDZ(pnt->pos[0][2]));
							t++;
						}
			}
			UNINDENT(dat->vr);
 			fprintf(dat->vr->file,"%s] }\n",dat->vr->indent);
		}
		UNINDENT(dat->vr);
		fprintf(dat->vr->file,"%s}\n",dat->vr->indent);  // close pointset
		if(surf) 
		{
			fprintf(dat->vr->file,"%sappearance Appearance {\n\t%smaterial Material {\n",dat->vr->indent,dat->vr->indent);
			fprintf(dat->vr->file,"%s\t\temissiveColor   %.4f %.4f %.4f } }\n",dat->vr->indent,
				surf->colr[0], surf->colr[1], surf->colr[2]);
		}
		UNINDENT(dat->vr);
		fprintf(dat->vr->file,"%s}\n",dat->vr->indent); // close shape
	} 

	return(n);
}


int VRML97_ProtoObjWrite(VRMLData *dat, void *fukd, char *url, char *comment)
{
	ObjectDB	*lwo = (ObjectDB*)fukd;
	int i=0,srfs=0,surf;
	char *fname;
	char *name, *argv[]={NULL,"Output by Lightwave3D from NewTek:  http://www.newtek.com",NULL};	  
	LWDVector bb,cnt;

	if(lwo)
	{
		char oname[MAX_STR+1];
		//fname = lwo->filename; // wrong for layers
		//fname = (char*)itemInfo->name(lwo->id); // wrong for clone!!
		fname = itemBaseName(lwo->id);
		name = filepart(fname);
		strncpy(oname,name,MAX_STR);
		name = oname;
		srfs=lwo->nsurfaces;
		if( !(dat->vr->vrprefs&VRPREF_INCLUDE_OBJS) )
	 	{
			argv[0] = comment;  
			VRML2_WorldInfoWrite(dat,name,2,argv);
		}
		if(dat->vr->vrprefs&VRPREF_COMMENTS)
			fprintf(dat->vr->file,"#\tObject: %s \n",name);
		killext(name);
		fixVRMLName(name,MAX_STR);
 // PROTO header
		fprintf(dat->vr->file,"%sPROTO %s"VDEF_OBJ_PROTO" [",dat->vr->indent,name);		
	//	fprintf(dat->vr->file," exposedField SFFloat fraction 0.0");
		fprintf(dat->vr->file," eventIn SFFloat set_fraction");
		fprintf(dat->vr->file," ]\n");

 // PROTO body
		fprintf(dat->vr->file,"%s{ # PROTO body \n",dat->vr->indent,name);		
		INDENT(dat->vr);
// points must be in shape, in transform to be legal child node! 
	/*	fprintf(dat->vr->file,"%sDEF "VDEF_VERTS" Coordinate { ",dat->vr->indent);
		VRMLPointArrayWrite(dat,lwo);
		fprintf(dat->vr->file,"}\n",dat->vr->indent); */

		boundsCenter(lwo, bb, cnt);
		if(url && *url)
		{
			fprintf(dat->vr->file,"%sAnchor {\n",dat->vr->indent);
			fprintf(dat->vr->file,"%s\turl  [ \"%s\" ]\n",dat->vr->indent,url);
			INDENT(dat->vr);
			fprintf(dat->vr->file,"%sbboxCenter %.4f  %.4f  %.4f\n",dat->vr->indent,cnt[0],cnt[1],cnt[2]);
  			fprintf(dat->vr->file,"%sbboxSize   %.4f  %.4f  %.4f\n",dat->vr->indent,bb[0],bb[1],bb[2]);
			fprintf(dat->vr->file,"%schildren  [ \n",dat->vr->indent);
			INDENT(dat->vr);
		}
		else if( !(dat->vr->vrprefs&VRPREF_INCLUDE_OBJS) ) 
		{
			fprintf(dat->vr->file,"%sGroup {\n",dat->vr->indent);
			INDENT(dat->vr);
			fprintf(dat->vr->file,"%sbboxCenter %.4f  %.4f  %.4f\n",dat->vr->indent,cnt[0],cnt[1],cnt[2]);
  			fprintf(dat->vr->file,"%sbboxSize   %.4f  %.4f  %.4f\n",dat->vr->indent,bb[0],bb[1],bb[2]);
			fprintf(dat->vr->file,"%schildren  [ \n",dat->vr->indent);
			INDENT(dat->vr);
		}
		VRML2_TransformOpen(dat,name); // needed for multiple surfaces!!!

/*		Done in ProtoShapeWrite for surf==0
		fprintf(dat->vr->file,"%sShape { geometry IndexedFaceSet { coord\n",dat->vr->indent);
		INDENT(dat->vr);
		fprintf(dat->vr->file,"%sDEF "VDEF_VERTS" Coordinate { ",dat->vr->indent);
		VRMLPointArrayWrite(dat,lwo);
		fprintf(dat->vr->file,"}\n",dat->vr->indent);
		UNINDENT(dat->vr);
		fprintf(dat->vr->file,"}}\n",dat->vr->indent); */


		VRML97_ProtoShapeWrite(dat,lwo,0,name);	  // write first one w/out a comma!
		for(surf=1; surf<srfs; surf++)
		{
			fprintf(dat->vr->file,"%s,\n",dat->vr->indent);
			VRML97_ProtoShapeWrite(dat,lwo,surf,name);
	//		fprintf(dat->vr->file,"%s,\n",dat->vr->indent);
		}
	//	VRML97_ProtoShapeWrite(dat,lwo,surf,name);	  // write last one w/out a comma!

//		if( !(dat->vr->vrprefs&VRPREF_INCLUDE_OBJS) )  // for easier parsing at Atari (?)
			VRML2_TransformClose(dat);

		if( (url && *url) || !(dat->vr->vrprefs&VRPREF_INCLUDE_OBJS))
		{
//			VRML2_GroupClose(dat);
			UNINDENT(dat->vr);
			fprintf(dat->vr->file,"%s]\n",dat->vr->indent);
			UNINDENT(dat->vr);
			fprintf(dat->vr->file,"%s}\n",dat->vr->indent);
		}
		UNINDENT(dat->vr);
		fprintf(dat->vr->file,"%s} # End PROTO body \n",dat->vr->indent,name);	
			
 // PROTO instantiation
		if( !(dat->vr->vrprefs&VRPREF_INCLUDE_OBJS) )	//if( !(dat->vr->vrprefs&VRPREF_NOINSTANCE) )
			fprintf(dat->vr->file,"%s %s"VDEF_OBJ_PROTO" { } # Default instance, for viewing this file directly, or inlining\n",dat->vr->indent,name);		
		if( (dat->vr->vrprefs&VRPREF_VIEWS)	&& !(dat->vr->vrprefs&VRPREF_INCLUDE_OBJS) )
			VRML97_ViewsWrite(dat->vr,0, bb, cnt, name);
	}
	return srfs;
}

int lwo2VRML2(GlobalFunc *global,void *fukd, char *vrml, char *url, char *comment, int flags)
{
	ObjectDB	*lwo = (ObjectDB*)fukd;
	VRMLData 	vdat;
	LWMessageFuncs	*message;
	int i, f1=0, f2=60, n=15;
	if (!(message = (*global) (LWMESSAGEFUNCS_GLOBAL, GFUSE_TRANSIENT)) )
		return 0;
	if(!lwo)	 
	{
		return 0; 
	}
	if(!*vrml)	 
	{
		(*message->error)("Bad file name",("   on VRML output" ));
		return 0; 
	}

	vdat.flags = flags;		  // 0 from obj.save plugin ObjectToVRML(), VRML2_SceneWrite() !!
	if( !(vdat.vr=openVRML97(vrml)) )
		return 0; 
	if( vdat.vr->vrprefs&VRPREF_PROTO_OBJS )
	//	i = VRML97_ProtoObjWrite(&vdat, lwo, url, comment);
		i = VRML97_ProtoMorphWrite(&vdat, lwo, url, f1,f2,n);
	else
		i = VRML2_ObjWrite(&vdat, lwo, url, comment);
	closeVRML97(vdat.vr);
	if(!i)
		(*message->warning)("Error writing VRML file from object", lwo->filename );
	return i; 
}


int lwo2VRML97(GlobalFunc *global,LWItemID id, char *vrml, char *url, char *comment, int flags)
{
	VRMLData 		 vdat;
	ObjectDB		*lwo = NULL;
	LWMessageFuncs	*message;
	char			 tag[200]="";
	int				 i, f1=0, f2=60, n=15;

	if (!(message = (*global) (LWMESSAGEFUNCS_GLOBAL, GFUSE_TRANSIENT)) )
		return 0;

	if(!*vrml)	 
	{
		(*message->error)("Bad file name",("   on VRML output" ));
		return 0; 
	}

	vdat.flags = flags;		  // 0 from obj.save plugin ObjectToVRML(), VRML2_SceneWrite() !!
	if( (vdat.vr=openVRML97(vrml)) )
	{
		vdat.vr->vrprefs = flags;
		if( vdat.vr->vrprefs&VRPREF_PROTO_OBJS )
		{
			if(getItemTag(itemInfo, id, VTAG_MORPH"=", tag,199))
			{
				sscanf(tag,"%d %d %d %d",&f1,&f2,&n,&i);
				if(n)
					n = (f2-f1)/n;
				else
					n = 1;
				i = VRML97_ProtoMorphWrite(&vdat, id, url, f1,f2,n);
			}
			else
				if( (lwo = getObjectDB( id, global )) )	 
				{
					i = VRML97_ProtoObjWrite(&vdat, lwo, url, comment);
					freeObjectDB(lwo);
				}
		}
		else
			if( (lwo = getObjectDB( id, global )) )	 
			{
				i = VRML2_ObjWrite(&vdat, lwo, url, comment);
				freeObjectDB(lwo);
			}
		closeVRML97(vdat.vr);
	}

	if(!i)
		(*message->warning)("Error writing VRML file from object", lwo->filename );
	return i; 
}


