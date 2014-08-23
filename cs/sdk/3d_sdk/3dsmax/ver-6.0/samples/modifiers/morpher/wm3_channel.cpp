/*===========================================================================*\
 | 
 |  FILE:	wM3_channel.cpp
 |			Weighted Morpher for MAX R3
 |			Stuff for channel management
 | 
 |  AUTH:   Harry Denholm
 |			Copyright(c) Kinetix 1999
 |			All Rights Reserved.
 |
 |  HIST:	Started 15-10-98
 | 
\*===========================================================================*/

#include "wM3.h"

// Construct the class with defaults
morphChannel::morphChannel()
{
	mNumPoints = 0;

	mConnection = NULL;
	mSel.ClearAll();

	// default name '-empty-'
	mName = _T(GetString(IDS_EMPTY_CHANNEL));
	// channel starts 'inactive'
	mActive = FALSE;
	// channel value limits defaults
	mSpinmin = 0.0f;
	mSpinmax = 100.0f;
	mUseLimit = FALSE;
	mUseSel = FALSE;
	cblock = NULL;
	mModded = FALSE;
	mInvalid = FALSE;
	mActiveOverride = TRUE;
//	mTargetCache=new TargetCache[MAX_TARGS];
	mTargetCache.resize(MAX_TARGS);
	mNumProgressiveTargs=0;
	mCurvature=0.5f;
	iTargetListSelection = 0;
	mTargetPercent = 0.0f;
}

morphChannel::~morphChannel()
{
	mPoints.erase(mPoints.begin(), mPoints.end());
	mPoints.resize(0);

	mWeights.erase(mWeights.begin(), mWeights.end());
	mWeights.resize(0);

	mDeltas.erase(mDeltas.begin(), mDeltas.end());
	mDeltas.resize(0);

	mTargetCache.erase(mTargetCache.begin(), mTargetCache.end());
	mTargetCache.resize(0);
	mConnection = NULL;
	cblock = NULL;
	mSel.ClearAll();

}

morphChannel::morphChannel(const morphChannel & from)
{
	mNumPoints = from.mNumPoints;

	mDeltas = from.mDeltas;
	mWeights = from.mWeights;

	mTargetCache = from.mTargetCache;

	mActive = from.mActive;
	mActiveOverride = from.mActiveOverride;
	mConnection = from.mConnection;
	mCurvature = from.mCurvature;
	mInvalid = from.mInvalid;
	mModded = from.mModded;
	mName = from.mName;
	mNumProgressiveTargs = from.mNumProgressiveTargs;
	mUseLimit = from.mUseLimit;
	mUseSel = from.mUseSel;
	mSpinmin = from.mSpinmin;
	mSpinmax = from.mSpinmax;
	
	mp = from.mp;
	iTargetListSelection = from.iTargetListSelection;
	mTargetPercent = from.mTargetPercent;
	mSel.ClearAll();

}


void morphChannel::ResetMe()
{
	mPoints.clear(); mPoints.resize(MAX_TARGS);
	mWeights.clear(); mWeights.resize(MAX_TARGS);
	mDeltas.clear(); mDeltas.resize(MAX_TARGS);
	mNumPoints = 0;
	mConnection = NULL;
	mName = _T(GetString(IDS_EMPTY_CHANNEL));
	mActive = FALSE;
	mSpinmin = 0.0f;
	mSpinmax = 100.0f;
	mUseLimit = FALSE;
	mUseSel = FALSE;
//	cblock = NULL;
	mModded = FALSE;
	mInvalid = FALSE;
	mActiveOverride = TRUE;
	mNumProgressiveTargs = 0;
	mCurvature=0.5f;
	ReNormalize();
}


void morphChannel::AllocBuffers( int sizeA, int sizeB )
{
	mPoints.clear();
	mWeights.clear();
	mDeltas.clear();

	mWeights.resize(sizeA);
	mDeltas.resize(sizeA);

	mPoints.resize(sizeA);
}


// Do some rough calculations about how much space this channel
// takes up
// This isn't meant to be fast or terribly accurate!
float morphChannel::getMemSize()
{
	float msize  = sizeof(*this);
	float pointsize = sizeof(Point3);
	msize += (pointsize*mNumPoints*(mNumProgressiveTargs+1));	// delta points
	msize += (sizeof(double)*mNumPoints);	// Weighting points
	return msize;
}

void morphChannel::operator=(const TargetCache& tcache)
{
	mNumPoints = tcache.mNumPoints;
	mPoints = tcache.mTargetPoints;
	mConnection = tcache.mTargetINode;
}

void morphChannel::ResetRefs(MorphR3 *mp, const int &cIndex)
{
	mp->ReplaceReference(101+cIndex, mConnection);

	int refnum;
	for(int i=1; i<=mNumProgressiveTargs; i++) {
		refnum = 200 + (cIndex * MAX_TARGS)	+ i;
		mp->ReplaceReference( refnum, mTargetCache[i-1].mTargetINode );
	}
}

void morphChannel::CopyTargetPercents(const morphChannel &chan)
{
	if(!&chan) return;
	mTargetPercent = chan.mTargetPercent;
	int targsize = mTargetCache.size();
	if( targsize!= chan.mTargetCache.size() ) return;
	for(int i=0; i<targsize; i++) {
		mTargetCache[i].mTargetPercent = chan.mTargetCache[i].mTargetPercent;
	}
}

// This = operator does everythinig BUT transfer paramblock references
void morphChannel::operator=(const morphChannel& from)
{
	// Don't allow self->self assignment
	if(&from == this) return;

	mNumPoints = from.mNumPoints;

	mPoints = from.mPoints;
	mDeltas = from.mDeltas;
	mWeights = from.mWeights;

	mTargetCache = from.mTargetCache;

	mSel = from.mSel;

	mActive = from.mActive;
	mActiveOverride = from.mActiveOverride;
	mConnection = from.mConnection;
	mCurvature = from.mCurvature;
	mInvalid = from.mInvalid;
	mModded = from.mModded;
	mName = from.mName;
	mNumProgressiveTargs = from.mNumProgressiveTargs;
	mUseLimit = from.mUseLimit;
	mUseSel = from.mUseSel;
	mSpinmin = from.mSpinmin;
	mSpinmax = from.mSpinmax;
	
	mp = from.mp;
	iTargetListSelection = from.iTargetListSelection;
}

float morphChannel::GetTargetPercent(const int &which)
{
	if(which<-1 || which>=mNumProgressiveTargs) return 0.0f;
	if(which==-1) return mTargetPercent;
	return mTargetCache[which].mTargetPercent;
}

void morphChannel::ReNormalize()
{
	mTargetPercent= 100.0f/(float)(1+mNumProgressiveTargs);
	for(int i=0; i<mNumProgressiveTargs; i++)
	{
		mTargetCache[i].mTargetPercent=(2+i)*mTargetPercent;
	}
}

void morphChannel::SetUpNewController()
{
	int in, out;
	GetBezierDefaultTangentType(in, out);
	SetBezierDefaultTangentType(BEZKEY_STEP, BEZKEY_STEP);
	Control *c = (Control*)CreateInstance(CTRL_FLOAT_CLASS_ID,GetDefaultController(CTRL_FLOAT_CLASS_ID)->ClassID());
	cblock->SetValue(0,0,0.0f);
	cblock->SetController(0,c);
	SetBezierDefaultTangentType(in, out);
}

// Reconstruct the optimization malarky using the current channel's point info
void morphChannel::rebuildChannel()
{
	int x,id = 0;
	Point3 DeltP;
	double wtmp;
	Point3 tVert;

	int tPc = mNumPoints;
	if(tPc!=mp->MC_Local.Count) goto CantLoadThis;
	if(!mp->MC_Local.CacheValid) goto CantLoadThis;

	mInvalid = FALSE;

	for(x=0;x<tPc;x++)
	{
		tVert = mPoints[x];
		wtmp = mWeights[x];

		// calculate the delta cache
		DeltP.x=(tVert.x-mp->MC_Local.oPoints[x].x)/100.0f;
		DeltP.y=(tVert.y-mp->MC_Local.oPoints[x].y)/100.0f;
		DeltP.z=(tVert.z-mp->MC_Local.oPoints[x].z)/100.0f;
		mDeltas[x] = DeltP;

	}

	CantLoadThis:
	tPc=0;
}

// Generate all the optimzation and geometry data
void morphChannel::buildFromNode( INode *node , BOOL resetTime, TimeValue t, BOOL picked )
{
	if(resetTime) t = GetCOREInterface()->GetTime();

	ObjectState os = node->EvalWorldState(t);

	int tPc = os.obj->NumPoints();
	int x,id = 0;
	Point3 DeltP;
	double wtmp;
	Point3 tVert;

	if(tPc!=mp->MC_Local.Count) {
		mNumPoints = 0;
		mActive = FALSE;
		mInvalid = TRUE;
		goto CantLoadThis;
	}
	if(!mp->MC_Local.CacheValid) goto CantLoadThis;


	mInvalid = FALSE;

	// if the channel hasn't been edited yet, change the 'empty'
	// name to that of the chosen object.
	if( !mModded || picked) mName = node->GetName();

	// Set the data into the morphChannel
	mActive = TRUE;
	mModded = TRUE;

	
	// Prepare the channel
	AllocBuffers(tPc, tPc);
	mSel.SetSize(tPc);
	mSel.ClearAll();


	mNumPoints = 0;

	for(x=0;x<tPc;x++)
	{
		tVert = os.obj->GetPoint(x);
		wtmp = os.obj->GetWeight(x);

		// calculate the delta cache
		DeltP.x=(tVert.x-mp->MC_Local.oPoints[x].x)/100.0f;
		DeltP.y=(tVert.y-mp->MC_Local.oPoints[x].y)/100.0f;
		DeltP.z=(tVert.z-mp->MC_Local.oPoints[x].z)/100.0f;
		mDeltas[x] = DeltP;

		mWeights[x] = os.obj->GetWeight(x);
		mSel.Set( x, os.obj->IsPointSelected(x)?1:0);

		mPoints[x] = tVert;
		mNumPoints++;
	}


	// Update *everything*
	mp->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	mp->NotifyDependents(FOREVER,PART_ALL,REFMSG_SUBANIM_STRUCTURE_CHANGED);
	mp->Update_channelFULL();
	mp->Update_channelParams();

	CantLoadThis:
	tPc=0;
}
