

/*****************************************************************

  This is just a header that contains all our vertex attribute and weight
  classes

******************************************************************/

#ifndef __BONESDEF_VERTEXWEIGHTS__H
#define __BONESDEF_VERTEXWEIGHTS__H



//this contains our vertex weight info
//basically it contains a index into the bones list, the weight and bunch of optional spline parameters
class VertexInfluenceListClass
{
public:
	int Bones;						// this is an index into the bones list,which bones belongs to the weight
    float Influences;				// this is the unnormalized weight for the Bones
	float normalizedInfluences;		// this is the normalized weight for Bones

//extra data to hold spline stuff
// this contains info on the closest point on the spline to the vertex
// the data is based on the initial spline position
    int SubCurveIds;				// this which curve the point is closest to NOTE NOT USED YET
    int SubSegIds;					// this is the closest seg
    float u;						// this is the U value along the segment which is the closest point to the vertex
    Point3 Tangents;				// this is the tangent
    Point3 OPoints;					// this is the point in spline space

	VertexInfluenceListClass()
		{
		Bones = 0;          
		Influences = 0.0f;  
		normalizedInfluences = 0.0f;
//extra data to hold spline stuff
		SubCurveIds = 0;
		SubSegIds=0;
		u=0.0f;
		Tangents = Point3(1.0f,0.0f,0.0f);
		OPoints = Point3(0.0f,0.0f,0.0f);
		}
};

// this is the per vertex data class
// it contains all the attributes of the vertex and a list of weights for this vertex
class VertexListClass
{
public:
	DWORD	flags; 

// These are get/sets for our flag properties
// the properties are
// Modified		whether the vertex has been hand weighted
// Unormalized  whether the vertex is normalized
// Rigid		whether the vertex is rigid,if it is rigid only one bone will be affect the vertex
// Rigid Handle only applies to patches, when set if it is a handle it will use the weights of the knot that owns the handle
// TempSelected	used internally to hold temporary selections for cut and paste
	inline BOOL IsSelectedTemp()  
		{
		if (flags & VERTEXFLAG_TEMPSELECTED) return TRUE;
		else return FALSE;
		}
			
	inline BOOL IsModified()
		{
		if (flags & VERTEXFLAG_MODIFIED) return TRUE;
		else return FALSE;
		}
	inline BOOL IsUnNormalized()
		{
		if (flags & VERTEXFLAG_UNNORMALIZED) return TRUE;
		else return FALSE;
		}

	inline BOOL IsRigid()
		{
		if (flags & VERTEXFLAG_RIGID) return TRUE;
		else return FALSE;
		}
	
	inline BOOL IsRigidHandle()
		{
		if (flags & VERTEXFLAG_RIGIDHANDLE) return TRUE;
		else return FALSE;
		}



	inline void SelectedTemp(BOOL sel)
		{
		if (sel)
			flags |= VERTEXFLAG_TEMPSELECTED;
		else flags &= ~VERTEXFLAG_TEMPSELECTED;
		}

	inline void Modified(BOOL modify)
		{
		if (modify)
			{
			if (!IsModified())
				NormalizeWeights();
			flags |= VERTEXFLAG_MODIFIED;
			}
		else flags &= ~VERTEXFLAG_MODIFIED;
		}
	
	inline void UnNormalized(BOOL unnormalized)
		{
		if (unnormalized)
			{
			flags |= VERTEXFLAG_UNNORMALIZED;
			if (!IsModified())
				{
				Modified(TRUE);					
				}
			}
		else 
			{
			BOOL wasNormal =  !(flags  | VERTEXFLAG_UNNORMALIZED);
			flags &= ~VERTEXFLAG_UNNORMALIZED;
			if (wasNormal)
				Modified(TRUE);
				
			}
		}
	inline void Rigid(BOOL rigid)
		{
		if (rigid)
			flags |= VERTEXFLAG_RIGID;
		else flags &= ~VERTEXFLAG_RIGID;
		}

	inline void RigidHandle(BOOL rigidHandle)
		{
		if (rigidHandle)
			flags |= VERTEXFLAG_RIGIDHANDLE;
		else flags &= ~VERTEXFLAG_RIGIDHANDLE;
		}


    Point3 LocalPos;				//this is local position of the vertex before any skin deformation
	Point3 LocalPosPostDeform;		//this is local position of the vertex before after skin deformation

	//table of misc data    
    Tab<VertexInfluenceListClass> d; //this is the table of of bones and weights that affect this vertex

	VertexListClass()
		{
		flags = 0; 
		Modified (FALSE);		
		LocalPos = Point3(0.0f,0.0f,0.0f);
		}

	//this returns the bone that most affects this vertex
	int GetMostAffectedBone()
		{
		if (d.Count() == 0) return -1;
		int largestID = d[0].Bones;
		float largestVal = d[0].Influences;
		for (int i = 1; i < (d.Count()); i++)
			{
			for (int j = i; j < d.Count(); j++)
				{
				if (d[j].Influences > largestVal)
					{
					largestVal = d[j].Influences;
					largestID = d[j].Bones;
					}
				}
			}
		return largestID;

		}

	//this returns the ith index of the bone that most affects this vertex
	int GetMostAffectedBoneID()
		{
		if (d.Count() == 0) return -1;
		int largestID = 0;
		float largestVal = d[0].Influences;
		for (int i = 1; i < (d.Count()); i++)
			{
			for (int j = i; j < d.Count(); j++)
				{
				if (d[j].Influences > largestVal)
					{
					largestVal = d[j].Influences;
					largestID = j;
					}
				}
			}
		return largestID;

		}

	//this loops though the unnormalized weights 
	// and stuffs the normalized values in the normalizedInfluences variable
	void NormalizeWeights()
		{
		float sum = 0.0f;
		for (int i = 0; i < d.Count(); i++)
			sum +=  d[i].Influences;

		for ( i = 0; i < d.Count(); i++)
			{
			if (sum == 0.0f)
				d[i].Influences = 0.0f;
			else d[i].Influences = d[i].Influences/sum ;
			d[i].normalizedInfluences = d[i].Influences;
			}

		}	

    };


//this class is used to cache our distances
//every time a bone is selected all the distances are computed from this bone
//and stored in a table of this class
class VertexDistanceClass
{
public:
	float dist;
	float u;
    int SubCurveIds;
    int SubSegIds;
    Point3 Tangents;
    Point3 OPoints;
};




//this is a legacy class to load older files
// THIS SHOULD NOT BE CHANGED
class VertexListClassOld
	{
public:
    BOOL selected;
    BOOL modified;
    Point3 LocalPos;
//table of misc data    
    Tab<VertexInfluenceListClass> d;
	int GetMostAffectedBone()
		{
		if (d.Count() == 0) return -1;
		int largestID = d[0].Bones;
		float largestVal = d[0].Influences;
		for (int i = 1; i < (d.Count()); i++)
			{
			for (int j = i; j < d.Count(); j++)
				{
				if (d[j].Influences > largestVal)
					{
					largestVal = d[j].Influences;
					largestID = d[j].Bones;
					}
				}
			}
		return largestID;

		}
	int GetMostAffectedBoneID()
		{
		if (d.Count() == 0) return -1;
		int largestID = 0;
		float largestVal = d[0].Influences;
		for (int i = 1; i < (d.Count()); i++)
			{
			for (int j = i; j < d.Count(); j++)
				{
				if (d[j].Influences > largestVal)
					{
					largestVal = d[j].Influences;
					largestID = j;
					}
				}
			}
		return largestID;

		}

    };


#endif