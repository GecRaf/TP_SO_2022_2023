#Makefile

# Compile all

all: Frontend Backend

Frontend: frontend.c
	gcc -c frontend.c -o Frontend

Backend: backend.c
	gcc -c backend.c -pthread
	gcc -o Backend backend.o users_lib.o

Clean:
	rm Frontend
	rm Backend
	rm backend.o



