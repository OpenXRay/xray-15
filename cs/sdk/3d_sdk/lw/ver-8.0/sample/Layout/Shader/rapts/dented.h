/*
 * dented.h - data structure for dented instance
 */

#include "pts_incs.h"
#include "pts_frctl.h"

/*
 * This is the dented instance data structure. An instance is a particular
 * set of options for this texture. The instance is passed as a parameter to
 * all of the functions used for this plug-in's operation.
 */
typedef struct st_Dented {

  // Version number for this procedural texture
  short		 tversion;

  // These are the parameters that will be displayed in the procedural
  // texture's Xpanel interface. These parameters will be saved in the
  // Lightwave object file.
  LWVParmID	 scale;
  LWVParmID	 power;
  LWVParmID	 frequency;
  LWVParmID	 octaves;
  FracNoise	 fnoise;

  // The following values are temporary and are not saved.

  // Character string used for describing this instance.
  char		 desc[64];

  // Seperate channel group IDs for each LWVParmID parameter
  LWChanGroupID	 sclgrp, pwrgrp, frqgrp, octgrp;

  // Used for easy access to the current value contained in the
  // LWVParmID variables above.
  double	 scl[3], pwr[3], frq[3], oct[3];

} Dented;


int DentedActivate(long, GlobalFunc *, LWTextureHandler *, void *);
int DentedInterface(long, GlobalFunc *, LWInterface *, void *);
