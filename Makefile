#Makefile

# Compile all

# Flags to be included CFLAGS = -Wall -Werror -Wextra

all: Clean Main

Main: Frontend Backend

Frontend: frontend.c
	gcc -pthread -o Frontend frontend.c

Backend: backend.c
	gcc -c backend.c -pthread
	gcc -o Backend backend.o users_lib.o -pthread

Clean:
	rm Frontend
	rm Backend
	rm backend.o

FifoCleaning:
	rm backend_fifo
	rm backend_fifo_frontend
	rm comms_fifo
	rm alive_fifo



