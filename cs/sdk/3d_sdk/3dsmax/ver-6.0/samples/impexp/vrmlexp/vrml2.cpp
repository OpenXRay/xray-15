/**********************************************************************
 *<
	FILE: vrml2.cpp

	DESCRIPTION:  VRML 2.0 .WRL file export module

	CREATED BY: Scott Morrison

	HISTORY: created 7 June, 1996

 *>	Copyright (c) 1996, All Rights Reserved.
 **********************************************************************/

#include <time.h>
#include "vrml.h"
#include "simpobj.h"
#include "istdplug.h"
#include "inline.h"
#include "lod.h"
#include "inlist.h"
#include "notetrck.h"
#include "bookmark.h"
#include "stdmat.h"
#include "normtab.h"
#include "vrml_api.h"
#include "vrmlexp.h"
#include "decomp.h"
#include "timer.h"
#include "navinfo.h"
#include "backgrnd.h"
#include "fog.h"
#include "audio.h"
#include "sound.h"
#include "touch.h"
#include "prox.h"
#include "appd.h"
#include "anchor.h"
#include "bboard.h"
#include "vrml2.h"
#include "pmesh.h"
#include "evalcol.h"
#include <maxscrpt.h>
#include <maxclses.h>

#ifdef _DEBUG
#define FUNNY_TEST
#endif

//#define TEST_MNMESH
#ifdef TEST_MNMESH
#include "mnmath.h"
#endif

#define MIRROR_BY_VERTICES
// alternative, mirror by scale, is deprecated  --prs.

#define AEQ(a, b) (fabs(a - b) < 0.5 * pow(10.0, -mDigits))

extern TCHAR *GetString(int id);

static HWND hWndPDlg;   // handle of the progress dialog
static HWND hWndPB;     // handle of progress bar 

////////////////////////////////////////////////////////////////////////
// VRML 2.0 Export implementation
////////////////////////////////////////////////////////////////////////

//#define TEMP_TEST
#ifdef TEMP_TEST
static int getworldmat = 0;
#endif

void AngAxisFromQa(const Quat& q, float *ang, Point3& axis) {
	double omega, s, x, y, z, w, l, c;
	x = (double)q.x;	
	y = (double)q.y;	
	z = (double)q.z;	
	w = (double)q.w;	
	l =  sqrt(x*x + y*y + z*z + w*w);
	if (l == 0.0) {
		w = 1.0;	
		y = z = x = 0.0;
		}
	else {	
		c = 1.0/l;
		x *= c;
		y *= c;
		z *= c;
		w *= c;
		}
	omega = acos(w);
	*ang = (float)(2.0*omega);
	s = sin(omega);	
	if (fabs(s) > 0.000001f)
    {
        axis[0] = (float)(x / s);
        axis[1] = (float)(y / s);
        axis[2] = (float)(z / s);
    }
	else
        axis = Point3(0,0,0); // RB: Added this so axis gets initialized
}


Matrix3
GetLocalTM(INode* node, TimeValue t)
{
    Matrix3 tm;
    tm = node->GetObjTMAfterWSM(t);
#ifdef TEMP_TEST
    if (getworldmat)
        return tm;
#endif
    if (!node->GetParentNode()->IsRootNode()) {
        Matrix3 ip = node->GetParentNode()->GetObjTMAfterWSM(t);
        tm = tm * Inverse(ip);
    }
    return tm;
}

inline float
round(float f)
{
    if (f < 0.0f) {
        if (f > -1.0e-5)
            return 0.0f;
        return f;
    }
    if (f < 1.0e-5)
        return 0.0f;
    return f;
}

void
CommaScan(TCHAR* buf)
{
    for(; *buf; buf++) if (*buf == ',') *buf = '.';
}

TCHAR*
VRML2Export::point(Point3& p)
{
    static TCHAR buf[50];
    TCHAR format[20];
    sprintf(format, _T("%%.%dg %%.%dg %%.%dg"), mDigits, mDigits, mDigits);
    if (mZUp)
        sprintf(buf, format, round(p.x), round(p.y), round(p.z));
    else
        sprintf(buf, format, round(p.x), round(p.z), round(-p.y));
    CommaScan(buf);
    return buf;
}

TCHAR*
VRML2Export::color(Color& c)
{
    static TCHAR buf[50];
    TCHAR format[20];
    sprintf(format, _T("%%.%dg %%.%dg %%.%dg"), mDigits, mDigits, mDigits);
    sprintf(buf, format, round(c.r), round(c.g), round(c.b));
    CommaScan(buf);
    return buf;
}

TCHAR*
VRML2Export::color(Point3& c)
{
    static TCHAR buf[50];
    TCHAR format[20];
    sprintf(format, _T("%%.%dg %%.%dg %%.%dg"), mDigits, mDigits, mDigits);
    sprintf(buf, format, round(c.x), round(c.y), round(c.z));
    CommaScan(buf);
    return buf;
}


TCHAR*
VRML2Export::floatVal(float f)
{
    static TCHAR buf[50];
    TCHAR format[20];
    sprintf(format, _T("%%.%dg"), mDigits);
    sprintf(buf, format, round(f));
    CommaScan(buf);
    return buf;
}


TCHAR*
VRML2Export::texture(UVVert& uv)
{
    static TCHAR buf[50];
    TCHAR format[20];
    sprintf(format, _T("%%.%dg %%.%dg"), mDigits, mDigits);
    sprintf(buf, format, round(uv.x), round(uv.y));
    CommaScan(buf);
    return buf;
}

// Format a scale value
TCHAR*
VRML2Export::scalePoint(Point3& p)
{
    static TCHAR buf[50];
    TCHAR format[20];
    sprintf(format, _T("%%.%dg %%.%dg %%.%dg"), mDigits, mDigits, mDigits);
    if (mZUp)
        sprintf(buf, format, round(p.x), round( p.y), round(p.z));
    else
        sprintf(buf, format, round(p.x), round( p.z), round(p.y));
    CommaScan(buf);
    return buf;
}

// Format a normal vector
TCHAR*
VRML2Export::normPoint(Point3& p)
{
    static TCHAR buf[50];
    TCHAR format[20];
    sprintf(format, _T("%%.%dg %%.%dg %%.%dg"), mDigits, mDigits, mDigits);
    if (mZUp)
        sprintf(buf, format, round(p.x), round(p.y), round(p.z));
    else
        sprintf(buf, format, round(p.x), round(p.z), round(-p.y));
    CommaScan(buf);
    return buf;
}

// Format an axis value
TCHAR*
VRML2Export::axisPoint(Point3& p, float angle)
{
    if (p == Point3(0., 0., 0.)) 
        p = Point3(1., 0., 0.); // default direction
    static TCHAR buf[50];
    TCHAR format[20];
    sprintf(format, _T("%%.%dg %%.%dg %%.%dg %%.%dg"),
            mDigits, mDigits, mDigits, mDigits);
    if (mZUp)
        sprintf(buf, format, round(p.x), round(p.y), round(p.z),
                round(angle));
    else
        sprintf(buf, format, round(p.x), round(p.z), round(-p.y),
                round(angle));
    CommaScan(buf);
    return buf;
}

// Indent to the given level.
void 
VRML2Export::Indent(int level)
{
    if (!mIndent) return;
    assert(level >= 0);
    for(; level; level--)
        fprintf(mStream, _T("  "));
}
    
// Translates name (if necessary) to VRML compliant name.
// Returns name in static buffer, so calling a second time trashes
// the previous contents.
#define CTL_CHARS      31
#define SINGLE_QUOTE   39
static TCHAR * VRMLName(TCHAR *name)
{
    static char buffer[256];
    static int seqnum = 0;
    TCHAR* cPtr;
    int firstCharacter = 1;

    _tcscpy(buffer, name);
    cPtr = buffer;
    while(*cPtr) {
        if( *cPtr <= CTL_CHARS ||
            *cPtr == ' ' ||
            *cPtr == SINGLE_QUOTE ||
            *cPtr == '"' ||
            *cPtr == '\\' ||
            *cPtr == '{' ||
            *cPtr == '}' ||
            *cPtr == ',' ||            
            *cPtr == '.' ||
            *cPtr == '[' ||
            *cPtr == ']' ||
            *cPtr == '-' ||
            *cPtr == '#' ||
            *cPtr >= 127 ||
            (firstCharacter && (*cPtr >= '0' && *cPtr <= '9'))) *cPtr = '_';
        firstCharacter = 0;
        cPtr++;
    }
    if (firstCharacter) {       // if empty name, quick, make one up!
        *cPtr++ = '_';
        *cPtr++ = '_';
        sprintf(cPtr, "%d", seqnum++);
    }
    
    return buffer;
}

// Write beginning of the Transform node.
void
VRML2Export::StartNode(INode* node, int level, BOOL outputName)
{
    TCHAR *nodnam = mNodes.GetNodeName(node);
    Indent(level);
    fprintf(mStream, _T("DEF %s Transform {\n"), nodnam);
    
    // Put note tracks as info nodes
    int numNotes = node->NumNoteTracks();
    for(int i=0; i < numNotes; i++) {
        DefNoteTrack *nt = (DefNoteTrack*) node->GetNoteTrack(i);
        for (int j = 0; j < nt->keys.Count(); j++) {
            NoteKey* nk = nt->keys[j];
            TSTR note = nk->note;
            if (note.Length() > 0) {
                Indent(level+1);
                fprintf(mStream, _T("#Info { string \"frame %d: %s\" }\n"),
                        nk->time/GetTicksPerFrame(), note.data());
            }
        }
    }
}

// Write end of the Transform node.
void
VRML2Export::EndNode(INode *node, Object* obj, int level, BOOL lastChild)
{
    Indent(level);
    if (lastChild || node->GetParentNode()->IsRootNode())
        fprintf(mStream, _T("}\n"));
    else
        fprintf(mStream, _T("},\n"));    
}

/* test
BOOL
VRML2Export::IsBBoxTrigger(INode* node)
{
    Object * obj = node->EvalWorldState(mStart).obj;
    if (obj->ClassID() != Class_ID(MR_BLUE_CLASS_ID1, MR_BLUE_CLASS_ID2))
        return FALSE;
    MrBlueObject* mbo = (MrBlueObject*) obj;
    return mbo->GetBBoxEnabled();
}
*/

static BOOL
HasPivot(INode* node)
{
    Point3 p = node->GetObjOffsetPos();
    return p.x != 0.0f || p.y != 0.0f || p.z != 0.0f;
}

// Write out the transform from the parent node to this current node.
// Return true if it has a mirror transform
BOOL
VRML2Export::OutputNodeTransform(INode* node, int level, BOOL mirrored)
{
    // Root node is always identity
    if (node->IsRootNode())
        return FALSE;

    Matrix3 tm = GetLocalTM(node, mStart);
    int i, j;
    Point3 p;
    Point3 s, axis;
    Quat q;
    float ang;

    BOOL isIdentity = TRUE;
    for (i=0;i<3;i++) {
        for (j=0;j<3;j++) {
            if (i==j) {
                if (tm.GetRow(i)[j] != 1.0) isIdentity = FALSE;
            } else if (fabs(tm.GetRow(i)[j]) > 0.00001) isIdentity = FALSE;
        }
    }


    if (isIdentity) {
        p = tm.GetTrans();
#ifdef MIRROR_BY_VERTICES
        if (mirrored)
            p = - p;
#endif
        Indent(level);
        fprintf(mStream, _T("translation %s\n"), point(p));
        return FALSE;
    }
    AffineParts parts;
#ifdef DDECOMP
    d_decomp_affine(tm, &parts);
#else
    decomp_affine(tm, &parts);      // parts is parts
#endif
    p = parts.t;
    q = parts.q;
    AngAxisFromQa(q, &ang, axis);
#ifdef MIRROR_BY_VERTICES
        if (mirrored)
            p = - p;
#endif
    Indent(level);
    fprintf(mStream, _T("translation %s\n"), point(p));
    Control*rc = node->GetTMController()->GetRotationController();

    if (ang != 0.0f && ang != -0.0f) {
        Indent(level);
        fprintf(mStream, _T("rotation %s\n"), axisPoint(axis, -ang));
    }
    ScaleValue sv(parts.k, parts.u);
    s = sv.s;
#ifndef MIRROR_BY_VERTICES
    if (parts.f < 0.0f)
        s = - s;            // this is where we mirror by scale
#endif
    if (!(AEQ(s.x, 1.0)) || !(AEQ(s.y, 1.0)) || !(AEQ(s.z, 1.0))) {
        Indent(level);
        fprintf(mStream, _T("scale %s\n"), scalePoint(s));
        q = sv.q;
        AngAxisFromQa(q, &ang, axis);
        if (ang != 0.0f && ang != -0.0f) {
            Indent(level);
            fprintf(mStream, _T("scaleOrientation %s\n"),
                    axisPoint(axis, -ang));
        }
    }
    return parts.f < 0.0f;
}

static BOOL
MeshIsAllOneSmoothingGroup(Mesh& mesh)
{
    return FALSE;           // to put out normals whenever they're called for

    int numfaces = mesh.getNumFaces();
    unsigned int sg;
    int i;

    for(i = 0; i < numfaces; i++) {
        if (i == 0) {
            sg = mesh.faces[i].getSmGroup();
            if (sg == 0)
                return FALSE;
        }
        else {
            if (sg != mesh.faces[i].getSmGroup())
                return FALSE;
        }
    }
    return TRUE;
}

#define CurrentWidth() (mIndent ? 2*level : 0)
#define MAX_WIDTH 60

int
VRML2Export::MaybeNewLine(int width, int level)
{
    if (width > MAX_WIDTH) {
        fprintf(mStream, _T("\n"));
        Indent(level);
        return CurrentWidth();
    }
    return width;
}

void
VRML2Export::OutputNormalIndices(Mesh& mesh, NormalTable* normTab, int level,
                                 int textureNum)
{
    Point3 n;
    int numfaces = mesh.getNumFaces();
    int i, j, v, norCnt;
    int width = CurrentWidth();

    Indent(level);
    
    fprintf(mStream, _T("normalIndex [\n"));
    Indent(level+1);
    for (i = 0; i < numfaces; i++) {
        int id = mesh.faces[i].getMatID();
        if (textureNum == -1 || id == textureNum) {
            int smGroup = mesh.faces[i].getSmGroup();
            for(v = 0; v < 3; v++) {
                int cv = mesh.faces[i].v[v];
                RVertex * rv = mesh.getRVertPtr(cv);
                if (rv->rFlags & SPECIFIED_NORMAL) {
                    n = rv->rn.getNormal();
                    continue;
                }
                else if((norCnt = (int)(rv->rFlags & NORCT_MASK)) && smGroup) {
                    if (norCnt == 1)
                        n = rv->rn.getNormal();
                    else for(j = 0; j < norCnt; j++) {
                        if (rv->ern[j].getSmGroup() & smGroup) {
                            n = rv->ern[j].getNormal();
                            break;
                        }
                    }
                } else
                    n = mesh.getFaceNormal(i);
                int index = normTab->GetIndex(n);
                assert (index != -1);
                width += fprintf(mStream, _T("%d, "), index);
                width = MaybeNewLine(width, level+1);
            }
            width += fprintf(mStream, _T("-1, "));
            width = MaybeNewLine(width, level+1);
        }
    }
    fprintf(mStream, _T("]\n"));
}

NormalTable*
VRML2Export::OutputNormals(Mesh& mesh, int level)
{
    int i, j, norCnt;
    int numverts = mesh.getNumVerts();
    int numfaces = mesh.getNumFaces();
    NormalTable* normTab;


    mesh.buildRenderNormals();

    if (MeshIsAllOneSmoothingGroup(mesh)) {
        return NULL;
    }

    normTab = new NormalTable();

    // Otherwise we have several smoothing groups
    for(int index = 0; index < numfaces; index++) {
        int smGroup = mesh.faces[index].getSmGroup();
        for(i = 0; i < 3; i++) {
            int cv = mesh.faces[index].v[i];
            RVertex * rv = mesh.getRVertPtr(cv);
            if (rv->rFlags & SPECIFIED_NORMAL) {
                normTab->AddNormal(rv->rn.getNormal());
            }
            else if((norCnt = (int)(rv->rFlags & NORCT_MASK)) && smGroup) {
                if (norCnt == 1)
                    normTab->AddNormal(rv->rn.getNormal());
                else for(j = 0; j < norCnt; j++) {
                    normTab->AddNormal(rv->ern[j].getNormal());
                }
            } else
                normTab->AddNormal(mesh.getFaceNormal(index));
        }
    }

    index = 0;
    NormalDesc* nd;
    Indent(level);
    fprintf(mStream, _T("normal "));
    fprintf(mStream, _T("Normal { vector [\n"));
    int width = CurrentWidth();
    Indent(level+1);

    for(i = 0; i < NORM_TABLE_SIZE; i++) {
        for(nd = normTab->Get(i); nd; nd = nd->next) {
            nd->index = index++;
            Point3 p = nd->n / NUM_NORMS;
            width += fprintf(mStream, _T("%s, "), normPoint(p));
            width = MaybeNewLine(width, level+1);
        }
    }
    fprintf(mStream, _T("] }\n"));

    Indent(level);
    fprintf(mStream, _T("normalPerVertex TRUE\n"));
    
#ifdef DEBUG_NORM_HASH
    normTab->PrintStats(mStream);
#endif

    return normTab;
}

void
VRML2Export::OutputPolygonObject(INode* node, TriObject* obj, BOOL isMulti,
                             BOOL isWire, BOOL twoSided, int level,
                             int textureNum, BOOL pMirror)
{
    assert(obj);
    int width;
    int i, j;
    NormalTable* normTab = NULL;
    TextureDesc* td = NULL;
    BOOL dummy, concave;
    Mesh &mesh = obj->GetMesh();
    int numtverts = mesh.getNumTVerts();
    int numfaces = mesh.getNumFaces();

    PMPoly* poly;
    PMesh polyMesh(obj->GetMesh(), (PType) mPolygonType, numtverts);
#ifdef TEST_MNMESH
    MNMesh mnmesh(obj->mesh);
    mnmesh.MakePolyMesh();
    FILE *mnfile = fopen("mnmesh0.txt", "w");
    fprintf(mnfile, "Vertices:\n");
    for (i = 0; i < mnmesh.VNum(); i++)
        fprintf(mnfile, "  %3d)  %8.3f, %8.3f, %8.3f\n", i,
            mnmesh.P(i).x, mnmesh.P(i).y, mnmesh.P(i).z);
    fprintf(mnfile, "Faces:\n");
    for (i = 0; i < mnmesh.FNum(); i++) {
        fprintf(mnfile, "  ");
        MNFace *mnf = mnmesh.F(i);
        for (j = 0; j < mnf->deg; j++) {
            if (j > 0)
                fprintf(mnfile, ", ");
            fprintf(mnfile, "%3d", mnf->vtx[j]);
        }
        fprintf (mnfile, "\n");
    }
    fclose(mnfile);
#endif
    concave = polyMesh.GenPolygons();

    Mtl* mtl = node->GetMtl();
    if (mtl && mtl->IsMultiMtl() && textureNum != -1) {
        if (mtl != NULL) {
            mtl = mtl->GetSubMtl(textureNum);
            td = GetMtlTex(mtl, dummy);
        }
    } else td = GetMatTex(node, dummy);
    
    if (!numfaces) {
        delete td;
        return;
    }
    
    Indent(level++);
    if (isWire)
        fprintf(mStream, _T("geometry DEF %s-FACES IndexedLineSet {\n"),
            mNodes.GetNodeName(node));
    else
        fprintf(mStream, _T("geometry DEF %s-FACES IndexedFaceSet {\n"),
                mNodes.GetNodeName(node));

    if (!isWire) {
        Indent(level);
        fprintf(mStream, _T("ccw %s\n"), pMirror ? _T("FALSE") : _T("TRUE"));
        Indent(level);
        fprintf(mStream, _T("solid %s\n"), twoSided ? _T("FALSE") : _T("TRUE"));
        Indent(level);
        fprintf(mStream, _T("convex %s\n"), concave ? _T("FALSE") : _T("TRUE"));
    }

 // color-------
    if (mPreLight) {
        if (!mCPVSource) {   // 1 if MAX, 0 if we calc
            ColorTab vxColDiffTab;
            calcMixedVertexColors(node, mStart, LIGHT_SCENELIGHT, vxColDiffTab);
            int numColors = 0;
            int cfaces;

            Color c;
            Indent(level);
            fprintf(mStream, _T("colorPerVertex TRUE\n"));
            Indent(level);
            width = CurrentWidth();
            fprintf(mStream, _T("color Color { color [\n"));
            Indent(level+1);
            cfaces = vxColDiffTab.Count();
            for (i = 0; i < cfaces; i++) {
                c = *((Color*) vxColDiffTab[i]);
                if (i == cfaces - 1) width += fprintf(mStream, _T("%s "), color(c));
                else width += fprintf(mStream, _T("%s, "), color(c));
                width = MaybeNewLine(width, level+1);
            }
            Indent(level);
            fprintf(mStream, _T("] }\n"));

            Indent(level);
            fprintf(mStream, _T("colorIndex [\n"));
            width = CurrentWidth();
            Indent(level+1);
            cfaces = polyMesh.GetPolygonCnt();
         // FIXME need to add colorlist to PMesh
            for (i = 0; i < cfaces; i++) {
                poly = polyMesh.GetPolygon(i);
                for (j = 0; j < poly->GetVIndexCnt(); j++) {
                    width += fprintf(mStream, _T("%d, "),
                                polyMesh.LookUpVert(poly->GetVIndex(j)));
			        width  = MaybeNewLine(width, level+1);
                }
                width += fprintf(mStream, _T("-1"));
		        if (i != polyMesh.GetPolygonCnt() - 1) {
        			width += fprintf(mStream, _T(", "));
			        width = MaybeNewLine(width, level+1);
		        }
	        }
	        fprintf(mStream, _T("]\n"));

            for (i = 0; i < vxColDiffTab.Count(); i++) {
                delete (Color*) vxColDiffTab[i];
            }
            vxColDiffTab.ZeroCount();
            vxColDiffTab.Shrink();
        } else {
            int numCVerts = mesh.getNumVertCol();
            if (numCVerts) {
                VertColor vColor;
                Indent(level);
                fprintf(mStream, _T("colorPerVertex TRUE\n"));
                Indent(level);
                width = CurrentWidth();
                fprintf(mStream, _T("color Color { color [\n"));
                Indent(level+1);

                int nVerts = polyMesh.GetVertexCnt();
                for (i = 0; i < nVerts; i++) {
                    /*
                    for (j = 0; j < poly->GetVIndexCnt(); j++) {
                        width += fprintf(mStream, _T("%d, "),
                                    polyMesh.LookUpVert(poly->GetVIndex(j)));
			            width  = MaybeNewLine(width, level+1);
                    }
                    */
                    int vIndex = polyMesh.LookUpVert(i);

                    if (vIndex > numCVerts) {
                        assert(FALSE);
                        break;
                    }

                    vColor = mesh.vertCol[vIndex];
                    if (i == nVerts - 1)
                        width += fprintf(mStream, _T("%s "), color(vColor));
                    else
                        width += fprintf(mStream, _T("%s, "), color(vColor));
                    width = MaybeNewLine(width, level+1);
                }
                Indent(level);
                fprintf(mStream, _T("] }\n"));

                Indent(level);
                fprintf(mStream, _T("colorIndex [\n"));
                width = CurrentWidth();
                Indent(level+1);
                int cfaces = polyMesh.GetPolygonCnt();
             // FIXME need to add colorlist to PMesh
                for (i = 0; i < cfaces; i++) {
                    poly = polyMesh.GetPolygon(i);
                    for (j = 0; j < poly->GetVIndexCnt(); j++) {
                        width += fprintf(mStream, _T("%d, "),
                                    polyMesh.LookUpVert(poly->GetVIndex(j)));
			            width  = MaybeNewLine(width, level+1);
                    }
                    width += fprintf(mStream, _T("-1"));
		            if (i != polyMesh.GetPolygonCnt() - 1) {
        			        width += fprintf(mStream, _T(", "));
			                width = MaybeNewLine(width, level+1);
		            }
	            }
	            fprintf(mStream, _T("]\n"));
            } else {
                mPreLight = FALSE;
            }
        }
    }

    int numColors = 0;
    if (!mPreLight && isMulti && textureNum == -1) {
        Color c;
        Indent(level);
        fprintf(mStream, _T("colorPerVertex FALSE\n"));
        Mtl* sub, * mtl = node->GetMtl();
        assert (mtl->IsMultiMtl());
        int num = mtl->NumSubMtls();
        Indent(level);
        width = CurrentWidth();
     
        fprintf(mStream, _T("color Color { color [\n"));
        Indent(level+1);
        for (i = 0; i < num; i++) {
            sub = mtl->GetSubMtl(i);
            if (!sub) continue;
            numColors++;
            c = sub->GetDiffuse(mStart);
            if (i == num - 1) width += fprintf(mStream, _T("%s "), color(c));
            else width += fprintf(mStream, _T("%s, "), color(c));
            width = MaybeNewLine(width, level+1);
        }
        Indent(level);
        fprintf(mStream, _T("] }\n"));
    }
    if (!mPreLight && isMulti && numColors > 0 && textureNum == -1) {
        Indent(level);
        fprintf(mStream, _T("colorIndex [\n"));
        width = CurrentWidth();
        Indent(level+1);
        numfaces = polyMesh.GetPolygonCnt();
     // FIXME need to add colorlist to PMesh
        for (i = 0; i < numfaces; i++) {
            poly = polyMesh.GetPolygon(i);
            int matID = mesh.faces[poly->GetTriFace(0)].getMatID();
            matID  = (matID % numColors);
            width += fprintf(mStream, _T("%d"), matID);
            if (i != numfaces - 1) {
                width += fprintf(mStream, _T(", "));
                width  = MaybeNewLine(width, level+1);
            }
        }
        fprintf(mStream, _T("]\n"));
    }
    
 // output coordinate---------
    if (textureNum < 1) {
        Indent(level);
        fprintf(mStream, _T("coord DEF %s-COORD Coordinate { point [\n"),
                mNodes.GetNodeName(node));
        width = CurrentWidth();
        Indent(level+1);
        for (i = 0; i < polyMesh.GetVertexCnt(); i++) {
            Point3 p = polyMesh.GetVertex(i);
#ifdef MIRROR_BY_VERTICES
            if (pMirror)
                p = - p;
#endif
            width += fprintf(mStream, _T("%s"), point(p));
            if (i == polyMesh.GetVertexCnt() - 1) {
                fprintf(mStream, _T("]\n"));
                Indent(level);
                fprintf(mStream, _T("}\n"));
            } else {
                width += fprintf(mStream, _T(", "));
                width = MaybeNewLine(width, level+1);
            }
        }
    } else {
        Indent(level);
        fprintf(mStream, _T("coord USE %s-COORD\n"),
                mNodes.GetNodeName(node));
    }
    Indent(level);
	fprintf(mStream, _T("coordIndex [\n"));
	Indent(level+1);
    width = CurrentWidth();
    for (i = 0; i < polyMesh.GetPolygonCnt(); i++) {
        poly = polyMesh.GetPolygon(i);
        for (j = 0; j < poly->GetVIndexCnt(); j++) {
            width += fprintf(mStream, _T("%d, "),
                            polyMesh.LookUpVert(poly->GetVIndex(j)));
			width  = MaybeNewLine(width, level+1);
        }
        width += fprintf(mStream, _T("-1"));
		if (i != polyMesh.GetPolygonCnt() - 1) {
			width += fprintf(mStream, _T(", "));
			width = MaybeNewLine(width, level+1);
		}
	}
	fprintf(mStream, _T("]\n"));

 // Output Texture coordinates
    if (numtverts > 0 && (td || textureNum == 0) && !isWire) {
        if (textureNum < 1) {
            Indent(level);
            fprintf(mStream, _T("texCoord DEF %s-TEXCOORD TextureCoordinate { point [\n"),
                mNodes.GetNodeName(node));
            width = CurrentWidth();
            Indent(level+1);
            for (i = 0; i < polyMesh.GetTVertexCnt(); i++) {
                UVVert t = polyMesh.GetTVertex(i);
                width += fprintf(mStream, _T("%s"), texture(t));
                if (i == polyMesh.GetTVertexCnt() - 1) {
                    fprintf(mStream, _T("]\n"));
                    Indent(level);
                    fprintf(mStream, _T("}\n"));
                }
                else {
                    width += fprintf(mStream, _T(", "));
                    width = MaybeNewLine(width, level+1);
                }
            }
        } else {
            Indent(level);
            fprintf(mStream, _T("texCoord USE %s-TEXCOORD\n"),
                    mNodes.GetNodeName(node));
        }
    }
    if (numtverts > 0 && td && !isWire) {
        Indent(level);
	    fprintf(mStream, _T("texCoordIndex [\n"));
	    Indent(level+1);
        width = CurrentWidth();
        int tmp = polyMesh.GetPolygonCnt();
        //for (i = 0; i < polyMesh.GetPolygonCnt(); i++) {
        for (i = 0; i < tmp; i++) {
            poly = polyMesh.GetPolygon(i);
            int tmp1 = poly->GetTVIndexCnt();
            //for (j = 0; j < poly->GetTVIndexCnt(); j++) {
            for (j = 0; j < tmp1; j++) {
                int tmp2 = poly->GetTVIndex(j);
                int tmp3 = polyMesh.LookUpTVert(tmp2);
			    width += fprintf(mStream, _T("%d, "),
                                //polyMesh.LookUpTVert(poly->GetTVIndex(j)));
                                tmp3);
			    width  = MaybeNewLine(width, level+1);
		    }
		    width += fprintf(mStream, _T("-1"));
		    if (i != polyMesh.GetPolygonCnt() - 1) {
		    	width += fprintf(mStream, _T(", "));
			    width = MaybeNewLine(width, level+1);
		    }
	    }
	    fprintf(mStream, _T("]\n"));
    }
    
 // output normals
    if (mGenNormals && !isWire && !MeshIsAllOneSmoothingGroup(mesh)) {
        NormalTable* normTab = NULL;
        normTab = new NormalTable();
    
        for (j = 0; j < polyMesh.GetPolygonCnt(); j++) {
            //Point3 n = polyMesh.GetPolygon(j)->GetFNormal();
            for (int k = 0; k < polyMesh.GetPolygon(j)->GetVNormalCnt(); k++) {
                Point3 n = polyMesh.GetPolygon(j)->GetVNormal(k);
                normTab->AddNormal(n);
            }
        }
        
        Indent(level);
        fprintf(mStream, _T("normalPerVertex TRUE\n"));
        int index = 0;
        NormalDesc* nd;
        Indent(level);
        fprintf(mStream, _T("normal "));
        fprintf(mStream, _T("Normal { vector [\n"));
        width = CurrentWidth();
        Indent(level+1);
/*
        for (i = 0; i < polyMesh.GetPolygonCnt(); i++) {
            Point3 n = polyMesh.GetPolygon(i)->GetFNormal();
            normTab->AddNormal(n);
            width += fprintf(mStream, _T("%s, "), normPoint(n));
			width  = MaybeNewLine(width, level+1);
		}
*/

        for (i = 0; i < NORM_TABLE_SIZE; i++) {
            for (nd = normTab->Get(i); nd; nd = nd->next) {
                nd->index = index++;
                Point3 n  = nd->n / NUM_NORMS;
                width    += fprintf(mStream, _T("%s, "), normPoint(n));
			    width     = MaybeNewLine(width, level+1);
            }
		}
        fprintf(mStream, _T("] }\n"));
    
        Indent(level);
        width = CurrentWidth();
        fprintf(mStream, _T("normalIndex [\n"));
        Indent(level+1);
        width = CurrentWidth();
      
        for (i = 0; i < polyMesh.GetPolygonCnt(); i++) {
            int index;
            for (int k = 0; k < polyMesh.GetPolygon(i)->GetVNormalCnt(); k++) {
                Point3 n = polyMesh.GetPolygon(i)->GetVNormal(k);
                index    = normTab->GetIndex(n);
                width   += fprintf(mStream, _T("%d, "), index);
			    width    = MaybeNewLine(width, level+1);
		    }
		    width += fprintf(mStream, _T("-1, "));
		    width = MaybeNewLine(width, level+1);
	    }
        normTab->PrintStats(mStream);

	    fprintf(mStream, _T("]\n"));  
    
        delete normTab;
    }

    Indent(level);
    fprintf(mStream, _T("}\n"));
    delete td;
}

// Write out the data for a single triangle mesh
void
VRML2Export::OutputTriObject(INode* node, TriObject* obj, BOOL isMulti,
                             BOOL isWire, BOOL twoSided, int level,
                             int textureNum, BOOL pMirror)
{
    assert(obj);
    Mesh &mesh = obj->GetMesh();
    int numverts = mesh.getNumVerts();
    int numtverts = mesh.getNumTVerts();
    int numfaces = mesh.getNumFaces();
    int i, width;
    NormalTable* normTab = NULL;
    TextureDesc* td = NULL;
    BOOL dummy;

    Mtl *mtl = node->GetMtl();
    if (mtl && mtl->IsMultiMtl() && textureNum != -1) {
        if (mtl != NULL) {
            mtl = mtl->GetSubMtl(textureNum);
            td = GetMtlTex(mtl, dummy);
        }
    } else
        td = GetMatTex(node, dummy);
    
    if (numfaces == 0) {
        delete td;
        return;
    }
    
    Indent(level++);
    if (isWire)
        fprintf(mStream, _T("geometry DEF %s-FACES IndexedLineSet {\n"), mNodes.GetNodeName(node));
    else
        fprintf(mStream, _T("geometry DEF %s-FACES IndexedFaceSet {\n"), mNodes.GetNodeName(node));

    if (!isWire) {
        Indent(level);
        fprintf(mStream, _T("ccw %s\n"), pMirror ? _T("FALSE") : _T("TRUE"));
        Indent(level);
        fprintf(mStream, _T("solid %s\n"),
                            twoSided ? _T("FALSE") : _T("TRUE"));
    }

    if (mPreLight) {
        if (!mCPVSource) {  // 1 if MAX, 0 if we calc
            ColorTab vxColDiffTab;
            calcMixedVertexColors(node, mStart, LIGHT_SCENELIGHT, vxColDiffTab);
            int numColors = 0;
            int cfaces;
            Color c;
            Indent(level);
            fprintf(mStream, _T("colorPerVertex TRUE\n"));
            Indent(level);
            width = CurrentWidth();
            fprintf(mStream, _T("color Color { color [\n"));
            Indent(level+1);
            cfaces = vxColDiffTab.Count();
            for (i = 0; i < cfaces; i++) {
                c = *((Color*) vxColDiffTab[i]);
                if (i == cfaces - 1) width += fprintf(mStream, _T("%s "), color(c));
                else width += fprintf(mStream, _T("%s, "), color(c));
                width = MaybeNewLine(width, level+1);
            }
            Indent(level);
            fprintf(mStream, _T("] }\n"));

            Indent(level);
            fprintf(mStream, _T("colorIndex [\n"));
            width = CurrentWidth();
            Indent(level+1);

            for (i = 0; i < numfaces; i++) {
                width += fprintf(mStream, _T("%d, %d, %d, -1"),
                            mesh.faces[i].v[0], mesh.faces[i].v[1],
                            mesh.faces[i].v[2]);
                if (i != numfaces-1) {
                    width += fprintf(mStream, _T(", "));
                    width = MaybeNewLine(width, level+1);
                }
	        }
	        fprintf(mStream, _T("]\n"));

            for (i = 0; i < vxColDiffTab.Count(); i++) {
                delete (Color*) vxColDiffTab[i];
            }
            vxColDiffTab.ZeroCount();
            vxColDiffTab.Shrink();
        } else {
            int numCVerts = mesh.getNumVertCol();
            if (numCVerts) {
                VertColor vColor;
                Indent(level);
                fprintf(mStream, _T("colorPerVertex TRUE\n"));
                Indent(level);
                width = CurrentWidth();
                fprintf(mStream, _T("color Color { color [\n"));
                Indent(level+1);
             // FIXME need to add colorlist to PMesh
                for (i = 0; i < numverts; i++) {
                    vColor = mesh.vertCol[i];
                    if (i == numverts - 1)
                        width += fprintf(mStream, _T("%s "), color(vColor));
                    else
                        width += fprintf(mStream, _T("%s, "), color(vColor));
                    width = MaybeNewLine(width, level+1);
                }
                Indent(level);
                fprintf(mStream, _T("] }\n"));

                Indent(level);
                fprintf(mStream, _T("colorIndex [\n"));
                width = CurrentWidth();
                Indent(level+1);

                for (i = 0; i < numfaces; i++) {
					int id = mesh.faces[i].getMatID();
					if (textureNum == -1 || id == textureNum) {
						if (!(mesh.faces[i].flags & FACE_HIDDEN)) {
							width += fprintf(mStream, _T("%d, %d, %d, -1"),
									mesh.faces[i].v[0], mesh.faces[i].v[1],
									mesh.faces[i].v[2]);
							if (i != numfaces-1) {
								width += fprintf(mStream, _T(", "));
								width = MaybeNewLine(width, level+1);
							}
						}
                    }
	            }
	            fprintf(mStream, _T("]\n"));

            } else {
                mPreLight = FALSE;
            }
        }
    }

    int numColors = 0;
    if (!mPreLight && isMulti && textureNum == -1) {
        Color c;
        Indent(level);
        fprintf(mStream, _T("colorPerVertex FALSE\n"));
        Mtl *sub, *mtl = node->GetMtl();
        assert (mtl->IsMultiMtl());
        int num = mtl->NumSubMtls();
        Indent(level);
        width = CurrentWidth();
        fprintf(mStream, _T("color Color { color [\n"));
        Indent(level+1);
        for(i = 0; i < num; i++) {
            sub = mtl->GetSubMtl(i);
            if (!sub)
                continue;
            numColors++;
            c = sub->GetDiffuse(mStart);
            if (i == num - 1)
                width += fprintf(mStream, _T("%s "), color(c));
            else
                width += fprintf(mStream, _T("%s, "), color(c));
            width = MaybeNewLine(width, level+1);
        }
        Indent(level);
        fprintf(mStream, _T("] }\n"));
    }

    if (textureNum < 1) {
        // Output the vertices
        Indent(level);
        fprintf(mStream, _T("coord DEF %s-COORD Coordinate { point [\n"),mNodes.GetNodeName(node));
        
        width = CurrentWidth();
        Indent(level+1);
        for(i = 0; i < numverts; i++) {
            Point3 p = mesh.verts[i];
#ifdef MIRROR_BY_VERTICES
            if (pMirror)
                p = - p;
#endif
            width += fprintf(mStream, _T("%s"), point(p));
            if (i == numverts-1) {
                fprintf(mStream, _T("]\n"));
                Indent(level);
                fprintf(mStream, _T("}\n"));
            }
            else {
                width += fprintf(mStream, _T(", "));
                width = MaybeNewLine(width, level+1);
            }
        }
    } else {
        Indent(level);
        fprintf(mStream, _T("coord USE %s-COORD\n"),mNodes.GetNodeName(node));
    }
    // Output the normals
    // FIXME share normals on multi-texture objects
    if (mGenNormals && !isWire) {
        normTab = OutputNormals(mesh, level);
    }

    // Output Texture coordinates
    if (numtverts > 0 && (td || textureNum == 0) && !isWire) {
        if (textureNum < 1) {
            Indent(level);
            fprintf(mStream,
                _T("texCoord DEF %s-TEXCOORD TextureCoordinate { point [\n"),mNodes.GetNodeName(node));
            width = CurrentWidth();
            Indent(level+1);
            for(i = 0; i < numtverts; i++) {
                Point3 uvw = mesh.tVerts[i];
                UVVert p = mesh.getTVert(i);
                width += fprintf(mStream, _T("%s"), texture(p));
                if (i == numtverts-1) {
                    fprintf(mStream, _T("]\n"));
                    Indent(level);
                    fprintf(mStream, _T("}\n"));
                }
                else {
                    width += fprintf(mStream, _T(", "));
                    width = MaybeNewLine(width, level+1);
                }
            }
        } else {
            Indent(level);
            fprintf(mStream, _T("texCoord USE %s-TEXCOORD\n"),mNodes.GetNodeName(node));
        }
    }
        
    // Output the triangles
    Indent(level);
    fprintf(mStream, _T("coordIndex [\n"));
    Indent(level+1);
    width = CurrentWidth();
    for(i = 0; i < numfaces; i++) {
        int id = mesh.faces[i].getMatID();
        if (textureNum == -1 || id == textureNum) {
            if (!(mesh.faces[i].flags & FACE_HIDDEN)) {
                width += fprintf(mStream, _T("%d, %d, %d, -1"),
                                 mesh.faces[i].v[0], mesh.faces[i].v[1],
                                 mesh.faces[i].v[2]);
                if (i != numfaces-1) {
                    width += fprintf(mStream, _T(", "));
                    width = MaybeNewLine(width, level+1);
                }
            }
        }
    }
    fprintf(mStream, _T("]\n"));

    if (numtverts > 0 && td && !isWire) {
        Indent(level);
        fprintf(mStream, _T("texCoordIndex [\n"));
        Indent(level+1);
        width = CurrentWidth();
        for(i = 0; i < numfaces; i++) {
            int id = mesh.faces[i].getMatID();
            if (textureNum == -1 || id == textureNum) {
                if (!(mesh.faces[i].flags & FACE_HIDDEN)) {
                    width += fprintf(mStream, _T("%d, %d, %d, -1"),
                                     mesh.tvFace[i].t[0], mesh.tvFace[i].t[1],
                                     mesh.tvFace[i].t[2]);
                    if (i != numfaces-1) {
                        width += fprintf(mStream, _T(", "));
                        width = MaybeNewLine(width, level+1);
                    }
                }
            }
        }
        fprintf(mStream, _T("]\n"));
    }

    if (!mPreLight && isMulti && numColors > 0 && textureNum == -1) {
        Indent(level);
        fprintf(mStream, _T("colorIndex [\n"));
        width = CurrentWidth();
        Indent(level+1);
        for(i = 0; i < numfaces; i++) {
            if (!(mesh.faces[i].flags & FACE_HIDDEN)) {
                int id = mesh.faces[i].getMatID();
                id = (id % numColors);   // this is the way MAX does it
                /*
                if (id >= numColors)
                    id = 0;
                */
                width += fprintf(mStream, _T("%d"), id);
                if (i != numfaces-1) {
                    width += fprintf(mStream, _T(", "));
                    width = MaybeNewLine(width, level+1);
                }
            }
        }
        fprintf(mStream, _T("]\n"));
    }
    if (mGenNormals && normTab && !isWire) {
        OutputNormalIndices(mesh, normTab, level, textureNum);
        delete normTab;
    }
        
    Indent(level);
    fprintf(mStream, _T("}\n"));
    delete td;

}

BOOL
VRML2Export::HasTexture(INode* node, BOOL &isWire)
{
    TextureDesc* td = GetMatTex(node, isWire);
    if (!td)
        return FALSE;
    delete td;
    return TRUE;
}

TSTR
VRML2Export::PrefixUrl(TSTR& fileName)
{
    if (mUsePrefix && mUrlPrefix.Length() > 0) {
        if (mUrlPrefix[mUrlPrefix.Length() - 1] != '/') {
            TSTR slash = "/";
            return mUrlPrefix + slash + fileName;
        } else
            return mUrlPrefix + fileName;
    }
    else
        return fileName;
}
    
// Get the name of the texture file
TextureDesc*
VRML2Export::GetMatTex(INode* node, BOOL& isWire)
{
    Mtl* mtl = node->GetMtl();
    return GetMtlTex(mtl, isWire);
}


TextureDesc*
VRML2Export::GetMtlTex(Mtl* mtl, BOOL& isWire)
{
    if (!mtl)
        return NULL;

    
    if (mtl->ClassID() != Class_ID(DMTL_CLASS_ID, 0))
        return NULL;

    StdMat* sm = (StdMat*) mtl;

    isWire = sm->GetWire();

    // Check for texture map
    Texmap* tm = (BitmapTex*) sm->GetSubTexmap(ID_DI);
    if (!tm)
        return NULL;

    if (tm->ClassID() != Class_ID(BMTEX_CLASS_ID, 0))
        return NULL;
    BitmapTex* bm = (BitmapTex*) tm;

    TSTR bitmapFile;
    TSTR fileName;

    bitmapFile = bm->GetMapName();
    if (bitmapFile.data() == NULL)
        return NULL;
    int l = strlen(bitmapFile)-1;
    if (l < 0)
        return NULL;
    
    TSTR path;
    SplitPathFile(bitmapFile, &path, &fileName);

    TSTR url = PrefixUrl(fileName);
    TextureDesc* td = new TextureDesc(bm, fileName, url);
    return td;
}

BOOL
VRML2Export::OutputMaterial(INode* node, BOOL& isWire, BOOL& twoSided,
                            int level, int textureNum)
{
    Mtl* mtl = node->GetMtl();
    BOOL isMulti = FALSE;
    isWire = FALSE;
    twoSided = FALSE;

    Indent(level++);
    fprintf(mStream, _T("appearance Appearance {\n"));

    if (mtl && mtl->IsMultiMtl()) {
        if (textureNum > -1)
            mtl = mtl->GetSubMtl(textureNum);
        else
            mtl = mtl->GetSubMtl(0);
        isMulti = TRUE;
        // Use first material for specular, etc.
    }

    // If no material is assigned, use the wire color
    if (!mtl || (mtl->ClassID() != Class_ID(DMTL_CLASS_ID, 0) &&
                 mtl->ClassID() != Class_ID(0x3e0810d6, 0x603532f0))) {
        Color col(node->GetWireColor());
        Indent(level);
        fprintf(mStream, _T("material "));
        fprintf(mStream, _T("Material {\n"));
        Indent(level+1);
        fprintf(mStream, _T("diffuseColor %s\n"), color(col));
//        Indent(level+1);
//        fprintf(mStream, _T("specularColor .9 .9 .9\n"));
//        fprintf(mStream, _T("specularColor %s\n"), color(col));
        Indent(level);
        fprintf(mStream, _T("}\n"));
        Indent(--level);
        fprintf(mStream, _T("}\n"));
        return FALSE;
    }

    StdMat* sm = (StdMat*) mtl;
    isWire = sm->GetWire();
    twoSided = sm->GetTwoSided();

    Interval i = FOREVER;
    sm->Update(0, i);
    Indent(level);
    fprintf(mStream, _T("material Material {\n"));
    Color c;

    Indent(level+1);
    c = sm->GetDiffuse(mStart);
    fprintf(mStream, _T("diffuseColor %s\n"), color(c));
#if 1
    Indent(level+1);
	float difin = (c.r+c.g+c.b) / 3.0f;
    c = sm->GetAmbient(mStart);
	float ambin = (c.r+c.g+c.b) / 3.0f;
	if (ambin >= difin)
		fprintf(mStream, _T("ambientIntensity 1.0\n"));
	else
		fprintf(mStream, _T("ambientIntensity %s\n"), floatVal(ambin/difin));
    Indent(level+1);
    c = sm->GetSpecular(mStart);
    c *= sm->GetShinStr(mStart);
    fprintf(mStream, _T("specularColor %s\n"), color(c));
#endif
    float sh = sm->GetShininess(mStart);
    sh = sh * 0.95f + 0.05f;
    Indent(level+1);
    fprintf(mStream, _T("shininess %s\n"), floatVal(sh));
    Indent(level+1);
    fprintf(mStream, _T("transparency %s\n"),
            floatVal(1.0f - sm->GetOpacity(mStart)));
    float si = sm->GetSelfIllum(mStart);
    if (si > 0.0f) {
        Indent(level+1);
        c = sm->GetDiffuse(mStart);
        Point3 p = si*Point3(c.r, c.g, c.b);
        fprintf(mStream, _T("emissiveColor %s\n"), color(p));
    }
    Indent(level);
    fprintf(mStream, _T("}\n"));

    if (isMulti && textureNum == -1) {
        Indent(--level);
        fprintf(mStream, _T("}\n"));
        return TRUE;
    }

    BOOL dummy;
    TextureDesc* td = GetMtlTex(mtl, dummy);
    if (!td) {
        Indent(--level);
        fprintf(mStream, _T("}\n"));
        return FALSE;
    }

    Indent(level);
    fprintf(mStream, _T("texture ImageTexture {\n"));
    Indent(level+1);
    fprintf(mStream, _T("url \"%s\"\n"), td->url);
    // fprintf(mStream, _T("repeatS TRUE\n"));
    // fprintf(mStream, _T("repeatT TRUE\n"));
    Indent(level);
    fprintf(mStream, _T("}\n"));

    BitmapTex* bm = td->tex;
    delete td;

    StdUVGen* uvGen = bm->GetUVGen();
    if (!uvGen)
        return FALSE;

    float uOff = uvGen->GetUOffs(mStart);
    float vOff = uvGen->GetVOffs(mStart);
    float uScl = uvGen->GetUScl(mStart);
    float vScl = uvGen->GetVScl(mStart);
    float ang =  uvGen->GetAng(mStart);

    if (uOff == 0.0f && vOff == 0.0f && uScl == 1.0f && vScl == 1.0f &&
        ang == 0.0f) {
        Indent(--level);
        fprintf(mStream, _T("}\n"));
        return FALSE;
    }

    Indent(level);
    fprintf(mStream, _T("textureTransform TextureTransform {\n"));
    Indent(level+1);
    fprintf(mStream, _T("center 0.5 0.5\n"));
    if (uOff != 0.0f || vOff != 0.0f) {
        Indent(level+1);
        UVVert uv = UVVert(uOff+0.5f, vOff+0.5f, 0.0f);
        fprintf(mStream, _T("translation %s\n"), texture(uv));
    }
    if (ang != 0.0f) {
        Indent(level+1);
        fprintf(mStream, _T("rotation %s\n"), floatVal(ang));
    }
    if (uScl != 1.0f || vScl != 1.0f) {
        Indent(level+1);
        UVVert uv = UVVert(uScl, vScl, 0.0f);
        fprintf(mStream, _T("scale %s\n"), texture(uv));
    }
    Indent(level);
    fprintf(mStream, _T("}\n"));

    Indent(--level);
    fprintf(mStream, _T("}\n"));
    return FALSE;
}

BOOL
VRML2Export::VrmlOutSphereTest(INode * node, Object *obj)
{
    SimpleObject* so = (SimpleObject*) obj;
    float hemi;
    int basePivot, genUV, smooth;
    BOOL isWire = FALSE;
    BOOL td = HasTexture(node, isWire);

    if (isWire)
        return FALSE;

    // Reject "base pivot" mapped, non-smoothed and hemisphere spheres
    so->pblock->GetValue(SPHERE_RECENTER, mStart, basePivot, FOREVER);
    so->pblock->GetValue(SPHERE_GENUVS, mStart, genUV, FOREVER);
    so->pblock->GetValue(SPHERE_HEMI, mStart, hemi, FOREVER);
    so->pblock->GetValue(SPHERE_SMOOTH, mStart, smooth, FOREVER);
    if (!smooth || basePivot || (genUV && td) || hemi > 0.0f)
        return FALSE;
    return TRUE;
}

BOOL
VRML2Export::VrmlOutSphere(INode * node, Object *obj, int level)
{
    SimpleObject* so = (SimpleObject*) obj;
    float radius, hemi;
    int basePivot, genUV, smooth;
    BOOL isWire = FALSE;
    BOOL td = HasTexture(node, isWire);

    if (isWire)
        return FALSE;

    // Reject "base pivot" mapped, non-smoothed and hemisphere spheres
    so->pblock->GetValue(SPHERE_RECENTER, mStart, basePivot, FOREVER);
    so->pblock->GetValue(SPHERE_GENUVS, mStart, genUV, FOREVER);
    so->pblock->GetValue(SPHERE_HEMI, mStart, hemi, FOREVER);
    so->pblock->GetValue(SPHERE_SMOOTH, mStart, smooth, FOREVER);
    if (!smooth || basePivot || (genUV && td) || hemi > 0.0f)
        return FALSE;

    so->pblock->GetValue(SPHERE_RADIUS, mStart, radius, FOREVER);
    
    Indent(level);

    fprintf(mStream, _T("geometry "));
    
    fprintf(mStream, _T("Sphere { radius %s }\n"), floatVal(radius));
 
    return TRUE;
}

BOOL
VRML2Export::VrmlOutCylinderTest(INode* node, Object *obj)
{
    SimpleObject* so = (SimpleObject*) obj;
    int sliceOn, genUV, smooth;
    BOOL isWire = FALSE;
    BOOL td = HasTexture(node, isWire);

    if (isWire)
        return FALSE;

    // Reject sliced, non-smooth and mapped cylinders
    so->pblock->GetValue(CYLINDER_GENUVS, mStart, genUV, FOREVER);
    so->pblock->GetValue(CYLINDER_SLICEON, mStart, sliceOn, FOREVER);
    so->pblock->GetValue(CYLINDER_SMOOTH, mStart, smooth, FOREVER);
    if (sliceOn || (genUV && td) || !smooth)
        return FALSE;
    return TRUE;
}

BOOL
VRML2Export::VrmlOutCylinderTform(INode* node, Object *obj, int level,
                                  BOOL mirrored)
{
    if (!VrmlOutCylinderTest(node, obj))
        return FALSE;

    float height;
    SimpleObject* so = (SimpleObject*) obj;
    so->pblock->GetValue(CYLINDER_HEIGHT, mStart, height, FOREVER);
#ifdef MIRROR_BY_VERTICES
    if (mirrored)
        height = - height;
#endif

    Indent(level);
    fprintf(mStream, _T("Transform {\n"));
    if (mZUp) {
        Indent(level+1);
        fprintf(mStream, _T("rotation 1 0 0 %s\n"),
                floatVal(float(PI/2.0)));
        Indent(level+1);
        fprintf(mStream, _T("translation 0 0 %s\n"),
                floatVal(float(height/2.0)));
    } else {
        Point3 p = Point3(0.0f, 0.0f, height/2.0f);
        Indent(level+1);
        fprintf(mStream, _T("translation %s\n"), point(p));
    }
    Indent(level+1);
    fprintf(mStream, _T("children [\n"));
    return TRUE;
}

BOOL
VRML2Export::VrmlOutCylinder(INode* node, Object *obj, int level)
{
    SimpleObject* so = (SimpleObject*) obj;
    float radius, height;
    int sliceOn, genUV, smooth;
    BOOL isWire = FALSE;
    BOOL td = HasTexture(node, isWire);

    if (isWire)
        return FALSE;

    // Reject sliced, non-smooth and mapped cylinders
    so->pblock->GetValue(CYLINDER_GENUVS, mStart, genUV, FOREVER);
    so->pblock->GetValue(CYLINDER_SLICEON, mStart, sliceOn, FOREVER);
    so->pblock->GetValue(CYLINDER_SMOOTH, mStart, smooth, FOREVER);
    if (sliceOn || (genUV && td) || !smooth)
        return FALSE;

    so->pblock->GetValue(CYLINDER_RADIUS, mStart, radius, FOREVER);
    so->pblock->GetValue(CYLINDER_HEIGHT, mStart, height, FOREVER);
    Indent(level);
    fprintf(mStream, _T("geometry "));
    fprintf(mStream, _T("Cylinder { radius %s "), floatVal(radius));
    fprintf(mStream, _T("height %s }\n"), floatVal(float(fabs(height))));
    
    return TRUE;
}

BOOL
VRML2Export::VrmlOutConeTest(INode* node, Object *obj)
{
    SimpleObject* so = (SimpleObject*) obj;
    float radius2;
    int sliceOn, genUV, smooth;
    BOOL isWire = FALSE;
    BOOL td = HasTexture(node, isWire);

    if (isWire)
        return FALSE;

    // Reject sliced, non-smooth and mappeded cylinders
    so->pblock->GetValue(CONE_GENUVS, mStart, genUV, FOREVER);
    so->pblock->GetValue(CONE_SLICEON, mStart, sliceOn, FOREVER);
    so->pblock->GetValue(CONE_SMOOTH, mStart, smooth, FOREVER);
    so->pblock->GetValue(CONE_RADIUS2, mStart, radius2, FOREVER);
    if (sliceOn || (genUV &&td) || !smooth || radius2 > 0.0f)
        return FALSE;
    return TRUE;
}

BOOL
VRML2Export::VrmlOutConeTform(INode* node, Object *obj, int level,
                              BOOL mirrored)
{
    if (!VrmlOutConeTest(node, obj))
        return FALSE;
    Indent(level);
    fprintf(mStream, _T("Transform {\n"));

    float height;
    SimpleObject* so = (SimpleObject*) obj;
    so->pblock->GetValue(CONE_HEIGHT, mStart, height, FOREVER);
#ifdef MIRROR_BY_VERTICES
    if (mirrored)
        height = - height;
#endif

    if (mZUp) {
        Indent(level+1);
        if (height > 0.0f)
            fprintf(mStream, _T("rotation 1 0 0 %s\n"),
                    floatVal(float(PI/2.0)));
        else
            fprintf(mStream, _T("rotation 1 0 0 %s\n"),
                    floatVal(float(-PI/2.0)));
        Indent(level+1);
        fprintf(mStream, _T("translation 0 0 %s\n"),
                floatVal(float(fabs(height)/2.0)));
    } else {
        Point3 p = Point3(0.0f, 0.0f, (float)height/2.0f);
        if (height < 0.0f) {
            Indent(level+1);
            fprintf(mStream, _T("rotation 1 0 0 %s\n"),
                    floatVal(float(PI)));
        }
        Indent(level+1);
        fprintf(mStream, _T("translation %s\n"), point(p));
    }

    Indent(level+1);
    fprintf(mStream, _T("children [\n"));
    return TRUE;
}

BOOL
VRML2Export::VrmlOutCone(INode* node, Object *obj, int level)
{
    SimpleObject* so = (SimpleObject*) obj;
    float radius1, radius2, height;
    int sliceOn, genUV, smooth;
    BOOL isWire = FALSE;
    BOOL td = HasTexture(node, isWire);

    if (isWire)
        return FALSE;

    // Reject sliced, non-smooth and mappeded cylinders
    so->pblock->GetValue(CONE_GENUVS, mStart, genUV, FOREVER);
    so->pblock->GetValue(CONE_SLICEON, mStart, sliceOn, FOREVER);
    so->pblock->GetValue(CONE_SMOOTH, mStart, smooth, FOREVER);
    so->pblock->GetValue(CONE_RADIUS2, mStart, radius2, FOREVER);
    if (sliceOn || (genUV &&td) || !smooth || radius2 > 0.0f)
        return FALSE;

    so->pblock->GetValue(CONE_RADIUS1, mStart, radius1, FOREVER);
    so->pblock->GetValue(CONE_HEIGHT, mStart, height, FOREVER);
    Indent(level);
    
    fprintf(mStream, _T("geometry "));

    fprintf(mStream, _T("Cone { bottomRadius %s "), floatVal(radius1));
    fprintf(mStream, _T("height %s }\n"), floatVal(float(fabs(height))));
    
    return TRUE;
}

BOOL
VRML2Export::VrmlOutCubeTest(INode* node, Object *obj)
{
    Mtl* mtl = node->GetMtl();
    // Multi materials need meshes
    if (mtl && mtl->IsMultiMtl())
        return FALSE;

    SimpleObject* so = (SimpleObject*) obj;
    BOOL isWire = FALSE;
    BOOL td = HasTexture(node, isWire);

    if (isWire)
        return FALSE;

    int genUV, lsegs, wsegs, hsegs;
    so->pblock->GetValue(BOXOBJ_GENUVS, mStart, genUV, FOREVER);
    so->pblock->GetValue(BOXOBJ_LSEGS,  mStart, lsegs, FOREVER);
    so->pblock->GetValue(BOXOBJ_WSEGS,  mStart, hsegs, FOREVER);
    so->pblock->GetValue(BOXOBJ_HSEGS,  mStart, wsegs, FOREVER);
    if ((genUV && td) || lsegs > 1 || hsegs > 1 || wsegs > 1)
        return FALSE;

    return TRUE;
}

BOOL
VRML2Export::VrmlOutCubeTform(INode* node, Object *obj, int level,
                              BOOL mirrored)
{
    if (!VrmlOutCubeTest(node, obj))
        return FALSE;
    Indent(level);
    fprintf(mStream, _T("Transform {\n"));

    float height;
    SimpleObject* so = (SimpleObject*) obj;
    so->pblock->GetValue(BOXOBJ_HEIGHT, mStart, height, FOREVER);
#ifdef MIRROR_BY_VERTICES
    if (mirrored)
        height = - height;
#endif

    Point3 p = Point3(0.0f,0.0f,height/2.0f);
    // VRML cubes grow from the middle, MAX grows from z=0
    Indent(level+1);
    fprintf(mStream, _T("translation %s\n"), point(p));

    Indent(level+1);
    fprintf(mStream, _T("children [\n"));
    return TRUE;
}

BOOL
VRML2Export::VrmlOutCube(INode* node, Object *obj, int level)
{
    Mtl* mtl = node->GetMtl();
    // Multi materials need meshes
    if (mtl && mtl->IsMultiMtl())
        return FALSE;

    SimpleObject* so = (SimpleObject*) obj;
    float length, width, height;
    BOOL isWire = FALSE;
    BOOL td = HasTexture(node, isWire);

    if (isWire)
        return FALSE;

    int genUV, lsegs, wsegs, hsegs;
    so->pblock->GetValue(BOXOBJ_GENUVS, mStart, genUV, FOREVER);
    so->pblock->GetValue(BOXOBJ_LSEGS,  mStart, lsegs, FOREVER);
    so->pblock->GetValue(BOXOBJ_WSEGS,  mStart, hsegs, FOREVER);
    so->pblock->GetValue(BOXOBJ_HSEGS,  mStart, wsegs, FOREVER);
    if ((genUV && td) || lsegs > 1 || hsegs > 1 || wsegs > 1)
        return FALSE;

    so->pblock->GetValue(BOXOBJ_LENGTH, mStart, length, FOREVER);
    so->pblock->GetValue(BOXOBJ_WIDTH, mStart,  width,  FOREVER);
    so->pblock->GetValue(BOXOBJ_HEIGHT, mStart, height, FOREVER);
    Indent(level);
    fprintf(mStream, _T("geometry "));
    if (mZUp) {
        fprintf(mStream, _T("Box { size %s "),
                floatVal(float(fabs(width))));
        fprintf(mStream, _T("%s "),
                floatVal(float(fabs(length))));
        fprintf(mStream, _T("%s }\n"),
                floatVal(float(fabs(height))));
    } else {
        fprintf(mStream, _T("Box { size %s "),
                floatVal(float(fabs(width))));
        fprintf(mStream, _T("%s "),
                floatVal(float(fabs(height))));
        fprintf(mStream, _T("%s }\n"),
                floatVal(float(fabs(length))));
    }

    return TRUE;
}

#define INTENDED_ASPECT_RATIO 1.3333

BOOL
VRML2Export::VrmlOutCamera(INode* node, Object* obj, int level)
{
    // Code stolen from render/rend.cpp

    // compute camera transform
    ViewParams vp;
    CameraState cs;
    Interval iv;
    CameraObject *cam = (CameraObject *)obj;
    cam->EvalCameraState(0, iv, &cs);
    vp.fov = (float)(2.0 * atan(tan(cs.fov / 2.0) / INTENDED_ASPECT_RATIO));

    Indent(level);
    fprintf(mStream, _T("DEF %s Viewpoint {\n"), mNodes.GetNodeName(node));
    Indent(level+1);
    fprintf(mStream, _T("position 0 0 0\n"));
    Indent(level+1);
    fprintf(mStream, _T("fieldOfView %s\n"), floatVal(vp.fov));
    Indent(level);
    fprintf(mStream, _T("}\n"));

    return TRUE;
}

#define FORDER(A, B) if (B < A) { float fOO = A; A = B; B = fOO; }

BOOL
VRML2Export::VrmlOutSound(INode* node, SoundObject* obj, int level)
{
    float intensity, priority, minBack, maxBack, minFront, maxFront;
    int spatialize;
    
    obj->pblock->GetValue(PB_SND_INTENSITY, mStart, intensity, FOREVER);
    obj->pblock->GetValue(PB_SND_PRIORITY, mStart, priority, FOREVER);
    obj->pblock->GetValue(PB_SND_SPATIALIZE, mStart, spatialize, FOREVER);
    obj->pblock->GetValue(PB_SND_MIN_BACK, mStart, minBack, FOREVER);
    obj->pblock->GetValue(PB_SND_MAX_BACK, mStart, maxBack, FOREVER);
    obj->pblock->GetValue(PB_SND_MIN_FRONT, mStart, minFront, FOREVER);
    obj->pblock->GetValue(PB_SND_MAX_FRONT, mStart, maxFront, FOREVER);

    Point3 dir(0, -1, 0);

    FORDER(minBack, maxBack);
    FORDER(minFront, maxFront);
    if (minFront < minBack) {
        float temp = minFront;
        minFront = minBack;
        minBack = temp;
        temp = maxFront;
        maxFront = maxBack;
        maxBack = temp;
        dir = - dir;
    }

    Indent(level);
    fprintf(mStream, _T("DEF %s Sound {\n"), mNodes.GetNodeName(node));
    Indent(level+1);
    fprintf(mStream, _T("direction %s\n"), point(dir));
    Indent(level+1);
    fprintf(mStream, _T("intensity %s\n"), floatVal(intensity));
    Indent(level+1);
    fprintf(mStream, _T("location 0 0 0\n"));
    Indent(level+1);
    fprintf(mStream, _T("maxBack %s\n"), floatVal(maxBack));
    Indent(level+1);
    fprintf(mStream, _T("maxFront %s\n"), floatVal(maxFront));
    Indent(level+1);
    fprintf(mStream, _T("minBack %s\n"), floatVal(minBack));
    Indent(level+1);
    fprintf(mStream, _T("minFront %s\n"), floatVal(minFront));
    Indent(level+1);
    fprintf(mStream, _T("priority %s\n"), floatVal(priority));
    Indent(level+1);
    fprintf(mStream, _T("spatialize %s\n"),
            spatialize ? _T("TRUE") : _T("FALSE"));
    if (obj->audioClip) {
        Indent(level+1);
//        fprintf(mStream, _T("source USE %s\n"), VRMLName(obj->audioClip->GetName()));
        fprintf(mStream, _T("source\n"));
        VrmlOutAudioClip(level + 2, obj->audioClip);
    }
    Indent(level);
    fprintf(mStream, _T("}\n"));

    return TRUE;
}

static INode *
GetTopLevelParent(INode* node)
{
    while(!node->GetParentNode()->IsRootNode())
        node = node->GetParentNode();
    return node;
}

BOOL
VRML2Export::VrmlOutTouchSensor(INode* node, int level)
{
    TouchSensorObject* obj = (TouchSensorObject*)
        node->EvalWorldState(mStart).obj;
    int enabled;
    obj->pblock->GetValue(PB_TS_ENABLED, mStart, enabled, FOREVER);
    Indent(level);
    fprintf(mStream, _T("DEF %s-SENSOR TouchSensor { enabled %s }\n"),mNodes.GetNodeName(node),
            enabled ? _T("TRUE") : _T("FALSE"));

    TCHAR* vrmlObjName = NULL;
    vrmlObjName = VrmlParent(node);
    INode *otop = NULL;
    int size = obj->objects.Count();
    for(int i=0; i < size; i++) {
        TouchSensorObj* animObj = obj->objects[i];
        Object *o = animObj->node->EvalWorldState(mStart).obj;
        if (!o)
            break;
        assert(vrmlObjName);
        if (IsAimTarget(animObj->node))
            break;
        INode* top;
        if (o->ClassID() == TimeSensorClassID)
            top = animObj->node;
        else
            top = GetTopLevelParent(animObj->node);
        ObjectBucket* ob =
            mObjTable.AddObject(top->EvalWorldState(mStart).obj);
        if (top != otop) {
            AddAnimRoute(vrmlObjName, ob->name.data(), node, top);
            AddCameraAnimRoutes(vrmlObjName, node, top);
            otop = top;
        }
    }
    return TRUE;
}

BOOL
VRML2Export::VrmlOutProxSensor(INode* node, ProxSensorObject* obj,
                                int level)
{
    int enabled;
    float length, width, height;

    obj->pblock->GetValue(PB_PS_ENABLED, mStart, enabled, FOREVER);
    obj->pblock->GetValue(PB_PS_LENGTH, mStart, length, FOREVER);
    obj->pblock->GetValue(PB_PS_WIDTH, mStart, width, FOREVER);
    obj->pblock->GetValue(PB_PS_HEIGHT, mStart, height, FOREVER);
    Indent(level);
    fprintf(mStream, _T("DEF %s ProximitySensor {\n"),mNodes.GetNodeName(node));
    Indent(level+1);
    fprintf(mStream, _T("enabled %s\n"),
            enabled ? _T("TRUE") : _T("FALSE"));
    Indent(level+1);
    Point3 center(0.0f, 0.0f, height/2.0f);
    fprintf(mStream, _T("center %s\n"), point(center));
    Indent(level+1);
    Point3 size(width, length, height);
    fprintf(mStream, _T("size %s\n"), scalePoint(size));
    Indent(level);
    fprintf(mStream, _T("}\n"));

    TCHAR* vrmlObjName = NULL;
    vrmlObjName = VrmlParent(node);
    INode *otop = NULL;
    int numObjs = obj->objects.Count();
    for(int i=0; i < numObjs; i++) {
        ProxSensorObj* animObj = obj->objects[i];
        Object *o = animObj->node->EvalWorldState(mStart).obj;
        if (!o)
            break;
        assert(vrmlObjName);
        if (IsAimTarget(animObj->node))
            break;
        INode* top;
        if (o->ClassID() == TimeSensorClassID)
            top = animObj->node;
        else
            top = GetTopLevelParent(animObj->node);
        ObjectBucket* ob =
            mObjTable.AddObject(top->EvalWorldState(mStart).obj);
        if (top != otop) {
            AddAnimRoute(vrmlObjName, ob->name.data(), node, top);
            AddCameraAnimRoutes(vrmlObjName, node, top);
            otop = top;
        }
    }
    return TRUE;
}

BOOL
VRML2Export::VrmlOutBillboard(INode* node, Object* obj, int level)
{
    BillboardObject* bb = (BillboardObject*) obj;
    int screenAlign;
    bb->pblock->GetValue(PB_BB_SCREEN_ALIGN, mStart, screenAlign, FOREVER);
    Indent(level);
    fprintf(mStream, _T("DEF %s Billboard {\n"), mNodes.GetNodeName(node));
    if (screenAlign) {
        Indent(level+1);
        fprintf(mStream, _T("axisOfRotation 0 0 0\n"));
    } else {
        Point3 axis(0, 0, 1);
        Indent(level+1);
        fprintf(mStream, _T("axisOfRotation %s\n"), point(axis));
    }
    Indent(level+1);
    fprintf(mStream, _T("children [\n"));
    
    return TRUE;
}

void
VRML2Export::VrmlOutTimeSensor(INode* node, TimeSensorObject* obj, int level)
{
    int start, end, duration, loop, startOnLoad;
    int animEnd = mIp->GetAnimRange().End();
    obj->pblock->GetValue(PB_START_TIME, mStart, start, FOREVER);
    obj->pblock->GetValue(PB_STOP_TIME,  mStart, end, FOREVER);
    obj->pblock->GetValue(PB_LOOP,  mStart, loop, FOREVER);
    obj->pblock->GetValue(PB_START_ON_LOAD,  mStart, startOnLoad, FOREVER);
    obj->needsScript = start != mStart || end != animEnd;
    duration = end - start;
    Indent(level);
    fprintf(mStream, _T("DEF %s-TIMER TimeSensor {\n"),mNodes.GetNodeName(node));
    Indent(level+1);
    fprintf(mStream, _T("cycleInterval %s\n"),
            floatVal(float(duration) / float(TIME_TICKSPERSEC)));
    Indent(level+1);
    fprintf(mStream, _T("loop %s\n"), loop ? _T("TRUE") : _T("FALSE"));
    Indent(level+1);
    if (startOnLoad)
        fprintf(mStream, _T("startTime 1\n"));
    else
        fprintf(mStream, _T("stopTime 1\n"));
    fprintf(mStream, _T("}\n"));
    if (obj->needsScript) {
        fprintf(mStream, _T("DEF %s-SCRIPT Script {\n"),mNodes.GetNodeName(node));
        Indent(1);
        fprintf(mStream, _T("eventIn SFFloat fractionIn\n"));
        Indent(1);
        fprintf(mStream, _T("eventOut SFFloat fractionOut\n"));
        Indent(1);
        fprintf(mStream, _T("url \"javascript:\n"));
        Indent(2);
        fprintf(mStream, _T("function fractionIn(i) {\n"));
        Indent(2);
        float fract = (float(end) - float(start))/
                       (float(animEnd) - float(mStart));
        float offset = (start - mStart) /
                       (float(animEnd) - float(mStart));
        fprintf(mStream, _T("fractionOut = %s * i"), floatVal(fract));
        if (offset != 0.0f)
            fprintf(mStream, _T(" + %s;\n"), floatVal(offset));
        else
            fprintf(mStream, _T(";\n"));
        Indent(1);
        fprintf(mStream, _T("}\"\n"));
        fprintf(mStream, _T("}\n"));
    }
}
        

BOOL
VRML2Export::VrmlOutPointLight(INode* node, LightObject* light, int level)
{
    LightState ls;
    Interval iv = FOREVER;

    light->EvalLightState(mStart, iv, &ls);

    Indent(level);
    fprintf(mStream, _T("DEF %s-LIGHT PointLight {\n"), mNodes.GetNodeName(node));
    Indent(level+1);
    fprintf(mStream, _T("intensity %s\n"),
            floatVal(light->GetIntensity(mStart, FOREVER)));
    Indent(level+1);
    Point3 col = light->GetRGBColor(mStart, FOREVER);
    fprintf(mStream, _T("color %s\n"), color(col));
    Indent(level+1);
    fprintf(mStream, _T("location 0 0 0\n"));

    Indent(level+1);
    fprintf(mStream, _T("on %s\n"), ls.on ? _T("TRUE") : _T("FALSE"));
    Indent(level+1);
    fprintf(mStream, _T("radius %s\n"), floatVal(ls.attenEnd));
    if (ls.useAtten) {
        Indent(level+1);
        fprintf(mStream, _T("attenuation 0 1 0\n"));
    }
    Indent(level);
    fprintf(mStream, _T("}\n"));
    return TRUE;
}

BOOL
VRML2Export::VrmlOutDirectLight(INode* node, LightObject* light, int level)
{
    Point3 dir(0,0,-1);

    LightState ls;
    Interval iv = FOREVER;

    light->EvalLightState(mStart, iv, &ls);

    Indent(level);
    fprintf(mStream, _T("DEF %s-LIGHT DirectionalLight {\n"), mNodes.GetNodeName(node));
    Indent(level+1);
    fprintf(mStream, _T("intensity %s\n"),
            floatVal(light->GetIntensity(mStart, FOREVER)));
    Indent(level+1);
    fprintf(mStream, _T("direction %s\n"), normPoint(dir));
    Indent(level+1);
    Point3 col = light->GetRGBColor(mStart, FOREVER);

    fprintf(mStream, _T("color %s\n"), color(col));

    Indent(level+1);
    fprintf(mStream, _T("on %s\n"), ls.on ? _T("TRUE") : _T("FALSE"));
    Indent(level);
    fprintf(mStream, _T("}\n"));
    return TRUE;
}

BOOL
VRML2Export::VrmlOutSpotLight(INode* node, LightObject* light, int level)
{
    LightState ls;
    Interval iv = FOREVER;

    Point3 dir(0,0,-1);

    light->EvalLightState(mStart, iv, &ls);
    Indent(level);
    fprintf(mStream, _T("DEF %s-LIGHT SpotLight {\n"), mNodes.GetNodeName(node));
    Indent(level+1);
    fprintf(mStream, _T("intensity %s\n"),
            floatVal(light->GetIntensity(mStart,FOREVER)));
    Indent(level+1);
    Point3 col = light->GetRGBColor(mStart, FOREVER);
    fprintf(mStream, _T("color %s\n"), color(col));
    Indent(level+1);
    fprintf(mStream, _T("location 0 0 0\n"));
    Indent(level+1);
    fprintf(mStream, _T("direction %s\n"), normPoint(dir));
    Indent(level+1);
    fprintf(mStream, _T("cutOffAngle %s\n"),
            floatVal(DegToRad(ls.fallsize)));
    Indent(level+1);
    fprintf(mStream, _T("beamWidth %s\n"), floatVal(DegToRad(ls.hotsize)));
    Indent(level+1);
    fprintf(mStream, _T("on %s\n"), ls.on ? _T("TRUE") : _T("FALSE"));
    Indent(level+1);
    fprintf(mStream, _T("radius %s\n"), floatVal(ls.attenEnd));
    if (ls.useAtten) {
        Indent(level+1);
        fprintf(mStream, _T("attenuation 0 1 0\n"));
    }
    Indent(level);
    fprintf(mStream, _T("}\n"));
    return TRUE;
}

BOOL
VRML2Export::VrmlOutTopPointLight(INode* node, LightObject* light)
{
    LightState ls;
    Interval iv = FOREVER;

    light->EvalLightState(mStart, iv, &ls);

    fprintf(mStream, _T("DEF %s PointLight {\n"), mNodes.GetNodeName(node));
    Indent(1);
    fprintf(mStream, _T("intensity %s\n"),
            floatVal(light->GetIntensity(mStart, FOREVER)));
    Indent(1);
    Point3 col = light->GetRGBColor(mStart, FOREVER);
    fprintf(mStream, _T("color %s\n"), color(col));
    Indent(1);
    Point3 p = node->GetObjTMAfterWSM(mStart).GetTrans();
    fprintf(mStream, _T("location %s\n"), point(p));

    Indent(1);
    fprintf(mStream, _T("on %s\n"), ls.on ? _T("TRUE") : _T("FALSE"));
    Indent(1);
    float radius;
    if (!ls.useAtten || ls.attenEnd == 0.0f)
        radius = Length(mBoundBox.Width());
    else
        radius = ls.attenEnd;
    fprintf(mStream, _T("radius %s\n"), floatVal(radius));
    if (ls.useAtten) {
        Indent(1);
        fprintf(mStream, _T("attenuation 0 1 0\n"));
    }
    fprintf(mStream, _T("}\n"));
    return TRUE;
}

BOOL
VRML2Export::VrmlOutTopDirectLight(INode* node, LightObject* light)
{
    LightState ls;
    Interval iv = FOREVER;

    light->EvalLightState(mStart, iv, &ls);

    fprintf(mStream, _T("DEF %s DirectionalLight {\n"), mNodes.GetNodeName(node));
    Indent(1);
    fprintf(mStream, _T("intensity %s\n"),
            floatVal(light->GetIntensity(mStart, FOREVER)));
    Indent(1);
    Point3 col = light->GetRGBColor(mStart, FOREVER);
    fprintf(mStream, _T("color %s\n"), color(col));
    Point3 p = Point3(0,0,-1);

    Matrix3 tm = node->GetObjTMAfterWSM(mStart);
    Point3 trans, s;
    Quat q;
    AffineParts parts;
    decomp_affine(tm, &parts);
    q = parts.q;
    Matrix3 rot;
    q.MakeMatrix(rot);
    p = p * rot;
    
    Indent(1);
    fprintf(mStream, _T("direction %s\n"), point(p));
    Indent(1);
    fprintf(mStream, _T("on %s\n"), ls.on ? _T("TRUE") : _T("FALSE"));
    fprintf(mStream, _T("}\n"));
    return TRUE;
}

BOOL
VRML2Export::VrmlOutTopSpotLight(INode* node, LightObject* light)
{
    LightState ls;
    Interval iv = FOREVER;

    light->EvalLightState(mStart, iv, &ls);
    fprintf(mStream, _T("DEF %s SpotLight {\n"), mNodes.GetNodeName(node));
    Indent(1);
    fprintf(mStream, _T("intensity %s\n"),
            floatVal(light->GetIntensity(mStart,FOREVER)));
    Indent(1);
    Point3 col = light->GetRGBColor(mStart, FOREVER);
    fprintf(mStream, _T("color %s\n"), color(col));
    Indent(1);
    Point3 p = node->GetObjTMAfterWSM(mStart).GetTrans();
    fprintf(mStream, _T("location %s\n"), point(p));

    Matrix3 tm = node->GetObjTMAfterWSM(mStart);
    p = Point3(0,0,-1);
    Point3 trans, s;
    Quat q;
    Matrix3 rot;
    AffineParts parts;
    decomp_affine(tm, &parts);
    q = parts.q;
    q.MakeMatrix(rot);
    p = p * rot;

    Indent(1);
    fprintf(mStream, _T("direction %s\n"), normPoint(p));
    Indent(1);
    fprintf(mStream, _T("cutOffAngle %s\n"),
            floatVal(DegToRad(ls.fallsize)));
    Indent(1);
    fprintf(mStream, _T("beamWidth %s\n"), floatVal(DegToRad(ls.hotsize)));
    Indent(1);
    fprintf(mStream, _T("on %s\n"), ls.on ? _T("TRUE") : _T("FALSE"));
    Indent(1);
    float radius;
    if (!ls.useAtten || ls.attenEnd == 0.0f)
        radius = Length(mBoundBox.Width());
    else
        radius = ls.attenEnd;
    fprintf(mStream, _T("radius %s\n"), floatVal(radius));
    if (ls.useAtten) {
        float attn;
        attn = (ls.attenStart <= 1.0f) ? 1.0f : 1.0f/ls.attenStart;
        Indent(1);
        fprintf(mStream, _T("attenuation 0 %s 0\n"), floatVal(attn));
    }
    fprintf(mStream, _T("}\n"));
    return TRUE;
}

void
VRML2Export::OutputTopLevelLight(INode* node, LightObject *light)
{
    Class_ID id = light->ClassID();
    if (id == Class_ID(OMNI_LIGHT_CLASS_ID, 0))
        VrmlOutTopPointLight(node, light);
    else if (id == Class_ID(DIR_LIGHT_CLASS_ID, 0) ||
             id == Class_ID(TDIR_LIGHT_CLASS_ID, 0))
        VrmlOutTopDirectLight(node, light);
    else if (id == Class_ID(SPOT_LIGHT_CLASS_ID, 0) ||
             id == Class_ID(FSPOT_LIGHT_CLASS_ID, 0))
        VrmlOutTopSpotLight(node, light);
    else
        return;
    
    // Write out any animation data
    InitInterpolators(node);
    VrmlOutControllers(node, 0);
    WriteInterpolatorRoutes(0, TRUE);
}

// Output a VRML Inline node.
BOOL
VRML2Export::VrmlOutInline(VRMLInsObject* obj, int level)
{
    char *url = obj->GetUrl().data();
    Indent(level);
    fprintf(mStream, _T("Inline {\n"));
    Indent(level+1);
    if (url && url[0] == '"')
        fprintf(mStream, _T("url %s\n"), url);
    else
        fprintf(mStream, _T("url \"%s\"\n"), url);
    if (obj->GetUseSize())
    {
        float size = obj->GetSize() * 2.0f;
        Indent(level+1);
        fprintf(mStream, _T("bboxSize %s\n"),
                scalePoint(Point3(size, size, size)));
    }
    Indent(level);
    fprintf(mStream, _T("}\n"));
    return TRUE;
}

// Distance comparison function for sorting LOD lists.
static int
DistComp(LODObj** obj1, LODObj** obj2)
{
    float diff = (*obj1)->dist - (*obj2)->dist;
    if (diff < 0.0f) return -1;
    if (diff > 0.0f) return 1;
    return 0;
}

// Create a level-of-detail object.
BOOL
VRML2Export::VrmlOutLOD(INode *node, LODObject* obj, int level, BOOL mirrored)
{
    int numLod = obj->NumRefs();
    Tab<LODObj*> lodObjects = obj->GetLODObjects();
    int i;

    if (numLod == 0)
        return TRUE;

    lodObjects.Sort((CompareFnc) DistComp);

//    if (numLod > 1) {
        Indent(level);
        fprintf(mStream, _T("LOD {\n"));
        Indent(level+1);
        Point3 p = node->GetObjTMAfterWSM(mStart).GetTrans();
        fprintf(mStream, _T("center %s\n"), point(p));
        Indent(level+1);
        fprintf(mStream, _T("range [ "));
        for(i = 0; i < numLod-1; i++) {
            if (i < numLod-2)
                fprintf(mStream, _T("%s, "), floatVal(lodObjects[i]->dist));
            else
//                fprintf(mStream, _T("%s ]\n"), floatVal(lodObjects[i]->dist));
                fprintf(mStream, _T("%s "), floatVal(lodObjects[i]->dist));
        }
fprintf(mStream, _T("]\n"));
//    }

    Indent(level+1);
    fprintf(mStream, _T("level [\n"));
    for(i = 0; i < numLod; i++) {
        INode *node = lodObjects[i]->node;
        INode *parent = node->GetParentNode();
        VrmlOutNode(node, parent, level+1, TRUE, FALSE, mirrored);
        if (i != numLod-1) {
            Indent(level);
            fprintf(mStream, _T(",\n"));
        }
    }

//    if (numLod > 1) {
        Indent(level);
        fprintf(mStream, _T("]\n"));
        Indent(level);
        fprintf(mStream, _T("}\n"));
//    }

    return TRUE;
}

BOOL
VRML2Export::VrmlOutSpecialTform(INode* node, Object* obj, int level,
                                 BOOL mirrored)
{
    if (!mPrimitives)
        return FALSE;

    Class_ID id = obj->ClassID();

    // Otherwise look for the primitives we know about
    if (id == Class_ID(CYLINDER_CLASS_ID, 0))
        return VrmlOutCylinderTform(node, obj, level+1, mirrored);

    if (id == Class_ID(CONE_CLASS_ID, 0))
        return VrmlOutConeTform(node, obj, level+1, mirrored);

    if (id == Class_ID(BOXOBJ_CLASS_ID, 0))
        return VrmlOutCubeTform(node, obj, level+1, mirrored);

    return FALSE;
        
}

BOOL
VRML2Export::ObjIsPrim(INode* node, Object* obj)
{
    Class_ID id = obj->ClassID();
    if (id == Class_ID(SPHERE_CLASS_ID, 0))
        return VrmlOutSphereTest(node, obj);

    if (id == Class_ID(CYLINDER_CLASS_ID, 0))
        return VrmlOutCylinderTest(node, obj);

    if (id == Class_ID(CONE_CLASS_ID, 0))
        return VrmlOutConeTest(node, obj);

    if (id == Class_ID(BOXOBJ_CLASS_ID, 0))
        return VrmlOutCubeTest(node, obj);

    return FALSE;
}

// Write out the VRML for node we know about, including Opus nodes, 
// lights, cameras and VRML primitives
BOOL
VRML2Export::VrmlOutSpecial(INode* node, INode* parent,
                             Object* obj, int level, BOOL mirrored)
{
    Class_ID id = obj->ClassID();

    /* test
    if (id == Class_ID(MR_BLUE_CLASS_ID1, MR_BLUE_CLASS_ID2)) {
        level++;
        VrmlOutMrBlue(node, parent, (MrBlueObject*) obj,
                      &level, FALSE);
        return TRUE;
    }
    */
    
    if (id == Class_ID(OMNI_LIGHT_CLASS_ID, 0))
        return VrmlOutPointLight(node, (LightObject*) obj, level+1);

    if (id == Class_ID(DIR_LIGHT_CLASS_ID, 0) ||
        id == Class_ID(TDIR_LIGHT_CLASS_ID, 0))
        return VrmlOutDirectLight(node, (LightObject*) obj, level+1);

    if (id == Class_ID(SPOT_LIGHT_CLASS_ID, 0) ||
        id == Class_ID(FSPOT_LIGHT_CLASS_ID, 0))
        return VrmlOutSpotLight(node, (LightObject*) obj, level+1);

    if (id == Class_ID(VRML_INS_CLASS_ID1, VRML_INS_CLASS_ID2))
        return VrmlOutInline((VRMLInsObject*) obj, level+1);

    if (id == Class_ID(LOD_CLASS_ID1, LOD_CLASS_ID2))
        return VrmlOutLOD(node, (LODObject*) obj, level+1, mirrored);

    if (id == Class_ID(SIMPLE_CAM_CLASS_ID, 0) ||
        id == Class_ID(LOOKAT_CAM_CLASS_ID, 0))
        return VrmlOutCamera(node, obj, level+1);

    if (id == SoundClassID)
        return VrmlOutSound(node, (SoundObject*) obj, level+1);

    if (id == ProxSensorClassID)
        return VrmlOutProxSensor(node, (ProxSensorObject*) obj, level+1);

    if (id == BillboardClassID)
        return VrmlOutBillboard(node, obj, level+1);

    // If object has modifiers or WSMs attached, do not output as
    // a primitive
    SClass_ID sid = node->GetObjectRef()->SuperClassID();
    if (sid == WSM_DERIVOB_CLASS_ID ||
        sid == DERIVOB_CLASS_ID)
        return FALSE;

    if (!mPrimitives)
        return FALSE;

    // Otherwise look for the primitives we know about
    if (id == Class_ID(SPHERE_CLASS_ID, 0))
        return VrmlOutSphere(node, obj, level+1);

    if (id == Class_ID(CYLINDER_CLASS_ID, 0))
        return VrmlOutCylinder(node, obj, level+1);

    if (id == Class_ID(CONE_CLASS_ID, 0))
        return VrmlOutCone(node, obj, level+1);

    if (id == Class_ID(BOXOBJ_CLASS_ID, 0))
        return VrmlOutCube(node, obj, level+1);

    return FALSE;
        
}

static BOOL
IsLODObject(Object* obj)
{
    return obj->ClassID() == Class_ID(LOD_CLASS_ID1, LOD_CLASS_ID2);
}

static BOOL
IsEverAnimated(INode* node)
{
    if (!node)
        return FALSE;
    for (; !node->IsRootNode(); node = node->GetParentNode())
        if (node->IsAnimated())
            return TRUE;
    return FALSE;
}

BOOL
VRML2Export::ChildIsAnimated(INode* node)
{
    if (node->IsAnimated())
        return TRUE;

    Object* obj = node->EvalWorldState(mStart).obj;

    if (ObjIsAnimated(obj))
        return TRUE;

    Class_ID id = node->GetTMController()->ClassID();

    if (id != Class_ID(PRS_CONTROL_CLASS_ID, 0))
        return TRUE;

    for (int i = 0; i < node->NumberOfChildren(); i++)
        if (ChildIsAnimated(node->GetChildNode(i)))
            return TRUE;
    return FALSE;
}

static BOOL
IsAnimTrigger(Object *obj)
{
    if (!obj)
        return FALSE;

    Class_ID id = obj->ClassID();
    /* test
    // Mr Blue nodes only 1st class if stand-alone
    if (id == Class_ID(MR_BLUE_CLASS_ID1, MR_BLUE_CLASS_ID2)) {
        MrBlueObject* mbo = (MrBlueObject*) obj;
        return mbo->GetMouseEnabled() && mbo->GetAction() == Animate;
    }
    */
    return FALSE;
}

BOOL
VRML2Export::isVrmlObject(INode * node, Object *obj, INode* parent)
{
    if (!obj)
        return FALSE;

    Class_ID id = obj->ClassID();

    /* test
    
    if (id == Class_ID(OMNI_LIGHT_CLASS_ID, 0))
    // Mr Blue nodes only 1st class if stand-alone
    if (id == Class_ID(MR_BLUE_CLASS_ID1, MR_BLUE_CLASS_ID2)) {
        MrBlueObject* mbo = (MrBlueObject*) obj;
        if ((mbo->GetAction() == HyperLinkJump ||
             mbo->GetAction() == SetViewpoint) &&
            mbo->GetMouseEnabled())
            return parent->IsRootNode();
        else
            return FALSE;
    }
    */

    if (id == Class_ID(VRML_INS_CLASS_ID1, VRML_INS_CLASS_ID2) ||
        id == SoundClassID ||
        id == ProxSensorClassID)
        return TRUE;

    // only animated lights come out in scene graph
    if (IsLight(node))
        return (IsEverAnimated(node) || IsEverAnimated(node->GetTarget()));
    if (IsCamera(node))
        return FALSE;

    if (node->NumberOfChildren() > 0)
        return TRUE;

#ifdef _LEC_
 // LEC uses dummies as place holders and need dummy leaves written.
    if (id == Class_ID(DUMMY_CLASS_ID, 0))
        return TRUE;
#endif

    return (obj->IsRenderable() ||
            id == Class_ID(LOD_CLASS_ID1, LOD_CLASS_ID2)) &&
        (mExportHidden || !node->IsHidden());
        
}

static BOOL
NodeIsChildOf(INode* child, INode* parent)
{
    if (child == parent)
        return TRUE;
 // skip invalid nodes (ex. user create the list then delete the node from the scene.)
	if (!parent)
		return FALSE;
    int num = parent->NumberOfChildren();
    int i;
    for (i = 0; i < num; i++) {
        if (NodeIsChildOf(child, parent->GetChildNode(i)))
            return TRUE;
    }
    return FALSE;
}

// For objects that change shape, output a CoodinateInterpolator
void
VRML2Export::VrmlOutCoordinateInterpolator(INode* node, Object *obj,
                                           int level, BOOL pMirror)
{
    int sampleRate;
    int t, i, j;
    int width = mIndent ? level * 2 : 0;
    TCHAR name[MAX_PATH];

    if (mCoordSample)
        sampleRate = GetTicksPerFrame();
    else
        sampleRate = TIME_TICKSPERSEC / mCoordSampleRate;

    int end = mIp->GetAnimRange().End();
    int realEnd = end;
    int frames = (end - mStart)/sampleRate + 1;

    if (((end - mStart) % sampleRate) != 0) {
        end += sampleRate;
        frames++;
    }
    
    Indent(level);
    sprintf(name, _T("%s-COORD-INTERP"), mNodes.GetNodeName(node));
    fprintf(mStream, _T("DEF %s CoordinateInterpolator {\n"), name);
    AddInterpolator(name, KEY_COORD, mNodes.GetNodeName(node));
    // Now check to see if a TimeSensor references this node
    INodeList* l;
    for(l = mTimerList; l; l = l->GetNext()) {
        TimeSensorObject* tso = (TimeSensorObject*)
            l->GetNode()->EvalWorldState(mStart).obj;
        for(int j = 0; j < tso->TimeSensorObjects.Count(); j++) {
            INode* anim = tso->TimeSensorObjects[j]->node;
         // skip invalid nodes (ex. user create the list then delete the node from the scene.)
            if (anim && NodeIsChildOf(node, anim)) {
                TSTR oTimer = mTimer;
                TCHAR timer[MAX_PATH];
                sprintf(timer, _T("%s"),mNodes.GetNodeName(l->GetNode()));
                if (tso->needsScript)
                    AddInterpolator(name, KEY_TIMER_SCRIPT, timer);
                else
                    AddInterpolator(name, KEY_TIMER, timer);
            }
        }
    }
            
    Indent(level+1);
    fprintf(mStream, _T("key ["));
    mCycleInterval = (mIp->GetAnimRange().End() - mStart) /
        ((float) GetTicksPerFrame()* GetFrameRate());

    for(i = 0, t = mStart; i < frames; i++, t += sampleRate) {
        if (t > realEnd)
            t = realEnd;
        width += fprintf(mStream, _T("%s, "),
                         floatVal(t / ((float) GetTicksPerFrame()
                                       * GetFrameRate() * mCycleInterval)));
        if (width > 60) {
            fprintf(mStream, _T("\n"));
            Indent(level+3);
            width = mIndent ? level * 2 : 0;
        }
    }
    fprintf(mStream, _T("]\n"));

    Indent(level+1);
    fprintf(mStream, _T("keyValue ["));

    // Now output the values for the interpolator
    for (i = 0, t = mStart; i < frames; i++, t += sampleRate) {
        if (t > realEnd)
            t = realEnd;
        Object *o = node->EvalWorldState(t).obj;
        TriObject *tri = (TriObject *)o->ConvertToType(t, triObjectClassID);
        Mesh &mesh = tri->GetMesh();
                
        int numverts = mesh.getNumVerts();
        for(j = 0; j < numverts; j++) {
            Point3 p = mesh.verts[j];
#ifdef MIRROR_BY_VERTICES
			if (pMirror)
				p = - p;
#endif
            width += fprintf(mStream, _T("%s, "), point(p));
            if (width > 60) {
                fprintf(mStream, _T("\n"));
                Indent(level+3);
                width = mIndent ? level * 2 : 0;
            }
        }
        
        if(o != (Object *) tri)
            tri->DeleteThis();
    }
    fprintf(mStream, _T("]\n"));
    Indent(level+1);
    fprintf(mStream, _T("}\n"));

 // get valid mStart object
    obj = node->EvalWorldState(mStart).obj;
}

BOOL
VRML2Export::ObjIsAnimated(Object *obj)
{
    if (!obj)
        return FALSE;
    Interval iv = obj->ObjectValidity(mStart);
    return !(iv == FOREVER);
}

static BOOL
MtlHasTexture(Mtl* mtl)
{
    if (mtl->ClassID() != Class_ID(DMTL_CLASS_ID, 0))
        return FALSE;

    StdMat* sm = (StdMat*) mtl;
    // Check for texture map
    Texmap* tm = (BitmapTex*) sm->GetSubTexmap(ID_DI);
    if (!tm)
        return FALSE;

    if (tm->ClassID() != Class_ID(BMTEX_CLASS_ID, 0))
        return FALSE;
    BitmapTex* bm = (BitmapTex*) tm;

    TSTR bitmapFile;

    bitmapFile = bm->GetMapName();
    if (bitmapFile.data() == NULL)
        return FALSE;
    int l = strlen(bitmapFile)-1;
    if (l < 0)
        return FALSE;

    return TRUE;
}

static int
NumTextures(INode* node)
{
    float firstxpar;
    Mtl *sub, *mtl = node->GetMtl();
    if (!mtl)
        return 0;

    if (!mtl->IsMultiMtl())
        return 0;
    int num = mtl->NumSubMtls();
    for(int i = 0; i < num; i++) {
        sub = mtl->GetSubMtl(i);
        if (!sub)
            continue;
        if (MtlHasTexture(sub))
            return num;
        if (i == 0)
            firstxpar = sub->GetXParency();
        else if (sub->GetXParency() != firstxpar)
            return num;
    }
    return 0;
}

// Write the data for a single object.
// This function also takes care of identifying VRML primitive objects
void
VRML2Export::VrmlOutObject(INode* node, INode* parent, Object* obj, int level,
                           BOOL mirrored)
{
 // need to get a valid obj ptr

    obj = node->EvalWorldState(mStart).obj;
    BOOL isTriMesh = obj->CanConvertToType(triObjectClassID);
    BOOL instance;
    BOOL special = FALSE;
    int numTextures = NumTextures(node);
    int start, end;

    if (numTextures == 0) {
        start = -1;
        end = 0;
    } else {
        start = 0;
        end = numTextures;
    }

    int old_level = level;
    for(int i = start; i < end; i++) {
        if (isTriMesh && obj->IsRenderable()) {
            special = VrmlOutSpecialTform(node, obj, level, mirrored);
            if (special)
                level += 3;
            Indent(level);
            fprintf(mStream, _T("Shape {\n"));
        }
        
        BOOL multiMat = FALSE;
        BOOL isWire = FALSE, twoSided = FALSE;
        
        // Output the material
        if (isTriMesh && obj->IsRenderable())  // if not trimesh, needs no matl
            multiMat = OutputMaterial(node, isWire, twoSided, level+1, i);
        
        // First check for VRML primitives and other special objects
        if (VrmlOutSpecial(node, parent, obj, level, mirrored)) {
            if (isTriMesh && obj->IsRenderable()) {
                Indent(level);
                fprintf(mStream, _T("}\n"));
                if (special) {
                    level = old_level;
                    Indent(level);
                    fprintf(mStream, _T("] }\n"));
                }
            }
            continue;
        }
        
        // Otherwise output as a triangle mesh
        if (isTriMesh && obj->IsRenderable()) {
            ObjectBucket* ob = mObjTable.AddObject(obj);
            if (ob->objectUsed && i == -1) {
                instance = TRUE;
                // We have an instance
                Indent(level);
                fprintf(mStream, _T("geometry USE %s-FACES\n"),
                        ob->instName.data());
            } else {
                ob->objectUsed = TRUE;
                ob->instName = mNodes.GetNodeName(node);
                instance = FALSE;
                TriObject *tri =
                    (TriObject *)obj->ConvertToType(mStart, triObjectClassID);
                
                if (mPolygonType && !ObjIsAnimated(obj))
                     OutputPolygonObject(node, tri, multiMat, isWire,
                                         twoSided, level+1, i, mirrored);
				else OutputTriObject(node, tri, multiMat, isWire, twoSided,
                                     level+1, i, mirrored);

#ifndef FUNNY_TEST
                if(obj != (Object *)tri)
                    tri->DeleteThis();
#endif
            }
            Indent(level);
            fprintf(mStream, _T("}\n"));
        }
    }

    // Check for animated object, and generate CordinateInterpolator
    if (mCoordInterp && isTriMesh && ObjIsAnimated(obj) && !instance)
        VrmlOutCoordinateInterpolator(node, obj, level, mirrored);
}

TCHAR*
VRML2Export::VrmlParent(INode* node)
{
    static TCHAR buf[256];
    /* test
    Object *obj = node->EvalWorldState(mStart).obj;
    Class_ID id = obj->ClassID();
    while (id == Class_ID(MR_BLUE_CLASS_ID1, MR_BLUE_CLASS_ID2)) {
        node = node->GetParentNode();
        obj = node->EvalWorldState(mStart).obj;
        if (!obj)
            return NULL;  // Unattached
        id = obj->ClassID();
    }
    */
    assert (node);
    _tcscpy(buf, mNodes.GetNodeName(node));
    return buf;
}

BOOL
VRML2Export::IsAimTarget(INode* node)
{
    INode* lookAt = node->GetLookatNode();
    if (!lookAt)
        return FALSE;
    Object* lookAtObj = lookAt->EvalWorldState(mStart).obj;
    Class_ID id = lookAtObj->ClassID();
    // Only generate aim targets for targetted spot lights and cameras
    if (id != Class_ID(SPOT_LIGHT_CLASS_ID, 0) &&
        id != Class_ID(LOOKAT_CAM_CLASS_ID, 0))
        return FALSE;
    return TRUE;
}
// Write out the node header for a Mr. Blue object
/* test
void
VRML2Export::VrmlAnchorHeader(INode* node, MrBlueObject* obj,
                              VRBL_TriggerType trigType, BOOL fromParent,
                              int level)
{
    TSTR desc;
    VRBL_Action action = obj->GetAction();
    TCHAR* vrmlObjName = NULL;
    vrmlObjName = VrmlParent(node);
    if (!vrmlObjName)
        return;

    if (action == HyperLinkJump || action == MrBlueMessage ||
        action == SetViewpoint) {
        switch (trigType) {
        case MouseClick:
            fprintf(mStream, _T("Anchor {\n"));
            break;
        case DistProximity:
            break;
        case BoundingBox:
            break;
        case LineOfSight:
            break;
        default:
            assert(FALSE);
        }

        Indent(level+1);
        TSTR camera;
        TCHAR *name = _T("url");

        switch (action) {
        case MrBlueMessage:
            fprintf(mStream, _T("%s \"signal:\"\n"), name);
            break;
        case HyperLinkJump:
            camera = obj->GetCamera();
            if (camera.Length() == 0)
                fprintf(mStream, _T("%s \"%s\"\n"), name, obj->GetURL());
            else
                fprintf(mStream, _T("%s \"%s#%s\"\n"), name, obj->GetURL(),
                        VRMLName(camera.data()));
            if (trigType == MouseClick) {
                desc = obj->GetDesc();
                if (desc.Length() > 0) {
                    Indent(level+1);
                    fprintf(mStream,
                            _T("description \"%s\"\n"), obj->GetDesc());
                }
            }
            break;
        case SetViewpoint:
            if (obj->GetVptCamera())
                camera = obj->GetVptCamera()->GetName();
            else
                camera = _T("");
            fprintf(mStream, _T("%s \"#%s\"\n"), name,
                    VRMLName(camera.data()));
            if (trigType == MouseClick) {
                desc = obj->GetVptDesc();
                if (desc.Length() > 0) {
                    Indent(level+1);
                    fprintf(mStream, _T("description \"%s\"\n"), desc);
                }
            }
            break;
        default:
            assert(FALSE);
        }
        switch (trigType) {
        case MouseClick:
            fprintf(mStream, _T("children [\n"));
            break;
        }
    } else {
        switch (trigType) {
        case MouseClick:
            fprintf(mStream, _T("DEF %s-SENSOR TouchSensor {}\n"), vrmlObjName);
            break;
        case DistProximity:
            break;
        case BoundingBox:
            break;
        case LineOfSight:
            break;
        default:
            assert(FALSE);
        }
        Indent(level+1);
        TSTR camera;
        TCHAR *name = _T("url");

        switch (action) {
        case MrBlueMessage:
            fprintf(mStream, _T("%s \"signal:\"\n"), name);
            break;
        case HyperLinkJump:
            camera = obj->GetCamera();
            if (camera.Length() == 0)
                fprintf(mStream, _T("%s \"%s\"\n"), name, obj->GetURL());
            else
                fprintf(mStream, _T("%s \"%s#%s\"\n"), name, obj->GetURL(),
                        camera.data());
            if (trigType == MouseClick) {
                desc = obj->GetDesc();
                if (desc.Length() > 0) {
                    Indent(level+1);
                    fprintf(mStream,
                            _T("description \"%s\"\n"), obj->GetDesc());
                }
            }
            break;
        case SetViewpoint:
            camera = obj->GetVptCamera()->GetName();
            fprintf(mStream, _T("%s \"#%s\"\n"), name, camera.data());
            if (trigType == MouseClick) {
                desc = obj->GetVptDesc();
                if (desc.Length() > 0) {
                    Indent(level+1);
                    fprintf(mStream, _T("description \"%s\"\n"), desc);
                }
            }
            break;
        case Animate: {
            // Output the objects to animate
            int size = obj->GetAnimObjects()->Count();
            for(int i=0; i < size; i++) {
                MrBlueAnimObj* animObj = (*obj->GetAnimObjects())[i];
                Object *o = animObj->node->EvalWorldState(mStart).obj;
                if (!o)
                    break;
                assert(vrmlObjName);
                if (IsAimTarget(animObj->node))
                    break;
                INode* top;
                if (o->ClassID() == TimeSensorClassID)
                    top = animObj->node;
                else
                    top = GetTopLevelParent(animObj->node);
                ObjectBucket* ob =
                    mObjTable.AddObject(top->EvalWorldState(mStart).obj);
                AddAnimRoute(vrmlObjName, ob->name.data(), node, top);
                AddCameraAnimRoutes(vrmlObjName, node, top);
            }
            break; }
         default:
            assert(FALSE);
        }
    }
}
*/

void
VRML2Export::AddCameraAnimRoutes(TCHAR* vrmlObjName, INode* fromNode,
                                 INode* top)
{
    for(int i = 0; i < top->NumberOfChildren(); i++) {
        INode *child = top->GetChildNode(i);
        Object* obj = child->EvalWorldState(mStart).obj;
        if (!obj)
            continue;
        SClass_ID sid = obj->SuperClassID();
        if (sid == CAMERA_CLASS_ID)
            AddAnimRoute(vrmlObjName, mNodes.GetNodeName(child),
                fromNode, child);
        AddCameraAnimRoutes(vrmlObjName, fromNode, child);
    }
}

void
VRML2Export::AddAnimRoute(TCHAR* from, TCHAR* to, INode* fromNode,
                          INode* toNode  )
{
    TCHAR fromStr[MAX_PATH];
    if (!from || *from == '\0') {
        _tcscpy(fromStr, mNodes.GetNodeName(fromNode));
        from = fromStr;
    }
    if (!to || *to == '\0')
        to = mNodes.GetNodeName(toNode);
    AnimRoute* ar = new AnimRoute(from, to, fromNode, toNode);
    mAnimRoutes.Append(1, ar);
}

int
VRML2Export::NodeNeedsTimeSensor(INode* node)
{
    BOOL isCamera = IsCamera(node);
	BOOL isAudio = IsAudio(node);
    BOOL isAnim = (isAudio ||
					(!isCamera && node->GetParentNode()->IsRootNode() &&
                        ChildIsAnimated(node)) ||
                    (isCamera && (IsEverAnimated(node) ||
                        IsEverAnimated(node->GetTarget()))));
    if (!isAnim)
        return 0;
    if (node->GetNodeLong() & (RUN_BY_PROX_SENSOR | RUN_BY_TOUCH_SENSOR))
        return 1;
    if (node->GetNodeLong() & RUN_BY_TIME_SENSOR)
        return 0;
    return -1;
}

void
VRML2Export::WriteAnimRoutes()
{
    int i;
    int ts;
    TCHAR from[MAX_PATH], to[MAX_PATH];
    for(i = 0; i < mAnimRoutes.Count(); i++) {
        INode* toNode = mAnimRoutes[i].mToNode;
        TCHAR *toName = mNodes.GetNodeName(toNode);
        Object* toObj = toNode->EvalWorldState(mStart).obj;
        Object* fromObj = mAnimRoutes[i].mFromNode->
            EvalWorldState(mStart).obj;
        BOOL isCamera = IsCamera(toNode);
        ts = NodeNeedsTimeSensor(toNode);
        if (ts != 0 || toObj->ClassID() == TimeSensorClassID) {
            if (toObj->ClassID() == AudioClipClassID)
                sprintf(to, _T("%s.startTime"), toName);
            else
                sprintf(to, _T("%s-TIMER.startTime"), toName);
            
            if (fromObj->ClassID() == ProxSensorClassID)
                sprintf(from, _T("%s.enterTime"), mAnimRoutes[i].mFromName);
            else
                sprintf(from, _T("%s-SENSOR.touchTime"), mAnimRoutes[i].mFromName);
            fprintf(mStream, _T("ROUTE %s TO %s\n"), from, to);
        }
    }
}

// Write out the header for a single Mr. Blue node
/* test
BOOL
VRML2Export::VrmlOutMrBlue(INode* node, INode* parent, MrBlueObject* obj,
                          int* level, BOOL fromParent)
{
    BOOL hadHeader = FALSE;
    TCHAR* name;
    if (fromParent)
        name = mNodes.GetNodeName(parent);
    else
        name = mNodes.GetNodeName(node);

    if (obj->GetMouseEnabled()) {
        MrBlueObject* mbo = (MrBlueObject*) obj;
        Indent(*level);
        VrmlAnchorHeader(node, obj, MouseClick, fromParent, *level);
        (*level)++;
        hadHeader = TRUE;
    }

    if (mType != Export_VRBL)
        goto end;

    if (obj->GetProxDistEnabled()) {
        Indent(*level);
        if (!hadHeader)
            fprintf(mStream, _T("DEF %s "), name);
        VrmlAnchorHeader(node, obj, DistProximity, fromParent, *level);
        Indent(*level+1);
        fprintf(mStream, _T("distance %s\n"), floatVal(obj->GetProxDist()));
        if (!fromParent) {
            // Generate proximity point for top-level objects.
            Indent(*level+1);
            fprintf(mStream, _T("point 0 0 0 \n"));
        }
        (*level)++;
        hadHeader = TRUE;
    }

    if (obj->GetBBoxEnabled() && !fromParent) {
        if (!fromParent)
        Indent(*level);
        if (!hadHeader)
            fprintf(mStream, _T("DEF %s "), name);
        VrmlAnchorHeader(node, obj, BoundingBox, fromParent, *level);
        Indent(*level+1);
        
        float x = obj->GetBBoxX()/2.0f,
            y = obj->GetBBoxY()/2.0f,
            z = obj->GetBBoxZ()/2.0f;
        Point3 p0 = Point3(-x, -y, -z), p1 = Point3(x, y, z);
        fprintf(mStream, _T("point [ %s, "), point(p0));
        fprintf(mStream, _T(" %s ]\n"), point(p1));
        (*level)++;
        hadHeader = TRUE;
    }
    
    if (obj->GetLosEnabled()) {
        if (obj->GetLosType() == CanSee) {
            Indent(*level);
            if (!hadHeader)
                fprintf(mStream, _T("DEF %s "), name);
            VrmlAnchorHeader(node, obj, LineOfSight, fromParent, *level);
            Indent(*level+1);
            fprintf(mStream, _T("distance %s\n"),
                    floatVal(GetLosProxDist(node, mStart)));
            Indent(*level+1);
            fprintf(mStream, _T("angle %s\n"),
                    floatVal(DegToRad(obj->GetLosVptAngle())));
        }
        else {
            Indent(*level);
            if (!hadHeader)
                fprintf(mStream, _T("DEF %s "), name);
            VrmlAnchorHeader(node, obj, LineOfSight, fromParent, *level);
            Indent(*level+1);
            fprintf(mStream, _T("distance %s\n"),
                    floatVal(GetLosProxDist(node, mStart)));
            Indent(*level+1);
            fprintf(mStream, _T("sightAngle %s\n"),
                    floatVal(DegToRad(obj->GetLosVptAngle())));
            Point3 p = GetLosVector(node, mStart);
            Indent(*level+1);
            fprintf(mStream, _T("vector %s\n"), normPoint(p));
            Indent(*level+1);
            fprintf(mStream, _T("vectorAngle %s\n"),
                    floatVal(DegToRad(obj->GetLosObjAngle())));
        }
        (*level)++;
    }

  end:
    // Close off the nodes if this is a stand-alone helper
    if (!fromParent)
        EndMrBlueNode(node, *level, FALSE);

    return TRUE;
}
*/

/*
// Start the headers for Mr. Blue nodes attached to the given node,
// returning the new indentation level
int
VRML2Export::StartMrBlueHelpers(INode* node, int level)
{
    // Check for Mr Blue helper at child nodes
    for(int i=0; i<node->NumberOfChildren(); i++) {
        INode* childNode = node->GetChildNode(i);
        Object *obj = childNode->EvalWorldState(mStart).obj;
        Class_ID id = obj->ClassID();
        if (id == Class_ID(MR_BLUE_CLASS_ID1, MR_BLUE_CLASS_ID2)) {
            MrBlueObject *mbo = (MrBlueObject*) obj;
            if ((mbo->GetAction() == HyperLinkJump ||
                 mbo->GetAction() == SetViewpoint) &&
                mbo->GetMouseEnabled())
                VrmlOutMrBlue(childNode, node, mbo, &level, TRUE);
        }
    }
    return level;
}
*/

// Write out the node closer for a Mr. Blue node
/* test
void
VRML2Export::EndMrBlueNode(INode* childNode, int& level, BOOL fromParent)
{
    Object *obj = childNode->EvalWorldState(mStart).obj;
    Class_ID id = obj->ClassID();
    if (id == Class_ID(MR_BLUE_CLASS_ID1, MR_BLUE_CLASS_ID2)) {
        MrBlueObject* mbo = (MrBlueObject*) obj;
        if (mbo->GetMouseEnabled()) {
            if (mbo->GetAction() == HyperLinkJump ||
                mbo->GetAction() == SetViewpoint) {
                if (!fromParent)
                    return;
                Indent(level);
                fprintf(mStream, _T("]\n"));
                Indent(--level);
                fprintf(mStream, _T("}\n"));
            }
        }
        // FIXME take care of these
        if (mbo->GetProxDistEnabled()) {
        }
        if (mbo->GetBBoxEnabled()) {
        }
        if (mbo->GetLosEnabled()) {
        }
    }
}
*/

// Write out the node closers for all the Mr. Blue headers
/* test
void
VRML2Export::EndMrBlueHelpers(INode* node, int level)
{
    // Check for Mr Blue helper at child nodes
    for(int i=0; i<node->NumberOfChildren(); i++) {
        EndMrBlueNode(node->GetChildNode(i), level, TRUE);
    }
}
*/

void
VRML2Export::InitInterpolators(INode* node)
{
#ifdef _LEC_
    if (mFlipBook)
        return;
#endif
    mInterpRoutes.SetCount(0);
    int sensors = node->GetNodeLong() & RUN_BY_ANY_SENSOR;
    if (sensors == RUN_BY_TIME_SENSOR)
        mTimer = (const char *)NULL;
    else
        mTimer = TSTR(mNodes.GetNodeName(node)) + TSTR(_T("-TIMER"));
}

void
VRML2Export::AddInterpolator(TCHAR* interp, int type, TCHAR* name)
{
    InterpRoute* r = new InterpRoute(interp, type, name);
    mInterpRoutes.Append(1, r);
}

void
VRML2Export::WriteInterpolatorRoutes(int level, BOOL isCamera)
{
#ifdef _LEC_
    if (mFlipBook)
        return;
#endif
    int i;
    for(i = 0; i < mInterpRoutes.Count(); i++) {
        Indent(level);
        if (mInterpRoutes[i].mType == KEY_TIMER)
            fprintf(mStream,
                    _T("ROUTE %s-TIMER.fraction_changed TO %s.set_fraction\n"),
                    (char *)mInterpRoutes[i].mNode, (char *)mInterpRoutes[i].mInterp);
        else if (mInterpRoutes[i].mType == KEY_TIMER_SCRIPT) {
            fprintf(mStream,
                    _T("ROUTE %s-TIMER.fraction_changed TO %s-SCRIPT.fractionIn\n"),
                    (char *)mInterpRoutes[i].mNode, (char *)mInterpRoutes[i].mNode);
            fprintf(mStream,
                    _T("ROUTE %s-SCRIPT.fractionOut TO %s.set_fraction\n"),
                    (char *)mInterpRoutes[i].mNode, (char *)mInterpRoutes[i].mInterp);
        } else if (!mTimer.isNull())
            fprintf(mStream, _T("ROUTE %s.fraction_changed TO %s.set_fraction\n"),
                    (char *)mTimer, (char *)mInterpRoutes[i].mInterp);
        Indent(level);
        TCHAR* setType = NULL;
        switch (mInterpRoutes[i].mType) {
        case KEY_POS:
            if (isCamera)
                setType = _T("set_position");
            else
                setType = _T("set_translation");
            break;
        case KEY_ROT:
            if (isCamera)
                setType = _T("set_orientation");
            else
                setType = _T("set_rotation");
            break;
        case KEY_SCL:
            setType = _T("set_scale");
            break;
        case KEY_SCL_ORI:
            setType = _T("set_scaleOrientation");
            break;
        case KEY_COORD:
            setType = _T("set_point");
            break;
        case KEY_COLOR:
            setType = _T("set_color");
            break;
        }
        if (mInterpRoutes[i].mType != KEY_TIMER &&
            mInterpRoutes[i].mType != KEY_TIMER_SCRIPT) {
            assert(setType);
            if (isCamera)
                fprintf(mStream, _T("ROUTE %s.value_changed TO %s.%s\n"),
                        (char *)mInterpRoutes[i].mInterp, (char *)mInterpRoutes[i].mNode,
                        setType);
            else if (mInterpRoutes[i].mType == KEY_COLOR)
                fprintf(mStream, _T("ROUTE %s.value_changed TO %s-LIGHT.%s\n"),
                        (char *)mInterpRoutes[i].mInterp, (char *)mInterpRoutes[i].mNode,
                        setType);
            else if (mInterpRoutes[i].mType == KEY_COORD)
                fprintf(mStream,_T("ROUTE %s.value_changed TO %s-COORD.%s\n"),
                        (char *)mInterpRoutes[i].mInterp, (char *)mInterpRoutes[i].mNode,
                        setType);
            else
                fprintf(mStream, _T("ROUTE %s.value_changed TO %s.%s\n"),
                        (char *)mInterpRoutes[i].mInterp, (char *)mInterpRoutes[i].mNode,
                        setType);
        }
    }
}

inline BOOL
ApproxEqual(float a, float b, float eps)
{
    float d = (float) fabs(a-b);
    return d < eps;
}

int
reducePoint3Keys(Tab<TimeValue>& times, Tab<Point3>& points, float eps)
{
    if (times.Count() < 3)
        return times.Count();

    BOOL *used = new BOOL[times.Count()];
    int i;
    for(i = 0; i < times.Count(); i++)
        used[i] = TRUE;

    // The two lines are represented as p0 + v * s and q0 + w * t.
    Point3 p0, q0;  
    for(i = 1; i < times.Count(); i++) {
        p0 = points[i];
        q0 = points[i-1];
        if (ApproxEqual(p0.x, q0.x, eps) && 
            ApproxEqual(p0.y, q0.y, eps) && 
            ApproxEqual(p0.z, q0.z, eps)) 
            used[i] = FALSE;
        else {
            used[i-1] = TRUE;
        }
    }

    int j = 0;
    for(i = 0; i<times.Count(); i++)
        if (used[i])
            j++;
    if (j == 1) {
        delete[] used;
        return 0;
    }
    j = 0;
    for(i = 0; i < times.Count(); i++) {
        if (used[i]) {
            times[j] = times[i];
            points[j] = points[i];
            j++;
        }
    }
    times.SetCount(j);
    points.SetCount(j);
    delete[] used;
    if (j == 1)
        return 0;
    if (j == 2) {
        p0 = points[0];
        q0 = points[1];
        if (ApproxEqual(p0.x, q0.x, eps) && 
            ApproxEqual(p0.y, q0.y, eps) && 
            ApproxEqual(p0.z, q0.z, eps)) 
            return 0;
    }
    return j;
}

int
reduceAngAxisKeys(Tab<TimeValue>& times, Tab<AngAxis>& points, float eps)
{
    if (times.Count() < 3)
        return times.Count();

    BOOL *used = new BOOL[times.Count()];
    int i;
    for(i = 0; i < times.Count(); i++)
        used[i] = TRUE;

    // The two lines are represented as p0 + v * s and q0 + w * t.
    AngAxis p0, q0;  
    for(i = 1; i < times.Count(); i++) {
        p0 = points[i];
        q0 = points[i-1];
        if (ApproxEqual(p0.axis.x, q0.axis.x, eps) && 
            ApproxEqual(p0.axis.y, q0.axis.y, eps) && 
            ApproxEqual(p0.axis.z, q0.axis.z, eps) && 
            ApproxEqual(p0.angle, q0.angle, eps)) 
            used[i] = FALSE;
        else {
            used[i-1] = TRUE;
        }
    }

    int j = 0;
    for(i = 0; i<times.Count(); i++)
        if (used[i])
            j++;
    if (j == 1) {
        delete[] used;
        return 0;
    }
    j = 0;
    for(i = 0; i < times.Count(); i++) {
        if (used[i]) {
            times[j] = times[i];
            points[j] = points[i];
            j++;
        }
    }
    times.SetCount(j);
    points.SetCount(j);
    delete[] used;
    if (j == 1)
        return 0;
    if (j == 2) {
        p0 = points[0];
        q0 = points[1];
        if (ApproxEqual(p0.axis.x, q0.axis.x, eps) && 
            ApproxEqual(p0.axis.y, q0.axis.y, eps) && 
            ApproxEqual(p0.axis.z, q0.axis.z, eps) && 
            ApproxEqual(p0.angle, q0.angle, eps)) 
            return 0;
    }
    return j;
}

int
reduceScaleValueKeys(Tab<TimeValue>& times, Tab<ScaleValue>& svs, float eps)
{
    if (times.Count() < 3)
        return times.Count();

    BOOL *used = new BOOL[times.Count()];
    BOOL alliso = (ApproxEqual(svs[0].s.x, svs[0].s.y, eps) &&
                   ApproxEqual(svs[0].s.x, svs[0].s.z, eps));
    int i;
    for (i = 0; i < times.Count(); i++)
        used[i] = TRUE;

    Point3 s0, t0;
    AngAxis p0, q0;
    for (i = 1; i < times.Count(); i++) {
        s0 = svs[i].s;
        t0 = svs[i-1].s;
        if (ApproxEqual(s0.x, t0.x, eps) &&
            ApproxEqual(s0.y, t0.y, eps) &&
            ApproxEqual(s0.z, t0.z, eps)) {
            AngAxisFromQa(svs[i].q, &p0.angle, p0.axis);
            AngAxisFromQa(svs[i-1].q, &q0.angle, q0.axis);
            if (ApproxEqual(p0.axis.x, q0.axis.x, eps) && 
                ApproxEqual(p0.axis.y, q0.axis.y, eps) && 
                ApproxEqual(p0.axis.z, q0.axis.z, eps) && 
                ApproxEqual(p0.angle, q0.angle, eps)) 
                used[i] = FALSE;
            else
                used[i-1] = TRUE;
        }
        else {
            used[i-1] = TRUE;
            alliso = FALSE;
        }
    }

    if (alliso) {       // scale always isotropic and constant
        delete [] used;
        return 0;
    }

    int j = 0;
    for (i = 0; i < times.Count(); i++)
        if (used[i])
            j++;
    if (j == 1) {
        delete [] used;
        return 0;
    }
    j = 0;
    for (i = 0; i < times.Count(); i++) {
        if (used[i]) {
            times[j] = times[i];
            svs[j] = svs[i];
            j++;
        }
    }
    times.SetCount(j);
    svs.SetCount(j);
    delete [] used;
    if (j == 1)
        return 0;
    if (j == 2) {
        s0 = svs[0].s;
        t0 = svs[1].s;
        AngAxisFromQa(svs[0].q, &p0.angle, p0.axis);
        AngAxisFromQa(svs[1].q, &q0.angle, q0.axis);
        if (ApproxEqual(s0.x, t0.x, eps) && 
            ApproxEqual(s0.y, t0.y, eps) && 
            ApproxEqual(s0.z, t0.z, eps) && 
            ApproxEqual(p0.axis.x, q0.axis.x, eps) && 
            ApproxEqual(p0.axis.y, q0.axis.y, eps) && 
            ApproxEqual(p0.axis.z, q0.axis.z, eps) && 
            ApproxEqual(p0.angle, q0.angle, eps)) 
            return 0;
    }
    return j;
}

// Write out all the keyframe data for the given controller
void
VRML2Export::WriteControllerData(INode* node,
                                 Tab<TimeValue>& posTimes,
                                 Tab<Point3>& posKeys,
                                 Tab<TimeValue>& rotTimes,
                                 Tab<AngAxis>& rotKeys,
                                 Tab<TimeValue>& sclTimes,
                                 Tab<ScaleValue>& sclKeys,
                                 int type, int level)
{
    AngAxis rval;
    Point3 p, s;
    Quat q;
    int i, width;
    TimeValue t;
    TCHAR name[128];
    Tab<TimeValue>& timeVals = posTimes;
    int newKeys;
    float eps;

    while (type) {

        // Set up
        switch (type) {
        case KEY_POS:
            eps = float(1.0e-5);
            newKeys = reducePoint3Keys(posTimes, posKeys, eps);
            if (newKeys == 0)
                return;
            timeVals = posTimes;
            sprintf(name, _T("%s-POS-INTERP"), mNodes.GetNodeName(node));
            Indent(level);
            fprintf(mStream, _T("DEF %s PositionInterpolator {\n"), name);
            break;
        case KEY_ROT:
            eps = float(1.0e-5);
            newKeys = reduceAngAxisKeys(rotTimes, rotKeys, eps);
            if (newKeys == 0)
                return;
            timeVals = rotTimes;
            sprintf(name, _T("%s-ROT-INTERP"), mNodes.GetNodeName(node));
            Indent(level);
            fprintf(mStream, _T("DEF %s OrientationInterpolator {\n"), name);
            break;
        case KEY_SCL:
            eps = float(1.0e-5);
            newKeys = reduceScaleValueKeys(sclTimes, sclKeys, eps);
            if (newKeys == 0)
                return;
            timeVals = sclTimes;
            sprintf(name, _T("%s-SCALE-INTERP"), mNodes.GetNodeName(node));
            Indent(level);
            fprintf(mStream, _T("DEF %s PositionInterpolator {\n"), name);
            break;
        case KEY_SCL_ORI:
            timeVals = sclTimes;
            sprintf(name, _T("%s-SCALE-ORI-INTERP"), mNodes.GetNodeName(node));
            Indent(level);
            fprintf(mStream, _T("DEF %s OrientationInterpolator {\n"), name);
            break;
        case KEY_COLOR:
            eps = float(1.0e-5);
            newKeys = reducePoint3Keys(posTimes, posKeys, eps);
            if (newKeys == 0)
                return;
            timeVals = posTimes;
            sprintf(name, _T("%s-COLOR-INTERP"), mNodes.GetNodeName(node));
            Indent(level);
            fprintf(mStream, _T("DEF %s ColorInterpolator {\n"), name);
            break;
        default: return;
        }

        AddInterpolator(name, type, mNodes.GetNodeName(node));
        // Now check to see if a TimeSensor references this node
        INodeList* l;
        for(l = mTimerList; l; l = l->GetNext()) {
            TimeSensorObject* tso = (TimeSensorObject*)
                l->GetNode()->EvalWorldState(mStart).obj;
            for(int j = 0; j < tso->TimeSensorObjects.Count(); j++) {
                INode* anim = tso->TimeSensorObjects[j]->node;
             // skip invalid nodes (ex. user create the list then delete the node from the scene.)
                if (anim && NodeIsChildOf(node, anim)) {
                    TSTR oTimer = mTimer;
                    TCHAR timer[MAX_PATH];
                    sprintf(timer, _T("%s"),mNodes.GetNodeName(l->GetNode()));
                    if (tso->needsScript)
                        AddInterpolator(name, KEY_TIMER_SCRIPT, timer);
                    else
                        AddInterpolator(name, KEY_TIMER, timer);
                    break;
                }
            }
        }
        
    
        // Output the key times
        mCycleInterval = (mIp->GetAnimRange().End() - mStart) /
            ((float) GetTicksPerFrame() * GetFrameRate());
        Indent(level+1);
        fprintf(mStream, _T("key ["));
        width = mIndent ? level * 2 : 0;
        for (i = 0; i < timeVals.Count(); i++) {
            t = timeVals[i] - mStart;
            if (t < 0)
                continue;
            width += fprintf(mStream, _T("%s, "),
                             floatVal(t / ((float) GetTicksPerFrame()
                                           * GetFrameRate() * mCycleInterval)));
            if (width > 60) {
                fprintf(mStream, _T("\n"));
                Indent(level+3);
                width = mIndent ? level * 2 : 0;
            }
        }
        fprintf(mStream, _T("]\n"));
        Indent(level+1);
        fprintf(mStream, _T("keyValue ["));

        width = mIndent ? level * 2 : 0;
        for (i=0; i < timeVals.Count(); i++) {
            t = timeVals[i];
            if (t < mStart)
                continue;

            // Write values
            switch (type) {
            case KEY_POS:
                mHadAnim = TRUE;
                p = posKeys[i];
                width += fprintf(mStream, _T("%s, "), point(p));
                break;
            
            case KEY_COLOR:
                mHadAnim = TRUE;
                p = posKeys[i];
                width += fprintf(mStream, _T("%s, "), color(p));
                break;
            
            case KEY_ROT:
                mHadAnim = TRUE;
                rval = rotKeys[i];
                width += fprintf(mStream, _T("%s, "),
                                 axisPoint(rval.axis, -rval.angle));
                break;
            case KEY_SCL:
                mHadAnim = TRUE;
                s = sclKeys[i].s;
                width += fprintf(mStream, _T("%s, "), scalePoint(s));
                break;
            case KEY_SCL_ORI:
                mHadAnim = TRUE;
                q = sclKeys[i].q;
                AngAxisFromQa(q, &rval.angle, rval.axis);
                width += fprintf(mStream, _T("%s, "),
                                 axisPoint(rval.axis, -rval.angle));
                break;
            
            }
            if (width > 50) {
                fprintf(mStream, _T("\n"));
                Indent(level+2);
                width = mIndent ? level * 2 : 0;
            }
        }
            
    //    Indent(level);
        fprintf(mStream, _T("] },\n"));

        type = (type == KEY_SCL ? KEY_SCL_ORI : 0);
    }               // while (type)
    return;
}

void
VRML2Export::WriteAllControllerData(INode* node, int flags, int level,
                                    Control* lc)
{
//TCHAR *name = node->GetName();    // for debugging
    float eps = float(1.0e-5);
    int i;
    int scalinc = 0;
    TimeValue t, prevT;
    TimeValue end = mIp->GetAnimRange().End();
    int frames;
    Point3 p, axis;
    ScaleValue s;
    Quat q;
    Quat oldu(0.0, 0.0, 0.0, 1.0);
    Matrix3 tm, ip;
    float ang;
    BOOL isCamera = IsCamera(node);
    int sampleRate;

    if (mTformSample)
        sampleRate = GetTicksPerFrame();
    else
        sampleRate = TIME_TICKSPERSEC / mTformSampleRate;
    frames = (end - mStart)/sampleRate + 1;
        
    int realEnd = end;
    if (((end - mStart) % sampleRate) != 0) {
        end += sampleRate;
        frames++;
    }
    
    
    // Tables of keyframe values
    Tab<Point3>     posKeys;
    Tab<TimeValue>  posTimes;
    Tab<ScaleValue> scaleKeys;
    Tab<TimeValue>  scaleTimes;
    Tab<AngAxis>    rotKeys;
    Tab<TimeValue>  rotTimes;

    // Set up 'k' to point at the right derived class
    if (flags & KEY_POS) {
        posKeys.SetCount(frames);
        posTimes.SetCount(frames);
    }
    if (flags & KEY_ROT) {
        rotKeys.SetCount(frames);
        rotTimes.SetCount(frames);
    }
    if (flags & KEY_SCL) {
        scaleKeys.SetCount(frames);
        scaleTimes.SetCount(frames);
    }
    if (flags & KEY_COLOR) {
        posKeys.SetCount(frames);
        posTimes.SetCount(frames);
    }

    for(i = 0, t = mStart; i < frames; i++, t += sampleRate) {
        if (t > realEnd)
            t = realEnd;
        if (flags & KEY_COLOR) {
            lc->GetValue(t, &posKeys[i], FOREVER);
            posTimes[i] = t;
            continue;
        }
        // otherwise we are sampling tform controller data

        AffineParts parts;
        if (!isCamera) {
            tm = GetLocalTM(node, t);
        } else {
            // We have a camera
            tm = node->GetObjTMAfterWSM(t);
        }
#ifdef DDECOMP
        d_decomp_affine(tm, &parts);
#else
        decomp_affine(tm, &parts);      // parts is parts
#endif

        if (flags & KEY_SCL) {
            s = ScaleValue(parts.k, parts.u);
            if (parts.f < 0.0f)
                s.s = - s.s;
#define AVOID_NEG_SCALE
#ifdef AVOID_NEG_SCALE
            if (s.s.x <= 0.0f && s.s.y <= 0.0f && s.s.z <= 0.0f) {
                s.s = - s.s;
                s.q = Conjugate(s.q);
            }
#endif
            // The following unholy kludge deals with the surprising fact
            // that, as a TM changes gradually, decomp_affine() may introduce
            // a sudden flip of sign in U at the same time as a jump in the
            // scale orientation axis.
            if (parts.u.x * oldu.x < -eps || parts.u.y * oldu.y < -eps ||
                        parts.u.z * oldu.z < -eps) {
                AffineParts pts;
                Matrix3 mat;
                TimeValue lowt, hight, midt;
                ScaleValue sv;
                int ct = scaleTimes.Count();

                for (hight = t, lowt = prevT;
                     hight - lowt > 1;        // 1/4800 sec.
                     ) {
                    midt = (hight + lowt) / 2;
                    if (!isCamera)
                        mat = GetLocalTM(node, midt);
                    else
                        mat = node->GetObjTMAfterWSM(midt);
#ifdef DDECOMP
                    d_decomp_affine(mat, &pts);
#else
                    decomp_affine(mat, &pts);
#endif
                    if (pts.u.x * oldu.x < -eps ||
                        pts.u.y * oldu.y < -eps ||
                        pts.u.z * oldu.z < -eps) 
                        hight = midt;
                    else
                        lowt = midt;
                }
                if (lowt > prevT) {
                    if (!isCamera)
                        mat = GetLocalTM(node, lowt);
                    else
                        mat = node->GetObjTMAfterWSM(lowt);
#ifdef DDECOMP
                    d_decomp_affine(mat, &pts);
#else
                    decomp_affine(mat, &pts);
#endif
                    sv = ScaleValue(pts.k, pts.u);
                    if (pts.f < 0.0f)
                        sv.s = - sv.s;
#ifdef AVOID_NEG_SCALE
                    if (sv.s.x <= 0.0f && sv.s.y <= 0.0f && sv.s.z <= 0.0f) {
                        sv.s = - sv.s;
                        sv.q = Conjugate(sv.q);
                    }
#endif
                    ct++;
                    scaleTimes.SetCount(ct);
                    scaleKeys.SetCount(ct);
                    scaleTimes[i + scalinc] = midt;
                    scaleKeys[i + scalinc] = sv;
                    scalinc++;
                }
                if (hight < t) {
                    if (!isCamera)
                        mat = GetLocalTM(node, hight);
                    else
                        mat = node->GetObjTMAfterWSM(hight);
#ifdef DDECOMP
                    d_decomp_affine(mat, &pts);
#else
                    decomp_affine(mat, &pts);
#endif
                    sv = ScaleValue(pts.k, pts.u);
                    if (pts.f < 0.0f)
                        sv.s = - sv.s;
#ifdef AVOID_NEG_SCALE
                    if (sv.s.x <= 0.0f && sv.s.y <= 0.0f && sv.s.z <= 0.0f) {
                        sv.s = - sv.s;
                        sv.q = Conjugate(sv.q);
                    }
#endif
                    ct++;
                    scaleTimes.SetCount(ct);
                    scaleKeys.SetCount(ct);
                    scaleTimes[i + scalinc] = midt;
                    scaleKeys[i + scalinc] = sv;
                    scalinc++;
                }
            }
            if (parts.u.x != 0.0f)
                oldu.x = parts.u.x;
            if (parts.u.y != 0.0f)
                oldu.y = parts.u.y;
            if (parts.u.z != 0.0f)
                oldu.z = parts.u.z;

            scaleTimes[i + scalinc] = t;
            scaleKeys[i + scalinc]  = s;
        }
            
        if (flags & KEY_POS) {
            p = parts.t;
            posTimes[i] = t;
            posKeys[i]  = p;
        }
            
        if (flags & KEY_ROT) {
            q = parts.q;
            if (isCamera && !mZUp) {
                // Now rotate around the X Axis PI/2
                Matrix3 rot = RotateXMatrix(PI/2);
                Quat qRot(rot);
                AngAxisFromQa(q/qRot, &ang, axis);
            } else
                AngAxisFromQa(q, &ang, axis);
            rotTimes[i] = t;
            rotKeys[i] = AngAxis(axis, ang);
        }
        prevT = t;
    }
    if (flags & KEY_POS) {
        WriteControllerData(node,
                            posTimes,   posKeys,
                            rotTimes,   rotKeys,
                            scaleTimes, scaleKeys,
                            KEY_POS, level);
    }
    if (flags & KEY_ROT) {
        WriteControllerData(node,
                            posTimes,   posKeys,
                            rotTimes,   rotKeys,
                            scaleTimes, scaleKeys,
                            KEY_ROT, level);
    }
    if (flags & KEY_SCL && !isCamera) {
        WriteControllerData(node,
                            posTimes,   posKeys,
                            rotTimes,   rotKeys,
                            scaleTimes, scaleKeys,
                            KEY_SCL, level);
    }
    if (flags & KEY_COLOR) {
        WriteControllerData(node,
                            posTimes,   posKeys,
                            rotTimes,   rotKeys,
                            scaleTimes, scaleKeys,
                            KEY_COLOR, level);
    }
}
    
void
VRML2Export::WriteVisibilityData(INode *node, int level) {
    int i;
    TimeValue t;
    int frames = mIp->GetAnimRange().End()/GetTicksPerFrame();
    BOOL lastVis = TRUE, vis;

    // Now generate the Hide keys
    for(i = 0, t = mStart; i <= frames; i++, t += GetTicksPerFrame()) {
	vis = node->GetVisibility(t) <= 0.0f ? FALSE : TRUE;
        if (vis != lastVis) {
            mHadAnim = TRUE;
            Indent(level);
            fprintf(mStream, _T("HideKey_ktx_com {\n"));
            if (mGenFields) {
                Indent(level+1);
                fprintf(mStream, _T("fields [ SFLong frame] \n"));
            }
            Indent(level+1);
            fprintf(mStream, _T("frame %d\n"), i);
            Indent(level);
            fprintf(mStream, _T("}\n"));
        }
        lastVis = vis;
    }    
}

BOOL
VRML2Export::IsLight(INode* node)
{
    Object* obj = node->EvalWorldState(mStart).obj;
    if (!obj)
        return FALSE;

    SClass_ID sid = obj->SuperClassID();
    return sid == LIGHT_CLASS_ID;
}

BOOL
VRML2Export::IsCamera(INode* node)
{
    Object* obj = node->EvalWorldState(mStart).obj;
    if (!obj)
        return FALSE;

    SClass_ID sid = obj->SuperClassID();
    return sid == CAMERA_CLASS_ID;
}

BOOL
VRML2Export::IsAudio(INode* node)
{
    Object* obj = node->EvalWorldState(mStart).obj;
    if (!obj)
        return FALSE;

    Class_ID cid = obj->ClassID();
    return cid == AudioClipClassID;
}

static Control* GetController(Object* obj, const TCHAR* name)
{
	Control* c = NULL;
	if (obj != NULL) {
		one_value_local(prop);			// Keep one local variables

		try {
			ParamDimension* dim;

			// Get the name and value to set
			vl.prop = Name::intern(const_cast<TCHAR*>(name));

			// Get the value.
			c = MAXWrapper::get_max_prop_controller(obj, vl.prop, &dim);
		} catch (MAXScriptException& ex) {
			(void)ex;
		}
		pop_value_locals();
	}
	return c;
}

Control *
VRML2Export::GetLightColorControl(INode* node)
{
    if (!IsLight(node))
        return NULL;
    Object* obj = node->EvalWorldState(mStart).obj;
	// MaxScript is more reliable for getting controllers
	return GetController(obj, "rgb");
//    IParamBlock *pblock = (IParamBlock *) obj->SubAnim(0);
//    Control* cont = pblock->GetController(0);  // I know color is index 0!
//    return cont;
}

#define NeedsKeys(nkeys) ((nkeys) > 1 || (nkeys) == NOT_KEYFRAMEABLE)

// Write out keyframe data, if it exists
void
VRML2Export::VrmlOutControllers(INode* node, int level)
{

#ifdef _LEC_
    if (mFlipBook)
        return;
#endif
    Control *pc, *rc, *sc, *lc;
    int npk = 0, nrk = 0, nsk = 0, nvk = 0, nlk = 0;

    int flags = 0;
    BOOL isCamera = IsCamera(node);
    Object *obj = node->EvalWorldState(mStart).obj;
    int ts = NodeNeedsTimeSensor(node);

    if (ts != 0) {
        mCycleInterval = (mIp->GetAnimRange().End() - mStart) /
            ((float) GetTicksPerFrame()* GetFrameRate());
        Indent(level);
        fprintf(mStream,
         _T("DEF %s-TIMER TimeSensor { loop %s cycleInterval %s },\n"),
                mNodes.GetNodeName(node),
                (ts < 0) ? _T("TRUE") : _T("FALSE"),
                floatVal(mCycleInterval));
    }

    lc = GetLightColorControl(node);
    if (lc) nlk = lc->NumKeys();
    if (NeedsKeys(nlk))
        WriteAllControllerData(node, KEY_COLOR, level, lc);

    Class_ID id = node->GetTMController()->ClassID();

    if (!node->IsAnimated() && id == Class_ID(PRS_CONTROL_CLASS_ID, 0) &&
            !isCamera && !IsLight(node))
        return;

#ifdef _DEBUG
    int inhf = node->GetTMController()->GetInheritanceFlags();
    int inhb = node->GetTMController()->InheritsParentTransform();
#endif
    
    if (!isCamera && id != Class_ID(PRS_CONTROL_CLASS_ID, 0))
        flags = KEY_POS | KEY_ROT | KEY_SCL;
    else if (isCamera && (IsEverAnimated(node) ||
                          IsEverAnimated(node->GetTarget())))
        flags = KEY_POS | KEY_ROT;
    else {
        pc = node->GetTMController()->GetPositionController();
        if (pc) npk = pc->NumKeys();
        rc = node->GetTMController()->GetRotationController();
        if (rc) nrk = rc->NumKeys();
        sc = node->GetTMController()->GetScaleController();
        if (sc) nsk = sc->NumKeys();
        if (NeedsKeys(npk) || NeedsKeys(nrk) || NeedsKeys(nsk))
            flags = KEY_POS | KEY_ROT | KEY_SCL;
    }
    if (flags)
        WriteAllControllerData(node, flags, level, NULL);
#if 0
    Control* vc = node->GetVisController();
    if (vc) nvk = vc->NumKeys();
    if (NeedsKeys(nvk))
        WriteVisibilityData(node, level);
#endif
}

void
VRML2Export::VrmlOutTopLevelCamera(int level, INode* node, BOOL topLevel)
{
    if (!topLevel && node == mCamera)
        return;
        
    CameraObject* cam = (CameraObject*) node->EvalWorldState(mStart).obj;
    Matrix3 tm = node->GetObjTMAfterWSM(mStart);
    Point3 p, s, axis;
    Quat q;
    float ang;

    AffineParts parts;
    decomp_affine(tm, &parts);
    p = parts.t;
    q = parts.q;
    if (!mZUp) {
        // Now rotate around the X Axis PI/2
        Matrix3 rot = RotateXMatrix(PI/2);
        Quat qRot(rot);
        AngAxisFromQa(q/qRot, &ang, axis);
    } else
        AngAxisFromQa(q, &ang, axis);

    // compute camera transform
    ViewParams vp;
    CameraState cs;
    Interval iv;
    cam->EvalCameraState(0, iv, &cs);
    vp.fov = (float)(2.0 * atan(tan(cs.fov / 2.0) / INTENDED_ASPECT_RATIO));

    Indent(level);
    fprintf(mStream, _T("DEF %s Viewpoint {\n"), mNodes.GetNodeName(node));
    Indent(level+1);
    fprintf(mStream, _T("position %s\n"), point(p));
    Indent(level+1);
    fprintf(mStream, _T("orientation %s\n"), axisPoint(axis, -ang));
    Indent(level+1);
    fprintf(mStream, _T("fieldOfView %s\n"), floatVal(vp.fov));
    Indent(level + 1);
    fprintf(mStream, _T("description \"%s\"\n"), mNodes.GetNodeName(node));
    Indent(level);
    fprintf(mStream, _T("}\n"));

    // Write out any animation data
    InitInterpolators(node);
    VrmlOutControllers(node, 0);
    WriteInterpolatorRoutes(level, TRUE);
}

// In navinfo.cpp
extern TCHAR* navTypes[];

void
VRML2Export::VrmlOutTopLevelNavInfo(int level, INode* node, BOOL topLevel)
{
    if (!topLevel && node == mNavInfo)
        return;

    NavInfoObject* ni = (NavInfoObject*) node->EvalWorldState(mStart).obj;
    int type, headlight;
    float visLimit, speed, collision, terrain, step;
    ni->pblock->GetValue(PB_TYPE, mIp->GetTime(), type, FOREVER);
    ni->pblock->GetValue(PB_HEADLIGHT, mIp->GetTime(), headlight, FOREVER);
    ni->pblock->GetValue(PB_VIS_LIMIT, mIp->GetTime(), visLimit, FOREVER);
    ni->pblock->GetValue(PB_SPEED, mIp->GetTime(), speed, FOREVER);
    ni->pblock->GetValue(PB_COLLISION, mIp->GetTime(), collision, FOREVER);
    ni->pblock->GetValue(PB_TERRAIN, mIp->GetTime(), terrain, FOREVER);
    ni->pblock->GetValue(PB_STEP, mIp->GetTime(), step, FOREVER);
    Indent(level);
    fprintf(mStream, _T("DEF %s NavigationInfo {\n"),mNodes.GetNodeName(node));
    Indent(level+1);
    fprintf(mStream, _T("avatarSize [%s, "), floatVal(collision));
    fprintf(mStream, _T("%s, "), floatVal(terrain));
    fprintf(mStream, _T("%s]\n"), floatVal(step));
    Indent(level+1);
    fprintf(mStream, _T("headlight %s\n"),
            headlight ? _T("TRUE") : _T("FALSE"));
    Indent(level+1);
    fprintf(mStream, _T("speed %s\n"), floatVal(speed));
    Indent(level+1);
    if (type < 0 || type > 3)
        type = 0;
    fprintf(mStream, _T("type \"%s\"\n"), navTypes[type]);
    Indent(level+1);
    fprintf(mStream, _T("visibilityLimit %s\n"), floatVal(visLimit));
    Indent(level);
    fprintf(mStream, _T("}\n"));
}

void
VRML2Export::VrmlOutTopLevelBackground(int level, INode* node, BOOL topLevel)
{
    if (!topLevel && node == mBackground)
        return;

    BackgroundObject* bg = (BackgroundObject*)
        node->EvalWorldState(mStart).obj;
    int numColors, i;
    Point3 col[3];
    float angle2, angle3;

    bg->pblock->GetValue(PB_SKY_NUM_COLORS, mIp->GetTime(), numColors,
                         FOREVER);
    bg->pblock->GetValue(PB_SKY_COLOR1, mIp->GetTime(), col[0], FOREVER);
    bg->pblock->GetValue(PB_SKY_COLOR2, mIp->GetTime(), col[1], FOREVER);
    bg->pblock->GetValue(PB_SKY_COLOR3, mIp->GetTime(), col[2], FOREVER);
    bg->pblock->GetValue(PB_SKY_COLOR2_ANGLE, mIp->GetTime(), angle2,
                         FOREVER);
    bg->pblock->GetValue(PB_SKY_COLOR3_ANGLE, mIp->GetTime(), angle3,
                         FOREVER);

    Indent(level);
    fprintf(mStream, _T("DEF %s Background {\n"),mNodes.GetNodeName(node));
    Indent(level+1);
    fprintf(mStream, _T("skyColor ["));
    for(i = 0; i < numColors + 1; i++)
        fprintf(mStream, _T("%s, "), color(col[i]));
    fprintf(mStream, _T("]\n"));
    
    if (numColors > 0) {
        Indent(level+1);
        fprintf(mStream, _T("skyAngle ["));
        fprintf(mStream, _T("%s, "), floatVal(angle2));
        if (numColors > 1)
            fprintf(mStream, _T("%s, "), floatVal(angle3));
        fprintf(mStream, _T("]\n"));
    }

    bg->pblock->GetValue(PB_GROUND_NUM_COLORS, mIp->GetTime(), numColors,
                         FOREVER);
    bg->pblock->GetValue(PB_GROUND_COLOR1, mIp->GetTime(), col[0], FOREVER);
    bg->pblock->GetValue(PB_GROUND_COLOR2, mIp->GetTime(), col[1], FOREVER);
    bg->pblock->GetValue(PB_GROUND_COLOR3, mIp->GetTime(), col[2], FOREVER);
    bg->pblock->GetValue(PB_GROUND_COLOR2_ANGLE, mIp->GetTime(), angle2,
                         FOREVER);
    bg->pblock->GetValue(PB_GROUND_COLOR3_ANGLE, mIp->GetTime(), angle3,
                         FOREVER);

    Indent(level+1);
    fprintf(mStream, _T("groundColor ["));
    for(i = 0; i < numColors + 1; i++)
        fprintf(mStream, _T("%s, "), color(col[i]));
    fprintf(mStream, _T("]\n"));
    
    if (numColors > 0) {
        Indent(level+1);
        fprintf(mStream, _T("groundAngle ["));
        fprintf(mStream, _T("%s, "), floatVal(angle2));
        if (numColors > 1)
            fprintf(mStream, _T("%s, "), floatVal(angle3));
        fprintf(mStream, _T("]\n"));
    }

    TSTR url;
    if (bg->back.Length() > 0) {
        Indent(level+1);
        url = PrefixUrl(bg->back);
        fprintf(mStream, _T("backUrl \"%s\"\n"), url.data());
    }
    if (bg->bottom.Length() > 0) {
        Indent(level+1);
        url = PrefixUrl(bg->bottom);
        fprintf(mStream, _T("bottomUrl \"%s\"\n"), url.data());
    }
    if (bg->front.Length() > 0) {
        Indent(level+1);
        url = PrefixUrl(bg->front);
        fprintf(mStream, _T("frontUrl \"%s\"\n"), url.data());
    }
    if (bg->left.Length() > 0) {
        Indent(level+1);
        url = PrefixUrl(bg->left);
        fprintf(mStream, _T("leftUrl \"%s\"\n"), url.data());
    }
    if (bg->right.Length() > 0) {
        Indent(level+1);
        url = PrefixUrl(bg->right);
        fprintf(mStream, _T("rightUrl \"%s\"\n"), url.data());
    }
    if (bg->top.Length() > 0) {
        Indent(level+1);
        url = PrefixUrl(bg->top);
        fprintf(mStream, _T("topUrl \"%s\"\n"), url.data());
    }
    Indent(level);
    fprintf(mStream, _T("}\n"));
}

void
VRML2Export::VrmlOutTopLevelFog(int level, INode* node, BOOL topLevel)
{
    if (!topLevel && node == mFog)
        return;

    FogObject* fog = (FogObject*) node->EvalWorldState(mStart).obj;
    Point3 p;
    float visLimit;
    int type;
    fog->pblock->GetValue(PB_COLOR, mIp->GetTime(), p, FOREVER);
    fog->pblock->GetValue(PB_TYPE, mIp->GetTime(), type, FOREVER);
    fog->pblock->GetValue(PB_VIS_LIMIT, mIp->GetTime(), visLimit, FOREVER);
    Indent(level);
    fprintf(mStream, _T("DEF %s Fog {\n"), mNodes.GetNodeName(node));
    Indent(level+1);
    fprintf(mStream, _T("color %s\n"), color(p));
    Indent(level+1);
    fprintf(mStream, _T("fogType \"%s\"\n"), type == 0 ? _T("LINEAR") :
            _T("EXPONENTIAL"));
    Indent(level+1);
    fprintf(mStream, _T("visibilityRange %s\n"), floatVal(visLimit));
    Indent(level);
    fprintf(mStream, _T("}\n"));
}

void
VRML2Export::VrmlOutInitializeAudioClip(int level, INode* node)
{
    AudioClipObject* ac = (AudioClipObject*) node->EvalWorldState(mStart).obj;
    if (ac)
        ac->written = 0;
}

void
VRML2Export::VrmlOutAudioClip(int level, INode* node)
{
    float pitch;
    int loop, start;
    AudioClipObject* ac = (AudioClipObject*) node->EvalWorldState(mStart).obj;
    if (ac->written) {
        Indent(level);
        fprintf(mStream, _T("USE %s\n"), mNodes.GetNodeName(node));
        return;
    }
    ac->pblock->GetValue(PB_AC_PITCH, mIp->GetTime(), pitch, FOREVER);
    ac->pblock->GetValue(PB_AC_LOOP, mIp->GetTime(), loop, FOREVER);
    ac->pblock->GetValue(PB_AC_START, mIp->GetTime(), start, FOREVER);

    Indent(level);
    fprintf(mStream, _T("DEF %s AudioClip {\n"), mNodes.GetNodeName(node));
    Indent(level+1);
    fprintf(mStream, _T("description \"%s\"\n"), ac->desc.data());
    Indent(level+1);
    fprintf(mStream, _T("url \"%s\"\n"), ac->url.data());
    Indent(level+1);
    fprintf(mStream, _T("pitch %s\n"), floatVal(pitch));
    Indent(level+1);
    fprintf(mStream, _T("loop %s\n"), loop ? _T("TRUE") : _T("FALSE"));
    Indent(level+1);
    if (start)
        fprintf(mStream, _T("startTime 1\n"));
    else
        fprintf(mStream, _T("stopTime 1\n"));
    Indent(level);
    fprintf(mStream, _T("}\n"));
    ac->written = TRUE;
}

// From dllmain.cpp
extern HINSTANCE hInstance;

void
VRML2Export::VrmlOutFileInfo()
{
    char filename[MAX_PATH];
    DWORD size, dummy;
    float vernum = 2.0f;
    float betanum = 0.0f;

    GetModuleFileName(hInstance, filename, MAX_PATH);
    size = GetFileVersionInfoSize(filename, &dummy);
    if (size) {
        char *buf = (char *)malloc(size);
        GetFileVersionInfo(filename, NULL, size, buf);
        VS_FIXEDFILEINFO *qbuf;
        UINT len;
        if (VerQueryValue(buf, "\\", (void **)&qbuf, &len)) {
            // got the version information
            DWORD ms = qbuf->dwProductVersionMS;
            DWORD ls = qbuf->dwProductVersionLS;
            vernum = HIWORD(ms) + (LOWORD(ms) / 100.0f);
            betanum = HIWORD(ls) + (LOWORD(ls) / 100.0f);
        }
        free(buf);
    }
    
    fprintf(mStream, _T("# Produced by 3D Studio MAX VRML97 exporter, Version %.5g, Revision %.5g\n"),
        vernum, betanum);
    
    
    time_t ltime;
    time( &ltime );
    char * time = ctime(&ltime);
    // strip the CR
    time[strlen(time)-1] = '\0';
    TCHAR* fn = mIp->GetCurFileName();
    if (fn && _tcslen(fn) > 0) {
        fprintf(mStream, _T("# MAX File: %s, Date: %s\n\n"), fn, time);
    } else {
        fprintf(mStream, _T("# Date: %s\n\n"), time);
    }
}

void
VRML2Export::VrmlOutWorldInfo()
{
    if (mTitle.Length() == 0 && mInfo.Length() == 0)
        return;

    fprintf(mStream, _T("WorldInfo {\n"));
    if (mTitle.Length() != 0) {
        Indent(1);
        fprintf(mStream, _T("title \"%s\"\n"), mTitle.data());
    }
    if (mInfo.Length() != 0) {
        Indent(1);
        fprintf(mStream, _T("info \"%s\"\n"), mInfo.data());
    }
    fprintf(mStream, _T("}\n"));
}

int
VRML2Export::StartAnchor(INode* node, int& level)
{
    SensorBucket *sb = mSensorTable.FindSensor(node);
    if (!sb)
        return 0;
    INode *sensor;
    INodeList *l;
    int numAnchors = 0;
    for(l = sb->mSensors; l; l = l->GetNext()) {
        sensor = l->GetNode();
        Object *obj = sensor->EvalWorldState(mStart).obj;
        assert(obj);
        if (obj->ClassID() == AnchorClassID) {
            numAnchors++;
            AnchorObject* ao = (AnchorObject*)
                sensor->EvalWorldState(mStart).obj;
            Indent(level);
            fprintf(mStream, _T("Anchor {\n"));
            level++;
            Indent(level);
            fprintf(mStream, _T("description \"%s\"\n"), ao->description.data());
            int type;
            ao->pblock->GetValue(PB_AN_TYPE, mStart, type, FOREVER);
            if (type == 0) {
                Indent(level);
                fprintf(mStream, _T("parameter \"%s\"\n"), ao->parameter.data());
                Indent(level);
                fprintf(mStream, _T("url \"%s\"\n"), ao->URL.data());
            } else {
                if (ao->cameraObject) {
                    Indent(level);
                    fprintf(mStream, _T("url \"#%s\"\n"),
                            VRMLName(ao->cameraObject->GetName()));
                }
            }
            Indent(level);
            fprintf(mStream, _T("children [\n"));
            level++;
        }
    }
    return numAnchors;
}

// Recursively count a node and all its children
static int
CountNodes(INode *node)
{
    int total, kids, i;
    
    if (node == NULL)
        return 0;
    total = 1;
    kids = node->NumberOfChildren();
    for (i = 0; i < kids; i++)
        total += CountNodes(node->GetChildNode(i));
    return total;
}

// Output a single node as VRML and recursively output the children of
// the node.
void
VRML2Export::VrmlOutNode(INode* node, INode* parent, int level, BOOL isLOD,
                         BOOL lastChild, BOOL mirrored) {

 // Don't gen code for LOD references, only LOD nodes
    if (!isLOD && ObjectIsLODRef(node)) return;
    
    if (mEnableProgressBar) SendMessage(hWndPDlg, 666, 0,
                                        (LPARAM) mNodes.GetNodeName(node));
    
    Object* obj         = node->EvalWorldState(mStart).obj;
    BOOL    outputName  = TRUE;
    int     numChildren = node->NumberOfChildren();
    BOOL    isVrml      = isVrmlObject(node, obj, parent);
    BOOL    numAnchors  = 0;
    BOOL    written     = FALSE;
    BOOL    mirror      = FALSE;
    int     cnt;

 
    if (node->IsRootNode()) {
        VrmlOutWorldInfo();
     // Compute the world bounding box and a list of timesensors
        ScanSceneGraph1();

        if (mCamera)
            VrmlOutTopLevelCamera(level+ 2, mCamera, TRUE);
        if (mNavInfo)
            VrmlOutTopLevelNavInfo(level+ 2, mNavInfo, TRUE);
        if (mBackground)
            VrmlOutTopLevelBackground(level+ 2, mBackground, TRUE);
        if (mFog)
            VrmlOutTopLevelFog(level+ 2, mFog, TRUE);

     // Make a list of al the LOD objects and texture maps in the scene.
     // Also output top-level objects
        ScanSceneGraph2();

        if (mHasLights && !mHasNavInfo) {
            fprintf(mStream, _T("NavigationInfo { headlight FALSE }\n"));
        }
    }

 // give third party dlls a chance to write the node
    if (!node->IsRootNode()) {
        written = FALSE;
        for (cnt = 0; cnt < mCallbacks->GetPreNodeCount(); cnt++) {
            DllPreNode preNode = mCallbacks->GetPreNode(cnt);
            PreNodeParam params;
            params.version  = 0;
            params.indent   = level;
            params.fName    = mFilename;
            params.i        = mIp;
            params.node     = node;

            if (mStream)
                fclose(mStream), mStream = NULL;
            written = (*(preNode))(&params);
            if (written)
                break; // only the first one gets to write the node
        }
        if (!mStream)
            mStream = _tfopen(mFilename, _T("a"));
    }

 // Anchors need to come first, even though they are child nodes
    if (!node->IsRootNode() && !written) {
        /* test
        int newLevel = StartMrBlueHelpers(node, level);
        level = newLevel;
        */
        numAnchors = StartAnchor(node, level);
     // Initialize set of timers/interpolator per top-level node
        if ( node->GetParentNode()->IsRootNode())
            InitInterpolators(node);
    }

    if (isVrml && !written) {
        StartNode(node, level, outputName);
        if (!IsLODObject(obj))
            mirror = OutputNodeTransform(node, level+1, mirrored);

     // Output the data for the object at this node
        Indent(level+1);
        fprintf(mStream, _T("children [\n"));
        if (!IsLODObject(obj)) {
         // If the node has a controller, output the data
            VrmlOutControllers(node, level+1);
        }
    }
    if ((isVrml && (mExportHidden || !node->IsHidden()) && !written) ||
        IsAnimTrigger(obj)) {
        VrmlOutObject(node, parent, obj, level+2, mirrored ^ mirror);
    }
    
    if (mEnableProgressBar) SendMessage(hWndPB, PBM_STEPIT, 0, 0);

    // Now output the children
    if (!(written & WroteNodeChildren)) {
        for(int i = 0; i < numChildren; i++) {
            VrmlOutNode(node->GetChildNode(i), node, level+2, FALSE,
                i == numChildren - 1, mirrored ^ mirror);
        }
    }

 // need to get a valid obj ptr VrmlOutNode (VrmlOutCoordinateInterpolator)
 // causes the obj ptr (cache) to be invalid
    obj = node->EvalWorldState(mStart).obj;

    if (obj && (obj->ClassID() == BillboardClassID) && (numChildren > 0) && !written) {
        Indent(level+1);
        fprintf(mStream, _T("] }\n"));
    }

    if (!node->IsRootNode() && isVrml && !written) {
        OutputTouchSensors(node, level);
        Indent(level+1);
        fprintf(mStream, _T("]\n"));
    }

    if (!node->IsRootNode() && !written) {
        if (node->GetParentNode()->IsRootNode())
            WriteInterpolatorRoutes(level, FALSE);  // must be in place of field
    }
    if (isVrml && !node->IsRootNode() && !written)
        EndNode(node, obj, level, lastChild);
 
 // give third party dlls a chance to finish up the node
    if (!node->IsRootNode()) {
        for (cnt = 0; cnt < mCallbacks->GetPostNodeCount(); cnt++) {
            DllPostNode postNode = mCallbacks->GetPostNode(cnt);
            PostNodeParam params;
            params.version  = 0;
            params.indent   = level;
            params.fName    = mFilename;
            params.i        = mIp;
            params.node     = node;

            if (mStream)
                fclose(mStream), mStream = NULL;

            (*(postNode))(&params);
        }
        if (!mStream)
            mStream = _tfopen(mFilename, _T("a"));
    }
    
    // End the anchors if needed
    if (!node->IsRootNode() && !written) {
        /* test
        EndMrBlueHelpers(node, level);
        */
        for (; numAnchors > 0; numAnchors--) {
            Indent(level);
            fprintf(mStream, _T("] }\n"));
            level--;
        }
//        if (node->GetParentNode()->IsRootNode())
//            WriteInterpolatorRoutes(level, FALSE);
    }
}

void
VRML2Export::OutputTouchSensors(INode* node, int level)
{
    SensorBucket *sb = mSensorTable.FindSensor(node);
    if (!sb)
        return;
    INode *sensor;
    INodeList *l;
    for(l = sb->mSensors; l; l = l->GetNext()) {
        sensor = l->GetNode();
        Object *obj = sensor->EvalWorldState(mStart).obj;
        assert(obj);
        if (obj->ClassID() == TouchSensorClassID)
            VrmlOutTouchSensor(sensor, level);
    }
}

// Traverse the scene graph looking for LOD nodes and texture maps.
// Mark nodes affected by sensors (time, touch, proximity).
void
VRML2Export::TraverseNode(INode* node)
{
    if (!node) return;
    Object* obj = node->EvalWorldState(mStart).obj;
    Class_ID id;
    
    if (obj) {
        id = obj->ClassID();
        if (id == Class_ID(LOD_CLASS_ID1, LOD_CLASS_ID2))
            mLodList = mLodList->AddNode(node);
        
        if (IsLight(node)) {
            mHasLights = TRUE;
            if (!IsEverAnimated(node) && !IsEverAnimated(node->GetTarget())) {
                OutputTopLevelLight(node, (LightObject*) obj);
            }
        }
        if ((id == Class_ID(SIMPLE_CAM_CLASS_ID, 0) ||
             id == Class_ID(LOOKAT_CAM_CLASS_ID, 0)))
            VrmlOutTopLevelCamera(0, node, FALSE);
        
        if (id == NavInfoClassID) {
            mHasNavInfo = TRUE;
            VrmlOutTopLevelNavInfo(0, node, FALSE);
        }
        
        if (id == BackgroundClassID)
            VrmlOutTopLevelBackground(0, node, FALSE);
        
        if (id == FogClassID)
            VrmlOutTopLevelFog(0, node, FALSE);
        
        if (id == AudioClipClassID)
            VrmlOutInitializeAudioClip(0, node);
        
        if (id == TouchSensorClassID) {
            TouchSensorObject *ts = (TouchSensorObject*) obj;
            if (ts->triggerObject) {
                mSensorTable.AddSensor(ts->triggerObject, node);
            }
            int ct;
            INode *nd;
            for (ct = ts->objects.Count() - 1; ct >= 0; ct--) {
                nd = ts->objects[ct]->node;
                nd->SetNodeLong(nd->GetNodeLong() | RUN_BY_TOUCH_SENSOR);
            }
        }
        if (id == ProxSensorClassID) {
            ProxSensorObject *ps = (ProxSensorObject*) obj;
            int ct;
            INode *nd;
            for (ct = ps->objects.Count() - 1; ct >= 0; ct--) {
                nd = ps->objects[ct]->node;
                nd->SetNodeLong(nd->GetNodeLong() | RUN_BY_PROX_SENSOR);
            }
        }
        if (id == TimeSensorClassID) {
            TimeSensorObject *ts = (TimeSensorObject*) obj;
            int ct;
            INode *nd;
            for (ct = ts->TimeSensorObjects.Count() - 1; ct >= 0; ct--) {
                nd = ts->TimeSensorObjects[ct]->node;
                nd->SetNodeLong(nd->GetNodeLong() | RUN_BY_TIME_SENSOR);
            }
        }
        if (id == AnchorClassID) {
            AnchorObject *ao = (AnchorObject*) obj;
            if (ao->triggerObject) {
                mSensorTable.AddSensor(ao->triggerObject, node);
            }
        }
        ObjectBucket* ob = mObjTable.AddObject(obj);
        if (!ob->hasName) {
            ob->name = mNodes.GetNodeName(node);
            ob->hasName = TRUE;
        }
        if (!ob->hasInstName && !ObjIsPrim(node, obj)) {
            ob->instName = mNodes.GetNodeName(node);
            ob->hasInstName = TRUE;
        }

    }

    int n = node->NumberOfChildren();
    for(int i = 0; i < n; i++)
        TraverseNode(node->GetChildNode(i));
}

void
VRML2Export::ComputeWorldBoundBox(INode* node, ViewExp* vpt)
{
    if (!node) return;
    Object* obj = node->EvalWorldState(mStart).obj;
    Class_ID id;

    node->SetNodeLong(0);
    if (obj) {
        id = obj->ClassID();
        if (id == TimeSensorClassID) {
            VrmlOutTimeSensor(node, (TimeSensorObject*) obj, 0);
            mTimerList = mTimerList->AddNode(node);
        }
        Box3 bb;
        obj->GetWorldBoundBox(mStart, node, vpt, bb);
        mBoundBox += bb;
    }

    int n = node->NumberOfChildren();
    for(int i = 0; i < n; i++)
        ComputeWorldBoundBox(node->GetChildNode(i), vpt);
}

// Compute the world bounding box and a list of timesensors;
// also initialize each INode's nodeLong data
void
VRML2Export::ScanSceneGraph1()
{
    ViewExp *vpt = mIp->GetViewport(NULL);
    INode* node = mIp->GetRootNode();
    ComputeWorldBoundBox(node, vpt);
}

// Make a list of al the LOD objects and texture maps in the scene.
// Also output top-level objects
void
VRML2Export::ScanSceneGraph2()
{
    INode* node = mIp->GetRootNode();
    TraverseNode(node);
}

// Return TRUE iff the node is referenced by the LOD node.
static BOOL
ObjectIsReferenced(INode* lodNode, INode* node)
{
    Object* obj = lodNode->GetObjectRef();
    int numRefs = obj->NumRefs();

    for(int i=0; i < numRefs; i++)
        if (node == (INode*) obj->GetReference(i))
            return TRUE;

    return FALSE;
}

// Return TRUE iff the node is referenced by ANY LOD node.
BOOL 
VRML2Export::ObjectIsLODRef(INode* node)
{
    INodeList* l = mLodList;

    for(; l; l = l->GetNext())
        if (ObjectIsReferenced(l->GetNode(), node))
            return TRUE;

    return FALSE;
}


extern HINSTANCE hInstance;

static INT_PTR CALLBACK
ProgressDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
    switch (msg) {
    case WM_INITDIALOG:
        CenterWindow(hDlg, GetParent(hDlg));
        Static_SetText(GetDlgItem(hDlg, IDC_PROGRESS_NNAME), " ");
        return TRUE;
    case WM_COMMAND:
        switch(LOWORD(wParam)) {
        case IDCANCEL:
            DestroyWindow(hDlg);
            hDlg = NULL;
            return TRUE;
        case IDOK:
            DestroyWindow(hDlg);
            hDlg = NULL;
            return TRUE;
        }
        return FALSE;
    case 666:
        Static_SetText(GetDlgItem(hDlg, IDC_PROGRESS_NNAME), (TCHAR *) lParam);
        return TRUE;
    }
    return FALSE;
}

// Export the current scene as VRML
#ifdef _LEC_
int
VRML2Export::DoFBExport(const TCHAR *filename, Interface *i, VRBLExport* exp,
                        int frame, TimeValue time)
{
    mIp = i;
    mStart = time;
    mCoordInterp = FALSE;

    mGenNormals      = exp->GetGenNormals();
    mIndent          = exp->GetIndent();
    mType            = exp->GetExportType();
    mUsePrefix       = exp->GetUsePrefix();
    mUrlPrefix       = exp->GetUrlPrefix();
    mCamera          = exp->GetCamera();
    mZUp             = exp->GetZUp();
    mDigits          = exp->GetDigits();
    mFlipBook        = exp->GetFlipBook();
    mCoordSample     = exp->GetCoordSample();
    mCoordSampleRate = exp->GetCoordSampleRate();
    mNavInfo         = exp->GetNavInfo();
    mBackground      = exp->GetBackground();
    mFog             = exp->GetFog();
    mTitle           = exp->GetTitle();
    mInfo            = exp->GetInfo();
    mExportHidden    = exp->GetExportHidden();
    mPrimitives      = exp->GetPrimitives();
    mPolygonType     = exp->GetPolygonType();
    mEnableProgressBar    = exp->GetEnableProgressBar();
    mPreLight        = exp->GetPreLight();
    mCPVSource       = exp->GetCPVSource();
    mCallbacks       = exp->GetCallbacks();  //FIXME add callback support

    char buf[16];
    sprintf(buf, "%d", frame);
    CStr wName(filename);
    int extLoc;
    extLoc = wName.last('.');
    if (extLoc != -1)
        wName.remove(extLoc);
    wName.Append(buf);
    wName.Append(".wrl");

 // write out the file
    WorkFile theFile(wName.data(), _T("w"));
    mStream = theFile.MStream();
    if (!mStream) {
        TCHAR msg[MAX_PATH];
        TCHAR title[MAX_PATH];
        LoadString(hInstance, IDS_OPEN_FAILED, msg, MAX_PATH);
        LoadString(hInstance, IDS_VRML_EXPORT, title, MAX_PATH);
        MessageBox(GetActiveWindow(), msg, title, MB_OK);
        return TRUE;
    }
    HCURSOR busy = LoadCursor(NULL, IDC_WAIT);
    HCURSOR normal = LoadCursor(NULL, IDC_ARROW);
    SetCursor(busy);

 // Write out the VRML header
    fprintf(mStream, _T("#VRML V2.0 utf8\n\n"));

    VrmlOutFileInfo();

 // give third party dlls a chance to write scene
    int cnt;
    for (cnt = 0; cnt < mCallbacks->GetPreSceneCount(); cnt++) {
        DllPreScene preScene = mCallbacks->GetPreScene(cnt);
        PreSceneParam params;
        params.version = 0;
        params.fName   = mFilename;
        params.i       = mIp;
        if (mStream)
            fclose(mStream), mStream = NULL;
        if ((*(preScene))(&params)) {  //third party wrote the scene
            mStream = _tfopen(mFilename, _T("a"));
            return TRUE;
        }
    }
    if (!mStream)
        mStream = _tfopen(mFilename, _T("a"));
        
 // generate the hash table of unique node names
    GenerateUniqueNodeNames(mIp->GetRootNode());

    if (mEnableProgressBar) {
        RECT rcClient;  // client area of parent window 
        int cyVScroll;  // height of a scroll bar arrow 
        hWndPDlg = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_PROGRESSDLG),
                        GetActiveWindow(), ProgressDlgProc);
        GetClientRect(hWndPDlg, &rcClient); 
        cyVScroll = GetSystemMetrics(SM_CYVSCROLL); 
        ShowWindow(hWndPDlg, SW_SHOW);
        hWndPB = CreateWindow(PROGRESS_CLASS, (LPSTR) NULL, 
                    WS_CHILD | WS_VISIBLE, rcClient.left, 
                    rcClient.bottom - cyVScroll, 
                    rcClient.right, cyVScroll, 
                    hWndPDlg, (HMENU) 0, hInstance, NULL); 
     // Set the range and increment of the progress bar. 
        SendMessage(hWndPB, PBM_SETRANGE, 0, MAKELPARAM(0,
            CountNodes(mIp->GetRootNode()) + 1));
        SendMessage(hWndPB, PBM_SETSTEP, (WPARAM) 1, 0); 
    }

 // Write out the scene graph
    VrmlOutNode(mIp->GetRootNode(), NULL, -2, FALSE, TRUE, FALSE);

    WriteAnimRoutes();
    delete mLodList;
    delete mTimerList;
    SetCursor(normal);
    if (hWndPB) {
        DestroyWindow(hWndPB);
        hWndPB = NULL;
    }
    if (hWndPDlg) {
        DestroyWindow(hWndPDlg);
        hWndPDlg = NULL;
    }

    if(theFile.Close())
        return 0;
    
    return 1;
}
#endif

// Export the current scene as VRML
int
VRML2Export::DoExport(const TCHAR* filename, Interface* i, VRBLExport* exp)
{
    mIp = i;
    mStart = mIp->GetAnimRange().Start();

    mGenNormals      = exp->GetGenNormals();
    mIndent          = exp->GetIndent();
    mType            = exp->GetExportType();
    mUsePrefix       = exp->GetUsePrefix();
    mUrlPrefix       = exp->GetUrlPrefix();
    mCamera          = exp->GetCamera();
    mZUp             = exp->GetZUp();
    mDigits          = exp->GetDigits();
    mCoordInterp     = exp->GetCoordInterp();
    mTformSample     = exp->GetTformSample();
    mTformSampleRate = exp->GetTformSampleRate();
    mCoordSample     = exp->GetCoordSample();
    mCoordSampleRate = exp->GetCoordSampleRate();
    mNavInfo         = exp->GetNavInfo();
    mBackground      = exp->GetBackground();
    mFog             = exp->GetFog();
    mTitle           = exp->GetTitle();
    mInfo            = exp->GetInfo();
    mExportHidden    = exp->GetExportHidden();
    mPrimitives      = exp->GetPrimitives();
    mPolygonType     = exp->GetPolygonType();
    mEnableProgressBar    = exp->GetEnableProgressBar();
    mPreLight        = exp->GetPreLight();
    mCPVSource       = exp->GetCPVSource();
	mCallbacks       = exp->GetCallbacks();
    mFilename        = (TCHAR*) filename;
    
    WorkFile theFile(mFilename, _T("w"));
    mStream = theFile.MStream();

    if (!mStream) {
        TCHAR msg[MAX_PATH];
        TCHAR title[MAX_PATH];
        LoadString(hInstance, IDS_OPEN_FAILED, msg, MAX_PATH);
        LoadString(hInstance, IDS_VRML_EXPORT, title, MAX_PATH);
        MessageBox(GetActiveWindow(), msg, title, MB_OK);
        return TRUE;
    }

    HCURSOR busy = LoadCursor(NULL, IDC_WAIT);
    HCURSOR normal = LoadCursor(NULL, IDC_ARROW);
    SetCursor(busy);

 // Write out the VRML header and file info
    fprintf(mStream, _T("#VRML V2.0 utf8\n\n"));
    VrmlOutFileInfo();

 // generate the hash table of unique node names
    GenerateUniqueNodeNames(mIp->GetRootNode());

    if (mEnableProgressBar) {
        RECT rcClient;  // client area of parent window 
        int cyVScroll;  // height of a scroll bar arrow 
        hWndPDlg = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_PROGRESSDLG),
                    GetActiveWindow(), ProgressDlgProc);
        GetClientRect(hWndPDlg, &rcClient); 
        cyVScroll = GetSystemMetrics(SM_CYVSCROLL); 
        ShowWindow(hWndPDlg, SW_SHOW);
     // InitCommonControls(); 
        hWndPB = CreateWindow(PROGRESS_CLASS, (LPSTR) NULL, 
            WS_CHILD | WS_VISIBLE, rcClient.left, 
            rcClient.bottom - cyVScroll, 
            rcClient.right, cyVScroll, 
            hWndPDlg, (HMENU) 0, hInstance, NULL); 
    // Set the range and increment of the progress bar. 
        SendMessage(hWndPB, PBM_SETRANGE, 0, MAKELPARAM(0,
            CountNodes(mIp->GetRootNode()) + 1));
        SendMessage(hWndPB, PBM_SETSTEP, (WPARAM) 1, 0); 
    }
 
 // give third party dlls a chance to write before the scene was written
    BOOL written = FALSE;
    int cnt;
    for (cnt = 0; cnt < mCallbacks->GetPreSceneCount(); cnt++) {
        DllPreScene preScene = mCallbacks->GetPreScene(cnt);
        PreSceneParam params;
        params.version = 0;
        params.fName   = mFilename;
        params.i       = mIp;
        if (mStream)
            fclose(mStream), mStream = NULL;
        written = (*(preScene))(&params);   //third party wrote the scene
        if (written)
            break; // first come first served
    }
    if (!mStream)
        mStream = _tfopen(mFilename, _T("a"));
     
 // Write out the scene graph
    if (!written) {
        VrmlOutNode(mIp->GetRootNode(), NULL, -2, FALSE, TRUE, FALSE);
        WriteAnimRoutes();
        delete mLodList;
        delete mTimerList;
    }

 // give third party dlls a chance to write after the scene was written
    for (cnt = 0; cnt < mCallbacks->GetPostSceneCount(); cnt++) {
        DllPostScene postScene = mCallbacks->GetPostScene(cnt);
        PostSceneParam params;
        params.version = 0;
        params.fName   = mFilename;
        params.i       = mIp;
        if (mStream)
            fclose(mStream), mStream = NULL;
        (*(postScene))(&params);
    }
    if (!mStream)
        mStream = _tfopen(mFilename, _T("a"));
    
    SetCursor(normal);
    if (hWndPB) {
        DestroyWindow(hWndPB);
        hWndPB = NULL;
    }
    if (hWndPDlg) {
        DestroyWindow(hWndPDlg);
        hWndPDlg = NULL;
    }

    if(theFile.Close())
        return 0;

    return 1;
}


VRML2Export::VRML2Export() 
{
    mGenNormals         = FALSE;
    mHadAnim            = FALSE;
    mLodList            = NULL;
    mTimerList          = NULL;
    mTformSample        = TRUE;
    mTformSampleRate    = 10;
    mCoordSample        = FALSE;
    mCoordSampleRate    = 3;
    mHasLights          = FALSE;
    mHasNavInfo         = FALSE;
    mFlipBook           = FALSE;
}

VRML2Export::~VRML2Export() {
}

// Traverse the scene graph generating Unique Node Names
void 
VRML2Export::GenerateUniqueNodeNames(INode* node)
{
	if (!node) return;

	NodeList* nList = mNodes.AddNode(node);
	if (!nList->hasName) {
	 // take mangled name and get a unique name
		nList->name    = mNodes.AddName(VRMLName(node->GetName()));
		nList->hasName = TRUE;
	}
    
    int n = node->NumberOfChildren();
    for(int i = 0; i < n; i++)
        GenerateUniqueNodeNames(node->GetChildNode(i));

}

static DWORD HashCode(DWORD o, int size)
{
    DWORD code = (DWORD) o;
    return (code >> 2) % size;
}

// Object Hash table stuff

ObjectBucket*
ObjectHashTable::AddObject(Object* o)
{
    DWORD hashCode = HashCode((DWORD) o, OBJECT_HASH_TABLE_SIZE);
    ObjectBucket *ob;
    
    for(ob = mTable[hashCode]; ob; ob = ob->next) {
        if (ob->obj == o) {
            return ob;
        }
    }
    ob = new ObjectBucket(o);
    ob->next = mTable[hashCode];
    mTable[hashCode] = ob;
    return ob;
}

void 
SensorHashTable::AddSensor(INode* node, INode* sensor)
{
    DWORD hashCode = HashCode((DWORD) node, SENSOR_HASH_TABLE_SIZE);
    SensorBucket *sb;
    
    for(sb = mTable[hashCode]; sb; sb = sb->mNext) {
        if (sb->mNode == node) {
            sb->mSensors = sb->mSensors->AddNode(sensor);
            return;
        }
    }
    sb = new SensorBucket(node);
    sb->mSensors = sb->mSensors->AddNode(sensor);
    sb->mNext = mTable[hashCode];
    mTable[hashCode] = sb;
}


SensorBucket* 
SensorHashTable::FindSensor(INode* node)
{
    DWORD hashCode = HashCode((DWORD) node, SENSOR_HASH_TABLE_SIZE);
    SensorBucket *sb;
    
    for(sb = mTable[hashCode]; sb; sb = sb->mNext) {
        if (sb->mNode == node) {
            return sb;
        }
    }
    return NULL;
}

// Output any grid helpers
/*
void
VRML2Export::VrmlOutGridHelpers(INode* node)
{
    if (!node) return;

    Object*  obj = node->EvalWorldState(mStart).obj;
    Class_ID id;

    if (obj) {
        id = obj->ClassID();
        if (id == Class_ID(GRIDHELP_CLASS_ID, 0)) {
            float len;
            float width;
            float grid;
            Matrix3 tm;

            len   = ((GridHelpObject*)obj)->GetLength(mStart);
            width = ((GridHelpObject*)obj)->GetWidth(mStart);
            grid  = ((GridHelpObject*)obj)->GetGrid(mStart);
            // tm    = ((GridHelpObject*)obj)->myTM; //private member
            
            int pCnt;
            pCnt = obj->NumPoints();
            for (int i = 0; i < pCnt; i++) {
                Point3 pt;
                pt = obj->GetPoint(i);
            }
        }
    }

    int n = node->NumberOfChildren();
    for (int i = 0; i < n; i++) {
        VrmlOutGridHelpers(node->GetChildNode(i));
    }
}
*/