#define STRICT
#include <strbasic.h>
#include <windows.h>
#include <windowsx.h>
#include <stdlib.h>
#include <math.h>
#include "maxtypes.h"
#include "gfxlib.h"
#include "utillib.h"
#include "BlobMesh.h"

//aszabo|Nov.27.01 - Leave particle related methods in so 
// that prim.dlo from the MAX SDK Samples can load (VIZ 4 shipped
// with the MAX SDK and not with a "VIZ SDK"
#if !defined(NO_PARTICLES) || defined( DESIGN_VER)
//
// see comment at top of particle.cpp, look for NO_PARTICLES
//


#define TOPF 0
#define FRONTF 1
#define RIGHTF 2	//2	
#define BACKF 3	
#define LEFTF 4		//4	
#define BOTF 5	
typedef struct
{
	Point3 vmin,vmax;
	int pstart,pstop;
}	BunchData;

typedef struct
{
	int pattern,v[4];
}	linedata;

#define locface 6

const float sqrt2=(float)sqrt(3.0f);

class corners
{ 
public:
	corners();
	float field;
	linedata oldfaces[locface];
	BOOL isdone,bisdone;
};

typedef corners *cornerlst;

typedef struct
{
	Point3 pos;
	int gridnum,goodnum;
	float field;
	BOOL sign;
}	cubepts;

typedef struct 
{ 
	int p1,p2,e1a,e1b,e2a,e2b;
}	bseg;

typedef struct
{ 
	int gridnum,z;
	Point3 cpos;
}	Surfpush;

corners::corners()
{ 
	field=-0.6f;
	isdone=FALSE;bisdone=FALSE;
	for (int i=0;i<6;i++) 
		oldfaces[i].pattern=0;
}

class BlobParticle 
{
	public:
		UniformGrid grid;

		Tab<BunchData> bloblst;

		Tab<int> plst;
		Tab<Point3> maxpoint;
		Tab<Point3> minpoint;
		Tab<Surfpush> scube;
		SphereData *InputData;
		corners *level;
		cubepts *ccube;
		BOOL smooth;
		float fthreshold,res;
		int bnum,cblob,Nx,Ny,Nz,lastline;
		Point3 deltax,deltay,deltaz,deltaxy;
		cornerlst *alllevels;
		int vertex,face;
		int vAlloc,vBlock,vFree;
		int faceAlloc,faceBlock,faceFree;
		void RecalcBounds(int num,Point3 pvmin,Point3 pvmax);
		BOOL CheckBounds(int num,Point3 pvmin,Point3 pvmax);
		void AddABlob(int c,int *incount);
		BOOL OneInSet(int bnum) {return(bloblst[bnum].pstart==bloblst[bnum].pstop);}
		void AddToSet(int bcount,int going);
		void MergeSet(int bcount);
		void FillInFields(corners *level,int start,int ix,int y,cubepts *cube);
		float CalcField(Point3 cpos);
		Point3 CheckForVertex(Point3 apos,Point3 bpos,float afield,float bfield);
		void AddLine(bseg *blst,int *bcount,int *vlst,int a,int b,int *sign,int *edge);
		BOOL FillInFace(cubepts *cube,bseg *blst,int *bcount,int a,int b,int c, int d,linedata *srec,int *oldv,linedata *olddata,Mesh *mesh,BOOL type=0);
		void FillInCube(cubepts *cube,bseg *blst,int *bcount,Mesh *mesh,corners *level0,corners *level1);
		void GenerateFaces(bseg *line,int lcount,Mesh *mesh);
		int AllocateCubes(int levelmax,int levels,cornerlst *allpts);
		BOOL AllocateData(int num);
		void DoCube(int ix,int x,int y,int z,Point3 cpos,Mesh *mesh);
		int CreateBlobs(Mesh *mesh,float tension,int alive,int many);
		BOOL WalkGrid(Mesh *mesh,int maxx,int maxy,int Nz,Point3 deltas,int i);
		void FillInSFields(corners *level,int start,cubepts *cube);
		void FillInSCube(cubepts *cube,bseg *blst,int *bcount,Mesh *mesh,int z);
		BOOL FindSCube(int ix,int x,int y,int z,Point3 cpos,Mesh *mesh);
	  BOOL DoSCube(int ix,int x,int y,int z,Point3 cpos,Mesh *mesh);
		BOOL Surface(int gridnum,int z,Point3 cpos,Mesh *mesh);
		BOOL TrackSurface(int gridnum,int z,Point3 cpos,Mesh *mesh);
		BOOL Chklst(int *oldv,BOOL type,int &ind,linedata *olddata,int a,int b,bseg *blst,int *bcount,int num);
		int ProcessOldBlobs(ParticleSys parts,Mesh *mesh,float tension,int many);
};

void BlobParticle::RecalcBounds(int num,Point3 pvmin,Point3 pvmax)
{ 
	BOOL inset=FALSE;
	if (pvmin.x<bloblst[num].vmin.x)
		bloblst[num].vmin.x=pvmin.x;
	if (pvmax.x>bloblst[num].vmax.x) 
		bloblst[num].vmax.x=pvmax.x;
	if (pvmin.y<bloblst[num].vmin.y) 
		bloblst[num].vmin.y=pvmin.y;
	if (pvmax.y>bloblst[num].vmax.y)
		bloblst[num].vmax.y=pvmax.y;
	if (pvmin.z<bloblst[num].vmin.z)
		bloblst[num].vmin.z=pvmin.z;
	if (pvmax.z>bloblst[num].vmax.z) 
		bloblst[num].vmax.z=pvmax.z;
}

void BlobParticle::AddToSet(int bcount,int going)
{ 
	int plen=1;
	if (bloblst[bcount].pstop==(bloblst[going].pstart-1))
		bloblst[bcount].pstop=bloblst[going].pstop;
	else
	{ 
		int pos;
		plen+=bloblst[going].pstop-(pos=bloblst[going].pstart);
		int *plst2=new int[plen];
		for (int i=0;i<plen;i++)
			plst2[i]=plst[pos++];
		pos=bloblst[going].pstop;
		for (i=bloblst[going].pstart-1;i>bloblst[bcount].pstop;i--)
			plst[pos--]=plst[i];
		pos=bloblst[bcount].pstop+1;
		for (i=0;i<plen;i++)
			plst[pos++]=plst2[i];
		bloblst[bcount].pstop+=plen;
		for (i=bcount+1;i<going;i++)
		{ 
			bloblst[i].pstart+=plen;
			bloblst[i].pstop+=plen;
		}
		delete[] plst2;
	}
	RecalcBounds(bcount,bloblst[going].vmin,bloblst[going].vmax);
	bloblst.Delete(going,1);
	bnum--;
}

BOOL CheckBoundingSeg(float min1,float max1,float min2,float max2)
{
	return(((max1<=max2)&&(((min1<=min2)&&(min2<=max1))||((min2<=min1)&&(min1<=max1))))||
		((max2<=max1)&&(((min1<=min2)&&(min2<=max2))||((min2<=min1)&&(min1<=max2)))));
}

BOOL BlobParticle::CheckBounds(int num,Point3 pvmin,Point3 pvmax)
{ 
	BOOL isx=((bloblst[num].vmin.x<=pvmin.x) && (bloblst[num].vmax.x>=pvmin.x)) || ((bloblst[num].vmin.x<=pvmax.x) && (bloblst[num].vmax.x>=pvmax.x)) || ((bloblst[num].vmin.x>=pvmin.x) && (bloblst[num].vmax.x<=pvmax.x));
	BOOL isy=((bloblst[num].vmin.y<=pvmin.y) && (bloblst[num].vmax.y>=pvmin.y)) || ((bloblst[num].vmin.y<=pvmax.y) && (bloblst[num].vmax.y>=pvmax.y)) || ((bloblst[num].vmin.y>=pvmin.y) && (bloblst[num].vmax.y<=pvmax.y));
	BOOL isz=((bloblst[num].vmin.z<=pvmin.z) && (bloblst[num].vmax.z>=pvmin.z)) || ((bloblst[num].vmin.z<=pvmax.z) && (bloblst[num].vmax.z>=pvmax.z)) || ((bloblst[num].vmin.z>=pvmin.z) && (bloblst[num].vmax.z<=pvmax.z));
	return ((isx && isy) && isz);
}

void BlobParticle::MergeSet(int bcount)
{ 
	int curblob=0;			
	BOOL inset;
	while (curblob<bnum)
	{ 
		inset=FALSE;
		curblob=0;
		while ((curblob<=bnum)&&((curblob==bcount)||(!(inset=CheckBounds(bcount,bloblst[curblob].vmin,bloblst[curblob].vmax)))))
			curblob++;
	    if (inset)
		{ 
			if (curblob<bcount) 
			{
				AddToSet(curblob,bcount);bcount=curblob;
			}
			else 
			AddToSet(bcount,curblob);
		}
	 }
}

void BlobParticle::AddABlob(int c,int *incount)
{ 
	int i;
	Point3 tmin,tmax;
	BOOL inset=FALSE;
	bloblst[bnum].vmin=minpoint[plst[*incount]];
	bloblst[bnum].vmax=maxpoint[plst[*incount]];
	int tmp,bcount=0;
  // if particle fits into an existing set, add it
	while ((bcount<bnum)&&(OneInSet(bcount)||!(inset=CheckBounds(bcount,minpoint[plst[*incount]],maxpoint[plst[*incount]]))))
		bcount++;
	(*incount)++;
	if (inset)   // see if new set not intersects any other set
	{ 
		AddToSet(bcount,bnum);
		MergeSet(bcount);
	}
	else 	// create new set by checking all unset particles with this one
	{ 
		for (i=*incount;i<c;i++)
		{ 
			inset=CheckBounds(bnum,minpoint[plst[i]],maxpoint[plst[i]]);
			if (inset)
			{ 
				RecalcBounds(bnum,minpoint[plst[i]],maxpoint[plst[i]]);
				if (*incount!=i)
				{ 
					tmp=plst[i];
					plst[i]=plst[*incount];
					plst[*incount]=tmp;
				}
				(*incount)++;
			}
		}
    bloblst[bnum].pstop=*incount-1;
	if (!OneInSet(bnum)) MergeSet(bnum);
	}
}

int BlobParticle::AllocateCubes(int levelmax,int levels,cornerlst *allpts)
{  
	BOOL ok=TRUE;
	for (int i=0;(i<levels)&&(ok);i++)
		ok=(NULL!=(allpts[i]=new corners[levelmax]));
	if (!ok) 
		for (int j=0;j<i-1;j++)
		{
			delete[] allpts[j];
		}
   return(ok);
}


BOOL checkx(float x,float xby)
{ 
	return ((x>xby)||(x<-xby));
}

float dosquare(float temp)
{ 
	return temp*temp;
}

float FieldSphere(SphereData s,Point3 cpos,int &inrange)
{ 
	float field=0.0f,r=Length(cpos-s.center);
	if (r<s.radius)
	{ 
		float issign;
		issign=(r>=s.oradius?-1.0f:1.0f);
		field=issign*dosquare(s.rsquare-r*r)/s.tover4;
		inrange=(r>=s.oradius?1:2);
	}
  return field;
}


float GetField(SphereData &InputData,Point3 cpos,int &inrange)
{ 
	float field=0.0f;
	field=FieldSphere(InputData,cpos,inrange);
	return field;
 }
void AddUV(float &absfield,UVVert &tmpuv,UVVert tmp,float cfield)
{ 
	float f=(cfield<0.0f?-cfield:cfield);
	absfield+=f;
	tmpuv+=tmp*f;
}

float BlobParticle::CalcField(Point3 cpos)
{ 
	float field=0.0f,absfield=0.0f;int inrange=0,maxrange=0;
	float holdfield=0.0f;
	
	Tab<int> indexList;
	grid.InRadius(cpos, indexList);

//	for (int i=bloblst[cblob].pstart;i<=bloblst[cblob].pstop;i++)
	for (int ip=0;ip<indexList.Count();ip++)

	{	
		int i = indexList[ip];

		float cfield=GetField(InputData[i],cpos,inrange);
		if ((inrange==2)&&(cfield!=0.0f))
		{ 
			field+=cfield;
			maxrange=2;
		} 
		else if ((maxrange==0)&&(inrange==1))
		{ 
			holdfield=cfield;maxrange=1;
		}
		else if (inrange==1) maxrange=3;
	}
	if (maxrange==1) { field+=holdfield;}
	return(field-fthreshold);
}

void BlobParticle::FillInFields(corners *level,int start,int ix,int y,cubepts *cube)
{ 
	int s=start;
	if (ix==0)
	{ 
		if (y==0) 
			level[cube[start].gridnum].field=CalcField(cube[start].pos);
		s=start+3;
		level[cube[s].gridnum].field=CalcField(cube[s].pos);
	}
	s=start+1;
	if (y==0) 
		level[cube[s].gridnum].field=CalcField(cube[s].pos);
	s++;
	level[cube[s].gridnum].field=CalcField(cube[s].pos);
	for (int i=start;i<start+4;i++)
	{ 
		cube[i].sign=((cube[i].field=level[cube[i].gridnum].field)>=0.0f);
	}
}


Point3 BlobParticle::CheckForVertex(Point3 apos,Point3 bpos,float afield,float bfield)
{ 
	Point3 v;
	if (afield>bfield)
  		v=apos-(apos-bpos)*(afield/(afield-bfield));
	else v=bpos-(bpos-apos)*(bfield/(bfield-afield));
	return(v);
}


int FindVNum(int edge,linedata oldrec)
{// I'm not checking for no vertex.If it doesn't exist, the program's really screwed up.
	int vnum;
	if (edge==0) 
	{
		if (oldrec.pattern>8) 
			vnum=oldrec.v[1];
		else 
			vnum=oldrec.v[0];
	}

	else if (edge==1)
	{ 
		if ((oldrec.pattern==1)||(oldrec.pattern==7)||(oldrec.pattern==12)||(oldrec.pattern==13))
			vnum=oldrec.v[1];
		else if ((oldrec.pattern==4)||(oldrec.pattern==5)||(oldrec.pattern==9)||(oldrec.pattern==16))
			vnum=oldrec.v[0];
		else if (oldrec.pattern==15) vnum=oldrec.v[2];
		else vnum=oldrec.v[3];
	}
	else if (edge==2)
	{	
		if ((oldrec.pattern==2)||(oldrec.pattern==4)||(oldrec.pattern==14))
			vnum=oldrec.v[1];
		else if ((oldrec.pattern==6)||(oldrec.pattern==10)||(oldrec.pattern==12))
			vnum=oldrec.v[0];
		else if ((oldrec.pattern==7)||(oldrec.pattern==8))
			vnum=oldrec.v[2];
		else vnum=oldrec.v[3];
	}
	else if (edge==3)
	{	
		if ((oldrec.pattern==3)||(oldrec.pattern==5)||(oldrec.pattern==6)||(oldrec.pattern==8))
			vnum=oldrec.v[1];
		else if ((oldrec.pattern==11)||(oldrec.pattern==13)||(oldrec.pattern==14)||(oldrec.pattern==15))
			vnum=oldrec.v[0];
		else if (oldrec.pattern==16)
			vnum=oldrec.v[2];
		else vnum=oldrec.v[3];
	}
  return(vnum);
}


int GetOldVertex(int oldface,int newface,linedata oldrec)
{ 
	int edge;
	if (newface==TOPF) edge=2;
	else if (newface==BOTF) edge=0;
	else if (oldface==TOPF)
	{	
		if (newface==FRONTF) edge=0;
		else if (newface==RIGHTF) edge=1;
		else if (newface==BACKF) edge=2;
		else if (newface==LEFTF) edge=3;
	} 
	else if (oldface==LEFTF)	edge=(newface==FRONTF?1:3);
	else if (oldface==FRONTF) edge=(newface==RIGHTF?1:3);
	else if (oldface==BOTF)
	{	
		if (newface==FRONTF) edge=2;
		else if (newface==RIGHTF) edge=1;
		else if (newface==BACKF) edge=0;
		else if (newface==LEFTF) edge=3;
	} 
	else if (oldface==RIGHTF)	edge=(newface==FRONTF?3:1);
	else if (oldface==BACKF) edge=(newface==RIGHTF?3:1);
	return(FindVNum(edge,oldrec));
}
int CheckCurBlst(bseg *blst,int bcount,int a, int b)
{ 
	int oldnum=-1;
	for (int i=0;i<bcount;i++)
	{
		if (((blst[i].e1a==b)&&(blst[i].e1b==a))||((blst[i].e1a==a)&&(blst[i].e1b==b)))
		{ 
			oldnum=blst[i].p1;
			break;
		}
		if (((blst[i].e2a==b)&&(blst[i].e2b==a))||((blst[i].e2a==a)&&(blst[i].e2b==b)))
		{ 
			oldnum=blst[i].p2;
			break;
		}
	}
	return (oldnum);
}

void BlobParticle::AddLine(bseg *blst,int *bcount,int *vlst,int a,int b,int *sign,int *edge)
{ 
	blst[*bcount].p1=vlst[a];
	blst[*bcount].e1a=sign[a];
	blst[*bcount].e1b=edge[a];
	blst[*bcount].e2a=sign[b];
	blst[*bcount].e2b=edge[b];
	blst[(*bcount)++].p2=vlst[b];
}


BOOL Alst(int *oldv,BOOL type,int &ind)
{ 
	BOOL found=(!oldv[ind=FRONTF]&&(oldv[locface]==TOPF));
	if (type && !found) found=((!oldv[ind=BOTF]&&(oldv[locface]!=TOPF))||(!oldv[ind=BACKF]&&(oldv[locface]==BOTF)));
	
	return found;
}

BOOL Blst(int *oldv,BOOL type,int &ind)
{ 
	BOOL found=((!oldv[ind=FRONTF]&&(oldv[locface]==LEFTF))||(!oldv[ind=LEFTF]&&(oldv[locface]==BACKF)));
	if (type && !found)
		found=((!oldv[ind=RIGHTF]&&((oldv[locface]==TOPF)||(oldv[locface]==FRONTF)||(oldv[locface]==BOTF)))||
		(!oldv[ind=BACKF]&&(oldv[locface]==RIGHTF)));
	
	return found;
}

BOOL Clst(int *oldv,BOOL type,int &ind)
{ 
	BOOL found=((!oldv[ind=TOPF]&&(oldv[locface]!=BOTF))||(!oldv[ind=FRONTF]&&(oldv[locface]==BOTF)));
	if (type && !found) found=(!oldv[ind=BACKF]&&(oldv[locface]==TOPF));
	
	return found;
}

BOOL Dlst(int *oldv,BOOL type,int &ind)
{ 
	BOOL found=((!oldv[ind=LEFTF]&&((oldv[locface]!=RIGHTF)&&(oldv[locface]!=BACKF)))||(!oldv[ind=FRONTF]&&(oldv[locface]==RIGHTF)));
	if (type && !found) found=(!oldv[ind=RIGHTF]&&(oldv[locface]==BACKF))||(!oldv[ind=BACKF]&&(oldv[locface]==LEFTF));
	
	return found;
}
BOOL alreadyfound(int *oldv,int type)
{ 
	return((oldv[locface]==BOTF)||  ((type>0)&&(oldv[locface]==LEFTF)) || ((type>1)&&((oldv[locface]==BACKF)||(oldv[locface]==RIGHTF))) || ((type==2)&&(oldv[locface]==FRONTF)) );
}

int GetVNum(linedata rec,int edge)
{ 
	return (rec.pattern>0?FindVNum(edge,rec):-1);
}

#define A 0
#define B 1
#define C 2
#define D 3
#define ymin 0
#define xplus 1
#define yplus 2
#define xmin 3

BOOL BlobParticle::Chklst(int *oldv,BOOL type,int &ind,linedata *olddata,int a,int b,bseg *blst,int *bcount,int num)
{ 
	BOOL found;
	if (num==0) found=Alst(oldv,type,ind);
	else if (num==1) found=Blst(oldv,type,ind);
	else if (num==2) found=Clst(oldv,type,ind);
	else found=Dlst(oldv,type,ind);

	if (found) ind=GetOldVertex(ind,oldv[locface],olddata[ind]);
	else
	{ 
		if (type && (!alreadyfound(oldv,num)))
		{ 
			if (oldv[locface]==TOPF) 
			{ 
				if (num==0)
				{
					ind=GetVNum(olddata[FRONTF],C);
					if ((ind<0)&&(ccube[ymin].goodnum>-1)) 
						ind=GetVNum(level[ccube[ymin].goodnum].oldfaces[TOPF],C);
				}
				else if (num==1) 
				{
					ind=GetVNum(olddata[RIGHTF],C);
					if ((ind<0)&&(ccube[xplus].goodnum>-1)) 
						ind=GetVNum(level[ccube[xplus].goodnum].oldfaces[TOPF],D);
				}
				else if (num==2) 
				{
					ind=GetVNum(olddata[BACKF],C);
					if ((ind<0)&&(ccube[yplus].goodnum>-1)) 
						ind=GetVNum(level[ccube[yplus].goodnum].oldfaces[TOPF],A);
				}
				else 
				{
					ind=GetVNum(olddata[LEFTF],C);
					if ((ind<0)&&(ccube[xmin].goodnum>-1)) 
						ind=GetVNum(level[ccube[xmin].goodnum].oldfaces[TOPF],B);
				}
			}
			else if (oldv[locface]==FRONTF)
			{ 
				if (num==0)
				{
					ind=GetVNum(olddata[BOTF],C);
					if ((ind<0)&&(ccube[ymin].goodnum>-1)) 
						ind=GetVNum(level[ccube[ymin].goodnum].oldfaces[BOTF],A);
				}
				else if (num==1) 
				{
					ind=GetVNum(olddata[RIGHTF],D);
					if ((ind<0)&&(ccube[xplus].goodnum>-1)) 
						ind=GetVNum(level[ccube[xplus].goodnum].oldfaces[FRONTF],D);
				}
				else 
				{
					ind=GetVNum(olddata[LEFTF],B);
					if ((ind<0)&&(ccube[xmin].goodnum>-1)) 
						ind=GetVNum(level[ccube[xmin].goodnum].oldfaces[FRONTF],B);
				}
			}
			else if (oldv[locface]==RIGHTF)
			{ 
				if (num==0)
				{
					ind=GetVNum(olddata[BOTF],B);
					if ((ind<0)&&(ccube[xplus].goodnum>-1)) 
						ind=GetVNum(level[ccube[xplus].goodnum].oldfaces[BOTF],D);
				}
				else if (num==1) 
				{
					ind=GetVNum(olddata[BACKF],D);
					if ((ind<0)&&(ccube[xplus].goodnum>-1)) 
						ind=GetVNum(level[ccube[xplus].goodnum].oldfaces[BACKF],B);
				}
			}
			else if (oldv[locface]==BACKF)
			{ 
				if (num==0)
				{
					ind=GetVNum(olddata[BOTF],A);
					if ((ind<0)&&(ccube[yplus].goodnum>-1)) 
						ind=GetVNum(level[ccube[yplus].goodnum].oldfaces[BOTF],C);
				}
				else if (num==1) 
				{
					ind=GetVNum(olddata[LEFTF],D);
					if ((ind<0)&&(ccube[yplus].goodnum>-1)) 
						ind=GetVNum(level[ccube[yplus].goodnum].oldfaces[LEFTF],B);
				}
			}
			else if (oldv[locface]==LEFTF)
			{ 
				ind=GetVNum(olddata[BOTF],D);
				if ((ind<0)&&(ccube[xmin].goodnum>-1)) 
					ind=GetVNum(level[ccube[xmin].goodnum].oldfaces[BOTF],B);
			}
		}
		else ind=CheckCurBlst(blst,*bcount,a,b);
		found=ind>-1;
	}
	return found;
}

BOOL BlobParticle::FillInFace(cubepts *cube,bseg *blst,int *bcount,int a,int b,int c,int d,linedata *srec,int *oldv,linedata *olddata,Mesh *mesh,BOOL type)
{
	int sign[4],center,pat=0,mvcount=0,vlst[4],vcount=0,oldind=0,edge[4],obcount=*bcount;
	Point3 mainvarr[12],midpos,tvarr[12];
	int test=0;
	BOOL save=(srec!=NULL); 

    if (cube[a].sign!=cube[b].sign)
	{ 
		sign[vcount]=a;edge[vcount]=b;
		if (Chklst(oldv,type,oldind,olddata,a,b,blst,bcount,0))
			vlst[vcount++]=oldind;
		else
		{ 
			vlst[vcount++]=vertex+mvcount;
			mainvarr[mvcount]=CheckForVertex(cube[a].pos,cube[b].pos,cube[a].field,cube[b].field);
			mvcount++;
		}
		test=0;
	}
    if (cube[b].sign!=cube[c].sign)
	{ 
		sign[vcount]=b;edge[vcount]=c;
		if (Chklst(oldv,type,oldind,olddata,b,c,blst,bcount,1))
			vlst[vcount++]=oldind;
		else
		{ 
			vlst[vcount++]=vertex+mvcount;
			mainvarr[mvcount]=CheckForVertex(cube[b].pos,cube[c].pos,cube[b].field,cube[c].field);
			mvcount++;
		}
		test=0;
	}
    if (cube[c].sign!=cube[d].sign)
	{ 
		sign[vcount]=c;edge[vcount]=d;
		if (Chklst(oldv,type,oldind,olddata,c,d,blst,bcount,2))
			vlst[vcount++]=oldind;
		else
		{ 
			vlst[vcount++]=vertex+mvcount;
			mainvarr[mvcount]=CheckForVertex(cube[c].pos,cube[d].pos,cube[c].field,cube[d].field);
			mvcount++;
		}
		test=0;
	}
    if ((vcount==1)||(vcount==3))
	{ 
		sign[vcount]=d;edge[vcount]=a;
		if (Chklst(oldv,type,oldind,olddata,d,a,blst,bcount,3))
			vlst[vcount++]=oldind;
		else
		{ 
			vlst[vcount++]=vertex+mvcount;
			mainvarr[mvcount]=CheckForVertex(cube[d].pos,cube[a].pos,cube[d].field,cube[a].field);
			mvcount++;
		}
		test=0;
	}
	if (vcount>0)
	{
// pattern values are reversed because we'll be using them on the next cube.
		BOOL top=((oldv[locface]==BOTF)||(oldv[locface]==TOPF));
		if (vcount==2) 
		{ 
			BOOL rev=FALSE;
			if (cube[sign[0]].sign>0)  //right order within the cube
			{	
				AddLine(blst,bcount,vlst,1,0,sign,edge);rev=TRUE;
			}
			else 
			{
				AddLine(blst,bcount,vlst,0,1,sign,edge);
			}
			if (save)
			{	
				if (sign[0]==c) 
					pat=(rev?(top?3:12):(top?11:4));
				else if (sign[0]==b) 
				{ 
					if (sign[1]==c) 
						pat=(rev?(top?9:14):(top?1:6)); 
					else pat=(rev?(top?5:13):(top?13:5));
				}
				else if (sign[1]==b) 
					pat=(rev?(top?12:3):(top?4:11));
				else if 
					(sign[1]==c) pat=(rev?(top?10:2):(top?2:10));
				else 
					pat=(rev?(top?6:1):(top?14:9));
				(*srec).pattern=pat;
				(*srec).v[0]=blst[obcount].p2;
				(*srec).v[1]=blst[obcount].p1;
			}
		}
		else
		{ 
			midpos=(cube[a].pos+cube[c].pos)/2.0f; 
			center=(CalcField(midpos)>=0.0f);
			if ((!center)&&(!cube[sign[0]].sign)) 
			{ 
				AddLine(blst,bcount,vlst,0,1,sign,edge);
				AddLine(blst,bcount,vlst,2,3,sign,edge);
				pat=15;
			}
			else if (center)
			{	
				if (cube[sign[0]].sign)
				{
					AddLine(blst,bcount,vlst,1,0,sign,edge);
					AddLine(blst,bcount,vlst,3,2,sign,edge); 
					pat=8;
				}
				else
				{ 
					AddLine(blst,bcount,vlst,0,3,sign,edge);
					AddLine(blst,bcount,vlst,2,1,sign,edge); 
					pat=16;
				}
			}														
			else
			{ 
				AddLine(blst,bcount,vlst,3,0,sign,edge);
				AddLine(blst,bcount,vlst,1,2,sign,edge);
				pat=7;
			}
			if (save) 
			{ 
				int inobc=obcount+1;
				(*srec).pattern=pat;
				if (top)
				{ 
					(*srec).v[0]=blst[inobc].p2;
					(*srec).v[1]=blst[inobc].p1;
					(*srec).v[2]=blst[obcount].p2;
					(*srec).v[3]=blst[obcount].p1;
				}
				else
				{ 
					(*srec).v[0]=blst[obcount].p2;
					(*srec).v[1]=blst[obcount].p1;
					(*srec).v[2]=blst[inobc].p2;
					(*srec).v[3]=blst[inobc].p1;
				}
  			}
		}
		if (mvcount>0)
		{ 
			if(mvcount>vFree) 
			{
				vAlloc+=vBlock;
				mesh->setNumVerts(vAlloc,TRUE);
				vFree+=vBlock;
			}
			for (int i=0;i<mvcount;i++)
			{	
				mesh->verts[vertex++]=mainvarr[i];
			}
			vFree-=mvcount;
		}
	}
	else if (save) (*srec).pattern=17;
	return (vcount>0);
}

void GetSavedFace(bseg *lineseg,int *lcount,linedata srec)
{ int i,j=0,vsaved=0;
   if (srec.pattern!=17)
   { vsaved=((srec.pattern<7)||((srec.pattern>8)&&(srec.pattern<15))?1:2);
	 for (i=0;i<vsaved;i++)
	 { lineseg[*lcount].p1=srec.v[j++];
	   lineseg[*lcount].e1a=0;
	   lineseg[*lcount].e1b=0;
	   lineseg[*lcount].e2a=0;
	   lineseg[*lcount].e2b=0;
	   lineseg[*lcount].p2=srec.v[j++];
	   (*lcount)++;
	 }
   }
}

#define FRONTC 6
#define LEFTC 7

void BlobParticle::FillInCube(cubepts *cube,bseg *blst,int *bcount,Mesh *mesh,corners *level0,corners *level1)
{ 
	int i;
	linedata *olddata=level0[cube[0].gridnum].oldfaces;
	int old[locface+1];
	for (i=0;i<locface;i++) 
		old[i]=(olddata[i].pattern==0);
	if (old[TOPF])
	{ 
		old[locface]=TOPF;
		FillInFace(cube,blst,bcount,0,1,2,3,NULL,old,olddata,mesh);
	}
	else GetSavedFace(blst,bcount,level0[cube[0].gridnum].oldfaces[TOPF]);
  // front
	if (old[FRONTF])
	{ 
		old[locface]=FRONTF;
		FillInFace(cube,blst,bcount,5,6,1,0,NULL,old,olddata,mesh);
	}
	else GetSavedFace(blst,bcount,level0[cube[0].gridnum].oldfaces[FRONTF]);
  // right
   old[locface]=RIGHTF;FillInFace(cube,blst,bcount,6,7,2,1,&level0[cube[1].gridnum].oldfaces[LEFTF],old,olddata,mesh);
  // back
   old[locface]=BACKF;FillInFace(cube,blst,bcount,7,8,3,2,&level0[cube[3].gridnum].oldfaces[FRONTF],old,olddata,mesh);
  //left
	if (old[LEFTF])
	{ 
		old[locface]=LEFTF;
		FillInFace(cube,blst,bcount,8,5,0,3,NULL,old,olddata,mesh);
	}
	else GetSavedFace(blst,bcount,level0[cube[0].gridnum].oldfaces[LEFTF]);
  //bottom
	old[locface]=BOTF;FillInFace(cube,blst,bcount,8,7,6,5,&level1[cube[5].gridnum].oldfaces[TOPF],old,olddata,mesh);
}

static void AddFace(int a, int b, int c,int face,Mesh *pm,int curmtl)
{ 
	pm->faces[face].setSmGroup(1);
	pm->faces[face].setVerts(a,b,c);
	pm->faces[face].setMatID((MtlID)curmtl);
	pm->faces[face].setEdgeVisFlags(1,1,0);
}

void BlobParticle::GenerateFaces(bseg *line,int lcount,Mesh *mesh)
{ 
	bseg temp;
	int vlst[12],vcount=0,start=0;
	for (int i=0;i<lcount;i++)
	{	
		for (int j=i+1;j<lcount;j++)
		{ 
			if (line[j].p1==line[i].p2)
			{	
				temp=line[j];
				line[j]=line[i+1];
				line[i+1]=temp;
				break;
			}
		}
	}
	i=0;
	while (i<lcount) 
	{ 
		vlst[0]=line[i].p1;
		vlst[1]=line[i++].p2;
		vcount=2;
		start=vlst[0];
		while (line[i].p2!=start)
  			vlst[vcount++]=line[i++].p2;
		i++;
 		if((vcount-2)>faceFree)
		{
			faceAlloc+=faceBlock;
			mesh->setNumFaces(faceAlloc,(mesh->faces!=NULL));
			faceFree+=faceBlock;
		}
	    AddFace(vlst[0],vlst[1],vlst[2],face++,mesh,0);
		faceFree--;
		if (vcount>3)
		{ 
			if (vcount<6) 
			{
				AddFace(vlst[0],vlst[2],vlst[3],face++,mesh,0);
				if (vcount==5) 
					AddFace(vlst[0],vlst[3],vlst[4],face++,mesh,0);
 					faceFree-=(1+(vcount==5));
			}
			else
			{ 
				AddFace(vlst[2],vlst[3],vlst[4],face++,mesh,0);
				AddFace(vlst[4],vlst[5],vlst[0],face++,mesh,0);
				AddFace(vlst[0],vlst[2],vlst[4],face++,mesh,0);
				faceFree-=3;
			} // end else
		}	  // end if
	}	 // end while
}

void BlobParticle::DoCube(int ix,int x,int y,int z,Point3 cpos,Mesh *mesh)
{ 
	int rowmax=Nx+1,otherside=x+rowmax,lcount=0;
	bseg line[12];
	Point3 bpos=cpos-deltaz;
	cubepts curcube[10];
	curcube[0].pos=cpos;
	curcube[0].gridnum=x;
	curcube[1].pos=cpos+deltax;
	curcube[1].gridnum=x+1;
	curcube[2].pos=cpos+deltaxy;
	curcube[2].gridnum=otherside+1;
	curcube[3].pos=cpos+deltay;
	curcube[3].gridnum=otherside;
	curcube[4].pos=cpos;
	curcube[4].gridnum=x;
	curcube[5].pos=bpos;
	curcube[5].gridnum=curcube[0].gridnum;
	curcube[6].pos=bpos+deltax;
	curcube[6].gridnum=curcube[1].gridnum;
	curcube[7].pos=bpos+deltaxy;
	curcube[7].gridnum=curcube[2].gridnum;
	curcube[8].pos=bpos+deltay;
	curcube[8].gridnum=curcube[3].gridnum;
	curcube[9].pos=bpos;
	curcube[9].gridnum=curcube[0].gridnum;

	if (z==0)	
		FillInFields(alllevels[0],0,ix,y,curcube);
	else for (int i=0;i<4;i++)
		curcube[i].sign=((curcube[i].field=alllevels[0][curcube[i].gridnum].field)>=0.0f);

	FillInFields(alllevels[1],5,ix,y,curcube);

	curcube[FRONTC].gridnum=-1;
	curcube[LEFTC].gridnum=-1;
	
	FillInCube(curcube,line,&lcount,mesh,alllevels[0],alllevels[1]);

	if (lcount>0) 
		GenerateFaces(line,lcount,mesh);
}

BOOL BlobParticle::WalkGrid(Mesh *mesh,int maxx,int maxy,int Nz,Point3 deltas,int i)
{	
	Point3 cpos;
	int ix,x,y,z;
	cpos=bloblst[i].vmin;
	cpos.z=bloblst[i].vmax.z;
	x=0;
	for (z=0;z<Nz;z++)
	{ 
		for (y=0;y<maxy;y++)
		{ 
			for (ix=0;ix<maxx;ix++)
			{ 
				DoCube(ix,x,y,z,cpos,mesh);
				cpos.x+=deltas.x;
				x++;
				if (GetAsyncKeyState (VK_ESCAPE)) return FALSE;
			}
			x++;
			cpos.x=bloblst[i].vmin.x;
			cpos.y+=deltas.y;
		}
		x=0;
		corners *temp;
		temp=alllevels[0];
		alllevels[0]=alllevels[1];
		alllevels[1]=temp;
		cpos.y=bloblst[i].vmin.y;
		cpos.z-=deltas.z;
	}
	return TRUE;
}


void BlobParticle::FillInSFields(corners *level,int start,cubepts *cube)
{ 
	for (int s=start;s<start+4;s++)
	{ 
		if (!level[cube[s].gridnum].isdone) 
		{ 
			level[cube[s].gridnum].isdone=TRUE;
			level[cube[s].gridnum].field=CalcField(cube[s].pos);
		}
		cube[s].sign=(((cube[s].field=level[cube[s].gridnum].field)>=0.0f)?1:0);
	}
}


void AppendCube(corners *l,int gridnum,int z,Tab<Surfpush> &scube,Point3 delta)
{ 
	l[gridnum].bisdone=TRUE;
	Surfpush sp;
	sp.gridnum=gridnum,sp.z=z;
	sp.cpos=delta;
	scube.Append(1,&sp);
}

void BlobParticle::FillInSCube(cubepts *cube,bseg *blst,int *bcount,Mesh *mesh,int z)
{ 
	int i;
	linedata *olddata=alllevels[z][cube[0].gridnum].oldfaces;
	int old[locface+1],up=z-1,down=z+1;level=alllevels[z];ccube=cube;
	for (i=0;i<locface;i++) 
		old[i]=(olddata[i].pattern==0);
	if (old[TOPF])
	{ 
		old[locface]=TOPF;
		if (FillInFace(cube,blst,bcount,0,1,2,3,(z>0?&alllevels[up][cube[0].gridnum].oldfaces[BOTF]:NULL),old,olddata,mesh,1))
			if ((z)&&(!alllevels[up][cube[0].gridnum].bisdone)) 
				AppendCube(alllevels[up],cube[0].gridnum,up,scube,cube[0].pos+deltaz);
	}
	else GetSavedFace(blst,bcount,alllevels[z][cube[0].gridnum].oldfaces[TOPF]);
  // front
	if (old[FRONTF])
	{ 
		old[locface]=FRONTF;
		if (FillInFace(cube,blst,bcount,5,6,1,0,(cube[FRONTC].gridnum!=-1?&alllevels[z][cube[FRONTC].gridnum].oldfaces[BACKF]:NULL),old,olddata,mesh,1))
			if ((cube[FRONTC].gridnum>-1)&&(!alllevels[z][cube[FRONTC].gridnum].bisdone)) 
				AppendCube(alllevels[z],cube[FRONTC].gridnum,z,scube,cube[0].pos-deltay);
	}
	else GetSavedFace(blst,bcount,alllevels[z][cube[0].gridnum].oldfaces[FRONTF]);
  // right
	if (old[RIGHTF])
	{ 
		old[locface]=RIGHTF; 
		int more=cube[1].goodnum>-1;
		if (FillInFace(cube,blst,bcount,6,7,2,1,(more?&alllevels[z][cube[1].gridnum].oldfaces[LEFTF]:NULL),old,olddata,mesh,1))
			if ((more)&&(!alllevels[z][cube[1].gridnum].bisdone)) 
				AppendCube(alllevels[z],cube[1].gridnum,z,scube,cube[1].pos);
	}
	else GetSavedFace(blst,bcount,alllevels[z][cube[0].gridnum].oldfaces[RIGHTF]);
  // back
	if (old[BACKF])
	{ 
		old[locface]=BACKF;
		BOOL more=(cube[3].gridnum<lastline);
		if (FillInFace(cube,blst,bcount,7,8,3,2,(more?&alllevels[z][cube[3].gridnum].oldfaces[FRONTF]:NULL),old,olddata,mesh,1))
		if ((cube[3].gridnum<lastline)&&(!alllevels[z][cube[3].gridnum].bisdone)) 
			AppendCube(alllevels[z],cube[3].gridnum,z,scube,cube[3].pos);
	}
	else GetSavedFace(blst,bcount,alllevels[z][cube[0].gridnum].oldfaces[BACKF]);
 //left
	if (old[LEFTF])
	{ 
		old[locface]=LEFTF;
		BOOL more=(cube[LEFTC].gridnum>-1);
		if (FillInFace(cube,blst,bcount,8,5,0,3,(more?&alllevels[z][cube[LEFTC].gridnum].oldfaces[RIGHTF]:NULL),old,olddata,mesh,1))
  		if ((more)&&(!alllevels[z][cube[LEFTC].gridnum].bisdone)) 
			AppendCube(alllevels[z],cube[LEFTC].gridnum,z,scube,cube[0].pos-deltax);
	}
	else GetSavedFace(blst,bcount,alllevels[z][cube[0].gridnum].oldfaces[LEFTF]);
  //bottom
	if (old[BOTF])
	{ 
		old[locface]=BOTF;
		BOOL isdown=down<Nz;
		if (FillInFace(cube,blst,bcount,8,7,6,5,(isdown?&alllevels[down][cube[5].gridnum].oldfaces[TOPF]:NULL),old,olddata,mesh,1))
		if ((isdown)&&(!alllevels[down][cube[5].gridnum].bisdone)) 
			AppendCube(alllevels[down],cube[5].gridnum,down,scube,cube[5].pos);
	}
	else GetSavedFace(blst,bcount,alllevels[z][cube[0].gridnum].oldfaces[BOTF]);
}

BOOL BlobParticle::DoSCube(int ix,int x,int y,int z,Point3 cpos,Mesh *mesh)
{ 
	int rowmax=Nx+1,otherside=x+rowmax,lcount=0;
	bseg line[12];
	Point3 bpos=cpos-deltaz;
	corners *level0=alllevels[z],*level1=alllevels[z+1];
	cubepts curcube[10];
	curcube[0].pos=cpos;
	curcube[0].gridnum=x;
	curcube[ymin].goodnum=(y>0?x-rowmax:-1);
	curcube[1].pos=cpos+deltax;
	curcube[1].gridnum=x+1;
	curcube[xplus].goodnum=(ix<Nx?x+1:-1);
	curcube[2].pos=cpos+deltaxy;
	curcube[2].gridnum=otherside+1;
	curcube[yplus].goodnum=(y<Ny?otherside:-1);
	curcube[3].pos=cpos+deltay;
	curcube[3].gridnum=otherside;
	curcube[xmin].goodnum=(ix>0?x-1:-1);
	curcube[4].pos=cpos;
	curcube[4].gridnum=x;
	curcube[5].pos=bpos;
	curcube[5].gridnum=curcube[0].gridnum;
	curcube[6].pos=bpos+deltax;
	curcube[6].gridnum=curcube[1].gridnum;
	curcube[7].pos=bpos+deltaxy;
	curcube[7].gridnum=curcube[2].gridnum;
	curcube[8].pos=bpos+deltay;
	curcube[8].gridnum=curcube[3].gridnum;
	curcube[9].pos=bpos;
	curcube[9].gridnum=curcube[0].gridnum;
//  for (int i=0;i<4;i++)  curcube[i].sign=((curcube[i].field=level0[curcube[i].gridnum].field)>=0.0f);
	FillInSFields(level0,0,curcube);
	FillInSFields(level1,5,curcube);
	curcube[FRONTC].gridnum=(y>0?x-rowmax:-1);
	curcube[LEFTC].gridnum=(ix>0?x-1:-1);
	FillInSCube(curcube,line,&lcount,mesh,z);
	if (lcount>0) 
		GenerateFaces(line,lcount,mesh);
	return lcount;
}

BOOL BlobParticle::TrackSurface(int gridnum,int z,Point3 cpos,Mesh *mesh)
{ int cnt;
  while ((cnt=scube.Count())>0)
	{ 
		cnt--;gridnum=scube[cnt].gridnum;
		z=scube[cnt].z;
		cpos=scube[cnt].cpos;
		scube.Delete(cnt,1);
		int realNx=Nx+1;
		DoSCube(gridnum % realNx,gridnum,(int)(gridnum/realNx),z,cpos,mesh);
		if (GetAsyncKeyState (VK_ESCAPE)) 
		{
			scube.ZeroCount();
			scube.Shrink();
			return FALSE;
		}
	}
	scube.ZeroCount();scube.Shrink();
	return TRUE;
}

BOOL BlobParticle::FindSCube(int ix,int x,int y,int z,Point3 cpos,Mesh *mesh)
{ 
	alllevels[z][x].bisdone=TRUE;
	return DoSCube(ix,x,y,z,cpos,mesh);
}

BOOL BlobParticle::Surface(int gridnum,int z,Point3 cpos,Mesh *mesh)
{ 
	BOOL found=FALSE;
	int rowmax=Nx+1,ix=gridnum % rowmax,iy=(int)(gridnum/rowmax),i=0,igridnum=gridnum;
	Point3 pos=cpos;
	scube.SetCount(0);
	for (i=ix;(i>-1)&&(!found);i--)
	{ 
		found=FindSCube(i,igridnum,iy,z,pos,mesh);
		if (!found) 
		{
			igridnum--;
			pos=pos-deltax;
		}
	}
	if (!found)
	{ 
		pos=cpos+deltax;igridnum=gridnum+1;
		for (i=ix+1;(i<Nx)&&(!found);i++)
		{ 
			found=FindSCube(i,igridnum,iy,z,pos,mesh);
			if (!found) 
			{
				igridnum++;
				pos=pos+deltax;
			}
		}

	}
	if (!found)
	{ 
		pos=cpos+deltaz;
		for (i=z-1;(i>-1)&&(!found);i--)
		{ 
			found=FindSCube(ix,gridnum,iy,i,pos,mesh);
			if (!found) 
			{
				pos=pos+deltaz;
			} 
		}
	}
	if (!found)
	{ 
		pos=cpos-deltaz;
		for (i=z+1;(i<Nz)&&(!found);i++)
		{ 
			found=FindSCube(ix,gridnum,iy,i,pos,mesh);
			if (!found) 
			{
				pos=pos-deltaz;
			}
		}

	}
	if (!found)
	{ 
		pos=cpos-deltay;
		igridnum=gridnum-rowmax;
		for (i=iy-1;(i>-1)&&(!found);i--)
		{ 
			found=FindSCube(ix,igridnum,i,z,pos,mesh);
			if (!found) 
			{
				igridnum-=rowmax;
				pos=pos-deltay;
			}
		}

	}
	if (!found)
	{ 
		pos=cpos+deltay;
		igridnum=gridnum+rowmax;
		for (i=iy+1;(i<Ny)&&(!found);i++)
		{ 
			found=FindSCube(ix,igridnum,i,z,pos,mesh);
			if (!found) 
			{
				igridnum+=rowmax;
				pos=pos+deltay;
			}
		}
	}
	if (found) 
	{
		int cnt=scube.Count()-1;
		return TrackSurface(scube[cnt].gridnum,scube[cnt].z,scube[cnt].cpos,mesh);
	}
	else return FALSE;
}

BOOL BlobParticle::AllocateData(int num)
{ 
	return (InputData=new SphereData[num])!=NULL;
}

int BlobParticle::ProcessOldBlobs(ParticleSys parts,Mesh *mesh,float tension,int many)
{ 
	InputData=NULL;
	int total=parts.Count(),alive=0;
	for (int i=0;i<total;i++)
		if (parts.Alive(i))
			alive++;
	if ((alive==0)||(!AllocateData(alive)))
	{ 
		mesh->setNumVerts(0);
		mesh->setNumFaces(0);
		mesh->setNumTVerts(0);
		mesh->setNumTVFaces(0);
		return (!alive);
	} 
	alive=0;
	for (i=0; i<total; i++)
		if (parts.Alive(i))
		{ 
			InputData[alive].center=parts[i];
			InputData[alive].radius=0.5f*parts.radius[i];
			InputData[alive].rsquare=InputData[alive].radius*InputData[alive].radius;
			InputData[alive].tover4=parts.tension[i]*(InputData[alive].rsquare*InputData[alive].rsquare);
			InputData[alive].oradius=InputData[alive].radius;
			alive++;
		 }
	int ok=CreateBlobs(mesh,tension,alive,many);
	if (InputData) 
		delete[] InputData;
	return ok;
}

int BlobParticle::CreateBlobs(Mesh *mesh,float tension,int alive,int many)
{ 
	int i;
	Point3 fudgept,rpt;
	float fudgyn,fudgy;

	Tab<Point3> pointList;
	Tab<float> radiusList;

	fudgyn=0.05f*res;
	plst.SetCount(alive);

	maxpoint.SetCount(alive);
	minpoint.SetCount(alive);

	for (i=0; i<alive; i++)
	{ 
		InputData[i].radius=InputData[i].radius;		
		rpt=Point3(InputData[i].radius,InputData[i].radius,InputData[i].radius);
		fudgy=fudgyn/(InputData[i].tover4/(InputData[i].rsquare*InputData[i].rsquare));
		fudgept=Point3(fudgy,fudgy,fudgy);
		Box3 bb(-rpt-fudgept,rpt+fudgept);
		plst[i]=i;
		bb.pmin+=InputData[i].center;bb.pmax+=InputData[i].center;
		minpoint[i]=bb.pmin;
		maxpoint[i]=bb.pmax;
	}

	vAlloc=vFree=0;vBlock=4096;
	faceAlloc=faceFree=0;faceBlock=1024;
	Point3 Center=InputData[0].center;
	scube.ZeroCount();
	int incount,levelmax;
	bnum=0;
	incount=0;

	if (many)
	{ 
		bloblst.SetCount(alive);
		while (incount<alive)
		{	
			bloblst[bnum].pstop=(bloblst[bnum].pstart=incount);
			AddABlob(alive,&incount);
			bnum++;
		}
	}
	else
	{ 
		bloblst.SetCount(1);bnum=1;
		bloblst[0].pstart=0;bloblst[0].pstop=alive-1;
		bloblst[0].vmin=minpoint[0];
		bloblst[0].vmax=maxpoint[0];
		for (i=1;i<alive;i++) RecalcBounds(0,minpoint[i],maxpoint[i]);
	}
	vertex=0;face=0;
	maxpoint.ZeroCount();maxpoint.Shrink();
	minpoint.ZeroCount();minpoint.Shrink();
	int maxx,maxy;   Point3 deltas;
	BOOL finishedok=FALSE; 
	int levels=2;alllevels=NULL;
	if (many) 	
	{ 
		alllevels=new cornerlst[levels];
		if (!alllevels) goto arrgh;
	}

	grid.InitializeGrid(50);


	int j;
 	for (j=0;j<alive;j++)
		{
		pointList.Append(1,&InputData[j].center,1000);
		radiusList.Append(1,&InputData[j].radius,1000);
		}

	for (i=0;i<bnum;i++)
	{ 

		cblob=i;
		deltas=bloblst[i].vmax-bloblst[i].vmin;
		Nx=int(1.0f+(deltas.x/res));
		Ny=int(1.0f+(deltas.y/res));
		Nz=int(1.0f+(deltas.z/res));
		maxx=Nx;maxy=Ny;
		deltas.x/=Nx;deltas.y/=Ny;deltas.z/=Nz;
		deltax=Point3(deltas.x,0.0f,0.0f);deltay=Point3(0.0f,deltas.y,0.0f);
		deltaz=Point3(0.0f,0.0f,deltas.z);deltaxy=Point3(deltas.x,deltas.y,0.0f);
		levelmax=(Nx+1)*(Ny+1);float dlen=Length(deltas);


	 	for (int j=bloblst[i].pstart;j<=bloblst[i].pstop;j++)
			{
			int index = plst[j];
			InputData[index].radius+=sqrt2*res;
			}

		if (pointList.Count()>0)
			grid.LoadPoints(pointList.Addr(0),radiusList.Addr(0), pointList.Count());

		if (!many) 
		{ 
			if (alllevels) 
				delete[] alllevels;
			levels=Nz+1;
			alllevels=new cornerlst[levels];
			if (!alllevels) goto arrgh;
		}
 		if (!AllocateCubes(levelmax,levels,alllevels)) 
			return(0);
		if (many)
		{ 
			if (!WalkGrid(mesh,maxx,maxy,Nz,deltas,i)) 
				goto arrgh;
		}
		else 
		{ 
			lastline=levelmax-Nx;
			Point3 gridnum=Point3(Center.x-bloblst[0].vmin.x,Center.y-bloblst[0].vmin.y,bloblst[0].vmax.z-Center.z)/deltas;
			int x=(int)gridnum.x,y=(int)gridnum.y,z=(int)gridnum.z;
			Center=bloblst[0].vmin;
			Center.z=bloblst[0].vmax.z;
			Center=Center+Point3(x*deltas.x,y*deltas.y,-z*deltas.z);
			x+=(Nx+1)*y;
			if (!Surface(x,z,Center,mesh)) 
				goto arrgh;
		}
		for (int q=0;q<levels;q++) 
			delete[] alllevels[q]; 

	}	
    finishedok=TRUE;
	arrgh:
	if (!finishedok) 
	{ 
		for (int q=0;q<levels;q++) delete[] 
			alllevels[q];
		mesh->setNumVerts(0);
		mesh->setNumFaces(0);
		mesh->setNumTVerts(0);
		mesh->setNumTVFaces(0);
	}
	else 
	{	
		mesh->setNumVerts(vertex,TRUE);
		mesh->setNumFaces(face,TRUE);
	}
	delete[] alllevels;
	mesh->setNumTVerts(0);
	mesh->setNumTVFaces(0);
	bloblst.ZeroCount();
	bloblst.Shrink();
	plst.ZeroCount();
	plst.Shrink();

	grid.FreeGrid();

	return 1;
}

int MetaParticleFast::CreateMetas(ParticleSys parts,Mesh *mesh,float threshold,float res,float strength,int many)
{ 
	BlobParticle blob;
	blob.fthreshold=threshold;
	blob.res=res;
	return(blob.ProcessOldBlobs(parts,mesh,strength,many));
}

int MetaParticleFast::CreatePodMetas(SphereData *data,int num,Mesh *mesh,float threshold,float res,int many)
{	BlobParticle blob;
	blob.fthreshold=threshold;
	blob.res=res;
	blob.InputData=data;
 	return (blob.CreateBlobs(mesh,1.0f,num,many));
}

#endif // NO_PARTICLES
