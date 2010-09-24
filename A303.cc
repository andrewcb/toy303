//  A303.cc  a TB-303-like bassline synth with step sequencer.

#include "A303.h"

#include <math.h>
#include <stdlib.h>
#include <iostream.h>

static AUKnobDesc knob_desc[] = {
  { "cutoff", 0.0, 1.0 },
  { "reso", 0.0, 1.0 },
  { "decay", 0.0, 1.0 },
};

static AUSwitchDesc switch_desc[] = {
  { "waveform", 0, 1 }
};

static inline double 
note2freq(int note) 
{
  return pow(1.05946309436, note)*32.7032;
}

A303::A303()
 : AUnit(1), steps(A303_MAXSTEPS), reso(0.05), emode(A303_ENV_RELEASE), 
   step(10000), vcophase(0), vcaamp(0), trigtime(0), wave(A303_WAVE_SAWTOOTH),
   vcfdecay(0.9999999)
{
  clear();
  // data for the Lars Hamre filter
  vcf_h=vcf_m=vcf_l=vcf_h2=vcf_m2=vcf_l2=0.0;

  // initialise the synth to the first step in the sequencer
//  step = 100000; //nextStep();
  vcofreq = note2freq(12);
}

au_index_t
A303::userRender(int channel, au_timestamp_t time)
{
#if 0
  if(time%100 == 0) {
    cerr<<vcf_h<<", "<<vcf_m<<", "<<vcf_l<<"\n";
  }
#endif
  for(int sam=0; sam<AU_BUFSIZE; sam++) {
    switch(wave) {
    case A303_WAVE_SQUARE: vco_t = vcophase<0.5?1.0:-1.0; break;
    case A303_WAVE_SAWTOOTH: vco_t = (vcophase*2.0)-1.0; break;
    }
    if(vco>0) {
      vco = vco+(vco_t-vco)*0.95;
    } else {
      vco = vco+(vco_t-vco)*0.9;
    }

    if(slide[step]) {
      vcofreq = vcofreq + (vcofreq_t - vcofreq)*0.0005;
    } else {
      vcofreq = vcofreq_t;
    }
    float cycles_per_sample = vcofreq / AU_SAMPLE_RATE;
    vcophase += cycles_per_sample;
    if(vcophase>=1.0) vcophase-=1;

    // ----- VCF -----
#if 1
    //double vcfco = 0.08 + 0.8*vcfenv;
    double vcfco = 0.00+cutoff*0.9 + 0.2*vcfenv;
    //double vcfco = 1.0;
    double out;

    float feedbk = reso*vcf_m;
#if 0
    if(isinf(feedbk)) {
      cerr<<"\nfeedbk == "<<feedbk<<" = "<<reso<<"*"<<vcf_m<<"\n";
      exit(1);
    }
#endif
#endif

    vcfenv *= vcfdecay;

#if 0
// for some reason this causes infinite feedback, resulting in NaNs in the
// filter.
    // useless (?) hack to simulate diode limiting of resonance
    if (feedbk > 0.01) {
      double sq = (feedbk-0.01)*2;
      feedbk += sq*sq;
    } else if (feedbk < -0.01) {
      double sq = (feedbk+0.01)*2;
      feedbk -= sq*sq;
    }
#endif

    // 2-pole filter #1
    vcf_h = vco - feedbk - vcf_l;
#if 0
    if(isnan(vcf_h)) {
      cerr<<"\n"<< vco<<" - "<<feedbk<<" - "<<vcf_l<<" = nan\n"; exit(1);
    }
    if(isinf(vcf_h)) {
      cerr<<"\nvcf_h: "<< vco<<" - "<<feedbk<<" - "<<vcf_l<<" = inf\n"; exit(1);
    }
#endif
    vcf_m += vcf_h * vcfco;
#if 0
    if(isnan(vcf_m)) {
      cerr<<"\n"<< vcf_h<<" * "<<vcfco<<" = nan\n"; exit(1);
    }
    if(isinf(vcf_m)) {
      cerr<<"\nvcf_m: "<< vcf_h<<" * "<<vcfco<<" = inf\n"; exit(1);
    }
#endif
    vcf_l += vcf_m * vcfco;
#if 0
    if(isnan(vcf_l)) {
      cerr<<"\n"<< vcf_m<<" * "<<vcfco<<" = nan\n";
      exit(1);
    }
#endif

    // 2-pole filter #2
    vcf_h2 = vcf_l - 1*vcf_m2 - vcf_l2;
    vcf_m2 += vcf_h2 * vcfco;
    vcf_l2 += vcf_m2 * vcfco;
#define OUT vcf_l2

//    vcf_l2 = vco;

    // ----- VCA -----

    if(trigtime == 4400) {
      if(note_length[(step+1)%steps] != A303_NOTE_TIE) {
        emode = A303_ENV_RELEASE;
      }
    }

    switch(emode) {
    case A303_ENV_ATTACK:
      vcaamp = vcaamp*1.1 + 0.01;
      if(vcaamp>=1.0) { 
        vcaamp = 1.0;
        emode = A303_ENV_DECAY;
      }
      break;
    case A303_ENV_DECAY:     vcaamp = vcaamp * 0.99998; break;
    case A303_ENV_RELEASE:   vcaamp = vcaamp * 0.99; break;
    }

    buffer[channel][sam] = (signed short)(OUT * vcaamp * (40<<8));
   
    trigtime++;
  }
  return AU_BUFSIZE;
}

void
A303::nextStep()
{
  step++;
  if(step>=steps) step = 0;
  if(note_length[step] == A303_NOTE_SEMIQUAVER)  {
    emode = A303_ENV_ATTACK;
    vcfenv = 1.0;
    trigtime = 0;
  } else if(note_length[step] == A303_NOTE_REST) {
    emode = A303_ENV_RELEASE;
  }
  vcofreq_t = note2freq(note[step]);
}

void
A303::clear()
{
  for(int i=0; i<A303_MAXSTEPS; i++) {
    note[i]=0;
    note_length[i] = 0;
    accent[i] = 0;
    slide[i] = 0;
  }
}

void
A303::randomise()
{
  for(int i=0; i<steps; i++) {
    note[i] = random()%36;
    note_length[i] = random()%3;
    //note_length[i] = 1;
    slide[i] = random()%2;
  }
}

int
A303::numKnobs() { 
  return sizeof(knob_desc)/sizeof(knob_desc[0]);
}

double
A303::getKnob(int index)
{
//  return 0.0;
  switch(index) {
  case 0:	return cutoff;
  case 1:	return (reso-0.5)*-2.0;
  case 2:	return 0.0; // not implemented
  }
}

void
A303::setKnob(int index, double val)
{
//  cerr<<"a303::setKnob("<<index<<", "<<val<<")\n";
  switch(index) {
  case 0:	cutoff = val; break;
  case 1:	reso = 0.5-(val/2.0); break;
  case 2:	vcfdecay = pow(0.999,1.0-val); break;
  }
}

const AUKnobDesc*
A303::knobDesc(int index)
{
  return &knob_desc[index];
}

int
A303::numSwitches() { 
  return sizeof(switch_desc)/sizeof(switch_desc[0]);
}

int
A303::getSwitch(int index)
{
  switch(index) {
  case 0: return wave;
  }
}

void
A303::setSwitch(int index, int val)
{
  switch(index) {
  case 0: wave=val%2;
  }
}

const AUSwitchDesc*
A303::switchDesc(int index)
{
  return &switch_desc[index];
}

#undef OUT
