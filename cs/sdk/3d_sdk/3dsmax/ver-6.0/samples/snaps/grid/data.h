/*****************************************************************************
 *<
	FILE: data.h

	DESCRIPTION:  Marker data for the grid snap

	CREATED BY: John Hutchinson		

	HISTORY: created 12/12/96

 *>	Copyright (c) 1994, All Rights Reserved.
 *****************************************************************************/

//DAta for the markers
static IPoint3 mark0verts[5]=
{ 
 IPoint3(5,      5,      0),
 IPoint3(-5,      5,      0),
 IPoint3(-5,      -5,      0),
 IPoint3(5,      -5,      0),
 IPoint3(5,      5,      0)
};

static int mark0es[5]=
{
	GW_EDGE_VIS,
	GW_EDGE_VIS,
	GW_EDGE_VIS,
	GW_EDGE_VIS,
	GW_EDGE_VIS
};




