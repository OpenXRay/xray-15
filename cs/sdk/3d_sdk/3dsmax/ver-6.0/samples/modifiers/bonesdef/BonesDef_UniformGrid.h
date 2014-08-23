#ifndef __BONESDEFUNIFORMGRID__H
#define __BONESDEFUNIFORMGRID__H

//Fast closest vertex lookup using a uniform grid

//juat a table of all the points in a cell
class Grid
{

public:
	Tab<int> index;
};

//Uniform Grid class to look up closest vertices faster than the typical
//nxn lookup
class UniformGrid
{
public:
	UniformGrid();
	~UniformGrid();

	//Initializes the grid.  The size if the width of the grid.  The bigger
	//the width the faster but the more memory
	void InitializeGrid(int size,int whichGrid);

	//Frees all our data
	void FreeGrid();

	//This loads up the points we want to check against
	void LoadPoints(Point3 *p, int count);

	//Given a point find the closest point form our LoadPoints function
	//	p the point to check
	//	radius the farthest distance to look out from p.  This is a threshold
	//		the tighter the threshold the faster the look up
	//	i the returning index of the table based into Load Points
	//	d the distance from p to the closest point
	void ClosestPoint(Point3 p, float radius, int &i, float &d);


	//Deguging tools that let you see the grid in the view port
	void DrawBounds(GraphicsWindow *gw, Box3 bounds);
	void Display(GraphicsWindow *gw);

	//some debug globals
	BOOL debug;
	int debugI;
	Point3 debugP;


private:

	//returns which cell a particle point in space occupies in our grid
	// x,y the position of the point in grid space
	// whichGrid is which grid to check.  we have 3 
	//		0 X aligned grid
	//		1 Y aligned grid
	//		2 Z aligned grid
	//	ix,iy the closest cell coord that contain this point
	//	returns the index into the Grid that contains this point
	int FindCell(float x, float y, int whichGrid, int &ix, int &iy);

	//This goes through a grid and tags cells which could potential hold
	//our closest point
	// p is the point we are checking
	// radius the farthest distance to look out from p.  This is a threshold
	//		the tighter the threshold the faster the look up
	// whichGrid is which grid we are filling out
	void TagCells(Point3 p,float radius, int whichGrid);

	//the bounds of all our points
	Box3 bounds;
	//the width, height and length of our bounds
	float fXWidth,fYWidth,fZWidth;


	//width of our grid
	int width;
	// width  times width
	int widthXwidth;

	//copy list of all our points from LoadPoints
	Tab<Point3> pointBase;

	//our grid containers
	Tab<Grid*> xGrid;
	Tab<Grid*> yGrid;
	Tab<Grid*> zGrid;

	//this is a list indices of all our potential hits.  There maybe duplicates
	//in here
	Tab<int> hitList;
	//list of all potential hit indices per grid 
	BitArray xHitList, yHitList, zHitList;

	BOOL activeGrid;
	int numberOfHits;
	int numberOfChecks;
	float mRadius;

	int largestCellCount;
	int whichLargestCell;

};

#endif
