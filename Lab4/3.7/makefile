shm_client.o: shm_client.c
	gcc -c shm_client.c

shm_client_out: shm_client.o
	gcc -o shm_client_out shm_client.o

shm_server.o: shm_server.c
	gcc -c shm_server.c

shm_server_out: shm_server.o
	gcc -o shm_server_out shm_server.o

clean: 
	rm pipes_out pipes.o