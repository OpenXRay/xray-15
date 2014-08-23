/**********************************************************************
 *<
	FILE:			PFActions_Icon.h

	DESCRIPTION:	PF Icon Shape Data (declaration)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 07-24-2002

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef  _PFACTIONS_ICON_H_
#define  _PFACTIONS_ICON_H_

#include "max.h"

namespace PFActions {

class PFActionCreateCallback : public CreateMouseCallBack 
{
public:
	void init(Object* pfAction, IParamBlock2* pblock, int sizeParamIndex);
	int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat);
private:
	Object *m_pfAction;
	IParamBlock2* m_pblock;
	int m_sizeParamIndex;
	Point3 m_p0,m_p1;
	IPoint2 m_sp0, m_sp1;
};

class PFActionIcon
{
public:
	static void BuildBox( Mesh *mesh );
	static void BuildRectangle( Mesh *mesh );
	static void BuildSphere( Mesh *mesh );
	static void BuildCircle( Mesh *mesh );
	// a single outcoming arrow in direction of -Z
	static void BuildLogoArrowOut( PolyShape* shape );
	// 4 incoming parallel arrows in direction of -Z
	static void BuildLogo4ArrowsInParallel( PolyShape* shape);
	// 4 outcoming parallel arrows in direction of -Z
	static void BuildLogo4ArrowsOutParallel( PolyShape* shape);
	// 4 incoming arrows create a circle in XY plane
	static void BuildLogo4ArrowsInCircle( PolyShape* shape);
	// 4 outcoming arrows create a circle in XY plane
	static void BuildLogo4ArrowsOutCircle( PolyShape* shape);
	// 6 incoming arrows create a sphere
	static void BuildLogo6ArrowsInSphere( PolyShape* shape);
	// 6 outcoming arrows create a sphere
	static void BuildLogo6ArrowsOutSphere( PolyShape* shape);
	// 12 incoming arrows touching sphere surface
	static void BuildLogo12ArrowsInSphereSurface( PolyShape* shape);
};

} // end of namespace PFActions


#endif // _PFACTIONS_ICON_H_