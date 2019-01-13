CFLAGS=-std=c11 -Wall
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

wdcc: $(OBJS)
	$(CC) -o wdcc $(OBJS) $(LDFLAGS)

$(OBJS): wdcc.h

test: wdcc
	./test.sh

clean:
	rm -f wdcc *.o *~ tmp* *.out
