/*
 * puffyclouds.h - data structure for puffyclouds instance
 */

#include "pts_incs.h"
#include "pts_frctl.h"

/*
 * This is the puffyclouds instance data structure. An instance is a particular
 * set of options for this texture. The instance is passed as a parameter to
 * all of the functions used for this plug-in's operation.
 */
typedef struct st_PuffyClouds {

  // Version number for this procedural texture
  short		 tversion;

  // These are the parameters that will be displayed in the procedural
  // texture's Xpanel interface. These parameters will be saved in the
  // Lightwave object file.
  LWVParmID	 threshold;
  LWVParmID	 increment;
  LWVParmID	 lacunarity;
  LWVParmID	 octaves;
  FracNoise	 fnoise;

  // The following values are temporary and are not saved.

  // Character string used for describing this instance.
  char		 desc[64];

  // Seperate channel group IDs for each LWVParmID parameter
  LWChanGroupID	 thrgrp, incgrp, lacgrp, octgrp;

  // Used for easy access to the current value contained in the
  // LWVParmID variables above.
  double	 thr[3], inc[3], lac[3], oct[3];

} PuffyClouds;


int PuffyCloudsActivate(long, GlobalFunc *, LWTextureHandler *, void *);
int PuffyCloudsInterface(long, GlobalFunc *, LWInterface *, void *);
