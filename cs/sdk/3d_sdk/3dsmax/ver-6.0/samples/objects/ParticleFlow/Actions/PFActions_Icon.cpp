/**********************************************************************
 *<
	FILE:			PFActions_Icon.h

	DESCRIPTION:	PF Icon Shape Data (definition)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 07-24-2002

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "PFActions_Icon.h"
#include "iparamb2.h"
#include "macrorec.h"

namespace PFActions {

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From PFActionCreateCallback						 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
void PFActionCreateCallback::init(Object* pfAction, IParamBlock2* pblock, int sizeParamIndex)
{
	m_pfAction = pfAction;
	m_pblock = pblock;
	m_sizeParamIndex = sizeParamIndex;
}

//+--------------------------------------------------------------------------+
//|							From PFActionCreateCallback						 |
//+--------------------------------------------------------------------------+
void SpaceToUnderscore(TSTR& name)
{
	TSTR spaceChar = _T(" ");
	TSTR underscoreChar = _T("_");
	for(int i=0; i<name.length(); i++) {
		if (name[i] == spaceChar[0])
			name[i] = underscoreChar[0];
	}
}

int PFActionCreateCallback::proc(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat)
{
#ifdef _3D_CREATE
	DWORD snapdim = SNAP_IN_3D;
#else
	DWORD snapdim = SNAP_IN_PLANE;
#endif

	if (msg == MOUSE_FREEMOVE) {
#ifdef _OSNAP
		vpt->SnapPreview(m,m,NULL, snapdim);
#endif
		vpt->TrackImplicitGrid(m);
	}
	if (msg==MOUSE_POINT||msg==MOUSE_MOVE) 
	{
		switch(point)  
		{
			case 0:
				// if hidden by category, re-display particles and helpers
				GetCOREInterface()->SetHideByCategoryFlags(
						GetCOREInterface()->GetHideByCategoryFlags() & ~(HIDE_HELPERS|HIDE_PARTICLES));
				macroRec->SetProperty(m_pfAction, _T("isSelected"), mr_bool, TRUE );
				vpt->CommitImplicitGrid(m, flags);
				m_sp0 = m;
				m_p0 = vpt->SnapPoint(m,m,NULL,snapdim);
				mat.SetTrans(m_p0);
				macroRec->Disable();
				m_pblock->SetValue(m_sizeParamIndex,0,0.01f);
				macroRec->Enable();
				break;
	
			case 1: 
			{
				m_sp1 = m;
				m_p1 = vpt->SnapPoint(m,m,NULL,snapdim);
				macroRec->Disable();
				float paramSize = Length(m_p1-m_p0);
				m_pblock->SetValue(m_sizeParamIndex,0,paramSize);
				macroRec->Enable();
				TSTR paramName = m_pblock->GetLocalName(m_sizeParamIndex);
				SpaceToUnderscore(paramName);
				TCHAR propName[256];
				sprintf(propName,"%s",paramName);
				Matrix3 constrTM;
				vpt->GetConstructionTM(constrTM);
				constrTM = constrTM*mat;
				macroRec->SetProperty(m_pfAction, propName, mr_float, paramSize);
				macroRec->SetProperty(m_pfAction, _T("transform"), mr_matrix3, &constrTM );
	
				if (msg==MOUSE_POINT) {
					if (Length(m-m_sp0)<3 || Length(m_p1-m_p0)<0.1f) {						
						macroRec->Cancel();
						return CREATE_ABORT;
					} else { 
						vpt->ReleaseImplicitGrid();
						macroRec->EmitScript();
						return CREATE_STOP;
					}
				}
				break;
			}
		}
	} else {
		if (msg == MOUSE_ABORT) {
			macroRec->Cancel();
			return CREATE_ABORT;
		}
	}
	return 1;
}


//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From PFActionIcon								 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
void PFActionIcon::BuildBox(Mesh *mesh)
{
	mesh->setNumVerts(8);
	mesh->setNumFaces(12);

	mesh->setVert(0, Point3(-0.500000,-0.500000,-0.500000));
	mesh->setVert(1, Point3(0.500000,-0.500000,-0.500000));
	mesh->setVert(2, Point3(-0.500000,0.500000,-0.500000));
	mesh->setVert(3, Point3(0.500000,0.500000,-0.500000));
	mesh->setVert(4, Point3(-0.500000,-0.500000,0.500000));
	mesh->setVert(5, Point3(0.500000,-0.500000,0.500000));
	mesh->setVert(6, Point3(-0.500000,0.500000,0.500000));
	mesh->setVert(7, Point3(0.500000,0.500000,0.500000));
	Face f;

	f.v[0] = 0;  f.v[1] = 2;  f.v[2] = 3;  	f.smGroup = 2;  f.flags = 65603; mesh->faces[0] = f;
	f.v[0] = 3;  f.v[1] = 1;  f.v[2] = 0;  	f.smGroup = 2;  f.flags = 65603; mesh->faces[1] = f;
	f.v[0] = 4;  f.v[1] = 5;  f.v[2] = 7;  	f.smGroup = 4;  f.flags = 67; mesh->faces[2] = f;
	f.v[0] = 7;  f.v[1] = 6;  f.v[2] = 4;  	f.smGroup = 4;  f.flags = 67; mesh->faces[3] = f;
	f.v[0] = 0;  f.v[1] = 1;  f.v[2] = 5;  	f.smGroup = 8;  f.flags = 262211; mesh->faces[4] = f;
	f.v[0] = 5;  f.v[1] = 4;  f.v[2] = 0;  	f.smGroup = 8;  f.flags = 262211; mesh->faces[5] = f;
	f.v[0] = 1;  f.v[1] = 3;  f.v[2] = 7;  	f.smGroup = 16;  f.flags = 196675; mesh->faces[6] = f;
	f.v[0] = 7;  f.v[1] = 5;  f.v[2] = 1;  	f.smGroup = 16;  f.flags = 196675; mesh->faces[7] = f;
	f.v[0] = 3;  f.v[1] = 2;  f.v[2] = 6;  	f.smGroup = 32;  f.flags = 327747; mesh->faces[8] = f;
	f.v[0] = 6;  f.v[1] = 7;  f.v[2] = 3;  	f.smGroup = 32;  f.flags = 327747; mesh->faces[9] = f;
	f.v[0] = 2;  f.v[1] = 0;  f.v[2] = 4;  	f.smGroup = 64;  f.flags = 131139; mesh->faces[10] = f;
	f.v[0] = 4;  f.v[1] = 6;  f.v[2] = 2;  	f.smGroup = 64;  f.flags = 131139; mesh->faces[11] = f;
}

//+--------------------------------------------------------------------------+
//|							From PFActionIcon								 |
//+--------------------------------------------------------------------------+
void PFActionIcon::BuildRectangle(Mesh *mesh)
{
	mesh->setNumVerts(4);
	mesh->setNumFaces(2);

	mesh->setVert(0, Point3(-0.500000,-0.500000,0.000000));
	mesh->setVert(1, Point3(0.500000,-0.500000,0.000000));
	mesh->setVert(2, Point3(-0.500000,0.500000,0.000000));
	mesh->setVert(3, Point3(0.500000,0.500000,0.000000));
	Face f;

	f.v[0] = 0;  f.v[1] = 2;  f.v[2] = 3;  	f.smGroup = 2;  f.flags = 65603; mesh->faces[0] = f;
	f.v[0] = 3;  f.v[1] = 1;  f.v[2] = 0;  	f.smGroup = 2;  f.flags = 65603; mesh->faces[1] = f;
}

//+--------------------------------------------------------------------------+
//|							From PFActionIcon								 |
//+--------------------------------------------------------------------------+
void PFActionIcon::BuildCircle(Mesh *mesh)
{
	mesh->setNumVerts(25);
	mesh->setNumFaces(24);

	mesh->setVert(0, Point3(0.000000,0.000000,0.000000));
	mesh->setVert(1, Point3(0.500000,0.000000,0.000000));
	mesh->setVert(2, Point3(0.482963,0.129410,0.000000));
	mesh->setVert(3, Point3(0.433013,0.250000,0.000000));
	mesh->setVert(4, Point3(0.353553,0.353553,0.000000));
	mesh->setVert(5, Point3(0.250000,0.433013,0.000000));
	mesh->setVert(6, Point3(0.129410,0.482963,0.000000));
	mesh->setVert(7, Point3(-0.000000,0.500000,0.000000));
	mesh->setVert(8, Point3(-0.129410,0.482963,0.000000));
	mesh->setVert(9, Point3(-0.250000,0.433013,0.000000));
	mesh->setVert(10, Point3(-0.353553,0.353553,0.000000));
	mesh->setVert(11, Point3(-0.433013,0.250000,0.000000));
	mesh->setVert(12, Point3(-0.482963,0.129409,0.000000));
	mesh->setVert(13, Point3(-0.500000,-0.000000,0.000000));
	mesh->setVert(14, Point3(-0.482963,-0.129410,0.000000));
	mesh->setVert(15, Point3(-0.433013,-0.250000,0.000000));
	mesh->setVert(16, Point3(-0.353553,-0.353553,0.000000));
	mesh->setVert(17, Point3(-0.250000,-0.433013,0.000000));
	mesh->setVert(18, Point3(-0.129409,-0.482963,0.000000));
	mesh->setVert(19, Point3(0.000000,-0.500000,0.000000));
	mesh->setVert(20, Point3(0.129410,-0.482963,0.000000));
	mesh->setVert(21, Point3(0.250000,-0.433013,0.000000));
	mesh->setVert(22, Point3(0.353553,-0.353553,0.000000));
	mesh->setVert(23, Point3(0.433013,-0.250000,0.000000));
	mesh->setVert(24, Point3(0.482963,-0.129409,0.000000));

	Face f;
  	f.smGroup = 1;	f.v[0] = 0;    f.flags = 65602;
	f.v[1] = 2;  f.v[2] = 1; mesh->faces[0] = f;
	f.v[1] = 3;  f.v[2] = 2; mesh->faces[1] = f;
	f.v[1] = 4;  f.v[2] = 3; mesh->faces[2] = f;
	f.v[1] = 5;  f.v[2] = 4; mesh->faces[3] = f;
	f.v[1] = 6;  f.v[2] = 5; mesh->faces[4] = f;
	f.v[1] = 7;  f.v[2] = 6; mesh->faces[5] = f;
	f.v[1] = 8;  f.v[2] = 7; mesh->faces[6] = f;
	f.v[1] = 9;  f.v[2] = 8; mesh->faces[7] = f;
	f.v[1] = 10;  f.v[2] = 9; mesh->faces[8] = f;
	f.v[1] = 11;  f.v[2] = 10; mesh->faces[9] = f;
	f.v[1] = 12;  f.v[2] = 11; mesh->faces[10] = f;
	f.v[1] = 13;  f.v[2] = 12; mesh->faces[11] = f;
	f.v[1] = 14;  f.v[2] = 13; mesh->faces[12] = f;
	f.v[1] = 15;  f.v[2] = 14; mesh->faces[13] = f;
	f.v[1] = 16;  f.v[2] = 15; mesh->faces[14] = f;
	f.v[1] = 17;  f.v[2] = 16; mesh->faces[15] = f;
	f.v[1] = 18;  f.v[2] = 17; mesh->faces[16] = f;
	f.v[1] = 19;  f.v[2] = 18; mesh->faces[17] = f;
	f.v[1] = 20;  f.v[2] = 19; mesh->faces[18] = f;
	f.v[1] = 21;  f.v[2] = 20; mesh->faces[19] = f;
	f.v[1] = 22;  f.v[2] = 21; mesh->faces[20] = f;
	f.v[1] = 23;  f.v[2] = 22; mesh->faces[21] = f;
	f.v[1] = 24;  f.v[2] = 23; mesh->faces[22] = f;
	f.v[1] = 1;  f.v[2] = 24; mesh->faces[23] = f;
}

//+--------------------------------------------------------------------------+
//|							From PFActionIcon								 |
//+--------------------------------------------------------------------------+
void PFActionIcon::BuildSphere(Mesh *mesh)
{
	mesh->setNumVerts(266);
	mesh->setNumFaces(528);

	mesh->setVert(0, Point3(0.000000,0.000000,0.500000));
	mesh->setVert(1, Point3(-0.000000,0.129410,0.482963));
	mesh->setVert(2, Point3(-0.033494,0.125000,0.482963));
	mesh->setVert(3, Point3(-0.064705,0.112072,0.482963));
	mesh->setVert(4, Point3(-0.091506,0.091506,0.482963));
	mesh->setVert(5, Point3(-0.112072,0.064705,0.482963));
	mesh->setVert(6, Point3(-0.125000,0.033494,0.482963));
	mesh->setVert(7, Point3(-0.129410,-0.000000,0.482963));
	mesh->setVert(8, Point3(-0.125000,-0.033494,0.482963));
	mesh->setVert(9, Point3(-0.112072,-0.064705,0.482963));
	mesh->setVert(10, Point3(-0.091506,-0.091506,0.482963));
	mesh->setVert(11, Point3(-0.064705,-0.112072,0.482963));
	mesh->setVert(12, Point3(-0.033494,-0.125000,0.482963));
	mesh->setVert(13, Point3(0.000000,-0.129410,0.482963));
	mesh->setVert(14, Point3(0.033494,-0.125000,0.482963));
	mesh->setVert(15, Point3(0.064705,-0.112072,0.482963));
	mesh->setVert(16, Point3(0.091506,-0.091506,0.482963));
	mesh->setVert(17, Point3(0.112072,-0.064705,0.482963));
	mesh->setVert(18, Point3(0.125000,-0.033494,0.482963));
	mesh->setVert(19, Point3(0.129410,0.000000,0.482963));
	mesh->setVert(20, Point3(0.125000,0.033494,0.482963));
	mesh->setVert(21, Point3(0.112072,0.064705,0.482963));
	mesh->setVert(22, Point3(0.091506,0.091506,0.482963));
	mesh->setVert(23, Point3(0.064705,0.112072,0.482963));
	mesh->setVert(24, Point3(0.033494,0.125000,0.482963));
	mesh->setVert(25, Point3(-0.000000,0.250000,0.433013));
	mesh->setVert(26, Point3(-0.064705,0.241481,0.433013));
	mesh->setVert(27, Point3(-0.125000,0.216506,0.433013));
	mesh->setVert(28, Point3(-0.176777,0.176777,0.433013));
	mesh->setVert(29, Point3(-0.216506,0.125000,0.433013));
	mesh->setVert(30, Point3(-0.241481,0.064705,0.433013));
	mesh->setVert(31, Point3(-0.250000,-0.000000,0.433013));
	mesh->setVert(32, Point3(-0.241481,-0.064705,0.433013));
	mesh->setVert(33, Point3(-0.216506,-0.125000,0.433013));
	mesh->setVert(34, Point3(-0.176777,-0.176777,0.433013));
	mesh->setVert(35, Point3(-0.125000,-0.216506,0.433013));
	mesh->setVert(36, Point3(-0.064705,-0.241481,0.433013));
	mesh->setVert(37, Point3(0.000000,-0.250000,0.433013));
	mesh->setVert(38, Point3(0.064705,-0.241481,0.433013));
	mesh->setVert(39, Point3(0.125000,-0.216506,0.433013));
	mesh->setVert(40, Point3(0.176777,-0.176777,0.433013));
	mesh->setVert(41, Point3(0.216506,-0.125000,0.433013));
	mesh->setVert(42, Point3(0.241481,-0.064705,0.433013));
	mesh->setVert(43, Point3(0.250000,0.000000,0.433013));
	mesh->setVert(44, Point3(0.241481,0.064705,0.433013));
	mesh->setVert(45, Point3(0.216506,0.125000,0.433013));
	mesh->setVert(46, Point3(0.176777,0.176777,0.433013));
	mesh->setVert(47, Point3(0.125000,0.216506,0.433013));
	mesh->setVert(48, Point3(0.064705,0.241481,0.433013));
	mesh->setVert(49, Point3(-0.000000,0.353553,0.353553));
	mesh->setVert(50, Point3(-0.091506,0.341506,0.353553));
	mesh->setVert(51, Point3(-0.176777,0.306186,0.353553));
	mesh->setVert(52, Point3(-0.250000,0.250000,0.353553));
	mesh->setVert(53, Point3(-0.306186,0.176777,0.353553));
	mesh->setVert(54, Point3(-0.341506,0.091506,0.353553));
	mesh->setVert(55, Point3(-0.353553,-0.000000,0.353553));
	mesh->setVert(56, Point3(-0.341506,-0.091506,0.353553));
	mesh->setVert(57, Point3(-0.306186,-0.176777,0.353553));
	mesh->setVert(58, Point3(-0.250000,-0.250000,0.353553));
	mesh->setVert(59, Point3(-0.176777,-0.306186,0.353553));
	mesh->setVert(60, Point3(-0.091506,-0.341506,0.353553));
	mesh->setVert(61, Point3(0.000000,-0.353553,0.353553));
	mesh->setVert(62, Point3(0.091506,-0.341506,0.353553));
	mesh->setVert(63, Point3(0.176777,-0.306186,0.353553));
	mesh->setVert(64, Point3(0.250000,-0.250000,0.353553));
	mesh->setVert(65, Point3(0.306186,-0.176777,0.353553));
	mesh->setVert(66, Point3(0.341506,-0.091506,0.353553));
	mesh->setVert(67, Point3(0.353553,0.000000,0.353553));
	mesh->setVert(68, Point3(0.341506,0.091506,0.353553));
	mesh->setVert(69, Point3(0.306186,0.176777,0.353553));
	mesh->setVert(70, Point3(0.250000,0.250000,0.353553));
	mesh->setVert(71, Point3(0.176777,0.306186,0.353553));
	mesh->setVert(72, Point3(0.091506,0.341506,0.353553));
	mesh->setVert(73, Point3(-0.000000,0.433013,0.250000));
	mesh->setVert(74, Point3(-0.112072,0.418258,0.250000));
	mesh->setVert(75, Point3(-0.216506,0.375000,0.250000));
	mesh->setVert(76, Point3(-0.306186,0.306186,0.250000));
	mesh->setVert(77, Point3(-0.375000,0.216506,0.250000));
	mesh->setVert(78, Point3(-0.418258,0.112072,0.250000));
	mesh->setVert(79, Point3(-0.433013,-0.000000,0.250000));
	mesh->setVert(80, Point3(-0.418258,-0.112072,0.250000));
	mesh->setVert(81, Point3(-0.375000,-0.216506,0.250000));
	mesh->setVert(82, Point3(-0.306186,-0.306186,0.250000));
	mesh->setVert(83, Point3(-0.216506,-0.375000,0.250000));
	mesh->setVert(84, Point3(-0.112072,-0.418258,0.250000));
	mesh->setVert(85, Point3(0.000000,-0.433013,0.250000));
	mesh->setVert(86, Point3(0.112072,-0.418258,0.250000));
	mesh->setVert(87, Point3(0.216506,-0.375000,0.250000));
	mesh->setVert(88, Point3(0.306186,-0.306186,0.250000));
	mesh->setVert(89, Point3(0.375000,-0.216506,0.250000));
	mesh->setVert(90, Point3(0.418258,-0.112072,0.250000));
	mesh->setVert(91, Point3(0.433013,0.000000,0.250000));
	mesh->setVert(92, Point3(0.418258,0.112072,0.250000));
	mesh->setVert(93, Point3(0.375000,0.216506,0.250000));
	mesh->setVert(94, Point3(0.306186,0.306186,0.250000));
	mesh->setVert(95, Point3(0.216506,0.375000,0.250000));
	mesh->setVert(96, Point3(0.112072,0.418258,0.250000));
	mesh->setVert(97, Point3(-0.000000,0.482963,0.129410));
	mesh->setVert(98, Point3(-0.125000,0.466506,0.129410));
	mesh->setVert(99, Point3(-0.241481,0.418258,0.129410));
	mesh->setVert(100, Point3(-0.341506,0.341506,0.129410));
	mesh->setVert(101, Point3(-0.418258,0.241481,0.129410));
	mesh->setVert(102, Point3(-0.466506,0.125000,0.129410));
	mesh->setVert(103, Point3(-0.482963,-0.000000,0.129410));
	mesh->setVert(104, Point3(-0.466506,-0.125000,0.129410));
	mesh->setVert(105, Point3(-0.418258,-0.241481,0.129410));
	mesh->setVert(106, Point3(-0.341506,-0.341506,0.129410));
	mesh->setVert(107, Point3(-0.241481,-0.418258,0.129410));
	mesh->setVert(108, Point3(-0.125000,-0.466506,0.129410));
	mesh->setVert(109, Point3(0.000000,-0.482963,0.129410));
	mesh->setVert(110, Point3(0.125000,-0.466506,0.129410));
	mesh->setVert(111, Point3(0.241482,-0.418258,0.129410));
	mesh->setVert(112, Point3(0.341506,-0.341506,0.129410));
	mesh->setVert(113, Point3(0.418258,-0.241481,0.129410));
	mesh->setVert(114, Point3(0.466506,-0.125000,0.129410));
	mesh->setVert(115, Point3(0.482963,0.000000,0.129410));
	mesh->setVert(116, Point3(0.466506,0.125000,0.129410));
	mesh->setVert(117, Point3(0.418258,0.241482,0.129410));
	mesh->setVert(118, Point3(0.341506,0.341506,0.129410));
	mesh->setVert(119, Point3(0.241481,0.418258,0.129410));
	mesh->setVert(120, Point3(0.125000,0.466506,0.129410));
	mesh->setVert(121, Point3(-0.000000,0.500000,-0.000000));
	mesh->setVert(122, Point3(-0.129410,0.482963,-0.000000));
	mesh->setVert(123, Point3(-0.250000,0.433013,-0.000000));
	mesh->setVert(124, Point3(-0.353553,0.353553,-0.000000));
	mesh->setVert(125, Point3(-0.433013,0.250000,-0.000000));
	mesh->setVert(126, Point3(-0.482963,0.129409,-0.000000));
	mesh->setVert(127, Point3(-0.500000,-0.000000,-0.000000));
	mesh->setVert(128, Point3(-0.482963,-0.129410,-0.000000));
	mesh->setVert(129, Point3(-0.433013,-0.250000,-0.000000));
	mesh->setVert(130, Point3(-0.353553,-0.353553,-0.000000));
	mesh->setVert(131, Point3(-0.250000,-0.433013,-0.000000));
	mesh->setVert(132, Point3(-0.129409,-0.482963,-0.000000));
	mesh->setVert(133, Point3(0.000000,-0.500000,-0.000000));
	mesh->setVert(134, Point3(0.129410,-0.482963,-0.000000));
	mesh->setVert(135, Point3(0.250000,-0.433013,-0.000000));
	mesh->setVert(136, Point3(0.353553,-0.353553,-0.000000));
	mesh->setVert(137, Point3(0.433013,-0.250000,-0.000000));
	mesh->setVert(138, Point3(0.482963,-0.129409,-0.000000));
	mesh->setVert(139, Point3(0.500000,0.000000,-0.000000));
	mesh->setVert(140, Point3(0.482963,0.129410,-0.000000));
	mesh->setVert(141, Point3(0.433013,0.250000,-0.000000));
	mesh->setVert(142, Point3(0.353553,0.353553,-0.000000));
	mesh->setVert(143, Point3(0.250000,0.433013,-0.000000));
	mesh->setVert(144, Point3(0.129409,0.482963,-0.000000));
	mesh->setVert(145, Point3(-0.000000,0.482963,-0.129410));
	mesh->setVert(146, Point3(-0.125000,0.466506,-0.129410));
	mesh->setVert(147, Point3(-0.241481,0.418258,-0.129410));
	mesh->setVert(148, Point3(-0.341506,0.341506,-0.129410));
	mesh->setVert(149, Point3(-0.418258,0.241481,-0.129410));
	mesh->setVert(150, Point3(-0.466506,0.125000,-0.129410));
	mesh->setVert(151, Point3(-0.482963,-0.000000,-0.129410));
	mesh->setVert(152, Point3(-0.466506,-0.125000,-0.129410));
	mesh->setVert(153, Point3(-0.418258,-0.241481,-0.129410));
	mesh->setVert(154, Point3(-0.341506,-0.341506,-0.129410));
	mesh->setVert(155, Point3(-0.241481,-0.418258,-0.129410));
	mesh->setVert(156, Point3(-0.125000,-0.466506,-0.129410));
	mesh->setVert(157, Point3(0.000000,-0.482963,-0.129410));
	mesh->setVert(158, Point3(0.125000,-0.466506,-0.129410));
	mesh->setVert(159, Point3(0.241482,-0.418258,-0.129410));
	mesh->setVert(160, Point3(0.341506,-0.341506,-0.129410));
	mesh->setVert(161, Point3(0.418258,-0.241481,-0.129410));
	mesh->setVert(162, Point3(0.466506,-0.125000,-0.129410));
	mesh->setVert(163, Point3(0.482963,0.000000,-0.129410));
	mesh->setVert(164, Point3(0.466506,0.125000,-0.129410));
	mesh->setVert(165, Point3(0.418258,0.241482,-0.129410));
	mesh->setVert(166, Point3(0.341506,0.341506,-0.129410));
	mesh->setVert(167, Point3(0.241481,0.418258,-0.129410));
	mesh->setVert(168, Point3(0.125000,0.466506,-0.129410));
	mesh->setVert(169, Point3(-0.000000,0.433013,-0.250000));
	mesh->setVert(170, Point3(-0.112072,0.418258,-0.250000));
	mesh->setVert(171, Point3(-0.216506,0.375000,-0.250000));
	mesh->setVert(172, Point3(-0.306186,0.306186,-0.250000));
	mesh->setVert(173, Point3(-0.375000,0.216506,-0.250000));
	mesh->setVert(174, Point3(-0.418258,0.112072,-0.250000));
	mesh->setVert(175, Point3(-0.433013,-0.000000,-0.250000));
	mesh->setVert(176, Point3(-0.418258,-0.112072,-0.250000));
	mesh->setVert(177, Point3(-0.375000,-0.216506,-0.250000));
	mesh->setVert(178, Point3(-0.306186,-0.306186,-0.250000));
	mesh->setVert(179, Point3(-0.216506,-0.375000,-0.250000));
	mesh->setVert(180, Point3(-0.112072,-0.418258,-0.250000));
	mesh->setVert(181, Point3(0.000000,-0.433013,-0.250000));
	mesh->setVert(182, Point3(0.112072,-0.418258,-0.250000));
	mesh->setVert(183, Point3(0.216506,-0.375000,-0.250000));
	mesh->setVert(184, Point3(0.306186,-0.306186,-0.250000));
	mesh->setVert(185, Point3(0.375000,-0.216506,-0.250000));
	mesh->setVert(186, Point3(0.418258,-0.112072,-0.250000));
	mesh->setVert(187, Point3(0.433013,0.000000,-0.250000));
	mesh->setVert(188, Point3(0.418258,0.112072,-0.250000));
	mesh->setVert(189, Point3(0.375000,0.216506,-0.250000));
	mesh->setVert(190, Point3(0.306186,0.306186,-0.250000));
	mesh->setVert(191, Point3(0.216506,0.375000,-0.250000));
	mesh->setVert(192, Point3(0.112072,0.418258,-0.250000));
	mesh->setVert(193, Point3(-0.000000,0.353553,-0.353553));
	mesh->setVert(194, Point3(-0.091506,0.341506,-0.353553));
	mesh->setVert(195, Point3(-0.176777,0.306186,-0.353553));
	mesh->setVert(196, Point3(-0.250000,0.250000,-0.353553));
	mesh->setVert(197, Point3(-0.306186,0.176777,-0.353553));
	mesh->setVert(198, Point3(-0.341506,0.091506,-0.353553));
	mesh->setVert(199, Point3(-0.353553,-0.000000,-0.353553));
	mesh->setVert(200, Point3(-0.341506,-0.091506,-0.353553));
	mesh->setVert(201, Point3(-0.306186,-0.176777,-0.353553));
	mesh->setVert(202, Point3(-0.250000,-0.250000,-0.353553));
	mesh->setVert(203, Point3(-0.176777,-0.306186,-0.353553));
	mesh->setVert(204, Point3(-0.091506,-0.341506,-0.353553));
	mesh->setVert(205, Point3(0.000000,-0.353553,-0.353553));
	mesh->setVert(206, Point3(0.091506,-0.341506,-0.353553));
	mesh->setVert(207, Point3(0.176777,-0.306186,-0.353553));
	mesh->setVert(208, Point3(0.250000,-0.250000,-0.353553));
	mesh->setVert(209, Point3(0.306186,-0.176777,-0.353553));
	mesh->setVert(210, Point3(0.341506,-0.091506,-0.353553));
	mesh->setVert(211, Point3(0.353553,0.000000,-0.353553));
	mesh->setVert(212, Point3(0.341506,0.091506,-0.353553));
	mesh->setVert(213, Point3(0.306186,0.176777,-0.353553));
	mesh->setVert(214, Point3(0.250000,0.250000,-0.353553));
	mesh->setVert(215, Point3(0.176777,0.306186,-0.353553));
	mesh->setVert(216, Point3(0.091506,0.341506,-0.353553));
	mesh->setVert(217, Point3(-0.000000,0.250000,-0.433013));
	mesh->setVert(218, Point3(-0.064705,0.241481,-0.433013));
	mesh->setVert(219, Point3(-0.125000,0.216506,-0.433013));
	mesh->setVert(220, Point3(-0.176777,0.176777,-0.433013));
	mesh->setVert(221, Point3(-0.216506,0.125000,-0.433013));
	mesh->setVert(222, Point3(-0.241481,0.064705,-0.433013));
	mesh->setVert(223, Point3(-0.250000,-0.000000,-0.433013));
	mesh->setVert(224, Point3(-0.241481,-0.064705,-0.433013));
	mesh->setVert(225, Point3(-0.216506,-0.125000,-0.433013));
	mesh->setVert(226, Point3(-0.176777,-0.176777,-0.433013));
	mesh->setVert(227, Point3(-0.125000,-0.216506,-0.433013));
	mesh->setVert(228, Point3(-0.064705,-0.241481,-0.433013));
	mesh->setVert(229, Point3(0.000000,-0.250000,-0.433013));
	mesh->setVert(230, Point3(0.064705,-0.241481,-0.433013));
	mesh->setVert(231, Point3(0.125000,-0.216506,-0.433013));
	mesh->setVert(232, Point3(0.176777,-0.176777,-0.433013));
	mesh->setVert(233, Point3(0.216506,-0.125000,-0.433013));
	mesh->setVert(234, Point3(0.241481,-0.064705,-0.433013));
	mesh->setVert(235, Point3(0.250000,0.000000,-0.433013));
	mesh->setVert(236, Point3(0.241481,0.064705,-0.433013));
	mesh->setVert(237, Point3(0.216506,0.125000,-0.433013));
	mesh->setVert(238, Point3(0.176777,0.176777,-0.433013));
	mesh->setVert(239, Point3(0.125000,0.216506,-0.433013));
	mesh->setVert(240, Point3(0.064705,0.241481,-0.433013));
	mesh->setVert(241, Point3(-0.000000,0.129409,-0.482963));
	mesh->setVert(242, Point3(-0.033494,0.125000,-0.482963));
	mesh->setVert(243, Point3(-0.064705,0.112072,-0.482963));
	mesh->setVert(244, Point3(-0.091506,0.091506,-0.482963));
	mesh->setVert(245, Point3(-0.112072,0.064705,-0.482963));
	mesh->setVert(246, Point3(-0.125000,0.033494,-0.482963));
	mesh->setVert(247, Point3(-0.129409,-0.000000,-0.482963));
	mesh->setVert(248, Point3(-0.125000,-0.033494,-0.482963));
	mesh->setVert(249, Point3(-0.112072,-0.064705,-0.482963));
	mesh->setVert(250, Point3(-0.091506,-0.091506,-0.482963));
	mesh->setVert(251, Point3(-0.064705,-0.112072,-0.482963));
	mesh->setVert(252, Point3(-0.033494,-0.125000,-0.482963));
	mesh->setVert(253, Point3(0.000000,-0.129409,-0.482963));
	mesh->setVert(254, Point3(0.033494,-0.125000,-0.482963));
	mesh->setVert(255, Point3(0.064705,-0.112072,-0.482963));
	mesh->setVert(256, Point3(0.091506,-0.091506,-0.482963));
	mesh->setVert(257, Point3(0.112072,-0.064705,-0.482963));
	mesh->setVert(258, Point3(0.125000,-0.033494,-0.482963));
	mesh->setVert(259, Point3(0.129409,0.000000,-0.482963));
	mesh->setVert(260, Point3(0.125000,0.033494,-0.482963));
	mesh->setVert(261, Point3(0.112072,0.064705,-0.482963));
	mesh->setVert(262, Point3(0.091506,0.091506,-0.482963));
	mesh->setVert(263, Point3(0.064705,0.112072,-0.482963));
	mesh->setVert(264, Point3(0.033494,0.125000,-0.482963));
	mesh->setVert(265, Point3(0.000000,0.000000,-0.500000));

	Face f;
	f.smGroup = 1;
	f.v[0] = 0;  f.v[1] = 1;  f.v[2] = 2;  f.flags = 65601; mesh->faces[0] = f;
	f.v[0] = 0;  f.v[1] = 2;  f.v[2] = 3;  f.flags = 65600; mesh->faces[1] = f;
	f.v[0] = 0;  f.v[1] = 3;  f.v[2] = 4;  f.flags = 65600; mesh->faces[2] = f;
	f.v[0] = 0;  f.v[1] = 4;  f.v[2] = 5;  f.flags = 65600; mesh->faces[3] = f;
	f.v[0] = 0;  f.v[1] = 5;  f.v[2] = 6;  f.flags = 65600; mesh->faces[4] = f;
	f.v[0] = 0;  f.v[1] = 6;  f.v[2] = 7;  f.flags = 65604; mesh->faces[5] = f;
	f.v[0] = 0;  f.v[1] = 7;  f.v[2] = 8;  f.flags = 65601; mesh->faces[6] = f;
	f.v[0] = 0;  f.v[1] = 8;  f.v[2] = 9;  f.flags = 65600; mesh->faces[7] = f;
	f.v[0] = 0;  f.v[1] = 9;  f.v[2] = 10;  f.flags = 65600; mesh->faces[8] = f;
	f.v[0] = 0;  f.v[1] = 10;  f.v[2] = 11;  f.flags = 65600; mesh->faces[9] = f;
	f.v[0] = 0;  f.v[1] = 11;  f.v[2] = 12;  f.flags = 65600; mesh->faces[10] = f;
	f.v[0] = 0;  f.v[1] = 12;  f.v[2] = 13;  f.flags = 65604; mesh->faces[11] = f;
	f.v[0] = 0;  f.v[1] = 13;  f.v[2] = 14;  f.flags = 65601; mesh->faces[12] = f;
	f.v[0] = 0;  f.v[1] = 14;  f.v[2] = 15;  f.flags = 65600; mesh->faces[13] = f;
	f.v[0] = 0;  f.v[1] = 15;  f.v[2] = 16;  f.flags = 65600; mesh->faces[14] = f;
	f.v[0] = 0;  f.v[1] = 16;  f.v[2] = 17;  f.flags = 65600; mesh->faces[15] = f;
	f.v[0] = 0;  f.v[1] = 17;  f.v[2] = 18;  f.flags = 65600; mesh->faces[16] = f;
	f.v[0] = 0;  f.v[1] = 18;  f.v[2] = 19;  f.flags = 65604; mesh->faces[17] = f;
	f.v[0] = 0;  f.v[1] = 19;  f.v[2] = 20;  f.flags = 65601; mesh->faces[18] = f;
	f.v[0] = 0;  f.v[1] = 20;  f.v[2] = 21;  f.flags = 65600; mesh->faces[19] = f;
	f.v[0] = 0;  f.v[1] = 21;  f.v[2] = 22;  f.flags = 65600; mesh->faces[20] = f;
	f.v[0] = 0;  f.v[1] = 22;  f.v[2] = 23;  f.flags = 65600; mesh->faces[21] = f;
	f.v[0] = 0;  f.v[1] = 23;  f.v[2] = 24;  f.flags = 65600; mesh->faces[22] = f;
	f.v[0] = 0;  f.v[1] = 24;  f.v[2] = 1;  f.flags = 65604; mesh->faces[23] = f;
	f.v[0] = 1;  f.v[1] = 25;  f.v[2] = 26;  f.flags = 65601; mesh->faces[24] = f;
	f.v[0] = 1;  f.v[1] = 26;  f.v[2] = 2;  f.flags = 65600; mesh->faces[25] = f;
	f.v[0] = 2;  f.v[1] = 26;  f.v[2] = 27;  f.flags = 65600; mesh->faces[26] = f;
	f.v[0] = 2;  f.v[1] = 27;  f.v[2] = 3;  f.flags = 65600; mesh->faces[27] = f;
	f.v[0] = 3;  f.v[1] = 27;  f.v[2] = 28;  f.flags = 65600; mesh->faces[28] = f;
	f.v[0] = 3;  f.v[1] = 28;  f.v[2] = 4;  f.flags = 65600; mesh->faces[29] = f;
	f.v[0] = 4;  f.v[1] = 28;  f.v[2] = 29;  f.flags = 65600; mesh->faces[30] = f;
	f.v[0] = 4;  f.v[1] = 29;  f.v[2] = 5;  f.flags = 65600; mesh->faces[31] = f;
	f.v[0] = 5;  f.v[1] = 29;  f.v[2] = 30;  f.flags = 65600; mesh->faces[32] = f;
	f.v[0] = 5;  f.v[1] = 30;  f.v[2] = 6;  f.flags = 65600; mesh->faces[33] = f;
	f.v[0] = 6;  f.v[1] = 30;  f.v[2] = 31;  f.flags = 65600; mesh->faces[34] = f;
	f.v[0] = 6;  f.v[1] = 31;  f.v[2] = 7;  f.flags = 65602; mesh->faces[35] = f;
	f.v[0] = 7;  f.v[1] = 31;  f.v[2] = 32;  f.flags = 65601; mesh->faces[36] = f;
	f.v[0] = 7;  f.v[1] = 32;  f.v[2] = 8;  f.flags = 65600; mesh->faces[37] = f;
	f.v[0] = 8;  f.v[1] = 32;  f.v[2] = 33;  f.flags = 65600; mesh->faces[38] = f;
	f.v[0] = 8;  f.v[1] = 33;  f.v[2] = 9;  f.flags = 65600; mesh->faces[39] = f;
	f.v[0] = 9;  f.v[1] = 33;  f.v[2] = 34;  f.flags = 65600; mesh->faces[40] = f;
	f.v[0] = 9;  f.v[1] = 34;  f.v[2] = 10;  f.flags = 65600; mesh->faces[41] = f;
	f.v[0] = 10;  f.v[1] = 34;  f.v[2] = 35;  f.flags = 65600; mesh->faces[42] = f;
	f.v[0] = 10;  f.v[1] = 35;  f.v[2] = 11;  f.flags = 65600; mesh->faces[43] = f;
	f.v[0] = 11;  f.v[1] = 35;  f.v[2] = 36;  f.flags = 65600; mesh->faces[44] = f;
	f.v[0] = 11;  f.v[1] = 36;  f.v[2] = 12;  f.flags = 65600; mesh->faces[45] = f;
	f.v[0] = 12;  f.v[1] = 36;  f.v[2] = 37;  f.flags = 65600; mesh->faces[46] = f;
	f.v[0] = 12;  f.v[1] = 37;  f.v[2] = 13;  f.flags = 65602; mesh->faces[47] = f;
	f.v[0] = 13;  f.v[1] = 37;  f.v[2] = 38;  f.flags = 65601; mesh->faces[48] = f;
	f.v[0] = 13;  f.v[1] = 38;  f.v[2] = 14;  f.flags = 65600; mesh->faces[49] = f;
	f.v[0] = 14;  f.v[1] = 38;  f.v[2] = 39;  f.flags = 65600; mesh->faces[50] = f;
	f.v[0] = 14;  f.v[1] = 39;  f.v[2] = 15;  f.flags = 65600; mesh->faces[51] = f;
	f.v[0] = 15;  f.v[1] = 39;  f.v[2] = 40;  f.flags = 65600; mesh->faces[52] = f;
	f.v[0] = 15;  f.v[1] = 40;  f.v[2] = 16;  f.flags = 65600; mesh->faces[53] = f;
	f.v[0] = 16;  f.v[1] = 40;  f.v[2] = 41;  f.flags = 65600; mesh->faces[54] = f;
	f.v[0] = 16;  f.v[1] = 41;  f.v[2] = 17;  f.flags = 65600; mesh->faces[55] = f;
	f.v[0] = 17;  f.v[1] = 41;  f.v[2] = 42;  f.flags = 65600; mesh->faces[56] = f;
	f.v[0] = 17;  f.v[1] = 42;  f.v[2] = 18;  f.flags = 65600; mesh->faces[57] = f;
	f.v[0] = 18;  f.v[1] = 42;  f.v[2] = 43;  f.flags = 65600; mesh->faces[58] = f;
	f.v[0] = 18;  f.v[1] = 43;  f.v[2] = 19;  f.flags = 65602; mesh->faces[59] = f;
	f.v[0] = 19;  f.v[1] = 43;  f.v[2] = 44;  f.flags = 65601; mesh->faces[60] = f;
	f.v[0] = 19;  f.v[1] = 44;  f.v[2] = 20;  f.flags = 65600; mesh->faces[61] = f;
	f.v[0] = 20;  f.v[1] = 44;  f.v[2] = 45;  f.flags = 65600; mesh->faces[62] = f;
	f.v[0] = 20;  f.v[1] = 45;  f.v[2] = 21;  f.flags = 65600; mesh->faces[63] = f;
	f.v[0] = 21;  f.v[1] = 45;  f.v[2] = 46;  f.flags = 65600; mesh->faces[64] = f;
	f.v[0] = 21;  f.v[1] = 46;  f.v[2] = 22;  f.flags = 65600; mesh->faces[65] = f;
	f.v[0] = 22;  f.v[1] = 46;  f.v[2] = 47;  f.flags = 65600; mesh->faces[66] = f;
	f.v[0] = 22;  f.v[1] = 47;  f.v[2] = 23;  f.flags = 65600; mesh->faces[67] = f;
	f.v[0] = 23;  f.v[1] = 47;  f.v[2] = 48;  f.flags = 65600; mesh->faces[68] = f;
	f.v[0] = 23;  f.v[1] = 48;  f.v[2] = 24;  f.flags = 65600; mesh->faces[69] = f;
	f.v[0] = 24;  f.v[1] = 48;  f.v[2] = 25;  f.flags = 65600; mesh->faces[70] = f;
	f.v[0] = 24;  f.v[1] = 25;  f.v[2] = 1;  f.flags = 65602; mesh->faces[71] = f;
	f.v[0] = 25;  f.v[1] = 49;  f.v[2] = 50;  f.flags = 65601; mesh->faces[72] = f;
	f.v[0] = 25;  f.v[1] = 50;  f.v[2] = 26;  f.flags = 65600; mesh->faces[73] = f;
	f.v[0] = 26;  f.v[1] = 50;  f.v[2] = 51;  f.flags = 65600; mesh->faces[74] = f;
	f.v[0] = 26;  f.v[1] = 51;  f.v[2] = 27;  f.flags = 65600; mesh->faces[75] = f;
	f.v[0] = 27;  f.v[1] = 51;  f.v[2] = 52;  f.flags = 65600; mesh->faces[76] = f;
	f.v[0] = 27;  f.v[1] = 52;  f.v[2] = 28;  f.flags = 65600; mesh->faces[77] = f;
	f.v[0] = 28;  f.v[1] = 52;  f.v[2] = 53;  f.flags = 65600; mesh->faces[78] = f;
	f.v[0] = 28;  f.v[1] = 53;  f.v[2] = 29;  f.flags = 65600; mesh->faces[79] = f;
	f.v[0] = 29;  f.v[1] = 53;  f.v[2] = 54;  f.flags = 65600; mesh->faces[80] = f;
	f.v[0] = 29;  f.v[1] = 54;  f.v[2] = 30;  f.flags = 65600; mesh->faces[81] = f;
	f.v[0] = 30;  f.v[1] = 54;  f.v[2] = 55;  f.flags = 65600; mesh->faces[82] = f;
	f.v[0] = 30;  f.v[1] = 55;  f.v[2] = 31;  f.flags = 65602; mesh->faces[83] = f;
	f.v[0] = 31;  f.v[1] = 55;  f.v[2] = 56;  f.flags = 65601; mesh->faces[84] = f;
	f.v[0] = 31;  f.v[1] = 56;  f.v[2] = 32;  f.flags = 65600; mesh->faces[85] = f;
	f.v[0] = 32;  f.v[1] = 56;  f.v[2] = 57;  f.flags = 65600; mesh->faces[86] = f;
	f.v[0] = 32;  f.v[1] = 57;  f.v[2] = 33;  f.flags = 65600; mesh->faces[87] = f;
	f.v[0] = 33;  f.v[1] = 57;  f.v[2] = 58;  f.flags = 65600; mesh->faces[88] = f;
	f.v[0] = 33;  f.v[1] = 58;  f.v[2] = 34;  f.flags = 65600; mesh->faces[89] = f;
	f.v[0] = 34;  f.v[1] = 58;  f.v[2] = 59;  f.flags = 65600; mesh->faces[90] = f;
	f.v[0] = 34;  f.v[1] = 59;  f.v[2] = 35;  f.flags = 65600; mesh->faces[91] = f;
	f.v[0] = 35;  f.v[1] = 59;  f.v[2] = 60;  f.flags = 65600; mesh->faces[92] = f;
	f.v[0] = 35;  f.v[1] = 60;  f.v[2] = 36;  f.flags = 65600; mesh->faces[93] = f;
	f.v[0] = 36;  f.v[1] = 60;  f.v[2] = 61;  f.flags = 65600; mesh->faces[94] = f;
	f.v[0] = 36;  f.v[1] = 61;  f.v[2] = 37;  f.flags = 65602; mesh->faces[95] = f;
	f.v[0] = 37;  f.v[1] = 61;  f.v[2] = 62;  f.flags = 65601; mesh->faces[96] = f;
	f.v[0] = 37;  f.v[1] = 62;  f.v[2] = 38;  f.flags = 65600; mesh->faces[97] = f;
	f.v[0] = 38;  f.v[1] = 62;  f.v[2] = 63;  f.flags = 65600; mesh->faces[98] = f;
	f.v[0] = 38;  f.v[1] = 63;  f.v[2] = 39;  f.flags = 65600; mesh->faces[99] = f;
	f.v[0] = 39;  f.v[1] = 63;  f.v[2] = 64;  f.flags = 65600; mesh->faces[100] = f;
	f.v[0] = 39;  f.v[1] = 64;  f.v[2] = 40;  f.flags = 65600; mesh->faces[101] = f;
	f.v[0] = 40;  f.v[1] = 64;  f.v[2] = 65;  f.flags = 65600; mesh->faces[102] = f;
	f.v[0] = 40;  f.v[1] = 65;  f.v[2] = 41;  f.flags = 65600; mesh->faces[103] = f;
	f.v[0] = 41;  f.v[1] = 65;  f.v[2] = 66;  f.flags = 65600; mesh->faces[104] = f;
	f.v[0] = 41;  f.v[1] = 66;  f.v[2] = 42;  f.flags = 65600; mesh->faces[105] = f;
	f.v[0] = 42;  f.v[1] = 66;  f.v[2] = 67;  f.flags = 65600; mesh->faces[106] = f;
	f.v[0] = 42;  f.v[1] = 67;  f.v[2] = 43;  f.flags = 65602; mesh->faces[107] = f;
	f.v[0] = 43;  f.v[1] = 67;  f.v[2] = 68;  f.flags = 65601; mesh->faces[108] = f;
	f.v[0] = 43;  f.v[1] = 68;  f.v[2] = 44;  f.flags = 65600; mesh->faces[109] = f;
	f.v[0] = 44;  f.v[1] = 68;  f.v[2] = 69;  f.flags = 65600; mesh->faces[110] = f;
	f.v[0] = 44;  f.v[1] = 69;  f.v[2] = 45;  f.flags = 65600; mesh->faces[111] = f;
	f.v[0] = 45;  f.v[1] = 69;  f.v[2] = 70;  f.flags = 65600; mesh->faces[112] = f;
	f.v[0] = 45;  f.v[1] = 70;  f.v[2] = 46;  f.flags = 65600; mesh->faces[113] = f;
	f.v[0] = 46;  f.v[1] = 70;  f.v[2] = 71;  f.flags = 65600; mesh->faces[114] = f;
	f.v[0] = 46;  f.v[1] = 71;  f.v[2] = 47;  f.flags = 65600; mesh->faces[115] = f;
	f.v[0] = 47;  f.v[1] = 71;  f.v[2] = 72;  f.flags = 65600; mesh->faces[116] = f;
	f.v[0] = 47;  f.v[1] = 72;  f.v[2] = 48;  f.flags = 65600; mesh->faces[117] = f;
	f.v[0] = 48;  f.v[1] = 72;  f.v[2] = 49;  f.flags = 65600; mesh->faces[118] = f;
	f.v[0] = 48;  f.v[1] = 49;  f.v[2] = 25;  f.flags = 65602; mesh->faces[119] = f;
	f.v[0] = 49;  f.v[1] = 73;  f.v[2] = 74;  f.flags = 65601; mesh->faces[120] = f;
	f.v[0] = 49;  f.v[1] = 74;  f.v[2] = 50;  f.flags = 65600; mesh->faces[121] = f;
	f.v[0] = 50;  f.v[1] = 74;  f.v[2] = 75;  f.flags = 65600; mesh->faces[122] = f;
	f.v[0] = 50;  f.v[1] = 75;  f.v[2] = 51;  f.flags = 65600; mesh->faces[123] = f;
	f.v[0] = 51;  f.v[1] = 75;  f.v[2] = 76;  f.flags = 65600; mesh->faces[124] = f;
	f.v[0] = 51;  f.v[1] = 76;  f.v[2] = 52;  f.flags = 65600; mesh->faces[125] = f;
	f.v[0] = 52;  f.v[1] = 76;  f.v[2] = 77;  f.flags = 65600; mesh->faces[126] = f;
	f.v[0] = 52;  f.v[1] = 77;  f.v[2] = 53;  f.flags = 65600; mesh->faces[127] = f;
	f.v[0] = 53;  f.v[1] = 77;  f.v[2] = 78;  f.flags = 65600; mesh->faces[128] = f;
	f.v[0] = 53;  f.v[1] = 78;  f.v[2] = 54;  f.flags = 65600; mesh->faces[129] = f;
	f.v[0] = 54;  f.v[1] = 78;  f.v[2] = 79;  f.flags = 65600; mesh->faces[130] = f;
	f.v[0] = 54;  f.v[1] = 79;  f.v[2] = 55;  f.flags = 65602; mesh->faces[131] = f;
	f.v[0] = 55;  f.v[1] = 79;  f.v[2] = 80;  f.flags = 65601; mesh->faces[132] = f;
	f.v[0] = 55;  f.v[1] = 80;  f.v[2] = 56;  f.flags = 65600; mesh->faces[133] = f;
	f.v[0] = 56;  f.v[1] = 80;  f.v[2] = 81;  f.flags = 65600; mesh->faces[134] = f;
	f.v[0] = 56;  f.v[1] = 81;  f.v[2] = 57;  f.flags = 65600; mesh->faces[135] = f;
	f.v[0] = 57;  f.v[1] = 81;  f.v[2] = 82;  f.flags = 65600; mesh->faces[136] = f;
	f.v[0] = 57;  f.v[1] = 82;  f.v[2] = 58;  f.flags = 65600; mesh->faces[137] = f;
	f.v[0] = 58;  f.v[1] = 82;  f.v[2] = 83;  f.flags = 65600; mesh->faces[138] = f;
	f.v[0] = 58;  f.v[1] = 83;  f.v[2] = 59;  f.flags = 65600; mesh->faces[139] = f;
	f.v[0] = 59;  f.v[1] = 83;  f.v[2] = 84;  f.flags = 65600; mesh->faces[140] = f;
	f.v[0] = 59;  f.v[1] = 84;  f.v[2] = 60;  f.flags = 65600; mesh->faces[141] = f;
	f.v[0] = 60;  f.v[1] = 84;  f.v[2] = 85;  f.flags = 65600; mesh->faces[142] = f;
	f.v[0] = 60;  f.v[1] = 85;  f.v[2] = 61;  f.flags = 65602; mesh->faces[143] = f;
	f.v[0] = 61;  f.v[1] = 85;  f.v[2] = 86;  f.flags = 65601; mesh->faces[144] = f;
	f.v[0] = 61;  f.v[1] = 86;  f.v[2] = 62;  f.flags = 65600; mesh->faces[145] = f;
	f.v[0] = 62;  f.v[1] = 86;  f.v[2] = 87;  f.flags = 65600; mesh->faces[146] = f;
	f.v[0] = 62;  f.v[1] = 87;  f.v[2] = 63;  f.flags = 65600; mesh->faces[147] = f;
	f.v[0] = 63;  f.v[1] = 87;  f.v[2] = 88;  f.flags = 65600; mesh->faces[148] = f;
	f.v[0] = 63;  f.v[1] = 88;  f.v[2] = 64;  f.flags = 65600; mesh->faces[149] = f;
	f.v[0] = 64;  f.v[1] = 88;  f.v[2] = 89;  f.flags = 65600; mesh->faces[150] = f;
	f.v[0] = 64;  f.v[1] = 89;  f.v[2] = 65;  f.flags = 65600; mesh->faces[151] = f;
	f.v[0] = 65;  f.v[1] = 89;  f.v[2] = 90;  f.flags = 65600; mesh->faces[152] = f;
	f.v[0] = 65;  f.v[1] = 90;  f.v[2] = 66;  f.flags = 65600; mesh->faces[153] = f;
	f.v[0] = 66;  f.v[1] = 90;  f.v[2] = 91;  f.flags = 65600; mesh->faces[154] = f;
	f.v[0] = 66;  f.v[1] = 91;  f.v[2] = 67;  f.flags = 65602; mesh->faces[155] = f;
	f.v[0] = 67;  f.v[1] = 91;  f.v[2] = 92;  f.flags = 65601; mesh->faces[156] = f;
	f.v[0] = 67;  f.v[1] = 92;  f.v[2] = 68;  f.flags = 65600; mesh->faces[157] = f;
	f.v[0] = 68;  f.v[1] = 92;  f.v[2] = 93;  f.flags = 65600; mesh->faces[158] = f;
	f.v[0] = 68;  f.v[1] = 93;  f.v[2] = 69;  f.flags = 65600; mesh->faces[159] = f;
	f.v[0] = 69;  f.v[1] = 93;  f.v[2] = 94;  f.flags = 65600; mesh->faces[160] = f;
	f.v[0] = 69;  f.v[1] = 94;  f.v[2] = 70;  f.flags = 65600; mesh->faces[161] = f;
	f.v[0] = 70;  f.v[1] = 94;  f.v[2] = 95;  f.flags = 65600; mesh->faces[162] = f;
	f.v[0] = 70;  f.v[1] = 95;  f.v[2] = 71;  f.flags = 65600; mesh->faces[163] = f;
	f.v[0] = 71;  f.v[1] = 95;  f.v[2] = 96;  f.flags = 65600; mesh->faces[164] = f;
	f.v[0] = 71;  f.v[1] = 96;  f.v[2] = 72;  f.flags = 65600; mesh->faces[165] = f;
	f.v[0] = 72;  f.v[1] = 96;  f.v[2] = 73;  f.flags = 65600; mesh->faces[166] = f;
	f.v[0] = 72;  f.v[1] = 73;  f.v[2] = 49;  f.flags = 65602; mesh->faces[167] = f;
	f.v[0] = 73;  f.v[1] = 97;  f.v[2] = 98;  f.flags = 65601; mesh->faces[168] = f;
	f.v[0] = 73;  f.v[1] = 98;  f.v[2] = 74;  f.flags = 65600; mesh->faces[169] = f;
	f.v[0] = 74;  f.v[1] = 98;  f.v[2] = 99;  f.flags = 65600; mesh->faces[170] = f;
	f.v[0] = 74;  f.v[1] = 99;  f.v[2] = 75;  f.flags = 65600; mesh->faces[171] = f;
	f.v[0] = 75;  f.v[1] = 99;  f.v[2] = 100;  f.flags = 65600; mesh->faces[172] = f;
	f.v[0] = 75;  f.v[1] = 100;  f.v[2] = 76;  f.flags = 65600; mesh->faces[173] = f;
	f.v[0] = 76;  f.v[1] = 100;  f.v[2] = 101;  f.flags = 65600; mesh->faces[174] = f;
	f.v[0] = 76;  f.v[1] = 101;  f.v[2] = 77;  f.flags = 65600; mesh->faces[175] = f;
	f.v[0] = 77;  f.v[1] = 101;  f.v[2] = 102;  f.flags = 65600; mesh->faces[176] = f;
	f.v[0] = 77;  f.v[1] = 102;  f.v[2] = 78;  f.flags = 65600; mesh->faces[177] = f;
	f.v[0] = 78;  f.v[1] = 102;  f.v[2] = 103;  f.flags = 65600; mesh->faces[178] = f;
	f.v[0] = 78;  f.v[1] = 103;  f.v[2] = 79;  f.flags = 65602; mesh->faces[179] = f;
	f.v[0] = 79;  f.v[1] = 103;  f.v[2] = 104;  f.flags = 65601; mesh->faces[180] = f;
	f.v[0] = 79;  f.v[1] = 104;  f.v[2] = 80;  f.flags = 65600; mesh->faces[181] = f;
	f.v[0] = 80;  f.v[1] = 104;  f.v[2] = 105;  f.flags = 65600; mesh->faces[182] = f;
	f.v[0] = 80;  f.v[1] = 105;  f.v[2] = 81;  f.flags = 65600; mesh->faces[183] = f;
	f.v[0] = 81;  f.v[1] = 105;  f.v[2] = 106;  f.flags = 65600; mesh->faces[184] = f;
	f.v[0] = 81;  f.v[1] = 106;  f.v[2] = 82;  f.flags = 65600; mesh->faces[185] = f;
	f.v[0] = 82;  f.v[1] = 106;  f.v[2] = 107;  f.flags = 65600; mesh->faces[186] = f;
	f.v[0] = 82;  f.v[1] = 107;  f.v[2] = 83;  f.flags = 65600; mesh->faces[187] = f;
	f.v[0] = 83;  f.v[1] = 107;  f.v[2] = 108;  f.flags = 65600; mesh->faces[188] = f;
	f.v[0] = 83;  f.v[1] = 108;  f.v[2] = 84;  f.flags = 65600; mesh->faces[189] = f;
	f.v[0] = 84;  f.v[1] = 108;  f.v[2] = 109;  f.flags = 65600; mesh->faces[190] = f;
	f.v[0] = 84;  f.v[1] = 109;  f.v[2] = 85;  f.flags = 65602; mesh->faces[191] = f;
	f.v[0] = 85;  f.v[1] = 109;  f.v[2] = 110;  f.flags = 65601; mesh->faces[192] = f;
	f.v[0] = 85;  f.v[1] = 110;  f.v[2] = 86;  f.flags = 65600; mesh->faces[193] = f;
	f.v[0] = 86;  f.v[1] = 110;  f.v[2] = 111;  f.flags = 65600; mesh->faces[194] = f;
	f.v[0] = 86;  f.v[1] = 111;  f.v[2] = 87;  f.flags = 65600; mesh->faces[195] = f;
	f.v[0] = 87;  f.v[1] = 111;  f.v[2] = 112;  f.flags = 65600; mesh->faces[196] = f;
	f.v[0] = 87;  f.v[1] = 112;  f.v[2] = 88;  f.flags = 65600; mesh->faces[197] = f;
	f.v[0] = 88;  f.v[1] = 112;  f.v[2] = 113;  f.flags = 65600; mesh->faces[198] = f;
	f.v[0] = 88;  f.v[1] = 113;  f.v[2] = 89;  f.flags = 65600; mesh->faces[199] = f;
	f.v[0] = 89;  f.v[1] = 113;  f.v[2] = 114;  f.flags = 65600; mesh->faces[200] = f;
	f.v[0] = 89;  f.v[1] = 114;  f.v[2] = 90;  f.flags = 65600; mesh->faces[201] = f;
	f.v[0] = 90;  f.v[1] = 114;  f.v[2] = 115;  f.flags = 65600; mesh->faces[202] = f;
	f.v[0] = 90;  f.v[1] = 115;  f.v[2] = 91;  f.flags = 65602; mesh->faces[203] = f;
	f.v[0] = 91;  f.v[1] = 115;  f.v[2] = 116;  f.flags = 65601; mesh->faces[204] = f;
	f.v[0] = 91;  f.v[1] = 116;  f.v[2] = 92;  f.flags = 65600; mesh->faces[205] = f;
	f.v[0] = 92;  f.v[1] = 116;  f.v[2] = 117;  f.flags = 65600; mesh->faces[206] = f;
	f.v[0] = 92;  f.v[1] = 117;  f.v[2] = 93;  f.flags = 65600; mesh->faces[207] = f;
	f.v[0] = 93;  f.v[1] = 117;  f.v[2] = 118;  f.flags = 65600; mesh->faces[208] = f;
	f.v[0] = 93;  f.v[1] = 118;  f.v[2] = 94;  f.flags = 65600; mesh->faces[209] = f;
	f.v[0] = 94;  f.v[1] = 118;  f.v[2] = 119;  f.flags = 65600; mesh->faces[210] = f;
	f.v[0] = 94;  f.v[1] = 119;  f.v[2] = 95;  f.flags = 65600; mesh->faces[211] = f;
	f.v[0] = 95;  f.v[1] = 119;  f.v[2] = 120;  f.flags = 65600; mesh->faces[212] = f;
	f.v[0] = 95;  f.v[1] = 120;  f.v[2] = 96;  f.flags = 65600; mesh->faces[213] = f;
	f.v[0] = 96;  f.v[1] = 120;  f.v[2] = 97;  f.flags = 65600; mesh->faces[214] = f;
	f.v[0] = 96;  f.v[1] = 97;  f.v[2] = 73;  f.flags = 65602; mesh->faces[215] = f;
	f.v[0] = 97;  f.v[1] = 121;  f.v[2] = 122;  f.flags = 65603; mesh->faces[216] = f;
	f.v[0] = 97;  f.v[1] = 122;  f.v[2] = 98;  f.flags = 65600; mesh->faces[217] = f;
	f.v[0] = 98;  f.v[1] = 122;  f.v[2] = 123;  f.flags = 65602; mesh->faces[218] = f;
	f.v[0] = 98;  f.v[1] = 123;  f.v[2] = 99;  f.flags = 65600; mesh->faces[219] = f;
	f.v[0] = 99;  f.v[1] = 123;  f.v[2] = 124;  f.flags = 65602; mesh->faces[220] = f;
	f.v[0] = 99;  f.v[1] = 124;  f.v[2] = 100;  f.flags = 65600; mesh->faces[221] = f;
	f.v[0] = 100;  f.v[1] = 124;  f.v[2] = 125;  f.flags = 65602; mesh->faces[222] = f;
	f.v[0] = 100;  f.v[1] = 125;  f.v[2] = 101;  f.flags = 65600; mesh->faces[223] = f;
	f.v[0] = 101;  f.v[1] = 125;  f.v[2] = 126;  f.flags = 65602; mesh->faces[224] = f;
	f.v[0] = 101;  f.v[1] = 126;  f.v[2] = 102;  f.flags = 65600; mesh->faces[225] = f;
	f.v[0] = 102;  f.v[1] = 126;  f.v[2] = 127;  f.flags = 65602; mesh->faces[226] = f;
	f.v[0] = 102;  f.v[1] = 127;  f.v[2] = 103;  f.flags = 65602; mesh->faces[227] = f;
	f.v[0] = 103;  f.v[1] = 127;  f.v[2] = 128;  f.flags = 65603; mesh->faces[228] = f;
	f.v[0] = 103;  f.v[1] = 128;  f.v[2] = 104;  f.flags = 65600; mesh->faces[229] = f;
	f.v[0] = 104;  f.v[1] = 128;  f.v[2] = 129;  f.flags = 65602; mesh->faces[230] = f;
	f.v[0] = 104;  f.v[1] = 129;  f.v[2] = 105;  f.flags = 65600; mesh->faces[231] = f;
	f.v[0] = 105;  f.v[1] = 129;  f.v[2] = 130;  f.flags = 65602; mesh->faces[232] = f;
	f.v[0] = 105;  f.v[1] = 130;  f.v[2] = 106;  f.flags = 65600; mesh->faces[233] = f;
	f.v[0] = 106;  f.v[1] = 130;  f.v[2] = 131;  f.flags = 65602; mesh->faces[234] = f;
	f.v[0] = 106;  f.v[1] = 131;  f.v[2] = 107;  f.flags = 65600; mesh->faces[235] = f;
	f.v[0] = 107;  f.v[1] = 131;  f.v[2] = 132;  f.flags = 65602; mesh->faces[236] = f;
	f.v[0] = 107;  f.v[1] = 132;  f.v[2] = 108;  f.flags = 65600; mesh->faces[237] = f;
	f.v[0] = 108;  f.v[1] = 132;  f.v[2] = 133;  f.flags = 65602; mesh->faces[238] = f;
	f.v[0] = 108;  f.v[1] = 133;  f.v[2] = 109;  f.flags = 65602; mesh->faces[239] = f;
	f.v[0] = 109;  f.v[1] = 133;  f.v[2] = 134;  f.flags = 65603; mesh->faces[240] = f;
	f.v[0] = 109;  f.v[1] = 134;  f.v[2] = 110;  f.flags = 65600; mesh->faces[241] = f;
	f.v[0] = 110;  f.v[1] = 134;  f.v[2] = 135;  f.flags = 65602; mesh->faces[242] = f;
	f.v[0] = 110;  f.v[1] = 135;  f.v[2] = 111;  f.flags = 65600; mesh->faces[243] = f;
	f.v[0] = 111;  f.v[1] = 135;  f.v[2] = 136;  f.flags = 65602; mesh->faces[244] = f;
	f.v[0] = 111;  f.v[1] = 136;  f.v[2] = 112;  f.flags = 65600; mesh->faces[245] = f;
	f.v[0] = 112;  f.v[1] = 136;  f.v[2] = 137;  f.flags = 65602; mesh->faces[246] = f;
	f.v[0] = 112;  f.v[1] = 137;  f.v[2] = 113;  f.flags = 65600; mesh->faces[247] = f;
	f.v[0] = 113;  f.v[1] = 137;  f.v[2] = 138;  f.flags = 65602; mesh->faces[248] = f;
	f.v[0] = 113;  f.v[1] = 138;  f.v[2] = 114;  f.flags = 65600; mesh->faces[249] = f;
	f.v[0] = 114;  f.v[1] = 138;  f.v[2] = 139;  f.flags = 65602; mesh->faces[250] = f;
	f.v[0] = 114;  f.v[1] = 139;  f.v[2] = 115;  f.flags = 65602; mesh->faces[251] = f;
	f.v[0] = 115;  f.v[1] = 139;  f.v[2] = 140;  f.flags = 65603; mesh->faces[252] = f;
	f.v[0] = 115;  f.v[1] = 140;  f.v[2] = 116;  f.flags = 65600; mesh->faces[253] = f;
	f.v[0] = 116;  f.v[1] = 140;  f.v[2] = 141;  f.flags = 65602; mesh->faces[254] = f;
	f.v[0] = 116;  f.v[1] = 141;  f.v[2] = 117;  f.flags = 65600; mesh->faces[255] = f;
	f.v[0] = 117;  f.v[1] = 141;  f.v[2] = 142;  f.flags = 65602; mesh->faces[256] = f;
	f.v[0] = 117;  f.v[1] = 142;  f.v[2] = 118;  f.flags = 65600; mesh->faces[257] = f;
	f.v[0] = 118;  f.v[1] = 142;  f.v[2] = 143;  f.flags = 65602; mesh->faces[258] = f;
	f.v[0] = 118;  f.v[1] = 143;  f.v[2] = 119;  f.flags = 65600; mesh->faces[259] = f;
	f.v[0] = 119;  f.v[1] = 143;  f.v[2] = 144;  f.flags = 65602; mesh->faces[260] = f;
	f.v[0] = 119;  f.v[1] = 144;  f.v[2] = 120;  f.flags = 65600; mesh->faces[261] = f;
	f.v[0] = 120;  f.v[1] = 144;  f.v[2] = 121;  f.flags = 65602; mesh->faces[262] = f;
	f.v[0] = 120;  f.v[1] = 121;  f.v[2] = 97;  f.flags = 65602; mesh->faces[263] = f;
	f.v[0] = 121;  f.v[1] = 145;  f.v[2] = 146;  f.flags = 65601; mesh->faces[264] = f;
	f.v[0] = 121;  f.v[1] = 146;  f.v[2] = 122;  f.flags = 65604; mesh->faces[265] = f;
	f.v[0] = 122;  f.v[1] = 146;  f.v[2] = 147;  f.flags = 65600; mesh->faces[266] = f;
	f.v[0] = 122;  f.v[1] = 147;  f.v[2] = 123;  f.flags = 65604; mesh->faces[267] = f;
	f.v[0] = 123;  f.v[1] = 147;  f.v[2] = 148;  f.flags = 65600; mesh->faces[268] = f;
	f.v[0] = 123;  f.v[1] = 148;  f.v[2] = 124;  f.flags = 65604; mesh->faces[269] = f;
	f.v[0] = 124;  f.v[1] = 148;  f.v[2] = 149;  f.flags = 65600; mesh->faces[270] = f;
	f.v[0] = 124;  f.v[1] = 149;  f.v[2] = 125;  f.flags = 65604; mesh->faces[271] = f;
	f.v[0] = 125;  f.v[1] = 149;  f.v[2] = 150;  f.flags = 65600; mesh->faces[272] = f;
	f.v[0] = 125;  f.v[1] = 150;  f.v[2] = 126;  f.flags = 65604; mesh->faces[273] = f;
	f.v[0] = 126;  f.v[1] = 150;  f.v[2] = 151;  f.flags = 65600; mesh->faces[274] = f;
	f.v[0] = 126;  f.v[1] = 151;  f.v[2] = 127;  f.flags = 65606; mesh->faces[275] = f;
	f.v[0] = 127;  f.v[1] = 151;  f.v[2] = 152;  f.flags = 65601; mesh->faces[276] = f;
	f.v[0] = 127;  f.v[1] = 152;  f.v[2] = 128;  f.flags = 65604; mesh->faces[277] = f;
	f.v[0] = 128;  f.v[1] = 152;  f.v[2] = 153;  f.flags = 65600; mesh->faces[278] = f;
	f.v[0] = 128;  f.v[1] = 153;  f.v[2] = 129;  f.flags = 65604; mesh->faces[279] = f;
	f.v[0] = 129;  f.v[1] = 153;  f.v[2] = 154;  f.flags = 65600; mesh->faces[280] = f;
	f.v[0] = 129;  f.v[1] = 154;  f.v[2] = 130;  f.flags = 65604; mesh->faces[281] = f;
	f.v[0] = 130;  f.v[1] = 154;  f.v[2] = 155;  f.flags = 65600; mesh->faces[282] = f;
	f.v[0] = 130;  f.v[1] = 155;  f.v[2] = 131;  f.flags = 65604; mesh->faces[283] = f;
	f.v[0] = 131;  f.v[1] = 155;  f.v[2] = 156;  f.flags = 65600; mesh->faces[284] = f;
	f.v[0] = 131;  f.v[1] = 156;  f.v[2] = 132;  f.flags = 65604; mesh->faces[285] = f;
	f.v[0] = 132;  f.v[1] = 156;  f.v[2] = 157;  f.flags = 65600; mesh->faces[286] = f;
	f.v[0] = 132;  f.v[1] = 157;  f.v[2] = 133;  f.flags = 65606; mesh->faces[287] = f;
	f.v[0] = 133;  f.v[1] = 157;  f.v[2] = 158;  f.flags = 65601; mesh->faces[288] = f;
	f.v[0] = 133;  f.v[1] = 158;  f.v[2] = 134;  f.flags = 65604; mesh->faces[289] = f;
	f.v[0] = 134;  f.v[1] = 158;  f.v[2] = 159;  f.flags = 65600; mesh->faces[290] = f;
	f.v[0] = 134;  f.v[1] = 159;  f.v[2] = 135;  f.flags = 65604; mesh->faces[291] = f;
	f.v[0] = 135;  f.v[1] = 159;  f.v[2] = 160;  f.flags = 65600; mesh->faces[292] = f;
	f.v[0] = 135;  f.v[1] = 160;  f.v[2] = 136;  f.flags = 65604; mesh->faces[293] = f;
	f.v[0] = 136;  f.v[1] = 160;  f.v[2] = 161;  f.flags = 65600; mesh->faces[294] = f;
	f.v[0] = 136;  f.v[1] = 161;  f.v[2] = 137;  f.flags = 65604; mesh->faces[295] = f;
	f.v[0] = 137;  f.v[1] = 161;  f.v[2] = 162;  f.flags = 65600; mesh->faces[296] = f;
	f.v[0] = 137;  f.v[1] = 162;  f.v[2] = 138;  f.flags = 65604; mesh->faces[297] = f;
	f.v[0] = 138;  f.v[1] = 162;  f.v[2] = 163;  f.flags = 65600; mesh->faces[298] = f;
	f.v[0] = 138;  f.v[1] = 163;  f.v[2] = 139;  f.flags = 65606; mesh->faces[299] = f;
	f.v[0] = 139;  f.v[1] = 163;  f.v[2] = 164;  f.flags = 65601; mesh->faces[300] = f;
	f.v[0] = 139;  f.v[1] = 164;  f.v[2] = 140;  f.flags = 65604; mesh->faces[301] = f;
	f.v[0] = 140;  f.v[1] = 164;  f.v[2] = 165;  f.flags = 65600; mesh->faces[302] = f;
	f.v[0] = 140;  f.v[1] = 165;  f.v[2] = 141;  f.flags = 65604; mesh->faces[303] = f;
	f.v[0] = 141;  f.v[1] = 165;  f.v[2] = 166;  f.flags = 65600; mesh->faces[304] = f;
	f.v[0] = 141;  f.v[1] = 166;  f.v[2] = 142;  f.flags = 65604; mesh->faces[305] = f;
	f.v[0] = 142;  f.v[1] = 166;  f.v[2] = 167;  f.flags = 65600; mesh->faces[306] = f;
	f.v[0] = 142;  f.v[1] = 167;  f.v[2] = 143;  f.flags = 65604; mesh->faces[307] = f;
	f.v[0] = 143;  f.v[1] = 167;  f.v[2] = 168;  f.flags = 65600; mesh->faces[308] = f;
	f.v[0] = 143;  f.v[1] = 168;  f.v[2] = 144;  f.flags = 65604; mesh->faces[309] = f;
	f.v[0] = 144;  f.v[1] = 168;  f.v[2] = 145;  f.flags = 65600; mesh->faces[310] = f;
	f.v[0] = 144;  f.v[1] = 145;  f.v[2] = 121;  f.flags = 65606; mesh->faces[311] = f;
	f.v[0] = 145;  f.v[1] = 169;  f.v[2] = 170;  f.flags = 65601; mesh->faces[312] = f;
	f.v[0] = 145;  f.v[1] = 170;  f.v[2] = 146;  f.flags = 65600; mesh->faces[313] = f;
	f.v[0] = 146;  f.v[1] = 170;  f.v[2] = 171;  f.flags = 65600; mesh->faces[314] = f;
	f.v[0] = 146;  f.v[1] = 171;  f.v[2] = 147;  f.flags = 65600; mesh->faces[315] = f;
	f.v[0] = 147;  f.v[1] = 171;  f.v[2] = 172;  f.flags = 65600; mesh->faces[316] = f;
	f.v[0] = 147;  f.v[1] = 172;  f.v[2] = 148;  f.flags = 65600; mesh->faces[317] = f;
	f.v[0] = 148;  f.v[1] = 172;  f.v[2] = 173;  f.flags = 65600; mesh->faces[318] = f;
	f.v[0] = 148;  f.v[1] = 173;  f.v[2] = 149;  f.flags = 65600; mesh->faces[319] = f;
	f.v[0] = 149;  f.v[1] = 173;  f.v[2] = 174;  f.flags = 65600; mesh->faces[320] = f;
	f.v[0] = 149;  f.v[1] = 174;  f.v[2] = 150;  f.flags = 65600; mesh->faces[321] = f;
	f.v[0] = 150;  f.v[1] = 174;  f.v[2] = 175;  f.flags = 65600; mesh->faces[322] = f;
	f.v[0] = 150;  f.v[1] = 175;  f.v[2] = 151;  f.flags = 65602; mesh->faces[323] = f;
	f.v[0] = 151;  f.v[1] = 175;  f.v[2] = 176;  f.flags = 65601; mesh->faces[324] = f;
	f.v[0] = 151;  f.v[1] = 176;  f.v[2] = 152;  f.flags = 65600; mesh->faces[325] = f;
	f.v[0] = 152;  f.v[1] = 176;  f.v[2] = 177;  f.flags = 65600; mesh->faces[326] = f;
	f.v[0] = 152;  f.v[1] = 177;  f.v[2] = 153;  f.flags = 65600; mesh->faces[327] = f;
	f.v[0] = 153;  f.v[1] = 177;  f.v[2] = 178;  f.flags = 65600; mesh->faces[328] = f;
	f.v[0] = 153;  f.v[1] = 178;  f.v[2] = 154;  f.flags = 65600; mesh->faces[329] = f;
	f.v[0] = 154;  f.v[1] = 178;  f.v[2] = 179;  f.flags = 65600; mesh->faces[330] = f;
	f.v[0] = 154;  f.v[1] = 179;  f.v[2] = 155;  f.flags = 65600; mesh->faces[331] = f;
	f.v[0] = 155;  f.v[1] = 179;  f.v[2] = 180;  f.flags = 65600; mesh->faces[332] = f;
	f.v[0] = 155;  f.v[1] = 180;  f.v[2] = 156;  f.flags = 65600; mesh->faces[333] = f;
	f.v[0] = 156;  f.v[1] = 180;  f.v[2] = 181;  f.flags = 65600; mesh->faces[334] = f;
	f.v[0] = 156;  f.v[1] = 181;  f.v[2] = 157;  f.flags = 65602; mesh->faces[335] = f;
	f.v[0] = 157;  f.v[1] = 181;  f.v[2] = 182;  f.flags = 65601; mesh->faces[336] = f;
	f.v[0] = 157;  f.v[1] = 182;  f.v[2] = 158;  f.flags = 65600; mesh->faces[337] = f;
	f.v[0] = 158;  f.v[1] = 182;  f.v[2] = 183;  f.flags = 65600; mesh->faces[338] = f;
	f.v[0] = 158;  f.v[1] = 183;  f.v[2] = 159;  f.flags = 65600; mesh->faces[339] = f;
	f.v[0] = 159;  f.v[1] = 183;  f.v[2] = 184;  f.flags = 65600; mesh->faces[340] = f;
	f.v[0] = 159;  f.v[1] = 184;  f.v[2] = 160;  f.flags = 65600; mesh->faces[341] = f;
	f.v[0] = 160;  f.v[1] = 184;  f.v[2] = 185;  f.flags = 65600; mesh->faces[342] = f;
	f.v[0] = 160;  f.v[1] = 185;  f.v[2] = 161;  f.flags = 65600; mesh->faces[343] = f;
	f.v[0] = 161;  f.v[1] = 185;  f.v[2] = 186;  f.flags = 65600; mesh->faces[344] = f;
	f.v[0] = 161;  f.v[1] = 186;  f.v[2] = 162;  f.flags = 65600; mesh->faces[345] = f;
	f.v[0] = 162;  f.v[1] = 186;  f.v[2] = 187;  f.flags = 65600; mesh->faces[346] = f;
	f.v[0] = 162;  f.v[1] = 187;  f.v[2] = 163;  f.flags = 65602; mesh->faces[347] = f;
	f.v[0] = 163;  f.v[1] = 187;  f.v[2] = 188;  f.flags = 65601; mesh->faces[348] = f;
	f.v[0] = 163;  f.v[1] = 188;  f.v[2] = 164;  f.flags = 65600; mesh->faces[349] = f;
	f.v[0] = 164;  f.v[1] = 188;  f.v[2] = 189;  f.flags = 65600; mesh->faces[350] = f;
	f.v[0] = 164;  f.v[1] = 189;  f.v[2] = 165;  f.flags = 65600; mesh->faces[351] = f;
	f.v[0] = 165;  f.v[1] = 189;  f.v[2] = 190;  f.flags = 65600; mesh->faces[352] = f;
	f.v[0] = 165;  f.v[1] = 190;  f.v[2] = 166;  f.flags = 65600; mesh->faces[353] = f;
	f.v[0] = 166;  f.v[1] = 190;  f.v[2] = 191;  f.flags = 65600; mesh->faces[354] = f;
	f.v[0] = 166;  f.v[1] = 191;  f.v[2] = 167;  f.flags = 65600; mesh->faces[355] = f;
	f.v[0] = 167;  f.v[1] = 191;  f.v[2] = 192;  f.flags = 65600; mesh->faces[356] = f;
	f.v[0] = 167;  f.v[1] = 192;  f.v[2] = 168;  f.flags = 65600; mesh->faces[357] = f;
	f.v[0] = 168;  f.v[1] = 192;  f.v[2] = 169;  f.flags = 65600; mesh->faces[358] = f;
	f.v[0] = 168;  f.v[1] = 169;  f.v[2] = 145;  f.flags = 65602; mesh->faces[359] = f;
	f.v[0] = 169;  f.v[1] = 193;  f.v[2] = 194;  f.flags = 65601; mesh->faces[360] = f;
	f.v[0] = 169;  f.v[1] = 194;  f.v[2] = 170;  f.flags = 65600; mesh->faces[361] = f;
	f.v[0] = 170;  f.v[1] = 194;  f.v[2] = 195;  f.flags = 65600; mesh->faces[362] = f;
	f.v[0] = 170;  f.v[1] = 195;  f.v[2] = 171;  f.flags = 65600; mesh->faces[363] = f;
	f.v[0] = 171;  f.v[1] = 195;  f.v[2] = 196;  f.flags = 65600; mesh->faces[364] = f;
	f.v[0] = 171;  f.v[1] = 196;  f.v[2] = 172;  f.flags = 65600; mesh->faces[365] = f;
	f.v[0] = 172;  f.v[1] = 196;  f.v[2] = 197;  f.flags = 65600; mesh->faces[366] = f;
	f.v[0] = 172;  f.v[1] = 197;  f.v[2] = 173;  f.flags = 65600; mesh->faces[367] = f;
	f.v[0] = 173;  f.v[1] = 197;  f.v[2] = 198;  f.flags = 65600; mesh->faces[368] = f;
	f.v[0] = 173;  f.v[1] = 198;  f.v[2] = 174;  f.flags = 65600; mesh->faces[369] = f;
	f.v[0] = 174;  f.v[1] = 198;  f.v[2] = 199;  f.flags = 65600; mesh->faces[370] = f;
	f.v[0] = 174;  f.v[1] = 199;  f.v[2] = 175;  f.flags = 65602; mesh->faces[371] = f;
	f.v[0] = 175;  f.v[1] = 199;  f.v[2] = 200;  f.flags = 65601; mesh->faces[372] = f;
	f.v[0] = 175;  f.v[1] = 200;  f.v[2] = 176;  f.flags = 65600; mesh->faces[373] = f;
	f.v[0] = 176;  f.v[1] = 200;  f.v[2] = 201;  f.flags = 65600; mesh->faces[374] = f;
	f.v[0] = 176;  f.v[1] = 201;  f.v[2] = 177;  f.flags = 65600; mesh->faces[375] = f;
	f.v[0] = 177;  f.v[1] = 201;  f.v[2] = 202;  f.flags = 65600; mesh->faces[376] = f;
	f.v[0] = 177;  f.v[1] = 202;  f.v[2] = 178;  f.flags = 65600; mesh->faces[377] = f;
	f.v[0] = 178;  f.v[1] = 202;  f.v[2] = 203;  f.flags = 65600; mesh->faces[378] = f;
	f.v[0] = 178;  f.v[1] = 203;  f.v[2] = 179;  f.flags = 65600; mesh->faces[379] = f;
	f.v[0] = 179;  f.v[1] = 203;  f.v[2] = 204;  f.flags = 65600; mesh->faces[380] = f;
	f.v[0] = 179;  f.v[1] = 204;  f.v[2] = 180;  f.flags = 65600; mesh->faces[381] = f;
	f.v[0] = 180;  f.v[1] = 204;  f.v[2] = 205;  f.flags = 65600; mesh->faces[382] = f;
	f.v[0] = 180;  f.v[1] = 205;  f.v[2] = 181;  f.flags = 65602; mesh->faces[383] = f;
	f.v[0] = 181;  f.v[1] = 205;  f.v[2] = 206;  f.flags = 65601; mesh->faces[384] = f;
	f.v[0] = 181;  f.v[1] = 206;  f.v[2] = 182;  f.flags = 65600; mesh->faces[385] = f;
	f.v[0] = 182;  f.v[1] = 206;  f.v[2] = 207;  f.flags = 65600; mesh->faces[386] = f;
	f.v[0] = 182;  f.v[1] = 207;  f.v[2] = 183;  f.flags = 65600; mesh->faces[387] = f;
	f.v[0] = 183;  f.v[1] = 207;  f.v[2] = 208;  f.flags = 65600; mesh->faces[388] = f;
	f.v[0] = 183;  f.v[1] = 208;  f.v[2] = 184;  f.flags = 65600; mesh->faces[389] = f;
	f.v[0] = 184;  f.v[1] = 208;  f.v[2] = 209;  f.flags = 65600; mesh->faces[390] = f;
	f.v[0] = 184;  f.v[1] = 209;  f.v[2] = 185;  f.flags = 65600; mesh->faces[391] = f;
	f.v[0] = 185;  f.v[1] = 209;  f.v[2] = 210;  f.flags = 65600; mesh->faces[392] = f;
	f.v[0] = 185;  f.v[1] = 210;  f.v[2] = 186;  f.flags = 65600; mesh->faces[393] = f;
	f.v[0] = 186;  f.v[1] = 210;  f.v[2] = 211;  f.flags = 65600; mesh->faces[394] = f;
	f.v[0] = 186;  f.v[1] = 211;  f.v[2] = 187;  f.flags = 65602; mesh->faces[395] = f;
	f.v[0] = 187;  f.v[1] = 211;  f.v[2] = 212;  f.flags = 65601; mesh->faces[396] = f;
	f.v[0] = 187;  f.v[1] = 212;  f.v[2] = 188;  f.flags = 65600; mesh->faces[397] = f;
	f.v[0] = 188;  f.v[1] = 212;  f.v[2] = 213;  f.flags = 65600; mesh->faces[398] = f;
	f.v[0] = 188;  f.v[1] = 213;  f.v[2] = 189;  f.flags = 65600; mesh->faces[399] = f;
	f.v[0] = 189;  f.v[1] = 213;  f.v[2] = 214;  f.flags = 65600; mesh->faces[400] = f;
	f.v[0] = 189;  f.v[1] = 214;  f.v[2] = 190;  f.flags = 65600; mesh->faces[401] = f;
	f.v[0] = 190;  f.v[1] = 214;  f.v[2] = 215;  f.flags = 65600; mesh->faces[402] = f;
	f.v[0] = 190;  f.v[1] = 215;  f.v[2] = 191;  f.flags = 65600; mesh->faces[403] = f;
	f.v[0] = 191;  f.v[1] = 215;  f.v[2] = 216;  f.flags = 65600; mesh->faces[404] = f;
	f.v[0] = 191;  f.v[1] = 216;  f.v[2] = 192;  f.flags = 65600; mesh->faces[405] = f;
	f.v[0] = 192;  f.v[1] = 216;  f.v[2] = 193;  f.flags = 65600; mesh->faces[406] = f;
	f.v[0] = 192;  f.v[1] = 193;  f.v[2] = 169;  f.flags = 65602; mesh->faces[407] = f;
	f.v[0] = 193;  f.v[1] = 217;  f.v[2] = 218;  f.flags = 65601; mesh->faces[408] = f;
	f.v[0] = 193;  f.v[1] = 218;  f.v[2] = 194;  f.flags = 65600; mesh->faces[409] = f;
	f.v[0] = 194;  f.v[1] = 218;  f.v[2] = 219;  f.flags = 65600; mesh->faces[410] = f;
	f.v[0] = 194;  f.v[1] = 219;  f.v[2] = 195;  f.flags = 65600; mesh->faces[411] = f;
	f.v[0] = 195;  f.v[1] = 219;  f.v[2] = 220;  f.flags = 65600; mesh->faces[412] = f;
	f.v[0] = 195;  f.v[1] = 220;  f.v[2] = 196;  f.flags = 65600; mesh->faces[413] = f;
	f.v[0] = 196;  f.v[1] = 220;  f.v[2] = 221;  f.flags = 65600; mesh->faces[414] = f;
	f.v[0] = 196;  f.v[1] = 221;  f.v[2] = 197;  f.flags = 65600; mesh->faces[415] = f;
	f.v[0] = 197;  f.v[1] = 221;  f.v[2] = 222;  f.flags = 65600; mesh->faces[416] = f;
	f.v[0] = 197;  f.v[1] = 222;  f.v[2] = 198;  f.flags = 65600; mesh->faces[417] = f;
	f.v[0] = 198;  f.v[1] = 222;  f.v[2] = 223;  f.flags = 65600; mesh->faces[418] = f;
	f.v[0] = 198;  f.v[1] = 223;  f.v[2] = 199;  f.flags = 65602; mesh->faces[419] = f;
	f.v[0] = 199;  f.v[1] = 223;  f.v[2] = 224;  f.flags = 65601; mesh->faces[420] = f;
	f.v[0] = 199;  f.v[1] = 224;  f.v[2] = 200;  f.flags = 65600; mesh->faces[421] = f;
	f.v[0] = 200;  f.v[1] = 224;  f.v[2] = 225;  f.flags = 65600; mesh->faces[422] = f;
	f.v[0] = 200;  f.v[1] = 225;  f.v[2] = 201;  f.flags = 65600; mesh->faces[423] = f;
	f.v[0] = 201;  f.v[1] = 225;  f.v[2] = 226;  f.flags = 65600; mesh->faces[424] = f;
	f.v[0] = 201;  f.v[1] = 226;  f.v[2] = 202;  f.flags = 65600; mesh->faces[425] = f;
	f.v[0] = 202;  f.v[1] = 226;  f.v[2] = 227;  f.flags = 65600; mesh->faces[426] = f;
	f.v[0] = 202;  f.v[1] = 227;  f.v[2] = 203;  f.flags = 65600; mesh->faces[427] = f;
	f.v[0] = 203;  f.v[1] = 227;  f.v[2] = 228;  f.flags = 65600; mesh->faces[428] = f;
	f.v[0] = 203;  f.v[1] = 228;  f.v[2] = 204;  f.flags = 65600; mesh->faces[429] = f;
	f.v[0] = 204;  f.v[1] = 228;  f.v[2] = 229;  f.flags = 65600; mesh->faces[430] = f;
	f.v[0] = 204;  f.v[1] = 229;  f.v[2] = 205;  f.flags = 65602; mesh->faces[431] = f;
	f.v[0] = 205;  f.v[1] = 229;  f.v[2] = 230;  f.flags = 65601; mesh->faces[432] = f;
	f.v[0] = 205;  f.v[1] = 230;  f.v[2] = 206;  f.flags = 65600; mesh->faces[433] = f;
	f.v[0] = 206;  f.v[1] = 230;  f.v[2] = 231;  f.flags = 65600; mesh->faces[434] = f;
	f.v[0] = 206;  f.v[1] = 231;  f.v[2] = 207;  f.flags = 65600; mesh->faces[435] = f;
	f.v[0] = 207;  f.v[1] = 231;  f.v[2] = 232;  f.flags = 65600; mesh->faces[436] = f;
	f.v[0] = 207;  f.v[1] = 232;  f.v[2] = 208;  f.flags = 65600; mesh->faces[437] = f;
	f.v[0] = 208;  f.v[1] = 232;  f.v[2] = 233;  f.flags = 65600; mesh->faces[438] = f;
	f.v[0] = 208;  f.v[1] = 233;  f.v[2] = 209;  f.flags = 65600; mesh->faces[439] = f;
	f.v[0] = 209;  f.v[1] = 233;  f.v[2] = 234;  f.flags = 65600; mesh->faces[440] = f;
	f.v[0] = 209;  f.v[1] = 234;  f.v[2] = 210;  f.flags = 65600; mesh->faces[441] = f;
	f.v[0] = 210;  f.v[1] = 234;  f.v[2] = 235;  f.flags = 65600; mesh->faces[442] = f;
	f.v[0] = 210;  f.v[1] = 235;  f.v[2] = 211;  f.flags = 65602; mesh->faces[443] = f;
	f.v[0] = 211;  f.v[1] = 235;  f.v[2] = 236;  f.flags = 65601; mesh->faces[444] = f;
	f.v[0] = 211;  f.v[1] = 236;  f.v[2] = 212;  f.flags = 65600; mesh->faces[445] = f;
	f.v[0] = 212;  f.v[1] = 236;  f.v[2] = 237;  f.flags = 65600; mesh->faces[446] = f;
	f.v[0] = 212;  f.v[1] = 237;  f.v[2] = 213;  f.flags = 65600; mesh->faces[447] = f;
	f.v[0] = 213;  f.v[1] = 237;  f.v[2] = 238;  f.flags = 65600; mesh->faces[448] = f;
	f.v[0] = 213;  f.v[1] = 238;  f.v[2] = 214;  f.flags = 65600; mesh->faces[449] = f;
	f.v[0] = 214;  f.v[1] = 238;  f.v[2] = 239;  f.flags = 65600; mesh->faces[450] = f;
	f.v[0] = 214;  f.v[1] = 239;  f.v[2] = 215;  f.flags = 65600; mesh->faces[451] = f;
	f.v[0] = 215;  f.v[1] = 239;  f.v[2] = 240;  f.flags = 65600; mesh->faces[452] = f;
	f.v[0] = 215;  f.v[1] = 240;  f.v[2] = 216;  f.flags = 65600; mesh->faces[453] = f;
	f.v[0] = 216;  f.v[1] = 240;  f.v[2] = 217;  f.flags = 65600; mesh->faces[454] = f;
	f.v[0] = 216;  f.v[1] = 217;  f.v[2] = 193;  f.flags = 65602; mesh->faces[455] = f;
	f.v[0] = 217;  f.v[1] = 241;  f.v[2] = 242;  f.flags = 65601; mesh->faces[456] = f;
	f.v[0] = 217;  f.v[1] = 242;  f.v[2] = 218;  f.flags = 65600; mesh->faces[457] = f;
	f.v[0] = 218;  f.v[1] = 242;  f.v[2] = 243;  f.flags = 65600; mesh->faces[458] = f;
	f.v[0] = 218;  f.v[1] = 243;  f.v[2] = 219;  f.flags = 65600; mesh->faces[459] = f;
	f.v[0] = 219;  f.v[1] = 243;  f.v[2] = 244;  f.flags = 65600; mesh->faces[460] = f;
	f.v[0] = 219;  f.v[1] = 244;  f.v[2] = 220;  f.flags = 65600; mesh->faces[461] = f;
	f.v[0] = 220;  f.v[1] = 244;  f.v[2] = 245;  f.flags = 65600; mesh->faces[462] = f;
	f.v[0] = 220;  f.v[1] = 245;  f.v[2] = 221;  f.flags = 65600; mesh->faces[463] = f;
	f.v[0] = 221;  f.v[1] = 245;  f.v[2] = 246;  f.flags = 65600; mesh->faces[464] = f;
	f.v[0] = 221;  f.v[1] = 246;  f.v[2] = 222;  f.flags = 65600; mesh->faces[465] = f;
	f.v[0] = 222;  f.v[1] = 246;  f.v[2] = 247;  f.flags = 65600; mesh->faces[466] = f;
	f.v[0] = 222;  f.v[1] = 247;  f.v[2] = 223;  f.flags = 65602; mesh->faces[467] = f;
	f.v[0] = 223;  f.v[1] = 247;  f.v[2] = 248;  f.flags = 65601; mesh->faces[468] = f;
	f.v[0] = 223;  f.v[1] = 248;  f.v[2] = 224;  f.flags = 65600; mesh->faces[469] = f;
	f.v[0] = 224;  f.v[1] = 248;  f.v[2] = 249;  f.flags = 65600; mesh->faces[470] = f;
	f.v[0] = 224;  f.v[1] = 249;  f.v[2] = 225;  f.flags = 65600; mesh->faces[471] = f;
	f.v[0] = 225;  f.v[1] = 249;  f.v[2] = 250;  f.flags = 65600; mesh->faces[472] = f;
	f.v[0] = 225;  f.v[1] = 250;  f.v[2] = 226;  f.flags = 65600; mesh->faces[473] = f;
	f.v[0] = 226;  f.v[1] = 250;  f.v[2] = 251;  f.flags = 65600; mesh->faces[474] = f;
	f.v[0] = 226;  f.v[1] = 251;  f.v[2] = 227;  f.flags = 65600; mesh->faces[475] = f;
	f.v[0] = 227;  f.v[1] = 251;  f.v[2] = 252;  f.flags = 65600; mesh->faces[476] = f;
	f.v[0] = 227;  f.v[1] = 252;  f.v[2] = 228;  f.flags = 65600; mesh->faces[477] = f;
	f.v[0] = 228;  f.v[1] = 252;  f.v[2] = 253;  f.flags = 65600; mesh->faces[478] = f;
	f.v[0] = 228;  f.v[1] = 253;  f.v[2] = 229;  f.flags = 65602; mesh->faces[479] = f;
	f.v[0] = 229;  f.v[1] = 253;  f.v[2] = 254;  f.flags = 65601; mesh->faces[480] = f;
	f.v[0] = 229;  f.v[1] = 254;  f.v[2] = 230;  f.flags = 65600; mesh->faces[481] = f;
	f.v[0] = 230;  f.v[1] = 254;  f.v[2] = 255;  f.flags = 65600; mesh->faces[482] = f;
	f.v[0] = 230;  f.v[1] = 255;  f.v[2] = 231;  f.flags = 65600; mesh->faces[483] = f;
	f.v[0] = 231;  f.v[1] = 255;  f.v[2] = 256;  f.flags = 65600; mesh->faces[484] = f;
	f.v[0] = 231;  f.v[1] = 256;  f.v[2] = 232;  f.flags = 65600; mesh->faces[485] = f;
	f.v[0] = 232;  f.v[1] = 256;  f.v[2] = 257;  f.flags = 65600; mesh->faces[486] = f;
	f.v[0] = 232;  f.v[1] = 257;  f.v[2] = 233;  f.flags = 65600; mesh->faces[487] = f;
	f.v[0] = 233;  f.v[1] = 257;  f.v[2] = 258;  f.flags = 65600; mesh->faces[488] = f;
	f.v[0] = 233;  f.v[1] = 258;  f.v[2] = 234;  f.flags = 65600; mesh->faces[489] = f;
	f.v[0] = 234;  f.v[1] = 258;  f.v[2] = 259;  f.flags = 65600; mesh->faces[490] = f;
	f.v[0] = 234;  f.v[1] = 259;  f.v[2] = 235;  f.flags = 65602; mesh->faces[491] = f;
	f.v[0] = 235;  f.v[1] = 259;  f.v[2] = 260;  f.flags = 65601; mesh->faces[492] = f;
	f.v[0] = 235;  f.v[1] = 260;  f.v[2] = 236;  f.flags = 65600; mesh->faces[493] = f;
	f.v[0] = 236;  f.v[1] = 260;  f.v[2] = 261;  f.flags = 65600; mesh->faces[494] = f;
	f.v[0] = 236;  f.v[1] = 261;  f.v[2] = 237;  f.flags = 65600; mesh->faces[495] = f;
	f.v[0] = 237;  f.v[1] = 261;  f.v[2] = 262;  f.flags = 65600; mesh->faces[496] = f;
	f.v[0] = 237;  f.v[1] = 262;  f.v[2] = 238;  f.flags = 65600; mesh->faces[497] = f;
	f.v[0] = 238;  f.v[1] = 262;  f.v[2] = 263;  f.flags = 65600; mesh->faces[498] = f;
	f.v[0] = 238;  f.v[1] = 263;  f.v[2] = 239;  f.flags = 65600; mesh->faces[499] = f;
	f.v[0] = 239;  f.v[1] = 263;  f.v[2] = 264;  f.flags = 65600; mesh->faces[500] = f;
	f.v[0] = 239;  f.v[1] = 264;  f.v[2] = 240;  f.flags = 65600; mesh->faces[501] = f;
	f.v[0] = 240;  f.v[1] = 264;  f.v[2] = 241;  f.flags = 65600; mesh->faces[502] = f;
	f.v[0] = 240;  f.v[1] = 241;  f.v[2] = 217;  f.flags = 65602; mesh->faces[503] = f;
	f.v[0] = 265;  f.v[1] = 242;  f.v[2] = 241;  f.flags = 65604; mesh->faces[504] = f;
	f.v[0] = 265;  f.v[1] = 243;  f.v[2] = 242;  f.flags = 65600; mesh->faces[505] = f;
	f.v[0] = 265;  f.v[1] = 244;  f.v[2] = 243;  f.flags = 65600; mesh->faces[506] = f;
	f.v[0] = 265;  f.v[1] = 245;  f.v[2] = 244;  f.flags = 65600; mesh->faces[507] = f;
	f.v[0] = 265;  f.v[1] = 246;  f.v[2] = 245;  f.flags = 65600; mesh->faces[508] = f;
	f.v[0] = 265;  f.v[1] = 247;  f.v[2] = 246;  f.flags = 65601; mesh->faces[509] = f;
	f.v[0] = 265;  f.v[1] = 248;  f.v[2] = 247;  f.flags = 65604; mesh->faces[510] = f;
	f.v[0] = 265;  f.v[1] = 249;  f.v[2] = 248;  f.flags = 65600; mesh->faces[511] = f;
	f.v[0] = 265;  f.v[1] = 250;  f.v[2] = 249;  f.flags = 65600; mesh->faces[512] = f;
	f.v[0] = 265;  f.v[1] = 251;  f.v[2] = 250;  f.flags = 65600; mesh->faces[513] = f;
	f.v[0] = 265;  f.v[1] = 252;  f.v[2] = 251;  f.flags = 65600; mesh->faces[514] = f;
	f.v[0] = 265;  f.v[1] = 253;  f.v[2] = 252;  f.flags = 65601; mesh->faces[515] = f;
	f.v[0] = 265;  f.v[1] = 254;  f.v[2] = 253;  f.flags = 65604; mesh->faces[516] = f;
	f.v[0] = 265;  f.v[1] = 255;  f.v[2] = 254;  f.flags = 65600; mesh->faces[517] = f;
	f.v[0] = 265;  f.v[1] = 256;  f.v[2] = 255;  f.flags = 65600; mesh->faces[518] = f;
	f.v[0] = 265;  f.v[1] = 257;  f.v[2] = 256;  f.flags = 65600; mesh->faces[519] = f;
	f.v[0] = 265;  f.v[1] = 258;  f.v[2] = 257;  f.flags = 65600; mesh->faces[520] = f;
	f.v[0] = 265;  f.v[1] = 259;  f.v[2] = 258;  f.flags = 65601; mesh->faces[521] = f;
	f.v[0] = 265;  f.v[1] = 260;  f.v[2] = 259;  f.flags = 65604; mesh->faces[522] = f;
	f.v[0] = 265;  f.v[1] = 261;  f.v[2] = 260;  f.flags = 65600; mesh->faces[523] = f;
	f.v[0] = 265;  f.v[1] = 262;  f.v[2] = 261;  f.flags = 65600; mesh->faces[524] = f;
	f.v[0] = 265;  f.v[1] = 263;  f.v[2] = 262;  f.flags = 65600; mesh->faces[525] = f;
	f.v[0] = 265;  f.v[1] = 264;  f.v[2] = 263;  f.flags = 65600; mesh->faces[526] = f;
	f.v[0] = 265;  f.v[1] = 241;  f.v[2] = 264;  f.flags = 65601; mesh->faces[527] = f;
}

//+--------------------------------------------------------------------------+
//|							From PFActionIcon								 |
//+--------------------------------------------------------------------------+
void PFActionIcon::BuildLogoArrowOut( PolyShape* shape )
{
	enum {	logo_lines = 3,
			num_points = 8 
	};
	static int lineNum[logo_lines] = { 2, 5, num_points };

	static Point3 LogoContur[ num_points ] = {

	Point3( 0.0f, 0.0f, -0.5f), Point3( 0.0f, 0.0f, 0.0f),

	Point3(  0.070730f, 0.0f, -0.355498f), Point3(  0.0f, 0.0f, -0.5f), Point3( -0.070730f, 0.0f, -0.355498f),

	Point3( 0.0f, -0.070731f, -0.355498f), Point3( 0.0f, 0.0f, -0.5f), Point3( 0.0f, 0.070730f, -0.355498f),

	};

	shape->NewShape();
	int pointIndex = 0;
	for (register lineIndex = 0; lineIndex < logo_lines; lineIndex++ )
	{
		PolyLine* line = shape->NewLine();
		if ( line == NULL ) return;
		while ( pointIndex < lineNum[ lineIndex ] )
			line->Append( PolyPt( LogoContur[ pointIndex++], POLYPT_KNOT ) );
		line->Open();
	}
}

//+--------------------------------------------------------------------------+
//|							From PFActionIcon								 |
//+--------------------------------------------------------------------------+
void PFActionIcon::BuildLogo4ArrowsInParallel( PolyShape* shape)
{
	enum {	logo_lines = 8,
			num_points = 20 
	};
	static int lineNum[logo_lines] = { 2, 5, 7, 10, 12, 15, 17, num_points };

	static Point3 LogoContur[ num_points ] = {

	Point3( 0.25f, 0.25f, 0.0f), Point3( 0.25f, 0.25f, 0.5f),

	Point3( 0.32f, 0.32f, 0.15f), Point3(  0.25f, 0.25f, 0.0f), Point3( 0.18f, 0.18f, 0.15f),
	
	Point3( -0.25f, 0.25f, 0.0f), Point3( -0.25f, 0.25f, 0.5f),

	Point3( -0.32f, 0.32f, 0.15f), Point3(  -0.25f, 0.25f, 0.0f), Point3( -0.18f, 0.18f, 0.15f),
		
	Point3( 0.25f, -0.25f, 0.0f), Point3( 0.25f, -0.25f, 0.5f),

	Point3( 0.32f, -0.32f, 0.15f), Point3(  0.25f, -0.25f, 0.0f), Point3( 0.18f, -0.18f, 0.15f),

	Point3( -0.25f, -0.25f, 0.0f), Point3( -0.25f, -0.25f, 0.5f),

	Point3( -0.32f, -0.32f, 0.15f), Point3(  -0.25f, -0.25f, 0.0f), Point3( -0.18f, -0.18f, 0.15f),

	};

	shape->NewShape();
	int pointIndex = 0;
	for (register lineIndex = 0; lineIndex < logo_lines; lineIndex++ )
	{
		PolyLine* line = shape->NewLine();
		if ( line == NULL ) return;
		while ( pointIndex < lineNum[ lineIndex ] )
			line->Append( PolyPt( LogoContur[ pointIndex++], POLYPT_KNOT ) );
		line->Open();
	}
}

//+--------------------------------------------------------------------------+
//|							From PFActionIcon								 |
//+--------------------------------------------------------------------------+
void PFActionIcon::BuildLogo4ArrowsOutParallel( PolyShape* shape)
{
	enum {	logo_lines = 8,
			num_points = 20 
	};
	static int lineNum[logo_lines] = { 2, 5, 7, 10, 12, 15, 17, num_points };

	static Point3 LogoContur[ num_points ] = {

	Point3( 0.25f, 0.25f, -0.5f), Point3( 0.25f, 0.25f, 0.0f),

	Point3( 0.32f, 0.32f, -0.35f), Point3(  0.25f, 0.25f, -0.5f), Point3( 0.18f, 0.18f, -0.35f),
	
	Point3( -0.25f, 0.25f, -0.5f), Point3( -0.25f, 0.25f, 0.0f),

	Point3( -0.32f, 0.18f, -0.35f), Point3(  -0.25f, 0.25f, -0.5f), Point3( -0.18f, 0.32f, -0.35f),
		
	Point3( 0.25f, -0.25f, -0.5f), Point3( 0.25f, -0.25f, 0.0f),

	Point3( 0.32f, -0.18f, -0.35f), Point3(  0.25f, -0.25f, -0.5f), Point3( 0.18f, -0.32f, -0.35f),

	Point3( -0.25f, -0.25f, -0.5f), Point3( -0.25f, -0.25f, 0.0f),

	Point3( -0.32f, -0.32f, -0.35f), Point3(  -0.25f, -0.25f, -0.5f), Point3( -0.18f, -0.18f, -0.35f),

	};

	shape->NewShape();
	int pointIndex = 0;
	for (register lineIndex = 0; lineIndex < logo_lines; lineIndex++ )
	{
		PolyLine* line = shape->NewLine();
		if ( line == NULL ) return;
		while ( pointIndex < lineNum[ lineIndex ] )
			line->Append( PolyPt( LogoContur[ pointIndex++], POLYPT_KNOT ) );
		line->Open();
	}
}

//+--------------------------------------------------------------------------+
//|							From PFActionIcon								 |
//+--------------------------------------------------------------------------+
void PFActionIcon::BuildLogo4ArrowsInCircle( PolyShape* shape)
{
	enum {	logo_lines = 8,
			num_points = 20 
	};
	static int lineNum[logo_lines] = { 2, 5, 7, 10, 12, 15, 17, num_points };

	static Point3 LogoContur[ num_points ] = {

	Point3( -0.5f, 0.0f, 0.0f), Point3( -0.1f, 0.0f, 0.0f),

	Point3( -0.2f, -0.1f, 0.0f), Point3( -0.1f, 0.0f, 0.0f), Point3( -0.2f, 0.1f, 0.0f),
	
	Point3( 0.5f, 0.0f, 0.0f), Point3( 0.1f, 0.0f, 0.0f),

	Point3( 0.2f, 0.1f, 0.0f), Point3( 0.1f, 0.0f, 0.0f), Point3( 0.2f, -0.1f, 0.0f),
		
	Point3( 0.0f, -0.5f, 0.0f), Point3( 0.0f, -0.1f, 0.0f),

	Point3( -0.1f, -0.2f, 0.0f), Point3( 0.0f, -0.1f, 0.0f), Point3( 0.1f, -0.2f, 0.0f),

	Point3( 0.0f, 0.5f, 0.0f), Point3( 0.0f, 0.1f, 0.0f),

	Point3( 0.1f, 0.2f, 0.0f), Point3( 0.0f, 0.1f, 0.0f), Point3( -0.1f, 0.2f, 0.0f),

	};

	shape->NewShape();
	int pointIndex = 0;
	for (register lineIndex = 0; lineIndex < logo_lines; lineIndex++ )
	{
		PolyLine* line = shape->NewLine();
		if ( line == NULL ) return;
		while ( pointIndex < lineNum[ lineIndex ] )
			line->Append( PolyPt( LogoContur[ pointIndex++], POLYPT_KNOT ) );
		line->Open();
	}
}

//+--------------------------------------------------------------------------+
//|							From PFActionIcon								 |
//+--------------------------------------------------------------------------+
void PFActionIcon::BuildLogo4ArrowsOutCircle( PolyShape* shape)
{
	enum {	logo_lines = 8,
			num_points = 20 
	};
	static int lineNum[logo_lines] = { 2, 5, 7, 10, 12, 15, 17, num_points };

	static Point3 LogoContur[ num_points ] = {

	Point3( -0.5f, 0.0f, 0.0f), Point3( -0.1f, 0.0f, 0.0f),

	Point3( -0.4f, -0.1f, 0.0f), Point3( -0.5f, 0.0f, 0.0f), Point3( -0.4f, 0.1f, 0.0f),
	
	Point3( 0.5f, 0.0f, 0.0f), Point3( 0.1f, 0.0f, 0.0f),

	Point3( 0.4f, 0.1f, 0.0f), Point3( 0.5f, 0.0f, 0.0f), Point3( 0.4f, -0.1f, 0.0f),
		
	Point3( 0.0f, -0.5f, 0.0f), Point3( 0.0f, -0.1f, 0.0f),

	Point3( -0.1f, -0.4f, 0.0f), Point3( 0.0f, -0.5f, 0.0f), Point3( 0.1f, -0.4f, 0.0f),

	Point3( 0.0f, 0.5f, 0.0f), Point3( 0.0f, 0.1f, 0.0f),

	Point3( 0.1f, 0.4f, 0.0f), Point3( 0.0f, 0.5f, 0.0f), Point3( -0.1f, 0.4f, 0.0f),

	};

	shape->NewShape();
	int pointIndex = 0;
	for (register lineIndex = 0; lineIndex < logo_lines; lineIndex++ )
	{
		PolyLine* line = shape->NewLine();
		if ( line == NULL ) return;
		while ( pointIndex < lineNum[ lineIndex ] )
			line->Append( PolyPt( LogoContur[ pointIndex++], POLYPT_KNOT ) );
		line->Open();
	}
}

//+--------------------------------------------------------------------------+
//|							From PFActionIcon								 |
//+--------------------------------------------------------------------------+
void PFActionIcon::BuildLogo6ArrowsInSphere( PolyShape* shape)
{
	enum {	logo_lines = 12,
			num_points = 30 
	};
	static int lineNum[logo_lines] = { 2, 5, 7, 10, 12, 15, 17, 20, 22, 25, 27, num_points };

	static Point3 LogoContur[ num_points ] = {

	Point3( -0.5f, 0.0f, 0.0f), Point3( -0.1f, 0.0f, 0.0f),

	Point3( -0.2f, -0.1f, 0.0f), Point3( -0.1f, 0.0f, 0.0f), Point3( -0.2f, 0.1f, 0.0f),
	
	Point3( 0.5f, 0.0f, 0.0f), Point3( 0.1f, 0.0f, 0.0f),

	Point3( 0.2f, 0.1f, 0.0f), Point3( 0.1f, 0.0f, 0.0f), Point3( 0.2f, -0.1f, 0.0f),
		
	Point3( 0.0f, -0.5f, 0.0f), Point3( 0.0f, -0.1f, 0.0f),

	Point3( 0.0f, -0.2f, -0.1f), Point3( 0.0f, -0.1f, 0.0f), Point3( 0.0f, -0.2f, 0.1f),

	Point3( 0.0f, 0.5f, 0.0f), Point3( 0.0f, 0.1f, 0.0f),

	Point3( 0.0f, 0.2f, 0.1f), Point3( 0.0f, 0.1f, 0.0f), Point3( 0.0f, 0.2f, -0.1f),

	Point3( 0.0f, 0.0f, -0.5f), Point3( 0.0f, 0.0f, -0.1f),

	Point3( -0.1f, 0.0f, -0.2f), Point3( 0.0f, 0.0f, -0.1f), Point3( 0.1f, 0.0f, -0.2f),
	
	Point3( 0.0f, 0.0f, 0.5f), Point3( 0.0f, 0.0f, 0.1f),

	Point3( 0.1f, 0.0f, 0.2f), Point3( 0.0f, 0.0f, 0.1f), Point3( -0.1f, 0.0f, 0.2f),

	};

	shape->NewShape();
	int pointIndex = 0;
	for (register lineIndex = 0; lineIndex < logo_lines; lineIndex++ )
	{
		PolyLine* line = shape->NewLine();
		if ( line == NULL ) return;
		while ( pointIndex < lineNum[ lineIndex ] )
			line->Append( PolyPt( LogoContur[ pointIndex++], POLYPT_KNOT ) );
		line->Open();
	}
}

//+--------------------------------------------------------------------------+
//|							From PFActionIcon								 |
//+--------------------------------------------------------------------------+
void PFActionIcon::BuildLogo6ArrowsOutSphere( PolyShape* shape)
{
	enum {	logo_lines = 12,
			num_points = 30 
	};
	static int lineNum[logo_lines] = { 2, 5, 7, 10, 12, 15, 17, 20, 22, 25, 27, num_points };

	static Point3 LogoContur[ num_points ] = {

	Point3( -0.5f, 0.0f, 0.0f), Point3( -0.1f, 0.0f, 0.0f),

	Point3( -0.4f, -0.1f, 0.0f), Point3( -0.5f, 0.0f, 0.0f), Point3( -0.4f, 0.1f, 0.0f),
	
	Point3( 0.5f, 0.0f, 0.0f), Point3( 0.1f, 0.0f, 0.0f),

	Point3( 0.4f, 0.1f, 0.0f), Point3( 0.5f, 0.0f, 0.0f), Point3( 0.4f, -0.1f, 0.0f),
		
	Point3( 0.0f, -0.5f, 0.0f), Point3( 0.0f, -0.1f, 0.0f),

	Point3( 0.0f, -0.4f, -0.1f), Point3( 0.0f, -0.5f, 0.0f), Point3( 0.0f, -0.4f, 0.1f),

	Point3( 0.0f, 0.5f, 0.0f), Point3( 0.0f, 0.1f, 0.0f),

	Point3( 0.0f, 0.4f, 0.1f), Point3( 0.0f, 0.5f, 0.0f), Point3( 0.0f, 0.4f, -0.1f),

	Point3( 0.0f, 0.0f, -0.5f), Point3( 0.0f, 0.0f, -0.1f),

	Point3( -0.1f, 0.0f, -0.4f), Point3( 0.0f, 0.0f, -0.5f), Point3( 0.1f, 0.0f, -0.4f),
	
	Point3( 0.0f, 0.0f, 0.5f), Point3( 0.0f, 0.0f, 0.1f),

	Point3( 0.1f, 0.0f, 0.4f), Point3( 0.0f, 0.0f, 0.5f), Point3( -0.1f, 0.0f, 0.4f),

	};

	shape->NewShape();
	int pointIndex = 0;
	for (register lineIndex = 0; lineIndex < logo_lines; lineIndex++ )
	{
		PolyLine* line = shape->NewLine();
		if ( line == NULL ) return;
		while ( pointIndex < lineNum[ lineIndex ] )
			line->Append( PolyPt( LogoContur[ pointIndex++], POLYPT_KNOT ) );
		line->Open();
	}
}

void PFActionIcon::BuildLogo12ArrowsInSphereSurface( PolyShape* shape)
{
	enum {	logo_lines = 12,
			num_points = 60 
	};
	static int lineNum[logo_lines] = { 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, num_points };

	static Point3 LogoContur[ num_points ] = {

	Point3(0.5f,0.5f,0.0f), Point3(0.36f,0.36f,0.0f), Point3(0.36f,0.43f,0.0f), Point3(0.36f,0.36f,0.0f), Point3(0.43f,0.36f,0.0f),

	Point3(-0.5f,0.5f,0.0f), Point3(-0.36f,0.36f,0.0f), Point3(-0.36f,0.43f,0.0f), Point3(-0.36f,0.36f,0.0f), Point3(-0.43f,0.36f,0.0f),

	Point3(0.5f,-0.5f,0.0f), Point3(0.36f,-0.36f,0.0f), Point3(0.36f,-0.43f,0.0f), Point3(0.36f,-0.36f,0.0f), Point3(0.43f,-0.36f,0.0f),

	Point3(-0.5f,-0.5f,0.0f), Point3(-0.36f,-0.36f,0.0f), Point3(-0.36f,-0.43f,0.0f), Point3(-0.36f,-0.36f,0.0f), Point3(-0.43f,-0.36f,0.0f),


	Point3(0.0f,0.5f,0.5f), Point3(0.0f,0.36f,0.36f), Point3(0.0f,0.43f,0.36f), Point3(0.0f,0.36f,0.36f), Point3(0.0f,0.36f,0.43f),

	Point3(0.0f,0.5f,-0.5f), Point3(0.0f,0.36f,-0.36f), Point3(0.0f,0.43f,-0.36f), Point3(0.0f,0.36f,-0.36f), Point3(0.0f,0.36f,-0.43f),

	Point3(0.0f,-0.5f,0.5f), Point3(0.0f,-0.36f,0.36f), Point3(0.0f,-0.43f,0.36f), Point3(0.0f,-0.36f,0.36f), Point3(0.0f,-0.36f,0.43f),

	Point3(0.0f,-0.5f,-0.5f), Point3(0.0f,-0.36f,-0.36f), Point3(0.0f,-0.43f,-0.36f), Point3(0.0f,-0.36f,-0.36f), Point3(0.0f,-0.36f,-0.43f),


	Point3(0.5f,0.0f,0.5f), Point3(0.36f,0.0f,0.36f), Point3(0.36f,0.0f,0.43f), Point3(0.36f,0.0f,0.36f), Point3(0.43f,0.0f,0.36f),

	Point3(-0.5f,0.0f,0.5f), Point3(-0.36f,0.0f,0.36f), Point3(-0.36f,0.0f,0.43f), Point3(-0.36f,0.0f,0.36f), Point3(-0.43f,0.0f,0.36f),

	Point3(0.5f,0.0f,-0.5f), Point3(0.36f,0.0f,-0.36f), Point3(0.36f,0.0f,-0.43f), Point3(0.36f,0.0f,-0.36f), Point3(0.43f,0.0f,-0.36f),

	Point3(-0.5f,0.0f,-0.5f), Point3(-0.36f,0.0f,-0.36f), Point3(-0.36f,0.0f,-0.43f), Point3(-0.36f,0.0f,-0.36f), Point3(-0.43f,0.0f,-0.36f),

	};

	shape->NewShape();
	int pointIndex = 0;
	for (register lineIndex = 0; lineIndex < logo_lines; lineIndex++ )
	{
		PolyLine* line = shape->NewLine();
		if ( line == NULL ) return;
		while ( pointIndex < lineNum[ lineIndex ] )
			line->Append( PolyPt( LogoContur[ pointIndex++], POLYPT_KNOT ) );
		line->Open();
	}
}




} // end of namespace PFActions