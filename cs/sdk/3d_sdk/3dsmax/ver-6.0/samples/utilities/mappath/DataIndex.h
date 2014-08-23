/**********************************************************************
 *<
	FILE..........: DataIndex.h

	DESCRIPTION...: A Singleton index of the current data type stored in list

	CREATED BY....: David Cunningham

	HISTORY.......: Created September 4, 2001

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef __MAPPATHDATAINDEX__
#define __MAPPATHDATAINDEX__

#include <lslights.h>

/**
 * This class acts a data index for the current contents of the 
 * mappath editor list.
 * Also keeps a list of lights in the scene, to ease path functions.
 */

class DataIndex	{
public:
	enum ItemType {
			INVALID,
			TEXMAP,
			PHOTOMETRIC,
			DXMATERIAL,
		};
	virtual ~DataIndex();
	static DataIndex* Instance();
	void Store(int index, ItemType type);
	void Reinitialize();
	ItemType Get(int index) const;
	void AddLight(LightscapeLight*);
	int GetLightCount() const { return lightCount; }
	void SetPathOnLights(TCHAR* oldPath, TCHAR* newPath);

	INodeTab nodeTab;

protected:
	DataIndex();


private:
	static DataIndex* instance;
	ItemType* dataIndex;
	int maxCount;
	int currCount;
	// an array of pointers to lights
	LightscapeLight** lights;
	int lightMax;
	int lightCount;
	

};

#endif

