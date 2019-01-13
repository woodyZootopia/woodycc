CFLAGS=-g -std=c11 -Wall
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

wdcc: $(OBJS)
	$(CC) -o wdcc $(OBJS) $(LDFLAGS)

$(OBJS): wdcc.h

test: wdcc
	./wdcc -test
	./test.sh

format:
	clang-format -i $(SRCS)

clean:
	rm -f wdcc *.o *~ tmp* *.out
