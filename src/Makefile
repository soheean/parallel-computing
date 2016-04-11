
SHELL = /bin/bash

CXX = g++

CFLAGS = -I. -O3 -Wall
LIBS = -lpthread

SRCS = dealer.cpp read_data.cpp write_file.cpp \
       interpolation.cpp idw.cpp 

HDRS = forcingMaker.h \
       matrix.hpp Time.hpp 

OBJS = $(SRCS:%.cpp=%.o) 

TARG = fm

.PHONY : clean
.PHONY : all
.PHONY : build
.PHONY : link

all : 
	make build
	make link

clean :
	\rm -f *.o $(TARG)$(EXT)

build : 
	$(CXX) $(CFRAGS) -c $(SRCS) $(LIBS)

link : $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) -o $(TARG)$(SRC) $(LIBS)

