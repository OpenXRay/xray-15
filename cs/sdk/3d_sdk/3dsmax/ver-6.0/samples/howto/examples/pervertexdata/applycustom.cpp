/*===========================================================================*\
 | 
 |  FILE:	ApplyCustom.cpp
 |			Project to demonstrate custom data per vertex
 |			Simply allows user to define a custom value and bind it to a vertex
 |			3D Studio MAX R3.0
 | 
 |  AUTH:   Harry Denholm
 |			Developer Consulting Group
 |			Copyright(c) Discreet 1999
 |
 |  HIST:	Started 6-4-99
 | 
\*===========================================================================*/

#include "CustomVData.h"



/*===========================================================================*\
 |	ModifyObject will do all the work in a full modifier
\*===========================================================================*/

#define MY_CHANNEL		5


void CVDModifier::ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node)
{
	if (!os->obj->IsSubClassOf(triObjectClassID)) return;

	// Get a mesh from input object
	TriObject *tobj = (TriObject*)os->obj;
	Mesh* mesh = &tobj->GetMesh();
	int numVert = mesh->getNumVerts();

	// Get parameters from pblock
	float sparam = 0.0f; 
	Interval valid = FOREVER;
	pblock->GetValue(cvd_codev, t, sparam, valid);

		// Take over the channel, realloc with size == number of verts
		mesh->setVDataSupport(MY_CHANNEL,TRUE);

		// Get a pointer back to the floating point array
		float *vdata = mesh->vertexFloat(MY_CHANNEL);
		if(vdata)
		{
			// loop through all verticies
			// Ask the random number generator for a value bound to the
			//	paramblock value
			// and encode it into the vertex.
			for(int i=0;i<numVert;i++)
			{
				vdata[i] = randomGen.getf(sparam);
			}
		}

}



/*===========================================================================*\
 |	NotifyInputChanged is called each time the input object is changed in some way
 |	We can find out how it was changed by checking partID and message
\*===========================================================================*/

void CVDModifier::NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc)
{
	if( (partID&PART_TOPO) || (partID&PART_GEOM) || (partID&PART_SELECT) )
	{
		NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
	}
}

