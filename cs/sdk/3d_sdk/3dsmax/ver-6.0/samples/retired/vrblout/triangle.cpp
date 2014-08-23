/* TRIANGLE.CPP
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


#include "triangle.h"



int PRED_INDEX[3] = { 2, 0, 1};
int SUCC_INDEX[3] = { 1, 2, 0};

//the exception handling function
//should be modified
void
LodError(char* str)
{
	exit(0);
}

/*--------------------------------LodVECTOR---------------------------------*/
//class LodVector is the basic structure for manipulating 3D coordinates
//it provide functionalities such as dot product, normalize, cross product
//it also has a special member function that compute the angle between two
//LodVectors 

//normalize this and return this
const LodVECTOR  
LodVECTOR::operator! () const 
{				// normalizing   
  float r = this->Mag();
  if (ISZERO(r)) 
  {
    r = 1.0f;
    /* ERROR_MSG("zero LodVECTOR exception\n"); */
  } 
  else 
  {
    r = 1.0f/ r;
  }
  LodVECTOR res(v[0]* r, v[1]* r,  v[2]* r);
  return res;
}

// normalize this, and return the norm
float 
LodVECTOR::Normalize() 
{				// normalize this, and assign it to vec
  float r = this->Mag();
  float temp;
  if (ISZERO(r)) 
  {
    temp = 1.0f;
    /* ERROR_MSG("zero LodVECTOR exception\n"); */
  } 
  else 
  {
    temp = 1.0f/ r;
  }
  v[0] *= temp;
  v[1] *= temp;
  v[2] *= temp;

  return r;
}

//compute the angle between this and vec
float				// the angle acos of two normalized vectors
LodVECTOR::Angle(const LodVECTOR& vec) const
{
	float tmf = this->operator*(vec);
	if(tmf<0.99999999 && tmf>-0.9999999)
		return ((float)acos(tmf));
	else {
		if(tmf < -0.9999999) return 3.14159265359f;
		else return 0.0f;
	}
}

/*--------------------------------LodTRG_ELM--------------------------------*/

//class LodTRG_ELM is the element in LodTRG_LT

//get the id of the vertex before the local vertex in the LodTRANGLE
const int
LodTRG_ELM::GetPredID() const
{
	return trg_ptr->GetVertID(PRED_INDEX[id]);
}


//get the id of the vertex before the local vertex in the LodTRANGLE
const int
LodTRG_ELM::GetSuccID() const
{
	return trg_ptr->GetVertID(SUCC_INDEX[id]);
}

//get the position of the vertex before the local vertex in the LodTRANGLE
const LodVECTOR&
LodTRG_ELM::GetPredVect() const
{
	return trg_ptr->GetVect(PRED_INDEX[id]);
}

//get the position of the vertex after the local vertex in the LodTRANGLE
const LodVECTOR&
LodTRG_ELM::GetSuccVect() const
{
	return trg_ptr->GetVect(SUCC_INDEX[id]);
}

//get the pointer of the vertex before the local vertex in the LodTRANGLE
LodVERTEX*
LodTRG_ELM::GetPredPtr() const
{
	return trg_ptr->GetVertPtr(PRED_INDEX[id]);
}

//get the pointer of the vertex after the local vertex in the LodTRANGLE
LodVERTEX*
LodTRG_ELM::GetSuccPtr() const
{
	return trg_ptr->GetVertPtr(SUCC_INDEX[id]);
}

//Get the triangle's normal
const LodVECTOR&
LodTRG_ELM::GetTrgNorm() const
{
	return trg_ptr->GetNorm();
}


/*--------------------------------LodTRG_LT--------------------------------*/

//class LodTRG_LT provides the doulbe linked list structure for class LodTRIANGLE
//besides standard list structure manipulation member functions, it also has the API
//to get the information of the LodTRIANGLE structure it is pointing to 
//LodTRG_LT is used in two places, one is a global LodTRIANGLE list
//one is the list of LodTRIANGLEs that sharing one LodVERTEX

//add a triangle to the list
LodTRG_ELM*
LodTRG_LT::Add(LodTRIANGLE* trg) 
{
	LodTRG_ELM* new_elm = (new LodTRG_ELM(trg));
	if(head==NULL) 
	{
		head = current = new_elm;
		new_elm->prev = new_elm->next = new_elm;		
	}
	else 
	{
		new_elm->next = current->next;
		current->next->prev = new_elm;
		current->next = new_elm;
		new_elm->prev = current;
		current = new_elm;
	}		
	count++;
	return current;
}

//add a triangle element to the list
void
LodTRG_LT::Add(LodTRG_ELM* new_elm) 
{
	if(head==NULL) 
	{
		head = current = new_elm;
		new_elm->prev = new_elm->next = new_elm;		
	}
	else 
	{
		new_elm->next = current->next;
		current->next->prev = new_elm;
		current->next = new_elm;
		new_elm->prev = current;
		current = new_elm;
	}		
	count++;
}


// for efficient memory management, we put the lodTRANGLEs on a freelist
// for later use rather than to delete them
// later we use the space from freelist to generate new trg_elms
int 
LodTRG_LT::AddOld(LodVERTEX* v1, LodVERTEX* v2, LodVERTEX* v3, LodTRG_LT *freelist)
{
	if(v1==v2 || v1==v3 || v2 == v3) {
		return 0;
	}

	LodTRG_ELM* tmp_elm = freelist->Pop();
	
	if(tmp_elm==NULL) LodError("LodTRG_LT::AddOld");

	tmp_elm->trg_ptr->Set(v1, v2, v3);
	tmp_elm->trg_ptr->Set(v1->GetID(), v2->GetID(), v3->GetID());

	tmp_elm->trg_ptr->EnterVtx();

	tmp_elm->trg_ptr->ComputeNormal();
	tmp_elm->trg_ptr->SetTrgInList(tmp_elm);

	Add(tmp_elm);
   	return 1;
	
}

//remove the trg_elm from the trg list
//assumption is made that the trg_elm is an element of the list
void 
LodTRG_LT::Remove(LodTRG_ELM* trg_elm)
{
	if(head==trg_elm)
	{
		if (head->next == head) 
		{
			head = current = NULL;
		}
		else 
		{
			head = head->next;
		}
	}
	if(current == trg_elm) 
	{
		current = trg_elm->prev;
	}
	if(trg_elm->prev == NULL || trg_elm->next==NULL || count < 1) 
	{
		/* ERROR_MSG("illegal trg element to be removed\n"); */
		exit(0);
	}
	trg_elm->prev->next = trg_elm->next;
	trg_elm->next->prev = trg_elm->prev;
	trg_elm->next = NULL;
	trg_elm->prev = NULL;
	count--;
}

// same as removing the head of the list and return the removed element
LodTRG_ELM*
LodTRG_LT::Pop() 
{
	LodTRG_ELM* tmp_elm;

	if(head==NULL) 
	{
		return NULL;
	}
	if(head->next == head)
	{
		tmp_elm = head;
		head = current = NULL;
	}
	else 
	{
		tmp_elm = head;
		head->prev->next = head->next;
		head->next->prev = head->prev;
		if(current == head) current = head->next;
		head = head->next;
	}
	tmp_elm->next = NULL;
	tmp_elm->prev = NULL;
	count--;
	return tmp_elm;
}

// Insert After the "current"
void
LodTRG_LT::InsertAfter(LodTRG_ELM* elm) 
{
	if(current==NULL)
	{
		head = current = elm;
		head->next = elm;
		head->prev = elm;
	}
	else 
	{
		elm->next = current->next;
		current->next->prev = elm;
		current->next = elm;
		elm->prev = current;
		current = elm;
	}
	count++;
}

// switch the position of the header and current
void
LodTRG_LT::SwapHC() 
{
	LodTRG_ELM* tmp_elm;
	tmp_elm = head;
	head = current;
	current = tmp_elm;
}

// Insert Before the "current"
void
LodTRG_LT::InsertBefore(LodTRG_ELM* elm) 
{
	if(current==NULL)
	{
		head = current = elm;
		head->next = elm;
		head->prev = elm;
	}
	else 
	{
		elm->prev = current->prev;
		current->prev->next = elm;
		current->prev = elm;
		elm->next = current;
		current = elm;
	}
	count++;
}

//set the "current" to be elm
void
LodTRG_LT::SetCurrent(LodTRG_ELM* elm)
{
	current = elm;
}


//disattach the triangle from the vertices and put it on the freelist
void
LodTRG_LT::Dissolve(LodTRG_LT* trg_lt, LodTRG_LT* freelist)
{
	LodTRG_ELM* tmp_elm;	
	while(count>0)
	{
		tmp_elm =head->trg_ptr->GetTrgInList();
		trg_lt->Remove(tmp_elm);
		head->trg_ptr->Dissolve(freelist);
		delete (tmp_elm);
	}	
}
	
//de-allocation
void
LodTRG_LT::Free()
{
	LodTRG_ELM *tmp_elm;
	
	if(head==NULL) return;
	for(current=head->next; current!=head; )	       
	{
		tmp_elm = current->next;
		current->prev->next = current->next;
		current->next->prev = current->prev;
		delete (current);
		current = tmp_elm;
	}
	delete (head);
	head = current = NULL;
	
}


//The API that output the LodTRG_LT into an array of LodTRIANGLES pointed by 
// *trgs------------------------------------------------------
void
LodTRG_LT::OutPut(LodTRIANGLE* trgs)
{
	int num = 0;
	int i, v[3];
	current = head;
	if(head == NULL) return; //do nothing for a empty list;

	do
	{
		for(i=0; i<3; i++) 
		{
			v[i] = current->trg_ptr->GetVertPtr(i)->GetID();
		}

		trgs[num].Set(num, v[0], v[1], v[2]);
		num++;
		current = current->next;
	}
	while(current!=head);
}

/*--------------------------------LodVTX_ELM--------------------------------*/
//class LodVTX_ELM is the element of the LodVTX_LT

//get the dihedral angle of the vertex
const float 
LodVTX_ELM::GetAngle() const 
{ 
	return (vtx_ptr->GetAngle()); 
}

//get the vertex's shape type
const int 
LodVTX_ELM::GetShapeType() const 
{ 
	return (vtx_ptr->GetShapeType()); 
}

//get the vertex's id
const int 
LodVTX_ELM::GetID() const 
{
	return vtx_ptr->GetID(); 
}

//get the position of the vertex
const  LodVECTOR&
LodVTX_ELM::GetLoc()  const
{
	return vtx_ptr->GetLoc();
}

//set the dihedral angle of the vertex
void
LodVTX_ELM::SetAngle(float tmpang)
{
	vtx_ptr->SetAngle(tmpang);
}

//set the shape type
void
LodVTX_ELM::SetShapeType(int typ)
{
	vtx_ptr->SetShapeType(typ);
}


/*--------------------------------LodVTX_LT--------------------------------*/
//class LodVTX_LT provides a circular double linked list structure
//that stores LodVERTEX, it also provides the interface to get the information
//of the LodVERTEX that it is pointing to

//add vert to the list
LodVTX_ELM*
LodVTX_LT::Add(LodVERTEX* vtx) 
{
	LodVTX_ELM* new_elm = (new LodVTX_ELM(vtx));
	if(head==NULL)		
	{
		head = current = new_elm;
		new_elm->prev = new_elm->next = new_elm;		
	}
	else 
	{
		new_elm->next = current->next;
		current->next->prev = new_elm;
		current->next = new_elm;
		new_elm->prev = current;
		current = new_elm;
	}		
	count++;
	return current;
}

//add vtx to list and also point vtx to the elm
LodVTX_ELM*
LodVTX_LT::AddandPoint(LodVERTEX* vtx) 
{
	LodVTX_ELM* new_elm = (new LodVTX_ELM(vtx));
	vtx->SetElm(new_elm);

	if(head==NULL)		
	{
		head = current = new_elm;
		new_elm->prev = new_elm->next = new_elm;		
	}
	else 
	{
		new_elm->next = current->next;
		current->next->prev = new_elm;
		current->next = new_elm;
		new_elm->prev = current;
		current = new_elm;
	}		
	count++;
	return current;
}

//remove from list
void 
LodVTX_LT::Remove(LodVTX_ELM* vtx_elm)
{
	if(head==vtx_elm)
	{
		if (head->next == head) 
		{
			head = current = NULL;
		}
		else 
		{
			head = head->next;
		}
	}
	if(current == vtx_elm) 
	{
		current = vtx_elm->prev;
	}
	if(vtx_elm->prev == NULL || vtx_elm->next==NULL) 
	{
		/* ERROR_MSG("illegal vtx element to be removed\n"); */
		exit(0);
	}
	vtx_elm->prev->next = vtx_elm->next;
	vtx_elm->next->prev = vtx_elm->prev;
	vtx_elm->prev = NULL;
	vtx_elm->next = NULL;
	count--;
}



//insert the vtx_elm in the proper place in the list to keep the list
//sorted
void  
LodVTX_LT::Relocate(LodVTX_ELM* elm)
{
	float ang;

		Remove(elm);
	current = head;
	if(!elm->GetShapeType()) 
	{
		//tmp_elm1 = elm->next;
		
		InsertBefore(elm);
		head = elm;
		return;
	}
	ang = elm->GetAngle();
	if(current->GetShapeType() && current->GetAngle()<ang)  //elm has big angle
	{
		//Remove(elm);
		InsertBefore(elm);
		head = elm;	 
		return;
	}
	if(current->prev->GetShapeType() && current->prev->GetAngle()>ang)  //elm has small angle
	{
		//Remove(elm);
		//current = current->prev;
		InsertBefore(elm);
		//head = elm;	 
		return;
	}

	while(current->prev->GetAngle()<ang && current->prev->GetShapeType())
	{
		current = current->prev;
	}
	InsertBefore(elm);
	
	return;		
}

//insert before the "current" element
void
LodVTX_LT::InsertBefore(LodVTX_ELM* elm) 
{
	if(current==NULL)
	{
		head = current = elm;
		head->next = elm;
		head->prev = elm;
	}
	else 
	{
		elm->prev = current->prev;
		current->prev->next = elm;
		current->prev = elm;
		elm->next = current;
		current = elm;
	}
	count++;
}

//sort the list by dihedral angle
void
LodVTX_LT::Sort()
{
	LodVTX_ELM *tmp_elm, *tmp_elm1;

	if(count<=1) return;


	current = head;
	while(!head->GetShapeType())
	{
		head = head->next;
		if(head == current) return;
	}
	do
	{
		tmp_elm = head->next;
		while(tmp_elm!=current)
	 	{
			if(head->GetAngle() < tmp_elm->GetAngle() || !tmp_elm->GetShapeType())
			{
				tmp_elm1 = tmp_elm;
				tmp_elm = tmp_elm->next;

				tmp_elm1->next->prev = tmp_elm1->prev; // delete
				tmp_elm1->prev->next = tmp_elm1->next;
				
				tmp_elm1->prev = head->prev; // insert
				tmp_elm1->next = head;
				head->prev->next = tmp_elm1;
				head->prev = tmp_elm1;

				if(current==head) current = tmp_elm1;
				head = tmp_elm1;
				if(!head->GetShapeType()) break;
			}
			else 
			{
				tmp_elm = tmp_elm->next;
			}
		}
		head = head->next;
	}
	while(head!=current);

}

//Start the optimization use the LodVTX_LT and the list of triangles
//put removed triangle on to freelist for later use
//remove the most flatten vertices until certain percentage is reached
//if it returns 0, it means that no further optimization
//can be taken, and the percentage is not reached
int
LodVTX_LT::Optimize( LodTRG_LT* trg_lt, LodTRG_LT* freelist, float percent)
{
	LodVTX_ELM *tmp_elm;
	float total = (float)count;

	tmp_elm = head->prev;

	
	while(tmp_elm!=head && count / total > percent)
	{
		if(!tmp_elm->GetShapeType()/* || tmp_elm->vtx_ptr->GetDegree()<3*/) {
			return 0; //can't be optimized
			}

		if(tmp_elm->vtx_ptr->GetDegree()>2) 
		{
			Remove(tmp_elm);
 			tmp_elm->vtx_ptr->Flatten(trg_lt, freelist, this);
		
			delete (tmp_elm);
		tmp_elm = head->prev;
		} else {
			tmp_elm = tmp_elm->prev;
		}
	}
      
	return 1;
}


//Triangulate the list of vertices, assuming that the vertex list forms
//a loop
void
LodVTX_LT::Triangulate(LodTRG_LT* trg_lt, LodTRG_LT* freelist, LodVTX_LT* vlist)
{
	LodVTX_ELM *tmp_elm, *max_elm;
	LodVECTOR vect1, vect2;

	tmp_elm = head;
	do
	{			// set the angle of  each vertex
		vect1 = !(tmp_elm->GetLoc() - tmp_elm->prev->GetLoc());
		vect2 = !(tmp_elm->next->GetLoc() - tmp_elm->GetLoc());
		tmp_elm->SetAngle(vect1.Angle(vect2));
		tmp_elm = tmp_elm->prev;
	}
	while(tmp_elm!=head);

	while(count>3)
	{
		max_elm = head;
		tmp_elm = head->next;
		
		while(tmp_elm!=head) // find the maximum corner
		{		
			if(tmp_elm->GetAngle() > max_elm->GetAngle())
			{
				max_elm = tmp_elm;
			}
			tmp_elm = tmp_elm->next;
		}

		if(trg_lt->AddOld(max_elm->next->vtx_ptr, max_elm->vtx_ptr, max_elm->prev->vtx_ptr, freelist)) 
		{


			max_elm->vtx_ptr->SetTrgAdjacency();
			max_elm->vtx_ptr->ComputeAngle();
			vlist->Relocate(max_elm->vtx_ptr->GetElm());  //keep the list sorted 
		}
		
 		tmp_elm = max_elm->prev;
		vect1 = !(tmp_elm->GetLoc() - tmp_elm->prev->GetLoc());
		vect2 = !(max_elm->next->GetLoc() - tmp_elm->GetLoc());
		tmp_elm->SetAngle(vect1.Angle(vect2));
		
		tmp_elm = max_elm->next;
		vect1 = !(tmp_elm->GetLoc() - max_elm->prev->GetLoc());
		vect2 = !(tmp_elm->next->GetLoc() - tmp_elm->GetLoc());

		tmp_elm->SetAngle(vect1.Angle(vect2));		
		Remove(max_elm);

		delete (max_elm);
	}
	if(count==3) {
		if(trg_lt->AddOld(head->next->vtx_ptr, head->vtx_ptr, head->prev->vtx_ptr, freelist))
		{
			head->next->vtx_ptr->SetTrgAdjacency();
			head->next->vtx_ptr->ComputeAngle();
			vlist->Relocate(head->next->vtx_ptr->GetElm());  //keep the list sorted 
			head->vtx_ptr->SetTrgAdjacency();
			head->vtx_ptr->ComputeAngle();
			vlist->Relocate(head->vtx_ptr->GetElm());  //keep the list sorted 
			head->prev->vtx_ptr->SetTrgAdjacency();
			head->prev->vtx_ptr->ComputeAngle();
			vlist->Relocate(head->prev->vtx_ptr->GetElm());  //keep the list sorted 
		}
	}
	else
	{
		exit(0);
	}
	Free();

}
	
//de-allocating
void
LodVTX_LT::Free()
{
	LodVTX_ELM *tmp_elm;
	if(head==NULL) return;

	for(current=head->next; current!=head; )	       
	{
		tmp_elm = current->next;
		current->prev->next = current->next;
		current->next->prev = current->prev;
		delete (current);
		current = tmp_elm;
	}
	delete (head);
	head = current = NULL;
	
}

//Output the vertex into an array of vertices pointed by verts
void
LodVTX_LT::OutPut(LodVERTEX* verts)
{
	int num = 0;
	current = head;
	
	do
	{
		verts[num].Set(num, (class LodVECTOR)current->vtx_ptr->GetLoc());
		current->vtx_ptr->SetID(num++);
		current = current->next;

	}
	while(current!=head);
	if(num!=count) LodError("output");
}

/*--------------------------------LodVERTEX--------------------------------*/

//class LodVERTEX stores the information of the vertices that constructing the
//mesh
//It stores the position of the vertices, it can also store the normal
//and other information, such as color, smoothing group and texture coordinates

//initialization
void 
LodVERTEX::Set(int index, LodVECTOR& v)  // init the control point
{
  id = index;
  loc = v;
}

//initialization
void 
LodVERTEX::Set(int index,  float v1, float v2, float v3) // init the vertex
{
  id = index;
  loc.Set(v1, v2, v3);
}

//add a triangle to this vertex
LodTRG_ELM*
LodVERTEX::AddTrg(LodTRIANGLE* trgptr, int i) 
{
	LodTRG_ELM *tmp_elm;
	degree++;    
	tmp_elm = trgs_lt.Add(trgptr);
	tmp_elm->SetIndex(i);  
	return tmp_elm;
}

//remove a trg elm from the vertex's trgs_lt
void 
LodVERTEX::RemoveTrg(LodTRG_ELM* trg_in_vtx)
{
	degree--;
	if(degree<0) LodError("removetrg");
	trgs_lt.Remove(trg_in_vtx);
}

//Compute the dihedral angle			
void
LodVERTEX::ComputeAngle()
{
	LodTRG_ELM *elm1, *elm2;
	LodVECTOR norm1, norm2;
	float tmpf;
	angle = 0.0f;
	
	switch(shape_type)
	{
	case 0:
		break;		// non-removable vertex
	case 1:			// normal vertex
		elm1 = trgs_lt.GetHead();
		elm2 = elm1;
		norm1 = elm1->GetTrgNorm();

		do
		{
			norm2 = norm1;
			elm2 = elm2->prev;
			norm1 = elm2->GetTrgNorm();
			//here we exaggerate the sharp corners a little bit
			tmpf = norm1.Angle(norm2); //*100.0;
			angle += tmpf*tmpf;
		}
		while(elm2!=elm1);
		break;
	case 2:			// boundary case
		elm1 = trgs_lt.GetHead();
		norm1 = elm1->GetPredVect() - loc;
		elm2 = elm1->next;
		norm2 = loc - elm2->GetSuccVect();

		norm1.Normalize(); // angle of the boundary edge
		norm2.Normalize();
		//here we exaggerate the sharp corners a little bit
		tmpf = norm1.Angle(norm2); //*100.0;
		angle += tmpf*tmpf;

		elm1 = trgs_lt.GetHead();
		elm2 = elm1->prev;
		norm2 = elm1->GetTrgNorm();

		while(elm2!=elm1)
		{
			norm1 = elm2->GetTrgNorm();
			//here we exaggerate the sharp corners a little bit
			tmpf = norm1.Angle(norm2); //*100.0;
			angle += tmpf*tmpf;
				
			norm2 = norm1;
			elm2 = elm2->prev;
		}

		break;
	}
}

// arrange adjacency of the trgs
void 
LodVERTEX::SetTrgAdjacency() 
{		
	// arrange adjacent trg in order
	LodTRG_ELM *head, *tmp_elm, *current;
	
	int num;
	int next_index, prev_index, matched;

	if(trgs_lt.GetCount()!=degree) LodError("vv");
	if(trgs_lt.GetCount()==1) 
	{
		SetShapeType(2); // boundary case
		return;
	}

	head =  trgs_lt.GetHead();

	if(head==NULL) 
	{
		/* ERROR_MSG("Error occured in seting triangle adjacency info\n"); */
		return;//exit(0);
	}
       
	//now we need to treat the collaped edge case
	prev_index = head->GetPredID();
	current = head->next;
	while(current!=head)
	{
		tmp_elm = current;
		while(tmp_elm!=head)
		{
			if(prev_index==tmp_elm->GetPredID()) // found an edge being shared by two trgs
			{
				SetShapeType(0);
				return;
			}
			tmp_elm = tmp_elm->next;
		}
		prev_index = current->GetPredID();
		current = current->next;
	}

	prev_index = head->GetPredID();
	tmp_elm = head->next;
	trgs_lt.SetCurrent(head);
	matched = 1;
	num = 1;

	do  
	{
		//first we look for the first trg in the list
		//if the vertex is not a boundary vertex, any trg will do
		//if it is, then we need to discover the boundary one to start
		next_index = tmp_elm->GetSuccID();
		if(next_index == prev_index) 
		{


			prev_index = tmp_elm->GetPredID();
			num++;
			
			if(matched)
			{
				trgs_lt.SetCurrent(tmp_elm);
				tmp_elm = tmp_elm->next;
				matched = 1;
				continue;
			}
			else 
			{
				trgs_lt.Remove(tmp_elm);
				trgs_lt.InsertAfter(tmp_elm);
				tmp_elm = tmp_elm->next;
				matched = 1;



				continue;
			}
		}
		else 
		{
			tmp_elm = tmp_elm->next;
			matched = 0;
			continue;
		}

	}
	while(tmp_elm!=head);
	
	if(matched) 
	{ 
		if(num!=trgs_lt.GetCount()) 
		{
			SetShapeType(0);
			return;
		}

		next_index = tmp_elm->GetSuccID();
		if(next_index == prev_index)
		{
			SetShapeType(1); // normal vertex
		}
		else
		{
			SetShapeType(2); // boundary case
			trgs_lt.SwapHC();
		}
	}
	else
	{
		next_index = head->GetSuccID();

		trgs_lt.SwapHC();

		head =  trgs_lt.GetHead();
		tmp_elm = trgs_lt.GetCurrent()->prev;

		
		matched = 1;
		do
		{
			prev_index = tmp_elm->GetPredID();
			if(next_index == prev_index) 
			{
				num++;
				next_index = tmp_elm->GetSuccID();
				
				if(matched)
				{
					trgs_lt.SetCurrent(tmp_elm);
					tmp_elm = tmp_elm->prev;
					matched = 1;
					continue;
				}
				else 
				{
					trgs_lt.Remove(tmp_elm);
					trgs_lt.InsertBefore(tmp_elm);
					tmp_elm = tmp_elm->prev;
					matched = 1;
					continue;
				}
			}
			else 
			{
				tmp_elm = tmp_elm->prev;
				matched = 0;
				continue;
			}

		}
		while(tmp_elm!=head);

		if(matched)
		{
			if(num!=trgs_lt.GetCount()) 
			{
				SetShapeType(0);
			}
			else
				SetShapeType(2); // boundary case
		}
		else 
		{
			SetShapeType(0); // do not remove;
		}		
	}
		
}


//Get rid off this vertex and re-trangulate the surrounding vertices
void
LodVERTEX::Flatten(LodTRG_LT* trg_lt, LodTRG_LT* freelist, LodVTX_LT* vlist)		// create a loop of verts and triangulate
{
	LodVTX_LT loop;
	LodTRG_ELM *tmp_trg_elm, *trg_head;

	trg_head = trgs_lt.GetHead();
	tmp_trg_elm = trg_head;
	
	do
	{
		loop.Add(tmp_trg_elm->GetPredPtr());
		tmp_trg_elm = tmp_trg_elm->prev;
	}
	while(tmp_trg_elm!= trg_head);

	if(shape_type == 2)	// boundary case
	{
		loop.Add(trg_head->next->GetSuccPtr());
	}
		
	
	trgs_lt.Dissolve(trg_lt, freelist);
	
	if(loop.GetCount()>2) loop.Triangulate(trg_lt, freelist, vlist);
	
	loop.Free();
		
}

/*---------------------------------LodTRIANGLE-------------------------------*/

// init the triangle
void 
LodTRIANGLE::Set(int index, int a, int b, int c)  // init the triangle
{
  id = index;
  verts_id[0] = a;
  verts_id[1] = b;
  verts_id[2] = c;
}

// init the triangle
void 
LodTRIANGLE::Set( int a, int b, int c)  // init the triangle
{
  verts_id[0] = a;
  verts_id[1] = b;
  verts_id[2] = c;
}

// computer the norm, and ang
void				 
LodTRIANGLE::ComputeNormal() 
{ 
  LodVECTOR v0, v1;

  v0 = (verts[1]->GetLoc()-verts[0]->GetLoc());
  v1 = (verts[2]->GetLoc()-verts[1]->GetLoc());

  norm = !(v0 % v1);
}

//Add the vertices to the triangle and
//let the vertices point to this trg
void
LodTRIANGLE::EnterVtx(LodVERTEX *lodvtx)
{
	int i;
	for(i= 0; i< 3; i++) 
	{
		verts[i] = &lodvtx[GetVertID(i)];
		trg_in_vtx[i] = verts[i]->AddTrg(this, i); 
	}
}

//let the vertices point to this trg
//given the trg has already got the verts[i]
void
LodTRIANGLE::EnterVtx()
{
	int i;
	for(i= 0; i< 3; i++) 
	{
		trg_in_vtx[i] = verts[i]->AddTrg(this, i); 
	}
}

// remove itself from vertex trgs_lt and enter freelist
void				
LodTRIANGLE::Dissolve(LodTRG_LT* freelist)
{
	for(int i=0; i<3; i++)
	{
		verts[i]->RemoveTrg(trg_in_vtx[i]);
		delete (trg_in_vtx[i]);
		trg_in_vtx[i] = NULL;
	}
	freelist->Add(this);
}

/*---------------------------------LOD stuff-------------------------------*/



//the basic API------------------------------------------------------------
//It takes an array of vertices and triangles, the number of vertices and triangles
//and the percentage of the old vertices that need to be kept
//it generates newvn many vertices and newfn many faces, and the result vertices and
//and triangles are put in newvtx and newtrg
//if it returns 0, it means that no further optimization
//can be taken, and the percentage is not reached
int
LodGenerate(LodVERTEX* lodvtx, LodTRIANGLE* lodtrg, int nverts, int ntrgs, 
	LodVERTEX** newvtx, LodTRIANGLE** newtrg, int* newvn, int* newfn, float percent)
{
	int i;
	int ret;
	
	LodVTX_LT vtx_lt;
	LodTRG_LT trg_lt;
	LodTRG_LT freelist;
	LodTRG_ELM *tmp_elm;

	if(percent > 1.0f || percent < 0.0f) percent = 0.5f; //	defualt

	for(i = 0; i< ntrgs; i++) 
	{
		lodtrg[i].EnterVtx(lodvtx); // set the vertices point to this trg
		lodtrg[i].ComputeNormal();
		tmp_elm = trg_lt.Add(&lodtrg[i]);
		lodtrg[i].SetTrgInList(tmp_elm);
	}
	for(i = 0; i< nverts; i++ ) 
	{
		vtx_lt.AddandPoint(&lodvtx[i]);
 		lodvtx[i].SetTrgAdjacency(); // arrange the adjacent trgs in order
		lodvtx[i].ComputeAngle();
	}

	vtx_lt.Sort();

	ret = vtx_lt.Optimize( &trg_lt, &freelist, percent);

	*newvn = vtx_lt.GetCount();
	*newfn = trg_lt.GetCount();
	
	*newvtx = new LodVERTEX[*newvn];
	*newtrg = new LodTRIANGLE[*newfn];

	vtx_lt.OutPut(*newvtx);
	trg_lt.OutPut(*newtrg);

	vtx_lt.Free();
	trg_lt.Free();
	return ret;
}