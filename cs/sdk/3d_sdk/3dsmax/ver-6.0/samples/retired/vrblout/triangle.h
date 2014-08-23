/* Triangle.H
   Copyright 1995 by Autodesk, Inc.

   Initial Version: 11/10/95, Zijiang Yang

******************************************************************************
*                                                                            *
* The information contained herein is confidential, proprietary to Autodesk, *
* Inc., and considered a trade secret as defined in section 499C of the      *
* penal code of the State of California.  Use of this information by anyone  *
* other than authorized employees of Autodesk, Inc. is granted only under a  *
* written non-disclosure agreement, expressly prescribing the scope and      *
* manner of such use.                                                        *
*                                                                            *
******************************************************************************/


#ifndef _TRIANGLE_H_
#define _TRIANGLE_H_

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define EPLISON 0.000001
#define ISZERO(x) (x> -EPLISON && x < EPLISON)

/*--------------------------------LodVECTOR---------------------------------*/

//class LodVector is the basic structure for manipulating 3D coordinates
//it provide functionalities such as dot product, normalize, cross product
//it also has a special member function that compute the angle between two
//LodVectors 
class LodVECTOR {  
	float v[3];
public:
	LodVECTOR() {   }; // v[0] = 0.0;    v[1] = 0.0;    v[2] = 0.0;  }

	LodVECTOR(float v0, float v1, float v2) {  v[0] = v0;    v[1] = v1;   v[2] = v2;  }
	
	//addition
	const LodVECTOR operator+ (const LodVECTOR& vec) const 
	{
		LodVECTOR res(v[0]+vec.v[0], v[1] + vec.v[1], v[2] + vec.v[2]);
		return res;
	}

	//substraction
	const LodVECTOR operator- (const LodVECTOR& vec) const 
	{
		LodVECTOR res(v[0]-vec.v[0], v[1] - vec.v[1], v[2] - vec.v[2]);
		return res;
	}
	
	// dot product
	float operator* (const LodVECTOR& vec) const 
	{ 
		return(v[0]*vec.v[0]+ v[1] * vec.v[1] + v[2]*vec.v[2]);
	}

	// indexing
	float operator[](int i) { return v[i]; }
	
	// scaling
	const LodVECTOR operator* (float val)  const 
	{ 
		LodVECTOR res(v[0]*val, v[1]* val, v[2]* val);
		return res;
	}

	// cross product
	const LodVECTOR operator% (const LodVECTOR& vec)  const 
	{ 
		LodVECTOR res(v[1]*vec.v[2] - v[2] * vec.v[1], 
			      v[2]*vec.v[0] - v[0] * vec.v[2], 
			      v[0]*vec.v[1] - v[1] * vec.v[0]);
		return res;
	}

	//normalize this and return this
	const LodVECTOR operator! () const; // return the normalized vec of this

	//the magnitude
	float Mag() const {    return((float)sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]));  }
	
	// normalize this, and return the norm
	float Normalize(); 

	//compute the angle between this and vec
	float Angle(const LodVECTOR& vec) const;

	//initialization  with x, y, z
	void Set(float v0, float v1, float v2) {    v[0] = v0;    v[1] = v1;    v[2] = v2; }
	
	//initialization  with array
	void Set(float vv[]) {    v[0] = vv[0];    v[1] = vv[1];    v[2] = vv[2];  }	
};


/*--------------------------------LodTRG_ELM--------------------------------*/

class LodTRIANGLE;
class LodVERTEX;

//class LodTRG_ELM is the element in LodTRG_LT
class LodTRG_ELM 
{
public:
	//id ranges 0, 1, 2, when the lodTRG_LT to store the triangles that are sharing 
	//one vertex, id keeps track the relative index of this vertex in the trangle
	int id;

	//trg_ptr points to the LodTRANGLE
	LodTRIANGLE* trg_ptr ;
	LodTRG_ELM *next, *prev;	
	
	//Initialization-------------------------------------------------------
	LodTRG_ELM() { trg_ptr = NULL; next = prev = NULL; }
	LodTRG_ELM(LodTRIANGLE* trg) { trg_ptr = trg; next = prev = NULL;} 
	
	//Only used if the LodTRG_LT is the local list in a LodVERTEX-----------
	void SetIndex(int index) { id = index; }
	//get the id of the vertex before the local vertex in the LodTRANGLE
	const int GetPredID() const;
	//get the id of the vertex after the local vertex in the LodTRANGLE
	const int GetSuccID() const;
	//get the position of the vertex before the local vertex in the LodTRANGLE
	const LodVECTOR& GetPredVect() const;
	//get the position of the vertex after the local vertex in the LodTRANGLE
	const LodVECTOR& GetSuccVect() const;
	//get the pointer of the vertex before the local vertex in the LodTRANGLE
	LodVERTEX* GetPredPtr() const;
	//get the pointer of the vertex after the local vertex in the LodTRANGLE
	LodVERTEX* GetSuccPtr() const;

	//Get the triangle's normal
	const LodVECTOR& GetTrgNorm() const;
};

/*--------------------------------LodTRG_LT--------------------------------*/

//class LodTRG_LT provides the doulbe linked list structure for class LodTRIANGLE
//besides standard list structure manipulation member functions, it also has the API
//to get the information of the LodTRIANGLE structure it is pointing to 
//LodTRG_LT is used in two places, one is a global LodTRIANGLE list
//one is the list of LodTRIANGLEs that sharing one LodVERTEX

//The header structure for LodTRG_ELM, which make it a circular double linked list
class LodTRG_LT 
{
public:
	// num of elements
	int count;
	// since it's a circular structure , we only need a head, no tail 			
	LodTRG_ELM *head, *current;

	// initialization
	LodTRG_LT() { count = 0; head = current = NULL; }

	// member interfaces------------------------------------
	const int GetCount() const { return count; } // returns num of elements
	LodTRG_ELM* GetHead() const { return head; }
	LodTRG_ELM* GetCurrent() const { return current; }

	// list structure manipulation-----------------------------------

	//add a triangle to the list
	LodTRG_ELM* Add(LodTRIANGLE* trg);
	//add a triangle element to the list
	void Add(LodTRG_ELM* trg_elm);
	//remove the trg_elm from the trg list
	//assumption is made that the trg_elm is an element of the list
	void Remove(LodTRG_ELM* trg_elm);
	// same as removing the head of the list and return the removed element
	LodTRG_ELM* Pop();
	// Insert After the "current"
	void InsertAfter(LodTRG_ELM *trg_elm);
	// Insert Before the "current"
	void InsertBefore(LodTRG_ELM *trg_elm);
	// switch the position of the header and current
	void SwapHC();			
	//set the "current" to be elm
	void SetCurrent(LodTRG_ELM *elm);
	//de-allocation
	void Free();

	// for efficient memory management, we put the lodTRANGLEs on a freelist
	// for later use rather than to delete them
	// later we use the space from freelist to generate new trg_elms
	int AddOld(LodVERTEX* v1, LodVERTEX* v2, LodVERTEX* v3, LodTRG_LT *freelist);
	
	//disattach the triangle from the vertices and put it on the freelist
	void Dissolve(LodTRG_LT* trg_lt, LodTRG_LT* freelist);

	//The API that output the LodTRG_LT into an array of LodTRIANGLES pointed by 
	// *trgs------------------------------------------------------
	void OutPut(LodTRIANGLE* trgs);
};


/*--------------------------------LodVTX_ELM--------------------------------*/

//class LodVTX_ELM is the element of the LodVTX_LT
class LodVTX_ELM 
{
public:
	LodVERTEX *vtx_ptr;
	LodVTX_ELM *next, *prev;

	//constructors--------------------------------------------
	LodVTX_ELM() { vtx_ptr = NULL; next = prev = NULL; }
	LodVTX_ELM(LodVERTEX* vtx) { vtx_ptr = vtx; next = prev = NULL; } 

	//get the dihedral angle of the vertex
	const float GetAngle() const;
	//set the dihedral angle of the vertex
	void SetAngle(float tmpang);
	//get the vertex's shape type
	const int GetShapeType() const;
	//set the shape type
	void SetShapeType(int typ);
	//get the position of the vertex
	const LodVECTOR& GetLoc() const;
	//get the vertex's id
	const int GetID() const;

};

/*--------------------------------LodVTX_LT--------------------------------*/

//class LodVTX_LT provides a circular double linked list structure
//that stores LodVERTEX, it also provides the interface to get the information
//of the LodVERTEX that it is pointing to


//the circular double linked list structure
class LodVTX_LT 
{
	int count;		// num of elements
	
	LodVTX_ELM *head, *current;
public:

	LodVTX_LT() { count = 0; head = current = NULL; }

	//basic member interface methods
	const int GetCount() const { return count; } // returns num of elements
	LodVTX_ELM* GetHead() const { return head; }
	LodVTX_ELM* GetCurrent() const { return current; }

	//structure manipulation functions-----------------------------------

	//add vert to the list
	LodVTX_ELM* Add(LodVERTEX* vert);
	//add vert to the list and also let vert point to the LodVTX_ELM
	LodVTX_ELM* AddandPoint(LodVERTEX* vert);
	//remove from list
	void Remove(LodVTX_ELM* vtx_elm);
	//sort the list by dihedral angle
	void Sort();
	//insert the vtx_elm in the proper place in the list to keep the list
	//sorted
	void Relocate(LodVTX_ELM *vtx_elm); 
	//Triangulate the list of vertices, assuming that the vertex list forms
	//a loop
	void Triangulate(LodTRG_LT* trg_lt, LodTRG_LT* freelist, LodVTX_LT* vlt);
	//insert before the "current" element
	void InsertBefore(LodVTX_ELM* vtx_elm);
	//de-allocating
	void Free();
	//Output the vertex into an array of vertices pointed by verts
	void OutPut(LodVERTEX* verts);

	// LOD API---------------------------------------------------

	//Start the optimization use the LodVTX_LT and the list of triangles
	//put removed triangle on to freelist for later use
	//remove the most flatten vertices until certain percentage is reached
	//if it returns 0, it means that no further optimization
	//can be taken, and the percentage is not reached
	int Optimize( LodTRG_LT* trg_lt,LodTRG_LT* freelist, float percent);
};

/*--------------------------------LodVERTEX--------------------------------*/

//class LodVERTEX stores the information of the vertices that constructing the
//mesh
//It stores the position of the vertices, it can also store the normal
//and other information, such as color, smoothing group and texture coordinates
class LodVERTEX {
	int id;
	int degree;			// number of triangles adjacent
	LodTRG_LT trgs_lt;		// list of triangles meeting at the vertex
	int shape_type;			// indicate a boundary control point
							// or a sharp corner point
	float angle;	// the dihedral angle of the vertex

	LodVTX_ELM *ltElm;  //The back pointer to the LodVTX_ELM that pointing to this vertex

	LodVECTOR loc;  //the position of the vertex

	//Additional information-----------------------------------------------------
	//LodVECTOR norm; and other properties should go here, such as texture coordinates
	//color and smoothing groups 

public:
	//constructor
	LodVERTEX() {    degree = 0;       shape_type = 0; } //trgs_lt.count = 0; trgs_lt.head = trgs_lt.current = NULL;  }
				// accumulate the degree of each vert

	//basic member interface methods------------------------------
	//get the degree of adjacency
	const int GetDegree() const { return degree; }
	//initialization
	void Set(int index, LodVECTOR& v); // init the vertex
	void Set(int index, float v1, float v2, float v3); // init the vertex
	//add a triangle to this vertex
	LodTRG_ELM* AddTrg(LodTRIANGLE* trg, int i);
	//remove a trg elm from the vertex's trgs_lt
	void RemoveTrg(LodTRG_ELM* trg_in_vtx);
	//set the back pointer for later on locating this vertex in the vtx list
	void SetElm(LodVTX_ELM *elm) {  ltElm = elm; }
	//get the location of this vertex in the vtx list
	LodVTX_ELM* GetElm() { return ltElm; }
	//get the dihedral angle
	const float GetAngle() const { return angle; }
	//set the dihedral angle
	void SetAngle(float tmpang) { angle = tmpang; }
	//get the shape type of the vertex
	const int GetShapeType() const { return shape_type; }
	//set the shape type of this vertex
	void SetShapeType(int typ) { shape_type = typ; }
	//get the 3d coordinate of this vertex
	const LodVECTOR& GetLoc() { return loc; }
	//get the list of triangles sharing this vertex
	LodTRG_LT* GetTrgLT()  { return &trgs_lt; }
	//get the x, y, z component of the 3d coordinates
	float operator[](int i) { return loc[i];}
	//set the index of this vertex
	void SetID(int index) { id = index; }
	//get the index of this vertex
	const int GetID() const { return id; }

	//topology shape handling functions----------------------------------------
	// arrange adjacency of the trgs
	void SetTrgAdjacency();	
	//Compute the dihedral angle			
	void ComputeAngle();

	//Optimization function---------------------------------------------

	//Get rid off this vertex and re-trangulate the surrounding vertices
	void Flatten(LodTRG_LT* trg_lt, LodTRG_LT* freelist, LodVTX_LT* vlt);
};

/*----------------------------LodTRIANGLE----------------------------*/

//class LodTRIANGLE stores the basic element of a mesh, i.e. triangles

class LodTRIANGLE {
	int id;	
	int verts_id[3];  //indices of the three vertices
	LodVERTEX* verts[3]; //pointers to the three vertices
	LodTRG_ELM* trg_in_vtx[3];		// index of this triangle in the vertex
	LodTRG_ELM* trg_in_list;	//back pointer of this triangle in the LodTRG_LT
	LodVECTOR norm;	  //face normal of this triangle

public:
	//constructor-------------------------------------------------
	LodTRIANGLE() {    verts[0] = verts[1] = verts[2] = NULL;  }

	//initialization--------------------------------------------
	//basic member interface-----------------------------------
	// init the triangle
	void Set(int index, int a, int b, int c); 	
	// init the triangle
	void Set(int a, int b, int c);
	//initialize the triangle with vertex pointers 	
	void Set(LodVERTEX* v1, LodVERTEX* v2, LodVERTEX* v3) { verts[0] = v1; verts[1] = v2; verts[2] = v3; }
	//get the  index of this triangle
	const int GetVertID(int i) const { return verts_id[i]; }
	//get the pointer of the i'th vertex
	LodVERTEX* GetVertPtr(int i) const { return verts[i]; }
	//get the pointer to the location of this triangle in the i'th vertex
	LodTRG_ELM* GetVertELM(int i) const { return trg_in_vtx[i]; }
	//get the normal of this triangle
	const LodVECTOR& GetNorm() const { return norm; }				
	//get the i'th vertex's 3d coordinates
	const LodVECTOR& GetVect(int i) const { return verts[i]->GetLoc(); }
	//set the pointer to location in the triangle list
	void SetTrgInList(LodTRG_ELM* trg_elm) { trg_in_list = trg_elm; }
	//get the location of this triangle in the triangle list
	LodTRG_ELM* GetTrgInList() { return trg_in_list; }

	//geometry util------------------------------------------------
	// computer the norm, and ang
	void ComputeNormal();	
		
	//topology util-------------------------------------------------
	// Add the vertex to this trg and let the vertex point to this trg
	void EnterVtx(LodVERTEX *lodvtx);
	// Let the vertices point to this trg		
	void EnterVtx();	
	//remove this triangle and put it on the free list
	void Dissolve(LodTRG_LT *freelist);
	
};

//the basic API------------------------------------------------------------
//It takes an array of vertices and triangles, the number of vertices and triangles
//and the percentage of the old vertices that need to be kept
//it generates newvn many vertices and newfn many faces, and the result vertices and
//and triangles are put in newvtx and newtrg
//if it returns 0, it means that no further optimization
//can be taken, and the percentage is not reached
extern int LodGenerate(LodVERTEX* lodvtx, LodTRIANGLE* lodtrg, int nverts, int ntrgs, 
	LodVERTEX** newvtx, LodTRIANGLE** newtrg, int* newvn, int* newfn, float percent);


#endif  //_TRIANGLE_H_