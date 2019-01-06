CFLAGS = -g -std=c11 -Wall
wdcc: wdcc.c

test: wdcc
	./test.sh

clean:
	rm -f wdcc *.o *~ tmp* *.out
