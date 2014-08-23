/**********************************************************************
 *<
	FILE..........: DataIndex.h

	DESCRIPTION...: A Singleton index implementation of the current data type stored in list

	CREATED BY....: David Cunningham

	HISTORY.......: Created September 4, 2001

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "DataIndex.h"
#include "mappath.h"
#include "dbgprint.h"

const int DEFAULT_MAX = 512;


DataIndex* DataIndex::instance = NULL;

/**
 * Return singleton instance, create it if it doesn't exist.
 */

DataIndex* DataIndex::Instance()	{
	if(!instance)	{
		instance = new DataIndex;
	}
	return instance;
}

DataIndex::DataIndex()  	{
	maxCount = DEFAULT_MAX;
	currCount = 0;
	dataIndex = new ItemType[maxCount];
	for(int i=0;i<maxCount;i++)	
		dataIndex[i] = DataIndex::INVALID;

	lightMax = DEFAULT_MAX;
	lightCount = 0;
	lights = new LightscapeLight* [lightMax];
	for(i=0;i<lightMax;i++)	
		lights[i] = NULL;
}

DataIndex::~DataIndex()	{
	delete [] dataIndex;
}

/**
 * Reinitialize list.  Should be done in RefCount::EndEditParams.  This is necessary since this
 * is a singleton and remains in the system even if MapPath is turned off.
 */

void DataIndex::Reinitialize()	{
	maxCount = DEFAULT_MAX;
	currCount = 0;
	delete [] dataIndex;
	dataIndex = new ItemType[maxCount];
	for(int i=0;i<maxCount;i++)	
		dataIndex[i] = DataIndex::INVALID;
	// reinitialize lights list
	delete [] lights;
	lightMax = DEFAULT_MAX;
	lightCount = 0;
	lights = new LightscapeLight* [lightMax];
	for(i=0;i<lightMax;i++)	
		lights[i] = NULL;
	nodeTab.ZeroCount();		
}

/**
 * Returns datatype at specific index.  Returns INVALID if out of bounds.
 */

DataIndex::ItemType DataIndex::Get(int index) const	{
	if(index < maxCount && index >= 0)	
		return dataIndex[index];
	else
		return DataIndex::INVALID;
}

/**
 * Store dataType in list.  This list widens dynamically, if needed.
 */

void DataIndex::Store(int index, DataIndex::ItemType type)	{
	if(index >=0)	{
		int tempCount = maxCount;
		while(index > maxCount)	{
			maxCount *=2;
		}
		// widen list, if it needs it
		if(tempCount != maxCount)	{
			ItemType* newList = new ItemType[maxCount];
			for(int i=0;i<tempCount;i++)	{
				newList[i] = dataIndex[i];
			}
			for(i=tempCount; i<maxCount;i++)
				newList[i] = DataIndex::INVALID;
			delete [] dataIndex;
			dataIndex = newList;
		}
		currCount++;
		// store type
		dataIndex[index] = type;
		return;
	}
}

/**
 * Adds a light pointer to list, and widens list if necessary.
 */

void DataIndex::AddLight(LightscapeLight* theLight)	{
	if(theLight)	{
		// widen list if necessary
		if(lightCount == lightMax - 1)	{
			
			int newMax = lightMax*2;
			LightscapeLight** newList = new LightscapeLight*[newMax];
			for(int i=0;i<lightMax; i++)	
				newList[i] = lights[i];
			for(i=lightMax;i<newMax;i++)	
				newList[i] = NULL;
			lightMax = newMax;
			delete [] lights;
			lights = newList;
		}
		
		lights[lightCount] = theLight;
		lightCount++;
	}
}

/**
 *  This function runs through the dataIndex's list of LS lights looking for matches for the
 *  old path and sets those lights to the new path.
 */
		
void DataIndex::SetPathOnLights(TCHAR* oldPath, TCHAR* newPath)	{
	// don't zero out paths
	try {
		if(oldPath[0] && newPath[0])	{
			for(int i=0;i<lightCount;i++)	{

				if(lights[i] && !_tcscmp(oldPath, lights[i]->GetWebFileName()))	{
					lights[i]->SetWebFileName(newPath);
				}
			}
		}
	} catch(...)	{
		DebugPrint("Access Error in DataIndex::SetPathOnLights.\n");
	}
}

			






