/*
 * cyclone.h - data structure for cyclone instance
 */

#include "pts_incs.h"
#include "pts_frctl.h"

/*
 * This is the cyclone instance data structure. An instance is a particular
 * set of options for this texture. The instance is passed as a parameter to
 * all of the functions used for this plug-in's operation.
 */
typedef struct st_Cyclone {

  // Version number for this procedural texture
  short		 tversion;

  // These are the parameters that will be displayed in the procedural
  // texture's Xpanel interface. These parameters will be saved in the
  // Lightwave object file.
  LWVParmID	 cycloneradius;
  LWVParmID	 cyclonetwist;
  LWVParmID	 cycloneoffset;
  LWVParmID	 increment;
  LWVParmID	 lacunarity;
  LWVParmID	 octaves;
  FracNoise	 fnoise;

  // The following values are temporary and are not saved.

  // Character string used for describing this instance.
  char		 desc[64];

  // Seperate channel group IDs for each LWVParmID parameter
  LWChanGroupID	 radgrp, twsgrp, offgrp, incgrp, lacgrp, octgrp;

  // Used for easy access to the current value contained in the
  // LWVParmID variables above.
  double	 rad[3], tws[3], off[3], inc[3], lac[3], oct[3];
} Cyclone;


int CycloneActivate(long, GlobalFunc *, LWTextureHandler *, void *);
int CycloneInterface(long, GlobalFunc *, LWInterface *, void *);
