//  a simple Curses-based frontend for the A303 class

#include <curses.h>
#include <ctype.h>
#include <string.h>
#include "AOutput.h"
#include "A303.h"
#include "AU_defs.h"

#define BUFS_PER_STEP(bpm) ((float)(60*AU_SAMPLE_RATE)/(4*AU_BUFSIZE*bpm))

int bpm=120;
float bufs_per_step = BUFS_PER_STEP(120);

au_timestamp_t time=0;
float step_time = 0.0;

#define NUMKNOBS 3
double fco=1.0, reso=0.7, decay=0.9;
static double knobs[NUMKNOBS] = { 1.0, 0.7, 0.9 };

int cursor=0;

#if 0
const char* numerals3x5[5] = {
  "+=+=+ ==+==+|  +==+====++=++=+",
  "| | |   |  || ||  | | / | || |",
  "| | |  =+=/.+=#+=++=+ | +=++=#",
  "| | | |    |  |  || | | | |  |",
  "+=+=+=+====+  |=/ +=+ | +=+  |"
};
#endif
const char* numerals3x5[5] = {
  "lqkqk qqkqqkx  lqqlqqqqklqklqk",
  "x x x   x  xx xx  x    xx xx x",
  "x x x lqjqqumqumqktqk ljtqumqu",
  "x x x x    x  x  xx x x x x  x",
  "mqjqvqmqqqqj  xqqjmqj x mqj  x"
};

// Curses stuff

WINDOW *bpmwin, *seqwin, *knobwin;

A303 tb;
AOutput out(44100,0);

void
decKnob(int i)
{
  if(knobs[i]>=0.05) {
    knobs[i]-=0.05;
    tb.setKnob(i, knobs[i]);
  }
}

void
incKnob(int i)
{
  if(knobs[i]<=0.95) {
    knobs[i]+=0.05;
    tb.setKnob(i, knobs[i]);
  }
}

void
setBPM(int newbpm)
{
  char strbuf[4];
  bpm = newbpm;
  bufs_per_step = BUFS_PER_STEP(newbpm);
  sprintf(strbuf, "%03d", bpm);
  //mvwaddstr(bpmwin, 0, 0, strbuf);
  for(int digit=0; digit<3; digit++) {
    if(strbuf[digit]-'0' <= 9) {
      for(int y=0; y<5; y++) {
        for(int x=0; x<3; x++) {
          char ch = numerals3x5[y][x+(3*(strbuf[digit]-'0'))];
          mvwaddch(bpmwin, y, x+(4*digit), ch>='a'?acs_map[ch]:ch);
        }
      }
    }
  }
  wrefresh(bpmwin);
}

void
drawStep()
{
  static int laststep=0, prelaststep=0;
  int step = tb.getStep();

  mvwaddstr(seqwin, 3, 1+prelaststep*4, "   ");
  mvwaddstr(seqwin, 3, 1+laststep*4, "...");
  mvwaddstr(seqwin, 3, 1+step*4, "***");
  wrefresh(seqwin);

  prelaststep = laststep; laststep = step;
}

void
drawCursor()
{
  static int lastcurs=0;
  mvwaddstr(seqwin, 0, 1+lastcurs*4, "   ");
  mvwaddstr(seqwin, 3, 1+lastcurs*4, "   ");
  mvwaddstr(seqwin, 0, 1+cursor*4, "VVV");
  mvwaddstr(seqwin, 3, 1+cursor*4, "^^^");
  lastcurs = cursor;
//  wrefresh(seqwin);
}

void
drawSeq()
{
  const char* nname[] = { "C ","C#","D ","D#","E ","F ","F#","G ", "G#", "A ", "A#", "B " };
  int s, ns=tb.numSteps();
  for(s=0; s<ns; s++) {
    int note = tb.getNote(s), nl=tb.getNoteLength(s);
    //wattron(seqwin, tb.getNoteLength(s)?A_STANDOUT:A_DIM);
    mvwaddstr(seqwin, 1, 1+s*4, nname[note%12]);
    mvwaddch(seqwin, 1, 3+s*4, '0'+(note/12));
    mvwaddstr(seqwin, 2, 1+s*4, (nl==A303_NOTE_TIE?"<<<":(nl?"===":"---")));
    //if(tb.getNoteLength(s)==A303_NOTE_TIE) mvwaddch(seqwin, 2, 2+s*4, 'T');
    if(tb.getSlide(s)) mvwaddch(seqwin, 2, 3+s*4, '~');
  }
  //wattron(seqwin, A_NORMAL);
  drawCursor();
//  wrefresh(seqwin);
}

void
drawKnobs()
{
  for(int i=0; i<NUMKNOBS; i++) {
    char buf[16];
    sprintf(buf, "%1.3f", knobs[i]);
    mvwaddstr(knobwin, 0, i*8, buf);
  }
  mvwaddstr(knobwin, 0, NUMKNOBS*8, " _        ");
  mvwaddstr(knobwin, 1, NUMKNOBS*8, "| |_| [   ] |\\|\\");
  mvwaddch(knobwin, 1, NUMKNOBS*8+7+(2*tb.getSwitch(0)), '#');
  wrefresh(knobwin);
}

void
setNoteUnderCursor(char c)
{
  int note = tb.getNote(cursor);
  int octave = note/12;
  note = note%12;
  if(c=='O') {
    octave = AU_MIN(octave+1, 3);
  } else if(c=='o') {
    octave = AU_MAX(octave-1, 0);
  } else {
    const char* keys = "zsxdcvgbhnjm";
    c = tolower(c);
    note = strchr(keys, c)?strchr(keys,c)-keys:-1;
  }
  if(note>=0 && note<12) tb.setNote(cursor, octave*12+note);
  drawSeq();
}

void
toggleNote()
{
  int n = tb.getNoteLength(cursor);
  tb.setNoteLength(cursor, (n==A303_NOTE_SEMIQUAVER)?A303_NOTE_REST:A303_NOTE_SEMIQUAVER);
  drawSeq();
}

void
toggleSlide()
{
  tb.setSlide(cursor, !tb.getSlide(cursor));
  drawSeq();
}

void
tieNote()
{
  tb.setNoteLength(cursor, A303_NOTE_TIE);
  drawSeq();
}

main()
{
  out.connectInput(0, &tb);

  tb.randomise();
  for(int i=0; i<3; i++) 
    tb.setKnob(i, knobs[i]);


  WINDOW* scr = initscr();
  noecho();
  cbreak();
  nodelay(scr, TRUE);
  keypad(scr, TRUE);

  bpmwin = newwin(5, 11, 18, 67);
  seqwin = newwin(8, 65, 5, 10);
  knobwin = newwin(4, 65, 0, 10);

  int bored=0;

  drawSeq();
  while(!bored) {
    out.render(0, (au_sample_t*)0, time++);
    step_time += 1.0;
    while(step_time > bufs_per_step) {
      tb.nextStep();
      step_time -= bufs_per_step;
      drawStep();
    }
    int c = getch();
    switch(c) {
      case '[':  setBPM(bpm-1); break;
      case ']':  setBPM(bpm+1); break;
      case '1':  decKnob(0); break;
      case '!':  incKnob(0); break;
      case '2':  decKnob(1); break;
      case '@':  incKnob(1); break;
      case '3':  decKnob(2); break;
      case '#':  incKnob(2); break;

      case KEY_DC: tb.clear(); drawSeq(); break;
      case 'r': tb.randomise(); drawSeq(); break;

      case KEY_LEFT: cursor = (cursor+15)%16; drawCursor(); break;
      case KEY_RIGHT: cursor = (cursor+1)%16; drawCursor(); break;
      case 'z': case 'x': case 'c': case 'v': case 'b': case 'n': case 'm':
      case 's': case 'd': case 'g': case 'h': case 'j': 
      case 'O': case 'o':
        setNoteUnderCursor(c); break;
      case ' ': toggleNote(); break;
      case '~': toggleSlide(); break;
      case 't': tieNote(); break;
      case 'w': tb.setSwitch(0, 1-tb.getSwitch(0)); break;

      case 'q':  bored=1;     break;
    }
    drawKnobs();
  }

  echo();
  nocbreak();
  endwin();


}
