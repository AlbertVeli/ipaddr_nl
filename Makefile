UNAME_S := $(shell uname -s)
ifneq ($(UNAME_S),Linux)
$(error Linux not detected, quitting)
endif
CC = gcc

EXE = ipaddr_nl
OBJS = $(EXE).o
EXE2 = iplink_nl
OBJS2 = $(EXE2).o

NLCFLAGS = $(shell pkg-config --cflags libnl-route-3.0)
CFLAGS = -g -O0 -W -Wall $(NLCFLAGS)
NLLIBS = $(shell pkg-config --libs libnl-route-3.0)
LIBS = $(NLLIBS)

all: $(EXE) $(EXE2)

$(EXE): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

$(EXE2): $(OBJS2)
	$(CC) $(CFLAGS) -o $@ $(OBJS2) $(LIBS)

.PHONY: clean

clean:
	rm -f $(OBJS) $(EXE) $(OBJS2) $(EXE2)
