CC=g++ -std=c++11

modche: main.o
	$(CC) main.o -o modche
main.o: main.cpp
	$(CC) -c main.cpp 
clean:
	rm *o modche 


