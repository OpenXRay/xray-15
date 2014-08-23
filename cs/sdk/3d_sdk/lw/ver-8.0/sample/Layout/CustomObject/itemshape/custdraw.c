 // Custom Object Class Drawing Routines
// Arnie Cachelin, Copyright 2001 NewTek, Inc.

#include <lwmath.h>
#include <lwcustobj.h>
#include <math.h>
#include "custdraw.h"
#include "minvert.h"

#define		ARC_STEP		RADIANS(12.0)   //  30 segs = 360/12

static int xax[] = {1,2,0}, yax[] = {2,0,1}, zax[] = {0,1,2};

void co_Arc(const LWCustomObjAccess *cobjAcc, double pos[3], double startAngle,double endAngle, double rad, int csys, int ax)
{
	double  endp[3], st[3], a, da = ARC_STEP;
	endp[yax[ax]] = rad*sin((startAngle));
	endp[xax[ax]] = rad*cos((startAngle));
	endp[ax] = pos[ax]; // endp[zax[ax]] = 0;
	st[ax] = pos[ax];
	for(a = (startAngle) + da; a<= endAngle; a += da)
	{
		st[xax[ax]] = endp[xax[ax]];
		st[yax[ax]] = endp[yax[ax]];
		endp[yax[ax]] = rad*sin(a);
		endp[xax[ax]] = rad*cos(a);
		(*cobjAcc->line)(cobjAcc->dispData, st,endp,csys);
	}
	if(a>endAngle)
	{
		a = endAngle;
		st[xax[ax]] = endp[xax[ax]];
		st[yax[ax]] = endp[yax[ax]];
		endp[yax[ax]] = rad*sin(a);
		endp[xax[ax]] = rad*cos(a);
		(*cobjAcc->line)(cobjAcc->dispData, st,endp,csys);
	}
}


void co_FillArc(const LWCustomObjAccess *cobjAcc, double pos[3], double startAngle,double endAngle, double rad, int csys, int ax)
{
	double  endp[3], st[3], a, da = ARC_STEP;
	endp[yax[ax]] = rad*sin((startAngle));
	endp[xax[ax]] = rad*cos((startAngle));
	endp[ax] = pos[ax]; // endp[zax[ax]] = 0;
	st[ax] = pos[ax];
	for(a = (startAngle) + da; a<= (endAngle)+da; a += da)
	{
		st[xax[ax]] = endp[xax[ax]];
		st[yax[ax]] = endp[yax[ax]];
		endp[yax[ax]] = rad*sin(a);
		endp[xax[ax]] = rad*cos(a);
		(*cobjAcc->triangle)(cobjAcc->dispData, pos,st,endp,csys);
	}
}

void co_Rectangle(const LWCustomObjAccess *cobjAcc, double pos[3], double w, double h, int csys, int axis)
{
	double dx,dy,stp[3],endp[3],z;
	int axx,axy;
	z = pos[axis];
	axx = xax[axis];
	axy = yax[axis];
	dx = 0.5*w;
	dy = 0.5*h;
	stp[axis] = endp[axis] = z;
	stp[axx] = pos[axx] - dx;
	stp[axy] = pos[axy] + dy;
	endp[axx] = pos[axx] + dx;
	endp[axy] = pos[axy] + dy;
	(*cobjAcc->line)(cobjAcc->dispData, stp,endp,csys);
	stp[axx] = pos[axx] + dx;
	stp[axy] = pos[axy] - dy;
	(*cobjAcc->line)(cobjAcc->dispData, endp,stp,csys);
	endp[axx] = pos[axx] - dx;
	endp[axy] = pos[axy] - dy;
	(*cobjAcc->line)(cobjAcc->dispData, stp,endp,csys);
	stp[axx] = pos[axx] - dx;
	stp[axy] = pos[axy] + dy;
	(*cobjAcc->line)(cobjAcc->dispData, endp,stp,csys);
}

void co_FillRect(const LWCustomObjAccess *cobjAcc, double pos[3], double w, double h, int csys, int axis)
{
	double dx,dy,stp[3],endp[3],crn[3],z;
	int axx,axy;
	z = pos[axis];
	axx = xax[axis];
	axy = yax[axis];
	dx = 0.5*w;
	dy = 0.5*h;
	stp[axis] = endp[axis] = z;
	stp[axx] = pos[axx] - dx;
	stp[axy] = pos[axy] + dy;
	endp[axx] = pos[axx] + dx;
	endp[axy] = pos[axy] - dy;
	VCPY(crn,stp);
	crn[axx] = endp[axx];
	(*cobjAcc->triangle)(cobjAcc->dispData, stp,crn,endp,csys);
	crn[axx] = stp[axx];
	crn[axy] = endp[axy];
	(*cobjAcc->triangle)(cobjAcc->dispData, stp,crn,endp,csys);
}


void co_Grid(const LWCustomObjAccess *cobjAcc, double pos[3], double siz[3], int div, int csys, int axis)
{
	double dx,dy,stp[3],endp[3],z;
	int axx,axy,i;
	z = pos[axis];
	axx = xax[axis];
	axy = yax[axis];
	dx = siz[axx]/div;
	dy = siz[axy]/div;
	stp[axis] = endp[axis] = z;
	stp[axx] = pos[axx];
	stp[axy] = pos[axy];
	endp[axx] = pos[axx];
	endp[axy] = pos[axy];

	endp[axx] += siz[axx];
	for(i=0;i<=div;i++)
	{
		if(i==1)
			(*cobjAcc->setPattern)(cobjAcc->dispData, LWLPAT_DOT);
		else if(i==div)
			(*cobjAcc->setPattern)(cobjAcc->dispData, LWLPAT_SOLID);
		(*cobjAcc->line)(cobjAcc->dispData, stp,endp,csys);
		stp[axy] += dy;
		endp[axy] += dy;
	}

	stp[axx] = pos[axx];
	stp[axy] = pos[axy];
	endp[axx] = pos[axx];
	endp[axy] = pos[axy];

	endp[axy] += siz[axy];
	for(i=0;i<=div;i++)
	{
		if(i==1)
			(*cobjAcc->setPattern)(cobjAcc->dispData, LWLPAT_DOT);
		else if(i==div)
			(*cobjAcc->setPattern)(cobjAcc->dispData, LWLPAT_SOLID);
		(*cobjAcc->line)(cobjAcc->dispData, stp,endp,csys);
		stp[axx] += dx;
		endp[axx] += dx;
	}

}

void co_Line(const LWCustomObjAccess *cobjAcc, LWDVector p0, LWDVector p1, int csys)
{
	LWDVector h1,h0;
	VCPY(h0,p0);
	VCPY(h1,p1);
	cobjAcc->line(cobjAcc->dispData, h0,h1,csys);
}


void co_Arrow(const LWCustomObjAccess *cobjAcc, double pos[3], double vec[3], double mag, int csys)
{
	double endp[3],r[3],v[3],hed[3], c[3];

	VADDS3(endp,pos,vec,mag);
	r[0] = vec[1] - vec[2]; /* perp vector */
	r[1] = vec[2] - vec[0];
	r[2] = vec[0] - vec[1];
	
	(*cobjAcc->line)(cobjAcc->dispData, pos,endp,csys);
	VADDS3(hed,pos,vec,mag*0.90);
	VCROSS(v,r,vec);  // perp to v,r
	mag *= 0.025;
	VSCL(v,mag);
	VSCL(r,mag);
	VSUB3(c,hed,v);
	VADD(v,hed);
	(*cobjAcc->triangle)(cobjAcc->dispData, v,c,endp,csys);
	VSUB3(c,hed,r);
	VADD(r,hed);
	(*cobjAcc->triangle)(cobjAcc->dispData, r,c,endp,csys);
}

// 2D (xy-plane) drawing Functions: HUD == Heads-Up Display!
double HUD_Depth(LWViewportInfo *ViewGlobal, int view)
{
	double	Near, Far, z;
	int	type;

	// RETURN 1 for ORTHO
	type = ViewGlobal->type(view);
	switch(type)
	{
		case LVVIEWT_BACK:
		case LVVIEWT_FRONT:
		case LVVIEWT_TOP:
		case LVVIEWT_BOTTOM:
		case LVVIEWT_RIGHT:
		case LVVIEWT_LEFT:
			z = 1;
			break;

		default:
			ViewGlobal->clip(view, &Near, &Far);
			z = 100*Near; // Allen's suggestion for good high-zoom appearance
			break;
	}
	return z; // tool handle preference for z==1 ???
}

double HUD_Transform(LWViewportInfo *ViewGlobal, int view, Matrix xf)
{
	double z, mat[9];
	z = HUD_Depth(ViewGlobal, view);
	ViewGlobal->xfrm(view, mat);
	VCPY(xf[0], mat);
	VCPY(xf[1], &(mat[3]) );
	VCPY(xf[2], &(mat[6]) );
	VSCL(xf[0],z);
	VSCL(xf[1],z);
	VSCL(xf[2],z);
	return z; 
}


void HUDPosition(LWViewportInfo *ViewGlobal, int view, LWDVector pos, const LWDVector dir)
{
	int type, ax = 0;
	double Near, Far, norm, z;

	type = ViewGlobal->type(view);
	ViewGlobal->clip(view, &Near, &Far);
	ViewGlobal->pos(view, pos);
	switch(type)
	{
		case LVVIEWT_BACK:
		case LVVIEWT_FRONT:
			ax++; // fallthrough: ax-> 2
		case LVVIEWT_TOP:
		case LVVIEWT_BOTTOM:
			ax++; // fallthrough: ax-> 1
		case LVVIEWT_RIGHT:
		case LVVIEWT_LEFT:
			pos[ax] = 0.5*(Near+Far); 
			break;

		case LVVIEWT_PERSPECTIVE:
		case LVVIEWT_LIGHT:
		case LVVIEWT_CAMERA:
			z = HUD_Depth(ViewGlobal, view);
			norm = VLEN(dir);
			if(norm>0)
				norm = 1.0/norm;
			norm *= z;
			VADDS(pos, dir, norm);
			break;
		case LVVIEWT_SCHEMATIC:
		case LVVIEWT_NONE:
		default:
			VCLR(pos);
			break;
	}
}



// transform pixel point into world space in place
void HUD_Point(LWViewportInfo *ViewGlobal, const LWCustomObjAccess *cob, Matrix m, LWDVector pt)
{
	LWDVector		p;

	VCPY(p,pt); MatrixApply(pt, m,p);
	HUDPosition(ViewGlobal, cob->view, p, cob->viewDir);
	VADD(pt, p);
}

// Draw pixel-space box
void HUD_Box(LWViewportInfo *ViewGlobal, const LWCustomObjAccess *cob, Matrix m, LWDVector corn, double w, double h, int flags)
{
	LWDVector		a,b,c,p, home;

	VCPY(b, corn);
	b[0] += w;
	VCPY(a, b);
	b[1] -= h;
	VCPY(c, corn);
	c[1] -= h;
	VCPY(p,a); MatrixApply(a, m,p);
	VCPY(p,b); MatrixApply(b, m,p);
	VCPY(p,c); MatrixApply(c, m,p);
	MatrixApply(p, m,corn);

	HUDPosition(ViewGlobal, cob->view, home, cob->viewDir);

	VADD(a, home);
	VADD(b, home);
	VADD(c, home);
	VADD(p, home);

	cob->line(cob->dispData, p, a, LWCSYS_WORLD);
	cob->line(cob->dispData, a, b, LWCSYS_WORLD);
	cob->line(cob->dispData, b, c, LWCSYS_WORLD);
	cob->line(cob->dispData, c, p, LWCSYS_WORLD);
	if(flags&HUDF_FILL)
	{
		cob->triangle(cob->dispData, p, a, b, LWCSYS_WORLD);
		cob->triangle(cob->dispData, c, p, b, LWCSYS_WORLD);
	}
}

void HUD_Line(LWViewportInfo *ViewGlobal, const LWCustomObjAccess *cob, Matrix m, LWDVector corn, double w, int flags)
{
	LWDVector		a,b,c,p, home;

	VCPY(b, corn);
	VCPY(a, b);
	b[0] += w;
	VCPY(p,a); MatrixApply(a, m,p);
	VCPY(p,b); MatrixApply(b, m,p);
	if(flags&HUDF_FILL)
	{
		VCPY(c, corn);
		c[0]+= 1;
		c[1]-= 1;
		c[2]+=1;
		VCPY(p, c);
		p[0] += w;
		VCPY(home,c); MatrixApply(c, m,home);
		VCPY(home,p); MatrixApply(p, m,home);
	}
	HUDPosition(ViewGlobal, cob->view, home, cob->viewDir);

	VADD(a, home);
	VADD(b, home);
	cob->line(cob->dispData, a, b, LWCSYS_WORLD);
	if(flags&HUDF_FILL)
	{
		float	shad[4] = {0, 0, 0, 0.5f};
		cob->setColor(cob->dispData, shad); // dangerous side-effect, leaves color changed
		VADD(c, home);
		VADD(p, home);
		cob->line(cob->dispData, c, p, LWCSYS_WORLD);
	}

}

void HUD_Knob(LWViewportInfo *ViewGlobal, const LWCustomObjAccess *cob, Matrix m, LWDVector cent, double siz, int flags)
{
	LWDVector   a, b, c, home;

	VCPY(a, cent);
	VCPY(b, cent);
	VCPY(c, cent);
	siz *= 0.5;

	if(flags&HUDF_SIDE)
	{
		a[0] += siz;
		b[0] -= siz;
		c[0] -= siz;
		b[1] -= siz;
		c[1] += siz;
	}
	else if(flags&HUDF_DOWN)
	{
		a[0] += siz;
		b[0] -= siz;
		a[1] += siz;
		b[1] += siz;
		c[1] -= siz;
	}
	else
	{
		a[0] += siz;
		b[0] -= siz;
		a[1] -= siz;
		b[1] -= siz;
		c[1] += siz;
	}
	
	VCPY(home,a); MatrixApply(a, m,home);
	VCPY(home,b); MatrixApply(b, m,home);
	VCPY(home,c); MatrixApply(c, m,home);
	HUDPosition(ViewGlobal, cob->view, home, cob->viewDir);

	VADD(a, home);
	VADD(b, home);
	VADD(c, home);

	if(flags&HUDF_LOCK)
	{
		LWDVector	d, p;
		VCPY(d, cent);
		VCPY(c, cent);
		d[0] += siz;
		c[0] -= siz;
		c[1] += siz;
		d[1] += siz;
		VCPY(p,c); MatrixApply(c, m,p);
		VCPY(p,d); MatrixApply(d, m,p);
		VADD(d, home);
		VADD(c, home);

		cob->line(cob->dispData, a, b, LWCSYS_WORLD);
		cob->line(cob->dispData, c, b, LWCSYS_WORLD);
		cob->line(cob->dispData, c, d, LWCSYS_WORLD);
		cob->line(cob->dispData, a, d, LWCSYS_WORLD);
	}
	else if(flags&HUDF_FILL)
	{
		float	shad[4] = {0, 0, 0, 0.5f}, glint[4] = {1, 1, 1, 0.5f};
		cob->triangle(cob->dispData, a, b, c, LWCSYS_WORLD);
		cob->setColor(cob->dispData, glint); 
		cob->line(cob->dispData, c, b, LWCSYS_WORLD);
		cob->setColor(cob->dispData, shad); // dangerous side-effect, leaves color changed
		cob->line(cob->dispData, a, b, LWCSYS_WORLD);
		cob->line(cob->dispData, a, c, LWCSYS_WORLD);
	}
	else
	{
		cob->line(cob->dispData, a, b, LWCSYS_WORLD);
		cob->line(cob->dispData, c, b, LWCSYS_WORLD);
		cob->line(cob->dispData, a, c, LWCSYS_WORLD);
	}
}
