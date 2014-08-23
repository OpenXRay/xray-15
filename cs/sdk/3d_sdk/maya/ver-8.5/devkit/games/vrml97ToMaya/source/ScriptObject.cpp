//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  ScriptObject.cpp
//  An abstract class to encapsulate the interface between the VRML97
//  scene graph and the supported scripting languages.
//

#include "config.h"
#include "ScriptObject.h"
#include "Doc.h"
#include "VrmlMFString.h"
#include "VrmlNodeScript.h"
#include "VrmlScene.h"
#include <string.h>

// JavaScript
#if HAVE_JAVASCRIPT
# include "ScriptJS.h"
#endif

// Java via Sun JDK
#if HAVE_JDK
# include "ScriptJDK.h"
#endif


ScriptObject::ScriptObject() {}

ScriptObject::~ScriptObject() {}

ScriptObject *ScriptObject::create( VrmlNodeScript *node,
				    VrmlMFString &url )
{
  // Try each url until we find one we like
  int i, n = url.size();
  for (i=0; i<n; ++i)
    {
      if (! url[i]) continue;

      // Get the protocol & mimetype...

#if HAVE_JAVASCRIPT
      // Need to handle external .js files too...
      if (strncmp(url[i], "javascript:", 11) == 0 ||
	  strncmp(url[i], "vrmlscript:", 11) == 0 )
	{
	  return new ScriptJS( node, url[i]+11 );
	}
#endif

#if HAVE_JDK
      int slen = strlen(url[i]);
      if (slen > 6 &&
	  (strcmp(url[i]+slen-6,".class") == 0 ||
	   strcmp(url[i]+slen-6,".CLASS") == 0))
	{
	  Doc *relative = 0;
	  if ( node->scene() )
	    relative = node->scene()->url();
	  Doc doc( url[i], relative );
	  if ( doc.localName() )
	    return new ScriptJDK( node, doc.urlBase(), doc.localPath() );
	}
#endif
    }

  return 0;
}

    
