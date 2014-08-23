/*
 * RAPTS - Recycled Additional Procedural Textures for Six (Lightwave 3D)
 */

#include "pts_incs.h"

#include "fbmnoise.h"
#include "turbnoise.h"
#include "multifractal.h"
#include "heteroterrain.h"
#include "hybridmfractal.h"
#include "ridgedmfractal.h"
#include "coriolis.h"
#include "cyclone.h"
#include "dented.h"
#include "puffyclouds.h"


ServerUserName userNames[] = 
{
	{"FBMNoise", LANGID_USENGLISH},
	{"TurbNoise", LANGID_USENGLISH},
	{"MultiFractal", LANGID_USENGLISH},
	{"HeteroTerrain", LANGID_USENGLISH},
	{"HybridMultiFractal", LANGID_USENGLISH},
	{"RidgedMultiFractal", LANGID_USENGLISH},
	{"Coriolis", LANGID_USENGLISH},
	{"Cyclone", LANGID_USENGLISH},
	{"Dented", LANGID_USENGLISH},
	{"PuffyClouds", LANGID_USENGLISH},
	{NULL}
};


/*
 * This plug-in module contains two servers for each texture: one for the
 * texture handler and one for the interface.
 */
ServerRecord ServerDesc[] =
{
	{ LWTEXTURE_HCLASS,	"fBmNoise",	            fBmNoiseActivate,  &userNames[0]},
	{ LWTEXTURE_ICLASS,	"fBmNoise",	            fBmNoiseInterface, &userNames[0] },
	{ LWTEXTURE_HCLASS,	"turbNoise",	        TurbNoiseActivate, &userNames[1]},
	{ LWTEXTURE_ICLASS,	"turbNoise",	        TurbNoiseInterface, &userNames[1]},
	{ LWTEXTURE_HCLASS,	"multiFractal",	        MultiFractalActivate, &userNames[2] },
	{ LWTEXTURE_ICLASS,	"multiFractal",	        MultiFractalInterface, &userNames[2]},
	{ LWTEXTURE_HCLASS,	"heteroTerrain",	    HeteroTerrainActivate, &userNames[3] },
	{ LWTEXTURE_ICLASS,	"heteroTerrain",	    HeteroTerrainInterface, &userNames[3] },
	{ LWTEXTURE_HCLASS,	"hybridMultiFractal",	HybridMFractalActivate, &userNames[4] },
	{ LWTEXTURE_ICLASS,	"hybridMultiFractal",	HybridMFractalInterface, &userNames[4] },
	{ LWTEXTURE_HCLASS,	"ridgedMultiFractal",	RidgedMFractalActivate, &userNames[5] },
	{ LWTEXTURE_ICLASS,	"ridgedMultiFractal",	RidgedMFractalInterface, &userNames[5] },
	{ LWTEXTURE_HCLASS,	"coriolis",	            CoriolisActivate, &userNames[6] },
	{ LWTEXTURE_ICLASS,	"coriolis",	            CoriolisInterface, &userNames[6] },
	{ LWTEXTURE_HCLASS,	"cyclone",	            CycloneActivate, &userNames[7] },
	{ LWTEXTURE_ICLASS,	"cyclone",	            CycloneInterface, &userNames[7] },
	{ LWTEXTURE_HCLASS,	"dented",	            DentedActivate, &userNames[8] },
	{ LWTEXTURE_ICLASS,	"dented",	            DentedInterface, &userNames[8] },
	{ LWTEXTURE_HCLASS,	"puffyClouds",	        PuffyCloudsActivate, &userNames[9] },
	{ LWTEXTURE_ICLASS,	"puffyClouds",	        PuffyCloudsInterface, &userNames[9] },
	{NULL}
};
