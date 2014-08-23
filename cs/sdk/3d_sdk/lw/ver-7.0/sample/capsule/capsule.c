/* Capsule Primitive Tool -- A Plugin for LightWave Modeler
 * Arnie Cachelin, Copyright 1999, NewTek, Inc.
 * Like a disc only rounder, like a ball, only longer
 */


#include <lwsdk/lwserver.h>
#include <lwsdk/lwmodtool.h>
#include <lwsdk/lwxpanel.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#define  REVISION 1.008


#ifndef PI
 #define PI     (3.1415926536)
#endif
#ifndef TWOPI
 #define TWOPI  (2.0 * PI)
#endif
#ifndef HALFPI
 #define HALFPI (0.5 * PI)
#endif
#define INVROOT2   0.707106781
#define VCPY(a,b)       ((a)[0] =(b)[0], (a)[1] =(b)[1], (a)[2] =(b)[2])
#define VCPY_F(a,b)     ((a)[0] =(float)((b)[0]), (a)[1] =(float)((b)[1]), (a)[2] =(float)((b)[2]))
#define VSCL(a,x)       ((a)[0]*= (x),   (a)[1]*= (x),   (a)[2]*= (x))
#define VADD(a,b)       ((a)[0]+=(b)[0], (a)[1]+=(b)[1], (a)[2]+=(b)[2])
#define VSUB(a,b)       ((a)[0]-=(b)[0], (a)[1]-=(b)[1], (a)[2]-=(b)[2])
#define VADD3(r,a,b)    ((r)[0]=(a)[0]+(b)[0], (r)[1]=(a)[1]+(b)[1], (r)[2]=(a)[2]+(b)[2])
#define VSUB3(r,a,b)    ((r)[0]=(a)[0]-(b)[0], (r)[1]=(a)[1]-(b)[1], (r)[2]=(a)[2]-(b)[2])

#define ABS(a) ((a < 0) ? (-(a)) : (a))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define CLAMP(a,b,c) (((a) < (b)) ? (b) : (((a) > (c)) ? (c) : (a)))

#define SWAP(a,b) { a^=b; b^=a; a^=b; }

static LWXPanelFuncs *globFun_pan = NULL;

#define MAX_SEGS  24
#define MIN_SEGS  2
#define MAX_TSEGS 64
#define MIN_TSEGS 1
#define MAX_SIDES 64
#define MIN_SIDES 3
#define MIN_ASPECT   3 // min ratio of len to radius
#define MAX_POINTS   (2*MAX_SEGS*MAX_SIDES+2 + 2*MAX_SIDES)
#define CAPHAND_CENTER  8

typedef struct st_CapsuleTool {
   LWFVector    top[4], bot[4];
   LWDVector    center;
   double       rad, len;
   int          axis, sides, capSegs, tubeSegs, uvs;
   int          active;
   int          dirty, update;
} CapsuleTool;

// EZ method for cyclic permutation of axes
static int xax[] = {1,2,0}, yax[]={2,0,1}, zax[] = {0,1,2};

static LWPntID plist[MAX_POINTS+1];

/* This could be much more efficient if it cached a circle of sin() and cos(),
   and/or mirrored the top to the bottom.
*/
int AddCapsule(MeshEditOp *op,CapsuleTool *tool)
{
   int             p=0,v,n, x, y, z,pnum=0;
   LWPntID         poly[4];
   LWDVector       pt;
   double          scale, theta, phi,dt,dp,du,len;
   float        uv[2] = {0.0f};
   z = tool->axis;
   x = xax[tool->axis];
   y = yax[tool->axis];

   if(tool->len==0.0)
      return 0;
   dp = HALFPI/(tool->capSegs);
   du = 1.0/(tool->sides);
   dt = TWOPI*du;
   len = (tool->len*0.5) - tool->rad;

/* Top Of Capsule */
   VCPY(pt,tool->center);
   pt[z] += tool->len*0.5;
   if(tool->uvs)
   {
      uv[0] = 0.0f;
      uv[1] = 1.0;
      (*op->initUV)(op->state, uv );
   }
   plist[0] = (*op->addPoint)(op->state, pt );
   v = 1;
   for(n=0, phi = HALFPI - dp; n<tool->capSegs; n++, phi -= dp)
   {
      scale = cos(phi);
      pt[z] = len + tool->rad*sin(phi);
      uv[1] = (float)((pt[z] + (tool->len*0.5))/tool->len);
      pt[z] += tool->center[z];
      for(p=0,theta = 0.0; p<tool->sides; p++, theta+=dt)
      {
         pt[x] = tool->center[x] + tool->rad*scale*cos(theta);
         pt[y] = tool->center[y] + tool->rad*scale*sin(theta);
         if(p==MAX_POINTS+1)
            return 0;
         if(tool->uvs)
         {
            uv[0] = (float)(p*du);
            (*op->initUV)(op->state, uv );
         }
         plist[v] = (*op->addPoint)(op->state, pt );
         v++;
      }
   }
   pnum = v;

   poly[0] = plist[0];
   for(p=1; p<tool->sides; p++)
   {
      (*op->addTri)(op->state,plist[0], plist[p],  plist[p+1]);
   }
   (*op->addTri)(op->state,plist[0], plist[p],  plist[1]);
   v = 1;
   for(n=1; n<tool->capSegs; n++)
   {
      for(p=0; p<tool->sides-1; p++)
      {
         (*op->addQuad)(op->state,plist[v+p+1], plist[v+p],  plist[v + tool->sides + p],  plist[v + tool->sides + p+1]);
      }
      (*op->addQuad)(op->state,plist[v], plist[v+p],  plist[v + tool->sides + p],  plist[v + tool->sides]);
      v += tool->sides;
   }

/* Bottom Of Capsule */
   v = pnum;
   VCPY(pt,tool->center);
   pt[z] -= tool->len*0.5;
   if(tool->uvs)
   {
      uv[0] = 0.0f;
      uv[1] = 0.0f;
      (*op->initUV)(op->state, uv );
   }
   plist[v++] = (*op->addPoint)(op->state, pt );
   for(n=0, phi = HALFPI - dp; n<tool->capSegs; n++, phi -= dp)
   {
      scale = cos(phi);
      pt[z] = -len - tool->rad*sin(phi);
      uv[1] = (float)((pt[z] + (tool->len*0.5))/tool->len);
      pt[z] += tool->center[z];
      for(p=0,theta = 0.0; p<tool->sides; p++, theta+=dt)
      {
         pt[x] = tool->center[x] + tool->rad*scale*cos(theta);
         pt[y] = tool->center[y] + tool->rad*scale*sin(theta);
         if(tool->uvs)
         {
            uv[0] = (float)(p*du);
            (*op->initUV)(op->state, uv );
         }
         plist[v] = (*op->addPoint)(op->state, pt );
         v++;
      }
   }
   poly[0] = plist[pnum];
   for(p=1; p<tool->sides; p++)
   {
      (*op->addTri)(op->state,plist[pnum], plist[pnum+p+1],  plist[pnum+p]);
   }
   (*op->addTri)(op->state,plist[pnum], plist[pnum+1],  plist[pnum+p]);
   v = pnum+1;
   for(n=1; n<tool->capSegs; n++)
   {
      for(p=0; p<tool->sides-1; p++)
      {
         (*op->addQuad)(op->state,plist[v+p] ,plist[v+p+1],  plist[v + tool->sides + p+1],  plist[v + tool->sides + p]);
      }
      (*op->addQuad)(op->state,plist[v+p], plist[v], plist[v + tool->sides], plist[v + tool->sides + p]);
      v += tool->sides;
   }

   v = pnum - tool->sides;
   pt[z] = tool->center[z] + len;
   len = (len+len)/tool->tubeSegs;
   for(n=1; n<tool->tubeSegs; n++)
   {
      pt[z] -= len;
      uv[1] = (float)( (pt[z] - (tool->center[z] - tool->len*0.5)) /tool->len);
      for(p=0,theta = 0.0; p<tool->sides; p++, theta+=dt)
      {
         pt[x] = tool->center[x] + tool->rad*cos(theta);
         pt[y] = tool->center[y] + tool->rad*sin(theta);
         if(tool->uvs)
         {
            uv[0] = (float)(p*du);
            (*op->initUV)(op->state, uv );
         }
         if(n==1)
            plist[v+pnum+2*tool->sides+p] = plist[v+p];
         else
            plist[v+pnum+2*tool->sides+p] = plist[v+pnum+tool->sides+p];
         plist[v+pnum+tool->sides+p] = (*op->addPoint)(op->state, pt );
      }
      for(p=0; p<tool->sides-1; p++)
         (*op->addQuad)(op->state,plist[v+pnum+2*tool->sides+p+1], plist[v+pnum+2*tool->sides+p], plist[v + pnum+tool->sides+p], plist[v + pnum+tool->sides + p+1]);
      (*op->addQuad)(op->state,plist[v+pnum+2*tool->sides], plist[v+pnum+2*tool->sides+p], plist[v + pnum+tool->sides+p], plist[v + tool->sides+pnum]);
   }

   v = pnum - tool->sides;
   n = tool->tubeSegs>1 ? tool->sides+pnum:0;
   for(p=0; p<tool->sides-1; p++)
      (*op->addQuad)(op->state,plist[v+p+n+1], plist[v+p+n], plist[v + pnum+p], plist[v + pnum + p+1]);
   (*op->addQuad)(op->state,plist[v+n], plist[v+p+n], plist[v + pnum+p], plist[v + pnum]);

   return(pnum+pnum);
}

/*
 * The 'build' function is called, on the request of the tool, to recompute
 * the new geometry whenever the parameters of the operation have changed.
 * Every time it runs, it resets the update state for the tool.
 */
   static LWError
Capsule_Build (
   CapsuleTool    *tool,
   MeshEditOp     *op)
{
   int          n=0;

   if (!tool->active)
      return NULL;

   n = AddCapsule(op, tool);
   tool->update = LWT_TEST_NOTHING;
   return (n ==0 ? "Failed" : NULL);
}


/*
 * The test function returns the mesh update action for the tool.  Mostly
 * this will be NOTHING, but sometimes there will be a need to advance to a
 * new state.  This will always result in a call to 'build' which will
 * reset the update state to NOTHING again.
 */
   static int
Capsule_Test (
   CapsuleTool    *tool)
{
   return tool->update;
}


/*
 * End is called when the interactive build is complete, either by the
 * direct request of the tool itself, or by the implicit action of some
 * other aspect of the application.
 */
   static void
Capsule_End (
   CapsuleTool    *tool,
   int          keep)
{
   tool->active  = 0;
   tool->update = LWT_TEST_NOTHING;
}


// Fill bounding box corners from center, len, radius
void setCapsuleBoxCorners(CapsuleTool  *tool)
{
   int x, y, z;
   z = tool->axis;
   x = xax[tool->axis];
   y = yax[tool->axis];

   VCPY_F( (tool->top[0]), (tool->center) );
   VCPY_F( (tool->bot[0]), (tool->center) );
   tool->top[0][z] += (float)(0.5*tool->len);
   tool->bot[0][z] -= (float)(0.5*tool->len);
   VCPY( (tool->top[1]),  (tool->top[0]));
   VCPY( (tool->bot[1]),  (tool->bot[0]));
   VCPY( (tool->top[2]),  (tool->top[0]));
   VCPY( (tool->bot[2]),  (tool->bot[0]));
   VCPY( (tool->top[3]),  (tool->top[0]));
   VCPY( (tool->bot[3]),  (tool->bot[0]));

   tool->top[0][x] += (float)tool->rad;
   tool->bot[0][x] += (float)tool->rad;
   tool->top[0][y] += (float)tool->rad;
   tool->bot[0][y] += (float)tool->rad;

   tool->top[1][x] += (float)tool->rad;
   tool->bot[1][x] += (float)tool->rad;
   tool->top[1][y] -= (float)tool->rad;
   tool->bot[1][y] -= (float)tool->rad;

   tool->top[2][x] -= (float)tool->rad;
   tool->bot[2][x] -= (float)tool->rad;
   tool->top[2][y] -= (float)tool->rad;
   tool->bot[2][y] -= (float)tool->rad;

   tool->top[3][x] -= (float)tool->rad;
   tool->bot[3][x] -= (float)tool->rad;
   tool->top[3][y] += (float)tool->rad;
   tool->bot[3][y] += (float)tool->rad;

}

/* Set / reset default values */
void resetCapsule(CapsuleTool *tool)
{
   tool->axis = 2;
   tool->uvs = 0;
   tool->len = 2.0;
   tool->rad = 0.5;
   tool->capSegs = 8;
   tool->tubeSegs = 8;
   tool->sides = 24;
   tool->update = LWT_TEST_UPDATE;
   setCapsuleBoxCorners(tool);
}


   static void
Capsule_Draw (
   CapsuleTool    *tool,
   LWWireDrawAccess  *draw)
{
   LWFVector   top[4], bot[4],del;
   int x, y, z;

   if (!tool->active)
      return;

   z = tool->axis;
   x = xax[tool->axis];
   y = yax[tool->axis];

   (*draw->moveTo) (draw->data, tool->top[0], 1);
   (*draw->lineTo) (draw->data, tool->top[1], 0);
   (*draw->lineTo) (draw->data, tool->top[2], 0);
   (*draw->lineTo) (draw->data, tool->top[3], 0);
   (*draw->lineTo) (draw->data, tool->top[0], 0);

   (*draw->moveTo) (draw->data, tool->bot[0], 1);
   (*draw->lineTo) (draw->data, tool->bot[1], 0);
   (*draw->lineTo) (draw->data, tool->bot[2], 0);
   (*draw->lineTo) (draw->data, tool->bot[3], 0);
   (*draw->lineTo) (draw->data, tool->bot[0], 0);

   (*draw->lineTo) (draw->data, tool->top[0], 0);

   (*draw->moveTo) (draw->data, tool->top[3], 1);
   (*draw->lineTo) (draw->data, tool->bot[3], 0);

   (*draw->moveTo) (draw->data, tool->bot[2], 1);
   (*draw->lineTo) (draw->data, tool->top[2], 0);

   (*draw->moveTo) (draw->data, tool->top[1], 1);
   (*draw->lineTo) (draw->data, tool->bot[1], 0);

   VSUB3(del, tool->top[0], tool->bot[2]);
   VSCL(del,0.25f); // center
   VCPY_F(top[0],tool->center);
   top[0][z] += del[z];
   VCPY_F(bot[0],tool->center);
   bot[0][z] -= del[z];
   VCPY_F(top[1],tool->center);
   top[1][x] += del[x];
   VCPY_F(bot[1],tool->center);
   bot[1][x] -= del[x];
   VCPY_F(top[2],tool->center);
   top[2][y] += del[y];
   VCPY_F(bot[2],tool->center);
   bot[2][y] -= del[y];
   (*draw->moveTo) (draw->data, top[0], 0);
   (*draw->lineTo) (draw->data, bot[0], 0);
   (*draw->moveTo) (draw->data, top[1], 0);
   (*draw->lineTo) (draw->data, bot[1], 0);
   (*draw->moveTo) (draw->data, top[2], 0);
   (*draw->lineTo) (draw->data, bot[2], 0);
   tool->dirty = 0;
}


   static int
Capsule_Dirty (
   CapsuleTool    *tool)
{
   return tool->dirty;
}


   static int
Capsule_Count (
   CapsuleTool    *tool,
   LWToolEvent    *event)
{
   //return (tool->active ? 14 : 0);
   return (tool->active ? 9 : 0);
}

   static int
Capsule_Start (
   CapsuleTool    *tool,
   LWToolEvent    *event)
{
   if(!tool->active)
      tool->active = 1;
   VCPY_F(tool->center, event->posSnap);
   if(event->portAxis>=0)
      tool->axis = event->portAxis;
   setCapsuleBoxCorners(tool);
   return 0;
}

   static const char *
Capsule_Help (
   CapsuleTool    *tool,
   LWToolEvent    *event)
{
   static char    *axes="XYZ",buf[80];
   if(tool->active)
      sprintf(buf,"Capsule: %0.3f x %0.3f, %c axis",tool->len, tool->rad, axes[tool->axis]);
   else
      sprintf(buf,"Capsule: ");
   return buf;
}

   static int
Capsule_Handle (
   CapsuleTool    *tool,
   LWToolEvent    *event,
   int          i,
   LWDVector       pos)
{
   if(i<=3)
   {
      VCPY(pos,tool->top[i]);
/*    VADD3(pos,tool->top[i],tool->bot[i]);
      VSCL(pos,0.5); */
   }
   else if(i<=7)
   {
      VCPY(pos,tool->bot[i-4]);
//    VADD3(pos,tool->top[i-4],tool->bot[i-4]);
//    VSCL(pos,0.5);
   }
   else if(i==CAPHAND_CENTER)
      VCPY(pos,tool->center);
/* else if(i<=10)
   {
      VCPY(pos,tool->center);
      i -= 9; // -1,0,1
      pos[tool->axis] += i*0.5*tool->len;
   }
   else if(i<=14)
   {
      VADD3(pos,tool->top[i-11],tool->bot[i-11]);  // avg = center of edge
      VSCL(pos,0.5);
   } */
   return 1;
}


   static int
Capsule_Adjust (
   CapsuleTool    *tool,
   LWToolEvent    *event,
   int          i)
{
   int         x,y,z,j;
   LWDVector   pos;
   LWFVector  *handle;

   x = xax[tool->axis];
   y = yax[tool->axis];
   z = tool->axis;
   VCPY(pos,event->posSnap);

   handle = i<4 ? &(tool->top[i]): (i<8 ? &(tool->bot[i-4]):NULL);
   if(event->flags&LWTOOLF_CONSTRAIN)
      if(event->portAxis>=0)
      {
         int      tx,ty;
         tx = xax[event->portAxis];
         ty = yax[event->portAxis];
         if(event->flags&LWTOOLF_CONS_Y)
            pos[tx] = handle ? (double)((*handle)[tx]) : tool->center[tx];
         else if(event->flags&LWTOOLF_CONS_X)
            pos[ty] = handle ? (double)((*handle)[ty]) : tool->center[ty];
      }
   if(i<=7)
   {
      VSUB(pos,tool->center);
      tool->len = 2*fabs(pos[z]);
      tool->rad = INVROOT2*sqrt(pos[x]*pos[x] + pos[y]*pos[y]);
      if(tool->len<MIN_ASPECT*tool->rad)
         tool->len = MIN_ASPECT*tool->rad;
      setCapsuleBoxCorners(tool);
   }
   else if(i==CAPHAND_CENTER)
   {
      LWDVector dif;
      VSUB3(dif,pos,tool->center);
      for(j=0;j<4;j++)
      {
         tool->top[j][0] += (float)dif[0];
         tool->top[j][1] += (float)dif[1];
         tool->top[j][2] += (float)dif[2];
         tool->bot[j][0] += (float)dif[0];
         tool->bot[j][1] += (float)dif[1];
         tool->bot[j][2] += (float)dif[2];
      }
      VCPY_F(tool->center, pos);
   }
/* else if(i<=10)// z handles 8,10
   {
   // tool->len = 2*ABS(event->posSnap[tool->axis] - tool->center[tool->axis]);
   // setCapsuleBoxCorners(tool);
      tool->capSegs += (int)(2*event->deltaSnap[tool->axis]/tool->len);
      tool->capSegs = CLAMP(tool->capSegs, MIN_SEGS,MAX_SEGS);
   }
   else if(i<=14) // edge handles
   {
      tool->tubeSegs += (int)(2*event->deltaSnap[tool->axis]/tool->len);
      tool->tubeSegs = CLAMP(tool->tubeSegs, MIN_TSEGS,MAX_TSEGS);
      tool->sides += (int)((event->deltaSnap[x]+event->deltaSnap[y])/tool->len);
      tool->sides = CLAMP(tool->sides, MIN_SIDES,MAX_SIDES);
   } */
   tool->dirty = LWT_DIRTY_WIREFRAME|LWT_DIRTY_HELPTEXT;
   tool->update = LWT_TEST_UPDATE;
   return i;
}

   static void
Capsule_Event (
   CapsuleTool    *tool,
   int          code)
{
   switch (code) {
       case LWT_EVENT_DROP:
      tool->update = LWT_TEST_REJECT;
      break;
   case LWT_EVENT_RESET:
      resetCapsule(tool);
      break;
   case LWT_EVENT_ACTIVATE:
      tool->update = LWT_TEST_UPDATE;
      tool->dirty = LWT_DIRTY_WIREFRAME|LWT_DIRTY_HELPTEXT;
      tool->active = 1;
      break;
   }
}


   static void
Capsule_Destroy (
   CapsuleTool    *tool)
{
   free (tool);
}


#define XID_AXIS  LWID_('a','x','i','i')
#define XID_CENT  LWID_('c','e','n','t')
#define XID_RADI  LWID_('r','a','d','i')
#define XID_LONG  LWID_('l','o','n','g')
#define XID_SIDS  LWID_('s','i','d','s')
#define XID_CSEG  LWID_('c','s','e','g')
#define XID_TSEG  LWID_('t','s','e','g')
#define XID_TXUV  LWID_('t','x','u','v')
#define XID_ACTI  LWID_('a','c','t','i')

   static void *
Capsule_Get (
   CapsuleTool    *tool,
   unsigned long      vid)
{
   switch (vid )
   {
      case XID_AXIS:
         return &tool->axis;
      case XID_RADI:
         return &tool->rad;
      case XID_LONG:
         return &tool->len;
      case XID_TSEG:
         return &tool->tubeSegs;
      case XID_CSEG:
         return &tool->capSegs;
      case XID_SIDS:
         return &tool->sides;
      case XID_CENT:
         return &tool->center;
      case XID_TXUV:
         return &tool->uvs;
      case XID_ACTI:
         return &tool->active;
   }
   return NULL;
}

   static int
Capsule_Set (
   CapsuleTool    *tool,
   unsigned long      vid,
   void        *value)
{
   int         *i = (int*)value;
   double      *v = (double*)value;
   LWDVector   *vec = (LWDVector*)value;
   switch (vid )
   {
      case XID_AXIS:
         tool->axis = i[0];
         tool->axis = CLAMP(tool->axis,0,2);
         break;
      case XID_RADI:
         tool->rad = v[0];
         break;
      case XID_LONG:
         tool->len = v[0];
         if(tool->len<MIN_ASPECT*tool->rad)
            tool->len = MIN_ASPECT*tool->rad;
         break;
      case XID_CSEG:
         tool->capSegs = i[0];
         tool->capSegs = CLAMP(tool->capSegs,MIN_SEGS,MAX_SEGS);
         break;
      case XID_TSEG:
         tool->tubeSegs = i[0];
         break;
      case XID_SIDS:
         tool->sides = i[0];
         tool->sides = CLAMP(tool->sides,MIN_SIDES,MAX_SIDES);
         break;
      case XID_CENT:
         VCPY(tool->center,(*vec));
         break;
      case XID_TXUV:
         tool->uvs = i[0];
         break;
      default:
         return 0;
   }

   tool->dirty = LWT_DIRTY_WIREFRAME|LWT_DIRTY_HELPTEXT;
   tool->update = LWT_TEST_UPDATE;
   setCapsuleBoxCorners(tool);
   return 1;
}

   static const char *AxisNames[] = {"X","Y","Z",NULL};
   static LWXPanelID
Capsule_Panel (
   CapsuleTool    *tool)
{
   static LWXPanelDataDesc  def[] = {
      { XID_AXIS,  "Axis", "integer" },
      { XID_CENT,  "Center",  "distance3" },
      { XID_RADI,  "Radius",  "distance" },
      { XID_LONG,  "Length",  "distance" },
      { XID_SIDS,  "Sides",   "integer" },
      { XID_CSEG,  "Segments","integer" },
      { XID_TSEG,  "Divisions","integer" },
      { XID_TXUV,  "Make UVs","integer" },
      { XID_ACTI,  "--hidden--", "integer" },
      { 0 }
   };
   static LWXPanelControl ctl[] = {
      { XID_AXIS,  "Axis", "axis" },
      { XID_CENT,  "Center",  "distance3" },
      { XID_RADI,  "Radius",  "distance" },
      { XID_LONG,  "Length",  "distance" },
      { XID_SIDS,  "Sides",   "integer" },
      { XID_CSEG,  "Segments","integer" },
      { XID_TSEG,  "Divisions","integer" },
      { XID_TXUV,  "Make UVs","iBoolean" },
      { XID_ACTI,  "--hidden--", "integer" },
      { 0 }
   };
   static LWXPanelHint hint[] = {
      XpDELETE     (XID_ACTI),
      XpMIN(XID_CSEG, MIN_SEGS),
      XpMAX(XID_CSEG, MAX_SEGS),
      XpMAX(XID_SIDS, MAX_SIDES),
      XpMIN(XID_SIDS, MIN_SIDES),
      XpMIN(XID_TSEG, MIN_TSEGS),
      XpMAX(XID_TSEG, MAX_TSEGS),
      XpENABLEMSG_ (XID_ACTI, "Tool is currently inactive."),
         XpH  (XID_AXIS),
         XpH  (XID_CENT),
         XpH  (XID_RADI),
         XpH  (XID_SIDS),
         XpH  (XID_TXUV),
         XpH  (XID_LONG),
         XpH  (XID_CSEG),
         XpH  (XID_TSEG),
         XpEND,
      XpEND
   };
   LWXPanelID      pan;

   pan = (*globFun_pan->create) (LWXP_VIEW, ctl);
   if (!pan)
      return NULL;

   (*globFun_pan->hint) (pan, 0, hint);
   (*globFun_pan->describe) (pan, def, Capsule_Get, Capsule_Set);
   return pan;
}




   XCALL_(int)
CapsuleActivate (
   long         version,
   GlobalFunc     *global,
   LWMeshEditTool    *local,
   void        *serverData)
{
   CapsuleTool    *tool;
   XCALL_INIT;
   if (version != LWMESHEDITTOOL_VERSION)
      return AFUNC_BADVERSION;

   globFun_pan = (*global) (LWXPANELFUNCS_GLOBAL, GFUSE_TRANSIENT);

   tool = malloc (sizeof (CapsuleTool));
   if (!tool)
      return AFUNC_OK;

   memset (tool, 0, sizeof (*tool));
   resetCapsule(tool);
   tool->update = LWT_TEST_NOTHING;

   local->instance     = tool;
   local->tool->done   = Capsule_Destroy;
   local->tool->draw   = Capsule_Draw;
   local->tool->count  = Capsule_Count;
   local->tool->start  = Capsule_Start;
   local->tool->handle = Capsule_Handle;
   local->tool->adjust = Capsule_Adjust;
   local->tool->dirty  = Capsule_Dirty;
   local->tool->event  = Capsule_Event;
   local->tool->help   = Capsule_Help;
   if (globFun_pan)
      local->tool->panel  = Capsule_Panel;
   local->build        = Capsule_Build;
   local->test         = Capsule_Test;
   local->end          = Capsule_End;

   return AFUNC_OK;
}

ServerRecord      ServerDesc[] = {
   {LWMESHEDITTOOL_CLASS, "Capsule", CapsuleActivate},
   { NULL }
};
