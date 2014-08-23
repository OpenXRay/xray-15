/****************************************************************************
 MAX File Finder
 Christer Janson
 September 20, 1998
 VTrackBl.h - Header for Virtual TrackBall
 Guts of the Virtual Trackball has been stolen from Microsoft Sample Code
 ***************************************************************************/

#ifndef __VTRACKBL_H__
#define __VTRACKBL_H__

#include "cmdmode.h"

class VirtualTrackball : public ICommandMode {
public:
	VirtualTrackball();

	virtual BOOL	Init(HWND hWnd, int nWidth, int nHeight);
	virtual BOOL	Resize(int nWidth, int nHeight);
	virtual BOOL	OnLMouseDown(short x, short y);
	virtual BOOL	OnLMouseUp(short x, short y);
	virtual BOOL	OnMMouseDown(short x, short y);
	virtual BOOL	OnMMouseUp(short x, short y);
	virtual BOOL	OnRMouseDown(short x, short y);
	virtual BOOL	OnRMouseUp(short x, short y);
	virtual BOOL	OnMouseMove(DWORD keyFlags, short x, short y);
	virtual BOOL	HasMotion();


	// Call to retrieve current matrix
	void	CalcRotMatrix(float matRot[4][4]);

	void	SetCanSpin(BOOL bSpin);
	BOOL	GetCanSpin();
	BOOL	IsDamping();
	void	SetDamping(BOOL bDamp);

private:
	void	CalcQuat(float q[4], float p1x, float p1y, float p2x, float p2y);

private:
	HWND	hWndCapture;
	int		nWidth;
	int		nHeight;

	int		nMouseDownX;
	int		nMouseDownY;
	int		nMousePosX;
	int		nMousePosY;
	BOOL	bButtonDown;
	BOOL	bSpinning;
	BOOL	bCanSpin;
	BOOL	bDamping;

	float	curquat[4];
	float	lastquat[4];

	float	lastCalcVal[4];
};

/*
 * This size should really be based on the distance from the center of
 * rotation to the point on the object underneath the mouse.  That
 * point would then track the mouse as closely as possible.  This is a
 * simple example, though, so that is left as an Exercise for the
 * Programmer.
 */
#define TRACKBALLSIZE  (0.8f)

#endif __VTRACKBL_H__

