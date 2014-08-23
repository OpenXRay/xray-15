// This header utilized only in VIZ.

#pragma once

#ifdef GEOREFSYS_UVW_MAPPING 
#include <GeoTableItem.h>
#endif
#include "tiffiop.h"

typedef struct {
	short keyID;
	short keyType;
	short keyCount;
	void *keyData;
} GTIFFKEYENTRY;

typedef struct {
	Point3 scale;
	Matrix3 transform;
	short numTies;
	Point3 * tiePoints;
} MODELHDR;

typedef struct {
	short width;
	short height;
	short version;
	short majorRev;
	short minorRev;
	short numKeys;
	MODELHDR model;
	GTIFFKEYENTRY * keys;
} GTIFF;


extern GTIFF * GeoTIFFRead(TIFF * tf, TIFFDirectory * td);
extern bool GeoTIFFCoordSysName(GTIFF * gt, char name[256]);
extern Matrix3 GeoTIFFModelTransform(GTIFF * gt);
extern void GeoTIFFClose(GTIFF * gt);
#define GEOREFSYS_UVW_MAPPING
extern void GeoTIFFExtents(GeoTableItem * data, GTIFF * gt);
#endif
