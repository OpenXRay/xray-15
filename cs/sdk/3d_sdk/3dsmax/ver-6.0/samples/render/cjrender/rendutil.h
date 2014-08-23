//***************************************************************************
// CJRender - [rendutil.h] Sample Plugin Renderer for 3D Studio MAX.
// 
// By Christer Janson - Kinetix
//
// May     11, 1996	CCJ Initial coding
// October 28, 1996	CCJ Texture mapping, raycasting etc.
// December    1996	CCJ General cleanup
//
// Description:
// This is the header for some utility functions needed throughout the code
//
//***************************************************************************

int				isFacing(Point3& p0, Point3& p1, Point3& p2, int projType);
void			RemoveScaling(Matrix3 &m);
Point3			CalcBaryCoords(Point3 p0, Point3 p1, Point3 p2, Point3 p);
void			MakeFaceUV(Face *f, Point3 *tv);
BMM_Color_64	colTo64(Color c);
BOOL			TMNegParity(Matrix3 &m);
