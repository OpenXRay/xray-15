/*
======================================================================
inertia.c

VMAP-Based Secondary Animation Displacement

by Arnie Cachelin Copyright 1999 NewTek, Inc.
Cosmetic enhancements by Ernie Wright
A remake of the venerable Lazypoints, by Allen Hastings
====================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <lwhost.h>
#include <lwserver.h>
#include <lwhandler.h>
#include <lwmeshes.h>
#include <lwdisplce.h>
#include <lwenvel.h>
#include <lwvparm.h>
#include <lwmath.h>


/* version number for loading and saving */

#define INERTIA_VERSION 1


/* some globals */

static LWObjectFuncs   *objf     = NULL;
static LWEnvelopeFuncs *envf     = NULL;
static LWChannelInfo   *chanf    = NULL;
static LWItemInfo      *iteminfo = NULL;
static LWVParmFuncs    *vparmf   = NULL;

/* our instance data */

typedef struct st_InertiaData {
   LWTime      time;          // current time
   LWFrame     frame;            // current frame
   LWItemID    item, pivot;         // self and optional pivot
   LWVParmID   lagParm;          // Enveloped parameter: lag, seconds of delay per unit of offset
   double      lagRate;          // cached lag value
   LWDVector   pivpos;           // cached center
   int         flags;            // use INF_ flag bit definitions
   int         vmapIdx, pvid;    // index into list
   void       *vmId;          // vmap id cached
   char        vmName[ 80 ];     // vmap name!
   char        desc[ 80 ];       // Your Message Here!
} InertiaData;

#define INF_PIVLOCAL 1

/* Someday these may be translated */
#define STR_Pivot_TEXT     "Inertia Pivot Object"
#define STR_Local_TEXT     "Local Pivot"
#define STR_Fall_TEXT      "Lag Rate (s/m)"
#define STR_Type_TEXT      "Weight Map"

/*
======================================================================
popCnt_VMAP()

Return the number of weight maps, plus 1 for "(none)".  An xpanel
callback for the vmap popup list, also used by Load().
====================================================================== */

XCALL_( static int )
popCnt_VMAP( void *data )
{
   return 1 + objf->numVMaps( LWVMAP_WGHT );
}


/*
======================================================================
popName_VMAP()

Return the name of a vmap, given an index.  An xpanel callback for the
vmap popup list, also used by Load() and Save().
====================================================================== */

XCALL_( static const char * )
popName_VMAP( void *data, int idx )
{
   if ( idx == 0 ) return "(none)";

   return objf->vmapName( LWVMAP_WGHT, idx - 1 );
}


/*
======================================================================
popCnt_Item()

Return the number of objects, plus 1 for "(none)".  An xpanel
callback for the pivot object popup list.
====================================================================== */

XCALL_( static int )
popCnt_Item( void *data )
{
   int n = 1;
   LWItemID id;

   id = iteminfo->first( LWI_OBJECT, NULL );
   while ( id ) {
      n++;
      id = iteminfo->next( id );
   }

   return n;
}


/*
======================================================================
popName_Item()

Return the name of an object, given an index.  An xpanel callback for
the pivot object popup list.
====================================================================== */

XCALL_( static const char * )
popName_Item( void *data, int idx )
{
   const char *a;
   int i = 0;
   LWItemID id;

   if ( idx == 0 ) return "(none)";

   idx--;

   if ( id = iteminfo->first( LWI_OBJECT, NULL ) )
    a = iteminfo->name( id );

   while ( id && i < idx ) {
      i++;
      if ( id = iteminfo->next( id ))
         a = iteminfo->name( id );
   }

   return a;
}


/*
======================================================================
popItemID()

Return the item ID of an object, given an index.
====================================================================== */

static LWItemID popItemID( int idx )
{
   LWItemID id;
   int i = 0;

   if ( idx <= 0 ) return NULL;
   idx--;

   id = iteminfo->first( LWI_OBJECT, NULL );
   while ( id && i < idx ) {
      i++;
      id = iteminfo->next( id );
   }

   return id;
}

/*
======================================================================
Create()

Handler callback.  Allocate and initialize instance data.
====================================================================== */

XCALL_( static LWInstance )
Create( void *priv, LWItemID item, LWError *err )
{
   InertiaData *dat;
   LWChanGroupID cgroup;

   if ( dat = calloc( 1, sizeof( InertiaData ))) {
      dat->item = item;
     dat->vmName[0] = 0;
      if ( dat->lagParm = vparmf->create( LWVP_FLOAT, LWVPDT_NOTXTR )) {
         cgroup = iteminfo->chanGroup( dat->item );
         vparmf->setup( dat->lagParm, "LagRate", cgroup,
                                    NULL, NULL, NULL, NULL );
      }
   }

   return dat;
}


/*
======================================================================
Destroy()

Handler callback.  Free resources allocated by Create().
====================================================================== */

XCALL_( static void )
Destroy( InertiaData *dat )
{
   if( dat ) {
      if ( dat->lagParm )
         vparmf->destroy( dat->lagParm );
      free( dat );
   }
}


/*
======================================================================
Copy()

Handler callback.  Copy instance data.  If your instance data contains
allocated resources, note that a simple *to = *from is insufficient.
====================================================================== */

XCALL_( static LWError )
Copy( InertiaData *to, InertiaData *from )
{
   LWItemID id;
   LWVParmID vpid;

   id = to->item;
   vpid = to->lagParm;
   *to = *from;
   to->item = id;
   to->lagParm = vpid;
   return vparmf->copy( to->lagParm, from->lagParm );
}


/*
======================================================================
Load()

Handler callback.  Read instance data.
====================================================================== */

XCALL_( static LWError )
Load( InertiaData *dat, const LWLoadState *ls )
{
 //  char buf[ 150 ];
   short ver;
   int i;

   LWLOAD_I2( ls, &ver, 1 );              // version
   LWLOAD_I4( ls, &dat->vmapIdx, 1 );     // vmap index
   LWLOAD_I4( ls, &dat->flags, 1 );       // flags
   LWLOAD_STR( ls, dat->vmName, sizeof( dat->vmName ));   // vmap name

  /* if ( i )
      for( i = 1; i < popCnt_VMAP( dat ); i++ )
         if ( !strcmp( dat->vmName, popName_VMAP( dat, i )))
            dat->vmapIdx = i; */

   LWLOAD_I4(ls,&i,1);
   dat->pivot = (LWItemID) i;

   return vparmf->load( dat->lagParm, ls );     // lag envelope
}


/*
======================================================================
Save()

Handler callback.  Write instance data.
====================================================================== */

XCALL_( static LWError )
Save( InertiaData *dat, const LWSaveState *ss )
{
   short ver = INERTIA_VERSION;
   int      i;

   LWSAVE_I2( ss, &ver, 1 );                             // version
   LWSAVE_I4( ss, &dat->vmapIdx, 1 );                    // vmap index
   LWSAVE_I4( ss, &dat->flags, 1 );                      // flags
   LWSAVE_STR( ss, popName_VMAP( dat, dat->vmapIdx ));   // vmap name

   i = (int)dat->pivot;
   LWSAVE_I4(ss,&i,1);
 /* This one should have a macro too..*/
   return vparmf->save( dat->lagParm, ss );              // lag envelope
}



/*
======================================================================

Describe()

Handler callback.  Write a short, human-readable string describing
the instance data.
====================================================================== */

XCALL_( static const char * )
Describe( InertiaData *dat )
{
   sprintf( dat->desc, " Lag = %g s/m", dat->lagRate );
   return dat->desc;
}


/*
======================================================================
UseItems()

Handler callback.  Return an array of items we depend on.
====================================================================== */

XCALL_( static const LWItemID * )
UseItems( InertiaData *dat )
{
   static LWItemID ids[ 2 ] = { NULL };

   ids[ 0 ] = dat->pivot;
   return ids;
}


/*
======================================================================
ChangeID()

Handler callback.  ID numbers for items in the scene aren't constant.
They can change when items are added or deleted.  If the ID of our
pivot object has changed, we update our instance data.
====================================================================== */

XCALL_( static void )
ChangeID( InertiaData *dat, const LWItemID *ids )
{
   int i = 0;

   while ( ids[ i ] )
      if ( ids[ i ] == dat->pivot ) {
         dat->pivot = ids[ i + 1 ];
         return;
      }
      else
         i += 2;

   return;
}


/*
======================================================================
Init()

Handler callback, called at the start of rendering.
====================================================================== */

XCALL_( static LWError )
Init( InertiaData *dat, int mode )
{
   return NULL;
}


/*
======================================================================
Cleanup()

Handler callback, called at the end of rendering.
====================================================================== */

XCALL_( static void )
Cleanup( InertiaData *dat )
{
   return;
}


/*
======================================================================
NewTime()

Handler callback, called at the start of each sampling pass.
====================================================================== */

XCALL_( static LWError )
NewTime( InertiaData *dat, LWFrame fr, LWTime t )
{
   dat->frame = fr;
   dat->time = t;
   dat->vmId = NULL;  // reset now, re-aquire on first vertex of this pass

   /* The following operations can be performed onse every time,
      rather than once per vertex. */

   /* get the rotation origin for the pivot object */
   if ( dat->pivot )
      iteminfo->param( dat->pivot,
         ( dat->flags & INF_PIVLOCAL ) ? LWIP_POSITION : LWIP_W_POSITION,
         dat->time, dat->pivpos );
   else
      iteminfo->param( dat->item, LWIP_PIVOT, dat->time, dat->pivpos );

   /* get the rate from the envelope */
   vparmf->getVal( dat->lagParm, dat->time, NULL, &dat->lagRate );

   return NULL;
}


/*
======================================================================
Flags()

Handler callback.
====================================================================== */

XCALL_( static int )
Flags( InertiaData *dat )
{
   return ( dat->flags & INF_PIVLOCAL ) ? 0 : LWDMF_WORLD;
}


/*
======================================================================
Evaluate()

Handler callback.  This is called for each vertex of the object the
instance is associated with.  The vertex is moved by setting the
members of the source[] array in the displacement access structure.
====================================================================== */

XCALL_( static void )
Evaluate( InertiaData *dat, LWDisplacementAccess *da )
{
   LWDVector pos, v, xcol, ycol, zcol, siz;
   double t, dist;
   LWMeshInfoID mesh;


   VSUB3(v,da->oPos,dat->pivpos); // maybe this should be after the scaling...
   iteminfo->param( dat->item,LWIP_SCALING,   t, siz );
   VMUL3(v,v,siz);
   dist = VLEN( v );

   /* if possible, get the weight for the vertex from a VMAP and use it
      to scale the effect */

   if ( mesh = da->info )
      if ( dat->vmapIdx ) {
         LWFVector weight;

         if ( !dat->vmId ) { /* This will only happen once per time */
           // dat->vmId = mesh->pntVLookup( mesh, LWVMAP_WGHT, popName_VMAP( dat, dat->vmapIdx ));
            dat->vmId = mesh->pntVLookup( mesh, LWVMAP_WGHT, dat->vmName );
            if ( dat->vmId )
               mesh->pntVSelect( mesh, dat->vmId );
         }
       weight[ 0 ] = 0.0f;
         if ( mesh->pntVGet( mesh, da->point, weight ))
         dist *= weight[ 0 ];
       else // If vertex is missing weight, use 0 for weight!!!
         return; //dist = 0.0;
      }

   /* Get the transformation matrix of the pivot object.  t is the
      current time, minus the lag factors. */

   t = dat->time - ( dat->lagRate * dist );
   if ( dat->pivot ) {
      iteminfo->param( dat->pivot, LWIP_RIGHT,   t, xcol );
      iteminfo->param( dat->pivot, LWIP_UP,      t, ycol );
      iteminfo->param( dat->pivot, LWIP_FORWARD, t, zcol );
      iteminfo->param( dat->pivot,
         ( dat->flags & INF_PIVLOCAL ) ? LWIP_POSITION : LWIP_W_POSITION,
         t, pos );
   }
   else {
      iteminfo->param( dat->item, LWIP_RIGHT,      t, xcol );
      iteminfo->param( dat->item, LWIP_UP,         t, ycol );
      iteminfo->param( dat->item, LWIP_FORWARD,    t, zcol );
      iteminfo->param( dat->item, LWIP_W_POSITION, t, pos  );
   }

   /* transform the vertex (multiply by the transformation matrix) */

   da->source[ 0 ] = v[ 0 ] * xcol[ 0 ] + v[ 1 ] * ycol[ 0 ]
      + v[ 2 ] * zcol[ 0 ] + pos[ 0 ];
   da->source[ 1 ] = v[ 0 ] * xcol[ 1 ] + v[ 1 ] * ycol[ 1 ]
      + v[ 2 ] * zcol[ 1 ] + pos[ 1 ];
   da->source[ 2 ] = v[ 0 ] * xcol[ 2 ] + v[ 1 ] * ycol[ 2 ]
      + v[ 2 ] * zcol[ 2 ] + pos[ 2 ];
}



/*
======================================================================
Inertia()

Handler activation function.  Check the version, get some globals, and
fill in the callback fields of the handler structure.
====================================================================== */

XCALL_( int )
Inertia( long version, GlobalFunc *global, LWDisplacementHandler *local,
   void *serverData)
{
   if ( version != LWDISPLACEMENT_VERSION )
      return AFUNC_BADVERSION;

   objf     = global( LWOBJECTFUNCS_GLOBAL,   GFUSE_TRANSIENT );
   envf     = global( LWENVELOPEFUNCS_GLOBAL, GFUSE_TRANSIENT );
   chanf    = global( LWCHANNELINFO_GLOBAL,   GFUSE_TRANSIENT );
   iteminfo = global( LWITEMINFO_GLOBAL,      GFUSE_TRANSIENT);
   vparmf   = global( LWVPARMFUNCS_GLOBAL,    GFUSE_TRANSIENT);

   if ( !objf || !envf || !chanf || !iteminfo || !vparmf )
      return AFUNC_BADGLOBAL;

   local->inst->create   = Create;
   local->inst->destroy  = Destroy;
   local->inst->load     = Load;
   local->inst->save     = Save;
   local->inst->copy     = Copy;
   local->inst->descln   = Describe;

   local->item->useItems = UseItems;
   local->item->changeID = ChangeID;

   local->rend->init     = Init;
   local->rend->cleanup  = Cleanup;
   local->rend->newTime  = NewTime;

   local->evaluate       = Evaluate;
   local->flags          = Flags;

   return AFUNC_OK;
}



/* ----- all of this stuff is for the interface ----- */

/* a couple of globals */

static LWInstUpdate    *lwupdate = NULL;
static LWXPanelFuncs   *xpanf    = NULL;

/* control IDs */

enum { CH_PVOT = 0x8001, CH_FALL, CH_LOCL, CH_VMAP };

/* control labels */

#define STR_Pivot_TEXT "Inertia Pivot Object"
#define STR_Local_TEXT "Local Pivot"
#define STR_Fall_TEXT  "Lag Rate (s/m)"
#define STR_Type_TEXT  "Weight Map"

/* control array */

static LWXPanelControl ctrl_list[] = {
   { CH_FALL, STR_Fall_TEXT,  "float-env"  },
   { CH_PVOT, STR_Pivot_TEXT, "iPopChoice" },
   { CH_LOCL, STR_Local_TEXT, "iBoolean"   },
   { CH_VMAP, STR_Type_TEXT,  "iPopChoice" },
   { 0 }
};

/* matching array of data descriptors */

static LWXPanelDataDesc data_descrip[] = {
   { CH_FALL, STR_Fall_TEXT,  "float-env" },
   { CH_PVOT, STR_Pivot_TEXT, "integer"   },
   { CH_LOCL, STR_Local_TEXT, "integer"   },
   { CH_VMAP, STR_Type_TEXT,  "integer"   },
   { 0 },
};


/*
======================================================================
InertiaData_get()

XPanel callback.  XPanels calls this when it wants to retrieve the
value of a control from your instance data.  Returns a pointer to the
field containing the requested value.
====================================================================== */

XCALL_( static void * )
InertiaData_get( InertiaData *dat, unsigned long vid )
{
   void *result = NULL;

   if ( dat )
      switch ( vid ) {
         case CH_VMAP:
            result = &dat->vmapIdx;
            break;
         case CH_PVOT:
            result = &dat->pvid;
            break;
         case CH_FALL:
            result = dat->lagParm;
            break;
         case CH_LOCL:
            result = &dat->flags;
            break;
      }

   return result;
}


/*
======================================================================
InertiaData_set()

XPanel callback.  XPanels calls this when it wants to store the value
of a control in your instance data.  Returns 1 if the value could be
stored, otherwise 0.
====================================================================== */

XCALL_( static int )
InertiaData_set( InertiaData *dat, unsigned long vid, void *value )
{
   int rc = 0;

   if ( dat )
      switch ( vid ) {
         case CH_PVOT:
            dat->pvid = *(( int * ) value );
            if( dat->pvid )
               dat->pivot = popItemID( dat->pvid );
            else
               dat->pivot = NULL;
            rc = 1;
         break;

         case CH_VMAP:
            dat->vmapIdx = *(( int * ) value );
         if(dat->vmapIdx)
             strncpy(dat->vmName, popName_VMAP(dat,dat->vmapIdx) ,sizeof( dat->vmName ));
            rc = 1;
            break;

         case CH_FALL:
            rc = 1;
            break;

         case CH_LOCL:
            if ( *(( int * ) value ))
               dat->flags |= INF_PIVLOCAL;
            else
               dat->flags &= ~INF_PIVLOCAL;
            rc = 1;
            break;
      }

   return rc;
}


/*
======================================================================
ChangeNotify()

Xpanel callback.  XPanels calls this when an event occurs that affects
the value of one of your controls.  We use the instance update global
to tell Layout that our instance data has changed.
====================================================================== */

XCALL_( static void )
ChangeNotify( LWXPanelID panID, unsigned long cid, unsigned long vid,
   int event_code )
{
   void *val = NULL;
   void *dat;

   if ( event_code == LWXPEVENT_VALUE )
      if ( dat = xpanf->getData( panID, 0 ))
         lwupdate( LWDISPLACEMENT_HCLASS, dat );
}


/*
======================================================================
get_xpanel()

Create and initialize an xpanel.
====================================================================== */

static LWXPanelID get_xpanel( GlobalFunc *global, InertiaData *dat )
{
   LWXPanelID panID = NULL;

   static LWXPanelHint hint[] = {
      XpLABEL( 0, "Inertia" ),
      XpCHGNOTIFY( ChangeNotify ),
      XpPOPFUNCS( CH_VMAP, popCnt_VMAP, popName_VMAP ),
      XpPOPFUNCS( CH_PVOT, popCnt_Item, popName_Item ),
      XpENABLE_( CH_PVOT ), XpH( CH_LOCL ), XpEND,
      XpMIN( CH_FALL, 0 ),
      XpEND
   };

   xpanf = global( LWXPANELFUNCS_GLOBAL, GFUSE_TRANSIENT );
   if ( xpanf ) {
      panID = xpanf->create( LWXP_VIEW, ctrl_list );
      if ( panID ) {
         xpanf->hint( panID, 0, hint );
         xpanf->describe( panID, data_descrip, InertiaData_get, InertiaData_set );
         xpanf->viewInst( panID, dat );
         xpanf->setData( panID, 0, dat );
      }
   }

   return panID;
}


/*
======================================================================
Inertia_UI()

Interface activation function.  Get a global, create an xpanel, and
fill in the fields of the LWInterface structure.
====================================================================== */

XCALL_( int )
Inertia_UI( long version, GlobalFunc *global, LWInterface *local,
   void *serverData )
{
   if ( version != LWINTERFACE_VERSION )
      return AFUNC_BADVERSION;

   lwupdate = global( LWINSTUPDATE_GLOBAL, GFUSE_TRANSIENT );
   if ( !lwupdate )
      return AFUNC_BADGLOBAL;

   local->panel = get_xpanel( global, local->inst );
   if ( !local->panel )
      return AFUNC_BADGLOBAL;

   local->options = NULL;
   local->command = NULL;

   return AFUNC_OK;
}


/*
======================================================================
Need to update this at some point to reflect the new name fields.
====================================================================== */

ServerRecord ServerDesc[] = {
   { LWDISPLACEMENT_HCLASS, "LW_Inertia", Inertia },
   { LWDISPLACEMENT_ICLASS, "LW_Inertia", Inertia_UI },
   { NULL }
};

