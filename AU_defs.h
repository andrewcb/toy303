//  definitions for the audio-processing unit system

#ifndef __AU_DEFS_H
#define __AU_DEFS_H

#define AU_SAMPLE_RATE 44100

typedef signed short    au_sample_t;
typedef unsigned int    au_index_t;
typedef signed int      au_timestamp_t;

#define AU_LOG2_BUFSIZE	9
#define AU_BUFSIZE      (1<<AU_LOG2_BUFSIZE)
#define AU_BUFBYTES     AU_BUFSIZE*sizeof(au_sample_t)
typedef au_sample_t     au_buffer_t[AU_BUFSIZE];

#define AU_MAX_OUT_CHANNELS     16

#define AU_MIN(a,b)  ((a<b)?(a):(b))
#define AU_MAX(a,b)  ((a>b)?(a):(b))

//enum param_type {PARAM_REAL = 0, PARAM_DISCRETE = 1};

// params are being replaced with two types of parameters: Knobs, which 
// are continuous values (of type double) and Switches, which are discrete.

typedef struct AUKnobDesc {
  const char* name;
  double min;
  double max;
} AUKnobDesc;

typedef struct AUSwitchDesc {
  const char* name;
  int min;
  int max;
} AUSwitchDesc;

#endif
