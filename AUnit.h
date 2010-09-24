//  AUnit.h  a base class for audio-processing units.

#ifndef __AUNIT_H
#define __AUNIT_H

#include "AU_defs.h"

class AUnit {
protected:
  int	channels;
  int	inputs;
  au_sample_t	**buffer;
  au_timestamp_t	last_rendered[AU_MAX_OUT_CHANNELS];
  AUnit	**input;

public:
  AUnit(int nchan, int nin=0);
  virtual ~AUnit();
  au_index_t render(int channel, au_sample_t *buf, au_timestamp_t time);
  inline au_index_t inputRender(int inp, int channel, au_sample_t *buf, au_timestamp_t time);

  virtual au_index_t userRender(int channel, au_timestamp_t time) =0;
  void connectInput(int index, AUnit* au) { input[index]=au; }

  virtual int numKnobs() { return 0; }
  virtual int numSwitches() { return 0; }
  virtual double getKnob(int index) { return 0.0; }
  virtual int getSwitch(int index) { return 0; }
  virtual void setKnob(int index, double val) { }
  virtual void setSwitch(int index, int val) { }
  virtual const AUKnobDesc* knobDesc(int index) { return (AUKnobDesc*) 0; }
  virtual const AUSwitchDesc* switchDesc(int index) { return (AUSwitchDesc*) 0; }
};

#endif
