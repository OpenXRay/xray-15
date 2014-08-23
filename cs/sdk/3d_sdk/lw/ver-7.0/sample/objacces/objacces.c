// ObjAcces.c -- LW Object Reading Global Plugin
//   By Arnie Cachelin, Copyright NewTek, Inc. 1996, 1997
// Last Update:
//    9/23/97
// 1/98
// Fri. Feb. 13 1998 - Unix Changes
// 28 Jan 00  bug fixes [EW]
// 06 Jun 00  fixed image seam angle in TextureUV() [EW]
// 07 Jun 00  initialize LWOBInfo->nSdat to 0 in OpenLWObject() [EW]

#include <splug.h>
#include <moni.h>
#include <lwran.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>


#include "lwobjacc.h"


#ifndef PI
#define PI        3.1415916f
#endif
#define M_PI      3.1415916f
#define TWOPI     6.2831832f
#define HALFPI    1.5707958f
#define SWAP(a,b) { a^=b; b^=a; a^=b; }
#define  BYTE  unsigned char
#define  WORD  unsigned short


#ifdef _WINDOWS
#include <dos.h>
#define HEAP_PROBLEM 1//Use this on the PC
#include <malloc.h>
#define BYTE_INVERT     1//Use this to swap bytes on the PC
#define FILE_MARK    '\\'
#define  BSWAP_W(w)     SWAP( (((BYTE *)&w)[0]), (((BYTE *)&w)[1]) )
#define  BSWAP_L(l)     { BSWAP_W( (((WORD *)&l)[0])); BSWAP_W( (((WORD *)&l)[1])); SWAP( (((WORD *)&l)[0]), (((WORD *)&l)[1]) ) }
#else
#define FILE_MARK    '/'
#define  BSWAP_W(w)
#define  BSWAP_L(l)
#endif


typedef struct st_LWOBInfo
{
   FILE        *fp;
   fpos_t      pntPos,polPos,srfPos,pntPosCur,polPosCur,srfPosCur;
   int         pntSize,polSize,srfSize,pntSizeCur,polSizeCur,srfSizeCur,nSdat;
} LWOBInfo;


typedef struct st_LWObject {
   char           *name;
   int            nPnts;
   LWPoint        *pnts;
   int            nPols;
   LWPolygon      *pols;
   int            nSurfs;
   LWSurface      *surfs;
   double         bounds[6];
   double         rMax;
} LWObject;




/* Changed this to pntID.  No real effect, but confusing.  EW 28 Jan 00
polID    MaxList[MAX_POINTS_PER_POLYGON];
*/
pntID    MaxList[MAX_POINTS_PER_POLYGON];
LWPolygon MaxPoly = {MaxList,0,0};
static int ret=0;


#ifdef _SUN
void Shutdown ( void *dat)
{
}


void *Startup()
{
   return (void *)&ret;
}
#endif
static int initPolygon(LWPolygon *poly)
{
   poly->npoints=0;
   poly->surface=0;
   poly->plist = calloc(sizeof(pntID),MAX_POINTS_PER_POLYGON);
   if (poly->plist==NULL)
      return(0);
   return(1);
}


static void copyPolygon(LWPolygon *newPoly, LWPolygon *poly)
{
   if(poly && newPoly)
   {
      newPoly->npoints = poly->npoints;
      newPoly->surface = poly->surface;
      newPoly->plist = calloc(sizeof(pntID),newPoly->npoints);
      if(newPoly->plist)
      {
         int i;
         for(i=0; i<newPoly->npoints; i++)
            newPoly->plist[i] = poly->plist[i];
      }
   }
}


static LWPolygon *makePolygon(LWPolygon *poly)
{
   LWPolygon *newPoly=NULL;
   if(newPoly = malloc(sizeof(LWPolygon)))
      if(poly)
      {
         newPoly->npoints=poly->npoints;
         newPoly->surface=poly->surface;
         newPoly->plist = calloc(sizeof(pntID),newPoly->npoints);
         if(newPoly->plist)
         {
            int i;
            for(i=0; i<newPoly->npoints; i++)
               newPoly->plist[i] = poly->plist[i];
         }
         else
         {
            free(newPoly);
            return NULL;
         }


      }
      else
         initPolygon(newPoly);
   return newPoly;
}


static void freePolygon(LWPolygon *poly)
{
   if(poly)
   {
      if(poly->plist)
         free(poly->plist);
      free(poly);
   }
}


static int GetPolygon(LWOBInfo *object, LWPolygon *poly)
{
   unsigned short pntcnt;
   unsigned short pointval;
   unsigned short junk;
   unsigned short detail;
   short surface;
   int k,x;


   fsetpos(object->fp,&object->polPosCur);
   if (object->polSizeCur<object->polSize){


      fread(&pntcnt,2,1,object->fp);


      BSWAP_W(pntcnt)   ;


      object->polSizeCur = object->polSizeCur + 2;


      poly->npoints = pntcnt;


      for(x=0;x<pntcnt;x++){
         fread(&pointval,2,1,object->fp);
         BSWAP_W(pointval);


         object->polSizeCur = object->polSizeCur + 2;
         poly->plist[x]=pointval;
         }


      fread(&surface,2,1,object->fp);
      BSWAP_W(surface);


      object->polSizeCur = object->polSizeCur + 2;


      if (surface<0)
      {
         fread(&detail,2,1,object->fp);
         BSWAP_W(detail);
         object->polSizeCur = object->polSizeCur + 2;


         for(k=0;k<detail;k++)
         {
            fread(&junk,2,1,object->fp);
            BSWAP_W(junk);


            fseek(object->fp,(junk+1)*2,SEEK_CUR);
            object->polSizeCur = object->polSizeCur + ((junk+2)*2);
         }
         poly->surface = abs(surface);
      }
      else
         poly->surface=surface;


      fgetpos(object->fp,&object->polPosCur);
      }
   else
      return(0);


   return(1);
}






static int GetPoint(LWOBInfo *object, LWPoint *pnt)
{
   float x,y,z;




   fsetpos(object->fp,&object->pntPosCur);


   if (object->pntSizeCur<object->pntSize)
   {
      fread(&x,4,1,object->fp);
      fread(&y,4,1,object->fp);
      fread(&z,4,1,object->fp);


      BSWAP_L(x);
      BSWAP_L(y);
      BSWAP_L(z);

      pnt->x=x;
      pnt->y=y;
      pnt->z=z;


      fgetpos(object->fp,&object->pntPosCur);
      object->pntSizeCur = object->pntSizeCur + 12;
   }
   else
      return(0);


   return(1);
}


#define MAX_SURFNAME 82


static int GetSurface(LWOBInfo *object, LWSurface *srf)
{
   char  buf[256];
   int   len;
   fsetpos(object->fp,&object->srfPosCur);


   if (object->srfSizeCur<object->srfSize)
   {
      fread(buf,1,256,object->fp);
      len = strlen(buf) + 1;  // get NULL too
      fsetpos(object->fp,&object->srfPosCur);
      if(len&1) len++;
      fread(buf,1,len,object->fp);
      strncpy(srf->name,buf,MAX_SURFNAME);
      fgetpos(object->fp,&object->srfPosCur);
      object->srfSizeCur = object->srfSizeCur + len;
   }
   else
      return(0);


   return(1);
}




static void GetSRF(LWOBInfo *object, char *buf, int siz)
{
   fsetpos(object->fp,&object->srfPos);
   fread(buf,1,siz,object->fp);
}


static int GetSDatSize(LWOBInfo *object, char *name, fpos_t *datpos)
{
   char temp[MAX_SURFNAME];
   int readsize=0,objsize,tempsize,len;
   if(!object->nSdat)
      return 0;
   if( !(len=strlen(name)) )
      return 0;
   len++;
   if(len&1)
      len++;
   fseek(object->fp, 8, SEEK_SET);
   fread(&temp,4,1,object->fp);
   if (strncmp(temp,"LWOB",4)==0)
   {
      //Get the object size
      fseek(object->fp,4,SEEK_SET);
      fread(&objsize,4,1,object->fp);
      BSWAP_L(objsize);
      fseek(object->fp,4,SEEK_CUR); //Skip the LWOB deal


      readsize=4;
      while(readsize<objsize)
      {
         fread(temp,4,1,object->fp);
         fread(&tempsize,4,1,object->fp);
         BSWAP_L(tempsize);
         if(strncmp(temp,"SURF",4)==0)
         {
            fgetpos(object->fp,datpos);
            fread(temp,1,len,object->fp);
            if(!strcmp(name,temp))
            {
               fgetpos(object->fp,datpos);
               return tempsize-len;
            }
            else
               fsetpos(object->fp,datpos);
         }
         fseek(object->fp,tempsize,SEEK_CUR);
         readsize+=tempsize+8;
      }
   }
   return 0;
}


static void GetSDat(LWOBInfo *object, char *buf, int siz,fpos_t *datpos)
{
   fsetpos(object->fp,datpos);
   fread(buf,1,siz,object->fp);
}


static void ResetPosition(LWOBInfo *object)
{
   object->pntPosCur=object->pntPos;
   object->polPosCur=object->polPos;
   object->srfPosCur=object->srfPos;
   object->pntSizeCur=0;
   object->polSizeCur=0;
   object->srfSizeCur=0;
}


static void CloseLWObject(LWOBInfo *object)
{
   fclose(object->fp);
}




static int OpenLWObject(char *name, LWOBInfo *object)
{
   char temp[10];
   int readsize=0;
   int objsize;
   int polsize, pntsize;
   int tempsize;


   object->fp=NULL;
   object->pntPos=0;
   object->polPos=0;
   object->polSize=0;
   object->pntSize=0;
   object->nSdat = 0;    /* [EW 7 Jun 00] */

   object->fp=fopen(name,"rb");
   if (object->fp==NULL)
      return(0);


   fseek(object->fp, 8, SEEK_SET);
   fread(&temp,4,1,object->fp);
   if (strncmp(temp,"LWOB",4)==0)
   {
      //Get the object size
      fseek(object->fp,4,SEEK_SET);
      fread(&objsize,4,1,object->fp);
      BSWAP_L(objsize);
      fseek(object->fp,4,SEEK_CUR); //Skip the LWOB deal


      readsize=4;
      while(readsize<objsize){


         fread(temp,4,1,object->fp);
         if(strncmp(temp,"POLS",4)==0){
            fread(&polsize,4,1,object->fp);
            BSWAP_L(polsize);


            fgetpos(object->fp,&object->polPos);
            fseek(object->fp,polsize,SEEK_CUR);
            object->polSize=polsize;
            readsize+=polsize+8;
            }
         else if(strncmp(temp,"PNTS",4)==0){
            fread(&pntsize,4,1,object->fp);
            BSWAP_L(pntsize);


            fgetpos(object->fp,&object->pntPos);
            fseek(object->fp,pntsize,SEEK_CUR);
            object->pntSize=pntsize;
            readsize+=pntsize+8;
            }
         else if(strncmp(temp,"SRFS",4)==0){
            fread(&pntsize,4,1,object->fp);
            BSWAP_L(pntsize);


            fgetpos(object->fp,&object->srfPos);
            fseek(object->fp,pntsize,SEEK_CUR);
            object->srfSize=pntsize;
            readsize+=pntsize+8;
            }
         else{
            if(strncmp(temp,"SURF",4)==0)
               object->nSdat++;
            fread(&tempsize,4,1,object->fp);
            BSWAP_L(tempsize);
            fseek(object->fp,tempsize,SEEK_CUR);
            readsize+=tempsize+8;
            }


         }


      ResetPosition(object);


   }
   else  return(0);


      //printf("Not a LightWave Object!\n");

   return(1);
}




static LWObject *createLWObject(const char *name)
{
   char  sname[MAX_SURFNAME];
   LWOBInfo LWObj;
   LWPoint pnt;
   LWObject   *Object=NULL;
   LWSurface   surf;
   int   p;


   surf.name = sname;
   surf.data = NULL;
   surf.size = 0;
   if(!OpenLWObject((char *)name,&LWObj))
      return(NULL);


   if(Object=malloc(sizeof(LWObject)+strlen(name)+2) )
   {
      memset(Object,0,sizeof(LWObject));
      Object->name = (char *)&(Object[1]);
      strcpy(Object->name,name);
      while(GetPolygon(&LWObj,&MaxPoly)==1)
         Object->nPols++;
      ResetPosition(&LWObj);
      if(Object->pols = calloc(sizeof(LWPolygon),Object->nPols))
      {
         for(p=0; (p<Object->nPols) && (GetPolygon(&LWObj,&MaxPoly)==1); p++ )
            copyPolygon(&(Object->pols[p]), &MaxPoly);
      }
      while(GetPoint(&LWObj,&pnt)==1)
         Object->nPnts++;
      ResetPosition(&LWObj);
      if(Object->pnts = calloc(sizeof(LWPoint),Object->nPnts))
      {
         double r;
         for(p=0; (p<Object->nPnts) && (GetPoint(&LWObj,&pnt)==1); p++ )
         {
            if(p)
            {
               if(pnt.x>Object->bounds[BB_XMAX])
                  Object->bounds[BB_XMAX] = pnt.x;
               if(pnt.x<Object->bounds[BB_XMIN])
                  Object->bounds[BB_XMIN] = pnt.x;
               if(pnt.y>Object->bounds[BB_YMAX])
                  Object->bounds[BB_YMAX] = pnt.y;
               if(pnt.y<Object->bounds[BB_YMIN])
                  Object->bounds[BB_YMIN] = pnt.y;
               if(pnt.z>Object->bounds[BB_ZMAX])
                  Object->bounds[BB_ZMAX] = pnt.z;
               if(pnt.z<Object->bounds[BB_ZMIN])
                  Object->bounds[BB_ZMIN] = pnt.z;
               r = sqrt(pnt.x*pnt.x + pnt.y*pnt.y + pnt.z*pnt.z);
               if(Object->rMax<r)
                  Object->rMax = r;
            }
            else   // initialize max values = first values
            {
               Object->rMax = sqrt(pnt.x*pnt.x + pnt.y*pnt.y + pnt.z*pnt.z);
               Object->bounds[BB_XMIN] = Object->bounds[BB_XMAX] = pnt.x;
               Object->bounds[BB_YMIN] = Object->bounds[BB_YMAX] = pnt.y;
               Object->bounds[BB_ZMIN] = Object->bounds[BB_ZMAX] = pnt.z;
            }
            Object->pnts[p] = pnt;
         }
         while(GetSurface(&LWObj,&surf)==1)
            Object->nSurfs++;
         if(Object->surfs = malloc( LWObj.srfSize+(sizeof(LWSurface)*(Object->nSurfs+1)) ))
         {
            int   i,len,siz;
            fpos_t datpos;
            char *buf = (char *)&(Object->surfs[Object->nSurfs+1]);
            memset(Object->surfs,0,( LWObj.srfSize + (sizeof(LWSurface)*(Object->nSurfs+1)) ));
            GetSRF(&LWObj,buf,LWObj.srfSize);
            for(i=0; i<Object->nSurfs; i++)
            {
               Object->surfs[i].name = buf;
               Object->surfs[i].size = 0;
               Object->surfs[i].data = NULL;
               if( (siz=GetSDatSize(&LWObj,buf,&datpos)) )
               {
                  if( (Object->surfs[i].data=malloc(siz)) )
                  {
                     GetSDat(&LWObj,Object->surfs[i].data,siz,&datpos);
                     Object->surfs[i].size = siz;
                  }
               }
               len = strlen(buf) + 1;  // get NULL too
               if(len&1) len++;
               buf +=  len;
            }
            Object->surfs[i].name = NULL;
         }
      }
   }
   CloseLWObject(&LWObj);
#ifdef HEAP_PROBLEM
   _heapmin();
#endif
   return(Object);
}


static void destroyLWObject(LWObject *Object)
{
   int p,i;
   if(Object)
   {
      if(Object->pols)
      {
         for(p=0; p<Object->nPols; p++)
            if((Object->pols[p]).plist)
               free((Object->pols[p]).plist);
         free(Object->pols);
      }
      if(Object->pnts)
      {
         free(Object->pnts);
      }
      if(Object->surfs)
      {
         for(i=0; i<Object->nSurfs; i++)
            if(Object->surfs[i].size)
               if(Object->surfs[i].data)
                  free(Object->surfs[i].data);
         free(Object->surfs);
      }
      free(Object);
#ifdef HEAP_PROBLEM
      _heapmin();
#endif
   }
}


static LWPoint *getObjPoint(LWObject *obj, pntID i)
{
   if(i<obj->nPnts)
      return(&(obj->pnts[i]));
   else
      return NULL;
}


static LWPolygon *getObjPoly(LWObject *obj, polID i)
{
   if(i<obj->nPols)
      return(&(obj->pols[i]));
   else
      return NULL;
}


static LWSurface *getObjSurf(LWObject *obj, srfID i)
{
   if(i<obj->nSurfs)
      return(&(obj->surfs[i]));
   else
      return NULL;
}


static int numObjPoints(LWObject *Object)
{
   return(Object->nPnts);
}


static int numObjPolys(LWObject *Object)
{
   return(Object->nPols);
}


static int numObjSurfs(LWObject *Object)
{
   return(Object->nSurfs);
}


static double objBBox(LWObject *Object, double *box)
{
   int i;
   for(i=0; i<6; i++)
      box[i] = Object->bounds[i];
   return Object->rMax;
}




/////*****************************************************************************************************************/




static void polyNormal(ObjectAccess *acc, LWObjectID obj, LWPolygon *poly, LWPoint *norm)
{
   int      j;
   pntID *vl,va,vb;
   float len,x, y, z;
   LWPoint     *pt1,*pt2;
   vl = poly->plist;
   norm->x = norm->y = norm->z = 0.0F; /* Face normal */
   for (j=0;j<poly->npoints;j++)
    {
      va = vl[j];
      vb = ((j+1) == poly->npoints) ? vl[0] : vl[j+1];
      pt1 =  (*acc->pointGet)(obj,va);
      pt2 =  (*acc->pointGet)(obj,vb);
      x = (pt1->y - pt2->y)*(pt1->z + pt2->z);
        y = (pt1->z - pt2->z)*(pt1->x + pt2->x);
        z = (pt1->x - pt2->x)*(pt1->y + pt2->y);
      norm->x += x;
      norm->y += y;
      norm->z += z;
   }
   len = (float)sqrt(norm->x*norm->x+norm->y*norm->y+norm->z*norm->z);
   if ( (len > 0.0F) && (len != 1.0) )
    {    /* Normalize length */
       len=1.0F/len;
       norm->x *= len;
       norm->y *= len;
       norm->z *= len;
    }
}


static float STR_TO_FLOAT(unsigned char *st)
{
   unsigned char  one[4];
   one[0] = st[0];
   one[1] = st[1];
   one[2] = st[2];
   one[3] = st[3];
   return *((float *)one);
}


#define STR_IS_ID(st,a,b,c,d)  ( (st[0]==a) && (st[1]==b) && (st[2]==c) && (st[3]==d) )


static int ReadSURF(LWSurface *lwsur, surfAttr *surf)
{
   unsigned char *p,r,g,b,*databuf;
   short *size,*t,s;
   int len,max;
   char     *st;

   max = lwsur->size;
   databuf = lwsur->data;
   surf->su_tx.xTextureCenter = 0.0f;
   surf->su_tx.yTextureCenter = 0.0f;
   surf->su_tx.zTextureCenter = 0.0f;
   surf->su_tx.xTextureSize = 1.0f;
   surf->su_tx.yTextureSize = 1.0f;
   surf->su_tx.zTextureSize = 1.0f;
   surf->su_tx.textureAxis = TA_Z;
   surf->su_tx.textureType = TT_CUBIC;
   surf->su_tx.textureFlags = 0;
   surf->su_tx.wTiles = 1.0f;
   surf->su_tx.hTiles = 1.0f;
   surf->su_red = 0.8f;
   surf->su_green = 0.8f;
   surf->su_blue = 0.8f;
   surf->su_lumi = 0;
   surf->su_diff = 256; // 256 = 100%
   surf->su_spec = 0;
   surf->su_refl = 0;
   surf->su_tran = 0;
   surf->su_image = NULL;


   for(len=0; len<max; databuf +=(6 + s), len+=(6 + s) )
   {
      size=(short *)&(databuf[4]);
      s=*size;
      BSWAP_W(s);
      if( STR_IS_ID(databuf,'C','O','L','R') ) //if(!strncmp(data,"COLR",4))
      {
         p=&databuf[6]; // Skip COLR, 2-byte size
         r=*p++;
         surf->su_red=r;
         surf->su_red /= 255;
         g=*p++;
         surf->su_green=g;
         surf->su_green /= 255;
         b=*p++;
         surf->su_blue=b;
         surf->su_blue /= 255;
      }
      else if( STR_IS_ID(databuf,'F','L','A','G') ) //  if(!strncmp(data,"FLAG",4))
      {
         t=(short *)&(databuf[6]);
         surf->su_flags = (short)*t;
         BSWAP_W(surf->su_flags);
      }
      else if( STR_IS_ID(databuf,'S','M','A','N') ) //  if(!strncmp(data,"SMAN",4))
      {
         float f;
         /*at = (float *)(data+6);
         f = at[0];*/
         f = STR_TO_FLOAT( (&(databuf[6])) );
         BSWAP_L(f);
         surf->su_tx.smAngle = f;
      }
      else if( STR_IS_ID(databuf,'T','R','A','N') ) // if(!strncmp(data,"TRAN",4))
      {
         t=(short *)(databuf+6);
         surf->su_tran = *t;
         BSWAP_W(surf->su_tran);
      }
      else if( STR_IS_ID(databuf,'L','U','M','I') ) // if(!strncmp(data,"LUMI",4))
      {
         t=(short *)(databuf+6);
         surf->su_lumi = *t;
         BSWAP_W(surf->su_lumi);
      }
      else if( STR_IS_ID(databuf,'S','P','E','C') ) // if(!strncmp(data,"SPEC",4))
      {
         t=(short *)(databuf+6);
         surf->su_spec = *t;
         BSWAP_W(surf->su_spec);
      }
      else if( STR_IS_ID(databuf,'D','I','F','F') ) // if(!strncmp(data,"DIFF",4))
      {
         t=(short *)(databuf+6);
         surf->su_diff = *t;
         BSWAP_W(surf->su_diff);
      }
      else if( STR_IS_ID(databuf,'C','T','E','X') ) // if(!strncmp(data,"CTEX",4))
      {
         st=(char *)databuf+6;
         surf->su_tx.textureType = TT_PLANAR;

         if( STR_IS_ID(st,'P','l','a','n') )  // if(!strncmp(st,"Plan",4))
         {
            surf->su_tx.textureType = TT_PLANAR;
         }
         else if( STR_IS_ID(st,'C','y','l','i') )  // if(!strncmp(st,"Cyli",4))
         {
            surf->su_tx.textureType = TT_CYLINDRICAL;
         }
         else if( STR_IS_ID(st,'S','p','h','e') )  // if(!strncmp(st,"Sphe",4))
         {
            surf->su_tx.textureType = TT_SPHERICAL;
         }
         else if( STR_IS_ID(st,'C','u','b','i') )  // if(!strncmp(st,"Cubi",4))
         {
            surf->su_tx.textureType = TT_CUBIC;
         }
      }
      else if( STR_IS_ID(databuf,'T','I','M','G') ) // if(chnk=='TIMG') //if(!strncmp(databuf,"TIMG",4))
      {
         surf->su_image = (char *)(databuf+6);
      }
      else if( STR_IS_ID(databuf,'T','F','P','0') ) // if(chnk=='TFP0') //if(!strncmp(databuf,"TFP0",4))  // width tiling.. for spher,cyl
      {
         float f;
         //at = (float *)(databuf+6);
         //f = at[0];
         f = STR_TO_FLOAT( (&(databuf[6])) );
         BSWAP_L(f);
         surf->su_tx.wTiles = f;
      }
      else if( STR_IS_ID(databuf,'T','F','P','1') ) // if(chnk=='TFP1') //if(!strncmp(databuf,"TFP1",4))   // height tiling.. for spher
      {
         float f;
         //at = (float *)(databuf+6);
         //f = at[0];
         f = STR_TO_FLOAT( (&(databuf[6])) );
         BSWAP_L(f);
         surf->su_tx.hTiles = f;
      }
      else if( STR_IS_ID(databuf,'T','W','R','P') ) // if(!strncmp(data,"TWRP",4))
      {
         unsigned short *f;
         f=(unsigned short *)(databuf+6);
         if(*f!=0)
            surf->su_tx.textureFlags |= TXF_UDECAL;
         f++;
         if(*f!=0)
            surf->su_tx.textureFlags |= TXF_VDECAL;
      }


      else if( STR_IS_ID(databuf,'T','C','T','R') ) // if(!strncmp(data,"TCTR",4))
      {
         float *at,f;


         at = (float *)(databuf+6);


         f = STR_TO_FLOAT( (&(databuf[6])) );
         //f = at[0];
         BSWAP_L(f);
         surf->su_tx.xTextureCenter = f;
         f = STR_TO_FLOAT( (&(databuf[10])) );
         //f = at[1];
         BSWAP_L(f);
         surf->su_tx.yTextureCenter = f;
         f = STR_TO_FLOAT( (&(databuf[14])) );
         //f = at[2];
         BSWAP_L(f);
         surf->su_tx.zTextureCenter = f;
      }
      else if( STR_IS_ID(databuf,'T','S','I','Z') ) // if(chnk=='TSIZ') //if(!strncmp(databuf,"TSIZ",4))
      {
         float *at,f;
         at = (float *)(databuf+6);
         f = STR_TO_FLOAT( (&(databuf[6])) );
         //f = at[0];
         BSWAP_L(f);
         surf->su_tx.xTextureSize = f;
         f = STR_TO_FLOAT( (&(databuf[10])) );
         //f = at[1];
         BSWAP_L(f);
         surf->su_tx.yTextureSize = f;
         f = STR_TO_FLOAT( (&(databuf[14])) );
         //f = at[2];
         BSWAP_L(f);
         surf->su_tx.zTextureSize = f;
      }
      else if( STR_IS_ID(databuf,'T','F','L','G') ) // if(chnk=='TFLG') //if(!strncmp(databuf,"TFLG",4))
      {
         unsigned short f;
         t=(short *)(databuf+6);
         f= *t;
         BSWAP_W(f);
         surf->su_tx.textureFlags |= f;
         if(f&TXF_AXIS_X)
            surf->su_tx.textureAxis = TA_X;
         else if(f&TXF_AXIS_Y)
            surf->su_tx.textureAxis = TA_Y;
         else // if(*t&TXF_AXIS_Z)
            surf->su_tx.textureAxis = TA_Z;
   //    BSWAP_W(surf->su_tx.textureFlags);
      }
   }
   return(len);
}


#ifndef _XGL
__inline
#endif
static double fract(double x)
{
   if( (x>1.0f) || (x<0.0f) )
   {
      float t = (float)(x-floor(x)) ;
      return (t>1.0f ? 0.0f:t);
   }
   else return x;
}


static void xyztoh( float x, float y, float z, float *h )
{
   if ( x == 0.0 && z == 0.0 )
      *h = 0.0f;

   else {
      if ( z == 0.0 )
         *h = x < 0.0 ? HALFPI : -HALFPI;
      else
         *h = ( float ) -atan2( x, z );

      if ( *h < 0 ) *h += TWOPI;        /* 0 <= *h < 2 PI */
   }
}


static void xyztohp( float x, float y, float z, float *h, float *p )
{
   if ( x == 0.0 && z == 0.0 ) {
      *h = 0.0f;
      if ( y != 0.0 )
         *p = y < 0.0 ? -HALFPI : HALFPI;
      else
         *p = 0.0f;
    }

    else {
       *h = ( float ) -atan2( x, z );
       if ( *h < 0 ) *h += TWOPI;       /* 0 <= *h < 2 PI */

       x = ( float ) sqrt( x * x + z * z );
       if ( x == 0.0 )
          *p = y < 0.0 ? -HALFPI : HALFPI;
       else
          *p = ( float ) atan( y / x );
    }
}


static TxAxis CubicTextureAxis(float x, float y, float z)
{
   float f1,f2,f0;


   f0=(float)fabs(x) ;
   f1=(float)fabs(y) ;
   f2=(float)fabs(z) ;
   if(f0 < f1)
   {
      if(f2 < f1) return(TA_Y);
      else return(TA_Z);
   }
   else if(f2 < f0) return(TA_X);
   else return(TA_Z);
}


static void TextureUV(LWTexture *tx,LWPoint *spot,LWUV *uv, LWPoint *tnorm)
{
   float t,lon,lat;

   spot->x -= ( float ) tx->xTextureCenter;
   spot->y -= ( float ) tx->yTextureCenter;
   spot->z -= ( float ) tx->zTextureCenter;

   if ( tx->textureType == TT_CYLINDRICAL )
   {
      if ( tx->textureAxis == TA_X ) {
         xyztoh( spot->z, spot->x, -spot->y, &lon );
         t = ( float )( -spot->x / tx->xTextureSize + 0.5f );
      }
      else if ( tx->textureAxis == TA_Y ) {
         xyztoh( -spot->x, spot->y, spot->z, &lon );
         t = ( float )( -spot->y / tx->yTextureSize + 0.5f );
      }
      else {
         xyztoh( -spot->x, spot->z, -spot->y, &lon );
         t = ( float )( -spot->z / tx->zTextureSize + 0.5f );
      }

      while ( t < 0 ) t += 1.0f;

      /* --- lon is in [0, 2PI], so we need to change this
      lon = 1.0f - ( lon + PI ) / TWOPI;   [EW 6 Jun 00] --- */
      lon = 1.0f - lon / TWOPI;

      if ( tx->wTiles != 1.0f )
         lon = ( float ) fract( lon * tx->wTiles );

      uv->u = lon;
      uv->v = t;
   }

   else if ( tx->textureType == TT_SPHERICAL )
   {
      if ( tx->textureAxis == TA_X )
         xyztohp( spot->z, spot->x, -spot->y, &lon, &lat );

      else if ( tx->textureAxis == TA_Y )
         xyztohp( -spot->x, spot->y, spot->z, &lon, &lat );

      else
         xyztohp( -spot->x, spot->z, -spot->y, &lon, &lat );

      /* --- lon is in [0, 2PI], so we need to change this
      lon = 1.0f - ( lon + PI ) / TWOPI;   [EW 6 Jun 00] --- */
      lon = 1.0f - lon / TWOPI;

      lat = 0.5f - lat / PI;

      if ( tx->wTiles != 1.0f )
         lon = ( float ) fract( lon * tx->wTiles );
      if ( tx->hTiles != 1.0f )
         lat = ( float ) fract( lat * tx->hTiles );

      uv->u = ( lon );
      uv->v = ( lat );
   }

   else   // TT_CUBIC or TT_PLANAR
   {
      if ( tx->textureType == TT_CUBIC )
         tx->textureAxis = CubicTextureAxis( tnorm->x, tnorm->y, tnorm->z );

      uv->u = ( float )(( tx->textureAxis == TA_X ) ?
         ( spot->z / tx->zTextureSize ) + 0.5f :
         ( spot->x / tx->xTextureSize ) + 0.5f );

      uv->v = ( float )(( tx->textureAxis == TA_Y) ?
         ( -( spot->z / tx->zTextureSize )) + 0.5f :
         ( -( spot->y / tx->yTextureSize )) + 0.5f );
   }
}


static LWPoint *createNormals(ObjectAccess *acc, LWObjectID obj)
{
   int      j,v,nv,num_polys,num_points,*vdiv;
   polID i;
   pntID *vl,va,vb;
   float len,x, y, z;
   LWPolygon   *poly;
   LWPoint     *pt1,*pt2,normal,*Fnormals,*normals;
   num_polys = (*acc->polyCount)(obj);
   num_points = (*acc->pointCount)(obj);
   if(!num_points)
      return NULL;
   if( !(vdiv=malloc(num_points*sizeof(int))) )
      return NULL;
   if( !(normals=malloc(sizeof(LWPoint)*(num_points+num_polys))) )
   {
      free(vdiv);
      return normals;
   }
   Fnormals = &(normals[num_points]);
   for (i=0; i<num_points; i++)  /* zero out the normal array */
   {
      normals[i].x = 0.0F;
      normals[i].y = 0.0F;
      normals[i].z = 0.0F;
      vdiv[i] = 0;
   }
   for (i=0; i<num_polys; i++)
   {
      poly = (*acc->polyGet)(obj,i);
      nv=poly->npoints; //j is set to the number of points in the polygon to follow
      vl = poly->plist;
      normal.x = normal.y = normal.z = 0.0F; /* Face normal */
      for (j=0;j<nv;j++)
        {
         va = vl[j];
         vb = ((j+1) == nv) ? vl[0] : vl[j+1];
         pt1 =  (*acc->pointGet)(obj,va);
         pt2 =  (*acc->pointGet)(obj,vb);
         x = (pt1->y - pt2->y)*(pt1->z + pt2->z);
            y = (pt1->z - pt2->z)*(pt1->x + pt2->x);
            z = (pt1->x - pt2->x)*(pt1->y + pt2->y);
         normal.x += x;
         normal.y += y;
         normal.z += z;
      }
      len = (float)sqrt(normal.x*normal.x+normal.y*normal.y+normal.z*normal.z);
      if (len > 0.0F)
        {      /* Normalize length */
          len=1.0F/len;
           normal.x *= len;
           normal.y *= len;
          normal.z *= len;
        }
        Fnormals[i].x=normal.x;
      Fnormals[i].y=normal.y;
      Fnormals[i].z=normal.z;
        /* add normal to each vertex of this polygon */
      for (j=0; j<nv; j++)
        {
            v = vl[j];
            normals[v].x += normal.x;
         normals[v].y += normal.y;
         normals[v].z += normal.z;
         vdiv[v]++;
        }
    }
   for (i=0; i<num_points; i++)
   {   /* Now normalize vertex normals */
      if(vdiv[i])
      {
         len=1.0F/vdiv[i];
         normals[i].x *= len;
         normals[i].y *= len;
         normals[i].z *= len;
         len = (float)sqrt(normals[i].x*normals[i].x+normals[i].y*normals[i].y+normals[i].z*normals[i].z);
         if (len > 0.0F)
         {     /* Normalize length */
            len=1.0F/len;
            normals[i].x *= len;
            normals[i].y *= len;
            normals[i].z *= len;
         }
      }
   }
   free(vdiv);
   return normals;
}


static LWUV *createUVs(ObjectAccess *acc, LWObjectID obj, LWPoint *normals, LWTexture *tex)
{
   int num_points;
   pntID i;
   LWPoint *temp_p, *temp_normals;
   LWUV *temp_uvs;
    temp_normals=normals;
   num_points = (*acc->pointCount)(obj);
   if( !(temp_uvs = malloc(sizeof(LWUV)*num_points)) )
      return temp_uvs;
    for(i=0;i<num_points;i++)
    {
      temp_p =  (*acc->pointGet)(obj,i);
      //temp_p->y = -temp_p->y;

/* Changed the following two lines.  Can't increment temp_uvs if we're going to
   use it as the return value.  EW 28 Jan 00

      TextureUV(tex,temp_p,temp_uvs,temp_normals);
      temp_uvs++;
*/
      TextureUV(tex,temp_p,&temp_uvs[i],temp_normals);
      temp_normals++;
    }
   return temp_uvs;
}


static void destroyArray(void *array)
{
   if(array) free(array);
}
/////*****************************************************************************************************************




static ObjectAccess  MyObjAcc = {
   createLWObject, destroyLWObject,
   numObjPoints, numObjPolys, numObjSurfs,
   getObjPoint, getObjPoly, getObjSurf, objBBox
};


static ObjectHelp  MyObjectHelp = {  polyNormal, createNormals, createUVs, ReadSURF, TextureUV, destroyArray   };


#ifndef LW_OBJECT_STATIC_LIB


XCALL_(int)HelpActivate (
 long                 version,
 GlobalFunc           *global,
 GlobalService        *local,
 void                 *serverData)
{
   XCALL_INIT;
   if(strcmp(local->id,OBJECT_HELP_NAME))
      return AFUNC_BADLOCAL;
   local->data = (void *)&MyObjectHelp;
   return AFUNC_OK;
}


XCALL_(int)AccessActivate (
 long                 version,
 GlobalFunc           *global,
 GlobalService        *local,
 void                 *serverData)
{
   XCALL_INIT;
   if(strcmp(local->id,OBJECT_ACCESS_NAME))
      return AFUNC_BADLOCAL;
   local->data = (void *)&MyObjAcc;
   return AFUNC_OK;
}


ServerRecord ServerDesc[] = {
   { "Global", OBJECT_ACCESS_NAME,  AccessActivate },
   { "Global", OBJECT_HELP_NAME, HelpActivate },
   { NULL }
};


#else
// Static library entry points, easy porting at least..
ObjectAccess   *GlobalObjectAccess =   &MyObjAcc;
ObjectHelp     *GlobalObjectHelp =  &MyObjectHelp;
#endif
