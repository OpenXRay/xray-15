/*
 * MEDIT.C -- Spikey operation as a mesh-edit command.
 *
 * Copyright 1999, NewTek, Inc.
 * written by Stuart Ferguson
 * last revision  8/30/99
 */
#include <lwsdk/lwserver.h>
#include "spikey.h"


/*
 * Main entry point for mesh operation.
 */
	XCALL_(int)
Activate (
	long         version,
	GlobalFunc     *global,
	MeshEditBegin     *local,
	void        *serverData)
{
	MeshEditOp     *op;
	double          factor;

	/*
	 * Check interface version.
	 */
	if (version != LWMESHEDIT_VERSION)
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
	 * Attempt to begin edit operation.  If the startup fails we still
	 * return OK, but just don't do anything.
	 */
	op = (*local) (0, 0, OPSEL_USER);
	if (!op)
		return AFUNC_OK;

	factor = 2.0;
	Spikey (op, factor);

	return AFUNC_OK;
}


/*
 * Globals necessary to declare the class and name of this plugin server.
 */
char     ServerClass[] = LWMESHEDIT_CLASS;
char     ServerName[]  = "LWMakeSpikey";
