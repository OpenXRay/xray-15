/* -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

   FILE: selLum.cpp

	 DESCRIPTION: luminance selection type - class definitions

	 CREATED BY: michael malone (mjm)

	 HISTORY: created November 4, 1998

	 Copyright (c) 1998, All Rights Reserved

// -----------------------------------------------------------------------------
// -------------------------------------------------------------------------- */

// precompiled header
#include "pch.h"

// local includes
#include "selLum.h"
#include "..\blurMgr.h"


SelLum::~SelLum()
{
	if (mp_radMap)
		delete[] mp_radMap;
}

void SelLum::notifyPrmChanged(int prmID)
{
	switch (prmID)
	{
		case prmLumMin:
		case prmLumMax:
			m_selectValid = m_featherValid = m_compValid = false;
			break;
		case prmLumFeathRad:
		{
			for (int i=0; i<m_imageSz; i++)
			{
				if ( mp_radMap[i] > 0.0f )
					mp_radMap[i] = 1.0f;
			}
			m_featherValid = m_compValid = false;
			break;
		}
		case prmLumBrighten:
		case prmLumBlend:
			m_compValid = false;
			break;
	}
}

bool SelLum::doSelect()
{
	float lum;
	int mapIndex, srcIndex;
	mp_nameCurProcess = GetString(IDS_PROCESS_SELECT);

	int interval = 3*m_imageW;
	for (mapIndex=0; mapIndex<m_imageSz; mapIndex++)
	{
		srcIndex = mapIndex*3;
		lum = luminanceNormFloat(mp_srcMap[srcIndex], mp_srcMap[srcIndex+1], mp_srcMap[srcIndex+2]); // grabs specular hilights better than max(r,g,b)
		if ((lum >= m_min) && (lum <= m_max))
			mp_radMap[mapIndex] = 0.0f;
		else
			mp_radMap[mapIndex] = 1.0f;
		if ( ( (mapIndex%interval) == 0 ) && mp_blurMgr->progress(mp_nameCurProcess, mapIndex, m_imageSz) )
			return false;
	}
	return true;
}


//#undef WINDOW_THREAD
#define WINDOW_THREAD

#ifdef WINDOW_THREAD
// Windows thread version
struct ThreadParams
{
	HANDLE startEvent;
	HANDLE numDoneEvent;
	int num;
	int x, y;
	int u, v;
	int index, normRadIdx;
	int startv;
	int Maxv;
	int step;
	int m_feathRad;
	int mapIndex;
	int m_imageW;
	int m_imageH;
	int m_normRadMaskW;
	float *mp_radMap;
	float *m_normRadMask;
	int terminateThread;
	int padding[12]; // padding to 128 bytes
	
};


// blurThread should become member function of the class SelLum
void blurThread(void *parameter) 
{
	ThreadParams *p = (ThreadParams*) parameter;
	
	while(1){
		WaitForSingleObject(p->startEvent,INFINITE);
		if (p->terminateThread)		// Thread should terminate
			break;
		for(p->v = p->startv; p->v <= p->Maxv; p->v += p->step){
			for (p->u =- p->m_feathRad; p->u <= p->m_feathRad; p->u++){
				if ( !inBounds(p->x+p->u, p->y+p->v, p->m_imageW, p->m_imageH) )
						continue;
				// compute offsets
				p->index = (p->mapIndex + p->v*p->m_imageW + p->u);
				
				if (p->mp_radMap[p->index] != 0.0f){
					p->normRadIdx = (p->u+p->m_feathRad) + (p->v+p->m_feathRad)*p->m_normRadMaskW;
					p->mp_radMap[p->index] = min(p->mp_radMap[p->index], p->m_normRadMask[p->normRadIdx]);
				}
			} // for u
		}// for i
		
		SetEvent(p->numDoneEvent);
	}// while(1)

}



// launch_threads should become member function of the class SelLum
int launch_threads(int num_processors, HANDLE *hThreads, HANDLE *numDoneEvent,
				   struct ThreadParams *threadParams)
{
	
	DWORD threadID;

	for(int i = 0; i < num_processors; i++){

		threadParams[i].startEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
		numDoneEvent[i] = CreateEvent(NULL,FALSE,FALSE,NULL);
		threadParams[i].numDoneEvent = numDoneEvent[i];
		threadParams[i].terminateThread = false;
		hThreads[i] = CreateThread(
				NULL,		// LPSECURITY_ATTRIBUTES lpsa
				0,			// DWORD cbStack
				(LPTHREAD_START_ROUTINE)blurThread,
				(LPVOID)(&threadParams[i]),
				0, 			// 
				&threadID);

	}

	return 0;

}



bool SelLum::doFeather()
{
	mp_nameCurProcess = GetString(IDS_PROCESS_FEATHER);

	// NOTE: there is _alot_ of room for optimization in this function
	// compute normalized radii mask for feathered pixels
	if (m_normRadMaskW != 2*m_feathRad + 1) {
		if (m_normRadMask)
			delete[] m_normRadMask;
		m_normRadMaskW = m_normRadMaskH = 2*m_feathRad + 1;
		m_normRadMask = new float[m_normRadMaskW*m_normRadMaskH];
		int index = -1;
		float x_dist, y_dist;
		
		for (int i=0; i<m_normRadMaskW; i++) {
			for (int j=0; j<m_normRadMaskH; j++) {
				x_dist = (float)i-m_feathRad;
				y_dist = (float)j-m_feathRad;
				m_normRadMask[++index] = (float)(sqrt(sqr(x_dist)+sqr(y_dist)) / (float)m_feathRad);
				m_normRadMask[index] = (m_normRadMask[index] > 1.0f) ? 1.0f : m_normRadMask[index];
			}
		}
	}


	// find feathered pixels
	//int index, normRadIdx; 
	int interval = 3*m_imageW;

// Introduce Window threading
	SYSTEM_INFO si;
	int num_processors;
	GetSystemInfo(&si);

	num_processors = ( GetCOREInterface()->GetRendMultiThread() ? si.dwNumberOfProcessors : 1 );

	HANDLE *hThreads = new HANDLE[num_processors];
	HANDLE *numDoneEvent = new HANDLE[num_processors];
	struct ThreadParams *threadParams = new struct ThreadParams[num_processors];

	int startv = -m_feathRad;
	int inc = (2*m_feathRad+1)/num_processors;

	for(int i = 0; i < num_processors; i++){
		threadParams[i].m_feathRad = m_feathRad;
		threadParams[i].m_imageW = m_imageW;
		threadParams[i].m_imageH = m_imageH;
		threadParams[i].m_normRadMaskW = m_normRadMaskW;
		threadParams[i].mp_radMap = mp_radMap;
		threadParams[i].m_normRadMask = m_normRadMask;
		threadParams[i].num = i;

		threadParams[i].step = 1;
		threadParams[i].startv = startv; 
		if(i == num_processors - 1)
			threadParams[i].Maxv = m_feathRad;
		else
			threadParams[i].Maxv = startv + inc - 1;
		startv = startv + inc;
		
	}

	launch_threads(num_processors, hThreads,numDoneEvent, threadParams);

	bool returnValue = true;
	for (int mapIndex =0; mapIndex < m_imageSz; mapIndex++){
		if (mp_radMap[mapIndex] == 0.0f){ // a selected pixel
			int y = mapIndex / m_imageW;
			int x = mapIndex % m_imageW;

			// skip pixel if surrounded by other selected pixels
			if (
				 ( ( x>0 )			&& (mp_radMap[mapIndex-1] == 0.0f) ) &&
				 ( ( x<m_imageW-1 ) && (mp_radMap[mapIndex+1] == 0.0f) ) &&
				 ( ( y>0 )			&& (mp_radMap[mapIndex-m_imageW] == 0.0f) ) &&
				 ( ( y<m_imageH-1 ) && (mp_radMap[mapIndex+m_imageW] == 0.0f) )
			   )
			   continue;
			// set offset and weight of the feathered pixels
		
			// spit the works among the threads
			for(int i= 0; i < num_processors; i++){
				threadParams[i].x = x;
				threadParams[i].y = y;
				threadParams[i].mapIndex = mapIndex;
				SetEvent(threadParams[i].startEvent);
			}

			WaitForMultipleObjects(num_processors, numDoneEvent, TRUE,INFINITE);

		} // if (mp_radMap[mapIndex] == 0.0f)

		if ( ( (mapIndex%interval) == 0 ) && mp_blurMgr->progress(mp_nameCurProcess, mapIndex, m_imageSz) ) {
			returnValue = false;
			break;
		}
	} // for mapIndex loop

	// Signal the threads to terminate.
	for(i = 0; i < num_processors; i++){
		threadParams[i].terminateThread = true;
		SetEvent(threadParams[i].startEvent);
	}

	// Wait for them to finish.
	DWORD wait = WaitForMultipleObjects(num_processors, hThreads, true, 5000);
	DbgAssert(wait != WAIT_TIMEOUT);

	// Close everything, terminate threads that hung.
	for(i = 0; i < num_processors; i++){
		CloseHandle(threadParams[i].startEvent);
		CloseHandle(threadParams[i].numDoneEvent);
		TerminateThread(hThreads[i],0);	 
		CloseHandle(hThreads[i]);
		hThreads[i] = NULL;
	}

	delete []hThreads;
	delete []numDoneEvent;
	delete []threadParams;

	return returnValue;
}
 

#else //#ifdef WINDOW_THREAD

// Original version

bool SelLum::doFeather()
{
	mp_nameCurProcess = GetString(IDS_PROCESS_FEATHER);

	// NOTE: there is _alot_ of room for optimization in this function

	// compute normalized radii mask for feathered pixels
	if (m_normRadMaskW != 2*m_feathRad + 1) {
		if (m_normRadMask)
			delete[] m_normRadMask;
		m_normRadMaskW = m_normRadMaskH = 2*m_feathRad + 1;
		m_normRadMask = new float[m_normRadMaskW*m_normRadMaskH];
		int index = -1;
		float x_dist, y_dist;
		for (int i=0; i<m_normRadMaskW; i++) {
			for (int j=0; j<m_normRadMaskH; j++) {
				x_dist = (float)i-m_feathRad;
				y_dist = (float)j-m_feathRad;
				m_normRadMask[++index] = (float)(sqrt(sqr(x_dist)+sqr(y_dist)) / (float)m_feathRad);
				m_normRadMask[index] = (m_normRadMask[index] > 1.0f) ? 1.0f : m_normRadMask[index];
			}
		}
	}

	// find feathered pixels
	int index, normRadIdx;
	int interval = 3*m_imageW;
	for (int mapIndex=0; mapIndex<m_imageSz; mapIndex++)
	{
		if (mp_radMap[mapIndex] == 0.0f) // a selected pixel
		{
			int y = mapIndex / m_imageW;
			int x = mapIndex % m_imageW;

			// skip pixel if surrounded by other selected pixels
			// lots of opportunity to optimize, but no time
			if (
				 ( ( x>0 )			&& (mp_radMap[mapIndex-1] == 0.0f) ) &&
				 ( ( x<m_imageW-1 ) && (mp_radMap[mapIndex+1] == 0.0f) ) &&
				 ( ( y>0 )			&& (mp_radMap[mapIndex-m_imageW] == 0.0f) ) &&
				 ( ( y<m_imageH-1 ) && (mp_radMap[mapIndex+m_imageW] == 0.0f) )
			   )
			   continue;
			
			// set offset and weight of the feathered pixels
			for (int v=-m_feathRad; v<=m_feathRad; v++)
			{
				for (int u=-m_feathRad; u<=m_feathRad; u++)
				{
					if ( !inBounds(x+u, y+v, m_imageW, m_imageH) )
						continue;

					// compute offsets
					index = (mapIndex + v*m_imageW + u);
					if (mp_radMap[index] != 0.0f)
					{
						normRadIdx = (u+m_feathRad) + (v+m_feathRad)*m_normRadMaskW;
						mp_radMap[index] = min(mp_radMap[index], m_normRadMask[normRadIdx]);
					}
				}
			}
		}
		if ( ( (mapIndex%interval) == 0 ) && mp_blurMgr->progress(mp_nameCurProcess, mapIndex, m_imageSz) )
			return false;
	}
	return true;
}

#endif  


float SelLum::calcBrighten(TimeValue t, const float &normRadius)
{
	return m_brighten * mp_blurMgr->getBrightenCurve()->GetValue(t, normRadius, FOREVER, TRUE);
}

float SelLum::calcBlend(TimeValue t, const float &normRadius)
{
	return m_blend * mp_blurMgr->getBlendCurve()->GetValue(t, normRadius, FOREVER, TRUE);
}

bool SelLum::doComposite(TimeValue t, CompMap &compMap)
{
	for (int mapIndex=0; mapIndex<m_imageSz; mapIndex++)
	{
		if ( mp_radMap[mapIndex] < 1.0f )
		{
			compMap[mapIndex].brighten = max( compMap[mapIndex].brighten, calcBrighten(t, mp_radMap[mapIndex]) );
			compMap[mapIndex].blend = max( compMap[mapIndex].blend, calcBlend(t, mp_radMap[mapIndex]) );
		}
	}
	return true;
}

void SelLum::select(TimeValue t, CompMap &compMap, Bitmap *bm, RenderGlobalContext *gc)
{
	float fTemp;
	int type;

	mp_srcAlpha = (WORD*)bm->GetAlphaPtr(&type);
	mp_srcMap = (WORD*)bm->GetStoragePtr(&type);
	assert(type == BMM_TRUE_48);

	// if source bitmap has changed since last call
	if ( m_lastBMModifyID != bm->GetModifyID() )
	{
		m_lastBMModifyID = bm->GetModifyID();
		m_selectValid = m_featherValid = m_compValid = false;
		if ( (bm->Width() != m_imageW) || (bm->Height() != m_imageH) )
		{
			m_imageW = bm->Width();
			m_imageH = bm->Height();
			m_imageSz = m_imageW * m_imageH;
			if (m_imageSz > m_mapSz)
			{
				if (mp_radMap)
					delete[] mp_radMap;
				mp_radMap = new float[m_imageSz];
				m_mapSz = m_imageSz;
			}
		}
	}

	if (!m_selectValid)
	{
		mp_blurMgr->getSelValue(prmLumMin, t, fTemp, FOREVER);
		LimitValue(fTemp, 0.0f, 100.0f); // mjm - 9.30.99
		m_min = fTemp*PERCENT2DEC;

		mp_blurMgr->getSelValue(prmLumMax, t, fTemp, FOREVER);
		LimitValue(fTemp, 0.0f, 100.0f); // mjm - 9.30.99
		m_max = fTemp*PERCENT2DEC;

		m_selectValid = doSelect();
	}
	
	if (!m_featherValid)
	{
		mp_blurMgr->getSelValue(prmLumFeathRad, t, fTemp, FOREVER);
		LimitValue(fTemp, 0.0f, 1000.0f); // mjm - 9.30.99
		m_feathRad = (int)floor(max(m_imageW, m_imageH)*(fTemp*PERCENT2DEC));
		m_featherValid = SelLum::doFeather();
	}

	mp_blurMgr->getSelValue(prmLumBrighten, t, fTemp, FOREVER);
	LimitValue(fTemp, 0.0f, 1000.0f); // mjm - 9.30.99
	m_brighten = fTemp*PERCENT2DEC;

	mp_blurMgr->getSelValue(prmLumBlend, t, fTemp, FOREVER);
	LimitValue(fTemp, 0.0f, 100.0f); // mjm - 9.30.99
	m_blend = fTemp*PERCENT2DEC;

	m_compValid = SelLum::doComposite(t, compMap);
}
