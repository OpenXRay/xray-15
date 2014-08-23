/**********************************************************************
 *<
	FILE: vrml2.h

	DESCRIPTION:  VRML 2.0 file export class defs

	CREATED BY: Scott Morrison

	HISTORY: created June, 1996

 *>	Copyright (c) 1996, All Rights Reserved.
 **********************************************************************/

#define RUN_BY_PROX_SENSOR 1
#define RUN_BY_TIME_SENSOR 2
#define RUN_BY_TOUCH_SENSOR 4
#define RUN_BY_ANY_SENSOR 7

struct AnimRoute {
    AnimRoute() { mToNode = NULL; }
    AnimRoute(TCHAR* from, TCHAR* to, INode* fromNode, INode* node)
    {
        mFromName = from;
        mToName   = to;
        mToNode   = node;
        mFromNode = fromNode;
    }
    TSTR        mFromName;      // route nodes from animate trigger
    TSTR        mToName;        // route anim trigger to
    INode*      mToNode;        // INode for route to
    INode*      mFromNode;      // INode for route from
};

struct InterpRoute {
    InterpRoute() {mType = 0;}
    InterpRoute(TCHAR* interp, int type, TCHAR* node)
    {
        mInterp = interp;
        mType = type;
        mNode = node;
    }

    TSTR mInterp;
    int  mType;
    TSTR mNode;
};

// Object hash table for instancing

struct ObjectBucket {
    ObjectBucket(Object* o)
    {
        obj = o;
        objectUsed = FALSE;
        hasName = FALSE;
        hasInstName = FALSE;
        next = NULL;
    }
    ~ObjectBucket() {delete next;}
    Object *obj;
    BOOL    objectUsed;
    BOOL    hasName;
    BOOL    hasInstName;
    TSTR    name;
    TSTR    instName;
    ObjectBucket *next;
};

#define OBJECT_HASH_TABLE_SIZE 1001

class ObjectHashTable {
  public:

    ObjectHashTable() {
        mTable.SetCount(OBJECT_HASH_TABLE_SIZE);
        for(int i = 0; i < OBJECT_HASH_TABLE_SIZE; i++)
            mTable[i] = NULL;
    }
    ~ObjectHashTable() {
        for(int i = 0; i < OBJECT_HASH_TABLE_SIZE; i++)
            delete mTable[i];
    }
        

    ObjectBucket* AddObject(Object* obj);

  private:

    Tab<ObjectBucket*> mTable;
};

struct SensorBucket {
    SensorBucket(INode * node)
    {
        mNode = node;
        mSensors = NULL;
        mNext = NULL;
    }
    ~SensorBucket()
    {
        delete mSensors;
    }
    INode* mNode;
    INodeList* mSensors;
    SensorBucket* mNext;
};

#define SENSOR_HASH_TABLE_SIZE 97

class SensorHashTable {
  public:

    SensorHashTable() {
        mTable.SetCount(SENSOR_HASH_TABLE_SIZE);
        for(int i = 0; i < SENSOR_HASH_TABLE_SIZE; i++)
            mTable[i] = NULL;
    }
    ~SensorHashTable() {
        for(int i = 0; i < SENSOR_HASH_TABLE_SIZE; i++)
            delete mTable[i];
    }
        

    void AddSensor(INode* node, INode* sensor);
    SensorBucket* FindSensor(INode* node);

  private:

    Tab<SensorBucket*> mTable;
};


class VRML2Export {
public:
    VRML2Export();
    ~VRML2Export();
    
    int   DoExport(const TCHAR *name, Interface *i, VRBLExport* exp);
#ifdef _LEC_
    int   DoFBExport(const TCHAR *name, Interface *i, VRBLExport* exp, int frame, TimeValue time);
#endif

    inline BOOL GetGenNormals() { return mGenNormals; }
    inline void SetGenNormals(BOOL gen) { mGenNormals = gen; }
    inline BOOL GetIndent() { return mIndent; }
    inline void SetIndent(BOOL in) { mIndent = in; }
    inline ExportType GetExportType() { return mType; }
    inline void SetExportType(ExportType t) { mType = t; }
    inline Interface* GetIP() { return mIp;}
    inline INode* GetCamera() { return mCamera; }
    inline void SetCamera(INode* cam) { mCamera = cam; }
    inline void SetUsePrefix(BOOL u) { mUsePrefix = u; }
    inline BOOL GetUsePrefix() { return mUsePrefix; }
    inline void SetUrlPrefix(TSTR& s) { mUrlPrefix = s; }
    inline TSTR& GetUrlPrefix() {return mUrlPrefix; }
    inline void SetFields(BOOL f) { mGenFields = f; }
    inline BOOL GetFields() { return mGenFields; }
    inline BOOL IsVRML2() { return TRUE; }
    inline BOOL GetZUp() { return mZUp; }
    inline void SetZUp(BOOL zup) {mZUp = zup; }
    inline int GetDigits() { return mDigits; }
    inline void SetDigits(int n) { mDigits = n; }
    inline BOOL GetCoordInterp() { return mCoordInterp; }
#ifdef _LEC_
    inline BOOL GetFlipBook() { return mFlipBook; }
#endif
    inline void SetCoordInterp(BOOL ci) { mZUp = ci; }
    inline BOOL GetPreLight() { return mPreLight; }
    inline void SetPreLight(BOOL i) { mPreLight = i; }
//    inline void SetPolygonType(int type)    { mPolygonType = type; }

    inline BOOL GetTformSample() { return mTformSample; }
    inline void SetTformSample(BOOL b) { mTformSample = b; }
    inline int GetTformSampleRate() { return mTformSampleRate; }
    inline void SetTformSampleRate(int rate) { mTformSampleRate = rate; }

    inline BOOL GetCoordSample() { return mCoordSample; }
    inline void SetCoordSample(BOOL b) { mCoordSample = b; }
    inline int GetCoordSampleRate() { return mCoordSampleRate; }
    inline void SetCoordSampleRate(int rate) { mCoordSampleRate = rate; }
    inline TSTR& GetInfo() { return mInfo; }
    inline void SetInfo (TCHAR* info) { mInfo = info; }
    inline TSTR& GetTitle() { return mInfo; }
    inline void SetTitle (TCHAR* title) { mTitle = title; }
    inline BOOL GetExportHidden() { return mExportHidden; }
    inline void SetExportHidden(BOOL eh) { mExportHidden = eh; }
    
    Interface* mIp;         // MAX interface pointer

private:
    TCHAR* point(Point3& p);
    TCHAR* scalePoint(Point3& p);
    TCHAR* normPoint(Point3& p);
    TCHAR* axisPoint(Point3& p, float ang);
    TCHAR* texture(UVVert& uv);
    TCHAR* color(Color& c);
    TCHAR* color(Point3& c);
    TCHAR* floatVal(float f);

    // VRML Output routines
    void Indent(int level);
    int  MaybeNewLine(int width, int level);
    void StartNode(INode* node, int level, BOOL outputName);
    void EndNode(INode* node, Object* obj, int level, BOOL lastChild);
    BOOL IsBBoxTrigger(INode* node);
    BOOL OutputNodeTransform(INode* node, int level, BOOL mirrored);
    void OutputMultiMtl(Mtl* mtl, int level);
    BOOL OutputMaterial(INode* node, BOOL& isWire, BOOL& twoSided, int level,
                        int textureNum);
    BOOL HasTexture(INode *node, BOOL& isWire);
    TSTR PrefixUrl(TSTR& fileName);
    TextureDesc* GetMtlTex(Mtl* mtl, BOOL &isWire);
    TextureDesc*GetMatTex(INode* node, BOOL& isWire);
    void OutputNormalIndices(Mesh& mesh, NormalTable* normTab, int level,
                             int textureNum);
    NormalTable* OutputNormals(Mesh& mesh, int level);
    void OutputTriObject(INode* node, TriObject* obj, BOOL multiMat,
                         BOOL isWire, BOOL twoSided, int level,
                         int textureNum, BOOL pMirror);
    void OutputPolygonObject(INode* node, TriObject* obj, BOOL multiMat,
             BOOL isWire, BOOL twoSided, int level, int textureNum,
             BOOL pMirror);
    BOOL isVrmlObject(INode * node, Object *obj, INode* parent);
    BOOL ChildIsAnimated(INode* node);
    BOOL ObjIsAnimated(Object *obj);
    BOOL ObjIsPrim(INode* node, Object* obj);
    void VrmlOutObject(INode* node, INode* parent, Object* obj, int level,
                       BOOL mirrored);
    BOOL VrmlOutSphereTest(INode * node, Object *obj);
    BOOL VrmlOutSphere(INode * node, Object *obj, int level);
    BOOL VrmlOutCylinder(INode* node, Object *obj, int level);
    BOOL VrmlOutCone(INode* node, Object *obj, int level);
    BOOL VrmlOutCube(INode* node, Object *obj, int level);
    BOOL VrmlOutCamera(INode* node, Object* obj, int level);
    BOOL VrmlOutSound(INode* node, SoundObject* obj, int level);
    BOOL VrmlOutTouchSensor(INode* node, int level);
    BOOL VrmlOutProxSensor(INode* node, ProxSensorObject* obj, int level);
    BOOL VrmlOutBillboard(INode* node, Object* obj, int level);
    void VrmlOutTimeSensor(INode* node, TimeSensorObject* obj, int level);
    //void VrmlAnchorHeader(INode* node, MrBlueObject* obj,
    //                      VRBL_TriggerType type, BOOL fromParent, int level);
    //BOOL VrmlOutMrBlue(INode* node, INode* parent, MrBlueObject* obj,
    //                   int* level, BOOL fromParent);
    BOOL VrmlOutInline(VRMLInsObject* obj, int level);
    BOOL VrmlOutLOD(INode* node, LODObject* obj, int level, BOOL mirrored);
    void VrmlOutCoordinateInterpolator(INode* node, Object *obj, int level,
                                       BOOL pMirror);
    BOOL VrmlOutCylinderTest(INode* node, Object *obj);
    BOOL VrmlOutCylinderTform(INode* node, Object *obj, int level,
                              BOOL mirrored);
    BOOL VrmlOutConeTest(INode* node, Object *obj);
    BOOL VrmlOutConeTform(INode* node, Object *obj, int level, BOOL mirrored);
    BOOL VrmlOutCubeTest(INode* node, Object *obj);
    BOOL VrmlOutCubeTform(INode* node, Object *obj, int level, BOOL mirrored);
    BOOL VrmlOutSpecialTform(INode* node, Object* obj, int level,
                             BOOL mirrored);
    BOOL VrmlOutSpecial(INode* node, INode* parent, Object* obj, int level,
                        BOOL mirrored);
    //int StartMrBlueHelpers(INode* node, int level);
    //void EndMrBlueNode(INode* childNode, int& level, BOOL fromParent);
    //void EndMrBlueHelpers(INode* node, int level);
    BOOL VrmlOutPointLight(INode* node, LightObject* light, int level);
    BOOL VrmlOutDirectLight(INode* node, LightObject* light, int level);
    BOOL VrmlOutSpotLight(INode* node, LightObject* light, int level);
    BOOL VrmlOutTopPointLight(INode* node, LightObject* light);
    BOOL VrmlOutTopDirectLight(INode* node, LightObject* light);
    BOOL VrmlOutTopSpotLight(INode* node, LightObject* light);
    void OutputTopLevelLight(INode* node, LightObject *light);
    void WriteControllerData(INode* node,
                             Tab<TimeValue>& posTimes, Tab<Point3>& posKeys,
                             Tab<TimeValue>& rotTimes, Tab<AngAxis>& rotKeys,
                             Tab<TimeValue>& sclTImes, Tab<ScaleValue>& sclKeys,
                             int type, int level);
    void WriteAllControllerData(INode* node, int type, int level,
                                Control* lc);

    void WriteVisibilityData(INode* node, int level);
    BOOL IsLight(INode* node);
    BOOL IsCamera(INode* node);
	BOOL IsAudio(INode* node);
    Control* GetLightColorControl(INode* node);
    void VrmlOutControllers(INode* node, int level);
    void ScanSceneGraph1();
    void ScanSceneGraph2();
    void ComputeWorldBoundBox(INode* node, ViewExp* vpt);
    void OutputTouchSensors(INode* node, int level);
    void TraverseNode(INode* node);
    BOOL ObjectIsLODRef(INode* node);
    void VrmlOutTopLevelCamera(int level, INode* node, BOOL topLevel);
    void VrmlOutTopLevelNavInfo(int level, INode* node, BOOL topLevel);
    void VrmlOutTopLevelBackground(int level, INode* node, BOOL topLevel);
    void VrmlOutTopLevelFog(int level, INode* node, BOOL topLevel);
    void VrmlOutInitializeAudioClip(int level, INode* node);
    void VrmlOutAudioClip(int level, INode* node);
    void VrmlOutFileInfo();
    void VrmlOutWorldInfo();
    void VrmlOutGridHelpers(INode*);

    int  StartAnchor(INode* node, int& level);
    void VrmlOutNode(INode* node, INode* parent, int level, BOOL isLOD,
                     BOOL lastChild, BOOL mirrored);
    void InitInterpolators(INode* node);
    void AddInterpolator(TCHAR* interp, int type, TCHAR *name);
    void WriteInterpolatorRoutes(int level, BOOL isCamera);
    void AddCameraAnimRoutes(TCHAR* vrmlObjName, INode* fromNode, INode* top);
    void AddAnimRoute(TCHAR* from, TCHAR* to, INode* fromNode, INode* node);
    int  NodeNeedsTimeSensor(INode* node);
    void WriteAnimRoutes();
    TCHAR* VrmlParent(INode* node);
    BOOL IsAimTarget(INode* node);
    void GenerateUniqueNodeNames(INode* node);

    FILE*          mStream;     // The file mStream to write
    TCHAR*         mFilename;   // The export .WRL filename
    BOOL           mGenNormals; // Generate normals in the VRML file
    BOOL           mIndent;     // Should we indent?
    INodeList*     mLodList;    // List of LOD objects in the scene
    INodeList*     mTimerList;  // List of TimeSensor Nodes in the scene
    ExportType     mType;       // Language to export (VRML, VRML, ...)
    INode*         mCamera;     // Initial camera;
    INode*         mNavInfo;    // Initial Navigation Info;
    INode*         mBackground; // Initial Background node
    INode*         mFog;        // Initial Fog node
    BOOL           mUsePrefix;  // Use URL Prefix
    TSTR           mUrlPrefix;  // The URL prefix
    BOOL           mGenFields;  // Generate "fields" statements
    BOOL           mHadAnim;    // File has animation data
    TimeValue      mStart;      // First frame of the animation
    TSTR           mTimer;      // Name of active TimeSensor
    Tab<TSTR>      mInterps;    // Interpolators that need ROUTE statements
    Tab<int>       mInterpTypes;// Type of interpolator nodes
    Tab<TSTR>      mInterpNodes;// Nodes for interpolators
    float          mCycleInterval; // Length of animation in seconds
    Tab<InterpRoute> mInterpRoutes; // Routes for Intpolator nodes
    Tab<AnimRoute> mAnimRoutes;  // route nodes from anim
    BOOL           mZUp;        // Z axis if true, Y axis otherwise
    int            mDigits;     // Digits of precision on output
    BOOL           mCoordInterp;// Generate coordinate interpolators
#ifdef _LEC_
    BOOL           mFlipBook;   // Generate one VRML file per frame (LEC request)
#endif
    BOOL           mTformSample;// TRUE for once per frame
    int            mTformSampleRate; // Custom sample rate
    BOOL           mCoordSample; // TRUE for once per frame
    int            mCoordSampleRate; // Custom sample rate
    ObjectHashTable mObjTable;    // Hash table of all objects in the scene
    SensorHashTable mSensorTable; // Hash table of all TouchSensor and Anchors
    Box3           mBoundBox;     // Bounding box for the whole scene
    TSTR           mTitle;        // Title of world
    TSTR           mInfo;         // Info for world
    BOOL           mExportHidden; // Export hidden objects
    BOOL           mPrimitives;   // Create VRML primitves
    BOOL           mHasLights;    // TRUE iff scene has lights
    BOOL           mHasNavInfo;   // TRUE iff scene has NavigationInfo
    int             mPolygonType;   // 0 triangles, 1 quads, 2 ngons
    NodeTable       mNodes;         // hash table of all nodes name in the scene
    BOOL            mEnableProgressBar;      // this is used by the progress bar
    BOOL            mPreLight;      // should we calculate the color per vertex
    BOOL            mCPVSource;     // 1 if MAX's; 0 if should we need to calculate the color per vertex
    CallbackTable*  mCallbacks;     // export callback methods
};

float GetLosProxDist(INode* node, TimeValue t);
Point3 GetLosVector(INode* node, TimeValue t);

int reduceAngAxisKeys(Tab<TimeValue>& times, Tab<AngAxis>& points, float eps);
int reducePoint3Keys(Tab<TimeValue>& times, Tab<Point3>& points, float eps);
int reduceScaleValueKeys(Tab<TimeValue>& times, Tab<ScaleValue>& points,
                         float eps);
void CommaScan(TCHAR* buf);


