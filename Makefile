#
# tst - the time series transmogrifier tool
#

CC= gcc 
CFLAGS= -std=c99 -g -Werror -Wall

include LaTeX.mk

all: tst main.pdf tst.cat

tst: tst.c options.c

tst.cat: tst.1
	nroff -man tst.1 >tst.cat

options.o: options.h

test-options:
	gcc -DTEST options.c
	./a.out
	./a.out -a arg-a
	./a.out /etc/passwd
	./a.out -bb arg-bb -cc arg-cc
	./a.out -bb arg-bb -cc arg-cc /etc/passwd
	./a.out -bb arg-bb -cc arg-cc /etc/passwd /etc/group

clean::
	rm -f tst a.out *.o

