#
# tst - the time series transmogrifier tool
#

CC= gcc 
CFLAGS= -std=gnu99 -g -Werror -Wall

all: tst main.pdf tst.cat

include LaTeX.mk


tst: tst.o options.o tst-split.o tst-t.o

tst.cat: tst.1 tst
	./tst -help 1 | \
	  sed -e 's/^# //' -e 's/\\/\\\\/g' | \
	  sort >tst.so
	nroff -man tst.1 >tst.cat

options.o: options.h

tst-t.o: tst-t.h

test-split:
	gcc -DTEST tst-split.c
	./a.out

test-t:
	gcc -DTEST tst-t.c
	./a.out <test.dates

test-options:
	gcc -DTEST options.c
	./a.out
	./a.out -a arg-a
	./a.out /etc/passwd
	./a.out -bb arg-bb -cc arg-cc
	./a.out -bb arg-bb -cc arg-cc /etc/passwd
	./a.out -bb arg-bb -cc arg-cc /etc/passwd /etc/group

clean::
	rm -f tst a.out *.o *~ test.cat main.pdf

