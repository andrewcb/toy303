//  A node connected to the audio output device on the current machine
//  (the Linux /dev/dsp version)

#include "AOutput.h"
#include <sys/soundcard.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream.h>

AOutput::AOutput(unsigned int srate, int stereo)
 : AUnit(stereo?2:1, 1), samplerate(srate)
{
  int io;
  audiofd = open("/dev/dsp", O_WRONLY);
  io = channels;
  outbuf = new au_sample_t[AU_BUFSIZE*channels];
  inbuf = new au_sample_t*[channels];
  for(int i=0; i<channels; i++) 
    inbuf[i] = new au_sample_t[AU_BUFSIZE];
  if(ioctl(audiofd, SNDCTL_DSP_CHANNELS, &io) == -1) {
    perror("Unable to set /dev/dsp channels");
    exit(1);
  }
  if(ioctl(audiofd, SNDCTL_DSP_SPEED, &srate)==-1) {
    perror("Unable to set /dev/dsp sample rate");
    exit(1);
  }
  io = AFMT_S16_LE;
  if(ioctl(audiofd, SNDCTL_DSP_SETFMT, &io)==-1) {
    perror("Unable to set /dev/dsp sample format");
    exit(1);
  }
  // set the audio device's buffer size to our own buffer size
  io = 0x00020000 | AU_LOG2_BUFSIZE;
  ioctl(audiofd, SNDCTL_DSP_SETFRAGMENT, &io);
  ioctl(audiofd, SNDCTL_DSP_GETBLKSIZE, &io);
  if(io!=AU_BUFSIZE) {
    cerr<<"warning: couldn't set /dev/dsp block size to "<<AU_BUFSIZE<<"; is set to "<<io<<"\n";
  }
  // set synchronous mode
  io = 1;
  ioctl(audiofd, SNDCTL_DSP_SYNC, &io);
}

AOutput::~AOutput()
{
  close(audiofd);
  delete outbuf;
  for(int i=0; i<channels; i++) delete inbuf[i];
  delete inbuf;
}

au_index_t 
AOutput::userRender(int channel, au_timestamp_t time)
{
  // as this has no meaningful output, we don't bother filling buffers, but
  // just write data to the output device.

  // we only work when called with channel==0, and then we process all
  // channels.  it's simpler that way...
//  cerr<<"AOutput::userRender("<<channel<<", "<<time<<")\n";


  if(channel!=0) return AU_BUFSIZE;

  if(channels == 1) {	// mono
    inputRender(0, 0, outbuf, time);
  } else { 		// stereo 
    register int i,j;
    for(i=0; i<channels; i++) 
      inputRender(0, i, inbuf[i], time);
    for(i=0,j=0; i<AU_BUFSIZE; i++,j+=2) {
      outbuf[j]=inbuf[0][i];
      outbuf[j+1]=inbuf[1][i];
    }
  }
  write(audiofd, outbuf, AU_BUFBYTES*channels);
  return AU_BUFSIZE;
}
