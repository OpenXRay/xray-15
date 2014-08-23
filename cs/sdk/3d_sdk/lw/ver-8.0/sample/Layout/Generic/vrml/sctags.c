// SCTags.c -- Scene Comment/Tag utility Functions
// 2/10/98

#include <lwtypes.h>
#include <lwrender.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sctags.h"

static char TagBuf[TAG_SIZE+1] = "";

int countTags(LWItemInfo *itinfo, LWItemID id)	 // for the next version
{
	int num=1;
	while( (*itinfo->getTag)(id,num) ) 	// using num=0 would create a tag
		num++;
	return num-1;
}

// return index of item in list, 
int setItemTag(LWItemInfo *itinfo, LWItemID id, char *key, char *val)
{
	int t,n=0,i;
	const char *tag;
	sprintf(TagBuf,"%s=", key);
	strncat(TagBuf, val, TAG_SIZE);
	for(t=0; t<MAX_TAGS; t++)	// store tag changes for current item
	{
		if( (tag=(*itinfo->getTag)(id,t+1)) )
		{
			n++;
			i = strlen(key);
			if( !strncmp(tag,key,i) )
			{
				(*itinfo->setTag)(id,t+1,TagBuf);
				return n;
			}
		}
		else // end of existing tags
		{
			n++;
			(*itinfo->setTag)(id,0,TagBuf);	// add tag
			return n;
		}
	}
	return n;
}

int getItemTag(LWItemInfo *itinfo, LWItemID id, char *key, char *val, int len)
{
	int t,n=0,i;
	const char *tag;
	*val = 0;
	for(t=0; t<MAX_TAGS; t++)	// store tag changes for current item
	{
		if( (tag=(*itinfo->getTag)(id,t+1)) )
		{
			n++;
			i = strlen(key);
			if(!strncmp(tag,key,i))
			{
				//i++; // skip '='
				strncpy(val,&(tag[i]),len-i); // tag = "KEY="tag[i+1]
				return n;
			}
		}
	}
	return 0;
}

// get Nth tag with key
int getItemTagN(LWItemInfo *itinfo, LWItemID id, char *key, int indx, char *val, int len)
{
	int t,n=0,i, count=0;
	const char *tag;
	*val = 0;
	for(t=0; t<MAX_TAGS; t++)	// store tag changes for current item
	{
		if( (tag=(*itinfo->getTag)(id,t+1)) )
		{
			n++;
			i = strlen(key);
			if(!strncmp(tag,key,i))
			{
				if(count==indx)
				{
					strncpy(val,&(tag[i]),len-i); // tag = "KEY="tag[i+1]
					return n;
				}
				count++;
			}
		}
	}
	return 0;
}

int findItemTag(LWItemInfo *itinfo, LWItemID id, char *key)
{
	int t=1,i;
	const char *tag;
	while( (tag=(*itinfo->getTag)(id,t)) )
	{
		i = strlen(key);
		if(!strncmp(tag,key,i))
		{
			return t;
		}
		t++;
	}
	return 0;
}

void killItemTag(LWItemInfo *itinfo, LWItemID id, char *key)
{
	int t=1,i;
	const char *tag;
	while( (tag=(*itinfo->getTag)(id,t)) )
	{
		i = strlen(key);
		if(!strncmp(tag,key,i))
		{
			(*itinfo->setTag)(id,t,"");
			return;
		}
		t++;
	}
	return;
}
