/*
 * TOOL.C -- Spikey operation as an interactive mesh-edit tool, using Custom
 *           Polygon data types.
 *
 * Copyright 1999, NewTek, Inc.
 * written by Stuart Ferguson
 * last revision  8/30/99
 *
 */
#include <lwsdk/lwserver.h>
#include <lwsdk/lwmodtool.h>
#include <lwsdk/lwio.h>
#include <lwsdk/lwpolygon.h>
#include <lwsdk/lwmath.h>
#include "spikey.h"

typedef struct st_SpikeyInstance {
  int         meshFlags;
  float      *vrts;
  float      *norm;
  int        *pols;
} SpikeyInstance;


GlobalFunc       *GlobalFunction   = (GlobalFunc *)       NULL;
const char PluginDescription[]     = { "Custom Polygon SDK Sample Plugin"};
/*
 * The Spikey tool is the successor to the venerable 'make spikey' command.
 * The spikey tool employs the same mesh edit code to generate the new
 * geometry, while also allowing the parameters of the operation to be varied
 * interactively with a tool.  The tool state-machine is similar to bevel.
 */
typedef struct st_SpikeyTool {
	double		 spike, s0;
	int		 update;
	int		 active;
} SpikeyTool;


/*
 * The 'build' function is called -- on the request of the tool -- to
 * recompute the new geometry whenever the parameters of the operation
 * have changed.  Every time it runs, it resets the update state for the
 * tool.
 */
	static LWError
Spikey_Build (
	SpikeyTool		*tool,
	MeshEditOp		*op)
{
	EDError			 err;

	err = Spikey (op, tool->spike);
	tool->update = LWT_TEST_NOTHING;
	return (err ? "Failed" : NULL);
}


/*
 * The test function returns the mesh update action for the tool.  Mostly
 * this will be NOTHING, but sometimes there will be a need to advance to a
 * new state.  This will always result in a call to 'build' which will
 * reset the update state to NOTHING again.
 */
	static int
Spikey_Test (
	SpikeyTool		*tool)
{
	return tool->update;
}


/*
 * End is called when the interactive build is complete, either by the
 * direct request of the tool itself, or by the implicit action of some
 * other aspect of the application.  We reset the update and active
 * states, but leave the spikey percentage alone in case the user wants
 * to use the same value again.
 */
	static void
Spikey_End (
	SpikeyTool		*tool,
	int			 keep)
{
	tool->update = LWT_TEST_NOTHING;
	tool->active = 0;
}


/*
 * This function is called to get the help text for the tool which will
 * be displayed in the status line while the tool is active.  If the text
 * were variable, the 'dirty' function would be required to update it.
 */
	static const char *
Spikey_Help (
	SpikeyTool		*tool,
	LWToolEvent		*event)
{
	return "Drag left or right to adjust the spikey percentage";
}


/*
 * Raw mouse actions come in the form of 'down', 'move' and 'up' events.
 * On the down event we record the current spike factor which we will
 * use as the base vale to vary on mouse moves.  If the user presses the
 * alternate mouse button, we complete the last action and start a new
 * one with zero spikeyness.  This is consistent with the bevel tool.
 */
	static int
Spikey_Down (
	SpikeyTool		*tool,
	LWToolEvent		*event)
{
	if (event->flags & LWTOOLF_ALT_BUTTON) {
		tool->spike  = 0.0;
		tool->update = LWT_TEST_CLONE;
	} else
		tool->update = LWT_TEST_UPDATE;

	tool->s0 = tool->spike;
	tool->active = 1;
	return 1;
}


/*
 * On the mouse move event, we set the new spikey factor to be the value
 * at initial mouse down, plus 1% for each pixel the user has offset to
 * right of the initial position.  The update state has to be set since
 * we need to recompute the mesh for the new spike factor.
 */
	static void
Spikey_Move (
	SpikeyTool		*tool,
	LWToolEvent		*event)
{
	tool->spike  = tool->s0 + 0.01 * event->dx;
	tool->update = LWT_TEST_UPDATE;
}


/*
 * A number of different one-time events are handled through this single
 * callback and mostly perform various state transitions.
 */
	static void
Spikey_Event (
	SpikeyTool		*tool,
	int			 code)
{
	switch (code) {
	    case LWT_EVENT_DROP:
		/*
		 * The 'drop' action is caused when the user clicks in
		 * the blank area of the display or uses the keyboard
		 * equivalent.  If the tool is active, we force a
		 * rejection of any interactive action partly complete.
		 * For inactive tools we drop through to...
		 */
		if (tool->active) {
			tool->update = LWT_TEST_REJECT;
			break;
		}

	    case LWT_EVENT_RESET:
		/*
		 * The reset action corresponds to the reset command on
		 * the numeric panel, and causes us to snap the spikey
		 * factor back to zero.  Resets are also implicit when
		 * the user drops an inactive tool, thus the passthru
		 * above.
		 */
		tool->spike = 0.0;
		break;

	    case LWT_EVENT_ACTIVATE:
		/*
		 * Activate can be triggered from the numeric window or
		 * with a keystroke, and it should restart the edit
		 * operation with its current settings.
		 */
		tool->update = LWT_TEST_UPDATE;
		tool->active = 1;
		break;
	}
}

static int Spikey_Update( SpikeyInstance *inst, int lnum, int changes, LWPolygonTypeAccessID ptinfo ) 
{
           
    if( !( changes & ( SYNC_GEOMETRY | SYNC_POSITION | SYNC_VMAP | SYNC_TAGS ) ) )
    {
        return 0;
    }

    return 0;
}


static int Spikey_TstMesh( SpikeyInstance *inst, LWPolID pol, LWPolygonTypeAccessID ptinfo ) 
{    
    return 1;
}

/************************************************************************
 * This is called when an instance of your custom polygon needs 
 * to be drawn in a view port using basic drawing functions.
 *
 * @param inst plugin instance pointer
 * @param pol  Polygon ID
 * @param disp basic drawing functions
 * @param ptinfo PolygonType Access functions
 */
static void Spikey_GenMesh( SpikeyInstance *inst, LWPolID pol, LWPolyMeshInfoID mesh, LWPolygonTypeAccessID ptinfo ) 
{
    int i;
    LWFVector pos1;
    LWFVector pos2;
    LWFVector pos3;
    LWFVector n;
    void     *offsetmap;
    float     offset;
    float     scale;
    int       numpoints;
    LWFVector midpoint;

    // we create a triangle mesh
    mesh->type = 3;

    // free previous data

    if ( inst->vrts )
    {
        free( inst->vrts );
        inst->vrts = NULL;
    }

    if ( inst->pols )
    {
        free( inst->pols );
        inst->pols = NULL;
    }

    if ( inst->norm )
    {
        free( inst->norm );
        inst->norm = NULL;
    }

    numpoints = ptinfo->polSize( ptinfo, pol );
    // minimum 3 vertex poly demanded
    if ( numpoints < 3 )
    {
        mesh->nvrt = 0;
        return; 
    }

    // set sizes
    mesh->npol = ptinfo->polSize( ptinfo, pol );
    mesh->nvrt = mesh->npol * 3; 
    mesh->ntex =  0;

    // allocate arreays
    inst->pols = malloc( sizeof( int )   * 3 * mesh->npol );
    inst->vrts = malloc( sizeof( float ) * 3 * (mesh->nvrt) );
    inst->norm = malloc( sizeof( float ) * 3 * (mesh->nvrt) );


    // get first three points
    ptinfo->pntOtherPos( ptinfo, ptinfo->polVertex( ptinfo, pol, 0 ), pos1 );
    ptinfo->pntOtherPos( ptinfo, ptinfo->polVertex( ptinfo, pol, 1 ), pos2 );
    ptinfo->pntOtherPos( ptinfo, ptinfo->polVertex( ptinfo, pol, 2 ), pos3 );

    VSUB3(pos1,pos2,pos1); 
    VSUB(pos3,pos2); 

    VCROSS( n, pos1, pos3 );

    // get the stored offset values from the vertex map
    offsetmap = ptinfo->pntVLookup( ptinfo, PTYP_SPIKEY, "Spikey.Offset" );
    ptinfo->pntVIDGet( ptinfo, ptinfo->polVertex( ptinfo, pol, 0 ), &offset, offsetmap );

    scale = (float)(1.0f/VLEN(n)) * offset;
    VSCL( n, scale );

    // calculate midpoint

    pos1[0] = pos1[1] = pos1[2] = 0.0f;
    for ( i = 0; i < numpoints; i++ )
    {
        ptinfo->pntOtherPos( ptinfo, ptinfo->polVertex( ptinfo, pol, i ), pos2 );
        VADD( pos1, pos2);
    }

    scale = (1.0f/ numpoints);
    VSCL( pos1, scale);
    VADD( pos1, n );
    VCPY( midpoint, pos1 );

	for ( i = 0; i < mesh->npol; i++ ) 
    {
        // save points
        ptinfo->pntOtherPos( ptinfo, ptinfo->polVertex( ptinfo, pol, i ), pos1 );
        ptinfo->pntOtherPos( ptinfo, ptinfo->polVertex( ptinfo, pol, ((i+1)%mesh->npol) ), pos3 );

        VCPY( (inst->vrts + i*9),   pos1 );
        VCPY( (inst->vrts + i*9+3), pos3 );
        VCPY( (inst->vrts + i*9+6), midpoint );

        // calc normal
        VSUB3(pos1,pos1,midpoint); 
        VSUB(pos3,midpoint); 

        VCROSS( n, pos1, pos3 );
        scale = (float)(1.0f/VLEN(n));
        VSCL( n, scale );

        // save normal
        VCPY( (inst->norm + i*9), n );
        VCPY( (inst->norm + i*9+3), n );
        VCPY( (inst->norm + i*9+6), n );

        // set vertex ids
        *(inst->pols + i*3    ) = i*3;
        *(inst->pols + i*3 + 1) = i*3+1;
        *(inst->pols + i*3 + 2) = i*3+2;
	}

    // copy pointer to data for LW to read
    mesh->vrts = inst->vrts;
    mesh->pols = inst->pols;
    mesh->norm = inst->norm;

    return;
}

/************************************************************************
 * This is called when an instance of your custom polygon needs 
 * to be drawn in a view port using basic drawing functions.
 *
 * @param inst plugin instance pointer
 * @param pol  Polygon ID
 * @param disp basic drawing functions
 * @param ptinfo PolygonType Access functions
 */
static void Spikey_Display( SpikeyInstance *inst, LWPolID pol, const LWWireDrawAccess *disp, LWPolygonTypeAccessID ptinfo ) 
{
    LWFVector pos1;
    int i;
    int nvert = ptinfo->polSize( ptinfo, pol );


    if ( nvert < 3 )
    {
        return;
    }

    ptinfo->pntOtherPos( ptinfo, ptinfo->polVertex( ptinfo, pol, 0 ), pos1 );
    disp->moveTo( disp->data, pos1, LWWIRE_SOLID );
    for ( i = 1; i < nvert; i++ )
    {   
        ptinfo->pntOtherPos( ptinfo, ptinfo->polVertex( ptinfo, pol, i ), pos1 );
        disp->lineTo( disp->data, pos1, LWWIRE_SOLID );
    }

    return;
}


/************************************************************************
 * Allocates private data for Polygon Type.
 *
 * @param ptinfo PolygonType Access functions
 */
static SpikeyInstance *Spikey_Alloc( LWPolygonTypeAccessID ptinfo ) 
{    
    SpikeyInstance *inst;
    
    if( inst = malloc( sizeof( SpikeyInstance ) ) ) 
    {
        memset( inst, 0, sizeof( SpikeyInstance ) );
        inst->meshFlags = 0;
    }
    return inst;
}

/************************************************************************
 * Frees private data.
 *
 * @param inst Pointer to private data.
 */
static void Spikey_Free( SpikeyInstance *inst ) 
{  
    if( inst != NULL ) 
    {       
        // free arrays

        if ( inst->vrts )
        {
            free( inst->vrts );
        }

        if ( inst->pols )
        {
            free( inst->pols );
        }

        if ( inst->norm )
        {
            free( inst->norm );
        }

        free( inst );
    }
}

/************************************************************************
*                                                                       *
*       PolygonLoad:                                                    *
*                                                                       *
*       Loads The Previously Saved Data From The Scene File.            *
*                                                                       *
*       Input:                                                          *
*                                                                       *
*       PolyType = Address Of Polygon    Structure.                     *
*       lState   = Address Of Load State Structure.                     *
*                                                                       *
*       Output:                                                         *
*                                                                       *
*       The Saved Polygon Values Are Read From The Scene File.          *
*                                                                       *
*       21.Jan.2004 Jamie Lisa Finch.                                   *
*                                                                       *
************************************************************************/

XCALL_ (LWError) PolygonLoad( void *vPolyType, const LWLoadState *lState ) {

  LWPolyType *PolyType;

  XCALL_INIT;

  PolyType = (LWPolyType *) vPolyType;
  if( ( PolyType       != (LWPolyType *) NULL ) &&
      ( lState->ioMode == LWIO_SCENE      ) ) {
    }
  return NULL;
  }

/************************************************************************
*                                                                       *
*       PolygonSave:                                                    *
*                                                                       *
*       Save The Current Values Into Scene File.                        *
*                                                                       *
*       Input:                                                          *
*                                                                       *
*       PolyType = Address Of Polygon    Structure.                     *
*       sState   = Address Of Save State Structure.                     *
*                                                                       *
*       Output:                                                         *
*                                                                       *
*       Saves The Polygon Values Into The Scene File.                   *
*                                                                       *
*       21.Jan.2004 Jamie Lisa Finch.                                   *
*                                                                       *
************************************************************************/

XCALL_ (LWError) PolygonSave( void *vPolyType, const LWSaveState *sState ) {

  LWPolyType *PolyType;

  XCALL_INIT;

  PolyType = (LWPolyType *) vPolyType;
  if( ( PolyType       != (LWPolyType *) NULL ) &&
      ( sState->ioMode == LWIO_SCENE      ) ) {
    }
  return NULL;
  }

/************************************************************************
*                                                                       *
*       PolygonCopy:                                                    *
*                                                                       *
*       Copies One Polygon Structure To Another.                        *
*                                                                       *
*       Input:                                                          *
*                                                                       *
*       ToPolyType   = Address Of Destination PolyType Structure.       *
*       FromPolyType = Address Of Source      PolyType Structure.       *
*                                                                       *
*       Output:                                                         *
*                                                                       *
*       The PolyType Structure Is Copy From One To The Other.           *
*                                                                       *
*       21.Jan.2004 Jamie Lisa Finch.                                   *
*                                                                       *
************************************************************************/

XCALL_ (LWError) PolygonCopy( void *vToPolyType, void *vFromPolyType ) {

  LWPolyType *ToPolyType, *FromPolyType;

  XCALL_INIT;

    ToPolyType = (LWPolyType *)   vToPolyType;
  FromPolyType = (LWPolyType *) vFromPolyType;

  if( (   ToPolyType != (LWPolyType *) NULL ) &&
      ( FromPolyType != (LWPolyType *) NULL ) ) {
    *ToPolyType = *FromPolyType;
    }
  return NULL;
  }

/************************************************************************
*                                                                       *
*       PolygonCreate:                                                  *
*                                                                       *
*       Creates The Structure And Initilized The Data.                  *
*                                                                       *
*       Input:                                                          *
*                                                                       *
*       Priv    = Address Of Private Data.                              *
*       Id      = LightWave Item Id.                                    *
*       Error   = Address Of LightWave Error Message.                   *
*                                                                       *
*       Output:                                                         *
*                                                                       *
*       The Polygon Structure Is Created And Initilized.                *
*                                                                       *
*       21.Jan.2004 Jamie Lisa Finch.                                   *
*                                                                       *
************************************************************************/

XCALL_ (LWInstance) PolygonCreate( void *pdata, LWItemID Id, LWError *Error ) {

  LWPolyType *PolyType;

  XCALL_INIT;

  if( PolyType = malloc( sizeof( LWPolyType ) ) ) {
    memset( PolyType, 0, sizeof( LWPolyType ) );

    PolyType->type    = PTYP_SPIKEY;
    PolyType->flags   = LWGPTF_SURFACE;
    PolyType->alloc   = Spikey_Alloc;
    PolyType->free    = Spikey_Free;
    PolyType->display = Spikey_Display;
    PolyType->genMesh = Spikey_GenMesh;
    PolyType->tstMesh = Spikey_TstMesh;
    PolyType->update  = Spikey_Update;
    }

  return (void *) PolyType;
  }

/************************************************************************
*                                                                       *
*       PolygonDestroy:                                                 *
*                                                                       *
*       Removes The Instance Of The Polygon.                            *
*                                                                       *
*       Input:                                                          *
*                                                                       *
*       Polygon = Address Of Polygon Structure.                         *
*                                                                       *
*       Output:                                                         *
*                                                                       *
*       All Data Associated With The Polygon Structure Is Freed.        *
*                                                                       *
*       21.Jan.2004 Jamie Lisa Finch.                                   *
*                                                                       *
************************************************************************/

XCALL_ (void) PolygonDestroy( void *vPolyType ) {

  LWPolyType *PolyType;

  XCALL_INIT;

  PolyType = (LWPolyType *) vPolyType;
  if( PolyType != (LWPolyType *) NULL ) {

    free( PolyType );
    PolyType = (LWPolyType *) NULL;
    }
  }

/************************************************************************
*                                                                       *
*       PolygonDescribe:                                                *
*                                                                       *
*       Returns A Character String Telling The Called Which Plugin      *
*       This Is.                                                        *
*                                                                       *
*       Input:                                                          *
*                                                                       *
*       Polygon = Address Of Destination Polygon Structure.             *
*                                                                       *
*       Output:                                                         *
*                                                                       *
*       The Description String Is Returned.                             *
*                                                                       *
*       21.Jan.2004 Jamie Lisa Finch.                                   *
*                                                                       *
************************************************************************/

XCALL_ (const char *) PolygonDescribe( void *vPolyType ) {

  XCALL_INIT;

  return PluginDescription;
  }

/************************************************************************
*                                                                       *
*       PolygonActivate:                                                *
*                                                                       *
*       This Functions Creates The Polygon Callback Functions.          *
*                                                                       *
*       Input:                                                          *
*                                                                       *
*       Version    = Query Version Of The Plugin.                       *
*       Global     = Address Of The Global Function.                    *
*       Local      = Address Of New LW Polygon Hander Structure.        *
*       ServerData = Address Of Server Data.                            *
*                                                                       *
*       Output:                                                         *
*                                                                       *
*       Returns -> Value Of Error Code, AFUNC_OK, AFUNC_BADVERSION.     *
*                                                                       *
*       21.Jan.2004 Jamie Lisa Finch.                                   *
*                                                                       *
************************************************************************/

XCALL_ (int) PolygonActivate( long Version, GlobalFunc *Global, void *vLocal, void *ServerData ) {

  LWPolygonHandler *Local;

  if( Version != LWPOLYGON_VERSION )
    return AFUNC_BADVERSION;

  Local = (LWPolygonHandler *) vLocal;
  if( Local != (LWPolygonHandler *) NULL ) {

    GlobalFunction = Global;

    Local->inst->create  = PolygonCreate;
    Local->inst->destroy = PolygonDestroy;
    Local->inst->load    = PolygonLoad;
    Local->inst->save    = PolygonSave;
    Local->inst->copy    = PolygonCopy;
    Local->inst->descln  = PolygonDescribe;
    }
  return AFUNC_OK;
  }


/*
 * The options for the spikey tool consist of an xPanel that will be
 * placed into the "Numeric" window in Modeler.  This is a VIEW panel
 * which displays the properties of the spikey instance.  There are
 * two properties -- the spikiness percentage, and the active state --
 * but the active state is a hidden parameter that only control the
 * enable state of the spikey percentage control.  The parameters are
 * assigned unique longword codes.
 */
#define XID_SPKF	LWID_('s','p','k','f')
#define XID_ACTI	LWID_('a','c','t','i')


/*
 * The 'get' function for the panel state reads out the two possible
 * values given their ID codes.  Pointers to the double and int values
 * are returned.
 */
	static void *
Spikey_Get (
	SpikeyTool		*tool,
	unsigned long		 vid)
{
	if (vid == XID_SPKF)
		return &tool->spike;

	if (vid == XID_ACTI)
		return &tool->active;

	return NULL;
}


/*
 * The 'set' function is called when a parameter is altered on the 
 * panel, and in this case only the spikey percentage can be altered.
 * If it is, the update state is set to compute the new geometry.
 */
	static int
Spikey_Set (
	SpikeyTool		*tool,
	unsigned long		 vid,
	double			*value)
{
	if (vid != XID_SPKF)
		return 0;

	tool->spike  = value[0];
	tool->update = LWT_TEST_UPDATE;
	return 1;
}


/*
 * To create our panel, we define our VIEW data state with the two
 * typed parameters, as well as hints for how to display them.  The
 * default control for the active state will be removed and then the
 * active state will be used to control the enable state of the
 * spikey percentage.
 */
	static LWXPanelID
Spikey_Panel (
	SpikeyTool		*tool)
{
	static LWXPanelDataDesc	 def[] = {
		{ XID_SPKF,  "Spike Factor",	"percent" },
		{ XID_ACTI,  "--hidden--",	"integer" },
		{ 0 }
	};
	static LWXPanelHint	 hint[] = {
		XpDELETE     (XID_ACTI),
		XpENABLEMSG_ (XID_ACTI, "Tool is currently inactive."),
			XpH  (XID_SPKF),
			XpEND,
		XpEND
	};
	LWXPanelID		 pan;

	pan = (*globFun_pan->create) (LWXP_VIEW, (LWXPanelControl *) def);
	if (!pan)
		return NULL;

	(*globFun_pan->describe) (pan, def, Spikey_Get, Spikey_Set);
	(*globFun_pan->hint) (pan, 0, hint);
	return pan;
}


/*
 * Free the tool instance when done.
 */
	static void
Spikey_Done (
	SpikeyTool		*tool)
{
	free (tool);
}


/*
 * Main entry point for the plug-in.  This allocates the tool, sets its
 * initial state and returns the filled-in local struct.
 */
	XCALL_(int)
Activate (
	long			 version,
	GlobalFunc		*global,
	LWMeshEditTool		*local,
	void			*serverData)
{
	SpikeyTool		*tool;

	if (version != LWMESHEDITTOOL_VERSION)
		return AFUNC_BADVERSION;

	/*
	 * Get global data -- monitor and xPanels functions.
	 */
	globFun_mon = (*global) ("LWM: Dynamic Monitor", GFUSE_TRANSIENT);
	if (!globFun_mon)
		return AFUNC_BADGLOBAL;

	globFun_pan = (*global) (LWXPANELFUNCS_GLOBAL, GFUSE_TRANSIENT);
	if (!globFun_pan)
		return AFUNC_BADGLOBAL;

	/*
	 * Allocate and init the tool instance.
	 */
	tool = malloc (sizeof (SpikeyTool));
	if (!tool)
		return AFUNC_OK;

	memset (tool, 0, sizeof (*tool));
	tool->spike  = 0.0;
	tool->update = LWT_TEST_NOTHING;
	tool->active = 0;

	/*
	 * Initialize the local tool struct with instance and handler
	 * callback functions.
	 */
	local->instance    = tool;
	local->tool->done  = Spikey_Done;
	local->tool->help  = Spikey_Help;
	local->tool->down  = Spikey_Down;
	local->tool->move  = Spikey_Move;
	local->tool->event = Spikey_Event;
	local->tool->panel = Spikey_Panel;
	local->build       = Spikey_Build;
	local->test        = Spikey_Test;
	local->end         = Spikey_End;

	return AFUNC_OK;
}


/*
 * Globals necessary to declare the class and name of this plugin server.
 */
ServerTagInfo	UserNames  [] = {
	"SpikeyPlus Tool",		SRVTAG_USERNAME   | LANGID_USENGLISH,
	"SpikeyPlus",		SRVTAG_BUTTONNAME | LANGID_USENGLISH,
	"polygons",		SRVTAG_CMDGROUP,
	"polygon/transform",	SRVTAG_MENU,
	"pol",			SRVTAG_ENABLE,
	0
};

static ServerTagInfo Polygon_Tags[] = {
  { "Spikey Polytype",        SRVTAG_USERNAME   | LANGID_USENGLISH }, 
  { NULL }
  };

ServerRecord ServerDesc[] = {
  { LWPOLYGON_HCLASS,     "Spikey Polytype SDK sample",        PolygonActivate,    Polygon_Tags },
  { LWMESHEDITTOOL_CLASS, "LWSpikeyPlusTool", Activate, UserNames },
  { NULL }
  };
