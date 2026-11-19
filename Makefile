CC = gcc
CFLAGS = -Wall
SRC = main.c balloons.c
LIBS = -lraylib -lm

jogo: $(SRC) balloons.h
	$(CC) $(CFLAGS) $(SRC) -o jogo $(LIBS)

clean:
	rm -f jogo