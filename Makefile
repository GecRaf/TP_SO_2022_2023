default all;

all: Frontend.o Backend.o 

Frontend.o: frontend.c

	gcc frontend.c -o Frontend

Backend.o: backend.c

	gcc backend.c -o Backend

Promotor.o: promotor.c

	gcc promotor.c -o Promotor