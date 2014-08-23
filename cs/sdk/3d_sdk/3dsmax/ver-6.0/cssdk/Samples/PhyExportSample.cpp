/******************************************************************************

	This file contains functions that demonstrate how to use the Physique 3.0 Export Interface
	These functions demonstrate how to export Physique Rigid vertex assignments
	Supports Floating Bones as well as Hierarchial Bones
	
	Written By:	Adam Felt (adam.felt@discreet.com)
	Date:		10/10/2000

******************************************************************************/


// This function demonstrates how to get rigid vertex assignments 
// from an object with a physique modifier applied.  
// Use the FindPhysiqueModifier function documented in phyexp.h to determine if 
// this node has a physique modifier applied, and then pass in the modifier as the Modifier* argument
// Enhance this function with your own unique export code to actually export the physique data gathered here
// *********************************************************************************************************
void ExportPhysiqueData(INode* node, Modifier* mod)
{

	//check to make sure it is physique
	if (mod->ClassID() != Class_ID(PHYSIQUE_CLASS_ID_A, PHYSIQUE_CLASS_ID_B)) { return; }
	
	//**************************************************************************
	//Get the data from the physique interface
	//**************************************************************************
	
	//get a pointer to the export interface
	IPhysiqueExport *phyExport = (IPhysiqueExport *)mod->GetInterface(I_PHYEXPORT);

	//get the physique version number.  
	//If the version number is > 30 you may have floating bones
	int ver = phyExport->Version();

	//get the node's initial transformation matrix and store it in a matrix3
	Matrix3 initTM;
	int msg = phyExport->GetInitNodeTM(node, initTM);

	//get a pointer to the export context interface
	IPhyContextExport *mcExport = (IPhyContextExport *)phyExport->GetContextInterface(node);

	//convert to rigid for time independent vertex assignment
	mcExport->ConvertToRigid(true);

	//allow blending to export multi-link assignments
	mcExport->AllowBlending(true);

	int x = 0;
	float normalizedWeight;
	Point3 offsetVector;

	//these are the different types of rigid vertices supported
	IPhyBlendedRigidVertex *rb_vtx;
	IPhyRigidVertex *r_vtx;
	IPhyFloatingVertex *f_vtx;
	
	//get the vertex count from the export interface
	int numverts = mcExport->GetNumberVertices();

	//Export the list of bones used by Physique
	//See the function below for more details
	ExportPhysiqueBoneList(mcExport);

	//gather the vertex-link assignment data
	for (int i = 0; i<numverts; i++)
	{
		//keep record of the total bone weight in order to normalize the flaoting bone weights later
		float totalWeight = 0.0f, weight = 0.0f;
		TSTR nodeName;
	
		//Get the hierarchial vertex interface for this vertex
		IPhyVertexExport* vi = mcExport->GetVertexInterface(i, HIERARCHIAL_VERTEX);
		if (vi){

			//check the vertex type and process accordingly	
			int type = vi->GetVertexType();
			switch (type)
			{
				//we have a vertex assigned to more than one link
				case RIGID_BLENDED_TYPE: 
					//type-cast the node to the proper class		
					rb_vtx = (IPhyBlendedRigidVertex*)vi;
					
					//iterate through all links this vertex is assigned to
					for (x = 0; x<rb_vtx->GetNumberNodes(); x++)
					{
						//Get the node and its name for export
						nodeName = rb_vtx->GetNode(x)->GetName();
						
						//Get the returned normalized weight for export.  
						//Use the weight reference to keep track of the total weight.
						normalizedWeight = rb_vtx->GetWeight(x, &weight);
						totalWeight += weight;
						
						//Get the offset vector for export
						offsetVector = rb_vtx->GetOffsetVector(x);
					}
					break;
				// we have a vertex assigned to a single link
				case RIGID_TYPE:
					//type-cast the node to the proper class
					r_vtx = (IPhyRigidVertex*)vi;
					
					//get the node and its name for export
					nodeName = r_vtx->GetNode()->GetName();
					
					//the weight is 1.0f since it is not blended
					float NormalizedWeight = 1.0f;
					//add this vertex weight to the running total
					totalWeight += 1.0f;

					//get the offset vector for export
					offsetVector = r_vtx->GetOffsetVector();
					break;

				//Should not make it here since assignments were converted to Rigid.  
				//Should be one of the above two types
				default:
					break;
			}
			//release the vertex interface
			mcExport->ReleaseVertexInterface(vi);
		}

		//an array of structs used to store floating bone information
		//see the struct definition below
		Tab<LinkEntry> linkTable;

		//check for floating bone assignments and gather the data if there are any
		//get the floating bone vertex interface if there is one
		f_vtx = (IPhyFloatingVertex*)mcExport->GetVertexInterface(i, FLOATING_VERTEX);
		if (f_vtx) {	
			//iterate through all floating bones assigned to this vertex
			for (x = 0; x<f_vtx->GetNumberNodes(); x++)
			{
				LinkEntry link;

				//get the node associated with the floating bone
				link.node = f_vtx->GetNode(x); 

				//get the offset vector
				link.offset = f_vtx->GetOffsetVector(x);

				//Get the returned normalized weight 
				//The normalized weight is not used if your blending floating and non-floating bones.  
				link.normalizedWeight = f_vtx->GetWeight(x, weight);

				//Use the weight reference to keep track of the total weight and store it as the actual weight.
				//The actual weight will need to be normalized in a later step
				link.actualWeight = weight;
				totalWeight += weight;
				
				//Append the floating bone link data to the list
				linkTable.Append(1, &link);

			}
			//release the vertex interface
			mcExport->ReleaseVertexInterface(f_vtx);
		}
		
		//Normalize the weights for the floating bone vertices
		//This is the actual weight of the floating link divided by the total weight of all links
		for (x=0;x<linkTable.Count();x++)
		{
			if (totalWeight != 0) 
				linkTable[x].normalizedWeight = linkTable[x].actualWeight/totalWeight;
			//just make sure you don't divide by zero...
			else linkTable[x].normalizedWeight = 0.0f;
		}

		//Export the floating bone vertex information
		for (x=0;x<linkTable.Count();x++)
		{
			//export thye node name, normalized weight, and local offset.
			nodeName = linkTable[x].node->GetName();
			nodeName.data();
			linkTable[x].normalizedWeight;
			linkTable[x].offset;
		}
	}
	
	//release the context interface
	phyExport->ReleaseContextInterface(mcExport);

	//Release the physique interface
	mod->ReleaseInterface(I_PHYINTERFACE, phyExport);
}



// This function can be used to gather the list of bones used by the modifier
// Takes an Export Interface pointer as an argument
//****************************************************************************
void ExportPhysiqueBoneList(IPhyContextExport *mcExport)
{
	int i = 0, x = 0;
	INodeTab bones;
	INode* bone;
	
	//These are the different types of vertex classes 
	IPhyBlendedRigidVertex *rb_vtx;
	IPhyRigidVertex *r_vtx;
	IPhyFloatingVertex *f_vtx;

	//get the vertex count from the export interface
	int numverts = mcExport->GetNumberVertices();
	
	//iterate through all vertices and gather the bone list
	for (i = 0; i<numverts; i++) 
	{
		BOOL exists = false;
		
		//get the hierarchial vertex interface
		IPhyVertexExport* vi = mcExport->GetVertexInterface(i, HIERARCHIAL_VERTEX);
		if (vi) {

			//check the vertex type and process accordingly
			int type = vi->GetVertexType();
			switch (type) 
			{
				//The vertex is rigid, blended vertex.  It's assigned to multiple links
				case RIGID_BLENDED_TYPE:
					//type-cast the node to the proper class		
					rb_vtx = (IPhyBlendedRigidVertex*)vi;
					
					//iterate through the bones assigned to this vertex
					for (x = 0; x<rb_vtx->GetNumberNodes(); x++) 
					{
						exists = false;
						//get the node by index
						bone = rb_vtx->GetNode(x);
						
						//If the bone is a biped bone, scale needs to be
						//restored before exporting skin data
						ScaleBiped(bone, 0);
						
						//check to see if we already have this bone
						for (int z=0;z<bones.Count();z++)
							if (bone == bones[z]) exists = true;

						//if you didn't find a match add it to the list
						if (!exists) bones.Append(1, &bone);
					}
					break;
				//The vertex is a rigid vertex and only assigned to one link
				case RIGID_TYPE:
					//type-cast the node to the proper calss
					r_vtx = (IPhyRigidVertex*)vi;
					
					//get the node
					bone = r_vtx->GetNode();

					//If the bone is a biped bone, scale needs to be
					//restored before exporting skin data
					ScaleBiped(bone, 0);

					//check to see if the bone is already in the list
					for (x = 0;x<bones.Count();x++)
					{
						if (bone == bones[x]) exists = true;
					}
					//if you didn't find a match add it to the list
					if (!exists) bones.Append(1, &bone);
					break;

				// Shouldn't make it here because we converted to rigid earlier.  
				// It should be one of the above two types
				default: break;  
			}
		}
		 
		// After gathering the bones from the rigid vertex interface
		// gather all floating bones if there are any 
		f_vtx = (IPhyFloatingVertex*)mcExport->GetVertexInterface(i, FLOATING_VERTEX);
		if (f_vtx) {	//We have a vertex assigned to a floating bone
			// iterate through the links assigned to this vertex
			for (x = 0; x<f_vtx->GetNumberNodes(); x++)
			{
				exists = false;

				 //get the node by index
				bone = f_vtx->GetNode(x); 
				
				//If the bone is a biped bone, scale needs to be
				//restored before exporting skin data
				ScaleBiped(bone, 0);
				
				//check to see if we already have this bone
				for (int z=0;z<bones.Count();z++)
					if (bone == bones[z]) exists = true;

				//if you didn't find a match add it to the list
				if (!exists) bones.Append(1, &bone);
			}
		}
	}
	//print out the list of bones
	for (i=0;i<bones.Count();i++)
	{
		//get the name of the bone
		TSTR boneName = bone->GetName();
		/* print boneName.data() here */
	}
}


// This is a struct used to store information for a vertex assigned to a floating bone
// ************************************************************************************
struct LinkEntry {
	INode *node;				//a node returned from GetNode(int index)
	float actualWeight;			//storage for the actual weight set in the 
								//weight argument in GetWeight(int index, float &weight);
	float normalizedWeight;		//storage for the normalized weight.  This is what you actually want to export
	Point3 offset;				//The offset distance of the vertex from the bone
};


// This function can be used to set the non-uniform scale of a biped.
// The node argument should be a biped node.
// If the scale argument is non-zero the non-uniform scale will be removed from the biped.
// Remove the non-uniform scale before exporting biped nodes and animation data
// If the scale argument is zero the non-uniform scaling will be reapplied to the biped.
// Add the non-uniform scaling back on the biped before exporting skin data
//***********************************************************************************
void ScaleBiped(INode* node, int scale)
{
	if (node->IsRootNode()) return;

	// Use the class ID to check to see if we have a biped node
	Control* c = node->GetTMController();
    if ((c->ClassID() == BIPSLAVE_CONTROL_CLASS_ID) ||
         (c->ClassID() == BIPBODY_CONTROL_CLASS_ID) ||
         (c->ClassID() == FOOTPRINT_CLASS_ID))
    {

        // Get the Biped Export Interface from the controller 
        IBipedExport *BipIface = (IBipedExport *) c->GetInterface(I_BIPINTERFACE);

        // Either remove the non-uniform scale from the biped, 
		// or add it back in depending on the boolean scale value
        BipIface->RemoveNonUniformScale(scale);
		Control* iMaster = (Control *) c->GetInterface(I_MASTER);
		iMaster->NotifyDependents(FOREVER, PART_TM, REFMSG_CHANGE);
		
		// Release the interfaces
		c->ReleaseInterface(I_MASTER,iMaster);
		c->ReleaseInterface(I_BIPINTERFACE,BipIface);
	}
}

