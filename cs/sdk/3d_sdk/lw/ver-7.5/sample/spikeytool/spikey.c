/*
 * SPIKEY.C -- Make a polygonal object spikey while subdividing
 *
 * Copyright 1999, NewTek, Inc.
 * written by Stuart Ferguson
 * last revision  8/30/99
 */
#include "spikey.h"


/*
 * Interface globals.
 */
DynaMonitorFuncs	*globFun_mon = NULL;
LWXPanelFuncs		*globFun_pan = NULL;


/*
 * Local information packet.  This includes the mesh edit context, monitor,
 * and polygon count.  Also holds the variable spike factor.
 */
typedef struct st_SpikeyData {
	MeshEditOp		*op;
	LWMonitor		*mon;
	unsigned int		 count;
	double			 spike;
} SpikeyData;


/*
 * Utility to return the distance between two 3D points.
 */
	static double
Dist3D (
	double			 v1[3],
	double			 v2[3])
{
	double			 d, z;
	int			 i;

	d = 0.0;
	for (i = 0; i < 3; i++) {
		z = v1[i] - v2[i];
		d += z * z;
	}
	return sqrt (d);
}


/*
 * This is called for all the polygons as a preliminary pass to count the
 * affected ones.  Polygons are only processed if they are faces, they are
 * selected and they have at least three vertices.
 */
	static EDError
CountPols (
	SpikeyData		*dat,
	const EDPolygonInfo	*pi)
{
	if ((pi->flags & EDDF_SELECT) && pi->numPnts >= 3 &&
	    (pi->type == LWPOLTYPE_FACE || pi->type == LWPOLTYPE_PTCH))
		dat->count ++;

	return EDERR_NONE;
}


/*
 * This is called for all the polygons to delete them and replace them with
 * triangles around a centeral displaced point.  The check for which ones
 * to process and which to skip is the same as above.
 */
	static EDError
Subdivide (
	SpikeyData		*dat,
	const EDPolygonInfo	*pi)
{
	MeshEditOp		*op = dat->op;
	EDStateRef		 s = op->state;
	EDPointInfo		*vi;
	double			 cen[3], norm[3], d;
	LWPntID			 cg;
	LWPntID			 vl[3];
	int			 i;

	if (!( (pi->flags & EDDF_SELECT) && pi->numPnts >= 3 &&
	       (pi->type == LWPOLTYPE_FACE || pi->type == LWPOLTYPE_PTCH) ))
		return EDERR_NONE;

	/*
	 * Count this polygon in the aggregate for the monitor.  The
	 * step function returns True if the user has requested an abort,
	 * which we can propogate by returning the appropriate code.
	 */
	if (dat->mon && (*dat->mon->step) (dat->mon->data, 1))
		return EDERR_USERABORT;

	/*
	 * Compute the CG of the polygon vertices as `cen.'
	 */
	cen[0] = cen[1] = cen[2] = 0.0;
	for (i = 0; i < pi->numPnts; i++) {
		vi = (*op->pointInfo) (s, pi->points[i]);
		if (!vi)
			return EDERR_NOMEMORY;

		cen[0] += vi->position[0];
		cen[1] += vi->position[1];
		cen[2] += vi->position[2];
	}
	cen[0] = cen[0] / pi->numPnts;
	cen[1] = cen[1] / pi->numPnts;
	cen[2] = cen[2] / pi->numPnts;

	/*
	 * Compute the average distance from a polygon vertex to
	 * the center point.
	 */
	d = 0.0;
	for (i = 0; i < pi->numPnts; i++) {
		vi = (*op->pointInfo) (s, pi->points[i]);
		if (!vi)
			return EDERR_NOMEMORY;

		d += Dist3D (vi->position, cen);
	}
	d = d / pi->numPnts;

	/*
	 * Translate the center point out of the polygon's plane by this
	 * average distance times the spikeyness factor.  This translation
	 * can only be done if the polygon has a valid normal.
	 */
	d *= dat->spike;
	if (d && (*op->polyNormal) (op->state, pi->pol, norm)) {
		cen[0] += norm[0] * d;
		cen[1] += norm[1] * d;
		cen[2] += norm[2] * d;
	}

	/*
	 * Create a new point at the offset center position.  VMAP vectors
	 * will be an average of the values at the polygon vertices.
	 */
	cg = (*op->addIPnt) (s, cen, pi->numPnts, pi->points, NULL);
	if (!cg)
		return EDERR_NOMEMORY;

	/*
	 * Loop over points in the polygon and create new triangles
	 * from each pair to the center.  We use `addPoly' rather than
	 * `addTri' since we want to preserve the original polygon
	 * tags and sidedness.  Note the "modulo numPnts" to make
	 * the point index wrap around the end of the point array.
	 */
	vl[0] = cg;
	for (i = 0; i < pi->numPnts; i++) {
		vl[1] = pi->points[i];
		vl[2] = pi->points[(i + 1) % pi->numPnts];
		if (!(*op->addPoly) (s, pi->type, pi->pol, NULL, 3, vl))
			return EDERR_NOMEMORY;
	}

	/*
	 * Delete the orginal and we're done.
	 */
	return ((*op->remPoly) (op->state, pi->pol));
}


/*
 * Perform a spikey subdivide given a MeshEdit context and a spike factor.
 * This returns the error code, if any.
 */
	int
Spikey (
	MeshEditOp		*op,
	double			 factor)
{
	SpikeyData		 dat;
	EDError			 err;

	dat.op    = op;
	dat.spike = factor;

	dat.count = 0;
	err = (*op->polyScan) (op->state, CountPols, &dat, OPLYR_PRIMARY);
	if (err) {
		(*op->done) (op->state, err, 0);
		return err;
	}

	/*
	 * Start the monitor.  If we can create a monitor, initialize it
	 * with the polygon count.
	 */
	dat.mon = (*globFun_mon->create) ("Spikey Subdivide", NULL);
	if (dat.mon)
		(*dat.mon->init) (dat.mon->data, dat.count);

	/*
	 * We will alter polygons in the primary layer only since we
	 * can only create new data in this layer.
	 */
	err = (*op->polyScan) (op->state, Subdivide, &dat, OPLYR_PRIMARY);

	/*
	 * End the monitor whether we completed successfully or not.
	 */
	if (dat.mon) {
		(*dat.mon->done) (dat.mon->data);
		(*globFun_mon->destroy) (dat.mon);
	}

	/*
	 * Complete the operation by calling `done.'  We pass the error
	 * code if there was one, or NONE.  We also want to select any
	 * new data if there was data selected to start with.  This is
	 * important for maintaining the user's idea that the same stuff
	 * is selected even though we replaced each polygon with several
	 * new ones.
	 */
	(*op->done) (op->state, err, EDSELM_SELECTNEW);
	return err;
}

