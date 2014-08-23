/**********************************************************************************
 *
 *	MKHEDRA.C	 	Creates Regular Polyhedra.
 *				Adapted from Dan Silva's HEDRA.C
 *
 *	Revision History
 *	----------------
 *	03-14-94  RWB  Created this file.
 *  10-14-95  RWB  Adapted to MAX
 *
 **********************************************************************************/
#include "prim.h"


typedef struct {
	int sclass;			
	float fp;				
	float fq;				
	float radius;			
	int axis;				
	char pmtl[17];
	char qmtl[17];
	char rmtl[17];
	int verts;			 
	float scale_axis[3];	
} State;

static State state;


static int MakeHedron(int np, int dp, int nq, int dq, double cp, double cq);
static void TesselatePolys(void);

typedef struct { double x,y,z; } DPoint;

#define CP(i,j)  (a[i]*b[j]-a[j]*b[i])

#define R_OP 0
#define P_OP 1
#define Q_OP 2

typedef struct {
	DPoint p[3];	
	DPoint v;
	} STri;

typedef struct {
	int num;
	DPoint n[30];
	STri t[30]; /* starter face */
	} AxisList;

#define MAXVERTS 380
#define MAXFACES 720
static AxisList axis[3]={0};
static DPoint verts[MAXVERTS];
static short imtl[3]={0,1,2};
static int nverts = 0;
static int nfaces = 0;
typedef struct {
	short nax[3];
	} TriReg;

#define MAXREGTRIS 120
static TriReg regtris[MAXREGTRIS];
static int nregtris = 0;
static int nsides[3];

/*----------------------------------------------------*/

typedef struct
{
unsigned short a,b,c;	/* Vertex numbers for triangular face */
unsigned long sm_group;	/* Smoothing group bits (32) */
unsigned char flags;	/* See below for meanings */
unsigned char material;	/* 0-255 */
} FData;

#define FC_ABLINE 1
#define FC_BCLINE 2
#define FC_CALINE 4

static FData fdata[MAXFACES];

/* Classes */

#define  CL_TETRA 0
#define  CL_CUBOCT 1
#define  CL_ICOSDODEC 2
#define  CL_ICOSDODEC2 3
#define  CL_DDODEC2 4

static short pnum[5] = {3,3,3,3,5};
static short pden[5] = {1,1,1,1,1};
static short qnum[5] = {3,4,5,5,5};
static short qden[5] = {1,1,1,2,2};



/*----------------------------------------------------*/

static void DCrossProd(double *p, double *a, double *b) {
	p[0] = CP(1,2); 	
	p[1] = CP(2,0);
	p[2] = CP(0,1);
	}

static double DDotProd(double *a, double *b) {
	return( a[0]*b[0]+a[1]*b[1]+a[2]*b[2]);
	}

static void NormVect(double *n) {
	double s,d;
	s = sqrt(n[0]*n[0] + n[1]*n[1] + n[2]*n[2]);
	if (s>0.0) {
		d = 1/s;
		n[0] *= d;	n[1] *= d; n[2] *= d;
		}
	else { n[0] = 1.0; n[1] = 0.0; n[2] = 0.0; }
	}

static int same_pt(DPoint *a, DPoint *b) {
	if (fabs(a->x-b->x)>.01) return(0);
	if (fabs(a->y-b->y)>.01) return(0);
	if (fabs(a->z-b->z)>.01) return(0);
	return(1);
	}

static void ReflVect(double *pnew, double *p, double *n) {
	double d = DDotProd(p,n);
	pnew[0] = p[0]-2.0*d*n[0];
	pnew[1] = p[1]-2.0*d*n[1];
	pnew[2] = p[2]-2.0*d*n[2];
	}

static int reg_vert(DPoint *p) {
	int i;
	for (i=0; i<nverts; i++) {
		if (same_pt(p,&verts[i])) return(i);
		}
	i = nverts;
	nverts++;
#ifdef DBG
	printf("reg_vert: p = (%.5f,%.5f,%.5f)  nverts = %d\n", p->x,p->y,p->z,nverts);
#endif
	verts[i]=*p;
	return(i);
	}

#ifdef DBG
static void prx(STri *t,char *s) {
	printf("%s: (%.3f,%.3f,%.3f)(%.3f,%.3f,%.3f)(%.3f,%.3f,%.3f)\n",s,
		t->p[0].x,	t->p[0].y,	t->p[0].z,
		t->p[1].x,	t->p[1].y,	t->p[1].z,
		t->p[2].x,	t->p[2].y,	t->p[2].z);
	}
#endif

static int reg_axis(int n, DPoint *p, STri *s) {
	int i; 
	AxisList *al = &axis[n];
	for (i=0; i<al->num; i++) {
		if (same_pt(p,&al->n[i])) 
			return(i);
		}
	i = al->num;
	al->num++;
	al->n[i] = *p;
	al->t[i] = *s;
	return(i);
	}

static reg_stri(STri *t) {
	int nax[3],i;
	for (i=0; i<3; i++) {
		nax[i] = reg_axis(i,&t->p[i],t);
		}
	for (i=0; i<nregtris; i++) {
		if (nax[0]==regtris[i].nax[0] && nax[1]==regtris[i].nax[1]
			&&nax[2]==regtris[i].nax[2]) return(0);
		}
	if(nregtris>=MAXREGTRIS) {
		printf(" REG TRI OVERFLOW \n");
		return 0;
		}
	i = nregtris;
	regtris[i].nax[0] = nax[0];
	regtris[i].nax[1] = nax[1];
	regtris[i].nax[2] = nax[2];
	nregtris++;
	return(1);
	}

static void R_refl(STri *t, STri *tnew) {
	DPoint n;
	/* reflect vert 0 in line connecting vert 1 and 2 */
	tnew->p[1] = t->p[1];
	tnew->p[2] = t->p[2];
	DCrossProd(&n.x, &t->p[1].x, &t->p[2].x);
	NormVect(&n.x);	
	ReflVect(&tnew->p[0].x,&t->p[0].x,&n.x);
	ReflVect(&tnew->v.x,&t->v.x,&n.x);
	reg_vert(&tnew->v);
	}
	
static void P_refl(STri *t, STri *tnew) {
	DPoint n;
	/* reflect vert 1 in line connecting vert 0 and 2 */
	tnew->p[0] = t->p[0];
	tnew->p[2] = t->p[2];
	DCrossProd(&n.x, &t->p[0].x, &t->p[2].x);
	NormVect(&n.x);	
	ReflVect(&tnew->p[1].x,&t->p[1].x,&n.x);
	ReflVect(&tnew->v.x,&t->v.x,&n.x);
	reg_vert(&tnew->v);
	}

static void Q_refl(STri *t, STri *tnew) {
	DPoint n;
	/* reflect vert 2 in line connecting vert 0 and 1 */
	tnew->p[0] = t->p[0];
	tnew->p[1] = t->p[1];
	DCrossProd(&n.x, &t->p[0].x, &t->p[1].x);
	NormVect(&n.x);	
	ReflVect(&tnew->p[2].x,&t->p[2].x,&n.x);
	ReflVect(&tnew->v.x,&t->v.x,&n.x);
	reg_vert(&tnew->v);
	}

int level = 0;

#if 0
static void traverse(STri *t, int op) {
	STri tnew;
#if 0
	dbgprintf(" traverse, op = %d, level = %d, &tnew = %X\n",op,level,&tnew);
	prx(t," ----"); 
#endif
	if (!reg_stri(t)) return;
	level++;
	if (op!=R_OP) { R_refl(t,&tnew);	traverse(&tnew,R_OP);	}
	if (op!=P_OP) { P_refl(t,&tnew);	traverse(&tnew,P_OP);	}
	if (op!=Q_OP) { Q_refl(t,&tnew);	traverse(&tnew,Q_OP);	}
	level--;
	}

#else

//  implements Stack
#define MAXQ 300
typedef struct {
	STri t;
	int  op;
	} QEntry;

static QEntry theq[MAXQ];
static int qnext;

static void qinit() {  qnext  = 0; }
static int qany() { return(qnext!=0); }

static void qput(STri *t, int op) {
	theq[qnext].t  = *t;	
	theq[qnext].op  = op;
	qnext++;
	}

static int qget(STri *t ) {
	int  op;
	qnext--;
	op  = theq[qnext].op;
	*t  = theq[qnext].t;
	return(op);		
	}


static void traverse(STri *t, int op) {
	STri tnew,tr;
	qinit();
	qput(t,op);
	while(qany()) {
		op = qget(&tr);
		if (!reg_stri(&tr)) continue;
		if (op!=Q_OP) { Q_refl(&tr,&tnew);  qput(&tnew,Q_OP);	}
		if (op!=P_OP) { P_refl(&tr,&tnew);  qput(&tnew,P_OP);	}
		if (op!=R_OP) { R_refl(&tr,&tnew);  qput(&tnew,R_OP);	}
		}
	}

#endif

static int curax = 0;
static short do_axis[3];
static short star_axis[3];

#ifdef DBG2
static double dist(int a, int b) {
	DPoint *pa = &verts[a];
	DPoint *pb = &verts[b];
	DPoint d;
	d.x = pa->x-pb->x;
	d.y = pa->y-pb->y;
	d.z = pa->z-pb->z;
	return(sqrt(d.x*d.x+d.y*d.y+d.z*d.z));
	}
#endif

static void outface(int a, int b, int c, int vab, int vbc, int vca) {
	unsigned char flags;
	
#ifdef DBG
	printf("outface[%d] (%d,%d,%d) \n",nfaces,a,b,c);
#endif

	fdata[nfaces].a = a;
	fdata[nfaces].b = b;
	fdata[nfaces].c = c;
	flags = 0;
	if (vab) flags|=FC_ABLINE;
	if (vbc) flags|=FC_BCLINE;
	if (vca) flags|=FC_CALINE;
	fdata[nfaces].flags = flags;
	fdata[nfaces].material = (unsigned char)imtl[curax];
	nfaces++;
	}

static int pol[20];
static int npoly=0;
static short center_verts, subdiv_sides;

static void init_poly() { npoly=0; }

static void outv(DPoint *p) {
	int i;
	int nv = reg_vert(p);

	/* make sure same vert doesnt occur twice in poly */
	for (i=0; i<npoly; i++) {
		if (nv==pol[i])
			return;
		}

#ifdef DBG
	printf("   outp ( %.5f, %.5f, %.5f )   nv = %d \n", 
		p->x,p->y,p->z,nv);
#endif
	pol[npoly++] = nv;
	}

static void check_facing() {
	DPoint *p[3], d01,d12,n; 
	int i,tmp;
	p[0] = &verts[pol[0]];
	p[1] = &verts[pol[1]];
	p[2] = &verts[pol[2]];
	d01.x = p[1]->x-p[0]->x;
	d01.y = p[1]->y-p[0]->y;
	d01.z = p[1]->z-p[0]->z;
	d12.x = p[2]->x-p[1]->x;
	d12.y = p[2]->y-p[1]->y;
	d12.z = p[2]->z-p[1]->z;
	DCrossProd(&n.x,&d01.x,&d12.x);
	if (DDotProd(&n.x,&p[1]->x)<0.0) {
#ifdef DBG
		printf("  Reverse Poly facing\n");
#endif
		for (i=0; i<npoly/2; i++) {
			tmp = pol[i];
			pol[i] = pol[npoly-i-1];
			pol[npoly-i-1] = tmp;
			}
		}
	}	

static void MidPoint(DPoint *mp, DPoint *a, DPoint *b) {
	mp->x = (a->x+b->x)*.5;
	mp->y = (a->y+b->y)*.5;
	mp->z = (a->z+b->z)*.5;
	}

static void ScaleVect(DPoint *v, double s) {
	v->x *= s;
	v->y *= s;
	v->z *= s;
	}

static void outpoly() {
	int i;
	int vab,vca;
#ifdef DBG
	printf("  outpoly, npoly = %d \n", npoly);
#endif
	/* check to make sure poly faces away from origin: if not, flip it */
	check_facing();	

	if (subdiv_sides) {
		DPoint midp;
		int nxti = 0;
		for (i=npoly-1; i>=0; i--) {
			pol[2*i] = pol[i];
			MidPoint(&midp,&verts[pol[i]],&verts[pol[nxti]]);
			pol[2*i+1] = reg_vert(&midp);
			nxti = 2*i;
			}
		npoly *= 2;
		}

	if (center_verts||star_axis[curax]||state.scale_axis[curax]!=1.0) {
		DPoint sum;
		int nc,nxt;
		sum.x = sum.y = sum.z = 0.0;
		for (i=0; i<npoly; i++) {
			sum.x += verts[pol[i]].x;
			sum.y += verts[pol[i]].y;
			sum.z += verts[pol[i]].z;
			}
		sum.x /= (float)npoly;
		sum.y /= (float)npoly;
		sum.z /= (float)npoly;

		if (state.scale_axis[curax]!=1.0) {
			ScaleVect(&sum,state.scale_axis[curax]);
			nc = reg_vert(&sum);
			for (i=0; i<npoly; i++) {
				nxt = (i==npoly-1)?0:i+1;
				outface(nc,pol[i],pol[nxt],1,1,1);
				}
			}
		else {
			nc = reg_vert(&sum);
			for (i=0; i<npoly; i++) {
				nxt = (i==npoly-1)?0:i+1;
				outface(nc,pol[i],pol[nxt],0,1,0);
				}
			}
		}
	else  {
		for (i=1; i<npoly-1; i++) {
			vab = vca = 0;
			if (i==1) vab = 1;
			if	(i==npoly-2) vca = 1;
#ifdef DBG2
			printf("         face (%d,%d,%d) vis = (%d %d %d) \n",
				pol[0],pol[i],pol[i+1],vab,1,vca);
#endif
			outface(pol[0],pol[i],pol[i+1],vab,1,vca);
#ifdef DBG2
			{
			double l = dist(pol[i],pol[i+1]);
			printf(" side length = %.6f \n",l);
			}
#endif

		
			}
		}
	}


static void DoRPolys() {
	STri t;
	int i;
#ifdef DBG
	printf("\nDoRPolys---------num = %d\n",axis[0].num);
#endif
	for (i=0; i<axis[0].num; i++) {
		init_poly();
#ifdef DBG
		printf("---------\n");
#endif
		t = axis[0].t[i];
		outv(&t.v);
		P_refl(&t,&t);	 outv(&t.v);
		Q_refl(&t,&t);	 outv(&t.v);
		P_refl(&t,&t);	 outv(&t.v);
		outpoly();
		}
	}

static void DoPPolys() {
	int i,j;
	STri t;
#ifdef DBG
	printf("\nDoPPolys---------num = %d, npsides = %d\n",axis[1].num,nsides[1]);
#endif	
	for (i=0; i<axis[1].num; i++) {
#ifdef DBG
		printf("---------\n");
#endif
		init_poly();
		t = axis[1].t[i];
		for (j=0; j<nsides[1]; j++) {
			Q_refl(&t,&t);	
			outv(&t.v); 
			R_refl(&t,&t);	
			outv(&t.v);
			}
		outpoly();
		}
	}

static void DoQPolys() {
	int i,j;
	STri t;
#ifdef DBG
	printf("\nDoQPolys---------num = %d, nqsides = %d\n",axis[2].num,nsides[2]);
#endif
	for (i=0; i<axis[2].num; i++) {
#ifdef DBG
		printf("---------\n");
#endif
		t = axis[2].t[i];
		init_poly();
		for (j=0; j<nsides[2]; j++) {
			P_refl(&t,&t);	
			outv(&t.v); 
			R_refl(&t,&t);	
			outv(&t.v);
			}
		outpoly();
		}
	}


static void TesselatePolys(void) {
	int i;

	for (i=0; i<3; i++) {
		if (do_axis[i]) {
			curax = i;
			switch(i) {
				case 0: DoRPolys(); break;
				case 1: DoPPolys(); break;
				case 2: DoQPolys(); break;
				}
			}
		}
#ifdef DBG
	printf(" ------------------ Tesselate done -----------------\n");
#endif
	}

static void RotY(DPoint *p, double s, double c) {
	DPoint t;
	t.x =  c*p->x + s*p->z;
	t.z =  -s*p->x + c*p->z;
	t.y =  p->y;
	*p = t;
	}

static void RotX(DPoint *p, double s, double c) {
	DPoint t;
	t.y =  c*p->y + s*p->z;
	t.z =  -s*p->y + c*p->z;
	t.x =  p->x;
	*p = t;
	}

static int MakeHedron(int np, int dp, int nq, int dq, double cp, double cq) {
	STri t0;
	double pang,qang,cosp,cosq,sinp,sinq;

#ifdef DBG
	printf("MakeHedron: p = %d/%d  q = %d/%d  cp = %.4f  cq = %.4f\n",np,dp,nq,dq,cp,cq);
#endif

	nregtris = nfaces = nverts = 0;
	center_verts = state.verts>0?1:0;
	subdiv_sides = state.verts==2?1:0;
	nsides[0] = 4;
	nsides[1] = np;
	nsides[2] = nq;

	/* these are two of the angles of a right spherical triangle */
	pang = PI*(double)dp/(double)np;
	qang = PI*(double)dq/(double)nq;

	/* compute the (sin,cos) of sides of the spherical triangle */
	cosp = cos(pang)/sin(qang);
	cosq = cos(qang)/sin(pang);
	sinp = sqrt(1.0-cosp*cosp);
	sinq = sqrt(1.0-cosq*cosq);

	t0.p[0].x = 0.0;  t0.p[0].y = 0.0;  t0.p[0].z = 1.0;
	t0.p[1].x = sinq; t0.p[1].y = 0.0;  t0.p[1].z = cosq;
	t0.p[2].x = 0.0;  t0.p[2].y = sinp; t0.p[2].z = cosp;

	t0.v.x = cq*sinq; 
	t0.v.y = cp*sinp;
	t0.v.z = 1.0 + cq*(cosq-1.0) + cp*(cosp-1.0);
 	NormVect(&t0.v.x); 

#ifdef DBG
	printf(" pang = %.5f, qang = %.5f \n", RadToDeg(pang), RadToDeg(qang));
	printf(" cpa = %.5f, cqa = %.5f\n", RadToDeg(acos(cosp)),RadToDeg(acos(cosq)));
	printf(" t = (%.4f,%.4f,%.4f) \n",t0.v.x,t0.v.y,t0.v.z);
#endif
	switch(state.axis) {
		case 1:  /* want P aligned with Z -- rotate into pos*/
			RotY(&t0.p[0],-sinq,cosq);									
			RotY(&t0.p[1],-sinq,cosq);									
			RotY(&t0.p[2],-sinq,cosq);									
			RotY(&t0.v,-sinq,cosq);									
			break;
		case 2:  /* Q aligned with Z -- */
			RotX(&t0.p[0],sinp,cosp);									
			RotX(&t0.p[1],sinp,cosp);									
			RotX(&t0.p[2],sinp,cosp);									
			RotX(&t0.v,sinp,cosp);									
			break;
		}

#ifdef DBG
	printf(" p0 = %.4f, %.4f, %.4f \n",t0.p[0].x,t0.p[0].y,t0.p[0].z); 
	printf(" p1 = %.4f, %.4f, %.4f \n",t0.p[1].x,t0.p[1].y,t0.p[1].z); 
	printf(" p2 = %.4f, %.4f, %.4f \n",t0.p[2].x,t0.p[2].y,t0.p[2].z); 
	printf(" v  = %.4f, %.4f, %.4f \n",t0.v.x,t0.v.y,t0.v.z); 
#endif

	memset(axis,0,3*sizeof(AxisList));
	reg_vert(&t0.v);
	level = 0;
	traverse(&t0, -1);

	do_axis[0] = do_axis[1] = do_axis[2] = 1;
	star_axis[0] = star_axis[1] = star_axis[2] = 0;
	if (dp>1) star_axis[1] = 1;	
	if (dq>1) star_axis[2] = 1;	

	if (cp==0||cq==0) do_axis[0] = 0;
	if (cp==0.0 && cq==1.0) do_axis[1] = 0;
	if (cp==1.0 && cq==0.0) do_axis[2] = 0;


#ifdef DBG
	printf(" nsides = %d,%d,%d \n",nsides[0],nsides[1],nsides[2]);
	printf(" do_axis= %d,%d,%d \n",do_axis[0],do_axis[1],do_axis[2]);
	printf(" numaxis = %d,%d,%d \n",axis[0].num,axis[1].num,axis[2].num);
	printf(" nverts = %d \n",nverts);
	printf(" ------------------ MakeHedron done -----------------\n");
#endif
	

	return(1);
	}

/*------------------------------------------------------------------------------------*/


void CreateHedron(
		Mesh &mesh, 
		int family, float fp, float fq, float radius,
		int axis, int vts, float *scale_axis)
	{
	int i;	
	
	state.sclass        = family;
	state.fp            = fp;
	state.fq            = fq;
	state.radius        = radius;
	state.axis          = axis;
	state.verts         = vts;	
	state.scale_axis[0] = scale_axis[0];
	state.scale_axis[1] = scale_axis[1];
	state.scale_axis[2] = scale_axis[2];
	
	MakeHedron( pnum[state.sclass], pden[state.sclass], qnum[state.sclass],
			    qden[state.sclass], state.fp, state.fq );
	TesselatePolys();
	
	mesh.setNumVerts(nverts);
	mesh.setNumFaces(nfaces);

	for (i=0; i<nverts; i++) {
		mesh.verts[i].x = float(state.radius * verts[i].x);
		mesh.verts[i].y = float(state.radius * verts[i].y);
		mesh.verts[i].z = float(state.radius * verts[i].z);
		}
	
	for (i=0; i<nfaces; i++) {
		mesh.faces[i].v[0]    = fdata[i].a;
		mesh.faces[i].v[1]    = fdata[i].b;
		mesh.faces[i].v[2]    = fdata[i].c;
		mesh.faces[i].flags   = fdata[i].flags;
		mesh.faces[i].smGroup = fdata[i].sm_group;
		mesh.setFaceMtlIndex(i,fdata[i].material);
		}
	}




