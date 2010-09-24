//  A303.h  a TB-303-like bassline synth with step sequencer.

#ifndef __A303_H
#define __A303_H

#include "AUnit.h"

// based partly on Lars Hamre's TB-303 emulator

#define A303_MAXSTEPS 16

// note lengths
enum {
  A303_NOTE_REST = 0,
  A303_NOTE_SEMIQUAVER = 1,
  A303_NOTE_TIE = 2
};

enum {
  A303_ENV_ATTACK = 0,
  A303_ENV_DECAY = 1,
  A303_ENV_RELEASE = 2
};

enum {
  A303_WAVE_SQUARE = 0,
  A303_WAVE_SAWTOOTH = 1
};


class A303 : public AUnit {
private:
  int note[A303_MAXSTEPS];
  int note_length[A303_MAXSTEPS];
  int accent[A303_MAXSTEPS];
  int slide[A303_MAXSTEPS];

  int steps;	// actual number of steps; can be adjusted down
  double cutoff;// filter cutoff
  double reso;	// low value == max. resonance
  double vcfdecay;	// env. multiplier
  int emode;	// env. mode
  int step;
  int wave;

  double vcofreq_t; // target oscillator frequency;
  double vcofreq;   // actual osc. frequency
  double vcophase;
  float vco_t, vco;
  double vcaamp;	// VCA amplitude
  double vcfenv;

  double vcf_h, vcf_m, vcf_l, vcf_h2, vcf_m2, vcf_l2;

  int trigtime;		// samples since trigger
  
public:
  A303();
  virtual au_index_t userRender(int channel, au_timestamp_t time);

  void nextStep();
  void clear();
  void randomise();
  void prime() { step=10000; nextStep(); }

  inline int getNote(int index) { return note[index]; }
  inline int getNoteLength(int index) { return note_length[index]; }
  inline int getAccent(int index) { return accent[index]; }
  inline int getSlide(int index) { return slide[index]; }
  inline void setNote(int index, int val) {  note[index] = val; }
  inline void setNoteLength(int index, int val) {  note_length[index] = val; }
  inline void setAccent(int index, int val) {  accent[index] = val; }
  inline void setSlide(int index, int val) {  slide[index] = val; }

  inline int getStep() { return step; }
  inline int numSteps() { return steps; }
  inline void setNumSteps(int s) { steps=s; if(step>=s)step=0; }

  virtual int numKnobs();
  virtual double getKnob(int index);
  virtual void setKnob(int index, double val);
  virtual const AUKnobDesc* knobDesc(int index);
  virtual int numSwitches();
  virtual int getSwitch(int index);
  virtual void setSwitch(int index, int val);
  virtual const AUSwitchDesc* switchDesc(int index);
};

#endif

