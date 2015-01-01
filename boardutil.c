/*	
 * boardutil.c
 * by Rob Upcraft
 * Functions & data structures for simulating chess board positions and moves.
 */

/* Includes */
#include <stdio.h>
#include <stdlib.h>
#include "boardutil.h"

/* Prepares a new game board. */
char* buildNewBoard()
{
	int i, idx;
	char* board = (char*) malloc(64);

	//populate white
	board[0] = ROOK;
	board[1] = KNIGHT;
	board[2] = BISHOP;
	board[3] = QUEEN;
	board[4] = KING;
	board[5] = BISHOP;
	board[6] = KNIGHT;
	board[7] = ROOK;

	for (i = 0, idx = 8; i < 8; i++, idx++)
		board[idx] = PAWN;

	//populate other squares with EMPTY
	for (idx = 16; idx < 48; idx++)
		board[idx] = EMPTY;

	//populate black
	for (i = 0, idx = 48; i < 8; i++, idx++)
		board[idx] = PAWN + BLACK;

	board[56] = ROOK + BLACK;
	board[57] = KNIGHT + BLACK;
	board[58] = BISHOP + BLACK;
	board[59] = QUEEN + BLACK;
	board[60] = KING + BLACK;
	board[61] = BISHOP + BLACK;
	board[62] = KNIGHT + BLACK;
	board[63] = ROOK + BLACK;

	return board;
}

/* Get string value of piece. */
char* toString(char pieceId)
{
	char* str;
	if (pieceId == EMPTY)
	{
		//empty square
		str = (char*) "EMPTY";
	} else if (pieceId > BLACK) {
		//black
		pieceId = (char) (pieceId - BLACK);
		switch (pieceId)
		{
			case PAWN:
				str = (char*) "BLACK PAWN";
				break;
			case ROOK:
				str = (char*) "BLACK ROOK";
				break;
			case KNIGHT:
				str = (char*) "BLACK KNIGHT";
				break;
			case BISHOP:
				str = (char*) "BLACK BISHOP";
				break;
			case QUEEN:
				str = (char*) "BLACK QUEEN";
				break;
			case KING:
				str = (char*) "BLACK KING";
				break;
		}
	} else {
		//white
		switch (pieceId)
		{
			case PAWN:
				str = (char*) "WHITE PAWN";
				break;
			case ROOK:
				str = (char*) "WHITE ROOK";
				break;
			case KNIGHT:
				str = (char*) "WHITE KNIGHT";
				break;
			case BISHOP:
				str = (char*) "WHITE BISHOP";
				break;
			case QUEEN:
				str = (char*) "WHITE QUEEN";
				break;
			case KING:
				str = (char*) "WHITE KING";
				break;
		}
	}
	return str;
}

/* Returns string value of a board position. */
const char* positionString(int row, int col)
{
	const char* str;
	if (row > 7 || row < 0 || col > 7 || col < 0)
	{
		str = (char*) "INVALID LOCATION";
	} else {
		const char* squares[] = {"A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8",
						   "B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8",
						   "C1", "C2", "C3", "C4", "C5", "C6", "C7", "C8",
						   "D1", "D2", "D3", "D4", "D5", "D6", "D7", "D8",
						   "E1", "E2", "E3", "E4", "E5", "E6", "E7", "E8",
						   "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8",
						   "G1", "G2", "G3", "G4", "G5", "G6", "G7", "G8",
						   "H1", "H2", "H3", "H4", "H5", "H6", "H7", "H8"};
		str = squares[col * 8 + row];
	}
	return str;
}

/* Returns the piece (id) at a given position. */
char getPieceAt(char* board, int row, int col)
{
	if (row >= 0 && col >= 0 && row < 8 && col < 8)
		return board[row * 8 + col];
	else
		return -1;
}

/* Set piece (id) at a given position. */
void setPieceAt(char* board, int row, int col, char pieceId)
{
	board[row * 8 + col] = pieceId;
}

/* Returns true if a piece is black.  False if EMPTY or white. */
char isBlack(char pieceId)
{
	return (pieceId > BLACK);
}

/* Returns true if piece is white.  False if EMPTY or black. */
char isWhite(char pieceId)
{
	return !(pieceId == EMPTY || pieceId > BLACK);
}

/* Makes an empty board. */
char* makeEmptyBoard()
{
	return (char*) malloc(64);
}

/* Makes a copy of the board. */
void cloneBoard(char* board, char* newBoard)
{
	int i;
	for (i = 0; i < 64; i++)
		newBoard[i] = board[i];
}

/* Get the total number of pieces on the board. */
int totalPieces(char* board)
{
	int i, count;
	for (i = 0, count = 0; i < 64; i++)
		if (board[i] != EMPTY)
			count++;
	return count;
}

/* Print a visualization of the board state. */
void printBoard(char* board)
{
	char black = '#';
	char white = '@';
	char pieceId;
	int i, j;
	for (i = 7; i >= 0; i--)
	{
		printf("%d|", i + 1);
		for (j = 0; j < 8; j++)
		{
			pieceId = getPieceAt(board, i, j);
			if (pieceId == EMPTY)
			{
				//empty square
				printf("  ");
			} else if (pieceId > BLACK) {
				//black
				pieceId = (char) (pieceId - BLACK);
				switch (pieceId)
				{
					case PAWN:
						printf("%cP", black);
						break;
					case ROOK:
						printf("%cR", black);
						break;
					case KNIGHT:
						printf("%cN", black);
						break;
					case BISHOP:
						printf("%cB", black);
						break;
					case QUEEN:
						printf("%cQ", black);
						break;
					case KING:
						printf("%cK", black);
						break;
				}
			} else {
				//white
				switch (pieceId)
				{
					case PAWN:
						printf("%cP", white);
						break;
					case ROOK:
						printf("%cR", white);
						break;
					case KNIGHT:
						printf("%cN", white);
						break;
					case BISHOP:
						printf("%cB", white);
						break;
					case QUEEN:
						printf("%cQ", white);
						break;
					case KING:
						printf("%cK", white);
						break;
				}
			}

			if (j < 8)
			{
				printf("|");
			}
		}
		printf("\n");
	}

	printf("-|--|--|--|--|--|--|--|--|\n");
	printf(" | A| B| C| D| E| F| G| H|\n");
}
