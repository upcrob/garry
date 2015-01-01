CC = mpic++ 

svenn: chess.c boardutil.c moves.c strutil.c
	$(CC) -o chess chess.c boardutil.c moves.c strutil.c

clean:
	rm chess
