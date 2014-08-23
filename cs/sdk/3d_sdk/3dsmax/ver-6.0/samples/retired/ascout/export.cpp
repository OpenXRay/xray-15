//************************************************************************** 
//* Export.cpp	- 3D Studio DOS : ASC File Exporter
//* 
//* By Christer Janson
//* Kinetix Development
//*
//* April 03, 1998 CCJ Initial coding
//*
//* This module contains the main export functions.
//*
//* Copyright (c) 1998, All Rights Reserved. 
//***************************************************************************

#include "ascout.h"
#include "buildver.h"

/****************************************************************************

  Global output [Scene info]
  
****************************************************************************/

// Dump some global animation information.
void AscOut::ExportGlobalInfo()
{
	int	i;
	Texmap* bgmap;

	// Ambient light
	Color ambLight = ip->GetAmbient(GetStaticFrame(), FOREVER);
	fprintf(pStream,"Ambient light color: Red=%s Green=%s Blue=%s\n",
						Format(ambLight.r),
						Format(ambLight.g),
						Format(ambLight.b));

	// Background
	bgmap = ip->GetEnvironmentMap();
	if (!bgmap || !ip->GetUseEnvironmentMap()) {
		// Solid color
		Color bg = ip->GetBackGround(GetStaticFrame(), FOREVER);
		if (MaxVal(bg) > 0.0f) {
			fprintf(pStream,"Solid background color: Red=%s Green=%s Blue=%s\n",
							Format(bg.r),
							Format(bg.g),
							Format(bg.b));
		}
	}
	else if (bgmap) {
		if (bgmap->ClassID() == Class_ID(BMTEX_CLASS_ID, 0)) {
			// Bitmap
			BitmapTex* bmtex = (BitmapTex*)bgmap;
			if (bmtex->GetMapName()) {
				fprintf(pStream,"Background image: \"%s\"\n", bmtex->GetMapName());
			}
		}
#ifndef NO_MAPTYPE_GRADIENT // orb 01-07-2001
		else if (bgmap->ClassID() == Class_ID(GRADIENT_CLASS_ID, 0)) {
			// Gradient
			IParamBlock* pb = (IParamBlock*)bgmap->GetReference(1);
			if (pb) {
				float	midPoint;
				Color	cols[3];
				pb->GetValue(7, GetStaticFrame(), midPoint, FOREVER);
				pb->GetValue(0, GetStaticFrame(), cols[0], FOREVER);
				pb->GetValue(1, GetStaticFrame(), cols[1], FOREVER);
				pb->GetValue(2, GetStaticFrame(), cols[2], FOREVER);

				fprintf(pStream,"Gradient top color: Red=%s Green=%s Blue=%s\n", Format(cols[0].r), Format(cols[0].g), Format(cols[0].b));
				fprintf(pStream,"Gradient middle color @ %s: Red=%s Green=%s Blue=%s\n", Format(midPoint), Format(cols[1].r), Format(cols[1].g), Format(cols[1].b));
				fprintf(pStream,"Gradient bottom color: Red=%s Green=%s Blue=%s\n", Format(cols[2].r), Format(cols[2].g), Format(cols[2].b));
			}
		}
#endif // NO_MAPTYPE_GRADIENT
	}

	// Environment
	for (i=0; i < ip->NumAtmospheric(); i++) {
		Atmospheric* atmos = ip->GetAtmospheric(i);
		if (atmos->ClassID() == Class_ID(FOG_CLASS_ID, 0)) {
			StdFog* fog = (StdFog*)atmos;
			Color col = fog->GetColor(GetStaticFrame());
			if (MaxVal(col) == 0 && fog->GetType() == 0) {
				// Distance cue (== black fog)
				// TBD: Where do we get the near and far planes?
				fprintf(pStream,"Dcue settings: Near %d Level %s Far %d Level: %s\n",
						0,
						Format(fog->GetNear(GetStaticFrame())),
						1000,
						Format(fog->GetFar(GetStaticFrame())));

				if (fog->GetFogBackground()) {
					fprintf(pStream,"Background dimmed\n");
				}
			}
			else {
				if (fog->GetType() == 0) {
					// Standard fog
					// TBD: Where do we get the near and far planes?
					fprintf(pStream,"Fog settings: Near %d Level %s Far %d Level: %s\n",
							0,
							Format(fog->GetNear(GetStaticFrame())),
							1000,
							Format(fog->GetFar(GetStaticFrame())));

					if (fog->GetFogBackground()) {
						fprintf(pStream,"Background fogged\n");
					}
				}
			}
			break;
		}
	}

	fprintf(pStream,"\n");
}

/****************************************************************************

  GeomObject output
  
****************************************************************************/

void AscOut::ExportGeomObject(INode* node, int indentLevel)
{
	int	i;

	ObjectState os = node->EvalWorldState(GetStaticFrame());
	if (!os.obj || os.obj->SuperClassID()!=GEOMOBJECT_CLASS_ID) {
		return;
	}
	
	// Targets are actually geomobjects, but we will export them
	// from the camera and light objects, so we skip them here.
	if (os.obj->ClassID() == Class_ID(TARGET_CLASS_ID, 0))
		return;
	
	BOOL needDel = FALSE;
	TriObject* tri = GetTriObjectFromNode(node, GetStaticFrame(), needDel);
	if (!tri) {
		return;
	}
	
	Mesh* mesh = &tri->GetMesh();
	Matrix3 tm = node->GetObjTMAfterWSM(GetStaticFrame());
		
	fprintf(pStream,"Named object: \"%s\"\n", FixupName(node->GetName()));
	fprintf(pStream,"Tri-mesh, Vertices: %d     Faces: %d\n",
			mesh->getNumVerts(),
			mesh->getNumFaces());

	if (mesh->numTVerts == 0) {
		// Vertices - no UV coords
		fprintf(pStream,"Vertex list:\n");
		for (i=0; i<mesh->getNumVerts(); i++) {
			Point3 v = (tm * mesh->verts[i]);
			fprintf(pStream, "Vertex %d:  X:%s     Y:%s     Z:%s\n",
				i, Format(v.x), Format(v.y), Format(v.z));
		}
	}
	else {
		// Vertices - with UV coords
		fprintf(pStream,"Mapped\n");
		fprintf(pStream,"Vertex list:\n");
		for (i=0; i<mesh->getNumVerts(); i++) {
			Point3 v = (tm * mesh->verts[i]);
			Point3 uv = mesh->numTVerts > i ? mesh->tVerts[i] : Point3(0.0f, 0.0f, 0.0f);
			fprintf(pStream, "Vertex %d:  X:%s     Y:%s     Z:%s     U:%s     V:%s\n",
				i, Format(v.x), Format(v.y), Format(v.z),
				Format(uv.x), Format(uv.y));
		}
	}

	Mtl* nodeMtl = node->GetMtl();

	fprintf(pStream,"Face list:\n");
	for (i=0; i<mesh->getNumFaces(); i++) {
		fprintf(pStream,"Face %d:    A:%d B:%d C:%d AB:%d BC:%d CA:%d\n",
			i, mesh->faces[i].v[0], mesh->faces[i].v[1], mesh->faces[i].v[2],
			mesh->faces[i].getEdgeVis(0) ? 1 : 0,
			mesh->faces[i].getEdgeVis(1) ? 1 : 0,
			mesh->faces[i].getEdgeVis(2) ? 1 : 0);
		if (nodeMtl) {
			if (nodeMtl->ClassID() == Class_ID(MULTI_CLASS_ID, 0)) {
				Mtl* faceMtl = nodeMtl->GetSubMtl(mesh->faces[i].getMatID() % nodeMtl->NumSubMtls());
				if (faceMtl) {
					fprintf(pStream,"Material: \"%s\"\n", faceMtl->GetName());
				}
			}
			else {
				fprintf(pStream,"Material: \"%s\"\n", node->GetMtl()->GetName());
			}
		}
		if (mesh->faces[i].smGroup) {
			fprintf(pStream,"Smoothing: ");			
			for (int j=0; j<32; j++) {
				if (mesh->faces[i].smGroup & (1<<j)) {
					if (mesh->faces[i].smGroup>>(j+1)) {
						fprintf(pStream,"%d, ",j+1);
					}
					else {
						fprintf(pStream,"%d ",j+1);
					}
				}
			}
		}
		fprintf(pStream,"\n");
	}
	if (needDel) {
		delete tri;
	}

	fprintf(pStream,"\n");
}

/****************************************************************************

  Shape output
  
****************************************************************************/

void AscOut::ExportShapeObject(INode* node, int indentLevel)
{
	// No shape export tp ASC.
	// To export shapes they need to be capped and then they will be
	// exported as a tri-object
}

/****************************************************************************

  Light output
  
****************************************************************************/

void AscOut::ExportLightObject(INode* node, int indentLevel)
{
	ObjectState os = node->EvalWorldState(GetStaticFrame());
	if (!os.obj) {
		return;
	}
	
	GenLight* light = (GenLight*)os.obj;

	fprintf(pStream,"Named object: \"%s\"\n", FixupName(node->GetName()));
	fprintf(pStream,"Direct light\n");

	Matrix3 tm = node->GetObjTMAfterWSM(GetStaticFrame());
	Point3	pos = tm.GetTrans();
	fprintf(pStream,"Position: X:%s\tY:%s\tZ:%s\n",
			Format(pos.x), Format(pos.y), Format(pos.z));

	if (light->IsSpot()) {
		INode* target = node->GetTarget();
		if (target) {
			tm = target->GetObjTMAfterWSM(GetStaticFrame());
			pos = tm.GetTrans();
			fprintf(pStream, "Spotlight to: X:%s\tY:%s\tZ:%s\n", Format(pos.x), Format(pos.y), Format(pos.z));
			fprintf(pStream, "Hotspot size: %s degrees\n", Format(light->GetHotspot(GetStaticFrame(), FOREVER)*180.0f/PI));
			fprintf(pStream, "Falloff size: %s degrees\n", Format(light->GetFallsize(GetStaticFrame(), FOREVER)*180.0f/PI));
		}
	}

	Color col = light->GetRGBColor(GetStaticFrame(), FOREVER);
	fprintf(pStream,"Light color: Red=%s\tGreen=%s\tBlue=%s\n",
			Format(col.r), Format(col.g), Format(col.b));
}

/****************************************************************************

  Camera output
  
****************************************************************************/

float FOVtoMM(float aperture, float fov)
{
	return (float)((0.5f*aperture)/tan(fov/2.0f));
}

void AscOut::ExportCameraObject(INode* node, int indentLevel)
{
	ObjectState os = node->EvalWorldState(GetStaticFrame());
	if (!os.obj) {
		return;
	}

	CameraObject *cam = (CameraObject *)os.obj;
	float fov = cam->GetFOV(GetStaticFrame(), FOREVER);
	float lens = FOVtoMM(ip->GetRendApertureWidth(), fov);

	fprintf(pStream,"Named object: \"%s\"\n", FixupName(node->GetName()));
	fprintf(pStream,"Camera (%smm)\n", Format(lens));

	Matrix3 tm = node->GetObjTMAfterWSM(GetStaticFrame());
	Point3	pos = tm.GetTrans();
	fprintf(pStream,"Position: X:%s\tY:%s\tZ:%s\n",
			Format(pos.x), Format(pos.y), Format(pos.z));

	INode* target = node->GetTarget();
	if (target) {
		tm = target->GetObjTMAfterWSM(GetStaticFrame());
		pos = tm.GetTrans();
		fprintf(pStream, "Target: X:%s\tY:%s\tZ:%s\n", Format(pos.x), Format(pos.y), Format(pos.z));
	}
	else {
		// Fake a target based on distance to target
		// The camera looks down the negative Z axis
		pos.z -= cam->GetTDist(GetStaticFrame(), FOREVER);
		fprintf(pStream, "Target: X:%s\tY:%s\tZ:%s\n", Format(pos.x), Format(pos.y), Format(pos.z));
	}



}

/****************************************************************************

  Helper object output
  
****************************************************************************/

void AscOut::ExportHelperObject(INode* node, int indentLevel)
{
	// No Helper object export to ASC
}


/****************************************************************************

  Node Header
  
****************************************************************************/

void AscOut::ExportNodeHeader(INode* node, TCHAR* type, int indentLevel)
{
	// Naah...
}


/****************************************************************************

  Misc Utility functions
  
****************************************************************************/

// Return an indentation string
TSTR AscOut::GetIndent(int indentLevel)
{
	TSTR indentString = "";
	for (int i=0; i<indentLevel; i++) {
		indentString += "\t";
	}
	
	return indentString;
}

// Return a pointer to a TriObject given an INode or return NULL
// if the node cannot be converted to a TriObject
TriObject* AscOut::GetTriObjectFromNode(INode *node, TimeValue t, int &deleteIt)
{
	deleteIt = FALSE;
	Object *obj = node->EvalWorldState(t).obj;
	if (obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0))) { 
		TriObject *tri = (TriObject *) obj->ConvertToType(t, 
			Class_ID(TRIOBJ_CLASS_ID, 0));
		// Note that the TriObject should only be deleted
		// if the pointer to it is not equal to the object
		// pointer that called ConvertToType()
		if (obj != tri) deleteIt = TRUE;
		return tri;
	}
	else {
		return NULL;
	}
}

/****************************************************************************

  String manipulation functions
  
****************************************************************************/

#define CTL_CHARS  31
#define SINGLE_QUOTE 39

// Replace some characters we don't care for.
TCHAR* AscOut::FixupName(TCHAR* name)
{
	static char buffer[256];
	TCHAR* cPtr;
	
    _tcscpy(buffer, name);
    cPtr = buffer;
	
    while(*cPtr) {
		if (*cPtr == '"')
			*cPtr = SINGLE_QUOTE;
        else if (*cPtr <= CTL_CHARS)
			*cPtr = _T('_');
        cPtr++;
    }
	
	return buffer;
}

// International settings in Windows could cause a number to be written
// with a "," instead of a ".".
// To compensate for this we need to convert all , to . in order to make the
// format consistent.
void AscOut::CommaScan(TCHAR* buf)
{
    for(; *buf; buf++) if (*buf == ',') *buf = '.';
}

TSTR AscOut::Format(int value)
{
	TCHAR buf[50];
	
	sprintf(buf, _T("%d"), value);
	return buf;
}


TSTR AscOut::Format(float value)
{
	TCHAR buf[40];
	
	sprintf(buf, szFmtStr, value);
	CommaScan(buf);
	return TSTR(buf);
}

TSTR AscOut::Format(Point3 value)
{
	TCHAR buf[120];
	TCHAR fmt[120];
	
	sprintf(fmt, "%s\t%s\t%s", szFmtStr, szFmtStr, szFmtStr);
	sprintf(buf, fmt, value.x, value.y, value.z);

	CommaScan(buf);
	return buf;
}

TSTR AscOut::Format(Color value)
{
	TCHAR buf[120];
	TCHAR fmt[120];
	
	sprintf(fmt, "%s\t%s\t%s", szFmtStr, szFmtStr, szFmtStr);
	sprintf(buf, fmt, value.r, value.g, value.b);

	CommaScan(buf);
	return buf;
}

TSTR AscOut::Format(AngAxis value)
{
	TCHAR buf[160];
	TCHAR fmt[160];
	
	sprintf(fmt, "%s\t%s\t%s\t%s", szFmtStr, szFmtStr, szFmtStr, szFmtStr);
	sprintf(buf, fmt, value.axis.x, value.axis.y, value.axis.z, value.angle);

	CommaScan(buf);
	return buf;
}


TSTR AscOut::Format(Quat value)
{
	// A Quat is converted to an AngAxis before output.
	
	Point3 axis;
	float angle;
	AngAxisFromQ(value, &angle, axis);
	
	return Format(AngAxis(axis, angle));
}

TSTR AscOut::Format(ScaleValue value)
{
	TCHAR buf[280];
	
	sprintf(buf, "%s %s", Format(value.s), Format(value.q));
	CommaScan(buf);
	return buf;
}
