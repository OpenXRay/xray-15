/*
 * coriolis.c --  Layout Procedural Texture Plugin
 *
 * based on the coriolis.sl Renderman shader written by F Kenton Musgrave
 *
 * by Marvin Landis
 * last revision  1/25/2000
 */

#include "pts_frctl.h"
#include "pts_gui.h"
#include "pts_math.h"

#include "coriolis.h"

#define CS_VERSION        1

static LWEnvelopeFuncs  *evfunc = NULL;
static LWTextureFuncs   *txfunc = NULL;
static LWVParmFuncs     *vpfunc = NULL;
LWXPanelFuncs    *xpfunc = NULL;


/*
 * Create() Handler callback.
 *
 * Allocate and initialize instance data.
 */
XCALL_(static LWInstance)
CoriolisCreate (void *priv, LWTLayerID txtrLayer, LWError *err)
{
  Coriolis        *inst = NULL;
  LWChanGroupID       parent = NULL;


  // Allocate instance data.
  inst = malloc (sizeof (Coriolis));
  if (!inst) {
	 *err = "No memory for new Coriolis texture.";
	 return NULL;
  }

  // Initialize the current version number
  inst->tversion = CS_VERSION;

  // The default Noise Type.
  inst->fnoise = PNOISE;

  // Initialize temporary variables, these values will be used to
  // initialize the LWVParmID variables.
  inst->scl[0] = 0.6; inst->scl[1] = 0.0; inst->scl[2] = 0.0;
  inst->tws[0] = 1.0; inst->tws[1] = 0.0; inst->tws[2] = 0.0;
  inst->off[0] = 0.5; inst->off[1] = 0.0; inst->off[2] = 0.0;
  inst->inc[0] = 0.5; inst->inc[1] = 0.0; inst->inc[2] = 0.0;
  inst->lac[0] = 2.0; inst->lac[1] = 0.0; inst->lac[2] = 0.0;
  inst->oct[0] = 6.0; inst->oct[1] = 0.0; inst->oct[2] = 0.0;

  // Create the LWVParmID and an envelope group for each parameter.
  // Initialize the LWVParmID with the values assigned to the
  // temporary variables above.

  // parent is the texture layer group, this groups belongs to the texture group
  // which might be also part of a bigger hierarchy (surface group, etc.)  
  if (txtrLayer)
	 parent = (*txfunc->layerEnvGroup)(txtrLayer);

  if (inst->coriolisscale = (*vpfunc->create) (LWVP_FLOAT, LWVPDT_NOTXTR)) {
	 inst->sclgrp = (*evfunc->createGroup) (parent, "CS_Scale");
	 (*vpfunc->setup) (inst->coriolisscale, "Scale", inst->sclgrp,
      NULL, NULL, NULL, NULL);
	 (*vpfunc->setVal) (inst->coriolisscale, inst->scl);
  }
  if (inst->coriolistwist = (*vpfunc->create) (LWVP_FLOAT, LWVPDT_NOTXTR)) {
	 inst->twsgrp = (*evfunc->createGroup) (parent, "CS_Twist");
	 (*vpfunc->setup) (inst->coriolistwist, "Twist", inst->twsgrp,
      NULL, NULL, NULL, NULL);
	 (*vpfunc->setVal) (inst->coriolistwist, inst->tws);
  }
  if (inst->coriolisoffset = (*vpfunc->create) (LWVP_FLOAT, LWVPDT_NOTXTR)) {
	 inst->offgrp = (*evfunc->createGroup) (parent, "CS_Offset");
	 (*vpfunc->setup) (inst->coriolisoffset, "Offset", inst->offgrp,
      NULL, NULL, NULL, NULL);
	 (*vpfunc->setVal) (inst->coriolisoffset, inst->off);
  }
  if (inst->increment = (*vpfunc->create) (LWVP_FLOAT, LWVPDT_NOTXTR)) {
	 inst->incgrp = (*evfunc->createGroup) (parent, "CS_Increment");
	 (*vpfunc->setup) (inst->increment, "Increment", inst->incgrp,
      NULL, NULL, NULL, NULL);
	 (*vpfunc->setVal) (inst->increment, inst->inc);
  }
  if (inst->lacunarity = (*vpfunc->create) (LWVP_FLOAT, LWVPDT_NOTXTR)) {
	 inst->lacgrp = (*evfunc->createGroup) (parent, "CS_Lacunarity");
	 (*vpfunc->setup) (inst->lacunarity, "Lacunarity", inst->lacgrp,
      NULL, NULL, NULL, NULL);
	 (*vpfunc->setVal) (inst->lacunarity, inst->lac);
  }
  if (inst->octaves = (*vpfunc->create) (LWVP_FLOAT, LWVPDT_NOTXTR)) {
	 inst->octgrp = (*evfunc->createGroup) (parent, "CS_Octaves");
	 (*vpfunc->setup) (inst->octaves, "Octaves", inst->octgrp,
      NULL, NULL, NULL, NULL);
	 (*vpfunc->setVal) (inst->octaves, inst->oct);
  }

  return inst;
}


/*
 * Destroy() Handler callback.
 *
 * Free resources allocated by Create().
 */
XCALL_(static void)
CoriolisDestroy (Coriolis *inst)
{
  if (inst) {

	// Free the LWVParmID memory
	 if (inst->coriolisscale)
		(*vpfunc->destroy) (inst->coriolisscale);
	 if (inst->coriolistwist)
		(*vpfunc->destroy) (inst->coriolistwist);
	 if (inst->coriolisoffset)
		(*vpfunc->destroy) (inst->coriolisoffset);
	 if (inst->increment)
		(*vpfunc->destroy) (inst->increment);
	 if (inst->lacunarity)
		(*vpfunc->destroy) (inst->lacunarity);
	 if (inst->octaves)
		(*vpfunc->destroy) (inst->octaves);

	 // Destroy the LWChanGroupIDs
	 if (inst->sclgrp)
		(*evfunc->destroyGroup) (inst->sclgrp);
	 if (inst->twsgrp)
		(*evfunc->destroyGroup) (inst->twsgrp);
	 if (inst->offgrp)
		(*evfunc->destroyGroup) (inst->offgrp);
	 if (inst->incgrp)
		(*evfunc->destroyGroup) (inst->incgrp);
	 if (inst->lacgrp)
		(*evfunc->destroyGroup) (inst->lacgrp);
	 if (inst->octgrp)
		(*evfunc->destroyGroup) (inst->octgrp);

	 // Now free the entire instance.
	 free (inst);
  }
}


/*
 * Load() Handler callback.
 *
 * Read the instance data.
 */
XCALL_(static LWError)
CoriolisLoad (Coriolis *inst, const LWLoadState *lState)
{
  LWError     err;
  short        fn;

  // Exit if it's not the same version number (defaults will be used). 
  LWLOAD_I2 (lState, &inst->tversion, 1);
  if (inst->tversion != CS_VERSION) {
	 inst->tversion = CS_VERSION;
	 return NULL;
  }

  LWLOAD_I2 (lState, &fn, 1);
  inst->fnoise = (FracNoise) fn;

  if (!(err = (*vpfunc->load) (inst->coriolisscale, lState)))
	 (*vpfunc->getVal) (inst->coriolisscale, 0, NULL, inst->scl);
  else
	 return (err);

  if (!(err = (*vpfunc->load) (inst->coriolistwist, lState)))
	 (*vpfunc->getVal) (inst->coriolistwist, 0, NULL, inst->tws);
  else
	 return (err);

  if (!(err = (*vpfunc->load) (inst->coriolisoffset, lState)))
	 (*vpfunc->getVal) (inst->coriolisoffset, 0, NULL, inst->off);
  else
	 return (err);

  if (!(err = (*vpfunc->load) (inst->increment, lState)))
	 (*vpfunc->getVal) (inst->increment, 0, NULL, inst->inc);
  else
	 return (err);

  if (!(err = (*vpfunc->load) (inst->lacunarity, lState)))
	 (*vpfunc->getVal) (inst->lacunarity, 0, NULL, inst->lac);
  else
	 return (err);

  if (!(err = (*vpfunc->load) (inst->octaves, lState)))
	 (*vpfunc->getVal) (inst->octaves, 0, NULL, inst->oct);
  else
	 return (err);

  return (NULL);
}


/*
 * Save() Handler callback.
 *
 * Write the instance data.
 */
XCALL_(static LWError)
CoriolisSave (Coriolis *inst, const LWSaveState *sState)
{
  LWError    err;
  short      fn;

  LWSAVE_I2 (sState, &inst->tversion, 1);

  fn = (short) inst->fnoise;
  LWSAVE_I2 (sState, &fn, 1);

  if (err = (*vpfunc->save) (inst->coriolisscale, sState))
	 return (err);
  if (err = (*vpfunc->save) (inst->coriolistwist, sState))
	 return (err);
  if (err = (*vpfunc->save) (inst->coriolisoffset, sState))
	 return (err);
  if (err = (*vpfunc->save) (inst->increment, sState))
	 return (err);
  if (err = (*vpfunc->save) (inst->lacunarity, sState))
	 return (err);
  if (err = (*vpfunc->save) (inst->octaves, sState))
	 return (err);

  return (NULL);
}


/*
 * Copy() Handler callback.
 *
 * Copy instance data.  The instance data contains allocated
 * resources (LWVParmID and LWChanGroupID variables), so a simple
 * *to = *from is insufficient.
 */
XCALL_(static LWError)
CoriolisCopy (Coriolis *to, Coriolis *from)
{
  LWError    err;
  LWVParmID  vpid1, vpid2, vpid3, vpid4, vpid5, vpid6;
  LWChanGroupID  gpid1, gpid2, gpid3, gpid4, gpid5, gpid6;

  gpid1 = to->sclgrp;
  gpid2 = to->twsgrp;
  gpid3 = to->offgrp;
  gpid4 = to->incgrp;
  gpid5 = to->lacgrp;
  gpid6 = to->octgrp;
  vpid1 = to->coriolisscale;
  vpid2 = to->coriolistwist;
  vpid3 = to->coriolisoffset;
  vpid4 = to->increment;
  vpid5 = to->lacunarity;
  vpid6 = to->octaves;

  *to = *from;

  to->sclgrp = gpid1;
  to->twsgrp = gpid2;
  to->offgrp = gpid3;
  to->incgrp = gpid4;
  to->lacgrp = gpid5;
  to->octgrp = gpid6;
  to->coriolisscale = vpid1;
  to->coriolistwist = vpid2;
  to->coriolisoffset = vpid3;
  to->increment = vpid4;
  to->lacunarity = vpid5;
  to->octaves = vpid6;

  // Use the LWVParmID copy function to handle these allocated resources.
  if (err = (*vpfunc->copy)(to->coriolisscale, from->coriolisscale))
	 return (err);
  if (err = (*vpfunc->copy)(to->coriolistwist, from->coriolistwist))
	 return (err);
  if (err = (*vpfunc->copy)(to->coriolisoffset, from->coriolisoffset))
	 return (err);
  if (err = (*vpfunc->copy)(to->increment, from->increment))
	 return (err);
  if (err = (*vpfunc->copy)(to->lacunarity, from->lacunarity))
	 return (err);
  if (err = (*vpfunc->copy)(to->octaves, from->octaves))
	 return (err);

  return (NULL);
}


/*
 * DescLn() Handler callback.
 *
 * Write a short, human-readable string describing the instance data.
 */
XCALL_(static const char *)
CoriolisDescLn (Coriolis *inst)
{
  sprintf (inst->desc, "%4.1f %4.1f %4.1f",
			  inst->scl[0], inst->tws[0], inst->off[0] );

  return inst->desc;
}


/*
 * Init() Handler callback.
 *
 * Called at the start of rendering.
 */
XCALL_(static LWError)
CoriolisInit (Coriolis *inst, int rtype)
{
  return (NULL);
}


/*
 * Cleanup() Handler callback.
 *
 * Called at the end of rendering.
 */
XCALL_(static void)
CoriolisCleanup (Coriolis *inst)
{
  return;
}


/*
 * NewTime() Handler callback.
 *
 * Called at the start of each rendering pass.
 */
XCALL_(static LWError)
CoriolisNewTime (Coriolis *inst, LWFrame f, LWTime t)
{
  // Put the current LWVParmID values into the temporary instance 
  // variables for easy access during the Evaluate stage.
  if (inst->coriolisscale)
	 (*vpfunc->getVal) (inst->coriolisscale, t, NULL, inst->scl);
  if (inst->coriolistwist)
	 (*vpfunc->getVal) (inst->coriolistwist, t, NULL, inst->tws);
  if (inst->coriolisoffset)
	 (*vpfunc->getVal) (inst->coriolisoffset, t, NULL, inst->off);
  if (inst->increment)
	 (*vpfunc->getVal) (inst->increment, t, NULL, inst->inc);
  if (inst->lacunarity)
	 (*vpfunc->getVal) (inst->lacunarity, t, NULL, inst->lac);
  if (inst->octaves)
	 (*vpfunc->getVal) (inst->octaves, t, NULL, inst->oct);

  return (NULL);
}


/*
 * Flags() Handler callback.
 *
 * Evaluation flags passed to the server at Evaluate stage.
 */
XCALL_(static unsigned int)
CoriolisFlags (Coriolis *inst)
{
  return (LWTEXF_HV_SRF | LWTEXF_HV_VOL | LWTEXF_DISPLACE | LWTEXF_AXIS);
}


/*
 * Evaluate() Handler callback.
 *
 * Calculation of the procedural texture.
 */
XCALL_(static double)
CoriolisEvaluate (Coriolis *inst, LWTextureAccess *ta)
{
  /* Local stuff */
  double  rsq, angle, value, sine, cosine, turb;

  /* Position Stuff */
  double  Pt[3], PtN[3], PP[3];

  // Lets work in shader space
  if (ta->axis == 0) {
	 Vec3Assign(Pt, ta->tPos[2], -ta->tPos[1], ta->tPos[0]);
  } else if (ta->axis == 1) {
	 Vec3Assign(Pt, ta->tPos[0], -ta->tPos[2], ta->tPos[1]);
  } else {
		Vec3Assign(Pt, ta->tPos[0], -ta->tPos[1], ta->tPos[2]);
  }

  Vec3Copy(PtN, Pt);
  normalize3(PtN);
	
  rsq = xcomp(PtN) * xcomp(PtN) + ycomp(PtN) * ycomp(PtN);

  angle = inst->tws[0] * rsq;

  sine   = sin( angle );
  cosine = cos( angle );
  PP[0] = Pt[0]*cosine - Pt[1]*sine;
  PP[1] = Pt[0]*sine + Pt[1]*cosine;
  PP[2] = Pt[2];

  turb = fBm(PP, inst->inc[0], inst->lac[0], inst->oct[0], inst->fnoise);
  value = Abs(inst->off[0] + inst->scl[0] * turb);
  value = clamp(value, 0, 1);

  return value;
}


/*
 * Activate() Handler activation function.
 *
 * Check the version, get some globals, and fill in the callback fields
 * of the handler structure.
 */
XCALL_(int)
CoriolisActivate (long version, GlobalFunc *global, LWTextureHandler *local,
  void *serverData)
{
  if (version != LWTEXTURE_VERSION)
	 return (AFUNC_BADVERSION);

  evfunc = global(LWENVELOPEFUNCS_GLOBAL, GFUSE_TRANSIENT);
  txfunc = global(LWTEXTUREFUNCS_GLOBAL, GFUSE_TRANSIENT);
  vpfunc = global(LWVPARMFUNCS_GLOBAL, GFUSE_TRANSIENT);

  if (!evfunc || !txfunc || !vpfunc ) 
	 return AFUNC_BADGLOBAL;

  local->inst->create   = CoriolisCreate;
  local->inst->destroy  = CoriolisDestroy;
  local->inst->load     = CoriolisLoad;
  local->inst->save     = CoriolisSave;
  local->inst->copy     = CoriolisCopy;
  local->inst->descln   = CoriolisDescLn;

  local->rend->init     = CoriolisInit;
  local->rend->cleanup  = CoriolisCleanup;
  local->rend->newTime  = CoriolisNewTime;

  local->flags    = CoriolisFlags;
  local->evaluate = CoriolisEvaluate;

  return (AFUNC_OK);
}


/*
 * Xpanels callbacks
 */

/*
 * Get() Xpanel VIEW Interface Callback.
 *
 * Retrieve the value of a control from the instance data.
 */
void *
CS_view_get ( Coriolis *inst, unsigned long vid ) 
{
  void *result = NULL;
  static int ival = 0; // static so persist after function returns
 
  if ( !inst ) return result;

  switch ( vid ) {
	 case (OPTS_SCALE):
		result = inst->coriolisscale;
		break;
	 case (OPTS_TWIST):
		result = inst->coriolistwist;
		break;
	 case (OPTS_OFFSET):
		result = inst->coriolisoffset;
		break;
	 case (OPTS_INCREMENT):
		result = inst->increment;
		break;
	 case (OPTS_LACUNARITY):
		result = inst->lacunarity;
		break;
	 case (OPTS_OCTAVES):
		result = inst->octaves;
		break;
	 case (OPTS_FNOISE):
		ival = (int) inst->fnoise;
		result = &ival;
		break;
	 default:
		result=NULL; 
		break;
  }

  return result;
}

/*
 * Set() Xpanel VIEW Interface Callback.
 *
 * Set the instance data from the interface controls.
 */
int
CS_view_set ( Coriolis *inst, unsigned long vid, void *value )
{
  int result = 0;

  if ( !inst ) return result;

  switch ( vid ) {
	 case (OPTS_SCALE):
		// inst->coriolisscale should already contain the correct value
		(*vpfunc->getVal) (inst->coriolisscale, 0, NULL, inst->scl);
		result = 1;
		break;
	 case (OPTS_TWIST):
		// inst->coriolistwist should already contain the correct value
		(*vpfunc->getVal) (inst->coriolistwist , 0, NULL, inst->tws);
		result = 1;
		break;
	 case (OPTS_OFFSET):
		// inst->octaves should already contain the correct value
		(*vpfunc->getVal) (inst->coriolisoffset, 0, NULL, inst->off);
		result = 1;
		break;
	 case (OPTS_INCREMENT):
		// inst->increment should already contain the correct value
		(*vpfunc->getVal) (inst->increment, 0, NULL, inst->inc);
		result = 1;
		break;
	 case (OPTS_LACUNARITY):
		// inst->lacunarity should already contain the correct value
		(*vpfunc->getVal) (inst->lacunarity , 0, NULL, inst->lac);
		result = 1;
		break;
	 case (OPTS_OCTAVES):
		// inst->octaves should already contain the correct value
		(*vpfunc->getVal) (inst->octaves, 0, NULL, inst->oct);
		result = 1;
		break;
	 case (OPTS_FNOISE):
		inst->fnoise = *((int*)value);
		result = 1;
		break;
  }

  return result;
}

/*
 * Destroy() NonModal XPanel Interface Callback.
 *
 * The following routine is required for Nonmodal type panels.
 *
 * The LWXPanel is a LightWave system data object which we have
 * "customized" to suit our needs. As a result, LW may destroy
 * the LWXPanel under various circumstances. This callback is
 * called after the LWXPanel is destroyed whether by LW or by
 * this plugin's instance destroy method.
 *
 * The action here is to NULL out our pointer to the XPanel.
 * If this pointer is still valid during the plugin destroy method,
 * we need to destroy the LWXPanel ourselves. Meanwhile, if LW calls
 * the UI activation again, and our LWXPanel has not been destroyed,
 * we can return the existing panel rather than create a new LWXPanel
 * instance.
 */
void
CS_view_destroy ( void *myData )
{
  Coriolis *inst = (Coriolis*)myData;

  return;
}



/*
 * NonModal VIEW Panel Interface
 *
 * NonModal interfaces are created on activation.
 */
int
CoriolisInterface (long version, GlobalFunc *global, LWInterface *iface,
  void *serverdata )
{
  int        rc = AFUNC_OK;
  LWXPanelID     panID = NULL;
  Coriolis     *inst = NULL;

  // These are the control lists and configuration hints
  static LWXPanelControl CS_ctrl_list[] = {
	{ OPTS_SCALE,      "Coriolis Scale",   "float-env" },
	{ OPTS_TWIST,      "Coriolis Twist",   "float-env" },
	{ OPTS_OFFSET,     "Coriolis Offset",  "float-env" },
	{ OPTS_INCREMENT,  "Increment",        "float-env" },
	{ OPTS_LACUNARITY, "Lacunarity",       "float-env" },
	{ OPTS_OCTAVES,    "Octaves",          "float-env" },
	{ OPTS_FNOISE,     "Noise Type",       "iPopChoice" },
	{0}
  };

  static LWXPanelDataDesc CS_data_descrip[] = {
	{ OPTS_SCALE,      "Coriolis Scale",   "float-env" },
	{ OPTS_TWIST,      "Coriolis Twist",   "float-env" },
	{ OPTS_OFFSET,     "Coriolis Offset",  "float-env" },
	{ OPTS_INCREMENT,  "Increment",        "float-env" },
	{ OPTS_LACUNARITY, "Lacunarity",       "float-env" },
	{ OPTS_OCTAVES,    "Octaves",          "float-env" },
	{ OPTS_FNOISE,     "Noise Type",       "integer" },
	{0}
  };

  static LWXPanelHint CS_hint[] = {
	 XpRANGE(OPTS_SCALE, -10, 10, 1),
	 XpRANGE(OPTS_TWIST, -5, 5, 1),
	 XpRANGE(OPTS_OFFSET, -5, 5, 1),
	 XpH(OPTS_SCALE), XpEND,
	 XpH(OPTS_TWIST), XpEND,
	 XpH(OPTS_OFFSET), XpEND,
	 XpLABEL(0,"Coriolis"),
	 XpDESTROYNOTIFY(CS_view_destroy),
	 XpEND
  };

  // Check the version
  // NOTE: The interface version is different from handler version
  if ( version != LWINTERFACE_VERSION )
	 return (AFUNC_BADVERSION);

  xpfunc = global(LWXPANELFUNCS_GLOBAL, GFUSE_TRANSIENT);

  if ( !xpfunc )
	 return AFUNC_BADGLOBAL;

  inst = (Coriolis *) iface->inst;

  // If we already have an XPanel which has not been destroyed,
  // we can destroy it and create another, or return the existing instance.

	// Create panel
	 panID = (*xpfunc->create)( LWXP_VIEW, CS_ctrl_list );
	 if (panID) {

	  // Apply some hints
		(*xpfunc->hint) ( panID, 0, CS_hint );
		(*xpfunc->hint) ( panID, 0, OPTS3_hint );

	  // Describe the structure of the data instance to the panel
		(*xpfunc->describe)( panID, CS_data_descrip, CS_view_get, CS_view_set );

		// Now give the data instance to the panel
	  (*xpfunc->viewInst)( panID, inst );

		// Set the userdata
		(*xpfunc->setData)(panID, 0, inst);

	}

	 // Set the interface panel pointer
	 iface->panel = panID;


  // Return and let LightWave play with the panel :)
  return rc;
}
