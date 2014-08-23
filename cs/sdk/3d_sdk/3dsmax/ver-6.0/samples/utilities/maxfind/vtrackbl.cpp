/****************************************************************************
 MAX File Finder
 Christer Janson
 September 20, 1998
 VTrackBl.h - Virtual TrackBall Implementation
 Guts of the Virtual Trackball has been stolen from Microsoft Sample Code
 ***************************************************************************/
#define STRICT 

#include <windows.h>
#include <math.h>

#include "stdio.h"
#include "stdlib.h"

#include "vtrackbl.h"

// Support code

static void		AddQuats(float *q1, float *q2, float *dest);
static void		BuildRotmatrix(float m[4][4], float q[4]);
static void		AxisToQuat(float a[3], float phi, float q[4]);
static void		vzero(float *v);
static void		vset(float *v, float x, float y, float z);
static void		vsub(const float *src1, const float *src2, float *dst);
static void		vcopy(const float *v1, float *v2);
static void		vcross(const float *v1, const float *v2, float *cross);
static float	vlength(const float *v);
static void		vscale(float *v, float div);
static void		vnormal(float *v);
static float	vdot(const float *v1, const float *v2);
static void		vadd(const float *src1, const float *src2, float *dst);


BOOL VirtualTrackball::Init(HWND hWnd, int w, int h)
{
	hWndCapture	= hWnd;
	nWidth		= w;
	nHeight		= h;

	// Initialize to the initial display angle
	CalcQuat(curquat, 0.0f, 0.0f, 0.0f, 0.8f);
	// Initial spin
	CalcQuat(lastquat, 0.0f, 0.0f, 0.0f, 0.0f);

	return TRUE;
}

BOOL VirtualTrackball::Resize(int w, int h)
{
	nWidth	= w;
	nHeight	= h;

	return TRUE;
}

BOOL VirtualTrackball::OnLMouseDown(short x, short y)
{
	nMouseDownX	= x;
	nMouseDownY	= y;
	bButtonDown = TRUE;

	nMousePosX	= x;
	nMousePosY	= y;
	return TRUE;
}

BOOL VirtualTrackball::OnLMouseUp(short x, short y)
{
	bButtonDown = FALSE;
	nMousePosX	= x;
	nMousePosY	= y;

	return TRUE;
}

BOOL VirtualTrackball::OnMMouseDown(short x, short y)
{
	return FALSE;
}

BOOL VirtualTrackball::OnMMouseUp(short x, short y)
{
	return FALSE;
}

BOOL VirtualTrackball::OnRMouseDown(short x, short y)
{
	return FALSE;
}

BOOL VirtualTrackball::OnRMouseUp(short x, short y)
{
	return FALSE;
}

BOOL VirtualTrackball::OnMouseMove(DWORD keyFlags, short x, short y)
{
	BOOL	bRetval = FALSE;

	if (keyFlags & MK_LBUTTON) {
		nMousePosX	= x;
		nMousePosY	= y;
		bRetval = TRUE;
	}

	return bRetval;
}

BOOL VirtualTrackball::HasMotion()
{
	return bSpinning;
}




VirtualTrackball::VirtualTrackball()
{
	bButtonDown = FALSE;
	bSpinning = FALSE;
	bCanSpin = TRUE;
	bDamping = TRUE;
}


// Call to retrieve current matrix
void VirtualTrackball::CalcRotMatrix(float matRot[4][4])
{
	static bCallAfterMouseUp = FALSE;
    if (bButtonDown || bCallAfterMouseUp)
    {
		bCallAfterMouseUp = TRUE;
		// If mouse has moved since button was pressed, change quaternion.
        if (nMousePosX != nMouseDownX || nMousePosY != nMouseDownY)
        {
			// Negate all params for proper operation with glTranslate(-z)
            CalcQuat(lastquat,
                      -(2.0f * ( nWidth - nMouseDownX ) / nWidth - 1.0f),
                      -(2.0f * nMouseDownY / nHeight - 1.0f),
                      -(2.0f * ( nWidth - nMousePosX ) / nWidth - 1.0f),
                      -(2.0f * nMousePosY / nHeight - 1.0f)
                     );

            bSpinning = TRUE;
        }
        else
            bSpinning = FALSE;

        nMouseDownX = nMousePosX;
        nMouseDownY = nMousePosY;
    }

	if (!bButtonDown) {
		bCallAfterMouseUp = FALSE;
		}

    if ((bCanSpin || bButtonDown) && bSpinning) {
        AddQuats(lastquat, curquat, curquat);
	}

	// Damp the value
	if (bSpinning && !bButtonDown && IsDamping()) {
		CalcQuat(lastquat,
			lastCalcVal[0] - lastCalcVal[0]/50.0f,
			lastCalcVal[1] - lastCalcVal[1]/50.0f,
			lastCalcVal[2] - lastCalcVal[2]/50.0f,
			lastCalcVal[3] - lastCalcVal[3]/50.0f
         );

		// Stop spinning when rotation is near 0
		if (lastCalcVal[0] < 0.0002f &&
			lastCalcVal[1] < 0.0002f &&
			lastCalcVal[2] < 0.0002f &&
			lastCalcVal[3] < 0.0002f) {
			bSpinning = FALSE;
		}
	}

    BuildRotmatrix(matRot, curquat);
}

void VirtualTrackball::SetCanSpin(BOOL bSpin)
{
	bCanSpin = bSpin;
}

BOOL VirtualTrackball::GetCanSpin()
{
	return bCanSpin;
}

BOOL VirtualTrackball::IsDamping()
{
	return bDamping;
}

void VirtualTrackball::SetDamping(BOOL bDamp)
{
	bDamping = bDamp;
}

/*
 * Project an x,y pair onto a sphere of radius r OR a hyperbolic sheet
 * if we are away from the center of the sphere.
 */
float ProjectToSphere(float r, float x, float y)
{
    float d, t, z;

    d = (float) sqrt(x*x + y*y);
    if (d < r * 0.70710678118654752440f) {    /* Inside sphere */
		z = (float) sqrt(r*r - d*d);
    } else {           /* On hyperbola */
        t = r / 1.41421356237309504880f;
        z = t*t / d;
    }
    return z;
}

/*
 *  Given an axis and angle, compute quaternion.
 */
void AxisToQuat(float a[3], float phi, float q[4])
{
    vnormal(a);
    vcopy(a,q);
    vscale(q,(float) sin(phi/2.0f));
    q[3] = (float) cos(phi/2.0f);
}

/*
 * Ok, simulate a track-ball.  Project the points onto the virtual
 * trackball, then figure out the axis of rotation, which is the cross
 * product of P1 P2 and O P1 (O is the center of the ball, 0,0,0)
 * Note:  This is a deformed trackball-- is a trackball in the center,
 * but is deformed into a hyperbolic sheet of rotation away from the
 * center.  This particular function was chosen after trying out
 * several variations.
 * 
 * It is assumed that the arguments to this routine are in the range
 * (-1.0 ... 1.0)
 */
void VirtualTrackball::CalcQuat(float q[4], float p1x, float p1y, float p2x, float p2y)
{
    float a[3]; /* Axis of rotation */
    float phi;  /* how much to rotate about axis */
    float p1[3], p2[3], d[3];
    float t;

	/*
	char tbuf[255];
	sprintf(tbuf, "%f %f %f %f", p1x, p1y, p2x, p2y);
	SetWindowText(hWndCapture, tbuf);
	*/

	lastCalcVal[0] = p1x;
	lastCalcVal[1] = p1y;
	lastCalcVal[2] = p2x;
	lastCalcVal[3] = p2y;

    if (p1x == p2x && p1y == p2y) {
	/* Zero rotation */
        vzero(q); 
	q[3] = 1.0f; 
        return;
    }

    /*
     * First, figure out z-coordinates for projection of P1 and P2 to
     * deformed sphere
     */
    vset(p1,p1x,p1y,ProjectToSphere(TRACKBALLSIZE,p1x,p1y));
    vset(p2,p2x,p2y,ProjectToSphere(TRACKBALLSIZE,p2x,p2y));

    /*
     *  Now, we want the cross product of P1 and P2
     */
    vcross(p2,p1,a);

    /*
     *  Figure out how much to rotate around that axis.
     */
    vsub(p1,p2,d);
    t = vlength(d) / (2.0f*TRACKBALLSIZE);

    /*
     * Avoid problems with out-of-control values...
     */
    if (t > 1.0f) t = 1.0f;
    if (t < -1.0f) t = -1.0f;
    phi = 2.0f * (float) asin(t);

    AxisToQuat(a,phi,q);
}


/*
 * Quaternions always obey:  a^2 + b^2 + c^2 + d^2 = 1.0
 * If they don't add up to 1.0, dividing by their magnitued will
 * renormalize them.
 *
 * Note: See the following for more information on quaternions:
 * 
 * - Shoemake, K., Animating rotation with quaternion curves, Computer
 *   Graphics 19, No 3 (Proc. SIGGRAPH'85), 245-254, 1985.
 * - Pletinckx, D., Quaternion calculus as a basic tool in computer
 *   graphics, The Visual Computer 5, 2-13, 1989.
 */
void NormalizeQuat(float q[4])
{
    int i;
    float mag;

    mag = (q[0]*q[0] + q[1]*q[1] + q[2]*q[2] + q[3]*q[3]);
    for (i = 0; i < 4; i++) q[i] /= mag;
}

/*
 * Given two rotations, e1 and e2, expressed as quaternion rotations,
 * figure out the equivalent single rotation and stuff it into dest.
 * 
 * This routine also normalizes the result every RENORMCOUNT times it is
 * called, to keep error from creeping in.
 *
 * NOTE: This routine is written so that q1 or q2 may be the same
 * as dest (or each other).
 */

#define RENORMCOUNT 97

void AddQuats(float q1[4], float q2[4], float dest[4])
{
    static int count=0;
    float t1[4], t2[4], t3[4];
    float tf[4];

    vcopy(q1,t1); 
    vscale(t1,q2[3]);

    vcopy(q2,t2); 
    vscale(t2,q1[3]);

    vcross(q2,q1,t3);
    vadd(t1,t2,tf);
    vadd(t3,tf,tf);
    tf[3] = q1[3] * q2[3] - vdot(q1,q2);

    dest[0] = tf[0];
    dest[1] = tf[1];
    dest[2] = tf[2];
    dest[3] = tf[3];

    if (++count > RENORMCOUNT) {
        count = 0;
        NormalizeQuat(dest);
    }
}

/*
 * Build a rotation matrix, given a quaternion rotation.
 *
 */
void BuildRotmatrix(float m[4][4], float q[4])
{
    m[0][0] = 1.0f - 2.0f * (q[1] * q[1] + q[2] * q[2]);
    m[0][1] = 2.0f * (q[0] * q[1] - q[2] * q[3]);
    m[0][2] = 2.0f * (q[2] * q[0] + q[1] * q[3]);
    m[0][3] = 0.0f;

    m[1][0] = 2.0f * (q[0] * q[1] + q[2] * q[3]);
    m[1][1]= 1.0f - 2.0f * (q[2] * q[2] + q[0] * q[0]);
    m[1][2] = 2.0f * (q[1] * q[2] - q[0] * q[3]);
    m[1][3] = 0.0f;

    m[2][0] = 2.0f * (q[2] * q[0] - q[1] * q[3]);
    m[2][1] = 2.0f * (q[1] * q[2] + q[0] * q[3]);
    m[2][2] = 1.0f - 2.0f * (q[1] * q[1] + q[0] * q[0]);
    m[2][3] = 0.0f;

    m[3][0] = 0.0f;
    m[3][1] = 0.0f;
    m[3][2] = 0.0f;
    m[3][3] = 1.0f;
}



// Support code

void vzero(float *v)
{
    v[0] = 0.0f;
    v[1] = 0.0f;
    v[2] = 0.0f;
}

void vset(float *v, float x, float y, float z)
{
    v[0] = x;
    v[1] = y;
    v[2] = z;
}

void vsub(const float *src1, const float *src2, float *dst)
{
    dst[0] = src1[0] - src2[0];
    dst[1] = src1[1] - src2[1];
    dst[2] = src1[2] - src2[2];
}

void vcopy(const float *v1, float *v2)
{
    register int i;
    for (i = 0 ; i < 3 ; i++)
        v2[i] = v1[i];
}

void vcross(const float *v1, const float *v2, float *cross)
{
    float temp[3];

    temp[0] = (v1[1] * v2[2]) - (v1[2] * v2[1]);
    temp[1] = (v1[2] * v2[0]) - (v1[0] * v2[2]);
    temp[2] = (v1[0] * v2[1]) - (v1[1] * v2[0]);
    vcopy(temp, cross);
}

float vlength(const float *v)
{
    return (float) sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

void vscale(float *v, float div)
{
    v[0] *= div;
    v[1] *= div;
    v[2] *= div;
}

void vnormal(float *v)
{
    vscale(v,1.0f/vlength(v));
}

float vdot(const float *v1, const float *v2)
{
    return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
}

void vadd(const float *src1, const float *src2, float *dst)
{
    dst[0] = src1[0] + src2[0];
    dst[1] = src1[1] + src2[1];
    dst[2] = src1[2] + src2[2];
}