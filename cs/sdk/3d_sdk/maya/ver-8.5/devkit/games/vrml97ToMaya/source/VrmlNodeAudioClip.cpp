//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeAudioClip.cpp
//    contributed by Kumaran Santhanam

#include "VrmlNodeAudioClip.h"
#include "VrmlNodeType.h"
#include "VrmlScene.h"
#include "MathUtils.h"
#include "Audio.h"
#include "Doc.h"
#include "sound.h"

#if defined AW_NEW_IOSTREAMS
#  include <iostream>
#else
#  include <iostream.h>
#endif

static VrmlNode *creator( VrmlScene *s ) { return new VrmlNodeAudioClip(s); }


// Define the built in VrmlNodeType:: "AudioClip" fields

VrmlNodeType *VrmlNodeAudioClip::defineType(VrmlNodeType *t)
{
  static VrmlNodeType *st = 0;

  if (! t)
    {
      if (st) return st;
      t = st = new VrmlNodeType("AudioClip", creator);
    }

  VrmlNode::defineType(t);	// Parent class
  t->addExposedField("description", VrmlField::SFSTRING);
  t->addExposedField("loop", VrmlField::SFBOOL);
  t->addExposedField("pitch", VrmlField::SFFLOAT);
  t->addExposedField("startTime", VrmlField::SFTIME);
  t->addExposedField("stopTime", VrmlField::SFTIME);
  t->addExposedField("url", VrmlField::MFSTRING);

  t->addEventOut("duration_changed", VrmlField::SFTIME);
  t->addEventOut("isActive", VrmlField::SFBOOL);

  return t;
}


VrmlNodeType *VrmlNodeAudioClip::nodeType() const { return defineType(0); }


VrmlNodeAudioClip::VrmlNodeAudioClip(VrmlScene *scene) :
  VrmlNode(scene),
  d_pitch(1.0),
  d_isActive(false),
  d_audio(0),
  d_url_modified(false),
  _audio_index(0),
  _audio_intensity(1.0),
  _audio_fd(-1)
{
    if (d_scene) d_scene->addAudioClip(this);
}


// Define copy constructor so clones don't share d_audio (for now, anyway...)

VrmlNodeAudioClip::VrmlNodeAudioClip(const VrmlNodeAudioClip &n) :
  VrmlNode(n),
  d_description(n.d_description),
  d_loop(n.d_loop),
  d_pitch(n.d_pitch),
  d_startTime(n.d_startTime),
  d_stopTime(n.d_stopTime),
  d_url(n.d_url),
  d_duration(n.d_duration),
  d_isActive(false),
  d_audio(0),			// can these be shared?...
  d_url_modified(false),
  _audio_index(0),
  _audio_intensity(1.0),
  _audio_fd(-1)
{
    if (d_scene) d_scene->addAudioClip(this);
}


VrmlNodeAudioClip::~VrmlNodeAudioClip()
{
  delete d_audio;
  if (d_scene) d_scene->removeAudioClip(this);
}

VrmlNode *VrmlNodeAudioClip::cloneMe() const
{
  return new VrmlNodeAudioClip( *this);
}

void VrmlNodeAudioClip::addToScene(VrmlScene *s, const char *rel)
{
  if (d_scene != s && (d_scene = s) != 0) d_scene->addAudioClip(this);
  d_relativeUrl.set(rel);
}

VrmlNodeAudioClip* VrmlNodeAudioClip::toAudioClip() const
{ return (VrmlNodeAudioClip*)this; }


ostream& VrmlNodeAudioClip::printFields(ostream& os, int indent)
{
  if (d_description.get()) PRINT_FIELD(description);
  if (d_loop.get()) PRINT_FIELD(loop);
  if (! FPEQUAL(d_pitch.get(),1.0)) PRINT_FIELD(pitch);
  if (! FPZERO(d_startTime.get())) PRINT_FIELD(startTime);
  if (! FPZERO(d_stopTime.get())) PRINT_FIELD(stopTime);
  if (d_url.size() > 0) PRINT_FIELD(url);
  return os;
}


void VrmlNodeAudioClip::update (VrmlSFTime &inTime)
{
    // If the URL has been modified, update the audio object
    if (d_url_modified)
    {
        Doc relDoc( d_relativeUrl.get() );
        delete d_audio;
        d_audio = new Audio(0);
        if (d_audio->tryURLs (d_url.size(),
                              d_url.get(),
			      &relDoc))
        {
            d_duration.set (d_audio->duration());
            eventOut (inTime.get(), "duration_changed", d_duration);
        }
        else
        {
            cerr << "Error: couldn't read AudioClip from URL "
                 << d_url << endl;
            delete d_audio;
            d_audio = 0;
        }

        d_url_modified = false;
    }

    // If there's no audio or START <= 0, we don't play anything
    if (d_audio == 0 || d_startTime.get() <= 0)
        return;

    // Determine if this clip should be playing right now
    bool audible = false;

    // If START < STOP  and  START <= NOW < STOP
    if (d_stopTime.get() > d_startTime.get())
        audible = (d_startTime.get() <= inTime.get() &&
                   inTime.get() < d_stopTime.get());

    // If STOP < START  and  START <= NOW
    else
        audible = (inTime.get() >= d_startTime.get());

    // If the clip is not looping, it's not audible after
    // its duration has expired.
    if (d_loop.get() == false)
        if (inTime.get()-d_startTime.get() > d_audio->duration())
            audible = false;

    // If the clip is audible, play it.  Otherwise, stop it.
    if (audible)
    {
        // If the sound device is not already open, open it.
        if (_audio_fd < 0)
        {
            _audio_fd = openSound (d_audio->channels(),
                                   d_audio->bitsPerSample(),
                                   d_audio->samplesPerSec());

            // If we can't get a sound device, silently return
            if (_audio_fd < 0)
                    return;

            _audio_index = d_audio->getByteIndex
                               (inTime.get() - d_startTime.get());

            d_isActive.set (true);
            eventOut (inTime.get(), "isActive", d_isActive);
        }

        // Send out a sound buffer
        _audio_index = outputSoundChunk (d_audio->numBytes(),
                                         d_audio->samples(),
                                         d_audio->bitsPerSample(),
                                         _audio_index, d_loop.get(),
                                         _audio_intensity,
                                         _audio_fd);
    }

    // Otherwise, close the sound device
    else
    {
        if (_audio_fd >= 0)
        {
            _audio_fd = closeSound (_audio_fd);
            d_isActive.set (false);
            eventOut (inTime.get(), "isActive", d_isActive);
        }
    }
}


// Set the value of one of the node fields.

void VrmlNodeAudioClip::setField(const char *fieldName,
				 const VrmlField &fieldValue)
{
  if TRY_FIELD(description, SFString)
  else if TRY_FIELD(loop, SFBool)
  else if TRY_FIELD(pitch, SFFloat)
  else if TRY_FIELD(startTime, SFTime)
  else if TRY_FIELD(stopTime, SFTime)
  else if TRY_FIELD(url, MFString)
  else
    VrmlNode::setField(fieldName, fieldValue);

  if (strcmp ("url", fieldName) == 0)
  {
      d_url_modified = true;
      setModified();
  }
}
