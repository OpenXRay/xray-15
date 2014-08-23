/*
 * fbmnoise.c --  Layout Procedural Texture Plugin
 *
 * based on fractal routines written by F Kenton Musgrave
 *
 * by Marvin Landis
 * last revision  1/25/2000
 */

#include "pts_frctl.h"
#include "pts_gui.h"
#include "pts_math.h"

#include "fbmnoise.h"

#define FN_VERSION        1

static LWEnvelopeFuncs  *evfunc = NULL;
static LWTextureFuncs   *txfunc = NULL;
static LWVParmFuncs     *vpfunc = NULL;
extern LWXPanelFuncs    *xpfunc;


/*
 * Create() Handler callback.
 *
 * Allocate and initialize instance data.
 */
XCALL_(static LWInstance)
fBmNoiseCreate (void *priv, LWTLayerID txtrLayer, LWError *err)
{
  fBmNoise			*inst = NULL;
  LWChanGroupID		 parent = NULL;


  // Allocate instance data.
  inst = malloc (sizeof (fBmNoise));
  if (!inst) {
    *err = "No memory for new fBmNoise texture.";
    return NULL;
  }

  // Initialize the current version number
  inst->tversion = FN_VERSION;

  // The default Noise Type.
  inst->fnoise = PNOISE;

  // Initialize temporary variables, these values will be used to
  // initialize the LWVParmID variables.
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

  if (inst->increment = (*vpfunc->create) (LWVP_FLOAT, LWVPDT_NOTXTR)) {
    inst->incgrp = (*evfunc->createGroup) (parent, "FN_Increment");
    (*vpfunc->setup) (inst->increment, "Increment", inst->incgrp,
      NULL, NULL, NULL, NULL );
    (*vpfunc->setVal) (inst->increment, inst->inc);
  }
  if (inst->lacunarity = (*vpfunc->create) (LWVP_FLOAT, LWVPDT_NOTXTR)) {
    inst->lacgrp = (*evfunc->createGroup) (parent, "FN_Lacunarity");
    (*vpfunc->setup) (inst->lacunarity, "Lacunarity", inst->lacgrp,
      NULL, NULL, NULL, NULL);
    (*vpfunc->setVal) (inst->lacunarity, inst->lac);
  }
  if (inst->octaves = (*vpfunc->create) (LWVP_FLOAT, LWVPDT_NOTXTR)) {
    inst->octgrp = (*evfunc->createGroup) (parent, "FN_Octaves");
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
fBmNoiseDestroy (fBmNoise *inst)
{
  if (inst) {

    // Free the LWVParmID memory
    if (inst->increment)
      (*vpfunc->destroy) (inst->increment);
    if (inst->lacunarity)
      (*vpfunc->destroy) (inst->lacunarity);
    if (inst->octaves)
      (*vpfunc->destroy) (inst->octaves);

    // Destroy the LWChanGroupIDs
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
fBmNoiseLoad (fBmNoise *inst, const LWLoadState *lState)
{
  LWError     err;
  short	      fn;

  // Exit if it's not the same version number (defaults will be used). 
  LWLOAD_I2 (lState, &inst->tversion, 1);
  if (inst->tversion != FN_VERSION) {
    inst->tversion = FN_VERSION;
    return NULL;
  }

  LWLOAD_I2 (lState, &fn, 1);
  inst->fnoise = (FracNoise) fn;

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
fBmNoiseSave (fBmNoise *inst, const LWSaveState *sState)
{
  LWError    err;
  short      fn;

  LWSAVE_I2 (sState, &inst->tversion, 1);

  fn = (short) inst->fnoise;
  LWSAVE_I2 (sState, &fn, 1);

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
fBmNoiseCopy (fBmNoise *to, fBmNoise *from)
{
  LWError	 err;
  LWVParmID	 vpid1, vpid2, vpid3;
  LWChanGroupID  gpid1, gpid2, gpid3;

  gpid1 = to->incgrp;
  gpid2 = to->lacgrp;
  gpid3 = to->octgrp;
  vpid1 = to->increment;
  vpid2 = to->lacunarity;
  vpid3 = to->octaves;

  *to = *from;

  to->incgrp = gpid1;
  to->lacgrp = gpid2;
  to->octgrp = gpid3;
  to->increment = vpid1;
  to->lacunarity = vpid2;
  to->octaves = vpid3;

  // Use the LWVParmID copy function to handle these allocated resources.
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
fBmNoiseDescLn (fBmNoise *inst)
{
  sprintf (inst->desc, "%4.1f %4.1f %4.1f",
           inst->inc[0], inst->lac[0], inst->oct[0] );

  return inst->desc;
}


/*
 * Init() Handler callback.
 *
 * Called at the start of rendering.
 */
XCALL_(static LWError)
fBmNoiseInit (fBmNoise *inst, int rtype)
{
  return (NULL);
}


/*
 * Cleanup() Handler callback.
 *
 * Called at the end of rendering.
 */
XCALL_(static void)
fBmNoiseCleanup (fBmNoise *inst)
{
  return;
}


/*
 * NewTime() Handler callback.
 *
 * Called at the start of each rendering pass.
 */
XCALL_(static LWError)
fBmNoiseNewTime (fBmNoise *inst, LWFrame f, LWTime t)
{
  // Put the current LWVParmID values into the temporary instance 
  // variables for easy access during the Evaluate stage.
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
fBmNoiseFlags (fBmNoise *inst)
{
  return (LWTEXF_HV_SRF | LWTEXF_HV_VOL | LWTEXF_DISPLACE);
}


/*
 * Evaluate() Handler callback.
 *
 * Calculation of the procedural texture.
 */
XCALL_(static double)
fBmNoiseEvaluate (fBmNoise *inst, LWTextureAccess *ta)
{
  double     turb;
  double     PP[3];

  // Lets work in texture space
  Vec3Assign(PP, ta->tPos[0], ta->tPos[1], ta->tPos[2]);

  // Use the fBm function to get some fractal turbulence
  turb = fBm(PP, inst->inc[0], inst->lac[0], inst->oct[0], inst->fnoise);

  return turb;
}


/*
 * Activate() Handler activation function.
 *
 * Check the version, get some globals, and fill in the callback fields
 * of the handler structure.
 */
XCALL_(int)
fBmNoiseActivate (long version, GlobalFunc *global, LWTextureHandler *local,
  void *serverData)
{
  if (version != LWTEXTURE_VERSION)
    return (AFUNC_BADVERSION);

  evfunc = global(LWENVELOPEFUNCS_GLOBAL, GFUSE_TRANSIENT);
  vpfunc = global(LWVPARMFUNCS_GLOBAL, GFUSE_TRANSIENT);
  txfunc = global(LWTEXTUREFUNCS_GLOBAL, GFUSE_TRANSIENT);

  if (!evfunc || !txfunc || !vpfunc ) 
    return AFUNC_BADGLOBAL;

  local->inst->create   = fBmNoiseCreate;
  local->inst->destroy  = fBmNoiseDestroy;
  local->inst->load     = fBmNoiseLoad;
  local->inst->save     = fBmNoiseSave;
  local->inst->copy     = fBmNoiseCopy;
  local->inst->descln   = fBmNoiseDescLn;

  local->rend->init     = fBmNoiseInit;
  local->rend->cleanup  = fBmNoiseCleanup;
  local->rend->newTime  = fBmNoiseNewTime;

  local->flags    = fBmNoiseFlags;
  local->evaluate = fBmNoiseEvaluate;

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
FN_view_get ( fBmNoise *inst, unsigned long vid ) 
{
  void *result = NULL;
  static int ival = 0; // static so persist after function returns
 
  if ( !inst ) return result;

  switch ( vid ) {
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
FN_view_set ( fBmNoise *inst, unsigned long vid, void *value )
{
  int result = 0;

  if ( !inst ) return result;

  switch ( vid ) {
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
FN_view_destroy ( void *myData )
{
  fBmNoise	*inst = (fBmNoise*)myData;

  return;
}


/*
 * NonModal VIEW Panel Interface
 *
 * NonModal interfaces are created on activation.
 */
int
fBmNoiseInterface (long version, GlobalFunc *global, LWInterface *iface,
  void *serverdata )
{
  int rc = AFUNC_OK;

  LWXPanelID     panID = NULL;

  fBmNoise	*inst = NULL;

  static LWXPanelHint FN_hint[] = {
    XpLABEL(0,"fBmNoise"),
    XpDESTROYNOTIFY(FN_view_destroy),
    XpEND
  };

  // Check the version
  // NOTE: The interface version is different from handler version
  if ( version != LWINTERFACE_VERSION )
    return (AFUNC_BADVERSION);

  xpfunc = global(LWXPANELFUNCS_GLOBAL, GFUSE_TRANSIENT);

  if ( !xpfunc )
    return AFUNC_BADGLOBAL;

  inst = (fBmNoise *) iface->inst;

  // If we already have an XPanel which has not been destroyed,
  // we can destroy it and create another, or return the existing instance.
    // Create panel
    panID = (*xpfunc->create)( LWXP_VIEW, OPTS3_ctrl_list );
    if (panID) {

      // Apply some hints
      (*xpfunc->hint) ( panID, 0, OPTS3_hint );
      (*xpfunc->hint) ( panID, 0, FN_hint );

      // Describe the structure of the data instance to the panel
      (*xpfunc->describe)( panID, OPTS3_data_descrip, FN_view_get, FN_view_set);

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
