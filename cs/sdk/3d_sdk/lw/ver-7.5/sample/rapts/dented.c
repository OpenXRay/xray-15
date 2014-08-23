/*
 * dented.c --  Layout Procedural Texture Plugin
 *
 * based on the dented.sl Renderman shader written by F Kenton Musgrave
 *
 * by Marvin Landis
 * last revision  1/25/2000
 */

#include "pts_frctl.h"
#include "pts_gui.h"
#include "pts_math.h"

#include "dented.h"

#define DT_VERSION        1

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
DentedCreate (void *priv, LWTLayerID txtrLayer, LWError *err)
{
  Dented			*inst = NULL;
  LWChanGroupID		 parent = NULL;


  // Allocate instance data.
  inst = malloc (sizeof (Dented));
  if (!inst) {
    *err = "No memory for new Dented texture.";
    return NULL;
  }

  // Initialize the current version number
  inst->tversion = DT_VERSION;

  // The default Noise Type.
  inst->fnoise = PNOISE;

  // Initialize temporary variables, these values will be used to
  // initialize the LWVParmID variables.
  inst->scl[0] = 4.0; inst->scl[1] = 0.0; inst->scl[2] = 0.0;
  inst->pwr[0] = 3.0; inst->pwr[1] = 0.0; inst->pwr[2] = 0.0;
  inst->frq[0] = 0.8; inst->frq[1] = 0.0; inst->frq[2] = 0.0;
  inst->oct[0] = 6.0; inst->oct[1] = 0.0; inst->oct[2] = 0.0;

  // Create the LWVParmID and an envelope group for each parameter.
  // Initialize the LWVParmID with the values assigned to the
  // temporary variables above.

  // parent is the texture layer group, this groups belongs to the texture group
  // which might be also part of a bigger hierarchy (surface group, etc.)	
  if (txtrLayer)
    parent = (*txfunc->layerEnvGroup)(txtrLayer);

  if (inst->scale = (*vpfunc->create) (LWVP_FLOAT, LWVPDT_NOTXTR)) {
    inst->sclgrp = (*evfunc->createGroup) (parent, "DT_Scale");
    (*vpfunc->setup) (inst->scale, "Scale", inst->sclgrp,
      NULL, NULL, NULL, NULL);
    (*vpfunc->setVal) (inst->scale, inst->scl);
  }
  if (inst->power = (*vpfunc->create) (LWVP_FLOAT, LWVPDT_NOTXTR)) {
    inst->pwrgrp = (*evfunc->createGroup) (parent, "DT_Power");
    (*vpfunc->setup) (inst->power, "Power", inst->pwrgrp,
      NULL, NULL, NULL, NULL);
    (*vpfunc->setVal) (inst->power, inst->pwr);
  }
  if (inst->frequency = (*vpfunc->create) (LWVP_FLOAT, LWVPDT_NOTXTR)) {
    inst->frqgrp = (*evfunc->createGroup) (parent, "DT_Frequency");
    (*vpfunc->setup) (inst->frequency, "Frequency", inst->frqgrp,
      NULL, NULL, NULL, NULL);
    (*vpfunc->setVal) (inst->frequency, inst->frq);
  }
  if (inst->octaves = (*vpfunc->create) (LWVP_FLOAT, LWVPDT_NOTXTR)) {
    inst->octgrp = (*evfunc->createGroup) (parent, "DT_Octaves");
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
DentedDestroy (Dented *inst)
{
  if (inst) {

    // Free the LWVParmID memory
    if (inst->scale)
      (*vpfunc->destroy) (inst->scale);
    if (inst->power)
      (*vpfunc->destroy) (inst->power);
    if (inst->frequency)
      (*vpfunc->destroy) (inst->frequency);
    if (inst->octaves)
      (*vpfunc->destroy) (inst->octaves);

    // Destroy the LWChanGroupIDs
    if (inst->sclgrp)
      (*evfunc->destroyGroup) (inst->sclgrp);
    if (inst->pwrgrp)
      (*evfunc->destroyGroup) (inst->pwrgrp);
    if (inst->frqgrp)
      (*evfunc->destroyGroup) (inst->frqgrp);
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
DentedLoad (Dented *inst, const LWLoadState *lState)
{
  LWError     err;
  short	      fn;

  // Exit if it's not the same version number (defaults will be used). 
  LWLOAD_I2 (lState, &inst->tversion, 1);
  if (inst->tversion != DT_VERSION) {
    inst->tversion = DT_VERSION;
    return NULL;
  }

  LWLOAD_I2 (lState, &fn, 1);
  inst->fnoise = (FracNoise) fn;

  if (!(err = (*vpfunc->load) (inst->scale, lState)))
    (*vpfunc->getVal) (inst->scale, 0, NULL, inst->scl);
  else
    return (err);

  if (!(err = (*vpfunc->load) (inst->power, lState)))
    (*vpfunc->getVal) (inst->power, 0, NULL, inst->pwr);
  else
    return (err);

  if (!(err = (*vpfunc->load) (inst->frequency, lState)))
    (*vpfunc->getVal) (inst->frequency, 0, NULL, inst->frq);
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
DentedSave (Dented *inst, const LWSaveState *sState)
{
  LWError    err;
  short      fn;

  LWSAVE_I2 (sState, &inst->tversion, 1);

  fn = (short) inst->fnoise;
  LWSAVE_I2 (sState, &fn, 1);

  if (err = (*vpfunc->save) (inst->scale, sState))
    return (err);
  if (err = (*vpfunc->save) (inst->power, sState))
    return (err);
  if (err = (*vpfunc->save) (inst->frequency, sState))
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
DentedCopy (Dented *to, Dented *from)
{
  LWError	 err;
  LWVParmID	 vpid1, vpid2, vpid3, vpid4;
  LWChanGroupID  gpid1, gpid2, gpid3, gpid4;

  gpid1 = to->sclgrp;
  gpid2 = to->pwrgrp;
  gpid3 = to->frqgrp;
  gpid4 = to->octgrp;
  vpid1 = to->scale;
  vpid2 = to->power;
  vpid3 = to->frequency;
  vpid4 = to->octaves;

  *to = *from;

  to->sclgrp = gpid1;
  to->pwrgrp = gpid2;
  to->frqgrp = gpid3;
  to->octgrp = gpid4;
  to->scale = vpid1;
  to->power = vpid2;
  to->frequency = vpid3;
  to->octaves = vpid4;

  // Use the LWVParmID copy function to handle these allocated resources.
  if (err = (*vpfunc->copy)(to->scale, from->scale))
    return (err);
  if (err = (*vpfunc->copy)(to->power, from->power))
    return (err);
  if (err = (*vpfunc->copy)(to->frequency, from->frequency))
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
DentedDescLn (Dented *inst)
{
  sprintf (inst->desc, "%4.1f %4.1f %4.1f %4.1f",
           inst->scl[0], inst->pwr[0], inst->frq[0], inst->oct[0] );

  return inst->desc;
}


/*
 * Init() Handler callback.
 *
 * Called at the start of rendering.
 */
XCALL_(static LWError)
DentedInit (Dented *inst, int rtype)
{
  return (NULL);
}


/*
 * Cleanup() Handler callback.
 *
 * Called at the end of rendering.
 */
XCALL_(static void)
DentedCleanup (Dented *inst)
{
  return;
}


/*
 * NewTime() Handler callback.
 *
 * Called at the start of each rendering pass.
 */
XCALL_(static LWError)
DentedNewTime (Dented *inst, LWFrame f, LWTime t)
{
  // Put the current LWVParmID values into the temporary instance 
  // variables for easy access during the Evaluate stage.
  if (inst->scale)
    (*vpfunc->getVal) (inst->scale, t, NULL, inst->scl);
  if (inst->power)
    (*vpfunc->getVal) (inst->power, t, NULL, inst->pwr);
  if (inst->frequency)
    (*vpfunc->getVal) (inst->frequency, t, NULL, inst->frq);
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
DentedFlags (Dented *inst)
{
  return (LWTEXF_HV_SRF | LWTEXF_HV_VOL | LWTEXF_DISPLACE);
}


/*
 * Evaluate() Handler callback.
 *
 * Calculation of the procedural texture.
 */
XCALL_(static double)
DentedEvaluate (Dented *inst, LWTextureAccess *ta)
{
  /* Local stuff */
  double	 size, result;
  double	 magnitude = 0;
  int		 i;

  /* Position Stuff */
  double	 PP[3], PS[3];

  // Lets work in texture space
  Vec3Assign(PP, ta->tPos[0], ta->tPos[1], ta->tPos[2]);
 
  size = inst->frq[0];
  for (i = 0;  i < inst->oct[0];  i++) {
    Vec3MultC(PS, PP, size);
    magnitude += Abs(noisey(PS, inst->fnoise)) / size;
    size *= 2;
  }

  result = pow(magnitude, inst->pwr[0]);

  /* Use value to determine new shading values.         */
  result *= inst->scl[0];

  return result;
}


/*
 * Activate() Handler activation function.
 *
 * Check the version, get some globals, and fill in the callback fields
 * of the handler structure.
 */
XCALL_(int)
DentedActivate (long version, GlobalFunc *global, LWTextureHandler *local,
  void *serverData)
{
  if (version != LWTEXTURE_VERSION)
    return (AFUNC_BADVERSION);

  evfunc = global(LWENVELOPEFUNCS_GLOBAL, GFUSE_TRANSIENT);
  txfunc = global(LWTEXTUREFUNCS_GLOBAL, GFUSE_TRANSIENT);
  vpfunc = global(LWVPARMFUNCS_GLOBAL, GFUSE_TRANSIENT);

  if (!evfunc || !txfunc || !vpfunc ) 
    return AFUNC_BADGLOBAL;

  local->inst->create   = DentedCreate;
  local->inst->destroy  = DentedDestroy;
  local->inst->load     = DentedLoad;
  local->inst->save     = DentedSave;
  local->inst->copy     = DentedCopy;
  local->inst->descln   = DentedDescLn;

  local->rend->init     = DentedInit;
  local->rend->cleanup  = DentedCleanup;
  local->rend->newTime  = DentedNewTime;

  local->flags    = DentedFlags;
  local->evaluate = DentedEvaluate;

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
DT_view_get ( Dented *inst, unsigned long vid ) 
{
  void *result = NULL;
  static int ival = 0; // static so persist after function returns
 
  if ( !inst ) return result;

  switch ( vid ) {
    case (OPTS_SCALE):
      result = inst->scale;
      break;
    case (OPTS_POWER):
      result = inst->power;
      break;
    case (OPTS_FREQUENCY):
      result = inst->frequency;
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
DT_view_set ( Dented *inst, unsigned long vid, void *value )
{
  int result = 0;

  if ( !inst ) return result;

  switch ( vid ) {
    case (OPTS_SCALE):
      // inst->scale should already contain the correct value
      (*vpfunc->getVal) (inst->scale, 0, NULL, inst->scl);
      result = 1;
      break;
    case (OPTS_POWER):
      // inst->power should already contain the correct value
      (*vpfunc->getVal) (inst->power , 0, NULL, inst->pwr);
      result = 1;
      break;
    case (OPTS_FREQUENCY):
      // inst->octaves should already contain the correct value
      (*vpfunc->getVal) (inst->frequency, 0, NULL, inst->frq);
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
 */
void
DT_view_destroy ( void *myData )
{
  Dented		*inst = (Dented*)myData;

  return;
}


/*
 * NonModal VIEW Panel Interface
 *
 * NonModal interfaces are created on activation.
 */
int
DentedInterface (long version, GlobalFunc *global, LWInterface *iface,
  void *serverdata )
{
  int			 rc = AFUNC_OK;
  LWXPanelID     panID = NULL;
  Dented		*inst = NULL;

  // These are the control lists and configuration hints
  static LWXPanelControl DT_ctrl_list[] = {
   { OPTS_SCALE,         "Scale",          "float-env" },
   { OPTS_POWER,         "Power",          "float-env" },
   { OPTS_FREQUENCY,     "Frequency",      "float-env" },
   { OPTS_OCTAVES,       "Octaves",        "float-env" },
   { OPTS_FNOISE,        "Noise Type",     "iPopChoice" },
   {0}
  };

  static LWXPanelDataDesc DT_data_descrip[] = {
   { OPTS_SCALE,         "Scale",          "float-env" },
   { OPTS_POWER,         "Power",          "float-env" },
   { OPTS_FREQUENCY,     "Frequency",      "float-env" },
   { OPTS_OCTAVES,       "Octaves",        "float-env" },
   { OPTS_FNOISE,        "Noise Type",     "integer" },
   {0}
  };

  static LWXPanelHint DT_hint[] = {
    XpSTRLIST(OPTS_FNOISE, FNoise),
    XpRANGE(OPTS_SCALE, -10, 10, 1),
    XpRANGE(OPTS_POWER, -5, 5, 1),
    XpRANGE(OPTS_FREQUENCY, -5, 5, 1),
    XpRANGE(OPTS_OCTAVES, 0, 10, 1),
    XpH(OPTS_SCALE), XpEND,
    XpH(OPTS_POWER), XpEND,
    XpH(OPTS_FREQUENCY), XpEND,
    XpH(OPTS_OCTAVES), XpEND,
    XpH(OPTS_FNOISE), XpEND,
    XpLABEL(0,"Dented"),
    XpDESTROYNOTIFY(DT_view_destroy),
    XpEND
  };

  // Check the version
  // NOTE: The interface version is different from handler version
  if ( version != LWINTERFACE_VERSION )
    return (AFUNC_BADVERSION);

  xpfunc = global(LWXPANELFUNCS_GLOBAL, GFUSE_TRANSIENT);

  if ( !xpfunc )
    return AFUNC_BADGLOBAL;

  inst = (Dented *) iface->inst;

  // If we already have an XPanel which has not been destroyed,
  // we can destroy it and create another, or return the existing instance.
  
    // Create panel
    panID = (*xpfunc->create)( LWXP_VIEW, DT_ctrl_list );
    if (panID) {

      // Apply some hints
      (*xpfunc->hint) ( panID, 0, DT_hint );

      // Describe the structure of the data instance to the panel
      (*xpfunc->describe)( panID, DT_data_descrip, DT_view_get, DT_view_set );

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
