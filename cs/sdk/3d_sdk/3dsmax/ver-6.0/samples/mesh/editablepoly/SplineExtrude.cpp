/**********************************************************************  
 *<
	FILE: SplineExtrude.cpp

	DESCRIPTION: Spline-based extrusion (similar to Loft) mathematics.

	CREATED BY: Steve Anderson

	HISTORY: created Nov 26, 2001, based on spline/patch modeler for 3D Studio R4,
	which was created by Minerva Software, Inc. under contract to Autodesk in 1995.

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "EPoly.h"
#include "PolyEdit.h"
#include "MeshDLib.h"
#include "macrorec.h"
#include "decomp.h"
#include "spline3d.h"
#include "splshape.h"
#include "shape.h"

static BasisFinder theBasisFinder;

// Luna task 748T
void ConvertPathToFrenets (Spline3D *pSpline, Matrix3 & relativeTransform, Tab<Matrix3> & tFrenets,
						   int numSegs, bool align, float rotateAroundZ) {
	// Given a path, a sequence of points in 3-space, create transforms
	// for putting a cross-section around each of those points, loft-style.

	// bezShape is provided by user, tFrenets contains output, numSegs is one less than the number of transforms requested.

	// Strategy: The Z-axis is mapped along the path, and the X and Y axes 
	// are chosen in a well-defined manner to get an orthonormal basis.

	int i;

	if (numSegs < 1) return;
	tFrenets.SetCount (numSegs+1);

	int numIntervals = pSpline->Closed() ? numSegs+1 : numSegs;
	float denominator = float(numIntervals);
	Point3 xDir, yDir, zDir, location, tangent, axis;
	float position, sine, cosine, theta;
	Matrix3 rotation;

	// Find initial x,y directions:
	location = relativeTransform * pSpline->InterpCurve3D (0.0f);
	tangent = relativeTransform.VectorTransform (pSpline->TangentCurve3D (0.0f));
	Point3 lastTangent = tangent;

	Matrix3 inverseBasisOfSpline(1);
	if (align) {
		theBasisFinder.BasisFromZDir (tangent, xDir, yDir);
		if (rotateAroundZ) {
			Matrix3 rotator(1);
			rotator.SetRotate (AngAxis (tangent, rotateAroundZ));
			xDir = xDir * rotator;
			yDir = yDir * rotator;
		}
		Matrix3 basisOfSpline(1);
		basisOfSpline.SetRow (0, xDir);
		basisOfSpline.SetRow (1, yDir);
		basisOfSpline.SetRow (2, tangent);
		basisOfSpline.SetTrans (location);
		inverseBasisOfSpline = Inverse (basisOfSpline);
		lastTangent = Point3(0,0,1);
	} else {
		inverseBasisOfSpline.SetRow (3, -location);
	}

	// Make relative transform take the spline from its own object space to our object space,
	// and from there into the space defined by its origin and initial direction:
	relativeTransform = relativeTransform * inverseBasisOfSpline;
	// (Note left-to-right evaluation order: Given matrices A,B, point x, x(AB) = (xA)B

	// The first transform is necessarily the identity:
	tFrenets[0].IdentityMatrix ();

	// Set up xDir, yDir, zDir to match our first-point basis:
	xDir = Point3 (1,0,0);
	yDir = Point3 (0,1,0);
	zDir = Point3 (0,0,1);

	for (i=1; i<=numIntervals; i++) {
		position = float(i) / denominator;
		location = relativeTransform * pSpline->InterpCurve3D (position);
		tangent = relativeTransform.VectorTransform (pSpline->TangentCurve3D (position));

		// This is the procedure we follow at each step in the path: find the
		// orthonormal basis with the right orientation, then compose with
		// the translation putting the origin at the path-point.

		// As we proceed along the path, we apply minimal rotations to
		// our original basis to keep the Z-axis tangent to the curve.
		// The X and Y axes follow in a natural manner.

		// xDir, yDir, zDir still have their values from last time...
		// Create a rotation matrix which maps the last tangent onto the current tangent:
		axis = lastTangent ^ tangent;	// gives axis, scaled by sine of angle.
		sine = FLength(axis);	// positive - keeps angle value in (0,PI) range.
		cosine = DotProd (lastTangent, tangent);	// Gives cosine of angle.
		theta = atan2f (sine, cosine);
		rotation.SetRotate (AngAxis (Normalize(axis), theta));
		xDir = Normalize (rotation * xDir);
		yDir = Normalize (rotation * yDir);
		zDir = Normalize (rotation * zDir);
		lastTangent = tangent;

		if (i<=numSegs) {
			tFrenets[i].IdentityMatrix ();
			tFrenets[i].SetRow (0, xDir);
			tFrenets[i].SetRow (1, yDir);
			tFrenets[i].SetRow (2, zDir);
			tFrenets[i].SetTrans (location);
		}
	}

	/* following code not needed for now - treating all splines as open.
	if (!pSpline->Closed ()) return;

	// If we have a closed loop, our procedure may result in X,Y vectors
	// that are twisted severely between the first and last point.
	// Here we correct for that by pre-transforming each frenet transform
	// by a small rotation around the Z-axis.  (The last pass through the above
	// loop should give the final twist: the initial and final Z-axis directions
	// are the same, and the difference between the initial and final X-axis
	// directions is the total twist along the spline.)

	// Now we measure the difference between our original and final x-vectors:
	axis = originalXDir ^ xDir;
	sine = FLength (axis);
	cosine = DotProd (originalXDir, xDir);
	theta = atan2f (sine, cosine);
	for (i=1; i<=numSegs; i++) {
		rotation.SetRotate (AngAxis (Normalize(axis), -theta/denominator));
		tFrenets[i] = rotation * tFrenets[i];
	}
*/	
}

