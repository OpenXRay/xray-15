
class BGContext; // Fwd decl of background context

// ShadeContext for evaluating materials
// See cjshade.cpp for details about ShadeContext
class SContext : public ShadeContext {
public:
	SContext(CJRenderer* r, BGContext* bgc);
	void SetBary(Point3 bary);

	TimeValue CurTime();
	int NodeID();
	INode* Node();
	Point3 BarycentricCoords();
	int FaceNumber();
	Point3 Normal();
	float Curve();

	LightDesc *Light(int lightNo);
	Point3 GNormal(void);
	Point3 ReflectVector(void);
	Point3 RefractVector(float ior);
	Point3 CamPos(void);
	Point3 V(void);
	Point3 P(void);
	Point3 DP(void);
	Point3 PObj(void);
	Point3 DPObj(void);
	Box3 ObjectBox(void);
	Point3 PObjRelBox(void);
	Point3 DPObjRelBox(void);
	void ScreenUV(Point2 &uv,Point2 &duv);
	IPoint2 ScreenCoord(void);
	Point3 UVW(int channel);
	Point3 DUVW(int channel);
	void DPdUVW(Point3 [], int channel);
	void GetBGColor(Color &bgCol, Color &transp, int fogBG);
	Point3 PointTo(const Point3 &p, RefFrame ito);
	Point3 PointFrom(const Point3 &p, RefFrame ito);
	Point3 VectorTo(const Point3 &p, RefFrame ito);
	Point3 VectorFrom(const Point3 &p, RefFrame ito);

	int		ProjType();
	void	SetInstance(Instance* instance);
	void	SetFaceNum(int f);
	void	SetScreenPos(IPoint2 pt);
	void	SetMtlNum(int mNo);
	void	SetHitPos(Point3 p);
	void	SetViewDir(Point3 v);
	void	CalcNormals();
	void	SetCamPos(Point3 p) { cameraPos = p; }

	BGContext* bc;
	void	getTVerts(int chan);
	void	calc_size_ratio();
	void	calc_dobpos();
	void	calc_derivs();

	int InMtlEditor() { return FALSE; /* TBD*/ }
	void SetView(Point3 p) { /* TBD*/ }

private:
	CJRenderer* renderer;
	Instance*	pInst;
	Point3		baryCoord;
	int			faceNum;
	IPoint2		screenPos;
	Point3		hitPos;
	Point3		viewDir;
	Point3		vxNormals[3];
	UVVert		tv[MAX_MESHMAPS][3];
	Point3		bumpv[MAX_MESHMAPS][3];
	Point3		dp;
	Point3		uvw[MAX_MESHMAPS];
	Point3		duvw[MAX_MESHMAPS];
	Point3		cameraPos;
	Point3		obpos[3];
	Point3		dobpos;

	// Derivatives of barycentric coords
	float		dsdx,dsdy,dtdx,dtdy;
	float		origDsdx,origDsdy,origDtdx,origDtdy;

	ULONG		matreq;
	float		ratio;
	float		curve;
};

//***************************************************************************
//* This is the ShadeContext implementation for backgrounds
//***************************************************************************

#define FARZ -1.0e10f

 class BGContext: public ShadeContext {
	public:
		Point3 viewDir;
		Point3 cpos;
		IPoint2 iscr;
		Point2 scrPos;

		BGContext(CJRenderParams *rpar);
		
		Point3 BGGetPoint(float z = FARZ);
	   	TimeValue CurTime()  { return globContext->time; }     	// current time value
		int FaceNumber() { return 0; }
		LightDesc* Light(int n) { return NULL; }
		int Antialias() { return 0; }
		int NodeID() { return 0; }
		int ProjType() { return globContext->projType;}  // returns: 0: perspective, 1: parallel
		Point3 Normal() { return -viewDir; }
		Point3 GNormal() { return -viewDir;	}
		void SetNormal(Point3 p) {}
		float Curve() { return (float)fabs(1.0f/globContext->xscale);	}
		Point3 ReflectVector() { return Point3(0,0,0); }	// reflection vector
		Point3 RefractVector(float ior) { return Point3(0,0,0); } // refraction vector
	    Point3 CamPos() { return cpos; }			// camera position
		Point3 P() { return viewDir*100.0f; }
		Point3 V() { return viewDir; }
		Point3 DP() { return Point3(0,0,0); }
		void DP(Point3& dpdx, Point3& dpdy) {}
		Point3 PObj() {
			// this makes procedural textures come out a reasonable scale.
			return 100.0f*VectorTransform(globContext->camToWorld,viewDir); 
			}
		Point3 DPObj() { return Point3(0,0,0); }
		Box3 ObjectBox() {return Box3(); } 
		Point3 PObjRelBox() {
			return 100.0f*VectorTransform(globContext->camToWorld,viewDir); 
			}
		Point3 DPObjRelBox() { return Point3(0,0,0); }

	   	void ScreenUV(Point2& uv, Point2 &duv);  // screen coordinate
		IPoint2 ScreenCoord() { return iscr; }		// integer

		Point3 UVW(int channel) { return 100.0f*viewDir; }
		Point3 DUVW(int channel) {	return Point3(0,0,0); }
		void DPdUVW(Point3 dP[3], int channel) { }
		AColor EvalEnvironMap(Texmap *map, Point3 view) {
			Point3 svView = viewDir; 
			viewDir = view;
			AColor rcol = map->EvalColor(*this);
			viewDir = svView;
			return rcol;
			}
		void GetBGColor(Color &bgcol, Color& transp, BOOL fogBG=TRUE) {;}
		Point3 PointFrom(const Point3& p, RefFrame ifrom); 
		Point3 PointTo(const Point3& p, RefFrame ito); 
		Point3 VectorFrom(const Point3& p, RefFrame ifrom);
		Point3 VectorTo(const Point3& p, RefFrame ito); 
		float CamNearRange() {return globContext->nearRange;}
		float CamFarRange() {return globContext->farRange;}

		void SetScreenPos(int x, int y, int width, int height);
		void SetViewDir(Point3 vd) { viewDir = vd; }
		void SetCamPos(Point3 cp)  { cpos = cp; }


// TBD Athena
		int InMtlEditor() { return FALSE; /* TBD*/ }
		void SetView(Point3 p) { /* TBD*/ }

	private:
		Point2		scrDUV;
};

static inline float size_meas(Point3 a, Point3 b, Point3 c) {
	double d  = fabs(b.x-a.x);
	d += fabs(b.y-a.y);
	d += fabs(b.z-a.z);
	d += fabs(c.x-a.x);
	d += fabs(c.y-a.y);
	d += fabs(c.z-a.z);
	return float(d/6.0);
}

