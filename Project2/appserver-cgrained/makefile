appserver-coarse: Bank.o appserver-coarse.o
	gcc -pthread -o appserver-coarse Bank.o appserver-coarse.o

appserver-coarse.o: appserver-coarse.c appserver-coarse.h
	gcc -c appserver-coarse.c

Bank.o: Bank.c Bank.h
	gcc -c Bank.c

clean:
	rm appserver-coarse Bank.o appserver-coarse.o