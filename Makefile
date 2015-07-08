OBJ = myserver.o 

all: clean server run

	

server: myserver.o 
	$(CC) -o $@ $^ -lpthread 
myserver: myserver.o
	$(CC) -c $@ $^ 




clean:
	clear
	rm -f *.o  server
run:
	./server
