//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeMovieTexture.cpp

#include "VrmlNodeMovieTexture.h"

#include "VRMLImage.h"
#include "MathUtils.h"
#include "System.h"
#include "VrmlNodeType.h"
#include "VrmlScene.h"
#include "Viewer.h"
#include "Doc.h"

#if defined AW_NEW_IOSTREAMS
#  include <iostream>
#else
#  include <iostream.h>
#endif

static VrmlNode *creator( VrmlScene *s ) 
{ return new VrmlNodeMovieTexture(s); }


// Define the built in VrmlNodeType:: "MovieTexture" fields

VrmlNodeType *VrmlNodeMovieTexture::defineType(VrmlNodeType *t)
{
  static VrmlNodeType *st = 0;

  if (! t)
    {
      if (st) return st;
      t = st = new VrmlNodeType("MovieTexture", creator);
    }

  VrmlNodeTexture::defineType(t);	// Parent class

  t->addExposedField("loop", VrmlField::SFBOOL);
  t->addExposedField("speed", VrmlField::SFFLOAT);
  t->addExposedField("startTime", VrmlField::SFTIME);
  t->addExposedField("stopTime", VrmlField::SFTIME);
  t->addExposedField("url", VrmlField::MFSTRING);
  t->addField("repeatS", VrmlField::SFBOOL);
  t->addField("repeatT", VrmlField::SFBOOL);
  t->addEventOut("duration_changed", VrmlField::SFTIME);
  t->addEventOut("isActive", VrmlField::SFBOOL);

  return t;
}


VrmlNodeType *VrmlNodeMovieTexture::nodeType() const { return defineType(0); }


VrmlNodeMovieTexture::VrmlNodeMovieTexture(VrmlScene *scene) :
  VrmlNodeTexture(scene),
  d_loop(false),
  d_speed(1.0),
  d_repeatS(true),
  d_repeatT(true),
  d_image(0),
  d_frame(0),
  d_lastFrame(-1),
  d_lastFrameTime(-1.0),
  d_texObject(0)
{
  if (d_scene) d_scene->addMovie( this );
}


VrmlNodeMovieTexture::~VrmlNodeMovieTexture()
{
  //if (d_texObject) d_viewer->removeTextureObject(d_texObject);
  if (d_scene) d_scene->removeMovie( this );
  delete d_image;
}


VrmlNode *VrmlNodeMovieTexture::cloneMe() const
{
  return new VrmlNodeMovieTexture(*this);
}


VrmlNodeMovieTexture* VrmlNodeMovieTexture::toMovieTexture() const
{ return (VrmlNodeMovieTexture*) this; }


void VrmlNodeMovieTexture::addToScene(VrmlScene *s, const char *rel)
{
  if (d_scene != s && (d_scene = s) != 0) d_scene->addMovie(this);
  VrmlNodeTexture::addToScene(s, rel);
}


ostream& VrmlNodeMovieTexture::printFields(ostream& os, int indent)
{
  if (d_loop.get()) PRINT_FIELD(loop);
  if (!FPEQUAL(d_speed.get(), 1.0)) PRINT_FIELD(speed);
  if (!FPZERO(d_startTime.get())) PRINT_FIELD(startTime);
  if (!FPZERO(d_stopTime.get())) PRINT_FIELD(stopTime);
  if (d_url.get()) PRINT_FIELD(url);
  if (! d_repeatS.get()) PRINT_FIELD(repeatS);
  if (! d_repeatT.get()) PRINT_FIELD(repeatT);
  return os;
}


void VrmlNodeMovieTexture::update( VrmlSFTime &timeNow )
{
  if ( isModified() )
    {
      if (d_image)
	{
	  const char *imageUrl = d_image->url();
	  int imageLen = (int)strlen(imageUrl);
	  int i, nUrls = d_url.size();
	  for (i=0; i<nUrls; ++i)
	    {
	      int len = (int)strlen(d_url[i]);
	      
	      if ((strcmp(imageUrl, d_url[i]) == 0) ||
		  (imageLen > len &&
		   strcmp(imageUrl+imageLen-len, d_url[i]) == 0))
		break;
	    }

	  // if (d_image->url() not in d_url list) ...
	  if (i == nUrls)
	    {
	      delete d_image;
	      d_image = 0;
	    }
	}
    }

  // Load the movie if needed (should check startTime...)
  if (! d_image && d_url.size() > 0)
    {
      Doc relDoc( d_relativeUrl.get() );
      Doc *rel = d_relativeUrl.get() ? &relDoc : d_scene->urlDoc();
      d_image = new Image;
      if ( ! d_image->tryURLs( d_url.size(), d_url.get(), rel ) )
	cerr << "Error: couldn't read MovieTexture from URL " << d_url << endl;


      int nFrames = d_image->nFrames();
      d_duration = (nFrames >= 0) ? nFrames : -1;
      eventOut( timeNow.get(), "duration_changed", d_duration );
      d_frame = (d_speed.get() >= 0) ? 0 : nFrames-1;

      //theSystem->debug("MovieTexture.%s loaded %d frames\n", name(), nFrames);
    }

  // No pictures to show
  if (! d_image || d_image->nFrames() == 0) return;

  // Become active at the first tick at or after startTime if either
  // the valid stopTime hasn't passed or we are looping.
  if (! d_isActive.get() &&
      d_startTime.get() <= timeNow.get() &&
      d_startTime.get() >= d_lastFrameTime &&
      ( (d_stopTime.get() < d_startTime.get() || // valid stopTime
	 d_stopTime.get() > timeNow.get()) ||    // hasn't passed
	d_loop.get() ))
    {
      //theSystem->debug("MovieTexture.%s::isActive TRUE\n", name());
      d_isActive.set(true);
      eventOut( timeNow.get(), "isActive", d_isActive );
      d_lastFrameTime = timeNow.get();
      d_frame = (d_speed.get() >= 0) ? 0 : d_image->nFrames() - 1;
      setModified();
    }

  // Check whether stopTime has passed
  else if ( d_isActive.get() &&
	    (( d_stopTime.get() > d_startTime.get() &&
	       d_stopTime.get() <= timeNow.get() ) ||
	     d_frame < 0))
    {
      //theSystem->debug("MovieTexture.%s::isActive FALSE\n", name());
      d_isActive.set(false);
      eventOut( timeNow.get(), "isActive", d_isActive );
      setModified();
    }

  // Check whether the frame should be advanced
  else if ( d_isActive.get() &&
	    d_lastFrameTime + fabs(double(d_speed.get())) <= timeNow.get() )
    {
      if (d_speed.get() < 0.0)
	--d_frame;
      else
	++d_frame;
      //theSystem->debug("MovieTexture.%s::frame %d\n", name(), d_frame);
      d_lastFrameTime = timeNow.get();
      setModified();
    }

  // Tell the scene when the next update is needed.
  if (d_isActive.get())
    {
      double d = d_lastFrameTime + fabs(double(d_speed.get())) - timeNow.get();
      d_scene->setDelta( 0.9 * d );
    }

}

// Ignore set_speed when active.

void VrmlNodeMovieTexture::eventIn(double timeStamp,
				   const char *eventName,
				   const VrmlField *fieldValue)
{
  const char *origEventName = eventName;
  if ( strncmp(eventName, "set_", 4) == 0 )
    eventName += 4;

  // Ignore set_speed when active
  if ( strcmp(eventName,"speed") == 0 )
    {
      if (! d_isActive.get())
	{
	  setField(eventName, *fieldValue);
	  eventOut(timeStamp, "speed_changed", *fieldValue);
	  setModified();
	}
    }

  // Let the generic code handle the rest.
  else
    VrmlNode::eventIn( timeStamp, origEventName, fieldValue );
}


// Render a frame if there is one available.

void VrmlNodeMovieTexture::render(Viewer *viewer)
{
  //theSystem->debug("MovieTexture.%s::render frame %d\n", name(), d_frame);

  if ( ! d_image || d_frame < 0 ) return;

  unsigned char *pix = d_image->pixels( d_frame );

  if ( d_frame != d_lastFrame && d_texObject )
    {
      viewer->removeTextureObject( d_texObject );
      d_texObject = 0;
    }

  if ( ! pix )
    {
      d_frame = -1;
    }
  else if ( d_texObject )
    {
      viewer->insertTextureReference( d_texObject, d_image->nc() );
    }
  else
    {
      // Ensure image dimensions are powers of 2 (move to VrmlNodeTexture...)
      int sizes[] = { 2, 4, 8, 16, 32, 64, 128, 256 };
      int nSizes = sizeof(sizes) / sizeof(int);
      int w = d_image->w();
      int h = d_image->h();
      int i, j;
      for (i=0; i<nSizes; ++i)
	if (w < sizes[i]) break;
      for (j=0; j<nSizes; ++j)
	if (h < sizes[j]) break;

      if (i > 0 && j > 0)
	{
	  // Always scale images down in size and reuse the same pixel memory.
	  if (w != sizes[i-1] || h != sizes[j-1])
	    {
	      viewer->scaleTexture( w, h, sizes[i-1], sizes[j-1],
				    d_image->nc(), pix );
	      d_image->setSize( sizes[i-1], sizes[j-1] );
	    }

	  d_texObject = viewer->insertTexture(d_image->w(),
					      d_image->h(),
					      d_image->nc(),
					      d_repeatS.get(),
					      d_repeatT.get(),
					      pix,
					      ! d_isActive.get() );
	}
    }

  d_lastFrame = d_frame;
  clearModified();
}


int VrmlNodeMovieTexture::nComponents()
{
  return d_image ? d_image->nc() : 0;
}

int VrmlNodeMovieTexture::width()
{
  return d_image ? d_image->w() : 0;
}

int VrmlNodeMovieTexture::height()
{
  return d_image ? d_image->h() : 0;
}

int VrmlNodeMovieTexture::nFrames()
{
  return d_image ? d_image->nFrames() : 0;
}

unsigned char* VrmlNodeMovieTexture::pixels()
{
  return d_image ? d_image->pixels() : 0;
}



// Set the value of one of the node fields.

void VrmlNodeMovieTexture::setField(const char *fieldName,
				    const VrmlField &fieldValue)
{
  if TRY_FIELD(loop, SFBool)
  else if TRY_FIELD(speed, SFFloat)
  else if TRY_FIELD(startTime, SFTime)
  else if TRY_FIELD(stopTime, SFTime)
  else if TRY_FIELD(url, MFString)
  else if TRY_FIELD(repeatS, SFBool)
  else if TRY_FIELD(repeatT, SFBool)
  else
    VrmlNode::setField(fieldName, fieldValue);
}

