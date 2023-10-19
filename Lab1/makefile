lab1: lab1.o message.o
	gcc -o lab1 lab1.o message.o

lab1.o : lab1.c message.h
	gcc -c lab1.c

message.o : message.c message.h
	gcc -c message.c

clean: 
	rm lab1 lab1.o message.o
