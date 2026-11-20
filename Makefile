CC = gcc
CFLAGS = -Wall
SRC = main.c balloons.c
LIBS = -lraylib -lm

jogo: $(SRC) balloons.h estouro.h upgrades.h explosao.h boss.h
	$(CC) $(CFLAGS) $(SRC) -o jogo $(LIBS)

clean:
	rm -f jogo