//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
// VrmlScene.cpp
//

#if defined AW_NEW_IOSTREAMS
#  include <iostream>
#else
#  include <iostream.h>
#endif

#include "config.h"

#include "VrmlScene.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "Doc.h"
#include "Viewer.h"
#include "System.h"

#include "VrmlNamespace.h"
#include "VrmlNodeType.h"


// Handle clicks on Anchor nodes
#include "VrmlNodeAnchor.h"

// Bindable children nodes
#include "VrmlNodeBackground.h"
#include "VrmlNodeFog.h"
#include "VrmlNodeNavigationInfo.h"
#include "VrmlNodeViewpoint.h"

// List of scene-scoped lights in the scene
#include "VrmlNodePointLight.h"
#include "VrmlNodeSpotLight.h"

// List of Movies in the scene
#include "VrmlNodeMovieTexture.h"

// List of AudioClips in the scene
#include "VrmlNodeAudioClip.h"

// List of Scripts in the scene
#include "VrmlNodeScript.h"

// List of TimeSensors in the scene
#include "VrmlNodeTimeSensor.h"

// Max time in seconds between updates. Make this user
// setable to balance performance with cpu usage.
#ifndef DEFAULT_DELTA
# define DEFAULT_DELTA 0.5
#endif


//
// Create a VrmlScene from a URL (optionally loading from a local copy,
// so I can run as a netscape helper but still retrieve embedded urls).
//

VrmlScene::VrmlScene( const char *sceneUrl, const char *localCopy ) :
  d_url(0),
  d_urlLocal(0),
  d_namespace(0),
  d_modified(false),
  d_newView(false),
  d_deltaTime(DEFAULT_DELTA),
  d_pendingUrl(0),
  d_pendingParameters(0),
  d_pendingNodes(0),
  d_pendingScope(0),
  d_frameRate(0.0),
  d_firstEvent(0),
  d_lastEvent(0)
{
  d_nodes.addToScene(this, sceneUrl);
  d_backgrounds = new VrmlNodeList;
  d_backgroundStack = new VrmlNodeList;
  d_fogs = new VrmlNodeList;
  d_fogStack = new VrmlNodeList;
  d_navigationInfos = new VrmlNodeList;
  d_navigationInfoStack = new VrmlNodeList;
  d_viewpoints = new VrmlNodeList;
  d_viewpointStack = new VrmlNodeList;
  d_scopedLights = new VrmlNodeList;
  d_scripts = new VrmlNodeList;
  d_timers = new VrmlNodeList;
  d_movies = new VrmlNodeList;
  d_audioClips = new VrmlNodeList;

  if (sceneUrl)
    if (! load(sceneUrl, localCopy))
      theSystem->error("Couldn't load '%s'.\n", sceneUrl);
}

VrmlScene::~VrmlScene()
{
  d_nodes.addToScene( 0, 0 );
  d_nodes.removeChildren();

  bindableRemoveAll( d_backgroundStack ); 
  delete d_backgroundStack;
  bindableRemoveAll( d_fogStack ); 
  delete d_fogStack;
  bindableRemoveAll( d_navigationInfoStack ); 
  delete d_navigationInfoStack;
  bindableRemoveAll( d_viewpointStack );
  delete d_viewpointStack;

  delete d_backgrounds;
  delete d_fogs;
  delete d_navigationInfos;
  delete d_viewpoints;

  delete d_scopedLights;
  delete d_scripts;
  delete d_timers;
  delete d_movies;
  delete d_audioClips;

  delete d_url;
  delete d_urlLocal;

  delete d_pendingUrl;
  delete d_pendingParameters;
  delete d_pendingNodes;
  delete d_pendingScope;

  delete d_namespace;
}

// Load a (possibly non-VRML) file...

bool VrmlScene::loadUrl( VrmlMFString *url, VrmlMFString *parameters )
{
  if (! url) return false;

  int np = parameters ? parameters->size() : 0;
  char **params = parameters ? parameters->get() : 0;

  // try each url until we find one we can handle
  int i, n = url->size();
  char **urls = url->get();
  for (i=0; i<n; ++i)
    {
      if (! urls[i]) continue;

      // #Viewpoint
      if (*urls[i] == '#')
	{
	  if (load( urls[i] ))
	    break;
	}

      // Load .wrl's, or pass off to system
      else
	{      // Check mime type...
	  char *tail = strrchr(urls[i], SLASH);
	  if (! tail) tail = urls[i];
	  char *mod = strchr(tail, '#');
	  if (! mod) mod = urls[i] + strlen(urls[i]);
	  if (mod-tail > 4 &&
	      ( strncmp( mod-4, ".wrl", 4) == 0 ||
		strncmp( mod-4, ".wrz", 4) == 0 ||
		strncmp( mod-4, ".WRL", 4) == 0 ||
		strncmp( mod-4, ".WRZ", 4) == 0 ||
		(mod-tail > 7 &&
		 strncmp( mod-7, ".wrl.gz", 7) == 0 )))
	    {
	      if (load( urls[i] ))
		break;
	    }
	  else
	    {
	      if (theSystem->loadUrl( urls[i], np, params ))
		break;
	    }
	}
    }   

  return i != n;		// true if we found a url that loaded
}

// Called by viewer when a destroy request is received. The request
// is just passed on to the client via the worldChanged CB.

void VrmlScene::destroyWorld()
{
  doCallbacks( DESTROY_WORLD );
}


// Replace nodes

void VrmlScene::replaceWorld( VrmlMFNode &nodes, VrmlNamespace *ns,
			      Doc *url, Doc *urlLocal )
{
  theSystem->debug("replaceWorld( url %s )\n", url->url());

  delete d_namespace;
  delete d_url;
  delete d_urlLocal;

  d_namespace = ns;
  d_url = url;
  d_urlLocal = urlLocal;

  // Clear bindable stacks.
  bindableRemoveAll( d_backgroundStack ); 
  bindableRemoveAll( d_fogStack ); 
  bindableRemoveAll( d_navigationInfoStack ); 
  bindableRemoveAll( d_viewpointStack );

  // Get rid of current world: pending events, nodes.
  flushEvents();
  d_nodes.removeChildren();

  // Do this to set the relative URL
  d_nodes.addToScene( (VrmlScene *) this, urlDoc()->url() );

  // Add the nodes to a Group and put the group in the scene.
  // This will load EXTERNPROTOs and Inlines.
  d_nodes.addChildren( nodes );

  // Send initial set_binds to bindable nodes
  double timeNow = theSystem->time();
  VrmlSFBool flag(true);
  VrmlNode *bindable = 0;	// compiler warning

  if (d_backgrounds->size() > 0 &&
      (bindable = d_backgrounds->front()) != 0)
    bindable->eventIn( timeNow, "set_bind", &flag );

  if (d_fogs->size() > 0 &&
      (bindable = d_fogs->front()) != 0)
    bindable->eventIn( timeNow, "set_bind", &flag );

  if (d_navigationInfos->size() > 0 &&
      (bindable = d_navigationInfos->front()) != 0)
    bindable->eventIn( timeNow, "set_bind", &flag );

  if (d_viewpoints->size() > 0 &&
      (bindable = d_viewpoints->front()) != 0)
    bindable->eventIn( timeNow, "set_bind", &flag );

  // Notify anyone interested that the world has changed
  doCallbacks( REPLACE_WORLD );

  setModified();
}

void VrmlScene::doCallbacks( int reason )
{
  SceneCBList::iterator cb, cbend = d_sceneCallbacks.end();
  for (cb = d_sceneCallbacks.begin(); cb != cbend; ++cb)
    (*cb)( reason );

}

void VrmlScene::addWorldChangedCallback( SceneCB cb )
{
  d_sceneCallbacks.push_front( cb );
}

// Read a VRML97 file.
// This is only for [*.wrl][#viewpoint] url loading (no parameters).

bool VrmlScene::load(const char *url, const char *localCopy)
{
  // Look for '#Viewpoint' syntax. There ought to be a current
  // scene if this format is used.
  VrmlSFBool flag(true);
  if (*url == '#')
    {
      VrmlNode *vp = d_namespace ? d_namespace->findNode(url+1) : 0;

      // spec: ignore if named viewpoint not found
      if (vp)
	{
	  vp->eventIn( theSystem->time(), "set_bind", &flag );
	  setModified();
	}

      return true;
    }

  // Try to load a file. Prefer a local copy if available.
  Doc *tryUrl;
  if (localCopy)
    tryUrl = new Doc( localCopy, 0 );
  else
    tryUrl = new Doc( url, d_url);

  VrmlNamespace *newScope = new VrmlNamespace();
  VrmlMFNode *newNodes = readWrl( tryUrl, newScope );
  if ( newNodes )
    {
      Doc *sourceUrl = tryUrl, *urlLocal = 0;
      if (localCopy)
	{
	  sourceUrl = new Doc( url );
	  urlLocal = tryUrl;
	}
      
      replaceWorld( *newNodes, newScope, sourceUrl, urlLocal );
      delete newNodes;

      // Look for '#Viewpoint' syntax
      if ( sourceUrl->urlModifier() )
	{
	  VrmlNode *vp = d_namespace->findNode( sourceUrl->urlModifier()+1 );
	  double timeNow = theSystem->time();
	  if (vp)
	    vp->eventIn( timeNow, "set_bind", &flag );
	}

      return true;      // Success.
    }

  delete tryUrl;
  return false;
}


// Read a VRML file from one of the urls.

VrmlMFNode* VrmlScene::readWrl( VrmlMFString *urls, Doc *relative,
				VrmlNamespace *ns )
{
  Doc url;
  int i, n = urls->size();
  for (i=0; i<n; ++i)
    {
      //theSystem->debug("Trying to read url '%s'\n", urls->get(i));
      url.seturl( urls->get(i), relative );
      VrmlMFNode *kids = VrmlScene::readWrl( &url, ns );
      if ( kids )
	return kids;
      else if (i < n-1 && strncmp(urls->get(i),"urn:",4))
	theSystem->warn("Couldn't read url '%s': %s\n",
			urls->get(i), strerror( errno));
    }

  return 0;
}


// yacc globals
extern void yystring(char *);
extern void yyfunction( int (*)(char *, int) );
extern int yyparse();

extern FILE *yyin;
extern VrmlNamespace *yyNodeTypes;
extern VrmlMFNode *yyParsedNodes;
extern Doc *yyDocument;

#if HAVE_LIBPNG

# include "zlib.h"
# define YYIN yygz
extern gzFile yygz;

#else

# define YYIN yyin

#endif


// Read a VRML file and return the (valid) nodes.

VrmlMFNode* VrmlScene::readWrl( Doc *tryUrl, VrmlNamespace *ns )
{
  VrmlMFNode* result = 0;

  theSystem->debug("readWRL %s\n", tryUrl->url());

  // Should verify MIME type...
#if HAVE_LIBPNG
  if ((YYIN = tryUrl->gzopen("rb")) != 0)
#else
  if ((YYIN = tryUrl->fopen("rb")) != 0)
#endif
    {
      // If the caller is not interested in PROTO defs, use a local namespace
      VrmlNamespace nodeDefs;
      if (ns)
	yyNodeTypes = ns;
      else
	yyNodeTypes = &nodeDefs;

      yyDocument = tryUrl;
      yyParsedNodes = 0;

      yyparse();

      yyNodeTypes = 0;
      yyDocument = 0;

      result = yyParsedNodes;
      yyParsedNodes = 0;

#if HAVE_LIBPNG
      tryUrl->gzclose();
#else
      tryUrl->fclose();
#endif
    }

  return result;
}

//

bool VrmlScene::loadFromString( const char *vrmlString )
{
  VrmlNamespace *newScope = new VrmlNamespace();
  VrmlMFNode *newNodes = readString( vrmlString, newScope );
  if ( newNodes )
    {
      replaceWorld( *newNodes, newScope, 0, 0 );
      delete newNodes;
      return true;
    }
  return false;
}

// Read VRML from a string and return the (valid) nodes.

VrmlMFNode* VrmlScene::readString( const char *vrmlString,
				   VrmlNamespace *ns )
{
  VrmlMFNode* result = 0;

  if (vrmlString != 0)
    {
      yyNodeTypes = ns;
      yyDocument = 0;
      yyParsedNodes = 0;

      // set input to be from string
      yyin = 0;
      yystring( (char *)vrmlString );

      yyparse();

      yyNodeTypes = 0;
      result = yyParsedNodes;
      yyParsedNodes = 0;
    }

  return result;
}


// Load VRML from an application-provided callback function

bool VrmlScene::loadFromFunction( LoadCB cb, const char *url )
{
  Doc *doc = url ? new Doc( url, 0 ) : 0;
  VrmlNamespace *ns = new VrmlNamespace();
  VrmlMFNode *newNodes = readFunction( cb, doc, ns );

  if ( newNodes )
    {
      replaceWorld( *newNodes, ns, doc, 0 );
      delete newNodes;
      return true;
    }
  if (doc) delete doc;
  return false;
}

// Read VRML from a cb and return the (valid) nodes.

VrmlMFNode* VrmlScene::readFunction( LoadCB cb, Doc *url, VrmlNamespace *ns )
{
  VrmlMFNode* result = 0;

  if (cb != 0)
    {
      yyNodeTypes = ns;
      yyParsedNodes = 0;
      yyDocument = url;

      // set input to be from cb
      yyfunction( cb );

      yyparse();

      yyDocument = 0;
      yyNodeTypes = 0;
      result = yyParsedNodes;
      yyParsedNodes = 0;
    }

  return result;
}


// Read a PROTO from a URL to get the implementation of an EXTERNPROTO.
// This should read only PROTOs and return when the first/specified PROTO
// is read...

VrmlNodeType* VrmlScene::readPROTO( VrmlMFString *urls, Doc *relative )
{
  // This is a problem. The nodeType of the EXTERNPROTO has a namespace
  // that refers back to this namespace (protos), which will be invalid
  // after we exit this function. I guess it needs to be allocated and
  // ref counted too...
  VrmlNamespace protos;
  Doc urlDoc;
  VrmlNodeType* def = 0;
  int i, n = urls->size();

  theSystem->debug("readPROTO\n");

  for (i=0; i<n; ++i)
    {
      //theSystem->debug("Trying to read url '%s'\n", urls->get(i));
      urlDoc.seturl( urls->get(i), relative );
      VrmlMFNode *kids = VrmlScene::readWrl( &urlDoc, &protos );
      if ( kids ) delete kids;

      // Grab the specified PROTO, or the first one.
      const char *whichProto = urlDoc.urlModifier();
      if (*whichProto)
	def = (VrmlNodeType*) protos.findType( whichProto+1 );
      else
	def = (VrmlNodeType*) protos.firstType();

      if (def)
	{
	  def->setActualUrl( urlDoc.url() );
	  break;
	}
      else if (i < n-1 && strncmp(urls->get(i),"urn:",4))
        theSystem->warn("Couldn't read EXTERNPROTO url '%s': %s\n",
			urls->get(i), strerror( errno));
    }

  return def;
}

// Write the current scene to a file.
// Need to save the PROTOs/EXTERNPROTOs too...

bool VrmlScene::save(const char *url)
{
  bool success = false;
  Doc save(url);
  ostream &os = save.outputStream();

  if (os)
    {
      os << "#VRML V2.0 utf8\n";
      os << d_nodes;
      success = true;
    }

  return success;
}


//
// Script node API functions
//
const char *VrmlScene::getName() { return "LibVRML97"; }

const char *VrmlScene::getVersion() {
  static char vs[32];
  sprintf(vs, "%d.%d.%d", LIBVRML_MAJOR_VERSION, LIBVRML_MINOR_VERSION, LIBVRML_MICRO_VERSION);
  return vs;
}

double VrmlScene::getFrameRate() { return d_frameRate; }

// Queue an event to load URL/nodes (async so it can be called from a node)

void VrmlScene::queueLoadUrl( VrmlMFString *url, VrmlMFString *parameters )
{
  if (! d_pendingNodes && ! d_pendingUrl)
    {
      d_pendingUrl = url->clone()->toMFString();
      d_pendingParameters = parameters->clone()->toMFString();
    }
}

void VrmlScene::queueReplaceNodes( VrmlMFNode *nodes, VrmlNamespace *ns )
{
  if (! d_pendingNodes && ! d_pendingUrl)
    {
      d_pendingNodes = nodes->clone()->toMFNode();
      d_pendingScope = ns;
    }
}

// Event processing. Current events are in the array 
// d_eventMem[d_firstEvent,d_lastEvent). If d_firstEvent == d_lastEvent,
// the queue is empty. There is a fixed maximum number of events. If we
// are so far behind that the queue is filled, the oldest events get
// overwritten.

void VrmlScene::queueEvent(double timeStamp,
			   VrmlField *value,
			   VrmlNode *toNode,
			   const char *toEventIn)
{
  Event *e = &d_eventMem[d_lastEvent];
  e->timeStamp = timeStamp;
  e->value = value;
  e->toNode = toNode;
  e->toEventIn = toEventIn;
  d_lastEvent = (d_lastEvent+1) % MAXEVENTS;

  // If the event queue is full, discard the oldest (in terms of when it
  // was put on the queue, not necessarily in terms of earliest timestamp).
  if (d_lastEvent == d_firstEvent)
    {
      e = &d_eventMem[d_lastEvent];
      delete e->value;
      d_firstEvent = (d_firstEvent+1) % MAXEVENTS;
    }
}

// Any events waiting to be distributed?

bool VrmlScene::eventsPending() 
{ return d_firstEvent != d_lastEvent; }


// Discard all pending events

void VrmlScene::flushEvents()
{
  while (d_firstEvent != d_lastEvent)
    {
      Event *e = &d_eventMem[d_firstEvent];
      d_firstEvent = (d_firstEvent+1) % MAXEVENTS;
      delete e->value;
    }
}

// Called by the viewer when the cursor passes over, clicks, drags, or
// releases a sensitive object (an Anchor or another grouping node with 
// an enabled TouchSensor child).

void VrmlScene::sensitiveEvent( void *object,
				double timeStamp,
				bool isOver, bool isActive,
				double *point )
{
  VrmlNode *n = (VrmlNode *)object;

  if (n)
    {
      VrmlNodeAnchor *a = n->toAnchor();
      if ( a )
	{
	  // This should really be (isOver && !isActive && n->wasActive)
	  // (ie, button up over the anchor after button down over the anchor)
	  if (isActive && isOver)
	    {
	      a->activate();
	      //theSystem->inform("");
	    }
	  else if (isOver)
	    {
	      const char *description = a->description();
	      const char *url = a->url();
	      if (description && url)
		theSystem->inform("%s (%s)", description, url);
	      else if (description || url)
		theSystem->inform("%s", description ? description : url);
	      //else
	      //theSystem->inform("");
	    }
	  //else
	  //theSystem->inform("");
	}

      // The parent grouping node is registered for Touch/Drag Sensors
      else
	{
	  VrmlNodeGroup *g = n->toGroup();
	  if (g)
	    {
	      //theSystem->inform("");
	      g->activate( timeStamp, isOver, isActive, point );
	      setModified();
	    }
	}
      
    }

  //else
  //theSystem->inform("");
}

//
// The update method is where the events are processed. It should be
// called after each frame is rendered.
//
bool VrmlScene::update( double timeStamp )
{
  if (timeStamp <= 0.0) timeStamp = theSystem->time();
  VrmlSFTime now( timeStamp );

  d_deltaTime = DEFAULT_DELTA;

  // Update each of the timers.
  VrmlNodeList::iterator i, end = d_timers->end();
  for (i = d_timers->begin(); i != end; ++i)
    {
      VrmlNodeTimeSensor *t = (*i)->toTimeSensor();
      if (t) t->update( now );
    }

  // Update each of the clips.
  end = d_audioClips->end();
  for (i = d_audioClips->begin(); i != end; ++i)
    {
      VrmlNodeAudioClip *c = (*i)->toAudioClip();
      if (c) c->update( now );
    }

  // Update each of the scripts.
  end = d_scripts->end();
  for (i = d_scripts->begin(); i != end; ++i)
    {
      VrmlNodeScript *s = (*i)->toScript();
      if (s) s->update( now );
    }

  // Update each of the movies.
  end = d_movies->end();
  for (i = d_movies->begin(); i != end; ++i)
    {
      VrmlNodeMovieTexture *m =  (*i)->toMovieTexture();
      if (m) m->update( now );
    }


  // Pass along events to their destinations
  while (d_firstEvent != d_lastEvent &&
	 ! d_pendingUrl && ! d_pendingNodes)
    {
      Event *e = &d_eventMem[d_firstEvent];
      d_firstEvent = (d_firstEvent+1) % MAXEVENTS;

      // Ensure that the node is in the scene graph
      VrmlNode *n = e->toNode;
      if (this != n->scene())
	{
	  theSystem->debug("VrmlScene::update: %s::%s is not in the scene graph yet.\n",
			   n->nodeType()->getName(), n->name());
	  n->addToScene((VrmlScene*)this, urlDoc()->url() );
	}
      n->eventIn(e->timeStamp, e->toEventIn, e->value);
      // this needs to change if event values are shared...
      delete e->value;
    }

  if (d_pendingNodes)
    {
      replaceWorld( *d_pendingNodes, d_pendingScope );
      delete d_pendingNodes;
      d_pendingNodes = 0;
      d_pendingScope = 0;
    }
  else if (d_pendingUrl)
    {
      (void) loadUrl( d_pendingUrl, d_pendingParameters );
      delete d_pendingUrl;
      delete d_pendingParameters;
      d_pendingUrl = 0;
      d_pendingParameters = 0;
    }

  // Signal a redisplay if necessary
  return isModified();
}




bool VrmlScene::headlightOn()
{
  VrmlNodeNavigationInfo *navInfo = bindableNavigationInfoTop();
  if (navInfo)
    return navInfo->headlightOn();
  return true;
}


// Draw this scene into the specified viewer

void VrmlScene::render(Viewer *viewer)
{
  //
  if (d_newView)
    {
      viewer->resetUserNavigation();
      d_newView = false;
    }
      
  // Default viewpoint parameters
  float position[3] = { 0.0, 0.0, 10.0 };
  float orientation[4] = { 0.0, 0.0, 1.0, 0.0 };
  float field = 0.785398;
  float avatarSize = 0.25;
  float visibilityLimit = 0.0;

  VrmlNodeViewpoint *vp = bindableViewpointTop();
  if (vp)
    {
      position[0] = vp->positionX();
      position[1] = vp->positionY();
      position[2] = vp->positionZ();
      orientation[0] = vp->orientationX();
      orientation[1] = vp->orientationY();
      orientation[2] = vp->orientationZ();
      orientation[3] = vp->orientationR();
      field = vp->fieldOfView();

      vp->inverseTransform(viewer);
    }

  VrmlNodeNavigationInfo *ni = bindableNavigationInfoTop();
  if (ni)
    {
      avatarSize = ni->avatarSize()[0];
      visibilityLimit = ni->visibilityLimit();
    }

  viewer->setViewpoint( position, orientation, field,
			avatarSize, visibilityLimit);

  // Set background.
  VrmlNodeBackground *bg = bindableBackgroundTop();
  if (bg)
    { // Should be transformed by the accumulated rotations above ...
      bg->renderBindable(viewer);
    }
  else
    viewer->insertBackground();	// Default background

  // Fog
  VrmlNodeFog *f = bindableFogTop();
  if (f)
    {
      viewer->setFog(f->color(), f->visibilityRange(), f->fogType());
    }

  // Activate the headlight.
  // ambient is supposed to be 0 according to the spec...
  if ( headlightOn() )
  {
    float rgb[3] = { 1.0, 1.0, 1.0 };
    float xyz[3] = { 0.0, 0.0, -1.0 };
    float ambient = 0.3;

    viewer->insertDirLight( ambient, 1.0, rgb, xyz );
  }

  // Top level object
  viewer->beginObject(0);

  // Do the scene-level lights (Points and Spots)
  VrmlNodeList::iterator li, end = d_scopedLights->end();
  for (li = d_scopedLights->begin(); li != end; ++li)
    {
      VrmlNodeLight* x = (*li)->toLight();
      if (x) x->renderScoped( viewer );
    }

  // Render the top level group
  d_nodes.render( viewer );

  viewer->endObject();

  // This is actually one frame late...
  d_frameRate = viewer->getFrameRate();

  clearModified();

  // If any events were generated during render (ugly...) do an update
  if (eventsPending())
    setDelta( 0.0 );
    
}

//
//  Bindable children node stacks. For the CS purists out there, these 
//  aren't really stacks as they allow arbitrary elements to be removed
//  (not just the top).
//

VrmlNode *VrmlScene::bindableTop( BindStack stack )
{
  return (stack == 0 || stack->empty()) ? 0 : stack->front();
}

void VrmlScene::bindablePush( BindStack stack, VrmlNode *node )
{
  bindableRemove( stack, node ); // Remove any existing reference
  stack->push_front( node->reference() );
  setModified();
}

void VrmlScene::bindableRemove( BindStack stack, VrmlNode *node )
{
  if (stack)
    {
      VrmlNodeList::iterator i;

      for (i = stack->begin(); i != stack->end(); ++i )
	if ( *i == node )
	  {
	    (*i)->dereference();
	    stack->erase( i );
	    setModified();
	    break;
	  }
    }
}


// Remove all entries from the stack

void VrmlScene::bindableRemoveAll( BindStack stack )
{
  VrmlNodeList::iterator i;

  for (i = stack->begin(); i != stack->end(); ++i )
    (*i)->dereference();
  stack->erase(stack->begin(), stack->end());
}


// The nodes in the "set of all nodes of this type" lists are not 
// ref'd/deref'd because they are only added to and removed from 
// the lists in their constructors and destructors.


// Bindable children nodes (stacks)
// Define for each Type:
//    add/remove to complete list of nodes of this type
//    VrmlNodeType *bindableTypeTop();
//    void bindablePush(VrmlNodeType *);
//    void bindableRemove(VrmlNodeType *);

// Background

void VrmlScene::addBackground( VrmlNodeBackground *n )
{
  d_backgrounds->push_back( n );
}

void VrmlScene::removeBackground( VrmlNodeBackground *n )
{
  d_backgrounds->remove( n );
}

VrmlNodeBackground *VrmlScene::bindableBackgroundTop()
{
  VrmlNode *b = bindableTop( d_backgroundStack );
  return b ? b->toBackground() : 0;
}

void VrmlScene::bindablePush( VrmlNodeBackground *n )
{
  bindablePush( d_backgroundStack, n );
}

void VrmlScene::bindableRemove( VrmlNodeBackground *n )
{
  bindableRemove( d_backgroundStack, n );
}

// Fog

void VrmlScene::addFog( VrmlNodeFog *n )
{
  d_fogs->push_back( n );
}

void VrmlScene::removeFog( VrmlNodeFog *n )
{
  d_fogs->remove( n );
}

VrmlNodeFog *VrmlScene::bindableFogTop()
{
  VrmlNode *f =  bindableTop( d_fogStack );
  return f ? f->toFog() : 0;
}

void VrmlScene::bindablePush( VrmlNodeFog *n )  
{
  bindablePush( d_fogStack, n );
}

void VrmlScene::bindableRemove( VrmlNodeFog *n )  
{
  bindableRemove( d_fogStack, n );
}

// NavigationInfo
void VrmlScene::addNavigationInfo( VrmlNodeNavigationInfo *n )
{
  d_navigationInfos->push_back( n );
}

void VrmlScene::removeNavigationInfo( VrmlNodeNavigationInfo *n )
{
  d_navigationInfos->remove( n );
}

VrmlNodeNavigationInfo *VrmlScene::bindableNavigationInfoTop()
{
  VrmlNode *n = bindableTop( d_navigationInfoStack );
  return n ? n->toNavigationInfo() : 0;
}

void VrmlScene::bindablePush( VrmlNodeNavigationInfo *n )
{
  bindablePush( d_navigationInfoStack, n );
}

void VrmlScene::bindableRemove( VrmlNodeNavigationInfo *n )
{
  bindableRemove( d_navigationInfoStack, n );
}

// Viewpoint
void VrmlScene::addViewpoint( VrmlNodeViewpoint *n )
{
  d_viewpoints->push_back( n );
}

void VrmlScene::removeViewpoint( VrmlNodeViewpoint *n )
{
  d_viewpoints->remove( n );
}

VrmlNodeViewpoint *VrmlScene::bindableViewpointTop()
{
  VrmlNode *t = bindableTop( d_viewpointStack );
  return t ? t->toViewpoint() : 0;
}

void VrmlScene::bindablePush( VrmlNodeViewpoint *n )
{
  bindablePush( d_viewpointStack, n );
  d_newView = true;
}

void VrmlScene::bindableRemove( VrmlNodeViewpoint *n )
{
  bindableRemove( d_viewpointStack, n );
  d_newView = true;
}

// Bind to the next viewpoint in the list

void VrmlScene::nextViewpoint()
{
  VrmlNodeViewpoint *vp = bindableViewpointTop();
  VrmlNodeList::iterator i;

  for (i = d_viewpoints->begin(); i != d_viewpoints->end(); ++i )
    if ((*i) == vp)
      {
	if (++i == d_viewpoints->end())
	  i = d_viewpoints->begin();

	VrmlSFBool flag(true);
	if ((*i) && (vp = (*i)->toViewpoint()) != 0)
	  vp->eventIn( theSystem->time(), "set_bind", &flag );

	return;
      }
}
  

void VrmlScene::prevViewpoint()
{
  VrmlNodeViewpoint *vp = bindableViewpointTop();
  VrmlNodeList::iterator i;

  for (i = d_viewpoints->begin(); i != d_viewpoints->end(); ++i )
    if ((*i) == vp)
      {
	if (i == d_viewpoints->begin())
	  i = d_viewpoints->end();

	VrmlSFBool flag(true);
	if ( *(--i) && (vp = (*i)->toViewpoint()) != 0 )
	  vp->eventIn( theSystem->time(), "set_bind", &flag );

	return;
      }
}

int VrmlScene::nViewpoints() { return (int)d_viewpoints->size(); }

void VrmlScene::getViewpoint(int nvp, const char **namep, const char **descriptionp)
{
  VrmlNodeList::iterator i;
  int n;

  *namep = *descriptionp = 0;
  for (i = d_viewpoints->begin(), n=0; i != d_viewpoints->end(); ++i, ++n )
    if (n == nvp)
      {
	*namep = (*i)->name();
	*descriptionp = ((VrmlNodeViewpoint*)(*i))->description();
	return;
      }
}

void VrmlScene::setViewpoint(const char *name, const char *description)
{
  VrmlNodeList::iterator i;

  for (i = d_viewpoints->begin(); i != d_viewpoints->end(); ++i)
    if (strcmp(name, (*i)->name()) == 0 &&
	strcmp(description, ((VrmlNodeViewpoint*)(*i))->description()) == 0)
      {
	VrmlNodeViewpoint *vp;
	VrmlSFBool flag(true);
	if ((vp = (VrmlNodeViewpoint*) *i) != 0)
	  vp->eventIn( theSystem->time(), "set_bind", &flag );
	return;
      }
}

void VrmlScene::setViewpoint(int nvp)
{
  VrmlNodeList::iterator i;
  int j = 0;

  for (i = d_viewpoints->begin(); i != d_viewpoints->end(); ++i) {
    if (j == nvp)
      {
	VrmlNodeViewpoint *vp;
	VrmlSFBool flag(true);
	if ((vp = (VrmlNodeViewpoint*) *i) != 0)
	  vp->eventIn( theSystem->time(), "set_bind", &flag );
	return;
      }
    ++j;
  }
}

// The nodes in these lists are not ref'd/deref'd because they
// are only added to and removed from the lists in their constructors
// and destructors.


// Scene-level distance-scoped lights

void VrmlScene::addScopedLight( VrmlNodeLight *light )
{
  d_scopedLights->push_back( light );
}

void VrmlScene::removeScopedLight( VrmlNodeLight *light )
{
  d_scopedLights->remove( light );
}


// Movies

void VrmlScene::addMovie( VrmlNodeMovieTexture *movie )
{
  d_movies->push_back( movie );
}

void VrmlScene::removeMovie( VrmlNodeMovieTexture *movie )
{
  d_movies->remove( movie );
}

// Scripts

void VrmlScene::addScript( VrmlNodeScript *script )
{
  d_scripts->push_back( script );
}

void VrmlScene::removeScript( VrmlNodeScript *script )
{
  d_scripts->remove( script );
}

// TimeSensors

void VrmlScene::addTimeSensor( VrmlNodeTimeSensor *timer )
{
  d_timers->push_back( timer );
}

void VrmlScene::removeTimeSensor( VrmlNodeTimeSensor *timer )
{
  d_timers->remove( timer );
}


// AudioClips

void VrmlScene::addAudioClip( VrmlNodeAudioClip *audio_clip )
{
  d_audioClips->push_back( audio_clip );
}

void VrmlScene::removeAudioClip( VrmlNodeAudioClip *audio_clip )
{
  d_audioClips->remove( audio_clip );
}
