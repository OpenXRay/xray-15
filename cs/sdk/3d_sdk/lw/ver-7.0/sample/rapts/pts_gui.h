/*
 * pts_gui.h - Declarations and preprocessor definitions for Xpanels
 *             control lists, hints, and functions
 */

#ifndef _PTSGUI_H_
#define _PTSGUI_H_ 1

#include "pts_incs.h"

/*
 * Menuchoices for Noise Type pulldown
 */
static const char *FNoise[] = {"Perlin Noise", "Value Noise", "Gradient Noise",
                        "Value-Gradient Noise", "Lattice Convolution Noise",
                        "Sparse Convolution Noise", NULL};

/*
 * Control ID list
 * NOTE: ID values must begin above 0x8000
 * (lower values are reserved for internal use)
 */
typedef enum en_IDLIST {
	// some control IDs
	OPTS_INCREMENT = 0x9901,
	OPTS_LACUNARITY,
	OPTS_OCTAVES,
	OPTS_OFFSET,
	OPTS_THRESHOLD,
	OPTS_FNOISE,
	OPTS_SCALE,
	OPTS_RADIUS,
	OPTS_TWIST,
	OPTS_POWER,
	OPTS_FREQUENCY,
} en_idlist;

/*
 * Common configuration control lists, data descriptions, and hints
 */
static LWXPanelControl OPTS3_ctrl_list[] = {
  { OPTS_INCREMENT,  "Increment",   "float-env" },
  { OPTS_LACUNARITY, "Lacunarity",  "float-env" },
  { OPTS_OCTAVES,    "Octaves",     "float-env" },
  { OPTS_FNOISE,     "Noise Type",  "iPopChoice" },
  {0}
};

static LWXPanelDataDesc OPTS3_data_descrip[] = {
  { OPTS_INCREMENT,  "Increment",   "float-env" },
  { OPTS_LACUNARITY, "Lacunarity",  "float-env" },
  { OPTS_OCTAVES,    "Octaves",     "float-env" },
  { OPTS_FNOISE,     "Noise Type",  "integer" },
  {0}
};

static LWXPanelHint OPTS3_hint[] = {
  XpSTRLIST(OPTS_FNOISE, FNoise),
  XpRANGE(OPTS_INCREMENT, 0, 5, 1),
  XpRANGE(OPTS_LACUNARITY, 0, 6, 1),
  XpRANGE(OPTS_OCTAVES, 0, 10, 1),
  XpH(OPTS_INCREMENT), XpEND,
  XpH(OPTS_LACUNARITY), XpEND,
  XpH(OPTS_OCTAVES), XpEND,
  XpH(OPTS_FNOISE), XpEND,
  XpEND
};

static LWXPanelControl OPTS4_ctrl_list[] = {
  { OPTS_INCREMENT,  "Increment",   "float-env" },
  { OPTS_LACUNARITY, "Lacunarity",  "float-env" },
  { OPTS_OCTAVES,    "Octaves",     "float-env" },
  { OPTS_OFFSET,     "Offset",      "float-env" },
  { OPTS_FNOISE,     "Noise Type",  "iPopChoice" },
  {0}
};

static LWXPanelDataDesc OPTS4_data_descrip[] = {
  { OPTS_INCREMENT,  "Increment",   "float-env" },
  { OPTS_LACUNARITY, "Lacunarity",  "float-env" },
  { OPTS_OCTAVES,    "Octaves",     "float-env" },
  { OPTS_OFFSET,     "Offset",      "float-env" },
  { OPTS_FNOISE,     "Noise Type",  "integer" },
  {0}
};

static LWXPanelHint OPTS4_hint[] = {
  XpSTRLIST(OPTS_FNOISE, FNoise),
  XpRANGE(OPTS_INCREMENT, 0, 5, 1),
  XpRANGE(OPTS_LACUNARITY, 0, 6, 1),
  XpRANGE(OPTS_OCTAVES, 0, 10, 1),
  XpRANGE(OPTS_OFFSET, 0, 1, 1),
  XpH(OPTS_INCREMENT), XpEND,
  XpH(OPTS_LACUNARITY), XpEND,
  XpH(OPTS_OCTAVES), XpEND,
  XpH(OPTS_OFFSET), XpEND,
  XpH(OPTS_FNOISE), XpEND,
  XpEND
};

static LWXPanelControl OPTS5_ctrl_list[] = {
  { OPTS_INCREMENT,  "Increment",   "float-env" },
  { OPTS_LACUNARITY, "Lacunarity",  "float-env" },
  { OPTS_OCTAVES,    "Octaves",     "float-env" },
  { OPTS_OFFSET,     "Offset",      "float-env" },
  { OPTS_THRESHOLD,  "Threshold",   "float-env" },
  { OPTS_FNOISE,     "Noise Type",  "iPopChoice" },
  {0}
};

static LWXPanelDataDesc OPTS5_data_descrip[] = {
  { OPTS_INCREMENT,  "Increment",   "float-env" },
  { OPTS_LACUNARITY, "Lacunarity",  "float-env" },
  { OPTS_OCTAVES,    "Octaves",     "float-env" },
  { OPTS_OFFSET,     "Offset",      "float-env" },
  { OPTS_THRESHOLD,  "Threshold",   "float-env" },
  { OPTS_FNOISE,     "Noise Type",  "integer" },
  {0}
};

static LWXPanelHint OPTS5_hint[] = {
  XpSTRLIST(OPTS_FNOISE, FNoise),
  XpRANGE(OPTS_INCREMENT, 0, 5, 1),
  XpRANGE(OPTS_LACUNARITY, 0, 6, 1),
  XpRANGE(OPTS_OCTAVES, 0, 10, 1),
  XpRANGE(OPTS_OFFSET, 0, 1, 1),
  XpRANGE(OPTS_THRESHOLD, -5, 5, 1),
  XpH(OPTS_INCREMENT), XpEND,
  XpH(OPTS_LACUNARITY), XpEND,
  XpH(OPTS_OCTAVES), XpEND,
  XpH(OPTS_OFFSET), XpEND,
  XpH(OPTS_THRESHOLD), XpEND,
  XpH(OPTS_FNOISE), XpEND,
  XpEND
};

extern void OPTS_view_destroy ( LWXPanelID );

#endif /* _PTSGUI_H_ */
