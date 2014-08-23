/*
 * TOOL.C -- Spikey operation as an interactive mesh-edit tool.
 *
 * Copyright 1999, NewTek, Inc.
 * written by Stuart Ferguson
 * last revision  8/30/99
 */
#include <lwsdk/lwserver.h>
#include <lwsdk/lwmodtool.h>
#include "spikey.h"


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
char		ServerClass[] = LWMESHEDITTOOL_CLASS;
char		ServerName[]  = "LWSpikeyTool";
