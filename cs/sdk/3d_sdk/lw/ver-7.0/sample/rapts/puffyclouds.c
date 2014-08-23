/*
 * puffyclouds.c --  Layout Procedural Texture Plugin
 *
 * based on the puffycloud.sl Renderman shader written by F Kenton Musgrave
 *
 * by Marvin Landis
 * last revision  1/25/2000
 */

#include "pts_frctl.h"
#include "pts_gui.h"
#include "pts_math.h"

#include "puffyclouds.h"

#define PC_VERSION        1

static LWEnvelopeFuncs  *evfunc = NULL;
static LWTextureFuncs   *txfunc = NULL;
static LWVParmFuncs     *vpfunc = NULL;
extern LWXPanelFuncs    *xpfunc;


/*
 * Create() Handler callback.
 *
 * Allocate and initialize instance data.
 * The context for a procedural is the texture layer it belongs to.
 */
XCALL_(static LWInstance)
PuffyCloudsCreate (void *priv, LWTLayerID txtrLayer, LWError *err)
{
  PuffyClouds		*inst = NULL;
  LWChanGroupID		 parent = NULL;


  // Allocate instance data.
  inst = malloc (sizeof (PuffyClouds));
  if (!inst) {
    *err = "No memory for new PuffyClouds texture.";
    return NULL;
  }

  // Initialize the current version number
  inst->tversion = PC_VERSION;

  // The default Noise Type.
  inst->fnoise = PNOISE;

  // Initialize temporary variables, these values will be used to
  // initialize the LWVParmID variables.
  inst->thr[0] = 0.0; inst->thr[1] = 0.0; inst->thr[2] = 0.0;
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

  if (inst->threshold = (*vpfunc->create) (LWVP_FLOAT, LWVPDT_NOTXTR)) {
    inst->thrgrp = (*evfunc->createGroup) (parent, "PC_Threshold");
    (*vpfunc->setup) (inst->threshold, "Threshold", inst->thrgrp,
      NULL, NULL, NULL, NULL );
    (*vpfunc->setVal) (inst->threshold, inst->thr);
  }
  if (inst->increment = (*vpfunc->create) (LWVP_FLOAT, LWVPDT_NOTXTR)) {
    inst->incgrp = (*evfunc->createGroup) (parent, "PC_Increment");
    (*vpfunc->setup) (inst->increment, "Increment", inst->incgrp,
      NULL, NULL, NULL, NULL);
    (*vpfunc->setVal) (inst->increment, inst->inc);
  }
  if (inst->lacunarity = (*vpfunc->create) (LWVP_FLOAT, LWVPDT_NOTXTR)) {
    inst->lacgrp = (*evfunc->createGroup) (parent, "PC_Lacunarity");
    (*vpfunc->setup) (inst->lacunarity, "Lacunarity", inst->lacgrp,
      NULL, NULL, NULL, NULL);
    (*vpfunc->setVal) (inst->lacunarity, inst->lac);
  }
  if (inst->octaves = (*vpfunc->create) (LWVP_FLOAT, LWVPDT_NOTXTR)) {
    inst->octgrp = (*evfunc->createGroup) (parent, "PC_Octaves");
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
PuffyCloudsDestroy (PuffyClouds *inst)
{
  if (inst) {

    // Free the LWVParmID memory
    if (inst->threshold)
      (*vpfunc->destroy) (inst->threshold);
    if (inst->increment)
      (*vpfunc->destroy) (inst->increment);
    if (inst->lacunarity)
      (*vpfunc->destroy) (inst->lacunarity);
    if (inst->octaves)
      (*vpfunc->destroy) (inst->octaves);

    // Destroy the LWChanGroupIDs
    if (inst->thrgrp)
      (*evfunc->destroyGroup) (inst->thrgrp);
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
PuffyCloudsLoad (PuffyClouds *inst, const LWLoadState *lState)
{
  LWError     err;
  short	      fn;

  // Exit if it's not the same version number (defaults will be used). 
  LWLOAD_I2 (lState, &inst->tversion, 1);
  if (inst->tversion != PC_VERSION) {
    inst->tversion = PC_VERSION;
    return NULL;
  }

  LWLOAD_I2 (lState, &fn, 1);
  inst->fnoise = (FracNoise) fn;

  if (!(err = (*vpfunc->load) (inst->threshold, lState)))
    (*vpfunc->getVal) (inst->threshold, 0, NULL, inst->thr);
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
PuffyCloudsSave (PuffyClouds *inst, const LWSaveState *sState)
{
  LWError    err;
  short      fn;

  LWSAVE_I2 (sState, &inst->tversion, 1);

  fn = (short) inst->fnoise;
  LWSAVE_I2 (sState, &fn, 1);

  if (err = (*vpfunc->save) (inst->threshold, sState))
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
PuffyCloudsCopy (PuffyClouds *to, PuffyClouds *from)
{
  LWError	 err;
  LWVParmID	 vpid1, vpid2, vpid3, vpid4;
  LWChanGroupID  gpid1, gpid2, gpid3, gpid4;

  gpid1 = to->thrgrp;
  gpid2 = to->incgrp;
  gpid3 = to->lacgrp;
  gpid4 = to->octgrp;
  vpid1 = to->threshold;
  vpid2 = to->increment;
  vpid3 = to->lacunarity;
  vpid4 = to->octaves;

  *to = *from;

  to->thrgrp = gpid1;
  to->incgrp = gpid2;
  to->lacgrp = gpid3;
  to->octgrp = gpid4;
  to->threshold = vpid1;
  to->increment = vpid2;
  to->lacunarity = vpid3;
  to->octaves = vpid4;

  // Use the LWVParmID copy function to handle these allocated resources.
  if (err = (*vpfunc->copy)(to->threshold, from->threshold))
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
PuffyCloudsDescLn (PuffyClouds *inst)
{
  sprintf (inst->desc, "%4.1f %4.1f %4.1f %4.1f",
           inst->thr[0], inst->inc[0], inst->lac[0], inst->oct[0] );

  return inst->desc;
}


/*
 * Init() Handler callback.
 *
 * Called at the start of rendering.
 */
XCALL_(static LWError)
PuffyCloudsInit (PuffyClouds *inst, int rtype)
{
  return (NULL);
}


/*
 * Cleanup() Handler callback.
 *
 * Called at the end of rendering.
 */
XCALL_(static void)
PuffyCloudsCleanup (PuffyClouds *inst)
{
  return;
}


/*
 * NewTime() Handler callback.
 *
 * Called at the start of each rendering pass.
 */
XCALL_(static LWError)
PuffyCloudsNewTime (PuffyClouds *inst, LWFrame f, LWTime t)
{
  // Put the current LWVParmID values into the temporary instance 
  // variables for easy access during the Evaluate stage.
  if (inst->threshold)
    (*vpfunc->getVal) (inst->threshold, t, NULL, inst->thr);
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
PuffyCloudsFlags (PuffyClouds *inst)
{
  // Make texture available for displacements
  return (LWTEXF_HV_SRF | LWTEXF_HV_VOL | LWTEXF_DISPLACE);
}


/*
 * Evaluate() Handler callback.
 *
 * Calculation of the procedural texture.
 */
XCALL_(static double)
PuffyCloudsEvaluate (PuffyClouds *inst, LWTextureAccess *ta)
{
  double     turb;
  double     PP[3];

  // Lets work in texture space
  Vec3Assign(PP, ta->tPos[0], ta->tPos[1], ta->tPos[2]);

  // Use the fBm function to get some fractal turbulence
  turb = fBm(PP, inst->inc[0], inst->lac[0], inst->oct[0], inst->fnoise);

  // Smooth the "edge" of the turbulence based on the threshold value
  turb = smoothstep(inst->thr[0], 1, turb);

  return turb;
}


/*
 * Activate() Handler activation function.
 *
 * Check the version, get some globals, and fill in the callback fields
 * of the handler structure.
 */
XCALL_(int)
PuffyCloudsActivate (long version, GlobalFunc *global, LWTextureHandler *local, void *serverData)
{
  if (version != LWTEXTURE_VERSION)
    return (AFUNC_BADVERSION);

  evfunc = global(LWENVELOPEFUNCS_GLOBAL, GFUSE_TRANSIENT);
  txfunc = global(LWTEXTUREFUNCS_GLOBAL, GFUSE_TRANSIENT);
  vpfunc = global(LWVPARMFUNCS_GLOBAL, GFUSE_TRANSIENT);

  if (!evfunc || !txfunc || !vpfunc ) 
    return AFUNC_BADGLOBAL;

  local->inst->create   = PuffyCloudsCreate;
  local->inst->destroy  = PuffyCloudsDestroy;
  local->inst->load     = PuffyCloudsLoad;
  local->inst->save     = PuffyCloudsSave;
  local->inst->copy     = PuffyCloudsCopy;
  local->inst->descln   = PuffyCloudsDescLn;

  local->rend->init     = PuffyCloudsInit;
  local->rend->cleanup  = PuffyCloudsCleanup;
  local->rend->newTime  = PuffyCloudsNewTime;

  local->flags    = PuffyCloudsFlags;
  local->evaluate = PuffyCloudsEvaluate;

  return (AFUNC_OK);
}



/*
 * Interface Declarations
 */

/*
 * Get() Xpanel VIEW Interface Callback.
 *
 * Retrieve the value of a control from the instance data.
 */
void *
PC_view_get ( PuffyClouds *inst, unsigned long vid ) 
{
  void *result = NULL;
  static int ival = 0; // static so persist after function returns
 
  if ( !inst ) return result;

  switch ( vid ) {
    case (OPTS_THRESHOLD):
      result = inst->threshold;
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
PC_view_set ( PuffyClouds *inst, unsigned long vid, void *value )
{
  int result = 0;

  if ( !inst ) return result;

  switch ( vid ) {
    case (OPTS_THRESHOLD):
      // inst->threshold should already contain the correct value
      (*vpfunc->getVal) (inst->threshold, 0, NULL, inst->thr);
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
PC_view_destroy ( void *myData )
{
  PuffyClouds	*inst = (PuffyClouds*)myData;

  return;
}


/*
 * NonModal VIEW Panel Interface
 *
 * NonModal interfaces are created on activation.
 */
int
PuffyCloudsInterface (long version, GlobalFunc *global, LWInterface *iface,
  void *serverdata )
{
  int            rc = AFUNC_OK;
  LWXPanelID     panID = NULL;
  PuffyClouds	*inst = NULL;

  // These are the control lists and configuration hints
  static LWXPanelControl PC_ctrl_list[] = {
   { OPTS_THRESHOLD,  "Threshold",   "float-env" },
   { OPTS_INCREMENT,  "Increment",   "float-env" },
   { OPTS_LACUNARITY, "Lacunarity",  "float-env" },
   { OPTS_OCTAVES,    "Octaves",     "float-env" },
   { OPTS_FNOISE,     "Noise Type",  "iPopChoice" },
   {0}
  };

  static LWXPanelDataDesc PC_data_descrip[] = {
   { OPTS_THRESHOLD,  "Threshold",   "float-env" },
   { OPTS_INCREMENT,  "Increment",   "float-env" },
   { OPTS_LACUNARITY, "Lacunarity",  "float-env" },
   { OPTS_OCTAVES,    "Octaves",     "float-env" },
   { OPTS_FNOISE,     "Noise Type",  "integer" },
   {0}
  };

  static LWXPanelHint PC_hint[] = {
    XpSTRLIST(OPTS_FNOISE, FNoise),
    XpRANGE(OPTS_THRESHOLD, -5, 5, 1),
    XpRANGE(OPTS_INCREMENT, 0, 5, 1),
    XpRANGE(OPTS_LACUNARITY, 0, 6, 1),
    XpRANGE(OPTS_OCTAVES, 0, 10, 1),
    XpH(OPTS_THRESHOLD), XpEND,
    XpH(OPTS_INCREMENT), XpEND,
    XpH(OPTS_LACUNARITY), XpEND,
    XpH(OPTS_OCTAVES), XpEND,
    XpH(OPTS_FNOISE), XpEND,
    XpLABEL(0,"PuffyClouds"),
    XpDESTROYNOTIFY(PC_view_destroy),
    XpEND
  };


  // Check the version
  // NOTE: The interface version is different from handler version
  if ( version != LWINTERFACE_VERSION )
    return (AFUNC_BADVERSION);

  xpfunc = global(LWXPANELFUNCS_GLOBAL, GFUSE_TRANSIENT);

  if ( !xpfunc )
    return AFUNC_BADGLOBAL;

  inst = (PuffyClouds *) iface->inst;

  // If we already have an XPanel which has not been destroyed,
  // we can destroy it and create another, or return the existing instance.
    // Create panel
    panID = (*xpfunc->create)( LWXP_VIEW, PC_ctrl_list );
    if (panID) {

      // Apply some hints
      (*xpfunc->hint) ( panID, 0, PC_hint );

      // Describe the structure of the data instance to the panel
      (*xpfunc->describe)( panID, PC_data_descrip, PC_view_get, PC_view_set );

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
