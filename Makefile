#Makefile

all: Frontend.o Backend.o Promotor.o

Frontend.o: frontend.c

	gcc frontend.c -o Frontend

Backend.o: backend.c

	gcc backend.c -o Backend

Promotor.o: promotor.c

	gcc promotor.c -o Promotor

clean:
	rm Frontend
	rm Backend
	rm Promotor
	rm -rf *.o
