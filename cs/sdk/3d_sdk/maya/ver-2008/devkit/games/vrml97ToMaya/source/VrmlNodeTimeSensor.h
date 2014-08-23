//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeTimeSensor.h

#ifndef  _VRMLNODETIMESENSOR_
#define  _VRMLNODETIMESENSOR_

#include "VrmlNodeChild.h"
#include "VrmlSFBool.h"
#include "VrmlSFFloat.h"
#include "VrmlSFTime.h"

class VrmlScene;


class VrmlNodeTimeSensor : public VrmlNodeChild {

public:

  // Define the fields of TimeSensor nodes
  static VrmlNodeType *defineType(VrmlNodeType *t = 0);
  virtual VrmlNodeType *nodeType() const;

  VrmlNodeTimeSensor( VrmlScene *scene = 0);
  virtual ~VrmlNodeTimeSensor();

  virtual VrmlNode *cloneMe() const;

  virtual VrmlNodeTimeSensor* toTimeSensor() const;

  virtual void addToScene( VrmlScene *s, const char* );

  virtual ostream& printFields(ostream& os, int indent);

  void update( VrmlSFTime &now );

  virtual void eventIn(double timeStamp,
		       const char *eventName,
		       const VrmlField *fieldValue);

  virtual void setField(const char *fieldName, const VrmlField &fieldValue);

  virtual double getCycleInterval(){ return d_cycleInterval.get();} 
  virtual bool getEnabled(){ return d_enabled.get();} 
  virtual bool getLoop(){ return d_loop.get();} 
  virtual double getStartTime(){ return d_startTime.get();} 
  virtual double getStopTime(){ return d_stopTime.get();} 


private:

  // Fields
  VrmlSFTime d_cycleInterval;
  VrmlSFBool d_enabled;
  VrmlSFBool d_loop;
  VrmlSFTime d_startTime;
  VrmlSFTime d_stopTime;

  // Internal state
  VrmlSFBool d_isActive;
  double d_lastTime;
};

#endif // _VRMLNODETIMESENSOR_

