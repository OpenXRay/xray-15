/******************************************************************************
	FILE: data.h

	DESCRIPTION:  Marker data for the shape snap

	CREATED BY: John Hutchinson		

	HISTORY: created 12/12/96

 *>	Copyright (c) 1994, All Rights Reserved.
 *****************************************************************************/

//DAta for the markers
static IPoint3 mark0verts[8]=
{ 
 IPoint3(-5,     0,      0),
 IPoint3(0,     0,      0),
 IPoint3(0,     -5,      0),
 IPoint3(-5,     -5,      0),
 IPoint3(-5,     5,      0),
 IPoint3(-5,     -5,      0),
 IPoint3(0,     -5,      0),
 IPoint3(5,     -5,      0)
};

static int mark0es[8]=
{
	GW_EDGE_VIS,
	GW_EDGE_VIS,
	GW_EDGE_VIS,
	GW_EDGE_VIS,
	GW_EDGE_SKIP,
	GW_EDGE_SKIP,
	GW_EDGE_VIS,
	GW_EDGE_VIS
};


static IPoint3 mark1verts[11]=
{ 
 IPoint3(-5,      5,      0),
 IPoint3(0,      5,      0),
 IPoint3(3,      2,      0),
 IPoint3(4,      0,      0),
 IPoint3(3,      -2,      0),
 IPoint3(0,     -4,      0),
 IPoint3(-3,      -2,      0),
 IPoint3(-4,      0,      0),
 IPoint3(-3,      2,      0),
 IPoint3(0,      5,      0),
 IPoint3(5,      5,      0)
};

static int mark1es[11]=
{
	GW_EDGE_VIS,
	GW_EDGE_VIS,
	GW_EDGE_VIS,
	GW_EDGE_VIS,
	GW_EDGE_VIS,
	GW_EDGE_VIS,
	GW_EDGE_VIS,
	GW_EDGE_VIS,
	GW_EDGE_VIS,
	GW_EDGE_VIS,
	GW_EDGE_VIS,
};



