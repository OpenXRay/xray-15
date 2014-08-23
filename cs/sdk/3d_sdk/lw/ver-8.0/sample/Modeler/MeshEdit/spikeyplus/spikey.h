/*
 * SPIKEY.H -- Spikey modeler plug-in header.
 *
 * Copyright 1999, NewTek, Inc.
 * written by Stuart Ferguson
 * last revision  8/30/99
 */
#include <lwsdk/lwmeshedt.h>
#include <lwsdk/lwxpanel.h>
#include <lwsdk/lwdyna.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

extern DynaMonitorFuncs	*globFun_mon;
extern LWXPanelFuncs	*globFun_pan;

extern int		 Spikey (MeshEditOp *, double);

#define PTYP_SPIKEY LWID_('S','P','I','K')