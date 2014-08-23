/*
 * hybridmfractal.h - data structure for hybridmfractal instance
 */

#include "pts_incs.h"
#include "pts_frctl.h"

/*
 * This is the hybridmfractal instance data structure. An instance is a particular
 * set of options for this texture. The instance is passed as a parameter to
 * all of the functions used for this plug-in's operation.
 */
typedef struct st_HybridMFractal {

  // Version number for this procedural texture
  short		 tversion;

  // These are the parameters that will be displayed in the procedural
  // texture's Xpanel interface. These parameters will be saved in the
  // Lightwave object file.
  LWVParmID	 increment;
  LWVParmID	 lacunarity;
  LWVParmID	 octaves;
  LWVParmID	 offset;
  FracNoise	 fnoise;

  // The following values are temporary and are not saved.

  // Character string used for describing this instance.
  char		 desc[64];

  // Seperate channel group IDs for each LWVParmID parameter
  LWChanGroupID	 incgrp, lacgrp, octgrp, offgrp;

  // Used for easy access to the current value contained in the
  // LWVParmID variables above.
  double	 inc[3], lac[3], oct[3], off[3];

} HybridMFractal;


int HybridMFractalActivate(long, GlobalFunc *, LWTextureHandler *, void *);
int HybridMFractalInterface(long, GlobalFunc *, LWInterface *, void *);
