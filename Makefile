OUTDEVICE	= AOutput.o 
SYSLIBS	= 
CXX	= g++

OBJS	= AUnit.o A303.o \
	$(OUTDEVICE)

#CXXFLAGS	= -g
CXXFLAGS	= -O6

all:	toy303

toy303:	toy303.o $(OBJS)
	$(CXX) -o toy303 toy303.o $(OBJS) -lm -lcurses $(SYSLIBS)


clean:
	/bin/rm -f $(OBJS) toy303 toy303.o

depend:
	$(CXX) -MM $(MYINCLUDES) *.cc >.depend

include .depend
