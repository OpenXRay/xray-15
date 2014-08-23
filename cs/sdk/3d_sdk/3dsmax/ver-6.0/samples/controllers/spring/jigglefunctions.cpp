/**********************************************************************
 *<
	FILE: jiggleFunctions.cpp

	DESCRIPTION:	A procedural Mass/Spring Position controller 
					Additional functions for the Spring controller

	CREATED BY:		Adam Felt

	HISTORY: 

 *>	Copyright (c) 1999-2000, All Rights Reserved.
 **********************************************************************/

#include "jiggle.h"


void Jiggle::SetTension(int index, float tension, int absolute, bool update)
{
	if (index < 0 || index >= dyn_pb->Count(jig_control_node))
	{
		MAXException(GetString(IDS_OUT_OF_RANGE));
		return;
	}
	
	for (int b=0;b<partsys->GetParticle(0)->GetSprings()->Count();b++)
	{
		if (index == partsys->GetParticle(0)->GetSpring(b)->GetPointConstraint()->GetIndex())
		{
			if (absolute == 0) 
				tension += partsys->GetParticle(0)->GetSpring(b)->GetTension();
			partsys->GetParticle(0)->GetSpring(b)->SetTension(tension);
		}
	}
	partsys->Invalidate();
	if (update)
	{
		UpdateNodeList();
		NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		if(ip) ip->RedrawViews(ip->GetTime());
	}
}

void Jiggle::SetMass(float mass, bool update)
{
	partsys->GetParticle(0)->SetMass(mass);
	partsys->Invalidate();
	if (update)
	{
		NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		if(ip) ip->RedrawViews(ip->GetTime());
	}
}

void Jiggle::SetDampening(int index, float dampening, int absolute, bool update)
{
	if (index < 0 || index >= dyn_pb->Count(jig_control_node))
	{
		MAXException(GetString(IDS_OUT_OF_RANGE));
		return;
	}

	for (int b=0;b<partsys->GetParticle(0)->GetSprings()->Count();b++)
	{
		if (index == partsys->GetParticle(0)->GetSpring(b)->GetPointConstraint()->GetIndex())
		{
			if (absolute == 0) 
				dampening += partsys->GetParticle(0)->GetSpring(b)->GetDampening();
			partsys->GetParticle(0)->GetSpring(b)->SetDampening(dampening);
		}
	}
	partsys->Invalidate();
	if (update)
	{
		UpdateNodeList();
		NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		if(ip) ip->RedrawViews(ip->GetTime());
	}
}

void Jiggle::SetDrag(float drag, bool update)
{
	partsys->GetParticle(0)->SetDrag(drag);
	partsys->Invalidate();
	if (update)
	{
		NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		if(ip) ip->RedrawViews(ip->GetTime());

	}
}

float Jiggle::GetMass()
{
	return partsys->GetParticle(0)->GetMass();
}

float Jiggle::GetDrag()
{
	return partsys->GetParticle(0)->GetDrag();
}

float Jiggle::GetTension(int index)
{
	if (index >=0 && index < dyn_pb->Count(jig_control_node))
		return partsys->GetParticle(0)->GetSpring(index)->GetTension();
	MAXException(GetString(IDS_OUT_OF_RANGE));
	return 0;
}

float Jiggle::GetDampening(int index)
{
	if (index >=0 && index < dyn_pb->Count(jig_control_node))
		return partsys->GetParticle(0)->GetSpring(index)->GetDampening();
	MAXException(GetString(IDS_OUT_OF_RANGE));
	return 0;
}

BOOL Jiggle::AddSpring(INode *node)
{
	if (node) 
	{
		if (!theHold.Holding()) 
			theHold.Begin();
		HoldAll();

		int start;
		force_pb->GetValue(jig_start, 0, start, FOREVER);

		int ct = dyn_pb->Count(jig_control_node);
		
		Point3 initPos = GetCurrentTM(start).GetTrans();
		//create a new bone for the spring
		SSConstraintPoint * bone = new SSConstraintPoint(); //spring->GetBone();
		bone->SetIndex(ct);
		bone->SetPos( initPos ); //node->GetNodeTM(start).GetTrans() );
		bone->SetVel( Point3(0,0,0) );
		
		//the particles world space pos - the bones world space pos
		Point3 length = initPos * Inverse(node->GetNodeTM(start));
		
		//add the spring to the particle
		partsys->GetParticle(0)->AddSpring(bone, length, JIGGLE_DEFAULT_TENSION, JIGGLE_DEFAULT_DAMPENING);
		dyn_pb->Append(jig_control_node, 1, &node);

		theHold.Accept(GetString(IDS_UNDO_ADD_SPRING));

		partsys->Invalidate();
		validStart = false;
		UpdateNodeList();
		if (dlg) SendDlgItemMessage(dlg->dynDlg, IDC_LIST2, LB_SETCURSEL, ct, 0);
		if (hParams1) SendDlgItemMessage(hParams1, IDC_LIST2, LB_SETCURSEL, ct, 0);


		NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
		

		return TRUE;
	}
	return FALSE;
}

int Jiggle::GetSpringCount( )
{
	return dyn_pb->Count(jig_control_node);
}

void Jiggle::RemoveSpring(int sel)
{
	
 
	if (sel == 0) 
	{
		MAXException("Can't remove the first spring");
		return;
	}else if (sel >= GetSpringCount() || sel < 0 )
		MAXException(GetString(IDS_OUT_OF_RANGE));

	Interface* my_ip = GetCOREInterface();
	
	if (theHold.Holding()) HoldAll();

	partsys->GetParticle(0)->DeleteSpring(sel);
	dyn_pb->Delete(jig_control_node, sel, 1);
	
	partsys->Invalidate();
	validStart = false;
	UpdateNodeList();
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	my_ip->RedrawViews(my_ip->GetTime());
}

void Jiggle::RemoveSpring(INode* node)
{
	if (!node) return;
	Interface* my_ip = GetCOREInterface();
	
	if (!theHold.Holding())
		theHold.Begin();
	HoldAll();
	int exists = 0;

	for (int i = 1; i< dyn_pb->Count(jig_control_node); i++)
	{
		INode* ctrlNode = NULL;
		dyn_pb->GetValue(jig_control_node, 0, ctrlNode, FOREVER, i);
		if ( ctrlNode == node)
		{
			exists = 1;
			break;
		}
	}
	if (!exists) { theHold.Cancel(); return; }

	partsys->GetParticle(0)->DeleteSpring(i);
	dyn_pb->Delete(jig_control_node, i, 1);
	
	theHold.Accept(GetString(IDS_UNDO_REMOVE_SPRING));

	partsys->Invalidate();
	validStart = false;
	UpdateNodeList();
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	my_ip->RedrawViews(my_ip->GetTime());
}



Matrix3 Jiggle::GetCurrentTM(TimeValue t)
{
	INode* baseNode = NULL;
	baseNode = selfNode;
	//if (dyn_pb->Count(jig_control_node)) 
	//	dyn_pb->GetValue(jig_control_node, 0, baseNode, FOREVER, 0);

	Matrix3 selfTM;
	Point3 pos = Point3(0.0f,0.0f,0.0f);
	posCtrl->GetValue(t, pos, FOREVER, CTRL_ABSOLUTE);
	if (baseNode != NULL )
	{
		selfTM = baseNode->GetNodeTM(t);
		selfTM.SetTrans(selfTM * pos);
	}
	else 
	{	
		selfTM.IdentityMatrix();
		selfTM.SetTrans(pos);
	}
	return selfTM;
}

//From SpringSysClient
//*************************************************
Tab<Matrix3> Jiggle::GetForceMatrices(TimeValue t)
{
	Tab<Matrix3> tms;
	INode* node;
	Matrix3 parentTM;
	
	parentTM = GetCurrentTM(t);

	tms.Append(1, &parentTM);
	
	for (int x=1;x<dyn_pb->Count(jig_control_node); x++)
	{
		dyn_pb->GetValue(jig_control_node, 0, node, FOREVER, x);
		if (node) tms.Append(1, &(node->GetNodeTM(t)));
	}
	return tms;
}


Point3 Jiggle::GetDynamicsForces(TimeValue t, Point3 pos, Point3 vel)
{
	INode *aforce;
	Point3 f(0.0f,0.0f,0.0f);

	for( int i=0;i<force_pb->Count(jig_force_node);i++)
	{
		force_pb->GetValue(jig_force_node, 0, aforce, FOREVER, i);
		
		if (aforce)
		{
			WSMObject *obref=(WSMObject*)aforce->GetObjectRef();
			ForceField* ff = obref->GetForceField(aforce);
			
			if (ff)
			{
				f += ff->Force(t, pos, vel, 0);
				ff->DeleteThis();
			}
		}
	}
	return f;
}

