#Makefile

# Compile all

# Flags to be included CFLAGS = -Wall -Werror -Wextra

all: Frontend Backend

Frontend: frontend.c
	gcc -c frontend.c -o Frontend

Backend: backend.c
	gcc -c backend.c -pthread
	gcc -o Backend backend.o users_lib.o -pthread

Clean:
	rm Frontend
	rm Backend
	rm backend.o

FifoCleaning:
	rm backend_fifo
	rm backend_frontend_fifo



