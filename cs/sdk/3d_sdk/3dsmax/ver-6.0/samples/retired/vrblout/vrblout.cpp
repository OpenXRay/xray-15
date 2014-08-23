/**********************************************************************
 *<
	FILE: vrblout.cpp

	DESCRIPTION:  VRML/VRBL .WRL file export module

	CREATED BY: Scott Morrison

	HISTORY: created 15 February, 1996

 *>	Copyright (c) 1996, 1997 All Rights Reserved.
 **********************************************************************/

#include <time.h>
#include "Max.h"
#include "simpobj.h"
#include "istdplug.h"
#include "mr_blue.h"
#include "vrml_ins.h"
#include "lod.h"
#include "inlist.h"
#include "resource.h"
#include "notetrck.h"
#include "bookmark.h"
#include "stdmat.h"
#include "normtab.h"
#include "vrblout.h"
#include "decomp.h"
#include "appd.h"
#include "iparamm.h"
#include "navinfo.h"
#include "backgrnd.h"
#include "fog.h"
#include "helpsys.h"

//#define FUNNY_TEST

extern TCHAR *GetString(int id);

static HWND hWndPDlg;   // handle of the progress dialog
static HWND hWndPB;     // handle of progress bar 

static Matrix3 identMat(TRUE);

inline BOOL
ApproxEqual(float a, float b, float eps)
{
    float d = (float) fabs(a-b);
    return d < eps;
}

void
CommaScan(TCHAR* buf)
{
    for(; *buf; buf++) if (*buf == ',') *buf = '.';
}

// Returns TRUE if an object or one of its ancestors in animated
static BOOL IsEverAnimated(INode* node);

// Round numbers near zero to zero.  This help reduce VRML file size.
inline float
round(float f)
{
    // This is used often, so we avoid calling fabs
    if (f < 0.0f) {
        if (f > -1.0e-5)
            return 0.0f;
        return f;
    }
    if (f < 1.0e-5)
        return 0.0f;
    return f;
}

// Function for writing values into a string for output.  
// These functions take care of rounding values near zero, and flipping
// Y and Z for VRML output.

// Format a 3D coordinate value.
TCHAR*
VRBLExport::point(Point3& p)
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
VRBLExport::color(Color& c)
{
    static TCHAR buf[50];
    TCHAR format[20];
    sprintf(format, _T("%%.%dg %%.%dg %%.%dg"), mDigits, mDigits, mDigits);
    sprintf(buf, format, round(c.r), round(c.g), round(c.b));
    CommaScan(buf);
    return buf;
}

TCHAR*
VRBLExport::color(Point3& c)
{
    static TCHAR buf[50];
    TCHAR format[20];
    sprintf(format, _T("%%.%dg %%.%dg %%.%dg"), mDigits, mDigits, mDigits);
    sprintf(buf, format, round(c.x), round(c.y), round(c.z));
    CommaScan(buf);
    return buf;
}


TCHAR*
VRBLExport::floatVal(float f)
{
    static TCHAR buf[50];
    TCHAR format[20];
    sprintf(format, _T("%%.%dg"), mDigits);
    sprintf(buf, format, round(f));
    CommaScan(buf);
    return buf;
}


TCHAR*
VRBLExport::texture(UVVert& uv)
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
VRBLExport::scalePoint(Point3& p)
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
VRBLExport::normPoint(Point3& p)
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
VRBLExport::axisPoint(Point3& p, float angle)
{
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

// Get the tranform matrix that take a point from its local coordinate
// system to it's parent's coordinate system
static Matrix3
GetLocalTM(INode* node, TimeValue t)
{
    Matrix3 tm;
    tm = node->GetObjTMAfterWSM(t);
    if (!node->GetParentNode()->IsRootNode()) {
        Matrix3 ip = Inverse(node->GetParentNode()->GetObjTMAfterWSM(t));
        tm = tm * ip;
    }
    return tm;
}

class VRBLClassDesc:public ClassDesc {
public:
    int    IsPublic()                   { return TRUE; }
    void * Create(BOOL loading = FALSE) { return new VRBLExport; }
    const TCHAR *ClassName()            { return _T("VRBL Export"); }
    SClass_ID	 SuperClassID()         { return SCENE_EXPORT_CLASS_ID; }
    Class_ID	 ClassID()    { return Class_ID(VRBL_EXPORT_CLASS_ID,0); }
    const TCHAR* Category()   { return GetString(IDS_TH_SCENEEXPORT); }
};

static VRBLClassDesc VRBLDesc;

ClassDesc* GetVRBLDesc() { return &VRBLDesc; }

////////////////////////////////////////////////////////////////////////
// VRBL Export implementation
////////////////////////////////////////////////////////////////////////

// Indent to the given level.
void 
VRBLExport::Indent(int level)
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
static TCHAR * VRMLName(TCHAR *name)
{
    static char buffer[256];
    TCHAR* cPtr;
    int firstCharacter = 1;

    _tcscpy(buffer, name);
    cPtr = buffer;
    while(*cPtr) {
        if( *cPtr <= CTL_CHARS ||
            *cPtr == ' ' ||
            *cPtr == '\''||
            *cPtr == '"' ||
            *cPtr == '\\'||
            *cPtr == '{' ||
            *cPtr == '}' ||
            *cPtr == ',' ||            
            *cPtr == '.' ||
            *cPtr == '[' ||
            *cPtr == ']' ||
            *cPtr == '.' ||
            *cPtr == '#' ||
            *cPtr >= 127 ||
            (firstCharacter && (*cPtr >= '0' && *cPtr <= '9'))) *cPtr = '_';
        firstCharacter = 0;
        cPtr++;
    }
    
    return buffer;
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
    }
    return 0;
}

// Return true if it has a mirror transform
BOOL
VRBLExport::IsMirror(INode* node)
{
    Matrix3 tm = GetLocalTM(node, mStart);
    AffineParts parts;
    decomp_affine(tm, &parts);

    return parts.f < 0.0f;
}

// Write beginning of the Separator node.
void
VRBLExport::StartNode(INode* node, Object* obj, int level, BOOL outputName,
                      TCHAR *pname)
{
    if (node->IsRootNode()) {
        fprintf(mStream, _T("Separator {\n"));
        return;
    }
    Indent(level);
    if (obj->SuperClassID() == CAMERA_CLASS_ID ||
        obj->SuperClassID() == LIGHT_CLASS_ID  || !outputName) {
     // Lights and cameras need different top-level names for triggers
        fprintf(mStream, _T("DEF %s_TopLevel Separator {\n"),
                         mNodes.GetNodeName(node));
    } else {
        if (pname != NULL)
            fprintf(mStream, _T("DEF %s Separator {\n"), pname);
        else
            fprintf(mStream, _T("DEF %s Separator {\n"),
                             mNodes.GetNodeName(node));
        if (IsMirror(node)) {
            Indent(level+1);
            fprintf(mStream, _T("ShapeHints {\n"));
            Indent(level+2);
            fprintf(mStream, _T("vertexOrdering CLOCKWISE\n"));
            Indent(level+1);
            fprintf(mStream, _T("}\n"));
        }
    }

    if (!obj)
        return;

    // Put note tracks as info nodes
    int numNotes = node->NumNoteTracks();
    for(int i=0; i < numNotes; i++) {
        DefNoteTrack *nt = (DefNoteTrack*) node->GetNoteTrack(i);
        for (int j = 0; j < nt->keys.Count(); j++) {
            NoteKey* nk = nt->keys[j];
            TSTR note = nk->note;
            if (note.Length() > 0) {
                Indent(level+1);
                fprintf(mStream, _T("Info { string \"frame %d: %s\" }\n"),
                        nk->time/GetTicksPerFrame(), note.data());
            }
        }
    }
}

// Write end of the separator node.
void
VRBLExport::EndNode(INode *node, int level, BOOL lastChild)
{
    Indent(level);
    fprintf(mStream, _T("}\n"));
}

// Returns TRUE iff the node is a bounding box trigger
BOOL
VRBLExport::IsBBoxTrigger(INode* node)
{
    Object * obj = node->EvalWorldState(mStart).obj;
    if (obj->ClassID() != Class_ID(MR_BLUE_CLASS_ID1, MR_BLUE_CLASS_ID2))
        return FALSE;
    MrBlueObject* mbo = (MrBlueObject*) obj;
    return mbo->GetBBoxEnabled();
}

// Write out the transform from the local coordinate system to the
// parent coordinate system
void
VRBLExport::OutputNodeTransform(INode* node, int level, Matrix3 &pendingTM)
{
    // Root node is always identity
    if (node->IsRootNode())
        return;

    Matrix3 tm = GetLocalTM(node, mStart) * pendingTM;
    int i, j;
    Point3 p;

    // Handle BBOX triggers differently (no rotations)
    if (IsBBoxTrigger(node)) {
        p = tm.GetTrans();
        Indent(level);
        fprintf(mStream, _T("Translation { translation %s }\n"),
                point(p));
        return;
    }

    // Check for scale and rotation part of matrix being identity.
    BOOL isIdentity = TRUE;
    for (i=0; i<3; i++) {
        for (j=0; j<3; j++) {
            if (i==j) {
                if (tm.GetRow(i)[j] != 1.0f) {
                    isIdentity = FALSE;
                    goto done;
                }
            } else if (fabs(tm.GetRow(i)[j]) > 1.0e-05) {
                isIdentity = FALSE;
                goto done;
            }
        }
    }

  done:
    if (isIdentity) {
        p = tm.GetTrans();
        Indent(level);
        fprintf(mStream, _T("Translation { translation %s }\n"), point(p));
    } else {
        // If not identity, decompose matrix into scale, rotation and
        // translation components.
        Point3 s, axis;
        Quat q;
        float ang;

        AffineParts parts;
        decomp_affine(tm, &parts);
        p = parts.t;
        q = parts.q;
        AngAxisFromQ(q, &ang, axis);
        Indent(level);
        fprintf(mStream, _T("Translation { translation %s }\n"), point(p));
        if (ang != 0.0f && ang != -0.0f) {
            Indent(level);
            // VRML angle convention is opposite of MAX convention,
            // so we negate the angle.
            fprintf(mStream, _T("Rotation { rotation %s }\n"),
                    axisPoint(axis, -ang));
        }
        ScaleValue sv(parts.k, parts.u);
        s = sv.s;
        if (parts.f < 0.0f)
            s = - s;
        if (s.x != 1.0f || s.y != 1.0f || s.z != 1.0f) {
            Indent(level);
            fprintf(mStream, _T("Scale { scaleFactor %s }\n"), scalePoint(s));
        }
    }
}

// Initialize the normal table
NormalTable::NormalTable()
{
    tab.SetCount(NORM_TABLE_SIZE);
    for(int i = 0; i < NORM_TABLE_SIZE; i++)
        tab[i] = NULL;
}

NormalTable::~NormalTable()
{
    // Delete the buckets in the normal table hash table.
    for(int i = 0; i < NORM_TABLE_SIZE; i++)
        delete tab[i];
}

// Add a normal to the hash table
void
NormalTable::AddNormal(Point3& norm)
{
    // Truncate normals to a value that brings close normals into
    // the same bucket.
    Point3 n = NormalizeNorm(norm);
    DWORD code = HashCode(n);
    NormalDesc* nd;
    for(nd = tab[code]; nd; nd = nd->next) {
        if (nd->n == n)  // Equality OK because of normalization procedure.
            return;
    }
    NormalDesc* newNorm = new NormalDesc(norm);
    newNorm->next = tab[code];
    tab[code] = newNorm;
}

// Get the index of a normal in the IndexedFaceSet
int
NormalTable::GetIndex(Point3& norm)
{
    Point3 n = NormalizeNorm(norm);
    DWORD code = HashCode(n);
    NormalDesc* nd;
    for(nd = tab[code]; nd; nd = nd->next) {
        if (nd->n == n)
            return nd->index;
    }
    return -1;
}


// Produce a hash code for a normal
DWORD
NormalTable::HashCode(Point3& norm)
{
    union {
        float p[3];
        DWORD i[3];
    } u;
    u.p[0] = norm.x; u.p[1] = norm.y; u.p[2] = norm.z;
    return ((u.i[0] >> 8) + (u.i[1] >> 16) + u.i[2]) % NORM_TABLE_SIZE;
}

// Print the hash table statistics for the normal table
void
NormalTable::PrintStats(FILE* mStream)
{
    int slots = 0;
    int buckets = 0;
    int i;
    NormalDesc* nd;

    for(i = 0; i < NORM_TABLE_SIZE; i++) {
        if (tab[i]) {
            slots++;
            for(nd = tab[i]; nd; nd = nd->next)
                buckets++;
        }
    }
    fprintf(mStream,_T("# slots = %d, buckets = %d, avg. chain length = %.5g\n"),
            slots, buckets, ((double) buckets / (double) slots));
            
}

// Returns true IFF the mesh is all in the same smoothing group
static BOOL
MeshIsAllOneSmoothingGroup(Mesh& mesh)
{
    int numfaces = mesh.getNumFaces();
    unsigned int sg;
    int i;

    for(i = 0; i < numfaces; i++) {
        if (i == 0) {
            sg = mesh.faces[i].getSmGroup();
            if (sg == 0)        // Smoothing group of 0 means faceted
                return FALSE;
        }
        else {
            if (sg != mesh.faces[i].getSmGroup())
                return FALSE;
        }
    }
    return TRUE;
}

// Write out the indices of the normals for the IndexedFaceSet
void
VRBLExport::OutputNormalIndices(Mesh& mesh, NormalTable* normTab, int level)
{
    Point3 n;
    int numfaces = mesh.getNumFaces();
    int i, j, v, norCnt;

    Indent(level);
    
    fprintf(mStream, _T("normalIndex [\n"));
    for (i = 0; i < numfaces; i++) {
        int smGroup = mesh.faces[i].getSmGroup();
        Indent(level+1);
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
            fprintf(mStream, _T("%d, "), index);
        }
        fprintf(mStream, _T("-1,\n"));
    }
    Indent(level);
    fprintf(mStream, _T("]\n"));
}

// Create the hash table of normals for the given mesh, and
// write out the normal values.
NormalTable*
VRBLExport::OutputNormals(Mesh& mesh, int level)
{
    int i, j, norCnt;
    int numverts = mesh.getNumVerts();
    int numfaces = mesh.getNumFaces();
    NormalTable* normTab;


    mesh.buildRenderNormals();

    if (MeshIsAllOneSmoothingGroup(mesh)) {
        // No need for normals when whole object is smooth.
        // VRML Browsers compute normals automatically in this case.
        return NULL;
    }

    normTab = new NormalTable();

    // Otherwise we have several smoothing groups
    for(int index = 0; index < numfaces; index++) {
        int smGroup = mesh.faces[index].getSmGroup();
        for(i = 0; i < 3; i++) {
            // Now get the normal for each vertex of the face
            // Given the smoothing group
            int cv = mesh.faces[index].v[i];
            RVertex * rv = mesh.getRVertPtr(cv);
            if (rv->rFlags & SPECIFIED_NORMAL) {
                normTab->AddNormal(rv->rn.getNormal());
            }
            else if((norCnt = (int)(rv->rFlags & NORCT_MASK)) && smGroup) {
                if (norCnt == 1)        // 1 normal, stored in rn
                    normTab->AddNormal(rv->rn.getNormal());
                else for(j = 0; j < norCnt; j++) {
                    // More than one normal, stored in ern.
                    normTab->AddNormal(rv->ern[j].getNormal());
                }
            } else
                normTab->AddNormal(mesh.getFaceNormal(index));
        }
    }

    // Now write out the table
    index = 0;
    NormalDesc* nd;
    Indent(level);
    fprintf(mStream, _T("Normal { vector [\n"));
       
    for(i = 0; i < NORM_TABLE_SIZE; i++) {
        for(nd = normTab->Get(i); nd; nd = nd->next) {
            nd->index = index++;
            Indent(level+1);
            Point3 p = nd->n/NUM_NORMS;
            fprintf(mStream, _T("%s,\n"), normPoint(p));
        }
    }
    Indent(level);
    fprintf(mStream, _T("] }\n"));

    Indent(level);
    fprintf(mStream, _T("NormalBinding { value PER_VERTEX_INDEXED }\n"));
    
#ifdef DEBUG_NORM_HASH
    normTab->PrintStats(mStream);
#endif

    return normTab;
}

// Write out the data for a single triangle mesh
void
VRBLExport::OutputTriObject(INode* node, TriObject* obj, BOOL isMulti,
                            BOOL twoSided, int textureNum, int level,
                            int morphing)
{
    assert(obj);
    Mesh &mesh = obj->GetMesh();
    int numverts = mesh.getNumVerts();
    int numtverts = mesh.getNumTVerts();
    int numfaces = mesh.getNumFaces();
    int i;
    NormalTable* normTab = NULL;
    TextureDesc* td;
    
    Mtl *mtl = node->GetMtl();
    if (mtl && mtl->IsMultiMtl() && textureNum != -1) {
        mtl = mtl->GetSubMtl(textureNum);
        td = GetMtlTex(mtl);
    } else
        td = GetMatTex(node);

    if (numfaces == 0) {
        delete td;
        return;
    }

   if (isMulti) {
        Indent(level);
        fprintf(mStream, _T("MaterialBinding { value PER_FACE_INDEXED }\n"));
    }

    // Output the vertices
    Indent(level);
    fprintf(mStream, _T("Coordinate3 { point [\n"));
        
    for(i = 0; i < numverts; i++) {
        Point3 p = mesh.verts[i];
        Indent(level+1);
        fprintf(mStream, _T("%s"), point(p));
        if (i == numverts-1) {
            fprintf(mStream, _T("]\n"));
            Indent(level);
            fprintf(mStream, _T("}\n"));
        }
        else
            fprintf(mStream, _T(",\n"));
    }

    // Output the normals
    if (mGenNormals) {
        normTab = OutputNormals(mesh, level);
    }

    // Output Texture coordinates
    if (numtverts > 0 && td) {
        Indent(level);
        if (morphing > 0)
            fprintf(mStream, _T("USE %s_TEXCO\n"), mNodes.GetNodeName(node));
        else {
            if (morphing < 0)
                fprintf(mStream, _T("DEF %s_TEXCO "),
                                        mNodes.GetNodeName(node));
            fprintf(mStream, _T("TextureCoordinate2 { point [\n"));

            for(i = 0; i < numtverts; i++) {
                UVVert p = mesh.getTVert(i);
                Indent(level+1);
                fprintf(mStream, _T("%s"), texture(p));
                if (i == numtverts-1) {
                    fprintf(mStream, _T("]\n"));
                    Indent(level);
                    fprintf(mStream, _T("}\n"));
                }
                else
                    fprintf(mStream, _T(",\n"));
            }
        }
    }

    if (twoSided) {
        Indent(level);
        fprintf(mStream, _T("ShapeHints {\n"));
        Indent(level+1);
        fprintf(mStream, _T("shapeType UNKNOWN_SHAPE_TYPE\n"));
        Indent(level);
        fprintf(mStream, _T("}\n"));
    }
    // Output the triangles
    Indent(level);
    if (morphing > 0)
        fprintf(mStream, _T("USE %s_FACES\n"), mNodes.GetNodeName(node));
    else {
        if (morphing < 0)
            fprintf(mStream, _T("DEF %s_FACES "), mNodes.GetNodeName(node));
        fprintf(mStream, _T("IndexedFaceSet { coordIndex [\n"));
        for(i = 0; i < numfaces; i++) {
        int id = mesh.faces[i].getMatID();
            if (textureNum == -1 || id == textureNum) {
                if (!(mesh.faces[i].flags & FACE_HIDDEN)) {
                    Indent(level+1);
                    fprintf(mStream, _T("%d, %d, %d, -1"), mesh.faces[i].v[0],
                            mesh.faces[i].v[1], mesh.faces[i].v[2]);
                    if (i != numfaces-1)
                        fprintf(mStream, _T(", \n"));
                }
            }
        }
        fprintf(mStream, _T("]\n"));

        // Output the texture coordinate indices
        if (numtverts > 0 && td) {
            Indent(level);
            fprintf(mStream, _T("textureCoordIndex [\n"));
            for(i = 0; i < numfaces; i++) {
                int id = mesh.faces[i].getMatID();
                if (textureNum == -1 || id == textureNum) {
                    if (!(mesh.faces[i].flags & FACE_HIDDEN)) {
                        Indent(level+1);
                        fprintf(mStream, _T("%d, %d, %d, -1"), mesh.tvFace[i].t[0],
                                mesh.tvFace[i].t[1], mesh.tvFace[i].t[2]);
                    if (i != numfaces-1)
                        fprintf(mStream, _T(", \n"));
                    }
                }
            }
            fprintf(mStream, _T("]\n"));
        }

        // Output the material indices
        if (isMulti) {
            Indent(level);
            fprintf(mStream, _T("materialIndex [\n"));
            for(i = 0; i < numfaces; i++) {
                if (!(mesh.faces[i].flags & FACE_HIDDEN)) {
                    Indent(level+1);
                    fprintf(mStream, _T("%d"), mesh.faces[i].getMatID());
                    if (i != numfaces-1)
                        fprintf(mStream, _T(", \n"));
                }
            }
            fprintf(mStream, _T("]\n"));
        }

        // Output the normal indices
        if (mGenNormals && normTab) {
            OutputNormalIndices(mesh, normTab, level);
            delete normTab;
        }
        
        Indent(level);
        fprintf(mStream, _T("}\n"));
    }
    delete td;
}

// Returns TRUE iff the node has an attached standard material with
// a texture map on the diffuse color
BOOL
VRBLExport::HasTexture(INode* node)
{
    TextureDesc* td = GetMatTex(node);
    if (!td)
        return FALSE;
    delete td;
    return TRUE;
}

// Get the name of the texture file of the texure on the diffuse
// color of the material attached to the given node.
TextureDesc*
VRBLExport::GetMatTex(INode* node)
{
    Mtl* mtl = node->GetMtl();
    return GetMtlTex(mtl);
}


TextureDesc*
VRBLExport::GetMtlTex(Mtl* mtl)
{
    if (!mtl)
        return NULL;

    // We only handle standard materials.
    if (mtl->ClassID() != Class_ID(DMTL_CLASS_ID, 0))
        return NULL;

    StdMat* sm = (StdMat*) mtl;
    // Check for texture map
    Texmap* tm = (BitmapTex*) sm->GetSubTexmap(ID_DI);
    if (!tm)
        return NULL;

    // We only handle bitmap textures in VRML
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

    // Split the name up
    TSTR path;
    SplitPathFile(bitmapFile, &path, &fileName);

    TSTR url;
    if (mUsePrefix && mUrlPrefix.Length() > 0) {
        if (mUrlPrefix[mUrlPrefix.Length() - 1] != '/') {
            TSTR slash = "/";
            url = mUrlPrefix + slash + fileName;
        } else
            url = mUrlPrefix + fileName;
    }
    else
        url = fileName;
    TextureDesc* td = new TextureDesc(bm, fileName, url);
    return td;
}

// Write out the colors for a multi/sub-object material
void
VRBLExport::OutputMultiMtl(Mtl* mtl, int level)
{
    int i;
    Mtl* sub;
    Color c;
    float f;

    Indent(level);
    fprintf(mStream, _T("Material {\n"));
    int num = mtl->NumSubMtls();

    Indent(level+1);
    fprintf(mStream, _T("ambientColor [ "));
    for(i = 0; i < num; i++) {
        sub = mtl->GetSubMtl(i);
        // Some slots might be empty!
        if (!sub)
            continue;
        c = sub->GetAmbient(mStart);
        if (i == num - 1)
            fprintf(mStream, _T("%s "), color(c));
        else
            fprintf(mStream, _T("%s, "), color(c));
    }
    fprintf(mStream, _T("]\n"));
    Indent(level+1);
    fprintf(mStream, _T("diffuseColor [ "));
    for(i = 0; i < num; i++) {
        sub = mtl->GetSubMtl(i);
        if (!sub)
            continue;
        c = sub->GetDiffuse(mStart);
        if (i == num - 1)
            fprintf(mStream, _T("%s "), color(c));
        else
            fprintf(mStream, _T("%s, "), color(c));
    }
    fprintf(mStream, _T("]\n"));
        
    Indent(level+1);
    fprintf(mStream, _T("specularColor [ "));
    for(i = 0; i < num; i++) {
        sub = mtl->GetSubMtl(i);
        if (!sub)
            continue;
        c = sub->GetSpecular(mStart);
        if (i == num - 1)
            fprintf(mStream, _T("%s "), color(c));
        else
            fprintf(mStream, _T("%s, "), color(c));
    }
    fprintf(mStream, _T("]\n"));
    
    Indent(level+1);
    fprintf(mStream, _T("shininess [ "));
    for(i = 0; i < num; i++) {
        sub = mtl->GetSubMtl(i);
        if (!sub)
            continue;
        f = sub->GetShininess(mStart);
        if (i == num - 1)
            fprintf(mStream, _T("%s "), floatVal(f));
        else
            fprintf(mStream, _T("%s, "), floatVal(f));
    }
    fprintf(mStream, _T("]\n"));
        
    Indent(level+1);
    fprintf(mStream, _T("emissiveColor [ "));
    for(i = 0; i < num; i++) {
        sub = mtl->GetSubMtl(i);
        if (!sub)
            continue;
        c = sub->GetDiffuse(mStart);
        float si;
        if (sub->ClassID() == Class_ID(DMTL_CLASS_ID, 0)) {
            StdMat* stdMtl = (StdMat *) sub;
            si = stdMtl->GetSelfIllum(mStart);
        }
        else
            si = 0.0f;
        Point3 p = si * Point3(c.r, c.g, c.b);
        if (i == num - 1)
            fprintf(mStream, _T("%s "), color(p));
        else
            fprintf(mStream, _T("%s, "), color(p));
    }
    fprintf(mStream, _T("]\n"));
        
    Indent(level);
    fprintf(mStream, _T("}\n"));
}

void
VRBLExport::OutputNoTexture(int level)
{
    Indent(level);
    fprintf(mStream, _T("Texture2 {}\n"));
}

// Output the material definition for a node.
BOOL
VRBLExport::OutputMaterial(INode* node, BOOL& twoSided, int level,
                           int textureNum)
{
    Mtl* mtl = node->GetMtl();
    BOOL isMulti = FALSE;
    twoSided = FALSE;

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
                 !mtl->IsMultiMtl())) {
        Color col(node->GetWireColor());
        Indent(level);
        fprintf(mStream, _T("Material {\n"));
        Indent(level+1);
        fprintf(mStream, _T("diffuseColor %s\n"), color(col));
//        Indent(level+1);
//        fprintf(mStream, _T("specularColor .9 .9 .9\n"));
        Indent(level);
        fprintf(mStream, _T("}\n"));
        OutputNoTexture(level);
        return FALSE;
    }

/*    if (mtl->IsMultiMtl()) {
        OutputMultiMtl(mtl, level);
        OutputNoTexture(level);
        return TRUE;
    }
*/

    if (mtl->ClassID() == Class_ID(DMTL_CLASS_ID, 0) ||
        mtl->ClassID() == Class_ID(DMTL2_CLASS_ID, 0)) {
        StdMat* sm = (StdMat*) mtl;
        twoSided = sm->GetTwoSided();
    } else
        twoSided = FALSE;

    Interval i = FOREVER;
//    sm->Update(0, i);
    mtl->Update(0, i);
    Indent(level);
    fprintf(mStream, _T("Material {\n"));
    Color c;

    Indent(level+1);
//    c = sm->GetAmbient(mStart);
    c = mtl->GetAmbient(mStart);
    fprintf(mStream, _T("ambientColor %s\n"), color(c));
    Indent(level+1);
//    c = sm->GetDiffuse(mStart);
    c = mtl->GetDiffuse(mStart);
    fprintf(mStream, _T("diffuseColor %s\n"), color(c));
    Indent(level+1);
//    c = sm->GetSpecular(mStart);
    c = mtl->GetSpecular(mStart);
    fprintf(mStream, _T("specularColor %s\n"), color(c));
    Indent(level+1);
    fprintf(mStream, _T("shininess %s\n"),
//            floatVal(sm->GetShininess(mStart)));
            floatVal(mtl->GetShininess(mStart)));
    Indent(level+1);
    fprintf(mStream, _T("transparency %s\n"),
//            floatVal(1.0f - sm->GetOpacity(mStart)));
            floatVal(mtl->GetXParency(mStart)));
//    float si = sm->GetSelfIllum(mStart);
    float si = mtl->GetSelfIllum(mStart);
    if (si > 0.0f) {
        Indent(level+1);
//        c = sm->GetDiffuse(mStart);
        c = mtl->GetDiffuse(mStart);
        Point3 p = si * Point3(c.r, c.g, c.b);
        fprintf(mStream, _T("emissiveColor %s\n"), color(p));
    }
    Indent(level);
    fprintf(mStream, _T("}\n"));


    if (isMulti && textureNum == -1) {
        OutputNoTexture(level);
        return TRUE;
    }

    TextureDesc* td = GetMtlTex(mtl);
    if (!td) {
        OutputNoTexture(level);
        return FALSE;
    }

    Indent(level);
    fprintf(mStream, _T("Texture2 {\n"));
    Indent(level+1);
    fprintf(mStream, _T("filename \"%s\"\n"), td->url);
    Indent(level);
    fprintf(mStream, _T("}\n"));

    BitmapTex* bm = td->tex;
    delete td;

    StdUVGen* uvGen = bm->GetUVGen();
    if (!uvGen) {
        return FALSE;
    }

    // Get the UV offset and scale value for Texture2Transform
    float uOff = uvGen->GetUOffs(mStart);
    float vOff = uvGen->GetVOffs(mStart);
    float uScl = uvGen->GetUScl(mStart);
    float vScl = uvGen->GetVScl(mStart);
    float ang =  uvGen->GetAng(mStart);

    if (uOff == 0.0f && vOff == 0.0f && uScl == 1.0f && vScl == 1.0f &&
        ang == 0.0f) {
        return FALSE;
    }

    Indent(level);
    fprintf(mStream, _T("Texture2Transform {\n"));
    if (uOff != 0.0f || vOff != 0.0f) {
        Indent(level+1);
        UVVert p = UVVert(uOff, vOff, 0.0f);
        fprintf(mStream, _T("translation %s\n"), texture(p));
    }
    if (ang != 0.0f) {
        Indent(level+1);
        fprintf(mStream, _T("rotation %s\n"), floatVal(ang));
    }
    if (uScl != 1.0f || vScl != 1.0f) {
        Indent(level+1);
        UVVert p = UVVert(uScl, vScl, 0.0f);
        fprintf(mStream, _T("scaleFactor %s\n"), texture(p));
    }
    Indent(level);
    fprintf(mStream, _T("}\n"));

    return FALSE;
}

// Create a VRMNL primitive sphere, if appropriate.  
// Returns TRUE if a primitive is created
BOOL
VRBLExport::VrblOutSphere(INode * node, Object *obj, int level)
{
    SimpleObject* so = (SimpleObject*) obj;
    float radius, hemi;
    int basePivot, genUV, smooth;
    BOOL td = HasTexture(node);

    // Reject "base pivot" mapped, non-smoothed and hemisphere spheres
    so->pblock->GetValue(SPHERE_RECENTER, mStart, basePivot, FOREVER);
    so->pblock->GetValue(SPHERE_GENUVS, mStart, genUV, FOREVER);
    so->pblock->GetValue(SPHERE_HEMI, mStart, hemi, FOREVER);
    so->pblock->GetValue(SPHERE_SMOOTH, mStart, smooth, FOREVER);
    if (!smooth || basePivot || (genUV && td) || hemi > 0.0f)
        return FALSE;

    so->pblock->GetValue(SPHERE_RADIUS, mStart, radius, FOREVER);
    
    Indent(level);

    fprintf(mStream, _T("Sphere { radius %s }\n"), floatVal(radius));
 
    return TRUE;
}

// Create a VRMNL primitive cylinder, if appropriate.  
// Returns TRUE if a primitive is created
BOOL
VRBLExport::VrblOutCylinder(INode* node, Object *obj, int level)
{
    SimpleObject* so = (SimpleObject*) obj;
    float radius, height;
    int sliceOn, genUV, smooth;
    BOOL td = HasTexture(node);

    // Reject sliced, non-smooth and mapped cylinders
    so->pblock->GetValue(CYLINDER_GENUVS, mStart, genUV, FOREVER);
    so->pblock->GetValue(CYLINDER_SLICEON, mStart, sliceOn, FOREVER);
    so->pblock->GetValue(CYLINDER_SMOOTH, mStart, smooth, FOREVER);
    if (sliceOn || (genUV && td) || !smooth)
        return FALSE;

    so->pblock->GetValue(CYLINDER_RADIUS, mStart, radius, FOREVER);
    so->pblock->GetValue(CYLINDER_HEIGHT, mStart, height, FOREVER);
    Indent(level);
    fprintf(mStream, _T("Separator {\n"));
    Indent(level+1);
    if (mZUp) {
        fprintf(mStream, _T("Rotation { rotation 1 0 0 %s }\n"),
                floatVal(float(PI/2.0)));
        Indent(level+1);
        fprintf(mStream, _T("Translation { translation 0 %s 0 }\n"),
                floatVal(float(height/2.0)));
    } else {
        Point3 p = Point3(0.0f, 0.0f, height/2.0f);
        fprintf(mStream, _T("Translation { translation %s }\n"), point(p));
    }
    Indent(level+1);
    fprintf(mStream, _T("Cylinder { radius %s "), floatVal(radius));
    fprintf(mStream, _T("height %s }\n"), floatVal(float(fabs(height))));
    Indent(level);
    fprintf(mStream, _T("}\n"));
    
    return TRUE;
}

// Create a VRMNL primitive cone, if appropriate.  
// Returns TRUE if a primitive is created
BOOL
VRBLExport::VrblOutCone(INode* node, Object *obj, int level)
{
    SimpleObject* so = (SimpleObject*) obj;
    float radius1, radius2, height;
    int sliceOn, genUV, smooth;
    BOOL td = HasTexture(node);

    // Reject sliced, non-smooth and mappeded cones
    so->pblock->GetValue(CONE_GENUVS, mStart, genUV, FOREVER);
    so->pblock->GetValue(CONE_SLICEON, mStart, sliceOn, FOREVER);
    so->pblock->GetValue(CONE_SMOOTH, mStart, smooth, FOREVER);
    so->pblock->GetValue(CONE_RADIUS2, mStart, radius2, FOREVER);
    if (sliceOn || (genUV &&td) || !smooth || radius2 > 0.0f)
        return FALSE;

    so->pblock->GetValue(CONE_RADIUS1, mStart, radius1, FOREVER);
    so->pblock->GetValue(CONE_HEIGHT, mStart, height, FOREVER);
    Indent(level);
    
    fprintf(mStream, _T("Separator {\n"));
    Indent(level+1);
    if (mZUp) {
        if (height > 0.0f)
            fprintf(mStream, _T("Rotation { rotation 1 0 0 %s }\n"),
                    floatVal(float(PI/2.0)));
        else
            fprintf(mStream, _T("Rotation { rotation 1 0 0 %s }\n"),
                    floatVal(float(-PI/2.0)));
        Indent(level+1);
        fprintf(mStream, _T("Translation { translation 0 %s 0 }\n"),
                floatVal(float(fabs(height)/2.0)));
    } else {
        Point3 p = Point3(0.0f, 0.0f, (float)fabs(height/2.0f));
        if (height < 0.0f) {
            fprintf(mStream, _T("Rotation { rotation 1 0 0 %s }\n"),
                    floatVal(float(PI)));
            Indent(level+1);
        }
        fprintf(mStream, _T("Translation { translation %s }\n"), point(p));
    }
    Indent(level+1);

    fprintf(mStream, _T("Cone { bottomRadius %s "), floatVal(radius1));
    fprintf(mStream, _T("height %s }\n"), floatVal(float(fabs(height))));
    
    Indent(level);
    fprintf(mStream, _T("}\n"));
    return TRUE;
}

// Create a VRMNL primitive cube, if appropriate.  
// Returns TRUE if a primitive is created
BOOL
VRBLExport::VrblOutCube(INode* node, Object *obj, int level)
{
    Mtl* mtl = node->GetMtl();
    // Multi materials need meshes
    if (mtl && mtl->IsMultiMtl())
        return FALSE;

    SimpleObject* so = (SimpleObject*) obj;
    float length, width, height;
    BOOL td = HasTexture(node);

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
    fprintf(mStream, _T("Separator {\n"));
    Indent(level+1);
    Point3 p = Point3(0.0f,0.0f,height/2.0f);
    // VRML cubes grow from the middle, MAX grows from z=0
    fprintf(mStream, _T("Translation { translation %s }\n"), point(p));
    Indent(level+1);

    if (mZUp) {
        fprintf(mStream, _T("Cube { width %s "),
                floatVal(float(fabs(width))));
        fprintf(mStream, _T("height %s "),
                floatVal(float(fabs(length))));
        fprintf(mStream, _T(" depth %s }\n"),
                floatVal(float(fabs(height))));
    } else {
        fprintf(mStream, _T("Cube { width %s "),
                floatVal(float(fabs(width))));
        fprintf(mStream, _T("height %s "),
                floatVal(float(fabs(height))));
        fprintf(mStream, _T(" depth %s }\n"),
                floatVal(float(fabs(length))));
    }
    Indent(level);
    fprintf(mStream, _T("}\n"));
    
    return TRUE;
}

#define INTENDED_ASPECT_RATIO 1.3333

// Output a perspective camera
BOOL
VRBLExport::VrblOutCamera(INode* node, Object* obj, int level)
{
    // compute camera transform
    ViewParams vp;
    CameraState cs;
    Interval iv;
    CameraObject *cam = (CameraObject *)obj;
    cam->EvalCameraState(0, iv, &cs);
    vp.fov = (float)(2.0 * atan(tan(cs.fov / 2.0) / INTENDED_ASPECT_RATIO));

    Indent(level);
    fprintf(mStream, _T("DEF %s_Animated PerspectiveCamera {\n"), mNodes.GetNodeName(node));
    Indent(level+1);
    fprintf(mStream, _T("position 0 0 0\n"));
    Indent(level+1);
    fprintf(mStream, _T("heightAngle %s\n"), floatVal(vp.fov));
    if (!mZUp) {
        Indent(level+1);
        fprintf(mStream, _T("orientation 1 0 0 %s\n"),
                floatVal(float(-PI/2.0)));
    }
    Indent(level);
    fprintf(mStream, _T("}\n"));

    return TRUE;
}

// Output an omni light
BOOL
VRBLExport::VrblOutPointLight(INode* node, LightObject* light, int level)
{
    LightState ls;
    Interval iv = FOREVER;

    light->EvalLightState(mStart, iv, &ls);

    Indent(level);
    fprintf(mStream, _T("DEF %s PointLight {\n"), mNodes.GetNodeName(node));
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
    Indent(level);
    fprintf(mStream, _T("}\n"));
    return TRUE;
}

// Output a directional light
BOOL
VRBLExport::VrblOutDirectLight(INode* node, LightObject* light, int level)
{
    LightState ls;
    Interval iv = FOREVER;

    light->EvalLightState(mStart, iv, &ls);
    Point3 dir(0,0,-1);

    Indent(level);
    fprintf(mStream, _T("DEF %s DirectionalLight {\n"),  mNodes.GetNodeName(node));
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

// Output a Spot Light
BOOL
VRBLExport::VrblOutSpotLight(INode* node, LightObject* light, int level)
{
    LightState ls;
    Interval iv = FOREVER;

    Point3 dir(0,0,-1);
    light->EvalLightState(mStart, iv, &ls);
    Indent(level);
    fprintf(mStream, _T("DEF %s SpotLight {\n"),  mNodes.GetNodeName(node));
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
    fprintf(mStream, _T("dropOffRate %s\n"),
            floatVal(1.0f - ls.hotsize/ls.fallsize));
    Indent(level+1);
    fprintf(mStream, _T("on %s\n"), ls.on ? _T("TRUE") : _T("FALSE"));
    Indent(level);
    fprintf(mStream, _T("}\n"));
    return TRUE;
}

// Output an omni light at the top-level Separator
BOOL
VRBLExport::VrblOutTopPointLight(INode* node, LightObject* light)
{
    LightState ls;
    Interval iv = FOREVER;

    light->EvalLightState(mStart, iv, &ls);

    Indent(1);
    fprintf(mStream, _T("DEF %s PointLight {\n"),  mNodes.GetNodeName(node));
    Indent(2);
    fprintf(mStream, _T("intensity %s\n"),
            floatVal(light->GetIntensity(mStart, FOREVER)));
    Indent(2);
    Point3 col = light->GetRGBColor(mStart, FOREVER);
    fprintf(mStream, _T("color %s\n"), color(col));
    Indent(2);
    Point3 p = node->GetObjTMAfterWSM(mStart).GetTrans();
    fprintf(mStream, _T("location %s\n"), point(p));

    Indent(2);
    fprintf(mStream, _T("on %s\n"), ls.on ? _T("TRUE") : _T("FALSE"));
    Indent(1);
    fprintf(mStream, _T("}\n"));
    return TRUE;
}

// Output a directional light at the top-level Separator
BOOL
VRBLExport::VrblOutTopDirectLight(INode* node, LightObject* light)
{
    LightState ls;
    Interval iv = FOREVER;

    light->EvalLightState(mStart, iv, &ls);

    Indent(1);
    fprintf(mStream, _T("DEF %s DirectionalLight {\n"),  mNodes.GetNodeName(node));
    Indent(2);
    fprintf(mStream, _T("intensity %s\n"),
            floatVal(light->GetIntensity(mStart, FOREVER)));
    Indent(2);
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
    
    Indent(2);
    fprintf(mStream, _T("direction %s\n"), normPoint(p));
    Indent(2);
    fprintf(mStream, _T("on %s\n"), ls.on ? _T("TRUE") : _T("FALSE"));
    Indent(1);
    fprintf(mStream, _T("}\n"));
    return TRUE;
}

// Output a spot light at the top-level Separator
BOOL
VRBLExport::VrblOutTopSpotLight(INode* node, LightObject* light)
{
    LightState ls;
    Interval iv = FOREVER;

    light->EvalLightState(mStart, iv, &ls);
    Indent(1);
    fprintf(mStream, _T("DEF %s SpotLight {\n"),  mNodes.GetNodeName(node));
    Indent(2);
    fprintf(mStream, _T("intensity %s\n"),
            floatVal(light->GetIntensity(mStart,FOREVER)));
    Indent(2);
    Point3 col = light->GetRGBColor(mStart, FOREVER);
    fprintf(mStream, _T("color %s\n"), color(col));
    Indent(2);
    Point3 p = node->GetObjTMAfterWSM(mStart).GetTrans();
    fprintf(mStream, _T("location %s\n"), point(p));

    Matrix3 tm = node->GetObjTMAfterWSM(mStart);
    p = Point3(0,0,-1);
    Point3 trans, s;
    Quat q;
    AffineParts parts;
    decomp_affine(tm, &parts);
    q = parts.q;
    Matrix3 rot;
    q.MakeMatrix(rot);
    p = p * rot;

    Indent(2);
    fprintf(mStream, _T("direction %s\n"), normPoint(p));
    Indent(2);
    fprintf(mStream, _T("cutOffAngle %s\n"),
            floatVal( DegToRad(ls.fallsize)));
    Indent(2);
    fprintf(mStream, _T("dropOffRate %s\n"),
            floatVal(1.0f - ls.hotsize/ls.fallsize));
    Indent(2);
    fprintf(mStream, _T("on %s\n"), ls.on ? _T("TRUE") : _T("FALSE"));
    Indent(1);
    fprintf(mStream, _T("}\n"));
    return TRUE;
}

// Create a light at the top-level of the file
void
VRBLExport::OutputTopLevelLight(INode* node, LightObject *light)
{
    Class_ID id = light->ClassID();
    if (id == Class_ID(OMNI_LIGHT_CLASS_ID, 0))
        VrblOutTopPointLight(node, light);
    else if (id == Class_ID(DIR_LIGHT_CLASS_ID, 0) ||
             id == Class_ID(TDIR_LIGHT_CLASS_ID, 0))
        VrblOutTopDirectLight(node, light);
    else if (id == Class_ID(SPOT_LIGHT_CLASS_ID, 0) ||
             id == Class_ID(FSPOT_LIGHT_CLASS_ID, 0))
        VrblOutTopSpotLight(node, light);
    
}

// Output a VRML Inline node.
BOOL
VRBLExport::VrblOutInline(VRMLInsObject* obj, int level)
{
    Indent(level);
    fprintf(mStream, _T("WWWInline {\n"));
    Indent(level+1);
    fprintf(mStream, _T("name %s\n"), obj->GetUrl().data());
    float size = obj->GetSize()/ 2.0f;
    Indent(level+1);
    Point3 p = Point3(size, size, size);
    fprintf(mStream, _T("bboxSize %s\n"), scalePoint(p));
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
VRBLExport::VrblOutLOD(INode *node, LODObject* obj, int level)
{
    int numLod = obj->NumRefs();
    Tab<LODObj*> lodObjects = obj->GetLODObjects();
    int i;

    if (numLod == 0)
        return TRUE;

    lodObjects.Sort((CompareFnc) DistComp);

    if (numLod > 1) {
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
                fprintf(mStream, _T("%s ]\n"), floatVal(lodObjects[i]->dist));
        }
    }

    for(i = 0; i < numLod; i++) {
        INode *node = lodObjects[i]->node;
        INode *parent = node->GetParentNode();
        VrblOutNode(node, parent, level+1, TRUE, FALSE, identMat, NULL);
    }

    if (numLod > 1) {
        Indent(level);
        fprintf(mStream, _T("}\n"));
    }

    return TRUE;
}

// return TRUE iff node is the target of a spot light or camera
BOOL
VRBLExport::IsAimTarget(INode* node)
{
    INode* lookAt = node->GetLookatNode();
    if (!lookAt)
        return FALSE;
    Object* lookAtObj = lookAt->EvalWorldState(mStart).obj;
    Class_ID id = lookAtObj->ClassID();
    // Only generate aim targets for targetted spot lights and cameras
    return id == Class_ID(SPOT_LIGHT_CLASS_ID, 0) ||
           id == Class_ID(LOOKAT_CAM_CLASS_ID, 0);
}

// Output an AimTarget.
BOOL
VRBLExport::VrblOutTarget(INode* node, int level)
{
    INode* lookAt = node->GetLookatNode();
    if (!lookAt)
        return TRUE;
    Object* lookAtObj = lookAt->EvalWorldState(mStart).obj;
    Class_ID id = lookAtObj->ClassID();
    // Only generate aim targets for targetted spot lights and cameras
    if (id != Class_ID(SPOT_LIGHT_CLASS_ID, 0) &&
        id != Class_ID(LOOKAT_CAM_CLASS_ID, 0))
        return TRUE;
    Indent(level);
    fprintf(mStream, _T("AimTarget_ktx_com {\n"));
    if (mGenFields) {
        Indent(level+1);
        fprintf(mStream, _T("fields [ SFString aimer ]\n"));
    }
    Indent(level+1);
	if ( (id == Class_ID(LOOKAT_CAM_CLASS_ID, 0)) && IsEverAnimated(lookAt))
		fprintf(mStream, _T("aimer \"%s_Animated\"\n"), mNodes.GetNodeName(lookAt));
	else
		fprintf(mStream, _T("aimer \"%s\"\n"), mNodes.GetNodeName(lookAt));
    Indent(level);
    fprintf(mStream, _T("}\n"));
    return TRUE;
}

// Write out the VRML for nodes we know about, including VRML helper nodes, 
// lights, cameras and VRML primitives
BOOL
VRBLExport::VrblOutSpecial(INode* node, INode* parent,
                             Object* obj, int level)
{
    Class_ID id = obj->ClassID();

    if (id == Class_ID(MR_BLUE_CLASS_ID1, MR_BLUE_CLASS_ID2)) {
        level++;
        VrblOutMrBlue(node, parent, (MrBlueObject*) obj,
                      &level, FALSE);
    }

    if (id == Class_ID(OMNI_LIGHT_CLASS_ID, 0))
        return VrblOutPointLight(node, (LightObject*) obj, level+1);

    if (id == Class_ID(DIR_LIGHT_CLASS_ID, 0) ||
        id == Class_ID(TDIR_LIGHT_CLASS_ID, 0))
        return VrblOutDirectLight(node, (LightObject*) obj, level+1);

    if (id == Class_ID(SPOT_LIGHT_CLASS_ID, 0) ||
        id == Class_ID(FSPOT_LIGHT_CLASS_ID, 0))
        return VrblOutSpotLight(node, (LightObject*) obj, level+1);

    if (id == Class_ID(VRML_INS_CLASS_ID1, VRML_INS_CLASS_ID2))
        return VrblOutInline((VRMLInsObject*) obj, level+1);

    if (id == Class_ID(LOD_CLASS_ID1, LOD_CLASS_ID2))
        return VrblOutLOD(node, (LODObject*) obj, level+1);

    if (id == Class_ID(SIMPLE_CAM_CLASS_ID, 0) ||
        id == Class_ID(LOOKAT_CAM_CLASS_ID, 0))
        return VrblOutCamera(node, obj, level+1);

    if (id == Class_ID(TARGET_CLASS_ID, 0))
        return VrblOutTarget(node, level+1);

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
        return VrblOutSphere(node, obj, level+1);

    if (id == Class_ID(CYLINDER_CLASS_ID, 0))
        return VrblOutCylinder(node, obj, level+1);

    if (id == Class_ID(CONE_CLASS_ID, 0))
        return VrblOutCone(node, obj, level+1);

    if (id == Class_ID(BOXOBJ_CLASS_ID, 0))
        return VrblOutCube(node, obj, level+1);

    return FALSE;
        
}

static BOOL
IsLODObject(Object* obj)
{
    return obj->ClassID() == Class_ID(LOD_CLASS_ID1, LOD_CLASS_ID2);
}

static BOOL
IsDummyObject(Object* obj)
{
    return (!obj || obj->ClassID() == Class_ID(DUMMY_CLASS_ID, 0));
}

// Returns TRUE iff an object or one of its ancestors in animated
static BOOL
IsEverAnimated(INode* node)
{
 // need to sample transform
    Class_ID id = node->EvalWorldState(0).obj->ClassID();
    if (id == Class_ID(SIMPLE_CAM_CLASS_ID, 0) ||
        id == Class_ID(LOOKAT_CAM_CLASS_ID, 0)) return TRUE;

    for (; !node->IsRootNode(); node = node->GetParentNode())
        if (node->IsAnimated())
            return TRUE;
    return FALSE;
}

// Returns TRUE for object that we want a VRML node to occur
// in the file.  
BOOL
VRBLExport::isVrblObject(INode * node, Object *obj, INode* parent)
{
    if (!obj)
        return FALSE;

    Class_ID id = obj->ClassID();
    // Mr Blue nodes only 1st class if stand-alone
    if (id == Class_ID(MR_BLUE_CLASS_ID1, MR_BLUE_CLASS_ID2) ||
        id == Class_ID(VRML_INS_CLASS_ID1, VRML_INS_CLASS_ID2))
        return parent->IsRootNode();

    // only animated light come out in scene graph
    if (IsLight(node) ||
        (id == Class_ID(SIMPLE_CAM_CLASS_ID, 0) ||
         id == Class_ID(LOOKAT_CAM_CLASS_ID, 0)))
        return IsEverAnimated(node);
    /*
 // only animated light come out in scene graph
    if (IsLight(node)) return IsEverAnimated(node);
 
    if (id == Class_ID(SIMPLE_CAM_CLASS_ID, 0) ||
        id == Class_ID(LOOKAT_CAM_CLASS_ID, 0)) {
        return TRUE;
    }
    */

    return (obj->IsRenderable() ||
            id == Class_ID(LOD_CLASS_ID1, LOD_CLASS_ID2) ||
            node->NumberOfChildren() > 0 ||
            IsAimTarget(node)) &&
            (mExportHidden || !node->IsHidden());        
}

// Write the VRML for a single object.
void
VRBLExport::VrblOutObject(INode* node, INode* parent, Object* obj, int level)
{
 // need to get a valid obj ptr

    obj = node->EvalWorldState(mStart).obj;
    BOOL isTriMesh = obj->CanConvertToType(triObjectClassID);
    int numTextures = NumTextures(node);
    int start, end;

    if (numTextures == 0) {
        start = -1;
        end = 0;
    } else {
        start = 0;
        end = numTextures;
    }

        
    for(int i = start; i < end; i++) {
        BOOL multiMat = FALSE, twoSided = FALSE;
        // Output the material
        if (obj->IsRenderable())
            multiMat = OutputMaterial(node, twoSided, level+1, i);

        // First check for VRML primitives and other special objects
        if (VrblOutSpecial(node, parent, obj, level)) {
            return;
        }

        // See if it morphs
        if (mCoordInterp && isTriMesh && ObjIsAnimated(obj)) {
            int j;
            TimeValue tim;
            TimeValue end = mIp->GetAnimRange().End();
            int frames = (end * mCoordSampleRate - 1) / TIME_TICKSPERSEC + 2;
            TimeValue inc = TIME_TICKSPERSEC / mCoordSampleRate;
            TriObject *tri;

            for (j = 0, tim = 0; j < frames; j++, tim += inc) {
                if (tim > end)
                    tim = end;
                Object *o = node->EvalWorldState(tim).obj;
                tri = (TriObject *)o->ConvertToType(tim, triObjectClassID);
                OutputMorphForm(node, tri, tim, j, multiMat, twoSided, i,
                                level+1);
                if (o != (Object *)tri)
                    tri->DeleteThis();
            }
            OutputMorphShape(node, frames, inc, end, level+1);
            mHadAnim = TRUE;
        }

        // Otherwise output as a triangle mesh
        else if (isTriMesh) {
            TriObject *tri = (TriObject *)obj->ConvertToType(0, triObjectClassID);
            OutputTriObject(node, tri, multiMat, twoSided, i, level+1, 0);
#ifndef FUNNY_TEST
            if (obj != (Object *)tri)
                tri->DeleteThis();
#endif
        }
    }
}

void
VRBLExport::OutputMorphForm(INode *node, TriObject *tri, TimeValue tim,
                            int idx, BOOL multiMat, BOOL twoSided, int texNum,
                            int level)
{
    Indent(level);
    fprintf(mStream, _T("DEF %s_form%d MorphForm_ktx_com {\n"),
                        mNodes.GetNodeName(node), tim/GetTicksPerFrame());
    OutputTriObject(node, tri, multiMat, twoSided, texNum, level+1,
                    idx == 0 ? -1 : idx);
    Indent(level);
    fprintf(mStream, _T("}\n"));
}

void
VRBLExport::OutputMorphShape(INode *node, int frames, TimeValue inc,
                             TimeValue end, int level)
{
    int i;
    TimeValue tim;

    Indent(level);
    fprintf(mStream, _T("MorphShape_ktx_com {\n"));
    if (mGenFields) {
        Indent(level+1);
        fprintf(mStream, _T("fields [ MFLong frame, MFString form ]\n"));
    }
    Indent(level+1);
    fprintf(mStream, _T("frame [ "));
    for (i = 0, tim = 0; i < frames; i++, tim += inc) {
        if (tim > end)
            tim = end;
        fprintf(mStream, _T("%d"), tim/GetTicksPerFrame());
        if (i != frames - 1) {
            fprintf(mStream, _T(", "));
            if (i % 10 == 9) {
                fprintf(mStream, _T("\n"));
                Indent(level+2);
            }
        }
    }
    fprintf(mStream, _T(" ]\n"));
    Indent(level+1);
    fprintf(mStream, _T("form [ "));
    for (i = 0, tim = 0; i < frames; i++, tim += inc) {
        if (tim > end)
            tim = end;
        if (i != 0)
            Indent(level+2);
        fprintf(mStream, _T("\"%s_form%d\""), mNodes.GetNodeName(node),
                        tim/GetTicksPerFrame());
        if (i != frames - 1)
            fprintf(mStream, _T(","));
        else
            fprintf(mStream, _T(" ]"));
        fprintf(mStream, _T("\n"));
    }
    Indent(level);
    fprintf(mStream, _T("}\n"));
}

BOOL
VRBLExport::ObjIsAnimated(Object *obj)
{
    if (!obj)
        return FALSE;
    Interval iv = obj->ObjectValidity(mStart);
    return !(iv == FOREVER);
}

// Get the distance to the line of sight target
float 
GetLosProxDist(INode* node, TimeValue t)
{
    Point3 p0 = node->GetObjTMAfterWSM(t).GetTrans();
    Matrix3 tmat;
    node->GetTargetTM(t,tmat);
    Point3 p1 = tmat.GetTrans();
    return Length(p1-p0);
}

// Get the vector to the line of sight target
Point3
GetLosVector(INode* node, TimeValue t)
{
    Point3 p0 = node->GetObjTMAfterWSM(t).GetTrans();
    Matrix3 tmat;
    node->GetTargetTM(t,tmat);
    Point3 p1 = tmat.GetTrans();
    return p1-p0;
}

// Write out the node header for a HyperWire object
void
VRBLExport::VrblAnchorHeader(MrBlueObject* obj, VRBL_TriggerType trigType,
                             BOOL fromParent, int level)
{
    TSTR desc;
    VRBL_Action action = obj->GetAction();

    if (action == HyperLinkJump || action == MrBlueMessage ||
        action == SetViewpoint) {
        switch (trigType) {
        case MouseClick:
            if (action == MrBlueMessage) {
                fprintf(mStream, _T("WWWAnchorPick_ktx_com {\n"));
                Indent(level+1);
                if (mGenFields)
                    fprintf(mStream, _T("fields [ SFString name, SFString description, SFEnum map ]\n"));
            } else {
                fprintf(mStream, _T("WWWAnchor {\n"));
            }
            break;
        case DistProximity:
            if (fromParent) {
                fprintf(mStream, _T("WWWAnchorProximityObject_ktx_com {\n"));
                Indent(level+1);
                if (mGenFields)
                    fprintf(mStream, _T("fields [ SFString name, SFEnum map, SFFloat distance ]\n"));
            } else {
                fprintf(mStream, _T("WWWAnchorProximityPoint_ktx_com {\n"));
                Indent(level+1);
                if (mGenFields)
                    fprintf(mStream, _T("fields [ SFString name, SFEnum map, SFVec3f point, SFFloat distance ]\n"));
            }
            break;
        case BoundingBox:
            fprintf(mStream, _T("WWWAnchorProximityBBox_ktx_com {\n"));
            Indent(level+1);
            if (mGenFields)
                fprintf(mStream, _T("fields [ SFString name, SFEnum map, MFVec3f point, SFFloat distance ]\n"));
            break;
        case LineOfSight:
            if (obj->GetLosType() == CanSee) {
                fprintf(mStream, _T("WWWAnchorLineOfSight_ktx_com {\n"));
                Indent(level+1);
                if (mGenFields)
                    fprintf(mStream, _T("fields [ SFString name, SFEnum map, SFFloat angle, SFFloat distance ]\n"));
            } else {
                fprintf(mStream, _T("WWWAnchorLineOfSightObject_ktx_com {\n"));
                Indent(level+1);
                if (mGenFields)
                    fprintf(mStream, _T("fields [ SFString name, SFEnum map, SFFloat sightAngle, SFVec3f vector, SFFloat vectorAngle, SFFloat distance ]\n"));
            }
            break;
        default:
            assert(FALSE);
        }

        Indent(level+1);
        TSTR camera;
        TCHAR *name = _T("name");

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
    } else {
        switch (trigType) {
        case MouseClick:
            fprintf(mStream, _T("AnimateOnPick_ktx_com {\n"));
            Indent(level+1);
            if (mGenFields)
                fprintf(mStream, _T("fields [ SFString objectname, SFString description, SFBool start ]\n"));
            break;
        case DistProximity:
            if (fromParent) {
                fprintf(mStream, _T("AnimateOnProximityObject_ktx_com {\n"));
                Indent(level+1);
                if (mGenFields)
                    fprintf(mStream, _T("fields [ SFString objectname, SFFloat distance, SFBool start]\n"));
            } else {
                fprintf(mStream, _T("AnimateOnProximityPoint_ktx_com {\n"));
                Indent(level+1);
                if (mGenFields)
                    fprintf(mStream, _T("fields [ SFString objectname, SFVec3f point, SFFloat distance, SFBool start]\n"));
            }
            break;
        case BoundingBox:
            fprintf(mStream, _T("AnimateOnProximityBBox_ktx_com {\n"));
            Indent(level+1);
            if (mGenFields)
                fprintf(mStream, _T("fields [ SFString objectname, MFVec3f point, SFBool start]\n"));
            break;
        case LineOfSight:
            if (obj->GetLosType() == CanSee) {
                fprintf(mStream, _T("AnimateOnLineOfSight_ktx_com {\n"));
                Indent(level+1);
                if (mGenFields)
                    fprintf(mStream, _T("fields [ SFString objectname, SFFloat angle, SFFloat distance, SFBool start ]\n"));
            } else {
                fprintf(mStream, _T("AnimateOnLineOfSightObject_ktx_com {\n"));
                Indent(level+1);
                if (mGenFields)
                    fprintf(mStream, _T("fields [ SFString objectname, SFFloat sightAngle, SFVec3f vector, SFFloat vectorAngle, SFFloat distance, SFBool start ]\n"));
            }
            break;
        default:
            assert(FALSE);
        }
        Indent(level+1);
        TSTR camera;
        TCHAR *name = _T("name");

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
            // Output the object to animate
            fprintf(mStream, _T("objectname \""));
            int size = obj->GetAnimObjects()->Count();
            for(int i=0; i < size; i++) {
                MrBlueAnimObj* animObj = (*obj->GetAnimObjects())[i];
                char *f;
                Object *o = animObj->node->EvalWorldState(mStart).obj;
                if (!o)
                    break;
                SClass_ID sid = o->SuperClassID();
                if (sid == CAMERA_CLASS_ID ||
                    sid == LIGHT_CLASS_ID) {
                    if (i == size-1)
                        f = _T("%s_TopLevel");
                    else
                        f = _T("%s_TopLevel ");
                } else {
                    if (i == size-1)
                        f = _T("%s");
                    else
                        f = _T("%s ");
                }
                fprintf(mStream, f, mNodes.GetNodeName(animObj->node));
            }
            fprintf(mStream, _T("\"\n"));
            Indent(level+1);
            fprintf(mStream, _T("start %s\n"), obj->GetAnimState() == Start ?
                    _T("TRUE") : _T("FALSE"));
            break; }
        default:
            assert(FALSE);
        }
    }
}

// Write out the header for a single Mr. Blue node
BOOL
VRBLExport::VrblOutMrBlue(INode* node, INode* parent, MrBlueObject* obj,
                          int* level, BOOL fromParent)
{
    BOOL hadHeader = FALSE;
    TCHAR* name;
    if (fromParent)
        name =  mNodes.GetNodeName(parent);
    else
        name =  mNodes.GetNodeName(node);

    // if parent node exists
    if (!fromParent && !parent->IsRootNode())
        return TRUE;

    if (obj->GetMouseEnabled()) {
        MrBlueObject* mbo = (MrBlueObject*) obj;
        if (mType == Export_VRBL || mbo->GetAction() == HyperLinkJump
                || mbo->GetAction() == SetViewpoint) {
            Indent(*level);
            fprintf(mStream, _T("DEF %s "), name);
            VrblAnchorHeader(obj, MouseClick, fromParent, *level);
            (*level)++;
            hadHeader = TRUE;
        }
    }

    if (mType != Export_VRBL)
        goto end;

    if (obj->GetProxDistEnabled()) {
        Indent(*level);
        if (!hadHeader)
            fprintf(mStream, _T("DEF %s "), name);
        VrblAnchorHeader(obj, DistProximity, fromParent, *level);
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
        VrblAnchorHeader(obj, BoundingBox, fromParent, *level);
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
            VrblAnchorHeader(obj, LineOfSight, fromParent, *level);
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
            VrblAnchorHeader(obj, LineOfSight, fromParent, *level);
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
        EndMrBlueNode(node, *level);

    return TRUE;
}

// Start the headers for Mr. Blue nodes attached to the given node,
// returning the new indentation level
int
VRBLExport::StartMrBlueHelpers(INode* node, int level)
{
    // Check for Mr Blue helper at child nodes
    for(int i=0; i<node->NumberOfChildren(); i++) {
        INode* childNode = node->GetChildNode(i);
        Object *obj = childNode->EvalWorldState(mStart).obj;
        Class_ID id = obj->ClassID();
        if (id == Class_ID(MR_BLUE_CLASS_ID1, MR_BLUE_CLASS_ID2))
            VrblOutMrBlue(childNode, node, (MrBlueObject*) obj,
                          &level, TRUE);
    }
    return level;
}

// Write out the node closer for a Mr. Blue node
void
VRBLExport::EndMrBlueNode(INode* childNode, int& level)
{
    Object *obj = childNode->EvalWorldState(mStart).obj;
    Class_ID id = obj->ClassID();
    if (id == Class_ID(MR_BLUE_CLASS_ID1, MR_BLUE_CLASS_ID2)) {
        MrBlueObject* mbo = (MrBlueObject*) obj;
        if (mbo->GetMouseEnabled()) {
            if (mType == Export_VRBL || mbo->GetAction() == HyperLinkJump
                || mbo->GetAction() == SetViewpoint) {
                Indent(--level);
                fprintf(mStream, _T("}\n"));
            }
        }
        if (mbo->GetProxDistEnabled()) {
            if (mType == Export_VRBL) {
                Indent(--level);
                fprintf(mStream, _T("}\n"));
            }
        }
        if (mbo->GetBBoxEnabled()) {
            if (mType == Export_VRBL &&
                childNode->GetParentNode()->IsRootNode()) {
                Indent(--level);
                fprintf(mStream, _T("}\n"));
            }
        }
        if (mbo->GetLosEnabled()) {
            if (mType == Export_VRBL) {
                Indent(--level);
                fprintf(mStream, _T("}\n"));
            }
        }
    }
}

// Write out the node closers for all the Mr. Blue headers
void
VRBLExport::EndMrBlueHelpers(INode* node, int level)
{
    // Check for Mr Blue helper at child nodes
    for(int i=0; i<node->NumberOfChildren(); i++) {
        EndMrBlueNode(node->GetChildNode(i), level);
    }
}

// Return TRUE iff the controller is a TCB controller
static BOOL 
IsTCBControl(Control *cont)
{
    return ( cont && (
        cont->ClassID()==Class_ID(TCBINTERP_FLOAT_CLASS_ID,0)    ||
        cont->ClassID()==Class_ID(TCBINTERP_POSITION_CLASS_ID,0) ||
        cont->ClassID()==Class_ID(TCBINTERP_ROTATION_CLASS_ID,0) ||
        cont->ClassID()==Class_ID(TCBINTERP_POINT3_CLASS_ID,0)   ||
        cont->ClassID()==Class_ID(TCBINTERP_SCALE_CLASS_ID,0)));
}

// Return TRUE iff the keys are different in any way.
static BOOL
TCBIsDifferent(ITCBKey *k, ITCBKey* oldK)
{
    return k->tens    != oldK->tens   ||
           k->cont    != oldK->cont   ||
           k->bias    != oldK->bias   ||
           k->easeIn  != oldK->easeIn ||
           k->easeOut != oldK->easeOut;
}

// returns TRUE iff the position keys are exactly the same
static BOOL
PosKeysSame(ITCBPoint3Key& k1, ITCBPoint3Key& k2)
{
    if (TCBIsDifferent(&k1, &k2))
        return FALSE;
    return k1.val == k2.val;
}

// returns TRUE iff the rotation keys are exactly the same
static BOOL
RotKeysSame(ITCBRotKey& k1, ITCBRotKey& k2)
{
    if (TCBIsDifferent(&k1, &k2))
        return FALSE;
    return k1.val.axis == k2.val.axis && k1.val.angle == k2.val.angle;
}

// returns TRUE iff the scale keys are exactly the same
static BOOL
ScaleKeysSame(ITCBScaleKey& k1, ITCBScaleKey& k2)
{
    if (TCBIsDifferent(&k1, &k2))
        return FALSE;
    return k1.val.s == k2.val.s;
}

// Write out all the keyframe data for the TCB given controller
BOOL
VRBLExport::WriteTCBKeys(INode* node, Control *cont,
                         int type, int level)
{
    ITCBFloatKey fkey, ofkey;
    ITCBPoint3Key pkey, opkey;
    ITCBRotKey rkey, orkey;
    ITCBScaleKey skey, oskey;
    ITCBKey *k, *oldK;	
    int num = cont->NumKeys();
    Point3 pval;
    Quat q, qLast = IdentQuat();
    AngAxis rval;
    ScaleValue sval;
    Interval valid;
    Point3 p, po;

    // Get the keyframe interface
    IKeyControl *ikeys = GetKeyControlInterface(cont);
    
    // Gotta have some keys
    if (num == NOT_KEYFRAMEABLE || num == 0 || !ikeys) {
        return FALSE;
    }
    
    // Set up 'k' to point at the right derived class
    switch (type) {
    case KEY_FLOAT: k = &fkey; oldK = &ofkey; break;
    case KEY_POS:   k = &pkey; oldK = &opkey; break;
    case KEY_ROT:   k = &rkey; oldK = &orkey; break;
    case KEY_SCL:   k = &skey; oldK = &oskey; break;
    case KEY_COLOR: k = &pkey; oldK = &opkey; break;
    default: return FALSE;
    }
    
    for (int i=0; i<ikeys->GetNumKeys(); i++) {
        ikeys->GetKey(i,k);
        if (k->time < mStart)
            continue;

        if (i == 0 || TCBIsDifferent(k, oldK)) {
            Indent(level);
            fprintf(mStream, _T("AnimationStyle_ktx_com {\n"));
            Indent(level+1);
            if (mGenFields)
                fprintf(mStream, _T("fields [ SFBool loop, SFBitMask splineUse, SFFloat tension, SFFloat continuity, SFFloat bias, SFFloat easeTo, SFFloat easeFrom, SFVec3f pivotOffset ]\n"));
            Indent(level+1);
            fprintf(mStream, _T("splineUse ("));
            
            // Write flags
            BOOL hadOne = FALSE;
            if (k->tens   != 0.0f) {
                fprintf(mStream, _T("TENSION"));
                hadOne = TRUE;
            }
            if (k->cont   != 0.0f) {
                if (hadOne)
                    fprintf(mStream, _T(" | "));
                fprintf(mStream, _T("CONTINUITY"));
                hadOne = TRUE;
            }
            if (k->bias   != 0.0f) {
                if (hadOne)
                    fprintf(mStream, _T(" | "));
                fprintf(mStream, _T("BIAS"));
                hadOne = TRUE;
            }
            if (k->easeIn != 0.0f) {
                if (hadOne)
                    fprintf(mStream, _T(" | "));
                fprintf(mStream, _T("EASE_TO"));
                hadOne = TRUE;
            }
            if (k->easeOut!= 0.0f) {
                if (hadOne)
                    fprintf(mStream, _T(" | "));
                fprintf(mStream, _T("EASE_FROM"));
                hadOne = TRUE;
            }
            fprintf(mStream, _T(")\n"));
            
            // Write TCB and ease
            if (k->tens   != 0.0f) {
                Indent(level+1);
                fprintf(mStream, _T("tension %s\n"), floatVal(k->tens));
            }
            if (k->cont   != 0.0f) {
                Indent(level+1);
                fprintf(mStream, _T("continuity %s\n"), floatVal(k->cont));
            }
            if (k->bias   != 0.0f) {
                Indent(level+1);
                fprintf(mStream, _T("bias %s\n"), floatVal(k->bias));
            }
            if (k->easeIn != 0.0f) {
                Indent(level+1);
                fprintf(mStream, _T("easeTo %s\n"), floatVal(k->easeIn));
            }
            if (k->easeOut!= 0.0f) {
                Indent(level+1);
                fprintf(mStream, _T("easeFrom %s\n"), floatVal(k->easeOut));
            }

	        // get the pivot offset and remove the rotational component
	        Matrix3 m = Matrix3(TRUE);
	        Quat q = node->GetObjOffsetRot();
	        q.MakeMatrix(m);
            p = -node->GetObjOffsetPos();
	        m = Inverse(m);
	        po = VectorTransform(m, p);
            
            Indent(level+1);
            if (type != KEY_POS) fprintf(mStream, _T("pivotOffset %s\n"), point(po));
            Indent(level);
            fprintf(mStream, _T("}\n"));
            
        }
        // Write values
        switch (type) {
        case KEY_FLOAT: 
            assert(FALSE);
            break;
            
        case KEY_SCL: {
            if (i == 0 && (k->time - mStart) != 0) {
                WriteScaleKey0(node, mStart, level, TRUE);
                WriteScaleKey0(node,
                               k->time-GetTicksPerFrame(), level, TRUE);
            }
            Matrix3 tm = GetLocalTM(node, mStart);
            AffineParts parts;
            decomp_affine(tm, &parts);
            ScaleValue sv(parts.k, parts.u);
            Point3 s = sv.s;
            if (parts.f < 0.0f) s = - s;
            else s = skey.val.s;
            if (i != 0 && ScaleKeysSame(skey, oskey))
                continue;
            mHadAnim = TRUE;
            Indent(level);
            fprintf(mStream, _T("ScaleKey_ktx_com {\n"));
            Indent(level+1);
            if (mGenFields)
                fprintf(mStream,
                        _T("fields [ SFLong frame, SFVec3f scale ]\n"));
            Indent(level+1);
            fprintf(mStream, _T("frame %d\n"), (k->time - mStart)/GetTicksPerFrame());
            Indent(level+1);
            fprintf(mStream, _T("scale %s\n"), scalePoint(s));
            Indent(level);
            fprintf(mStream, _T("}\n"));
            memcpy(oldK, k, sizeof(skey));
            break; }
            
        case KEY_COLOR:
            if (i == 0 && k->time != 0) {
                WritePositionKey0(node, mStart, level, TRUE);
                WritePositionKey0(node,
                                  k->time-GetTicksPerFrame(), level,TRUE);
            }
            if (i != 0 && PosKeysSame(pkey, opkey))
                continue;
            mHadAnim = TRUE;
            Indent(level);
            fprintf(mStream, _T("ColorKey_ktx_com {\n"));
            Indent(level+1);
            if (mGenFields)
                fprintf(mStream,
                        _T("fields [ SFLong frame, SFColor color ]\n"));
            Indent(level+1);
            fprintf(mStream, _T("frame %d\n"), (k->time - mStart)/GetTicksPerFrame());
            Indent(level+1);
            fprintf(mStream, _T("color %s\n"), color(pkey.val));
            Indent(level);
            fprintf(mStream, _T("}\n"));
            memcpy(oldK, k, sizeof(pkey));
            break;

        case KEY_POS:
            if (i == 0 && (k->time - mStart) != 0) {
                WritePositionKey0(node, mStart, level, TRUE);
                WritePositionKey0(node,
                                  k->time-GetTicksPerFrame(), level,TRUE);
            }
            if (i != 0 && PosKeysSame(pkey, opkey))
                continue;
            mHadAnim = TRUE;
            Indent(level);
            fprintf(mStream, _T("PositionKey_ktx_com {\n"));
            Indent(level+1);
            if (mGenFields)
                fprintf(mStream,
                        _T("fields [ SFLong frame, SFVec3f translation ]\n"));
            Indent(level+1);
            fprintf(mStream, _T("frame %d\n"), (k->time - mStart)/GetTicksPerFrame());
	        p = pkey.val;
            Indent(level+1);
            fprintf(mStream, _T("translation %s\n"), point(p));
            Indent(level);
            fprintf(mStream, _T("}\n"));
            memcpy(oldK, k, sizeof(pkey));
            break;
            
        case KEY_ROT: {
            // Note rotation keys are cummulative unlike other keys.
            if (i == 0 && (k->time - mStart) != 0) {
                WriteRotationKey0(node, mStart, level, TRUE);
                WriteRotationKey0(node,
                                  k->time-GetTicksPerFrame(), level,TRUE);
            }
            Matrix3 tm = GetLocalTM(node, k->time);
            Point3 axis;
            Quat q;
            float ang;
            
            AffineParts parts;
            decomp_affine(tm, &parts);
            q = parts.q;
            AngAxisFromQ(q/qLast, &ang, axis);
	        // this removes rotational direction errors when rotating PI
	        // and reduces negative rotational errors
	        if (!round(axis.x + rkey.val.axis.x) &&
	    	    !round(axis.y + rkey.val.axis.y) &&
	    	    !round(axis.z + rkey.val.axis.z)) {
                    ang = rkey.val.angle;
                    axis = rkey.val.axis;
	        }

            // this removes errors if q = (0 0 0 0) for rkey (1 0 0 360)
            if (axis.x == 0.0 &&
                axis.y == 0.0 &&
                axis.z == 0.0 &&
                ang    == 0.0) {
                ang =  rkey.val.angle;
                axis = rkey.val.axis;
            }

            if (i !=0 && ang == 0.0f)
                continue;
            qLast = q;
            mHadAnim = TRUE;
            Indent(level);
            fprintf(mStream, _T("RotationKey_ktx_com {\n"));
            Indent(level+1);
            if (mGenFields)
                fprintf(mStream,
                        _T("fields [ SFLong frame, SFRotation rotation ]\n"));
            Indent(level+1);
            fprintf(mStream, _T("frame %d\n"), (k->time - mStart)/GetTicksPerFrame());
            Indent(level+1);
            fprintf(mStream, _T("rotation %s\n"),
                    axisPoint(axis, ang));
            Indent(level);
            fprintf(mStream, _T("}\n"));
            memcpy(oldK, k, sizeof(rkey));
            break; }
        }
    }
    return TRUE;
}

// Write out all the keyframe data for an arbitrary PRS controller
void
VRBLExport::WriteLinearKeys(INode* node,
                             Tab<TimeValue>& posTimes,
                             Tab<Point3>& posKeys,
                             Tab<TimeValue>& rotTimes,
                             Tab<AngAxis>& rotKeys,
                             Tab<TimeValue>& sclTimes,
                             Tab<Point3>& sclKeys,
                             int type, int level)
{
    AngAxis rval;
    Point3 p, po, s;
    int i;
    TimeValue t;
    Tab<TimeValue>& timeVals = posTimes;

    // Set up 'k' to point at the right derived class
    switch (type) {
    case KEY_POS:
    case KEY_COLOR:
        timeVals = posTimes;
        break;
    case KEY_ROT:
        timeVals = rotTimes;
        break;
    case KEY_SCL:
        timeVals = sclTimes;
        break;
    default: return;
    }

    Indent(level);
    fprintf(mStream, _T("AnimationStyle_ktx_com {\n"));
    Indent(level+1);
    if (mGenFields)
        fprintf(mStream, _T("fields [ SFVec3f pivotOffset ]\n"));
    Indent(level+1);
    // get the pivot offset and remove rotational component
    Matrix3 m = Matrix3(TRUE);
    Quat q = node->GetObjOffsetRot();
    q.MakeMatrix(m);
    p = -node->GetObjOffsetPos();
    m = Inverse(m);
    po = VectorTransform(m, p);

    Indent(level+1);
//    if (type != KEY_POS)		// "rigid motion" change to 3D Engine
								// requires pivot before pos. keys
		fprintf(mStream, _T("pivotOffset %s\n"), point(po));
    Indent(level);
    fprintf(mStream, _T("}\n"));
    
    for (i=0; i < timeVals.Count(); i++) {
        t = timeVals[i];
        if (t < mStart)
            continue;

        // Write values
        switch (type) {
        case KEY_POS:
            mHadAnim = TRUE;
            p = posKeys[i];
            Indent(level);
            fprintf(mStream, _T("PositionKey_ktx_com {\n"));
            Indent(level+1);
            if (mGenFields)
                fprintf(mStream,
                        _T("fields [ SFLong frame, SFVec3f translation ]\n"));
            Indent(level+1);
            fprintf(mStream, _T("frame %d\n"),
                    (timeVals[i]-mStart)/GetTicksPerFrame());
            Indent(level+1);
            fprintf(mStream, _T("translation %s\n"), point(p));
            Indent(level);
            fprintf(mStream, _T("}\n"));
            break;
            
        case KEY_ROT:
            mHadAnim = TRUE;
            rval = rotKeys[i];
            if (rval.angle == 0.0f ||
                fabs(rval.angle - 2.0*PI) < 1.0e-5)
                break;
            Indent(level);
            fprintf(mStream, _T("RotationKey_ktx_com {\n"));
            Indent(level+1);
            if (mGenFields)
                fprintf(mStream,
                        _T("fields [ SFLong frame, SFRotation rotation ]\n"));
            Indent(level+1);
            fprintf(mStream, _T("frame %d\n"),
                    (timeVals[i]-mStart)/GetTicksPerFrame());
            Indent(level+1);
            fprintf(mStream, _T("rotation %s\n"),
                    axisPoint(rval.axis, rval.angle));
            Indent(level);
            fprintf(mStream, _T("}\n"));
            break;
        case KEY_SCL:
            mHadAnim = TRUE;
            s = sclKeys[i];
            Indent(level);
            fprintf(mStream, _T("ScaleKey_ktx_com {\n"));
            Indent(level+1);
            if (mGenFields)
                fprintf(mStream,
                        _T("fields [ SFLong frame, SFVec3f scale ]\n"));
            Indent(level+1);
            fprintf(mStream, _T("frame %d\n"),
                    (timeVals[i]-mStart)/GetTicksPerFrame());
            Indent(level+1);
            fprintf(mStream, _T("scale %s\n"), scalePoint(s));
            Indent(level);
            fprintf(mStream, _T("}\n"));
            break;
            
        case KEY_COLOR:
            mHadAnim = TRUE;
            Indent(level);
            fprintf(mStream, _T("ColorKey_ktx_com {\n"));
            Indent(level+1);
            if (mGenFields)
                fprintf(mStream,
                        _T("fields [ SFLong frame, SFColor color ]\n"));
            Indent(level+1);
            fprintf(mStream, _T("frame %d\n"),
                    (timeVals[i]-mStart)/GetTicksPerFrame());
            Indent(level+1);
            p = posKeys[i];
            fprintf(mStream, _T("color %s\n"), color(p));
            Indent(level);
            fprintf(mStream, _T("}\n"));
            break;

        }
    }
            
    return;
}

int
VRBLExport::WriteAllControllerData(INode* node, int flags, int level,
                                   Control* lc)
{
    int i;
    TimeValue t;
    TimeValue end = mIp->GetAnimRange().End();
    int frames = (end - mStart)/GetTicksPerFrame();
    Point3 p, axis, s;
    Quat q, qLast = IdentQuat();
    Matrix3 tm, ip;
    int retVal = 0;

    // Tables of keyframe values
    Tab<Point3>    posKeys;
    Tab<TimeValue> posTimes;
    Tab<Point3>    scaleKeys;
    Tab<TimeValue> scaleTimes;
    Tab<AngAxis>   rotKeys;
    Tab<TimeValue> rotTimes;
    BOOL keys;

    // Set up 'k' to point at the right derived class
    if (flags & KEY_POS) {
        Control* pc = node->GetTMController()->GetPositionController();
        if (IsTCBControl(pc)) {
            keys = WriteTCBKeys(node, pc, KEY_POS, level);
            flags &= ~KEY_POS;
            if (keys)
                retVal |= KEY_POS;
        } else {
            posKeys.SetCount(frames+1);
            posTimes.SetCount(frames+1);
        }
    }
    if (flags & KEY_COLOR) {
        posKeys.SetCount(frames+1);
        posTimes.SetCount(frames+1);
    }
    if (flags & KEY_ROT) {
        Control* rc = node->GetTMController()->GetRotationController();
        // disabling writing tcb rotation keys because position controller
        // like path controller also change rotation so you have a tcbrotation controller
        // with no keys.
        if (IsTCBControl(rc) && rc->NumKeys() && FALSE) {
            keys = WriteTCBKeys(node, rc, KEY_ROT, level);
            flags &= ~KEY_ROT;
            if (keys)
                retVal |= KEY_ROT;
        } else {
            rotKeys.SetCount(frames+1);
            rotTimes.SetCount(frames+1);
        }
    }
    if (flags & KEY_SCL) {
        Control* sc = node->GetTMController()->GetScaleController();
        if (IsTCBControl(sc)) {
            keys = WriteTCBKeys(node, sc, KEY_SCL, level);
            flags &= ~KEY_SCL;
            if (keys)
                retVal |= KEY_SCL;
        } else {
            scaleKeys.SetCount(frames+1);
            scaleTimes.SetCount(frames+1);
        }
    }

    if (!flags)
        return retVal;

    // Sample the controller at every frame
    for(i = 0, t = mStart; i <= frames; i++, t += GetTicksPerFrame()) {
        if (flags & KEY_COLOR) {
            lc->GetValue(t, &posKeys[i], FOREVER);
            posTimes[i] = t;
            continue;
        }
        tm = GetLocalTM(node, t);
        AffineParts parts;
        decomp_affine(tm, &parts);
        if (flags & KEY_SCL) {
            s = ScaleValue(parts.k, parts.u).s;
            if (parts.f < 0.0f)
                s = - s;
            scaleTimes[i] = t;
            scaleKeys[i]  = s;
        }
            
        if (flags & KEY_POS) {
            p = parts.t;
            posTimes[i] = t;
            posKeys[i]  = p;
        }
            
        if (flags & KEY_ROT) {
            q = parts.q;
            rotTimes[i] = t;
            rotKeys[i] = AngAxis(q/qLast);
            qLast = q;
        }
    }

    int newKeys;
    float eps;
    if (flags & KEY_POS) {
        eps = float(1.0e-5);
        newKeys = reducePoint3Keys(posTimes, posKeys, eps);
        if (newKeys != 0) {
            retVal |= KEY_POS;
            WriteLinearKeys(node,
                            posTimes,   posKeys,
                            rotTimes,   rotKeys,
                            scaleTimes, scaleKeys,
                            KEY_POS, level);
        }
    }
    if (flags & KEY_ROT) {
        eps = float(1.0e-5);
        newKeys = reduceAngAxisKeys(rotTimes, rotKeys, eps, TRUE);
												// cumulative angles
        if (newKeys != 0) {
            retVal |= KEY_ROT;
            WriteLinearKeys(node,
                            posTimes,   posKeys,
                            rotTimes,   rotKeys,
                            scaleTimes, scaleKeys,
                            KEY_ROT, level);
        }
    }
    if (flags & KEY_SCL) {
        eps = float(1.0e-5);
        newKeys = reducePoint3Keys(scaleTimes, scaleKeys, eps);
        if (newKeys != 0) {
            retVal |= KEY_SCL;
            WriteLinearKeys(node,
                            posTimes,   posKeys,
                            rotTimes,   rotKeys,
                            scaleTimes, scaleKeys,
                            KEY_SCL, level);
        }
    }
    if (flags & KEY_COLOR) {
        eps = float(1.0e-5);
        newKeys = reducePoint3Keys(posTimes, posKeys, eps);
        if (newKeys != 0) {
            retVal |= KEY_SCL;
            WriteLinearKeys(node,
                            posTimes,   posKeys,
                            rotTimes,   rotKeys,
                            scaleTimes, scaleKeys,
                            KEY_COLOR, level);
        }
    }
    return retVal;
}

// Write out the initial position key, relative to the parent.
void
VRBLExport::WritePositionKey0(INode* node, TimeValue t, int level, BOOL force)
{
    Matrix3 tm = GetLocalTM(node, mStart);
    Point3 p = tm.GetTrans();

    // Don't need a key for identity translate
    if (!force && (p.x == 0.0f && p.y == 0.0f && p.z == 0.0f))
        return;

    mHadAnim = TRUE;
    Indent(level);
    fprintf(mStream, _T("PositionKey_ktx_com {\n"));
    Indent(level+1);
    if (mGenFields)
        fprintf(mStream, _T("fields [ SFLong frame, SFVec3f translation ]\n"));
    Indent(level+1);
    fprintf(mStream, _T("frame %d\n"), (t-mStart)/GetTicksPerFrame());
    Indent(level+1);
    fprintf(mStream, _T("translation %s\n"), point(p));
    Indent(level);
    fprintf(mStream, _T("}\n"));
    
}

// Write out the initial rotation key, relative to the parent.
void
VRBLExport::WriteRotationKey0(INode* node, TimeValue t, int level, BOOL force)
{
    Matrix3 tm = GetLocalTM(node, mStart);
    Point3 p, s, axis;
    Quat q;
    float ang;

    AffineParts parts;
    decomp_affine(tm, &parts);
    p = parts.t;
    q = parts.q;
    AngAxisFromQ(q, &ang, axis);

    // Dont't need a ket for identity rotate
    if (!force && ang == 0.0f)
        return;

    mHadAnim = TRUE;
    Indent(level);
    fprintf(mStream, _T("RotationKey_ktx_com {\n"));
    Indent(level+1);
    if (mGenFields)
        fprintf(mStream,
                _T("fields [ SFLong frame, SFRotation rotation ]\n"));
    Indent(level+1);
    fprintf(mStream, _T("frame %d\n"), (t-mStart)/GetTicksPerFrame());
    Indent(level+1);
    fprintf(mStream, _T("rotation %s\n"), axisPoint(axis, ang));
    Indent(level);
    fprintf(mStream, _T("}\n"));
}

// Write out the initial scale key, relative to the parent.
void
VRBLExport::WriteScaleKey0(INode* node, TimeValue t, int level, BOOL force)
{ 
    Matrix3 tm = GetLocalTM(node, mStart);
    AffineParts parts;
    decomp_affine(tm, &parts);
    ScaleValue sv(parts.k, parts.u);
    Point3 s = sv.s;
    if (parts.f < 0.0f)
        s = - s;

    // Don't need a key for identity scale
    if (!force && (s.x == 1.0f && s.y == 1.0f && s.z == 1.0f))
        return;

    mHadAnim = TRUE;
    Indent(level);
    fprintf(mStream, _T("ScaleKey_ktx_com {\n"));
    Indent(level+1);
    if (mGenFields)
        fprintf(mStream, _T("fields [ SFLong frame, SFVec3f scale ]\n"));
    Indent(level+1);
    fprintf(mStream, _T("frame %d\n"), (t-mStart)/GetTicksPerFrame());
    Indent(level+1);
    fprintf(mStream, _T("scale %s\n"), scalePoint(s));
    Indent(level);
    fprintf(mStream, _T("}\n"));
}

void
VRBLExport::WriteVisibilityData(INode *node, int level) {
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
VRBLExport::IsLight(INode* node)
{
    Object* obj = node->EvalWorldState(mStart).obj;
    if (!obj)
        return FALSE;

    SClass_ID sid = obj->SuperClassID();
    return sid == LIGHT_CLASS_ID;
}

Control *
VRBLExport::GetLightColorControl(INode* node)
{
    if (!IsLight(node))
        return NULL;
    Object* obj = node->EvalWorldState(mStart).obj;
    IParamBlock *pblock = (IParamBlock *) obj->SubAnim(0);
    Control* cont = pblock->GetController(0);  // I know color is index 0!
    return cont;
}

#define NeedsKeys(nkeys) ((nkeys) > 0 || (nkeys) == NOT_KEYFRAMEABLE)

BOOL
VRBLExport::NeedsSomeKeys(INode *node)
{
    Control *c;

    if (mType != Export_VRBL)
        return FALSE;
    if (node->IsRootNode())
        return FALSE;
    c = node->GetTMController()->GetPositionController();
    if (c && c->NumKeys() > 0)
        return TRUE;
    c = node->GetTMController()->GetRotationController();
    if (c && c->NumKeys() > 0)
        return TRUE;
    c = node->GetTMController()->GetScaleController();
    if (c && c->NumKeys() > 0)
        return TRUE;
    c = node->GetVisController();
    if (c && c->NumKeys() > 0)
        return TRUE;
    c = GetLightColorControl(node);
    if (c && NeedsKeys(c->NumKeys()))
        return TRUE;
    return FALSE;
}

// Write out all PRS keyframe data, if it exists
void
VRBLExport::VrblOutControllers(INode* node, int level)
{
    Control *pc, *rc, *sc, *vc, *lc;
    int npk = 0, nrk = 0, nsk = 0, nvk = 0, nlk = 0;

    if (mType != Export_VRBL)
        return;

    pc = node->GetTMController()->GetPositionController();
    if (pc) npk = pc->NumKeys();
    rc = node->GetTMController()->GetRotationController();
    if (rc) nrk = rc->NumKeys();
    sc = node->GetTMController()->GetScaleController();
    if (sc) nsk = sc->NumKeys();
    vc = node->GetVisController();
    if (vc) nvk = vc->NumKeys();
    lc = GetLightColorControl(node);
    if (lc) nlk = lc->NumKeys();
    if (NeedsKeys(nlk))
        WriteAllControllerData(node, KEY_COLOR, level, lc);

    Class_ID id = node->GetTMController()->ClassID();
    int flags = 0;

    if (id != Class_ID(PRS_CONTROL_CLASS_ID, 0))
        flags = KEY_POS | KEY_ROT | KEY_SCL;
     else {
         pc = node->GetTMController()->GetPositionController();
         if (pc) npk = pc->NumKeys();
         rc = node->GetTMController()->GetRotationController();
         if (rc) nrk = rc->NumKeys();
         sc = node->GetTMController()->GetScaleController();
         if (sc) nsk = sc->NumKeys();
         if (NeedsKeys(npk))
             flags |= KEY_POS | KEY_ROT;  // pos controllers can affect rot
         if (NeedsKeys(nrk))
             flags |= KEY_ROT;
         if (NeedsKeys(nsk))
             flags |=  KEY_SCL;
     }
    if (flags) {
        int newFlags = WriteAllControllerData(node, flags, level, NULL);
        if (!(newFlags & KEY_POS))
            WritePositionKey0(node, mStart, level, FALSE);
        if (!(newFlags & KEY_ROT))
            WriteRotationKey0(node, mStart, level, FALSE);
        if (!(newFlags & KEY_SCL))
            WriteScaleKey0(node, mStart, level, FALSE);
    }
    if (NeedsKeys(nvk))
        WriteVisibilityData(node, level);
#if 0
    // FIXME add this back!
    if (NeedsKeys(nlk))
        WriteControllerData(node, lc, KEY_COLOR, level);
#endif
}

// Output a camera at the top level of the file
void
VRBLExport::VrmlOutTopLevelCamera(int level, INode* node, BOOL topLevel)
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
		if (qRot == q) {
			axis.x = axis.z = 0.0f;
			axis.y = 1.0f;
			ang = 0.0f;
		} else AngAxisFromQ(q/qRot, &ang, axis);
	} else
        AngAxisFromQ(q, &ang, axis);

    ViewParams vp;
    CameraState cs;
    Interval iv;
    cam->EvalCameraState(0, iv, &cs);
    vp.fov = (float)(2.0 * atan(tan(cs.fov / 2.0) / INTENDED_ASPECT_RATIO));

    Indent(level);
    fprintf(mStream, _T("DEF %s PerspectiveCamera {\n"), mNodes.GetNodeName(node));
    Indent(level+1);
    fprintf(mStream, _T("position %s\n"), point(p));
    Indent(level+1);
    fprintf(mStream, _T("orientation %s\n"), axisPoint(axis, -ang));
    Indent(level+1);
    fprintf(mStream, _T("heightAngle %s\n"), floatVal(vp.fov));
    Indent(level);
    fprintf(mStream, _T("}\n"));

}

// From dllmain.cpp
extern HINSTANCE hInstance;

// Write out some comments at the top of the file.
void
VRBLExport::VrblOutFileInfo()
{
    char filename[MAX_PATH];
    DWORD size, dummy;
    float vernum = 1.0f;
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
    Indent(1);
    fprintf(mStream, _T("Info { string \"Produced by 3D Studio MAX VRML 1.0/VRBL exporter, Version %.5g, Revision %.5g\" }\n"), vernum, betanum);
    
    
    time_t ltime;
    time( &ltime );
    char * time = ctime(&ltime);
    // strip the CR
    time[strlen(time)-1] = '\0';
    TCHAR* fn = mIp->GetCurFileName();
    if (fn && _tcslen(fn) > 0) {
        Indent(1);
        fprintf(mStream, _T("Info { string \"MAX File: %s, Date: %s\" }\n"), fn, time);
    } else {
        Indent(1);
        fprintf(mStream, _T("Info { string \"Date: %s\" }\n"), time);
    }
    Indent(1);
    fprintf(mStream, _T("ShapeHints {\n"));
    Indent(2);
    fprintf(mStream, _T("shapeType SOLID\n"));
    Indent(2);
    fprintf(mStream, _T("vertexOrdering COUNTERCLOCKWISE\n"));
    Indent(2);
    fprintf(mStream, _T("faceType CONVEX\n"));
    Indent(1);
    fprintf(mStream, _T("}\n"));
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
VRBLExport::VrblOutNode(INode* node, INode* parent, int level, BOOL isLOD,
                        BOOL lastChild, Matrix3 &pendingTM, TCHAR *pname)
{
    // Don't gen code for LOD references, only LOD nodes
    if (!isLOD && ObjectIsLODRef(node)) {
        if (mEnableProgressBar)
            SendMessage(hWndPB, PBM_STEPIT, 0, 0);
        return;
    }

    if (mEnableProgressBar) SendMessage(hWndPDlg, 666, 0, (LPARAM) mNodes.GetNodeName(node));
    
    Object *obj = node->EvalWorldState(mStart).obj;
    BOOL outputName = TRUE;
    int numChildren = node->NumberOfChildren();
    BOOL isVrml = isVrblObject(node, obj, parent);

    // Anchors need to come first, even though they are child nodes
    if (!node->IsRootNode()) {
        int newLevel = StartMrBlueHelpers(node, level);
        if (newLevel != level)
            outputName = FALSE;
        level = newLevel;
    }

    BOOL condense = mCondenseTM && numChildren <= 1 && IsDummyObject(obj) &&
                    !NeedsSomeKeys(node) && !node->IsRootNode()  && outputName;
    if (condense && numChildren == 1) {
        INode* childNode = node->GetChildNode(0);
        if (NeedsSomeKeys(childNode))
            condense = FALSE;
        else {
            Object *obj = childNode->EvalWorldState(mStart).obj;
            Class_ID id = obj->ClassID();
            if (id == Class_ID(MR_BLUE_CLASS_ID1, MR_BLUE_CLASS_ID2))
                condense = FALSE;
        }
    }

    if (node->IsRootNode() || (obj && isVrml)) {
        if (!condense)
            StartNode(node, obj, level, outputName, pname);
        if (node->IsRootNode()) {
            VrblOutFileInfo();
            // Collect a list of all LOD nodes and textures for later use.
            if (mCamera)
                VrmlOutTopLevelCamera(level+ 1, mCamera, TRUE);
            ScanSceneGraph();
        }
    }

    if (obj && isVrml && !condense) {
        if (!IsLODObject(obj)) {
            OutputNodeTransform(node, level+1, pendingTM);
        
            // If the node has a controller, output the data
            VrblOutControllers(node, level+1);
        }
        // Output the data for the object at this node
        VrblOutObject(node, parent, obj, level);
    }

    obj = node->EvalWorldState(mStart).obj;
    if (mEnableProgressBar)
        SendMessage(hWndPB, PBM_STEPIT, 0, 0);

    // Now output the children
    for(int i=0; i < numChildren; i++) {
        if (condense)
            VrblOutNode(node->GetChildNode(i), node, level,
                        FALSE, i == numChildren - 1,
                        GetLocalTM(node, mStart) * pendingTM,
                        pname == NULL ? mNodes.GetNodeName(node) : pname);
        else
            VrblOutNode(node->GetChildNode(i), node, level+1,
                        FALSE, i == numChildren - 1, identMat, NULL);
    }
    
    if (node->IsRootNode() || (obj && (isVrblObject(node, obj, parent)))) {
        if (node->IsRootNode())
            VrblOutAnimationFrames();
        if (!condense)
            EndNode(node, level, lastChild);
    }
    
    // End the anchors if needed
    if (!node->IsRootNode())
        EndMrBlueHelpers(node, level);

}

// Write the "AnimationFrames" VRBL node
void
VRBLExport::VrblOutAnimationFrames()
{
    if (mType == Export_VRBL && mHadAnim) {
        Indent(1);
        fprintf(mStream, _T("AnimationFrames_ktx_com {\n"));
        Indent(2);
        if (mGenFields)
            fprintf(mStream, _T("fields [ SFLong length, SFLong segmentStart, SFLong segmentEnd, SFLong current, SFFloat rate ]\n"));
        Indent(2);
        int frames = (mIp->GetAnimRange().End()-mIp->GetAnimRange().Start())/
                      GetTicksPerFrame() + 1;
        fprintf(mStream, _T("length %d\n"), frames);
        Indent(2);
        fprintf(mStream, _T("rate %d\n"), GetFrameRate());
        Indent(1);
        fprintf(mStream, _T("}\n"));
    }
}


// Traverse the scene graph looking for LOD nodes.
void
VRBLExport::TraverseNode(INode* node)
{
    if (!node) return;
    Object* obj = node->EvalWorldState(mStart).obj;

    if (obj && obj->ClassID() == Class_ID(LOD_CLASS_ID1, LOD_CLASS_ID2))
        mLodList = mLodList->AddNode(node);

    if (IsLight(node) && !IsEverAnimated(node)) {
        OutputTopLevelLight(node, (LightObject*) obj);
    }

    if (obj) {
        Class_ID id = obj->ClassID();
        if ((id == Class_ID(SIMPLE_CAM_CLASS_ID, 0) ||
             id == Class_ID(LOOKAT_CAM_CLASS_ID, 0)) && !IsEverAnimated(node))
            VrmlOutTopLevelCamera(1, node, FALSE);
    }

    int n = node->NumberOfChildren();
    for(int i = 0; i < n; i++)
        TraverseNode(node->GetChildNode(i));
}

void
VRBLExport::ComputeWorldBoundBox(INode* node, ViewExp* vpt)
{
    if (!node) return;
    Object* obj = node->EvalWorldState(mStart).obj;
    Class_ID id;

    if (obj) {
        Box3 bb;
        obj->GetWorldBoundBox(mStart, node, vpt, bb);
        mBoundBox += bb;
    }

    int n = node->NumberOfChildren();
    for(int i = 0; i < n; i++)
        ComputeWorldBoundBox(node->GetChildNode(i), vpt);
}

// Make a list of al the LOD objects in the scene.
void
VRBLExport::ScanSceneGraph()
{
//    ViewExp *vpt = mIp->GetViewport(NULL);
    INode* node = mIp->GetRootNode();
//    ComputeWorldBoundBox(node, vpt);
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
VRBLExport::ObjectIsLODRef(INode* node)
{
    INodeList* l = mLodList;

    for(; l; l = l->GetNext())
        if (ObjectIsReferenced(l->GetNode(), node))
            return TRUE;

    return FALSE;
}


// Dialog procedures

// Collect up a table with pointers to all the camera nodes in it
void
VRBLExport::GetCameras(INode *inode, Tab<INode*> *camList,
                       Tab<INode*> *navInfoList,
                       Tab<INode*> *backgrounds,
                       Tab<INode*> *fogs)
{
    const ObjectState& os = inode->EvalWorldState(mStart);
    Object* ob = os.obj;
    if (ob != NULL) {
        if (ob->SuperClassID() == CAMERA_CLASS_ID)
            camList->Append(1, &inode);

        if (ob->ClassID() == NavInfoClassID)
            navInfoList->Append(1, &inode);

        if (ob->ClassID() == BackgroundClassID)
            backgrounds->Append(1, &inode);

        if (ob->ClassID() == FogClassID)
            fogs->Append(1, &inode);
    }
    int count = inode->NumberOfChildren();
    for (int i = 0; i < count; i++)
        GetCameras(inode->GetChildNode( i), camList, navInfoList,
                   backgrounds, fogs);
}

// Get a chunk of app data off the sound object
void
GetAppData(Interface * ip, int id, TCHAR* def, TCHAR* val, int len)
{
    SoundObj *node = ip->GetSoundObject();
    AppDataChunk *ad = node->GetAppDataChunk(Class_ID(VRBL_EXPORT_CLASS_ID,0),
                                             SCENE_EXPORT_CLASS_ID, id);
    if (!ad)
        _tcscpy(val, def);
    else
        _tcscpy(val, (TCHAR*) ad->data);
}

// Write a chunk of app data on the sound object
void
WriteAppData(Interface* ip, int id, TCHAR* val)
{
    SoundObj *node = ip->GetSoundObject();
    node->RemoveAppDataChunk(Class_ID(VRBL_EXPORT_CLASS_ID,0),
                             SCENE_EXPORT_CLASS_ID, id);
    TCHAR* buf = (TCHAR*) malloc(_tcslen(val)+1);
    _tcscpy(buf, val);
    node->AddAppDataChunk(Class_ID(VRBL_EXPORT_CLASS_ID,0),
                          SCENE_EXPORT_CLASS_ID, id,
                          _tcslen(val)+1, buf);
    SetSaveRequiredFlag(TRUE);
}

extern HINSTANCE hInstance;

ISpinnerControl* VRBLExport::tformSpin = NULL;
ISpinnerControl* VRBLExport::coordSpin = NULL;


static INT_PTR CALLBACK
SampleRatesDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
    TCHAR text[MAX_PATH];
    VRBLExport *exp;
    if (msg == WM_INITDIALOG) {
        SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
    }
    exp = (VRBLExport *)GetWindowLongPtr(hDlg, GWLP_USERDATA);
    switch (msg) {
    case WM_INITDIALOG: {
        CenterWindow(hDlg, GetParent(hDlg));
        GetAppData(exp->mIp, TFORM_SAMPLE_ID, _T("custom"), text, MAX_PATH);
        BOOL once = _tcscmp(text, _T("once")) == 0;
        CheckDlgButton(hDlg, IDC_TFORM_ONCE, once);
        CheckDlgButton(hDlg, IDC_TFORM_CUSTOM, !once);
        EnableWindow(GetDlgItem(hDlg, IDC_TFORM_EDIT), !once);
        EnableWindow(GetDlgItem(hDlg, IDC_TFORM_SPIN), !once);
        
        GetAppData(exp->mIp, TFORM_SAMPLE_RATE_ID, _T("10"), text, MAX_PATH);
        int sampleRate = atoi(text);

        exp->tformSpin = GetISpinner(GetDlgItem(hDlg, IDC_TFORM_SPIN));
        exp->tformSpin->SetLimits(1, 100);
        exp->tformSpin->SetValue(sampleRate, FALSE);
        exp->tformSpin->SetAutoScale();
        exp->tformSpin->LinkToEdit(GetDlgItem(hDlg, IDC_TFORM_EDIT), EDITTYPE_INT);

        GetAppData(exp->mIp, COORD_SAMPLE_ID, _T("custom"), text, MAX_PATH);
        once = _tcscmp(text, _T("once")) == 0;
        CheckDlgButton(hDlg, IDC_COORD_ONCE, once);
        CheckDlgButton(hDlg, IDC_COORD_CUSTOM, !once);
        EnableWindow(GetDlgItem(hDlg, IDC_COORD_EDIT), !once);
        EnableWindow(GetDlgItem(hDlg, IDC_COORD_SPIN), !once);
        
        GetAppData(exp->mIp, COORD_SAMPLE_RATE_ID, _T("3"), text, MAX_PATH);
        sampleRate = atoi(text);

        exp->coordSpin = GetISpinner(GetDlgItem(hDlg, IDC_COORD_SPIN));
        exp->coordSpin->SetLimits(1, 100);
        exp->coordSpin->SetValue(sampleRate, FALSE);
        exp->coordSpin->SetAutoScale();
        exp->coordSpin->LinkToEdit(GetDlgItem(hDlg, IDC_COORD_EDIT), EDITTYPE_INT);
        return TRUE;
    }
    case WM_DESTROY:
        ReleaseISpinner(exp->tformSpin);
        ReleaseISpinner(exp->coordSpin);
        break;
    case WM_COMMAND:
        switch(LOWORD(wParam)) {
        case IDC_TFORM_ONCE:
            exp->tformSpin->Disable();
            return TRUE;
        case IDC_TFORM_CUSTOM:
            exp->tformSpin->Enable();
            return TRUE;
        case IDC_COORD_ONCE:
            exp->coordSpin->Disable();
            return TRUE;
        case IDC_COORD_CUSTOM:
            exp->coordSpin->Enable();
            return TRUE;
        case IDCANCEL:
            EndDialog(hDlg, FALSE);
            return TRUE;
            break;
        case IDOK: {
            BOOL once = IsDlgButtonChecked(hDlg, IDC_TFORM_ONCE);
            exp->SetTformSample(once);
            TCHAR* val = once ? _T("once") : _T("custom");
            WriteAppData(exp->mIp, TFORM_SAMPLE_ID, val);
            int rate = exp->tformSpin->GetIVal();
            exp->SetTformSampleRate(rate);
            sprintf(text, _T("%d"), rate);
            WriteAppData(exp->mIp, TFORM_SAMPLE_RATE_ID, text);

            once = IsDlgButtonChecked(hDlg, IDC_COORD_ONCE);
            exp->SetCoordSample(once);
            val = once ? _T("once") : _T("custom");
            WriteAppData(exp->mIp, COORD_SAMPLE_ID, val);
            rate = exp->coordSpin->GetIVal();
            exp->SetCoordSampleRate(rate);
            sprintf(text, _T("%d"), rate);
            WriteAppData(exp->mIp, COORD_SAMPLE_RATE_ID, text);

            EndDialog(hDlg, TRUE);
            return TRUE;
        }
        }
    }
    return FALSE;
}

static INT_PTR CALLBACK
WorldInfoDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
    TCHAR text[MAX_PATH];
    VRBLExport *exp;
    if (msg == WM_INITDIALOG) {
        SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
    }
    exp = (VRBLExport *)GetWindowLongPtr(hDlg, GWLP_USERDATA);
    switch (msg) {
    case WM_INITDIALOG: {
        CenterWindow(hDlg, GetParent(hDlg));
        GetAppData(exp->mIp, TITLE_ID, _T(""), text, MAX_PATH);
        Edit_SetText(GetDlgItem(hDlg, IDC_TITLE), text);
        GetAppData(exp->mIp, INFO_ID, _T(""), text, MAX_PATH);
        Edit_SetText(GetDlgItem(hDlg, IDC_INFO), text);
        return TRUE;
    }
    case WM_COMMAND:
        switch(LOWORD(wParam)) {
        case IDCANCEL:
            EndDialog(hDlg, FALSE);
            return TRUE;
        case IDOK:
            Edit_GetText(GetDlgItem(hDlg, IDC_TITLE), text, MAX_PATH);
            WriteAppData(exp->mIp, TITLE_ID, text);
            Edit_GetText(GetDlgItem(hDlg, IDC_INFO), text, MAX_PATH);
            WriteAppData(exp->mIp, INFO_ID, text);
            EndDialog(hDlg, TRUE);
            return TRUE;
        }
    }
    return FALSE;
}

static INT_PTR CALLBACK
AboutDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
    switch (msg) {
    case WM_INITDIALOG: {
        CenterWindow(hDlg, GetParent(hDlg));
        return TRUE;
    }
    case WM_COMMAND:
        switch(LOWORD(wParam)) {
        case IDOK:
            EndDialog(hDlg, TRUE);
            return TRUE;
        }
    }
    return FALSE;
}

static void
EnableControls(HWND hDlg, ExportType type)
{
    BOOL enable = type == Export_VRBL;
    EnableWindow(GetDlgItem(hDlg, IDC_GEN_FIELDS), enable);
    EnableWindow(GetDlgItem(hDlg, IDC_ENABLE_PROGRESS_BAR), TRUE);
    EnableWindow(GetDlgItem(hDlg, IDC_COLOR_PER_VERTEX), TRUE);
}

// Dialog procedure for the export dialog.
static INT_PTR CALLBACK
VrblExportDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
    TCHAR text[MAX_PATH];
    VRBLExport *exp;
    ExportType type;
    int sampleRate;
    ISpinnerControl *morphPSSpin;
    if (msg == WM_INITDIALOG) {
        SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
    }
    exp = (VRBLExport *)GetWindowLongPtr(hDlg, GWLP_USERDATA);
    switch (msg) {
    case WM_INITDIALOG: {
		SetWindowContextHelpId(hDlg, idh_vrmlimp_import);
        CenterWindow(hDlg, GetParent(hDlg));
        GetAppData(exp->mIp, NORMALS_ID, _T("no"), text, MAX_PATH);
        BOOL gen = _tcscmp(text, "yes") == 0;
        CheckDlgButton(hDlg, IDC_GENNORMALS, gen);
        GetAppData(exp->mIp, INDENT_ID, _T("yes"), text, MAX_PATH);
        gen = _tcscmp(text, "yes") == 0;
        CheckDlgButton(hDlg, IDC_INDENT, gen);
        GetAppData(exp->mIp, FIELDS_ID, _T("yes"), text, MAX_PATH);
        gen = _tcscmp(text, "yes") == 0;
        CheckDlgButton(hDlg, IDC_GEN_FIELDS, gen);
        GetAppData(exp->mIp, UPDIR_ID, _T("Y"), text, MAX_PATH);
        gen = _tcscmp(text, "Z") == 0;
        CheckDlgButton(hDlg, IDC_Z_UP, gen);
        CheckDlgButton(hDlg, IDC_Y_UP, !gen);
        GetAppData(exp->mIp, EXPORT_HIDDEN_ID, _T("no"), text, MAX_PATH);
        gen = _tcscmp(text, "yes") == 0;
        CheckDlgButton(hDlg, IDC_EXPORT_HIDDEN, gen);
        GetAppData(exp->mIp, ENABLE_PROGRESS_BAR_ID, _T("yes"), text, MAX_PATH);
        gen = _tcscmp(text, "yes") == 0;
        CheckDlgButton(hDlg, IDC_ENABLE_PROGRESS_BAR, gen);
        GetAppData(exp->mIp, PRIMITIVES_ID, _T("yes"), text, MAX_PATH);
        gen = _tcscmp(text, "yes") == 0;
        CheckDlgButton(hDlg, IDC_PRIM, gen);
        GetAppData(exp->mIp, EXPORT_PRE_LIGHT_ID, _T("no"), text, MAX_PATH);
        gen = _tcscmp(text, "yes") == 0;
        CheckDlgButton(hDlg, IDC_COLOR_PER_VERTEX, gen);
        EnableWindow(GetDlgItem(hDlg, IDC_CPV_CALC), gen);
        EnableWindow(GetDlgItem(hDlg, IDC_CPV_MAX),  gen);

        GetAppData(exp->mIp, CPV_SOURCE_ID, _T("max"), text, MAX_PATH);
        gen = _tcscmp(text, "max") == 0;
        CheckDlgButton(hDlg, IDC_CPV_MAX, gen);
        CheckDlgButton(hDlg, IDC_CPV_CALC, !gen);

        GetAppData(exp->mIp, COORD_SAMPLE_RATE_ID, _T("3"), text, MAX_PATH);
        sampleRate = atoi(text);
        morphPSSpin = GetISpinner(GetDlgItem(hDlg,IDC_MORPHSPSSPINNER));
        morphPSSpin->SetLimits(1, 100, FALSE);
        morphPSSpin->SetValue(sampleRate, FALSE);
        morphPSSpin->SetAutoScale();
        morphPSSpin->LinkToEdit(GetDlgItem(hDlg,IDC_MORPHSPS),
                                EDITTYPE_POS_INT);
        GetAppData(exp->mIp, COORD_INTERP_ID, _T("no"), text, MAX_PATH);
        gen = _tcscmp(text, "yes") == 0;
        CheckDlgButton(hDlg, IDC_COORD_INTERP, gen);
        EnableWindow(GetDlgItem(hDlg, IDC_MORPHSPS), gen);
        EnableWindow(GetDlgItem(hDlg, IDC_MORPHSPSSPINNER), gen);

        // Time to make a list of all the camera's in the scene
        Tab<INode*> cameras, navInfos, backgrounds, fogs;
        exp->GetCameras(exp->GetIP()->GetRootNode(), &cameras, &navInfos,
                        &backgrounds, &fogs);
        int c = cameras.Count();
        for (int i = 0; i < c; i++) {
            // add the name to the list
            TSTR name = cameras[i]->GetName();
            int ind = SendMessage(GetDlgItem(hDlg,IDC_CAMERA_COMBO),
                                  CB_ADDSTRING, 0, (LPARAM)name.data());
            SendMessage(GetDlgItem(hDlg,IDC_CAMERA_COMBO), CB_SETITEMDATA,
                        ind, (LPARAM)cameras[i]);
            
        }
        if (c > 0) {
            TSTR name;
            GetAppData(exp->mIp, CAMERA_ID, _T(""), text, MAX_PATH);
            if (_tcslen(text) == 0)
                name = cameras[0]->GetName();
            else
                name = text;
            // try to set the current selecttion to the current camera
            SendMessage(GetDlgItem(hDlg,IDC_CAMERA_COMBO), CB_SELECTSTRING,
                        0, (LPARAM)name.data());
        }

        c = navInfos.Count();
        for (i = 0; i < c; i++) {
            // add the name to the list
            TSTR name = navInfos[i]->GetName();
            int ind = SendMessage(GetDlgItem(hDlg,IDC_NAV_INFO_COMBO),
                                  CB_ADDSTRING, 0, (LPARAM)name.data());
            SendMessage(GetDlgItem(hDlg,IDC_NAV_INFO_COMBO), CB_SETITEMDATA,
                        ind, (LPARAM)navInfos[i]);
            
        }
        if (c > 0) {
            TSTR name;
            GetAppData(exp->mIp, NAV_INFO_ID, _T(""), text, MAX_PATH);
            if (_tcslen(text) == 0)
                name = navInfos[0]->GetName();
            else
                name = text;
            // try to set the current selecttion to the current camera
            SendMessage(GetDlgItem(hDlg,IDC_NAV_INFO_COMBO), CB_SELECTSTRING,
                        0, (LPARAM)name.data());
        }

        c = backgrounds.Count();
        for (i = 0; i < c; i++) {
            // add the name to the list
            TSTR name = backgrounds[i]->GetName();
            int ind = SendMessage(GetDlgItem(hDlg,IDC_BACKGROUND_COMBO),
                                  CB_ADDSTRING, 0, (LPARAM)name.data());
            SendMessage(GetDlgItem(hDlg,IDC_BACKGROUND_COMBO), CB_SETITEMDATA,
                        ind, (LPARAM)backgrounds[i]);
            
        }
        if (c > 0) {
            TSTR name;
            GetAppData(exp->mIp, BACKGROUND_ID, _T(""), text, MAX_PATH);
            if (_tcslen(text) == 0)
                name = backgrounds[0]->GetName();
            else
                name = text;
            // try to set the current selecttion to the current camera
            SendMessage(GetDlgItem(hDlg,IDC_BACKGROUND_COMBO),
                        CB_SELECTSTRING, 0, (LPARAM)name.data());
        }

        c = fogs.Count();
        for (i = 0; i < c; i++) {
            // add the name to the list
            TSTR name = fogs[i]->GetName();
            int ind = SendMessage(GetDlgItem(hDlg,IDC_FOG_COMBO),
                                  CB_ADDSTRING, 0, (LPARAM)name.data());
            SendMessage(GetDlgItem(hDlg,IDC_FOG_COMBO), CB_SETITEMDATA,
                        ind, (LPARAM)fogs[i]);
            
        }
        if (c > 0) {
            TSTR name;
            GetAppData(exp->mIp, FOG_ID, _T(""), text, MAX_PATH);
            if (_tcslen(text) == 0)
                name = fogs[0]->GetName();
            else
                name = text;
            // try to set the current selecttion to the current camera
            SendMessage(GetDlgItem(hDlg,IDC_FOG_COMBO),
                        CB_SELECTSTRING, 0, (LPARAM)name.data());
        }

        HWND cb = GetDlgItem(hDlg,IDC_OUTPUT_LANG);
        ComboBox_AddString(cb, _T("VRML 1.0"));
        ComboBox_AddString(cb, _T("VRBL"));
        GetAppData(exp->mIp, OUTPUT_LANG_ID, _T("VRBL"), text, MAX_PATH);
        if (_tcscmp(text, _T("VRBL")) == 0)
            type = Export_VRBL;
        else
            type = Export_VRML_1_0;
        EnableControls(hDlg, type);
        ComboBox_SelectString(cb, 0, text);
        GetAppData(exp->mIp, USE_PREFIX_ID, _T("yes"), text, MAX_PATH);
        CheckDlgButton(hDlg, IDC_USE_PREFIX, _tcscmp(text, _T("yes")) == 0);
        GetAppData(exp->mIp, URL_PREFIX_ID, _T("../maps"), text, MAX_PATH);
        Edit_SetText(GetDlgItem(hDlg, IDC_URL_PREFIX), text);
        cb = GetDlgItem(hDlg,IDC_DIGITS);
        ComboBox_AddString(cb, _T("3"));
        ComboBox_AddString(cb, _T("4"));
        ComboBox_AddString(cb, _T("5"));
        ComboBox_AddString(cb, _T("6"));
        GetAppData(exp->mIp, DIGITS_ID, _T("4"), text, MAX_PATH);
        ComboBox_SelectString(cb, 0, text);

        cb = GetDlgItem(hDlg, IDC_POLYGON_TYPE);
        ComboBox_AddString(cb, _T("Triangles"/*JP_LOC*/));
#if TRUE   // outputing higher order polygons
        ComboBox_AddString(cb, _T("Quads"/*JP_LOC*/));
        ComboBox_AddString(cb, _T("Ngons"/*JP_LOC*/));
        ComboBox_AddString(cb, _T("Visible Edges"/*JP_LOC*/));
#endif
        GetAppData(exp->mIp, POLYGON_TYPE_ID, _T("Triangles"/*JP_LOC*/), text, MAX_PATH);
        ComboBox_SelectString(cb, 0, text);

        return TRUE; }
    case WM_COMMAND:
        switch(LOWORD(wParam)) {
        case IDC_OUTPUT_LANG:
            switch(HIWORD(wParam)) {
            case LBN_SELCHANGE:
                ComboBox_GetText(GetDlgItem(hDlg, IDC_OUTPUT_LANG), text,
                                 MAX_PATH);
                if (_tcscmp(text, _T("VRBL")) == 0)
                    type = Export_VRBL;
                else
                    type = Export_VRML_1_0;
                EnableControls(hDlg, type);
                return TRUE;
            }
            break;
        case IDC_ABOUT:
            DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_ABOUT), 
                           GetActiveWindow(), AboutDlgProc, 0);
            break;
        case IDC_EXP_HELP: {
            TCHAR* helpDir = exp->mIp->GetDir(APP_HELP_DIR);
            TCHAR helpFile[MAX_PATH];
            _tcscpy(helpFile, helpDir);
            _tcscat(helpFile, _T("\\vrmlout.hlp"));
            WinHelp(hDlg, helpFile, HELP_CONTENTS, NULL);
            break; }
        case IDC_COLOR_PER_VERTEX: {
            BOOL checked = IsDlgButtonChecked(hDlg, IDC_COLOR_PER_VERTEX);
            EnableWindow(GetDlgItem(hDlg, IDC_CPV_CALC), checked);
            EnableWindow(GetDlgItem(hDlg, IDC_CPV_MAX),  checked);
            break;
            }
        case IDC_COORD_INTERP: {
            BOOL checked = IsDlgButtonChecked(hDlg, IDC_COORD_INTERP);
            EnableWindow(GetDlgItem(hDlg, IDC_MORPHSPS), checked);
            EnableWindow(GetDlgItem(hDlg, IDC_MORPHSPSSPINNER), checked);
            break;
            }
        case IDCANCEL:
            EndDialog(hDlg, FALSE);
            break;
        case IDOK: {
            char tbuf[8];

            exp->SetGenNormals(IsDlgButtonChecked(hDlg, IDC_GENNORMALS));
            WriteAppData(exp->mIp, NORMALS_ID, exp->GetGenNormals() ?
                         _T("yes"): _T("no"));
            
            exp->SetIndent(IsDlgButtonChecked(hDlg, IDC_INDENT));
            WriteAppData(exp->mIp, INDENT_ID, exp->GetIndent() ? _T("yes"):
                         _T("no"));

            exp->SetFields(IsDlgButtonChecked(hDlg, IDC_GEN_FIELDS));
            WriteAppData(exp->mIp, FIELDS_ID, exp->GetFields() ? _T("yes"):
                         _T("no"));

#if 0            
            exp->SetZUp(IsDlgButtonChecked(hDlg, IDC_Z_UP));
#else
            exp->SetZUp(FALSE);
#endif
            WriteAppData(exp->mIp, UPDIR_ID, exp->GetZUp() ? _T("Z"):
                         _T("Y"));

            exp->SetCoordInterp(IsDlgButtonChecked(hDlg, IDC_COORD_INTERP));
            WriteAppData(exp->mIp, COORD_INTERP_ID, exp->GetCoordInterp() ?
                         _T("yes"): _T("no"));

            exp->SetExportHidden(IsDlgButtonChecked(hDlg, IDC_EXPORT_HIDDEN));
            WriteAppData(exp->mIp, EXPORT_HIDDEN_ID, exp->GetExportHidden() ?
                         _T("yes"): _T("no"));

            exp->SetEnableProgressBar(IsDlgButtonChecked(hDlg, IDC_ENABLE_PROGRESS_BAR));
            WriteAppData(exp->mIp, ENABLE_PROGRESS_BAR_ID, exp->GetEnableProgressBar() ?
                         _T("yes"): _T("no"));

            exp->SetPrimitives(IsDlgButtonChecked(hDlg, IDC_PRIM));
            WriteAppData(exp->mIp, PRIMITIVES_ID, exp->GetPrimitives() ?
                         _T("yes"): _T("no"));

            int index = SendMessage(GetDlgItem(hDlg,IDC_CAMERA_COMBO),
                                    CB_GETCURSEL, 0, 0);
            if (index != CB_ERR) {
                exp->SetCamera((INode *)
                               SendMessage(GetDlgItem(hDlg, IDC_CAMERA_COMBO),
                                           CB_GETITEMDATA, (WPARAM)index,
                                           0));
                ComboBox_GetText(GetDlgItem(hDlg, IDC_CAMERA_COMBO),
                                 text, MAX_PATH);
                WriteAppData(exp->mIp, CAMERA_ID, text);
            } else
                exp->SetCamera(NULL);

            index = SendMessage(GetDlgItem(hDlg,IDC_NAV_INFO_COMBO),
                                CB_GETCURSEL, 0, 0);
            if (index != CB_ERR) {
                exp->SetNavInfo((INode *)
                      SendMessage(GetDlgItem(hDlg, IDC_NAV_INFO_COMBO),
                                  CB_GETITEMDATA, (WPARAM)index,
                                  0));
                ComboBox_GetText(GetDlgItem(hDlg, IDC_NAV_INFO_COMBO),
                                 text, MAX_PATH);
                WriteAppData(exp->mIp, NAV_INFO_ID, text);
            } else
                exp->SetNavInfo(NULL);

            index = SendMessage(GetDlgItem(hDlg,IDC_BACKGROUND_COMBO),
                                CB_GETCURSEL, 0, 0);
            if (index != CB_ERR) {
                exp->SetBackground((INode *)
                      SendMessage(GetDlgItem(hDlg, IDC_BACKGROUND_COMBO),
                                  CB_GETITEMDATA, (WPARAM)index,
                                  0));
                ComboBox_GetText(GetDlgItem(hDlg, IDC_BACKGROUND_COMBO),
                                 text, MAX_PATH);
                WriteAppData(exp->mIp, BACKGROUND_ID, text);
            } else
                exp->SetBackground(NULL);

            index = SendMessage(GetDlgItem(hDlg,IDC_FOG_COMBO),
                                CB_GETCURSEL, 0, 0);
            if (index != CB_ERR) {
                exp->SetFog((INode *)
                      SendMessage(GetDlgItem(hDlg, IDC_FOG_COMBO),
                                  CB_GETITEMDATA, (WPARAM)index,
                                  0));
                ComboBox_GetText(GetDlgItem(hDlg, IDC_FOG_COMBO),
                                 text, MAX_PATH);
                WriteAppData(exp->mIp, FOG_ID, text);
            } else
                exp->SetFog(NULL);

            ComboBox_GetText(GetDlgItem(hDlg, IDC_OUTPUT_LANG), text,
                             MAX_PATH);
            WriteAppData(exp->mIp, OUTPUT_LANG_ID, text);
            if (_tcscmp(text, _T("VRBL")) == 0)
                exp->SetExportType(Export_VRBL);
            else
                exp->SetExportType(Export_VRML_1_0);

            ComboBox_GetText(GetDlgItem(hDlg, IDC_POLYGON_TYPE), text, MAX_PATH);
            WriteAppData(exp->mIp, POLYGON_TYPE_ID, text);
            if (_tcscmp(text, _T("Visible Edges"/*JP_LOC*/)) == 0)
                exp->SetPolygonType(OUTPUT_VISIBLE_EDGES);
            else if (_tcscmp(text, _T("Ngons"/*JP_LOC*/)) == 0)
                exp->SetPolygonType(OUTPUT_NGONS);
            else if (_tcscmp(text, _T("Quads"/*JP_LOC*/)) == 0)
                exp->SetPolygonType(OUTPUT_QUADS);
            else
                exp->SetPolygonType(OUTPUT_TRIANGLES);

            exp->SetPreLight(IsDlgButtonChecked(hDlg, IDC_COLOR_PER_VERTEX));
            WriteAppData(exp->mIp, EXPORT_PRE_LIGHT_ID, exp->GetPreLight() ?
                         _T("yes"): _T("no"));

            exp->SetCPVSource(IsDlgButtonChecked(hDlg, IDC_CPV_MAX));
            WriteAppData(exp->mIp, CPV_SOURCE_ID, exp->GetCPVSource() ?
                         _T("max"): _T("calc"));

            exp->SetUsePrefix(IsDlgButtonChecked(hDlg, IDC_USE_PREFIX));
            WriteAppData(exp->mIp, USE_PREFIX_ID, exp->GetUsePrefix()
                         ? _T("yes") : _T("no"));
            Edit_GetText(GetDlgItem(hDlg, IDC_URL_PREFIX), text, MAX_PATH);
            TSTR prefix = text;
            exp->SetUrlPrefix(prefix);
            WriteAppData(exp->mIp, URL_PREFIX_ID, exp->GetUrlPrefix());
            ComboBox_GetText(GetDlgItem(hDlg, IDC_DIGITS), text, MAX_PATH);
            exp->SetDigits(atoi(text));
            WriteAppData(exp->mIp, DIGITS_ID, text);

            GetAppData(exp->mIp, TFORM_SAMPLE_ID, _T("custom"), text,
                       MAX_PATH);
            BOOL once = _tcscmp(text, _T("once")) == 0;
            exp->SetTformSample(once);
            GetAppData(exp->mIp, TFORM_SAMPLE_RATE_ID, _T("10"), text,
                       MAX_PATH);
            int sampleRate = atoi(text);
            exp->SetTformSampleRate(sampleRate);
/*
            GetAppData(exp->mIp, COORD_SAMPLE_ID, _T("custom"), text,
                       MAX_PATH);
            once = _tcscmp(text, _T("once")) == 0;
            exp->SetCoordSample(once);
*/
            morphPSSpin = GetISpinner(GetDlgItem(hDlg,IDC_MORPHSPSSPINNER));
            exp->SetCoordSampleRate(morphPSSpin->GetIVal());
            sprintf(tbuf, "%d", exp->GetCoordSampleRate());
            WriteAppData(exp->mIp, COORD_SAMPLE_RATE_ID, tbuf);
            
            GetAppData(exp->mIp, TITLE_ID, _T(""), text, MAX_PATH);
            exp->SetTitle(text);
            GetAppData(exp->mIp, INFO_ID, _T(""), text, MAX_PATH);
            exp->SetInfo(text);
            EndDialog(hDlg, TRUE);
            break; }
        return TRUE;
        case IDC_SAMPLE_RATES:
            DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_SAMPLE_RATES), 
                           GetActiveWindow(), SampleRatesDlgProc,
                           (LPARAM) exp);
            break;
        case IDC_WORLD_INFO:
            DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_WORLD_INFO), 
                           GetActiveWindow(), WorldInfoDlgProc,
                           (LPARAM) exp);
            break;
        }
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_CONTEXTHELP)
			DoHelp(HELP_CONTEXT, idh_vrmlimp_import);
		break;
    }
    return FALSE;
}

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

void
VRBLExport::initializeDefaults() {
    TCHAR text[MAX_PATH];

    GetAppData(mIp, NORMALS_ID, _T("no"), text, MAX_PATH);
    BOOL gen = _tcscmp(text, "yes") == 0;
    SetGenNormals(gen);
    GetAppData(mIp, INDENT_ID, _T("yes"), text, MAX_PATH);
    gen = _tcscmp(text, "yes") == 0;
    SetIndent(gen);
	GetAppData(mIp, FIELDS_ID, _T("yes"), text, MAX_PATH);
	gen = _tcscmp(text, "yes") == 0;
	SetFields(gen);
    SetZUp(FALSE);
    GetAppData(mIp, COORD_INTERP_ID, _T("no"), text, MAX_PATH);
    gen = _tcscmp(text, "yes") == 0;
    SetCoordInterp(gen);
    GetAppData(mIp, EXPORT_HIDDEN_ID, _T("no"), text, MAX_PATH);
    gen = _tcscmp(text, "yes") == 0;
    SetExportHidden(gen);
    GetAppData(mIp, ENABLE_PROGRESS_BAR_ID, _T("yes"), text, MAX_PATH);
    gen = _tcscmp(text, "yes") == 0;
    SetEnableProgressBar(gen);

    GetAppData(mIp, PRIMITIVES_ID, _T("yes"), text, MAX_PATH);
    gen = _tcscmp(text, "yes") == 0;
    SetPrimitives(gen);

    GetAppData(mIp, EXPORT_PRE_LIGHT_ID, _T("no"), text, MAX_PATH);
    gen = _tcscmp(text, "yes") == 0;
    SetPreLight(gen);
    GetAppData(mIp, CPV_SOURCE_ID, _T("max"), text, MAX_PATH);
    gen = _tcscmp(text, "max") == 0;
    SetCPVSource(gen);

    GetAppData(mIp, USE_PREFIX_ID, _T("yes"), text, MAX_PATH);
    SetUsePrefix(_tcscmp(text, _T("yes")) == 0);
    GetAppData(mIp, URL_PREFIX_ID, _T("../maps"), text, MAX_PATH);
    TSTR prefix = text;
    SetUrlPrefix(prefix);
    GetAppData(mIp, DIGITS_ID, _T("4"), text, MAX_PATH);
    SetDigits(atoi(text));

    GetAppData(mIp, POLYGON_TYPE_ID, _T("Triangles"), text, MAX_PATH);
    if (_tcscmp(text, _T("Visible Edges"/*JP_LOC*/)) == 0)
        SetPolygonType(OUTPUT_VISIBLE_EDGES);
    else if (_tcscmp(text, _T("Ngons"/*JP_LOC*/)) == 0)
        SetPolygonType(OUTPUT_NGONS);
    else if (_tcscmp(text, _T("Quads"/*JP_LOC*/)) == 0)
        SetPolygonType(OUTPUT_QUADS);
    else
        SetPolygonType(OUTPUT_TRIANGLES);

    Tab<INode*> cameras, navInfos, backgrounds, fogs;
    GetCameras(GetIP()->GetRootNode(), &cameras, &navInfos,
                    &backgrounds, &fogs);
    int c = cameras.Count();
    int ci;
    INode *inode = NULL;
    if (c > 0) {
        TSTR name;
        GetAppData(mIp, CAMERA_ID, _T(""), text, MAX_PATH);
        if (_tcslen(text) == 0)
            inode = cameras[0];
        else {
            name = text;
            for (ci = 0; ci < c; ci++)
                if (_tcscmp(cameras[ci]->GetName(), name) == 0) {
                    inode = cameras[ci];
                    break;
                }
        }
    }
    SetCamera(inode);

    c = navInfos.Count();
    inode = NULL;
    if (c > 0) {
        TSTR name;
        GetAppData(mIp, NAV_INFO_ID, _T(""), text, MAX_PATH);
        if (_tcslen(text) == 0)
            inode = navInfos[0];
        else {
            name = text;
            for (ci = 0; ci < c; ci++)
                if (_tcscmp(navInfos[ci]->GetName(), name) == 0) {
                    inode = navInfos[ci];
                    break;
                }
        }
    }
    SetNavInfo(inode);

    c = backgrounds.Count();
    inode = NULL;
    if (c > 0) {
        TSTR name;
        GetAppData(mIp, BACKGROUND_ID, _T(""), text, MAX_PATH);
        if (_tcslen(text) == 0)
            inode = backgrounds[0];
        else {
            name = text;
            for (ci = 0; ci < c; ci++)
                if (_tcscmp(backgrounds[ci]->GetName(), name) == 0) {
                    inode = backgrounds[ci];
                    break;
                }
        }
    }
    SetBackground(inode);

    c = fogs.Count();
    inode = NULL;
    if (c > 0) {
        TSTR name;
        GetAppData(mIp, FOG_ID, _T(""), text, MAX_PATH);
        if (_tcslen(text) == 0)
            inode = fogs[0];
        else {
            name = text;
            for (ci = 0; ci < c; ci++)
                if (_tcscmp(fogs[ci]->GetName(), name) == 0) {
                    inode = fogs[ci];
                    break;
                }
        }
    }
    SetFog(inode);

    GetAppData(mIp, TFORM_SAMPLE_ID, _T("custom"), text, MAX_PATH);
    BOOL once = _tcscmp(text, _T("once")) == 0;
    SetTformSample(once);
    GetAppData(mIp, TFORM_SAMPLE_RATE_ID, _T("10"), text, MAX_PATH);
    SetTformSampleRate(atoi(text));

/*
    GetAppData(mIp, COORD_SAMPLE_ID, _T("custom"), text, MAX_PATH);
    once = _tcscmp(text, _T("once")) == 0;
    SetCoordSample(once);
*/
    GetAppData(mIp, COORD_SAMPLE_RATE_ID, _T("3"), text, MAX_PATH);
    SetCoordSampleRate(atoi(text));

    GetAppData(mIp, TITLE_ID, _T(""), text, MAX_PATH);
    SetTitle(text);
    GetAppData(mIp, INFO_ID, _T(""), text, MAX_PATH);
    SetInfo(text);

    SetExportType(Export_VRML_2_0);
}


// Export the current scene as VRML
int
VRBLExport::DoExport(const TCHAR *filename, ExpInterface *ei, Interface *i,
                     int nodialog, DWORD options) 
{
    mIp = i;
    mStart = mIp->GetAnimRange().Start();
    
    if (nodialog)
        initializeDefaults();
    else if (!DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_VRBLEXP), 
                        GetActiveWindow(), VrblExportDlgProc,
                        (LPARAM) this))
        return TRUE;
	
/*
    if (IsVRML2()) {
        VRML2Export vrml2;
        int val = vrml2.DoExport(filename, i, this);
        return val;

    }
*/

    WorkFile theFile(filename, _T("w"));
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
    fprintf(mStream, _T("#VRML V1.0 ascii\n\n"));
	
	// generate a hash table of unique node names
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

    // Write out the scene graph
    VrblOutNode(mIp->GetRootNode(), NULL, 0, FALSE, TRUE, identMat, NULL);

    delete mLodList;


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

VRBLExport::VRBLExport() 
{
    mGenNormals = FALSE;
    mHadAnim = FALSE;
    mLodList = NULL;
    mTformSample = FALSE;
    mTformSampleRate = 10;
    mCoordSample = FALSE;
    mCoordSampleRate = 3;
    mCondenseTM = TRUE;
}

VRBLExport::~VRBLExport() {
}

// Number of file extensions supported by the exporter
int
VRBLExport::ExtCount() {
    return 1;
}

// The exension supported
const TCHAR *
VRBLExport::Ext(int n) {
    switch(n) {
    case 0:
        return _T("WRL");
    }
    return _T("");
}

const TCHAR *
VRBLExport::LongDesc() {
    return _T("Autodesk VRBL");
}
	
const TCHAR *
VRBLExport::ShortDesc() {
    return _T("VRML 1.0/VRBL");
}

const TCHAR *
VRBLExport::AuthorName() {
    return _T("Scott Morrison");
}

const TCHAR *
VRBLExport::CopyrightMessage() {
    return _T("Copyright 1996, Autodesk, Inc.");
}

const TCHAR *
VRBLExport::OtherMessage1() {
    return _T("");
}

const TCHAR *
VRBLExport::OtherMessage2() {
    return _T("");
}

unsigned int
VRBLExport::Version() {
    return 100;
}

void
VRBLExport::ShowAbout(HWND hWnd) {
}

static DWORD HashNode(DWORD o, int size)
{
    DWORD code = (DWORD) o;
    return (code >> 2) % size;
}

// form the hash value for string s
static unsigned HashName(TCHAR* s, int size)
{
	unsigned hashVal;
	for (hashVal = 0; *s; s++)
		hashVal = *s + 31 * hashVal;
    return hashVal % size;
}

// Node Hash table lookup
NodeList* NodeTable::AddNode(INode* node)
{
    DWORD hash = HashNode((DWORD) node, NODE_HASH_TABLE_SIZE);
    NodeList* nList;
    
    for(nList = mTable[hash]; nList; nList = nList->next) {
        if (nList->node == node) {
            return nList;
        }
    }
    nList = new NodeList(node);
    nList->next = mTable[hash];
    mTable[hash] = nList;
    return nList;
}

// Node Name lookup
TCHAR* NodeTable::GetNodeName(INode* node)
{
    DWORD hash = HashNode((DWORD) node, NODE_HASH_TABLE_SIZE);
    NodeList* nList;
    
    for(nList = mTable[hash]; nList; nList = nList->next) {
        if (nList->node == node) {
            if (nList->hasName) return nList->name;
			else return NULL;	// if it wasn't created
        }
    }
    return NULL;	// if for some unknown reason we dont find it
}

// Node unique name list lookup
TCHAR* NodeTable::AddName(TCHAR* name)
{
	unsigned  hashVal = HashName(name, NODE_HASH_TABLE_SIZE);
	NameList* nList;
	NameList  newName(NULL);
	TCHAR     buf[256];
	TCHAR*    matchStr;
	int		  matchVal;

	for (nList = mNames[hashVal]; nList; nList = nList->next) {
		if (nList->name && !strcmp(name, nList->name)) { // found a match
		 // checkout name for "_0xxx" that is our tag
			matchStr = strrchr(name, '_');
			if (matchStr) {		// possible additional duplicate names
				if (matchStr[1] == '0') {		// assume additional duplicate names
					matchVal = atoi(matchStr + 1);	// get number
					strncpy(buf, name, strlen(name) - strlen(matchStr)); // first part
					buf[strlen(name) - strlen(matchStr)] = '\0';	// terminate
					sprintf(newName.name, "%s_0%d", buf, matchVal+1);	// add one
					return AddName(newName.name); // check for unique new name
				}
			}
			sprintf(newName.name, "%s_0", name);	// first duplicate name
			return AddName(newName.name); // check for unique new name
		}
	}
	nList = new NameList(name);
	nList->next = mNames[hashVal];
	mNames[hashVal] = nList;
	return nList->name;
}

// Traverse the scene graph generating Unique Node Names
void 
VRBLExport::GenerateUniqueNodeNames(INode* node)
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
reduceAngAxisKeys(Tab<TimeValue>& times, Tab<AngAxis>& points, float eps,
				  BOOL cumu)		// cumu TRUE if angles are cumulative
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
		if (cumu)
		{
			if (ApproxEqual(points[i].angle, 0.0F, eps))
				used[i] = FALSE;
			else
				used[i-1] = TRUE;
		}
		else
		{
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
