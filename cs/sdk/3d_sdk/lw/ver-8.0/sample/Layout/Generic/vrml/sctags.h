// SCTags.h

#define MAX_TAGS	255 // Vastly more than necessary... ;}
#define TAG_SIZE	255
// return index of item in list, 
int countTags(LWItemInfo *itinfo, LWItemID id);	
int setItemTag(LWItemInfo *itinfo, LWItemID id, char *key, char *val);
int getItemTag(LWItemInfo *itinfo, LWItemID id, char *key, char *val, int len);
int getItemTagN(LWItemInfo *itinfo, LWItemID id, char *key, int N,char *val, int len);
int findItemTag(LWItemInfo *itinfo, LWItemID id, char *key);
void killItemTag(LWItemInfo *itinfo, LWItemID id, char *key);

