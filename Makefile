CFLAGS=-g -std=c11 -Wall
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

wdcc: $(OBJS)
	$(CC) -g -o wdcc $(OBJS) $(LDFLAGS)

$(OBJS): wdcc.h

test: wdcc
	./test.sh

format:
	clang-format -i $(SRCS)

clean:
	rm -f wdcc *.o *~ tmp* *.out

.PHONY: test clean
# remove object files except for foo.o
.INTERMEDIATE: $(filter-out foo.o, $(OBJS))
