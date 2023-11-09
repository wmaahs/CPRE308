appserver: Bank.o Client.o
	gcc -pthread -o appserver Bank.o Client.o

Client.o: Client.c Client.h
	gcc -c Client.c

Bank.o: Bank.c Bank.h
	gcc -c Bank.c

clean:
	rm appserver Bank.o Client.o