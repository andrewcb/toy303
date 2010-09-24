//  AUnit.cc  a base class for audio processing units


#include "AUnit.h"
#include <string.h>

AUnit::AUnit(int nchan, int nin)
 : channels(nchan), inputs(nin)
{
  buffer = new au_sample_t*[nchan];
  for(register int i=0; i<nchan; i++) {
    buffer[i] = new au_sample_t[AU_BUFSIZE];
    last_rendered[i] = -1;
  }
  input = new AUnit*[nin];
  memset(input, 0, sizeof(AUnit*)*nin);
}

AUnit::~AUnit()
{
  for(register int i=0; i<channels; i++) {
    delete buffer[i];
  }
  delete buffer;
}

au_index_t
AUnit::render(int channel, au_sample_t *buf, au_timestamp_t time)
{
  if(channel>=channels) {
    memset(buf, 0, AU_BUFBYTES);
    return AU_BUFSIZE;
  }
  if(last_rendered[channel] != time) {
    userRender(channel, time);
    last_rendered[channel] = time;
  }
  if(buf) memcpy(buf, buffer[channel], sizeof(au_sample_t)*AU_BUFSIZE);
  return AU_BUFSIZE;
}

inline au_index_t
AUnit::inputRender(int inp, int channel, au_sample_t *buf, au_timestamp_t time)
{
  if(input[inp]) 
    return input[inp]->render(channel, buf, time);
  else {
    memset((void*)buf, 0, sizeof(au_sample_t)*AU_BUFSIZE);
    return AU_BUFSIZE;
  }
   
}
