//  A node connected to the audio output device on the current machine

#ifndef __AOUTPUT_H
#define __AOUTPUT_H

#include "AUnit.h"

// this is the Linux /dev/dsp version

class AOutput: public AUnit {
protected:
  AUnit *in;
  unsigned int samplerate;
  int audiofd;
  au_sample_t** inbuf;
  au_sample_t* outbuf;

public:
  AOutput(unsigned int srate, int stereo);
  virtual ~AOutput();
  virtual au_index_t userRender(int channel, au_timestamp_t time);
};

#endif
