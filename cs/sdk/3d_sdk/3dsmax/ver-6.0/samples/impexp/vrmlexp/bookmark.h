/**********************************************************************
 *<
    FILE: bookmark.h
 
    DESCRIPTION:  Access to the URL bookmark
 
    CREATED BY: Charles Thaeler
  
    HISTORY: created 26 Mar. 1996
 
 *> Copyright (c) 1996, All Rights Reserved.
 **********************************************************************/
 
#ifndef __BOOKMARK__H__
 
#define __BOOKMARK__H__

#define VRMBLIO_INI_FILE _T("vrblio.ini")
#define BOOKMARK_SECTION _T("Bookmarks")

TCHAR * ExportIniFilename(Interface* ip);

extern void SplitURL(TSTR in, TSTR *url, TSTR *camera);
extern int GetBookmarkURL(Interface *ip, TSTR *u, TSTR *c, TSTR *d);

#endif
