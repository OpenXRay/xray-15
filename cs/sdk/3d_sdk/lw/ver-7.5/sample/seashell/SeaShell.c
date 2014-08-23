/*
 * SEASHELL.C -- SeaShell interactive mesh-edit tool version.
 *
 * operations:
 * 1. Make a disc
 *    Bottom <0> Top <0> Center<1.5m,-1.5m,0> Radius<1m,1m,0>
 * 2. Run SeaShellTool
 * 3. Drag left or right to adjust Scale per loop.
 *
 * written by Yoshiaki Tazaki
 * last revision  2/09/2000
 */
#include <lwserver.h>
#include <lwmodtool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>


/*
 * SeaShell parameters..
 */
#define DEF_AXIS		1
#define DEF_NREP		4
#define DEF_NSID		20
#define DEF_OFF			1
#define DEF_UWRP		0.2
#define DEF_VWRP		1.0
#define DEF_SCL			0.6
#define DEF_VROT		0

#define XID_AXIS	LWID_('a','x','i','s')
#define XID_NREP	LWID_('n','r','e','p')
#define XID_NSID	LWID_('n','s','i','d')
#define XID_OFF 	LWID_('o','f','f',' ')
#define XID_SCL 	LWID_('s','c','l',' ')
#define XID_TXUV	LWID_('t','x','u','v')
#define XID_UWRP	LWID_('u','w','r','p')
#define XID_VWRP	LWID_('v','w','r','p')
#define XID_VROT	LWID_('v','r','o','t')

#define CLAMP(a,b,c) (((a) < (b)) ? (b) : (((a) > (c)) ? (c) : (a)))
#define DEG2RAD(x)	 ((x) *  0.01745329252222)

/*
 * Vertices coordinate structure.
 */
typedef struct _vertex {
	double			co[3];
} Vertex;

typedef struct _face {
	int				 numPnts;
	char			*surface;
    unsigned long    type;
} Face;

typedef struct st_SeaShellTool {
	LWXPanelFuncs	*panel;
    MeshEditOp      *op;

/*  SeaShell Parameters  */
	double		 	 scl, scl0, uwrp, vwrp;
	int	         	 axis, nrep, nsid, off, uvs, vrot;
	int			 	 active;
	int			 	 dirty, update;

/*  SeaShell Working Area  */
	int		 		numPnts;			/*  number of points  */
	int		 		numPols;			/*  number of polygons  */
	LWPntID			*points0;			/*  front polygons list  */
	LWPntID			*points1;			/*  back polygons list  */
	Face			*faces;				/*  face info  */
	Vertex			*vert;				/*  vertex list  */
} SeaShellTool;


static int          SeaShell_Test   ( SeaShellTool *tool );
static LWError      SeaShell_Build  ( SeaShellTool *tool, MeshEditOp *op );
static void         SeaShell_End    ( SeaShellTool *tool, int keep );

static void         SeaShell_Done   ( SeaShellTool *tool );
static void         SeaShell_Draw   ( SeaShellTool *tool, LWWireDrawAccess *draw );
static const char * SeaShell_Help   ( SeaShellTool *tool, LWToolEvent *event );
static int          SeaShell_Dirty  ( SeaShellTool *tool );
static int          SeaShell_Count  ( SeaShellTool *tool, LWToolEvent *event );
static int          SeaShell_Handle ( SeaShellTool *tool, LWToolEvent *event, int i, LWDVector pos );
static int          SeaShell_Start  ( SeaShellTool *tool, LWToolEvent *event );
static int          SeaShell_Adjust ( SeaShellTool *tool, LWToolEvent *event, int i );
static int          SeaShell_Down   ( SeaShellTool *tool, LWToolEvent *event );
static void         SeaShell_Move   ( SeaShellTool *tool, LWToolEvent *event );
static void         SeaShell_Up     ( SeaShellTool *tool, LWToolEvent *event );
static void         SeaShell_Event  ( SeaShellTool *tool, int code );
static LWXPanelID   SeaShell_Panel  ( SeaShellTool *tool );
static void *       SeaShell_Get    ( void *data, unsigned long vid );
static int          SeaShell_Set    ( void *data, unsigned long vid, void *value );

static EDError      SeaShell_GetProfile( SeaShellTool *tool, const EDPolygonInfo *info );
static void         SeaShell_TransPoint( int axis, double rot, double scal, double cen, double *sco, double *dco );
static EDError      SeaShell_GetIndexCount( int *count, const EDPolygonInfo *info );


/*
 * Main entry point for mesh tool initialization.
 */
	XCALL_(int)
Activate (
	long			 version,
	GlobalFunc		*global,
	LWMeshEditTool	*local,
	void			*serverData)
{
	SeaShellTool	*tool;

	if (version != LWMESHEDITTOOL_VERSION)
		return AFUNC_BADVERSION;


	tool = malloc (sizeof(SeaShellTool));
	if (!tool)
		return AFUNC_OK;

	memset (tool, 0, sizeof (*tool));
	tool->panel  		= (*global) (LWXPANELFUNCS_GLOBAL, GFUSE_TRANSIENT);
	tool->active 		= 1;
	tool->update 		= LWT_TEST_UPDATE;
	tool->axis   		= DEF_AXIS;
	tool->nrep   		= DEF_NREP;
	tool->nsid   		= DEF_NSID;
	tool->off    		= DEF_OFF;
	tool->uwrp			= DEF_UWRP;
	tool->vwrp			= DEF_VWRP;
	tool->scl    		= DEF_SCL;
	tool->vrot			= DEF_VROT;

	tool->numPnts		= 0;
	tool->numPols		= 0;
	tool->points0		= NULL;
	tool->points1		= NULL;
	tool->faces			= NULL;
	tool->vert			= NULL;

	local->instance     = tool;
	local->tool->done   = SeaShell_Done;
	local->tool->draw   = SeaShell_Draw;
	local->tool->count  = SeaShell_Count;
	local->tool->dirty  = SeaShell_Dirty;
	local->tool->help   = SeaShell_Help;
	local->tool->handle = SeaShell_Handle;
	local->tool->start  = SeaShell_Start;
	local->tool->adjust = SeaShell_Adjust;
	local->tool->down   = SeaShell_Down;
	local->tool->move   = SeaShell_Move;
	local->tool->event  = SeaShell_Event;
	local->tool->panel  = SeaShell_Panel;

	local->test         = SeaShell_Test;
	local->build        = SeaShell_Build;
	local->end          = SeaShell_End;

	return AFUNC_OK;
}


/*
 *  SeaShell_Test
 *
 *  this function returns a code for the edit action that needs to be
 *  performed.  Actions given below.
 *
 */
	static int
SeaShell_Test (
	SeaShellTool		*tool)
{
	return tool->update;
}


/*
 *  SeaShell_Build
 *
 *  this function performs the mesh edit operation to reflect the current
 *  tool settings.
 *
 */
	static LWError
SeaShell_Build (
	SeaShellTool	*tool,
	MeshEditOp		*op)
{
	EDError			 result;
	LWPntID			 points[4], *front_p, *back_p, *temp_p;
	Face			*face;
	int				 num_pnts, num_pols;
    int              i, j, k, l, n, sum, loop;
	double			 rot, scl, cen, sc, rt, co[3];
	float			 uv[2];

											/*  ALLOCATE WORKING MEMORIES  */
	if (tool->numPols == 0)
	{
		num_pnts  = 0;
		(*op->polyScan)(op->state, (EDPolyScanFunc *)SeaShell_GetIndexCount, &num_pnts, OPLYR_PRIMARY);
		num_pols  = (*op->polyCount)  (op->state, OPLYR_PRIMARY, EDCOUNT_ALL);
		if (tool->points0) free(tool->points0);
		if (tool->faces  ) free(tool->faces);
		if (tool->vert   ) free(tool->vert   );
		tool->points0  = (LWPntID *) malloc (sizeof(LWPntID) * num_pnts * 2);
		tool->points1  = tool->points0 + num_pnts;
		tool->faces    = (Face    *) malloc (sizeof(Face   ) * num_pols);
		tool->vert     = (Vertex  *) malloc (sizeof(Vertex ) * num_pnts);
	}

											/*  GET PROFILE POLYGONS  */
	tool->numPols  = 0;
	tool->numPnts  = 0;
	tool->op       = op;
	result = (*op->polyScan)(op->state, (EDPolyScanFunc *)SeaShell_GetProfile, tool, OPLYR_PRIMARY);
	if (result != EDERR_NONE) goto EXIT;

											/*  MAKE SEASHELL SHAPE  */
    n   = tool->nsid * tool->nrep; 
	rot = DEG2RAD(360.0 / tool->nsid);
	cen = (double) tool->off / tool->scl;
	scl = pow (tool->scl, 1.0 / tool->nsid);
	sc  = 1.0;
	rt  = 0.0;

	front_p = tool->points0;
	back_p  = tool->points1;
											/*  MAKE SIDE FACES  */
    for(i = 0; i < n; i++)
    {
		sc *= scl;
		rt += rot;

		for (j = sum = 0; j < tool->numPols; j++)
		{
			face = &tool->faces[j];

			for (k = 0; k < face->numPnts; k++)
			{
				if (tool->uvs) {
					uv[0] = 1.0f - (float) i * (float) tool->uwrp;
					uv[1] = (float) k / (face->numPnts-1) * (float) tool->vwrp;
					(*op->pntVMap)(op->state, front_p[sum + k], LWVMAP_TXUV, "UV Texture", 2, uv );
				}
				SeaShell_TransPoint (tool->axis, rt, sc, cen, tool->vert[sum + k].co, co);
				back_p[sum + k] = (*op->addPoint) (op->state, co);
			}

			loop = tool->uvs ? (face->numPnts-1) : face->numPnts;

			for (k = 0; k < loop; k++)
			{
				l = (k == face->numPnts-1) ? 0 : k+1;
				points[0] = front_p[sum + l];
				points[1] = front_p[sum + k];
				points[2] = back_p [sum + k];
				points[3] = back_p [sum + l];
				(*op->addFace) (op->state, face->surface, 4, points);
			}

			sum += face->numPnts;
		}

		temp_p = front_p; front_p = back_p; back_p = temp_p;
    }

										/*  ADD UVs FOR THE LAST PROFILE  */
	for (j = sum = 0; j < tool->numPols; j++)
	{
		face = &tool->faces[j];

		for (k = 0; k < face->numPnts; k++)
		{
			if (tool->uvs) {
				uv[0] = 1.0f - (float) n * (float) tool->uwrp;
				uv[1] = (float) k / (face->numPnts-1) * (float) tool->vwrp;
				(*op->pntVMap)(op->state, front_p[sum + k], LWVMAP_TXUV, "UV Texture", 2, uv );
			}
		}
		sum += face->numPnts;
	}

										/*  MAKE THE CAP POLYGON AT TOP  */
	for (i = sum = 0; i < tool->numPols; i++)
	{
		face = &tool->faces[i];

		for (j = 0; j < face->numPnts; j++) {
			back_p[sum + j] = front_p[sum + face->numPnts - j - 1];
		}
		(*op->addFace) (op->state, face->surface, face->numPnts, back_p + sum);
		sum += face->numPnts;
	}

EXIT:
	tool->update = LWT_TEST_NOTHING;
	return (result ? "SeaShell build error." : NULL);
}


/*
 *  SeaShell_End
 *
 *  this function clears the state when the last edit action is completed.
 *  This can be a result of the 'test' update code or it can be triggered 
 *  by an external action.
 *
 */
	static void
SeaShell_End (
	SeaShellTool	*tool,
	int			 	keep)
{
	if (tool->points0) free(tool->points0);
	if (tool->faces  ) free(tool->faces  );
	if (tool->vert   ) free(tool->vert   );
	tool->numPols = 0;
	tool->numPnts = 0;
	tool->points0 = NULL;
	tool->points1 = NULL;
	tool->faces   = NULL;
	tool->vert    = NULL;
	tool->active  = 0;
	tool->update  = LWT_TEST_NOTHING;
}


/*
 *  SeaShell_Help
 *
 *  this function returns a text string to be displayed as a help tip for
 *  this tool.
 *
 */
	static const char *
SeaShell_Help (
	SeaShellTool		*tool,
	LWToolEvent			*event)
{
	return "Drag left or right to adjust the Scale per Loop";
}



/*
 *  SeaShell_Down
 *
 *  this function processs a mouse-down event.  If this function returns
 *  false, handle processing will be done instead of raw mouse event
 *  processing.
 *
 */
	static int
SeaShell_Down (
	SeaShellTool		*tool,
	LWToolEvent			*event)
{
	tool->update = LWT_TEST_UPDATE;
	tool->scl0   = tool->scl;
	return 1;
}



/*
 *  SeaShell_Move
 *
 *  this function process a mouse-move event.  This is only called if the
 *  down function returned true.
 *
 */
	static void
SeaShell_Move (
	SeaShellTool		*tool,
	LWToolEvent			*event)
{
	tool->scl    = tool->scl0 + 0.01 * event->dx;
	tool->update = LWT_TEST_UPDATE;
}



/*
 *  SeaShell_Draw
 *
 *  this function display a wireframe representation of the tool in a 3D
 *  viewport.  Typically this draws the handles.
 *
 */
	static void
SeaShell_Draw (
	SeaShellTool		*tool,
	LWWireDrawAccess	*draw)
{
	if (!tool->active)
		return;

	tool->dirty = 0;
}


/*
 *  SeaShell_Dirty
 *
 *  this function returns flag bit if either the wireframe or help string
 *  need to be refreshed.
 *
 */
	static int
SeaShell_Dirty (
	SeaShellTool		*tool)
{
	return (tool->dirty ? LWT_DIRTY_WIREFRAME : 0);
}


/*
 *  SeaShell_Count
 *
 *  this function returns the number of handles.  If zero, then 'start' is
 *  used to set the initial handle point.
 *
 */
	static int
SeaShell_Count (
	SeaShellTool		*tool,
	LWToolEvent			*event)
{
	return 0;
}


/*
 *  SeaShell_Handle
 *
 *  this function returns the 3D location and priority of handle 'i', or zero
 *  if the handle is currently invalid.
 *
 */
	static int
SeaShell_Handle (
	SeaShellTool		*tool,
	LWToolEvent			*event,
	int 				 i,
	LWDVector 			 pos )
{
	return 0;
}


/*
 *  SeaShell_Start
 *
 *  this function takes an initial mouse-down position and return the index
 *  of the handle that should be dragged.
 *
 */
	static int
SeaShell_Start (
	SeaShellTool		*tool,
	LWToolEvent			*event)
{
	return 0;
}


/*
 *  SeaShell_Adjust
 *
 *  this function drags the given handle to a new location and return the
 *  index of the handle that should continue being dragged
 *
 */
	static int
SeaShell_Adjust (
	SeaShellTool		*tool,
	LWToolEvent			*event,
	int					 i)
{
	return i;
}


/*
 *  SeaShell_Event
 *
 *  this function process a general event: DROP, RESET or ACTIVATE
 *
 */
	static void
SeaShell_Event (
	SeaShellTool		*tool,
	int                  code)
{
	switch (code) {
	case LWT_EVENT_DROP:
	  tool->update = LWT_TEST_REJECT;
	  break;
	case LWT_EVENT_RESET:
	  tool->update = LWT_TEST_UPDATE;
	  tool->axis   = DEF_AXIS;
	  tool->nrep   = DEF_NREP;
	  tool->nsid   = DEF_NSID;
	  tool->off    = DEF_OFF;
	  tool->uwrp   = DEF_UWRP;
	  tool->vwrp   = DEF_VWRP;
	  tool->scl    = DEF_SCL;
	  tool->vrot   = DEF_VROT;
	  break;
	case LWT_EVENT_ACTIVATE:
	  tool->update = LWT_TEST_UPDATE;
	  break;
	}
}


/*
 *  SeaShell_Done
 *
 *  this function process a general event: DROP, RESET or ACTIVATE
 *
 */
	static void
SeaShell_Done (
	SeaShellTool		*tool)
{
	if (tool->points0) free(tool->points0);
	if (tool->faces)   free(tool->faces);
	if (tool->vert)    free(tool->vert);
	free (tool);
}


/*
 *  SeaShell_Panel
 *
 *  this function creates and returns a view-type xPanel for the tool instance.
 *
 */
	static LWXPanelID
SeaShell_Panel (
	SeaShellTool			*tool)
{
	LWXPanelID		 		 pan_id;

	static const char       *axis[] = 
		{"X", "Y", "Z", NULL };
	static LWXPanelDataDesc	 def[] = {
		{ XID_AXIS,  "Axis"          , "integer"  },
		{ XID_NREP,  "# of Loops"    , "integer"  },
		{ XID_NSID,  "Sides per Loop", "integer"  },
		{ XID_OFF ,  "Shift per Loop", "integer"  },
		{ XID_SCL ,  "Scale per Loop", "float"    },
		{ XID_TXUV,  "Make UVs"      , "integer"  },
		{ XID_UWRP,  "U Wrap Amount" , "float"    },
		{ XID_VWRP,  "V Wrap Amount" , "float"    },
		{ XID_VROT,  "Rotate V Wrap" , "integer"  },
		{ 0 }
	};
	static LWXPanelControl ctl[] = {
		{ XID_AXIS,  "Axis"          , "iPopChoice" },
		{ XID_NREP,  "# of Loops"    , "integer"    },
		{ XID_NSID,  "Sides per Loop", "integer"    },
		{ XID_OFF ,  "Shift per Loop", "integer"    },
		{ XID_SCL ,  "Scale per Loop", "float"      },
		{ XID_TXUV,  "Make UVs"      , "iBoolean"   },
		{ XID_UWRP,  "U Wrap Amount" , "float"      },
		{ XID_VWRP,  "V Wrap Amount" , "float"      },
		{ XID_VROT,  "Rotate V Wrap" , "integer"    },
		{ 0 }
	};
	static LWXPanelHint hint[] = {
		XpSTRLIST(XID_AXIS, axis),
		XpMIN(XID_NREP, 1),
		XpMIN(XID_NSID, 1),
		XpMIN(XID_OFF , 1),
		XpMIN(XID_VROT, 0),
		XpEND
	};

	pan_id = (*tool->panel->create) (LWXP_VIEW, ctl);
	if (!pan_id)
		return NULL;

	(*tool->panel->hint) (pan_id, 0, hint);
	(*tool->panel->describe) (pan_id, def, SeaShell_Get, SeaShell_Set);
	return pan_id;
}


	static void *
SeaShell_Get (
	void				*data,
	unsigned long		 vid)
{
	SeaShellTool	*tool = (SeaShellTool *) data;

	switch (vid)
	{
		case XID_AXIS:
			return &tool->axis;
		case XID_NREP:
			return &tool->nrep;
		case XID_NSID:
			return &tool->nsid;
		case XID_OFF:
			return &tool->off;
		case XID_SCL:
			return &tool->scl;
		case XID_TXUV:
			return &tool->uvs;
		case XID_UWRP:
			return &tool->uwrp;
		case XID_VWRP:
			return &tool->vwrp;
		case XID_VROT:
			return &tool->vrot;
		default:
			return NULL;
	}
	return NULL;
}


	static int
SeaShell_Set (
	void				*data,
	unsigned long		 vid,
	void				*value)
{
	SeaShellTool	*tool = (SeaShellTool *) data;
	int				*i = (int   *) value;
	double			*v = (double*) value;

	switch (vid )
	{
		case XID_AXIS:
		  tool->axis = i[0];
		  CLAMP(tool->axis,0,2);
		  break;
		case XID_NREP:
		  tool->nrep = i[0];
		  break;
		case XID_NSID:
		  tool->nsid = i[0];
		  break;
		case XID_OFF:
		  tool->off  = i[0];
		  break;
		case XID_SCL:
		  tool->scl  = v[0];
		  break;
		case XID_TXUV:
		  tool->uvs  = i[0];
		  break;
		case XID_UWRP:
		  tool->uwrp = v[0];
		  break;
		case XID_VWRP:
		  tool->vwrp = v[0];
		  break;
		case XID_VROT:
		  tool->vrot = i[0];
		  break;
		default:
		  return 0;
	}

	tool->dirty  = 1;
	tool->update = LWT_TEST_UPDATE;
	return 1;
}



/*
 * the callback routine to scan in all of polygons.
 */
	static EDError
SeaShell_GetProfile (
	SeaShellTool		*tool,
	const EDPolygonInfo	*info)
{
	EDPointInfo			*pi;
	LWPntID				pnt;
    MeshEditOp          *op = tool->op;
	int					i, idx;

	idx = tool->vrot % info->numPnts;

	for (i = 0; i < info->numPnts; i++)
	{
		pi = (*op->pointInfo) (op->state, info->points[idx]);
		tool->points0[tool->numPnts]    = info->points[idx];
		tool->vert[tool->numPnts].co[0] = pi->position[0];
		tool->vert[tool->numPnts].co[1] = pi->position[1];
		tool->vert[tool->numPnts].co[2] = pi->position[2];
		tool->numPnts++;
		idx++; if (idx == info->numPnts) idx = 0;
	}

	if (tool->uvs) 
	{
		idx = tool->vrot % info->numPnts;
		pi  = (*op->pointInfo) (op->state, info->points[idx]);
		pnt = (*op->addPoint) (op->state, pi->position);
		tool->points0[tool->numPnts]    = pnt;
		tool->vert[tool->numPnts].co[0] = pi->position[0];
		tool->vert[tool->numPnts].co[1] = pi->position[1];
		tool->vert[tool->numPnts].co[2] = pi->position[2];
		tool->numPnts++;
	}

	tool->faces[tool->numPols].numPnts = tool->uvs ? (info->numPnts+1) : info->numPnts;
	tool->faces[tool->numPols].surface = (char *)info->surface;
	tool->faces[tool->numPols].type    = info->type;
	tool->numPols++;

	return EDERR_NONE;
}



/*
 * point translation function.
 */
	static void
SeaShell_TransPoint (
	int					axis,
	double				rot,
	double				scal,
	double				cen,
	double				*sco,
	double				*dco)
{
	if (axis == 0)
	{
		dco[1] = (sco[1] * scal * cos(rot) - sco[2] * scal * sin(rot)) * (-1);
		dco[0] = (sco[0] - cen) * scal + cen;
		dco[2] =  sco[1] * scal * sin(rot) + sco[2] * scal * cos(rot);
	}
	else if (axis == 1)
	{
		dco[0] =  sco[0] * scal * cos(rot) - sco[2] * scal * sin(rot);
		dco[1] = (sco[1] - cen) * scal + cen;
		dco[2] = (sco[0] * scal * sin(rot) + sco[2] * scal * cos(rot)) * (-1);
	}
	else if (axis == 2)
	{
		dco[0] = (sco[0] * scal * cos(rot) - sco[1] * scal * sin(rot)) * (-1);
		dco[2] = (sco[2] - cen) * scal + cen;
		dco[1] =  sco[0] * scal * sin(rot) + sco[1] * scal * cos(rot);
	}
}


	static EDError
SeaShell_GetIndexCount (
	int					*count,
	const EDPolygonInfo	*info)
{
	*count += info->numPnts + 1;
	return EDERR_NONE;
}

/*
 * Globals necessary to declare the class and name of this plugin server.
 */
char		ServerClass[] = LWMESHEDITTOOL_CLASS;
char		ServerName[]  = "SeaShellTool";
