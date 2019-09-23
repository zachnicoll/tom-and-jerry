# Makefile for the ZDK
#
# $Revision:Sat Feb 23 00:47:31 EAST 2019$

TARGETS=libzdk.a 

FLAGS=-Wall -Werror -std=gnu99 -g
LIB_SRC=cab202_graphics.c cab202_timers.c
LIB_HDR=cab202_graphics.h cab202_timers.h
LIB_OBJ=cab202_graphics.o cab202_timers.o

all: $(TARGETS)

clean:
	for f in $(TARGETS); do \
		if [ -f $${f} ]; then rm $${f}; fi; \
		if [ -f $${f}.exe ]; then rm $${f}.exe; fi; \
	done

rebuild: clean all

libzdk.a: $(LIB_SRC) $(LIB_HDR)
	gcc -c $(LIB_SRC) $(FLAGS)
	ar r $@ $(LIB_OBJ)
	rm $(LIB_OBJ)
