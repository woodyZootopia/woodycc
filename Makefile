woodycc: woodycc.c

test: woodycc
	./test.sh

clean:
	rm -f woodycc *.o *~ tmp*
