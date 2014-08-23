/*
 * coriolis.h - data structure for coriolis instance
 */

#include "pts_incs.h"
#include "pts_frctl.h"

/*
 * This is the coriolis instance data structure. An instance is a particular
 * set of options for this texture. The instance is passed as a parameter to
 * all of the functions used for this plug-in's operation.
 */
typedef struct st_Coriolis {

  // Version number for this procedural texture
  short		 tversion;

  // These are the parameters that will be displayed in the procedural
  // texture's Xpanel interface. These parameters will be saved in the
  // Lightwave object file.
  LWVParmID	 coriolisscale;
  LWVParmID	 coriolistwist;
  LWVParmID	 coriolisoffset;
  LWVParmID	 increment;
  LWVParmID	 lacunarity;
  LWVParmID	 octaves;
  FracNoise	 fnoise;

  // The following values are temporary and are not saved.

  // Character string used for describing this instance.
  char		 desc[64];

  // Seperate channel group IDs for each LWVParmID parameter
  LWChanGroupID	 sclgrp, twsgrp, offgrp, incgrp, lacgrp, octgrp;

  // Used for easy access to the current value contained in the
  // LWVParmID variables above.
  double	 scl[3], tws[3], off[3], inc[3], lac[3], oct[3];
} Coriolis;


int CoriolisActivate(long, GlobalFunc *, LWTextureHandler *, void *);
int CoriolisInterface(long, GlobalFunc *, LWInterface *, void *);
