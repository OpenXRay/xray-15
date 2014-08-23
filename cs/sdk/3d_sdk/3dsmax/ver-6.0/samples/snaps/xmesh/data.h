/******************************************************************************
	FILE: data.h

	DESCRIPTION:  Marker data for the xmesh snap

	CREATED BY: John Hutchinson		

	HISTORY: created 12/12/96

 *>	Copyright (c) 1994, All Rights Reserved.
 *****************************************************************************/

//DAta for the one and only marker
static IPoint3 mark0verts[4]=
{ 
 IPoint3(-5,      0,      0),
 IPoint3(5,      0,      0),
 IPoint3(0,      -5,      0),
 IPoint3(0,      5,      0)
};

static int mark0es[4]=
{
	GW_EDGE_VIS,
	GW_EDGE_SKIP,
	GW_EDGE_VIS,
	GW_EDGE_VIS
};


static IPoint3 mark1verts[6]=
{ 
 IPoint3(5,      5,      0),
 IPoint3(-5,      5,      0),
 IPoint3(-5,      -5,      0),
 IPoint3(5,      -5,      0),
 IPoint3(5,      5,      0),
 IPoint3(0,      0,      0)
};

static int mark1es[6]=
{
	GW_EDGE_VIS,
	GW_EDGE_VIS,
	GW_EDGE_VIS,
	GW_EDGE_VIS,
	GW_EDGE_VIS,
	GW_EDGE_VIS
};

static IPoint3 mark2verts[5]=
{ 
 IPoint3(5,      5,      0),
 IPoint3(-5,      5,      0),
 IPoint3(-5,      -5,      0),
 IPoint3(5,      -5,      0),
 IPoint3(5,      5,      0)
};

static int mark2es[5]=
{
	GW_EDGE_VIS,
	GW_EDGE_VIS,
	GW_EDGE_VIS,
	GW_EDGE_VIS,
	GW_EDGE_VIS
};

static IPoint3 mark3verts[6]=
{ 
 IPoint3(5,      5,      0),
 IPoint3(-5,      5,      0),
 IPoint3(-5,      -5,      0),
 IPoint3(5,      -5,      0),
 IPoint3(5,      5,      0),
 IPoint3(-5,      -5,      0)
};

static int mark3es[6]=
{
	GW_EDGE_VIS,
	GW_EDGE_VIS,
	GW_EDGE_VIS,
	GW_EDGE_VIS,
	GW_EDGE_VIS,
	GW_EDGE_VIS
};

static IPoint3 mark4verts[4]=
{ 
 IPoint3(5,      -5,      0),
 IPoint3(0,      5,      0),
 IPoint3(-5,      -5,      0),
 IPoint3(5,      -5,      0)
};

static int mark4es[4]=
{
	GW_EDGE_VIS,
	GW_EDGE_VIS,
	GW_EDGE_VIS,
	GW_EDGE_VIS
};

static IPoint3 mark5verts[4]=
{ 
 IPoint3(5,      5,      0),
 IPoint3(-5,      5,      0),
 IPoint3(0,      -5,      0),
 IPoint3(5,      5,      0)
};

static int mark5es[4]=
{
	GW_EDGE_VIS,
	GW_EDGE_VIS,
	GW_EDGE_VIS,
	GW_EDGE_VIS
};


