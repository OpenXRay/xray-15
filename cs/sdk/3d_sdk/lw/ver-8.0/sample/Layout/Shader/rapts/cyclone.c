/*
 * cyclone.c --  Layout Procedural Texture Plugin
 *
 * based on the cyclone.sl Renderman shader written by F Kenton Musgrave
 *
 * by Marvin Landis
 * last revision  1/25/2000
 */

#include "pts_frctl.h"
#include "pts_gui.h"
#include "pts_math.h"

#include "cyclone.h"

#define CY_VERSION        1

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
CycloneCreate (void *priv, LWTLayerID txtrLayer, LWError *err)
{
  Cyclone			*inst = NULL;
  LWChanGroupID		 parent = NULL;


  // Allocate instance data.
  inst = malloc (sizeof (Cyclone));
  if (!inst) {
    *err = "No memory for new Cyclone texture.";
    return NULL;
  }

  // Initialize the current version number
  inst->tversion = CY_VERSION;

  // The default Noise Type.
  inst->fnoise = PNOISE;

  // Initialize temporary variables, these values will be used to
  // initialize the LWVParmID variables.
  inst->rad[0] = 1.0; inst->rad[1] = 0.0; inst->rad[2] = 0.0;
  inst->tws[0] = 0.9; inst->tws[1] = 0.0; inst->tws[2] = 0.0;
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

  if (inst->cycloneradius = (*vpfunc->create) (LWVP_FLOAT, LWVPDT_NOTXTR)) {
    inst->radgrp = (*evfunc->createGroup) (parent, "CY_Radius");
    (*vpfunc->setup) (inst->cycloneradius, "Radius", inst->radgrp,
      NULL, NULL, NULL, NULL);
    (*vpfunc->setVal) (inst->cycloneradius, inst->rad);
  }
  if (inst->cyclonetwist = (*vpfunc->create) (LWVP_FLOAT, LWVPDT_NOTXTR)) {
    inst->twsgrp = (*evfunc->createGroup) (parent, "CY_Twist");
    (*vpfunc->setup) (inst->cyclonetwist, "Twist", inst->twsgrp,
      NULL, NULL, NULL, NULL);
    (*vpfunc->setVal) (inst->cyclonetwist, inst->tws);
  }
  if (inst->cycloneoffset = (*vpfunc->create) (LWVP_FLOAT, LWVPDT_NOTXTR)) {
    inst->offgrp = (*evfunc->createGroup) (parent, "CY_Offset");
    (*vpfunc->setup) (inst->cycloneoffset, "Offset", inst->offgrp,
      NULL, NULL, NULL, NULL);
    (*vpfunc->setVal) (inst->cycloneoffset, inst->off);
  }
  if (inst->increment = (*vpfunc->create) (LWVP_FLOAT, LWVPDT_NOTXTR)) {
    inst->incgrp = (*evfunc->createGroup) (parent, "CY_Increment");
    (*vpfunc->setup) (inst->increment, "Increment", inst->incgrp,
      NULL, NULL, NULL, NULL);
    (*vpfunc->setVal) (inst->increment, inst->inc);
  }
  if (inst->lacunarity = (*vpfunc->create) (LWVP_FLOAT, LWVPDT_NOTXTR)) {
    inst->lacgrp = (*evfunc->createGroup) (parent, "CY_Lacunarity");
    (*vpfunc->setup) (inst->lacunarity, "Lacunarity", inst->lacgrp,
      NULL, NULL, NULL, NULL);
    (*vpfunc->setVal) (inst->lacunarity, inst->lac);
  }
  if (inst->octaves = (*vpfunc->create) (LWVP_FLOAT, LWVPDT_NOTXTR)) {
    inst->octgrp = (*evfunc->createGroup) (parent, "CY_Octaves");
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
CycloneDestroy (Cyclone *inst)
{
  if (inst) {

    // Free the LWVParmID memory
    if (inst->cycloneradius)
      (*vpfunc->destroy) (inst->cycloneradius);
    if (inst->cyclonetwist)
      (*vpfunc->destroy) (inst->cyclonetwist);
    if (inst->cycloneoffset)
      (*vpfunc->destroy) (inst->cycloneoffset);
    if (inst->increment)
      (*vpfunc->destroy) (inst->increment);
    if (inst->lacunarity)
      (*vpfunc->destroy) (inst->lacunarity);
    if (inst->octaves)
      (*vpfunc->destroy) (inst->octaves);

    // Destroy the LWChanGroupIDs
    if (inst->radgrp)
      (*evfunc->destroyGroup) (inst->radgrp);
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
CycloneLoad (Cyclone *inst, const LWLoadState *lState)
{
  LWError     err;
  short	      fn;

  // Exit if it's not the same version number (defaults will be used). 
  LWLOAD_I2 (lState, &inst->tversion, 1);
  if (inst->tversion != CY_VERSION) {
    inst->tversion = CY_VERSION;
    return NULL;
  }

  LWLOAD_I2 (lState, &fn, 1);
  inst->fnoise = (FracNoise) fn;

  if (!(err = (*vpfunc->load) (inst->cycloneradius, lState)))
    (*vpfunc->getVal) (inst->cycloneradius, 0, NULL, inst->rad);
  else
    return (err);

  if (!(err = (*vpfunc->load) (inst->cyclonetwist, lState)))
    (*vpfunc->getVal) (inst->cyclonetwist, 0, NULL, inst->tws);
  else
    return (err);

  if (!(err = (*vpfunc->load) (inst->cycloneoffset, lState)))
    (*vpfunc->getVal) (inst->cycloneoffset, 0, NULL, inst->off);
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
CycloneSave (Cyclone *inst, const LWSaveState *sState)
{
  LWError    err;
  short      fn;

  LWSAVE_I2 (sState, &inst->tversion, 1);

  fn = (short) inst->fnoise;
  LWSAVE_I2 (sState, &fn, 1);

  if (err = (*vpfunc->save) (inst->cycloneradius, sState))
    return (err);
  if (err = (*vpfunc->save) (inst->cyclonetwist, sState))
    return (err);
  if (err = (*vpfunc->save) (inst->cycloneoffset, sState))
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
CycloneCopy (Cyclone *to, Cyclone *from)
{
  LWError	 err;
  LWVParmID	 vpid1, vpid2, vpid3, vpid4, vpid5, vpid6;
  LWChanGroupID  gpid1, gpid2, gpid3, gpid4, gpid5, gpid6;

  gpid1 = to->radgrp;
  gpid2 = to->twsgrp;
  gpid3 = to->offgrp;
  gpid4 = to->incgrp;
  gpid5 = to->lacgrp;
  gpid6 = to->octgrp;
  vpid1 = to->cycloneradius;
  vpid2 = to->cyclonetwist;
  vpid3 = to->cycloneoffset;
  vpid4 = to->increment;
  vpid5 = to->lacunarity;
  vpid6 = to->octaves;

  *to = *from;

  to->radgrp = gpid1;
  to->twsgrp = gpid2;
  to->offgrp = gpid3;
  to->incgrp = gpid4;
  to->lacgrp = gpid5;
  to->octgrp = gpid6;
  to->cycloneradius = vpid1;
  to->cyclonetwist = vpid2;
  to->cycloneoffset = vpid3;
  to->increment = vpid4;
  to->lacunarity = vpid5;
  to->octaves = vpid6;

  // Use the LWVParmID copy function to handle these allocated resources.
  if (err = (*vpfunc->copy)(to->cycloneradius, from->cycloneradius))
    return (err);
  if (err = (*vpfunc->copy)(to->cyclonetwist, from->cyclonetwist))
    return (err);
  if (err = (*vpfunc->copy)(to->cycloneoffset, from->cycloneoffset))
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
CycloneDescLn (Cyclone *inst)
{
  sprintf (inst->desc, "%4.1f %4.1f %4.1f",
           inst->rad[0], inst->tws[0], inst->off[0] );

  return inst->desc;
}


/*
 * Init() Handler callback.
 *
 * Called at the start of rendering.
 */
XCALL_(static LWError)
CycloneInit (Cyclone *inst, int rtype)
{
  return (NULL);
}


/*
 * Cleanup() Handler callback.
 *
 * Called at the end of rendering.
 */
XCALL_(static void)
CycloneCleanup (Cyclone *inst)
{
  return;
}


/*
 * NewTime() Handler callback.
 *
 * Called at the start of each rendering pass.
 */
XCALL_(static LWError)
CycloneNewTime (Cyclone *inst, LWFrame f, LWTime t)
{
  // Put the current LWVParmID values into the temporary instance 
  // variables for easy access during the Evaluate stage.
  if (inst->cycloneradius)
    (*vpfunc->getVal) (inst->cycloneradius, t, NULL, inst->rad);
  if (inst->cyclonetwist)
    (*vpfunc->getVal) (inst->cyclonetwist, t, NULL, inst->tws);
  if (inst->cycloneoffset)
    (*vpfunc->getVal) (inst->cycloneoffset, t, NULL, inst->off);
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
CycloneFlags (Cyclone *inst)
{
  return (LWTEXF_HV_SRF | LWTEXF_HV_VOL | LWTEXF_DISPLACE | LWTEXF_AXIS);
}


/*
 * Evaluate() Handler callback.
 *
 * Calculation of the procedural texture.
 */
XCALL_(static double)
CycloneEvaluate (Cyclone *inst, LWTextureAccess *ta)
{
  /* Local stuff */
  double	dist, max_radius, radius, angle, sine, cosine;
  double	eye_weight, value, turb;

  /* Position Stuff */
  double	 Pt[3], PtN[3], PP[3];

  // Lets work in texture space
  Vec3Assign(Pt, ta->tPos[0], ta->tPos[1], ta->tPos[2]);

  Vec3Copy(PtN, Pt);
  normalize3(PtN);
  	
  if (ta->axis == 0)
    radius = sqrt(ycomp(PtN) * ycomp(PtN) + zcomp(PtN) * zcomp(PtN));
  else if (ta->axis == 1)
    radius = sqrt(xcomp(PtN) * xcomp(PtN) + zcomp(PtN) * zcomp(PtN));
  else
    radius = sqrt(xcomp(PtN) * xcomp(PtN) + ycomp(PtN) * ycomp(PtN));

  max_radius = inst->rad[0];	
  if ( radius < max_radius) {  /* We are in the cyclone */
  	
    /* invert distance from center */

    dist   = pow( max_radius - radius, 3);
    angle  = M_PI + inst->tws[0] * M_2_PI * (max_radius-dist)/max_radius;
    sine   = sin( angle );
    cosine = cos( angle );

    PP[0] = Pt[0]*cosine - Pt[1]*sine;
    PP[1] = Pt[0]*sine + Pt[1]*cosine;
    PP[2] = Pt[2];

    if( radius < .05 * max_radius) { /* we are in the eye */
      eye_weight = (.1*max_radius-radius) * 10;
      eye_weight = pow(1-eye_weight, 4);
    }
    else eye_weight = 1;

  }
  else {
    Vec3Copy(PP, Pt);
    eye_weight = 0;
  }

  if(eye_weight > 0) { /* If we are in the storm area... */
    turb = fBm(PP, inst->inc[0], inst->lac[0], inst->oct[0], inst->fnoise);
    value = Abs(eye_weight * (inst->off[0] + turb));
  }
  else value = 0;

  return value;
}


/*
 * Activate() Handler activation function.
 *
 * Check the version, get some globals, and fill in the callback fields
 * of the handler structure.
 */
XCALL_(int)
CycloneActivate (long version, GlobalFunc *global, LWTextureHandler *local,
  void *serverData)
{
  if (version != LWTEXTURE_VERSION)
    return (AFUNC_BADVERSION);

  evfunc = global(LWENVELOPEFUNCS_GLOBAL, GFUSE_TRANSIENT);
  txfunc = global(LWTEXTUREFUNCS_GLOBAL, GFUSE_TRANSIENT);
  vpfunc = global(LWVPARMFUNCS_GLOBAL, GFUSE_TRANSIENT);

  if (!evfunc || !txfunc || !vpfunc ) 
    return AFUNC_BADGLOBAL;

  local->inst->create   = CycloneCreate;
  local->inst->destroy  = CycloneDestroy;
  local->inst->load     = CycloneLoad;
  local->inst->save     = CycloneSave;
  local->inst->copy     = CycloneCopy;
  local->inst->descln   = CycloneDescLn;

  local->rend->init     = CycloneInit;
  local->rend->cleanup  = CycloneCleanup;
  local->rend->newTime  = CycloneNewTime;

  local->flags    = CycloneFlags;
  local->evaluate = CycloneEvaluate;

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
CY_view_get ( Cyclone *inst, unsigned long vid ) 
{
  void *result = NULL;
  static int ival = 0; // static so persist after function returns
 
  if ( !inst ) return result;

  switch ( vid ) {
    case (OPTS_RADIUS):
      result = inst->cycloneradius;
      break;
    case (OPTS_TWIST):
      result = inst->cyclonetwist;
      break;
    case (OPTS_OFFSET):
      result = inst->cycloneoffset;
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
CY_view_set ( Cyclone *inst, unsigned long vid, void *value )
{
  int result = 0;

  if ( !inst ) return result;

  switch ( vid ) {
    case (OPTS_RADIUS):
      // inst->cycloneradius should already contain the correct value
      (*vpfunc->getVal) (inst->cycloneradius, 0, NULL, inst->rad);
      result = 1;
      break;
    case (OPTS_TWIST):
      // inst->cyclonetwist should already contain the correct value
      (*vpfunc->getVal) (inst->cyclonetwist , 0, NULL, inst->tws);
      result = 1;
      break;
    case (OPTS_OFFSET):
      // inst->octaves should already contain the correct value
      (*vpfunc->getVal) (inst->cycloneoffset, 0, NULL, inst->off);
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
CY_view_destroy ( void *myData )
{
  Cyclone		*inst = (Cyclone*)myData;

  return;
}



/*
 * NonModal VIEW Panel Interface
 *
 * NonModal interfaces are created on activation.
 */
int
CycloneInterface (long version, GlobalFunc *global, LWInterface *iface,
  void *serverdata )
{
  int			 rc = AFUNC_OK;
  LWXPanelID     panID = NULL;
  Cyclone		*inst = NULL;

  // These are the control lists and configuration hints
  static LWXPanelControl CY_ctrl_list[] = {
   { OPTS_RADIUS,     "Cyclone Radius",   "float-env" },
   { OPTS_TWIST,      "Cyclone Twist",    "float-env" },
   { OPTS_OFFSET,     "Cyclone Offset",   "float-env" },
   { OPTS_INCREMENT,  "Increment",        "float-env" },
   { OPTS_LACUNARITY, "Lacunarity",       "float-env" },
   { OPTS_OCTAVES,    "Octaves",          "float-env" },
   { OPTS_FNOISE,     "Noise Type",       "iPopChoice" },
   {0}
  };

  static LWXPanelDataDesc CY_data_descrip[] = {
   { OPTS_RADIUS,     "Cyclone Radius",   "float-env" },
   { OPTS_TWIST,      "Cyclone Twist",    "float-env" },
   { OPTS_OFFSET,     "Cyclone Offset",   "float-env" },
   { OPTS_INCREMENT,  "Increment",        "float-env" },
   { OPTS_LACUNARITY, "Lacunarity",       "float-env" },
   { OPTS_OCTAVES,    "Octaves",          "float-env" },
   { OPTS_FNOISE,     "Noise Type",       "integer" },
   {0}
  };

  static LWXPanelHint CY_hint[] = {
    XpRANGE(OPTS_RADIUS, -5, 5, 1),
    XpRANGE(OPTS_TWIST, -5, 5, 1),
    XpRANGE(OPTS_OFFSET, -5, 5, 1),
    XpH(OPTS_RADIUS), XpEND,
    XpH(OPTS_TWIST), XpEND,
    XpH(OPTS_OFFSET), XpEND,
    XpLABEL(0,"Cyclone"),
    XpDESTROYNOTIFY(CY_view_destroy),
    XpEND
  };

  // Check the version
  // NOTE: The interface version is different from handler version
  if ( version != LWINTERFACE_VERSION )
    return (AFUNC_BADVERSION);

  xpfunc = global(LWXPANELFUNCS_GLOBAL, GFUSE_TRANSIENT);

  if ( !xpfunc )
    return AFUNC_BADGLOBAL;

  inst = (Cyclone *) iface->inst;

  // If we already have an XPanel which has not been destroyed,
  // we can destroy it and create another, or return the existing instance.

		// Create panel
    panID = (*xpfunc->create)( LWXP_VIEW, CY_ctrl_list );
    if (panID) {

      // Apply some hints
      (*xpfunc->hint) ( panID, 0, CY_hint );
      (*xpfunc->hint) ( panID, 0, OPTS3_hint );

      // Describe the structure of the data instance to the panel
      (*xpfunc->describe)( panID, CY_data_descrip, CY_view_get, CY_view_set );

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
