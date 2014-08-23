/*===========================================================================*\
 | 
 |  FILE:	wM3_undo.cpp
 |			Weighted Morpher for MAX R3
 |			RestoreObj for deleting morph channels
 | 
 |  AUTH:   Harry Denholm
 |			Copyright(c) Kinetix 1999
 |			All Rights Reserved.
 |
 |  HIST:	Started 17-2-99
 | 
\*===========================================================================*/

#include "wM3.h"



/*===========================================================================*\
 | Restore_TargetMove Class
\*===========================================================================*/

// Constructor
Restore_FullChannel::Restore_FullChannel(MorphR3 *mpi, const int idx, const BOOL upd) 
{ 
	targpercents_undo = NULL;
	targpercents_redo = NULL;
	mp = mpi;
	mcIndex = idx;
	undoMC = mp->chanBank[idx];
	update = upd;
	ntargs_undo = mp->chanBank[idx].mNumProgressiveTargs+1;
	targpercents_undo = new float[ntargs_undo];
	targpercents_undo[0] = mp->chanBank[idx].mTargetPercent;
	for(int j=1; j<ntargs_undo; j++) targpercents_undo[j] = mp->chanBank[idx].mTargetCache[j-1].mTargetPercent;
}

Restore_FullChannel::~Restore_FullChannel() 
{ 
	if(targpercents_undo) delete [] targpercents_undo;
	if(targpercents_redo) delete [] targpercents_redo;
}

// Called when Undo is selected
void Restore_FullChannel::Restore(int isUndo) 
{
	morphChannel &bank =mp->chanBank[mcIndex];
	if (isUndo) {
		redoMC = bank;
		ntargs_redo = bank.mNumProgressiveTargs+1;
		targpercents_redo = new float[ntargs_redo];
		targpercents_redo[0] = bank.mTargetPercent;
		for(int j=1; j<ntargs_redo; j++) targpercents_redo[j] = 
			bank.mTargetCache[j-1].mTargetPercent;
	}

	bank = undoMC;
	bank.mTargetPercent = targpercents_undo[0];
	for(int j=1; j<ntargs_undo; j++) 
		bank.mTargetCache[j-1].mTargetPercent = targpercents_undo[j];

	bank.rebuildChannel();
	mp->Update_channelFULL();
	mp->Update_channelParams();

	mp->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	//mp->ip->RedrawViews(mp->ip->GetTime());
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
}

// Called when Redo is selected
// since my undo-er is only for deleting channels
// if you redo, you're deleting a channel. So i do that here.
void Restore_FullChannel::Redo() 
{
	morphChannel &bank =mp->chanBank[mcIndex];

	bank = redoMC;
	bank.mTargetPercent = targpercents_redo[0];
	for(int j=1; j<ntargs_redo; j++) 
		bank.mTargetCache[j-1].mTargetPercent = targpercents_redo[j];

}


// Called to return the size in bytes of this RestoreObj
int Restore_FullChannel::Size() {
	return 0;
}

// markerRestore methods **************************************
Restore_Marker::Restore_Marker(MorphR3 *mpi)
{
	mIndex = mpi->markerIndex;
	mName = mpi->markerName;
	markerSel = mpi->markerSel;
	mp = mpi;
}


void Restore_Marker::Restore(int isUndo) 
{
	if(isUndo) 
	{
		Tab<int>			rIndex = mp->markerIndex;
		NameTab				rName = mp->markerName;
		int					rSel = mp->markerSel;

		mp->markerIndex = mIndex;
		mp->markerName = mName;
		mp->markerSel = markerSel;

		mIndex = rIndex;
		mName = rName;
		markerSel = rSel;

		mp->Update_channelMarkers();
	}


}

void Restore_Marker::Redo() 
{

	mp->markerIndex = mIndex;
	mp->markerName = mName;
	mp->markerSel = markerSel;

	mp->Update_channelMarkers();

}


int Restore_Marker::Size() {
	return 1;
}

Restore_CompactChannel::Restore_CompactChannel(MorphR3 *mpi, Tab<int> &targ, Tab<int> &src)
{
	mtarg = targ; msrc = src; mp = mpi;
}

void Restore_CompactChannel::Restore(int isUndo)
{
	morphChannel storechan;
	for(int i=0;i<mtarg.Count();i++)
	{
		mp->ChannelOp(msrc[i],mtarg[i],OP_MOVE);
	}

	mp->Update_channelFULL();
	mp->Update_channelParams();	
	mp->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	mp->NotifyDependents(FOREVER,PART_ALL,REFMSG_SUBANIM_STRUCTURE_CHANGED);
}

void Restore_CompactChannel::Redo()
{
	for(int i=0;i<mtarg.Count();i++)
	{
		mp->ChannelOp(mtarg[i],msrc[i],OP_MOVE);
	}
	mp->Update_channelFULL();
	mp->Update_channelParams();	
	mp->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	mp->NotifyDependents(FOREVER,PART_ALL,REFMSG_SUBANIM_STRUCTURE_CHANGED);
}

void Restore_TargetMove::Restore(int isUndo)
{
	mp->SwapTargets(to, from, true);
}

void Restore_TargetMove::Redo()
{
		mp->SwapTargets(from, to, true);
}

	// Constructor
Restore_Display::Restore_Display(MorphR3 *mpi) { mp = mpi; }
	
	// Called when Undo is selected
void Restore_Display::Restore(int isUndo) { mp->Update_channelFULL(); mp->Update_channelParams();}

	// Called when Redo is selected
void Restore_Display::Redo() { mp->Update_channelFULL(); mp->Update_channelParams();}
int Restore_Display::Size() {
	return 1;
}


