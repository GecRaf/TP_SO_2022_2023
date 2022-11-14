#Makefile

all: Frontend.o Backend.o 

Frontend.o: frontend.c

	gcc frontend.c -o Frontend

Backend.o: backend.c

	gcc backend.c -o Backend



clean:
	rm Frontend
	rm Backend
	rm Promotor
	rm -rf *.o
