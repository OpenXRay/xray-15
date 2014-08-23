//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeAudioClip.h
//    contributed by Kumaran Santhanam

#ifndef  _VRMLNODEAUDIOCLIP_
#define  _VRMLNODEAUDIOCLIP_

#include "VrmlNode.h"
#include "VrmlMFString.h"
#include "VrmlSFBool.h"
#include "VrmlSFFloat.h"
#include "VrmlSFString.h"
#include "VrmlSFTime.h"

class Audio;

class VrmlNodeAudioClip : public VrmlNode {

public:

  // Define the fields of AudioClip nodes
  static VrmlNodeType *defineType(VrmlNodeType *t = 0);
  virtual VrmlNodeType *nodeType() const;

  VrmlNodeAudioClip(VrmlScene *);
  VrmlNodeAudioClip(const VrmlNodeAudioClip&);
  virtual ~VrmlNodeAudioClip();

  // Copy the node.
  virtual VrmlNode *cloneMe() const;

  virtual void addToScene(VrmlScene *s, const char *relativeUrl);

  void update (VrmlSFTime &now);

  virtual VrmlNodeAudioClip* toAudioClip() const;

  virtual ostream& printFields(ostream& os, int indent);

  virtual void setField(const char *fieldName, const VrmlField &fieldValue);

private:

  VrmlSFString d_description;
  VrmlSFBool d_loop;
  VrmlSFFloat d_pitch;
  VrmlSFTime d_startTime;
  VrmlSFTime d_stopTime;
  VrmlMFString d_url;

  VrmlSFString d_relativeUrl;

  VrmlSFTime d_duration;
  VrmlSFBool d_isActive;

  Audio *d_audio;
  bool   d_url_modified;
  int    _audio_index;
  double _audio_intensity;
  int    _audio_fd;
};

#endif // _VRMLNODEAUDIOCLIP_

