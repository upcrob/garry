/*
 * moves.c
 * by Rob Upcraft
 * Functions for determining possible moves for a given board configuration.
 */

/* Includes */
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "stddef.h"
#include "strutil.h"
#include "boardutil.h"
#include "moves.h"
#include <mpi.h>

using namespace MPI;

//Variables for castling
static char whiteKCastleInvalid = FALSE;
static char whiteQCastleInvalid = FALSE;
static char blackKCastleInvalid = FALSE;
static char blackQCastleInvalid = FALSE;

/*	Returns TRUE or FALSE if king of a given color is in check. */
char kingInCheck(char* board, char color)
{
	int i, j, row, col;
	if (color == WHITE)
	{
		//Find white king
		for (i = 0; i < 8; i++)
		{
			for (j = 0; j < 8; j++)
			{
				if (getPieceAt(board, i, j) == KING)
				{
					row = i;
					col = j;
				}
			}
		}

		return !pieceSafe(board, row, col);
	} else {
		//Find black king
		for (i = 0; i < 8; i++)
		{
			for (j = 0; j < 8; j++)
			{
				if (getPieceAt(board, i, j) == KING + BLACK)
				{
					row = i;
					col = j;
				}
			}
		}

		return !pieceSafe(board, row, col);
	}
}

/*
 * Determine if a piece is safe.
 */
char pieceSafe(char* board, int row, int col)
{
	int i, j;
	if (isWhite(getPieceAt(board, row, col)))
	{
		//White
		//Check for pawns
		if (row + 1 < 8 && col - 1 >= 0 &&
			getPieceAt(board, row + 1, col - 1) == PAWN + BLACK)
			return FALSE;
		if (row + 1 < 8 && col + 1 < 8 &&
			getPieceAt(board, row + 1, col - 1) == PAWN + BLACK)
			return FALSE;

		//Check general diagonals (bishop / queen / king)
		//Check diagonal to upper right
		for (i = row + 1, j = col + 1; i < 8 && j < 8; i++, j++)
		{
			char p = getPieceAt(board, i, j);
			if (p != EMPTY)
			{
				if (!isBlack(p))
					break;	//this piece is friendly, break

				//piece is black
				if (p == BISHOP + BLACK || p == QUEEN + BLACK ||
					(p == KING + BLACK && i - row == 1))
				{
					return FALSE;	//piece threatens given location
				} else {
					break;	//non-threatening piece
				}
			}
		}

		//Check diagonal to upper left
		for (i = row + 1, j = col - 1; i < 8 && j >= 0; i++, j--)
		{
			char p = getPieceAt(board, i, j);
			if (p != EMPTY)
			{
				if (!isBlack(p))
					break;	//this piece is friendly, break

				//piece is black
				if (p == BISHOP + BLACK || p == QUEEN + BLACK ||
					(i - row == 1 && p == KING + BLACK))
				{
					return FALSE;	//piece threatens given location
				} else {
					break;	//non-threatening piece
				}
			}
		}

		//Check diagonal to lower right
		for (i = row - 1, j = col + 1; i >= 0 && j < 8; i--, j++)
		{
			char p = getPieceAt(board, i, j);
			if (p != EMPTY)
			{
				if (!isBlack(p))
					break;	//this piece is friendly, break

				//piece is black
				if (p == BISHOP + BLACK || p == QUEEN + BLACK ||
					(row - i == 1 && p == KING + BLACK))
				{
					return FALSE;	//piece threatens given location
				} else {
					break;	//non-threatening piece
				}
			}
		}

		//Check diagonal to lower left
		for (i = row - 1, j = col - 1; i >= 0 && j >= 0; i--, j--)
		{
			char p = getPieceAt(board, i, j);
			if (p != EMPTY)
			{
				if (!isBlack(p))
					break;	//this piece is friendly, break

				//piece is black
				if (p == BISHOP + BLACK || p == QUEEN + BLACK ||
					(row - i == 1 && p == KING + BLACK))
				{
					return FALSE;	//piece threatens given location
				} else {
					break;	//non-threatening piece
				}
			}
		}

		//Check horizontal (queen, rook, king)
		//Check to the right
		for (j = col + 1; j < 8; j++)
		{
			char p = getPieceAt(board, row, j);
			if (p != EMPTY)
			{
				if (!isBlack(p))
					break;	//this piece is friendly, break

				//piece is black
				if (p == ROOK + BLACK || p == QUEEN + BLACK ||
					(j - col == 1 && p == KING + BLACK))
				{
					return FALSE;	//piece threatens given location
				} else {
					break;	//non-threatening piece
				}
			}
		}

		//Check to the left
		for (j = col - 1; j >= 0; j--)
		{
			char p = getPieceAt(board, row, j);
			if (p != EMPTY)
			{
				if (!isBlack(p))
					break;	//this piece is friendly, break

				//piece is black
				if (p == ROOK + BLACK || p == QUEEN + BLACK ||
					(col - j == 1 && p == KING + BLACK))
				{
					return FALSE;	//piece threatens given location
				} else {
					break;	//non-threatening piece
				}
			}
		}

		//Check vertical (queen, rook, king)
		//Check up
		for (i = row + 1; i < 8; i++)
		{
			char p = getPieceAt(board, i, col);
			if (p != EMPTY)
			{
				if (!isBlack(p))
					break;	//this piece is friendly, break

				//piece is black
				if (p == ROOK + BLACK || p == QUEEN + BLACK ||
					(i - row == 1 && p == KING + BLACK))
				{
					return FALSE;	//piece threatens given location
				} else {
					break;	//non-threatening piece
				}
			}
		}

		//Check down
		for (i = row - 1; i >= 0; i--)
		{
			char p = getPieceAt(board, i, col);
			if (p != EMPTY)
			{
				if (!isBlack(p))
					break;	//this piece is friendly, break

				//piece is black
				if (p == ROOK + BLACK || p == QUEEN + BLACK ||
					(row - i == 1 && p == KING + BLACK))
				{
					return FALSE;	//piece threatens given location
				} else {
					break;	//non-threatening piece
				}
			}
		}

		//Check for knights
		//Up 2
		i = row + 2;
		if (i < 8)
		{
			//Left
			j = col - 1;
			if (j >= 0)
			{
				char p = getPieceAt(board, i, j);
				if (p != EMPTY)
				{
					//check to see if piece is a knight
					if (p == KNIGHT + BLACK)
						return FALSE;
				}
			}

			//Right
			j = col + 1;
			if (j < 8)
			{
				char p = getPieceAt(board, i, j);
				if (p != EMPTY)
				{
					//check to see if piece is a knight
					if (p == KNIGHT + BLACK)
						return FALSE;
				}
			}
		}

		//Down 2
		i = row - 2;
		if (i >= 0)
		{
			//Left
			j = col - 1;
			if (j >= 0)
			{
				char p = getPieceAt(board, i, j);
				if (p != EMPTY)
				{
					//check to see if piece is a knight
					if (p == KNIGHT + BLACK)
						return FALSE;
				}
			}

			//Right
			j = col + 1;
			if (j < 8)
			{
				char p = getPieceAt(board, i, j);
				if (p != EMPTY)
				{
					//check to see if piece is a knight
					if (p == KNIGHT + BLACK)
						return FALSE;
				}
			}
		}

		//Left 2
		j = col - 2;
		if (j >= 0)
		{
			//Up
			i = row + 1;
			if (i < 8)
			{
				char p = getPieceAt(board, i, j);
				if (p != EMPTY)
				{
					//check to see if piece is a knight
					if (p == KNIGHT + BLACK)
						return FALSE;
				}
			}

			//Down
			i = row - 1;
			if (i >= 0)
			{
				char p = getPieceAt(board, i, j);
				if (p != EMPTY)
				{
					//check to see if piece is a knight
					if (p == KNIGHT + BLACK)
						return FALSE;
				}
			}
		}

		//Right 2
		j = col + 2;
		if (j < 8)
		{
			//Up
			i = row + 1;
			if (i < 8)
			{
				char p = getPieceAt(board, i, j);
				if (p != EMPTY)
				{
					//check to see if piece is a knight
					if (p == KNIGHT + BLACK)
						return FALSE;
				}
			}

			//Down
			i = row - 1;
			if (i >= 0)
			{
				char p = getPieceAt(board, i, j);
				if (p != EMPTY)
				{
					//check to see if piece is a knight
					if (p == KNIGHT + BLACK)
						return FALSE;
				}
			}
		}

		//No threats found
		return TRUE;
	} else if (isBlack(getPieceAt(board, row, col))) {
		//Black
		//Check for pawns
		if (row - 1 < 8 && col - 1 >= 0 &&
			getPieceAt(board, row - 1, col - 1) == PAWN)
			return FALSE;
		if (row - 1 < 8 && col + 1 < 8 &&
			getPieceAt(board, row - 1, col + 1) == PAWN)
			return FALSE;

		//Check general diagonals (bishop / queen / king)
		//Check diagonal to upper right
		for (i = row + 1, j = col + 1; i < 8 && j < 8; i++, j++)
		{
			char p = getPieceAt(board, i, j);
			if (p != EMPTY)
			{
				if (isBlack(p))
					break;	//this piece is friendly, break

				//piece is black
				if (p == BISHOP || p == QUEEN ||
					(p == KING && i - row == 1))
				{
					return FALSE;	//piece threatens given location
				} else {
					break;	//non-threatening piece
				}
			}
		}

		//Check diagonal to upper left
		for (i = row + 1, j = col - 1; i < 8 && j >= 0; i++, j--)
		{
			char p = getPieceAt(board, i, j);
			if (p != EMPTY)
			{
				if (isBlack(p))
					break;	//this piece is friendly, break

				//piece is black
				if (p == BISHOP || p == QUEEN ||
					(i - row == 1 && p == KING))
				{
					return FALSE;	//piece threatens given location
				} else {
					break;	//non-threatening piece
				}
			}
		}

		//Check diagonal to lower right
		for (i = row - 1, j = col + 1; i >= 0 && j < 8; i--, j++)
		{
			char p = getPieceAt(board, i, j);
			if (p != EMPTY)
			{
				if (isBlack(p))
					break;	//this piece is friendly, break

				//piece is black
				if (p == BISHOP || p == QUEEN ||
					(row - i == 1 && p == KING))
				{
					return FALSE;	//piece threatens given location
				} else {
					break;	//non-threatening piece
				}
			}
		}

		//Check diagonal to lower left
		for (i = row - 1, j = col - 1; i >= 0 && j >= 0; i--, j--)
		{
			char p = getPieceAt(board, i, j);
			if (p != EMPTY)
			{
				if (isBlack(p))
					break;	//this piece is friendly, break

				//piece is black
				if (p == BISHOP || p == QUEEN ||
					(row - i == 1 && p == KING))
				{
					return FALSE;	//piece threatens given location
				} else {
					break;	//non-threatening piece
				}
			}
		}

		//Check horizontal (queen, rook, king)
		//Check to the right
		for (j = col + 1; j < 8; j++)
		{
			char p = getPieceAt(board, row, j);
			if (p != EMPTY)
			{
				if (isBlack(p))
					break;	//this piece is friendly, break

				//piece is black
				if (p == ROOK || p == QUEEN ||
					(j - col == 1 && p == KING))
				{
					return FALSE;	//piece threatens given location
				} else {
					break;	//non-threatening piece
				}
			}
		}

		//Check to the left
		for (j = col - 1; j >= 0; j--)
		{
			char p = getPieceAt(board, row, j);
			if (p != EMPTY)
			{
				if (isBlack(p))
					break;	//this piece is friendly, break

				//piece is black
				if (p == ROOK || p == QUEEN ||
					(col - j == 1 && p == KING))
				{
					return FALSE;	//piece threatens given location
				} else {
					break;	//non-threatening piece
				}
			}
		}

		//Check vertical (queen, rook, king)
		//Check up
		for (i = row + 1; i < 8; i++)
		{
			char p = getPieceAt(board, i, col);
			if (p != EMPTY)
			{
				if (isBlack(p))
					break;	//this piece is friendly, break

				//piece is black
				if (p == ROOK || p == QUEEN ||
					(i - row == 1 && p == KING))
				{
					return FALSE;	//piece threatens given location
				} else {
					break;	//non-threatening piece
				}
			}
		}

		//Check down
		for (i = row - 1; i >= 0; i--)
		{
			char p = getPieceAt(board, i, col);
			if (p != EMPTY)
			{
				if (isBlack(p))
					break;	//this piece is friendly, break

				//piece is black
				if (p == ROOK || p == QUEEN ||
					(row - i == 1 && p == KING))
				{
					return FALSE;	//piece threatens given location
				} else {
					break;	//non-threatening piece
				}
			}
		}

		//Check for knights
		//Up 2
		i = row + 2;
		if (i < 8)
		{
			//Left
			j = col - 1;
			if (j >= 0)
			{
				char p = getPieceAt(board, i, j);
				if (p != EMPTY)
				{
					//check to see if piece is a knight
					if (p == KNIGHT)
						return FALSE;
				}
			}

			//Right
			j = col + 1;
			if (j < 8)
			{
				char p = getPieceAt(board, i, j);
				if (p != EMPTY)
				{
					//check to see if piece is a knight
					if (p == KNIGHT)
						return FALSE;
				}
			}
		}

		//Down 2
		i = row - 2;
		if (i >= 0)
		{
			//Left
			j = col - 1;
			if (j >= 0)
			{
				char p = getPieceAt(board, i, j);
				if (p != EMPTY)
				{
					//check to see if piece is a knight
					if (p == KNIGHT)
						return FALSE;
				}
			}

			//Right
			j = col + 1;
			if (j < 8)
			{
				char p = getPieceAt(board, i, j);
				if (p != EMPTY)
				{
					//check to see if piece is a knight
					if (p == KNIGHT)
						return FALSE;
				}
			}
		}

		//Left 2
		j = col - 2;
		if (j >= 0)
		{
			//Up
			i = row + 1;
			if (i < 8)
			{
				char p = getPieceAt(board, i, j);
				if (p != EMPTY)
				{
					//check to see if piece is a knight
					if (p == KNIGHT)
						return FALSE;
				}
			}

			//Down
			i = row - 1;
			if (i >= 0)
			{
				char p = getPieceAt(board, i, j);
				if (p != EMPTY)
				{
					//check to see if piece is a knight
					if (p == KNIGHT)
						return FALSE;
				}
			}
		}

		//Right 2
		j = col + 2;
		if (j < 8)
		{
			//Up
			i = row + 1;
			if (i < 8)
			{
				char p = getPieceAt(board, i, j);
				if (p != EMPTY)
				{
					//check to see if piece is a knight
					if (p == KNIGHT)
						return FALSE;
				}
			}

			//Down
			i = row - 1;
			if (i >= 0)
			{
				char p = getPieceAt(board, i, j);
				if (p != EMPTY)
				{
					//check to see if piece is a knight
					if (p == KNIGHT)
						return FALSE;
				}
			}
		}

		//No threats found
		return TRUE;
	} else {
		//Empty
		return TRUE;
	}
}

/*
 * Determine if a space is safe.
 */
char spaceSafe(char* board, char color, int row, int col)
{
	if (row < 0 || row > 7 || col < 0 || col > 7)
	{
		return FALSE;
	} else {
		if (getPieceAt(board, row, col) != EMPTY)
		{
			return FALSE;
		} else {
			char p;
			if (color == WHITE)
				p = PAWN;
			else
				p = BLACK + PAWN;
			setPieceAt(board, row, col, p);
			if (pieceSafe(board, row, col))
			{
				setPieceAt(board, row, col, EMPTY);
				return TRUE;
			} else {
				setPieceAt(board, row, col, EMPTY);
				return FALSE;
			}
		}
	}
}

/*
 * Get legal moves.
 */
MoveSet* getLegalMoves(char* board, int color, Move* prevMove, int evaluator)
{
	int i, j, y, x;
	int size = 20;
	char p, c, t;
	MoveSet* moves = newMoveSet();
	char* tmpBoard = buildNewBoard();

	//iterate through each piece on the board
	for (i = 0; i < 8; i++)
	{
		for (j = 0; j < 8; j++)
		{
			//check to see if this piece is of the correct color
			p = getPieceAt(board, i, j);
			if (color == WHITE)
			{
				if (isWhite(p))
				{
					//this is a white piece
					switch (p)
					{
						case PAWN:
							//try moving forward
							cloneBoard(board, tmpBoard);
							if (i + 1 < 8 && getPieceAt(tmpBoard, i + 1, j) == EMPTY)
							{
								setPieceAt(tmpBoard, i + 1, j, p);
								setPieceAt(tmpBoard, i, j, EMPTY);
								if (!kingInCheck(tmpBoard, WHITE))
									addMove(moves, p, i, j, i + 1, j, evaluateBoard(tmpBoard, evaluator));
							}

							//try moving diagonally
							cloneBoard(board, tmpBoard);
							if (i + 1 < 8 && j + 1 < 8 && isBlack(getPieceAt(board, i + 1, j + 1)))
							{
								setPieceAt(tmpBoard, i + 1, j + 1, p);
								setPieceAt(tmpBoard, i, j, EMPTY);
								if (!kingInCheck(tmpBoard, WHITE))
									addMove(moves, p, i, j, i + 1, j + 1, evaluateBoard(tmpBoard, evaluator));
							}

							cloneBoard(board, tmpBoard);
							if (i + 1 < 8 && j - 1 >= 0 && isBlack(getPieceAt(board, i + 1, j - 1)))
							{
								setPieceAt(tmpBoard, i + 1, j - 1, p);
								setPieceAt(tmpBoard, i, j, EMPTY);
								if (!kingInCheck(tmpBoard, WHITE))
									addMove(moves, p, i, j, i + 1, j - 1, evaluateBoard(tmpBoard, evaluator));
							}

							//try moving 2 spaces forward
							cloneBoard(board, tmpBoard);
							if (i == 1 && getPieceAt(board, 2, j) == EMPTY &&
								getPieceAt(board, 3, j) == EMPTY)
							{
								setPieceAt(tmpBoard, 3, j, p);
								setPieceAt(tmpBoard, 1, j, EMPTY);
								if (!kingInCheck(tmpBoard, WHITE))
									addMove(moves, p, 1, j, 3, j, evaluateBoard(tmpBoard, evaluator));
							}
							
							//check for en passant
							if (i == 4)	//make sure pawn is on its fifth rank
							{
								if (prevMove->endRow == 4 && prevMove->startRow == 6 &&
									prevMove->endCol == j - 1 && prevMove->pieceId == PAWN + BLACK)
								{
									cloneBoard(board, tmpBoard);
									setPieceAt(tmpBoard, 5, j - 1, p);		//move current pawn
									setPieceAt(tmpBoard, 4, j, EMPTY);		//empty current space
									setPieceAt(tmpBoard, 4, j - 1, EMPTY);	//capture enemy pawn
									if (!kingInCheck(tmpBoard, WHITE))
										addMove(moves, p, 4, j, 5, j - 1, evaluateBoard(tmpBoard, evaluator));
								} else if (prevMove->endRow == 4 && prevMove->startRow == 6 && 
									prevMove->endCol == j + 1 && prevMove->pieceId == PAWN + BLACK) {
									cloneBoard(board, tmpBoard);
									setPieceAt(tmpBoard, 5, j + 1, p);		//move current pawn
									setPieceAt(tmpBoard, 4, j, EMPTY);		//empty current space
									setPieceAt(tmpBoard, 4, j + 1, EMPTY);	//capture enemy pawn
									if (!kingInCheck(tmpBoard, WHITE))
										addMove(moves, p, 4, j, 5, j + 1, evaluateBoard(tmpBoard, evaluator));
								}
							}
							break;
						case ROOK:
							//try moving up
							cloneBoard(board, tmpBoard);
							for (y = i + 1; y < 8; y++)
							{
								t = getPieceAt(board, y, j);
								if (t == EMPTY)
								{
									setPieceAt(tmpBoard, y, j, p);
									setPieceAt(tmpBoard, y - 1, j, EMPTY);
									if (!kingInCheck(tmpBoard, WHITE))
										addMove(moves, p, i, j, y, j, evaluateBoard(tmpBoard, evaluator));
								} else if (!isBlack(t)) {
									break;	//collided with piece on same team
								} else {
									//this is an enemy piece, capture it
									setPieceAt(tmpBoard, y, j, p);
									setPieceAt(tmpBoard, y - 1, j, EMPTY);
									if (!kingInCheck(tmpBoard, WHITE))
										addMove(moves, p, i, j, y, j, evaluateBoard(tmpBoard, evaluator));
									break;
								}
							}

							//try moving down
							cloneBoard(board, tmpBoard);
							for (y = i - 1; y >= 0; y--)
							{
								t = getPieceAt(board, y, j);
								if (t == EMPTY)
								{
									setPieceAt(tmpBoard, y, j, p);
									setPieceAt(tmpBoard, y + 1, j, EMPTY);
									if (!kingInCheck(tmpBoard, WHITE))
										addMove(moves, p, i, j, y, j, evaluateBoard(tmpBoard, evaluator));
								} else if (!isBlack(t)) {
									break;	//collided with piece on same team
								} else {
									//this is an enemy piece, capture it
									setPieceAt(tmpBoard, y, j, p);
									setPieceAt(tmpBoard, y + 1, j, EMPTY);
									if (!kingInCheck(tmpBoard, WHITE))
										addMove(moves, p, i, j, y, j, evaluateBoard(tmpBoard, evaluator));
									break;
								}
							}

							//try moving left
							cloneBoard(board, tmpBoard);
							for (x = j - 1; x >= 0; x--)
							{
								t = getPieceAt(board, i, x);
								if (t == EMPTY)
								{
									setPieceAt(tmpBoard, i, x, p);
									setPieceAt(tmpBoard, i, x + 1, EMPTY);
									if (!kingInCheck(tmpBoard, WHITE))
										addMove(moves, p, i, j, i, x, evaluateBoard(tmpBoard, evaluator));
								} else if (!isBlack(t)) {
									break;	//collided with piece on same team
								} else {
									//this is an enemy piece, capture it
									setPieceAt(tmpBoard, i, x, p);
									setPieceAt(tmpBoard, i, x + 1, EMPTY);
									if (!kingInCheck(tmpBoard, WHITE))
										addMove(moves, p, i, j, i, x, evaluateBoard(tmpBoard, evaluator));
									break;
								}
							}

							//try moving right
							cloneBoard(board, tmpBoard);
							for (x = j + 1; x < 8; x++)
							{
								t = getPieceAt(board, i, x);
								if (t == EMPTY)
								{
									setPieceAt(tmpBoard, i, x, p);
									setPieceAt(tmpBoard, i, x - 1, EMPTY);
									if (!kingInCheck(tmpBoard, WHITE))
										addMove(moves, p, i, j, i, x, evaluateBoard(tmpBoard, evaluator));
								} else if (!isBlack(t)) {
									break;	//collided with piece on same team
								} else {
									//this is an enemy piece, capture it
									setPieceAt(tmpBoard, i, x, p);
									setPieceAt(tmpBoard, i, x - 1, EMPTY);
									if (!kingInCheck(tmpBoard, WHITE))
										addMove(moves, p, i, j, i, x, evaluateBoard(tmpBoard, evaluator));
									break;
								}
							}

							break;
						case KNIGHT:
							//2 up, 1 right
							y = i + 2;
							x = j + 1;
							t = getPieceAt(board, y, x);
							if (y < 8 && x < 8)
							{
								if (t == EMPTY || isBlack(t))
								{
									cloneBoard(board, tmpBoard);
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, i, j, EMPTY);
									if (!kingInCheck(tmpBoard, WHITE))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
								}
							}

							//2 up, 1 left
							y = i + 2;
							x = j - 1;
							t = getPieceAt(board, y, x);
							if (y < 8 && x >= 0)
							{
								if (t == EMPTY || isBlack(t))
								{
									cloneBoard(board, tmpBoard);
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, i, j, EMPTY);
									if (!kingInCheck(tmpBoard, WHITE))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
								}
							}

							//1 up, 2 right
							y = i + 1;
							x = j + 2;
							t = getPieceAt(board, y, x);
							if (y < 8 && x < 8)
							{
								if (t == EMPTY || isBlack(t))
								{
									cloneBoard(board, tmpBoard);
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, i, j, EMPTY);
									if (!kingInCheck(tmpBoard, WHITE))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
								}
							}

							//1 up, 2 left
							y = i + 1;
							x = j - 2;
							t = getPieceAt(board, y, x);
							if (y < 8 && x >= 0)
							{
								if (t == EMPTY || isBlack(t))
								{
									cloneBoard(board, tmpBoard);
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, i, j, EMPTY);
									if (!kingInCheck(tmpBoard, WHITE))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
								}
							}

							//2 down, 1 right
							y = i - 2;
							x = j + 1;
							t = getPieceAt(board, y, x);
							if (y >= 0 && x < 8)
							{
								if (t == EMPTY || isBlack(t))
								{
									cloneBoard(board, tmpBoard);
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, i, j, EMPTY);
									if (!kingInCheck(tmpBoard, WHITE))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
								}
							}

							//2 down 1 left
							y = i - 2;
							x = j - 1;
							t = getPieceAt(board, y, x);
							if (y >= 0 && x >= 0)
							{
								if (t == EMPTY || isBlack(t))
								{
									cloneBoard(board, tmpBoard);
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, i, j, EMPTY);
									if (!kingInCheck(tmpBoard, WHITE))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
								}
							}

							//1 down, 2 right
							y = i - 1;
							x = j + 2;
							t = getPieceAt(board, y, x);
							if (y >= 0 && x < 8)
							{
								if (t == EMPTY || isBlack(t))
								{
									cloneBoard(board, tmpBoard);
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, i, j, EMPTY);
									if (!kingInCheck(tmpBoard, WHITE))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
								}
							}

							//1 down, 2 left
							y = i - 1;
							x = j - 2;
							t = getPieceAt(board, y, x);
							if (y >= 0 && x >= 0)
							{
								if (t == EMPTY || isBlack(t))
								{
									cloneBoard(board, tmpBoard);
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, i, j, EMPTY);
									if (!kingInCheck(tmpBoard, WHITE))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
								}
							}

							break;
						case BISHOP:
							//upper left
							cloneBoard(board, tmpBoard);
							for (x = j - 1, y = i + 1; y < 8 && x >= 0; x--, y++)
							{
								t = getPieceAt(board, y, x);
								if (t == EMPTY)
								{
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, y - 1, x + 1, EMPTY);
									if (!kingInCheck(tmpBoard, WHITE))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
								} else if (isWhite(t)) {
									break;	//collided with piece on same team
								} else {
									//this is an enemy piece, capture it
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, y - 1, x + 1, EMPTY);
									if (!kingInCheck(tmpBoard, WHITE))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
									break;
								}
							}

							//upper right
							cloneBoard(board, tmpBoard);
							for (x = j + 1, y = i + 1; y < 8 && x < 8; x++, y++)
							{
								t = getPieceAt(board, y, x);
								if (t == EMPTY)
								{
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, y - 1, x - 1, EMPTY);
									if (!kingInCheck(tmpBoard, WHITE))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
								} else if (isWhite(t)) {
									break;	//collided with piece on same team
								} else {
									//this is an enemy piece, capture it
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, y - 1, x - 1, EMPTY);
									if (!kingInCheck(tmpBoard, WHITE))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
									break;
								}
							}

							//lower left
							cloneBoard(board, tmpBoard);
							for (x = j - 1, y = i - 1; y >= 0 && x >= 0; x--, y--)
							{
								t = getPieceAt(board, y, x);
								if (t == EMPTY)
								{
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, y + 1, x + 1, EMPTY);
									if (!kingInCheck(tmpBoard, WHITE))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
								} else if (isWhite(t)) {
									break;	//collided with piece on same team
								} else {
									//this is an enemy piece, capture it
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, y + 1, x + 1, EMPTY);
									if (!kingInCheck(tmpBoard, WHITE))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
									break;
								}
							}

							//lower right
							cloneBoard(board, tmpBoard);
							for (x = j + 1, y = i - 1; y >= 0 && x < 8; x++, y--)
							{
								t = getPieceAt(board, y, x);
								if (t == EMPTY)
								{
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, y + 1, x - 1, EMPTY);
									if (!kingInCheck(tmpBoard, WHITE))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
								} else if (isWhite(t)) {
									break;	//collided with piece on same team
								} else {
									//this is an enemy piece, capture it
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, y + 1, x - 1, EMPTY);
									if (!kingInCheck(tmpBoard, WHITE))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
									break;
								}
							}

							break;
						case QUEEN:
							//try moving up
							cloneBoard(board, tmpBoard);
							for (y = i + 1; y < 8; y++)
							{
								t = getPieceAt(board, y, j);
								if (t == EMPTY)
								{
									setPieceAt(tmpBoard, y, j, p);
									setPieceAt(tmpBoard, y - 1, j, EMPTY);
									if (!kingInCheck(tmpBoard, WHITE))
										addMove(moves, p, i, j, y, j, evaluateBoard(tmpBoard, evaluator));
								} else if (!isBlack(t)) {
									break;	//collided with piece on same team
								} else {
									//this is an enemy piece, capture it
									setPieceAt(tmpBoard, y, j, p);
									setPieceAt(tmpBoard, y - 1, j, EMPTY);
									if (!kingInCheck(tmpBoard, WHITE))
										addMove(moves, p, i, j, y, j, evaluateBoard(tmpBoard, evaluator));
									break;
								}
							}

							//try moving down
							cloneBoard(board, tmpBoard);
							for (y = i - 1; y >= 0; y--)
							{
								t = getPieceAt(board, y, j);
								if (t == EMPTY)
								{
									setPieceAt(tmpBoard, y, j, p);
									setPieceAt(tmpBoard, y + 1, j, EMPTY);
									if (!kingInCheck(tmpBoard, WHITE))
										addMove(moves, p, i, j, y, j, evaluateBoard(tmpBoard, evaluator));
								} else if (!isBlack(t)) {
									break;	//collided with piece on same team
								} else {
									//this is an enemy piece, capture it
									setPieceAt(tmpBoard, y, j, p);
									setPieceAt(tmpBoard, y + 1, j, EMPTY);
									if (!kingInCheck(tmpBoard, WHITE))
										addMove(moves, p, i, j, y, j, evaluateBoard(tmpBoard, evaluator));
									break;
								}
							}

							//try moving left
							cloneBoard(board, tmpBoard);
							for (x = j - 1; x >= 0; x--)
							{
								t = getPieceAt(board, i, x);
								if (t == EMPTY)
								{
									setPieceAt(tmpBoard, i, x, p);
									setPieceAt(tmpBoard, i, x + 1, EMPTY);
									if (!kingInCheck(tmpBoard, WHITE))
										addMove(moves, p, i, j, i, x, evaluateBoard(tmpBoard, evaluator));
								} else if (!isBlack(t)) {
									break;	//collided with piece on same team
								} else {
									//this is an enemy piece, capture it
									setPieceAt(tmpBoard, i, x, p);
									setPieceAt(tmpBoard, i, x + 1, EMPTY);
									if (!kingInCheck(tmpBoard, WHITE))
										addMove(moves, p, i, j, i, x, evaluateBoard(tmpBoard, evaluator));
									break;
								}
							}

							//try moving right
							cloneBoard(board, tmpBoard);
							for (x = j + 1; x < 8; x++)
							{
								t = getPieceAt(board, i, x);
								if (t == EMPTY)
								{
									setPieceAt(tmpBoard, i, x, p);
									setPieceAt(tmpBoard, i, x - 1, EMPTY);
									if (!kingInCheck(tmpBoard, WHITE))
										addMove(moves, p, i, j, i, x, evaluateBoard(tmpBoard, evaluator));
								} else if (!isBlack(t)) {
									break;	//collided with piece on same team
								} else {
									//this is an enemy piece, capture it
									setPieceAt(tmpBoard, i, x, p);
									setPieceAt(tmpBoard, i, x - 1, EMPTY);
									if (!kingInCheck(tmpBoard, WHITE))
										addMove(moves, p, i, j, i, x, evaluateBoard(tmpBoard, evaluator));
									break;
								}
							}

							//upper left
							cloneBoard(board, tmpBoard);
							for (x = j - 1, y = i + 1; y < 8 && x >= 0; x--, y++)
							{
								t = getPieceAt(board, y, x);
								if (t == EMPTY)
								{
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, y - 1, x + 1, EMPTY);
									if (!kingInCheck(tmpBoard, WHITE))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
								} else if (isWhite(t)) {
									break;	//collided with piece on same team
								} else {
									//this is an enemy piece, capture it
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, y - 1, x + 1, EMPTY);
									if (!kingInCheck(tmpBoard, WHITE))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
									break;
								}
							}

							//upper right
							cloneBoard(board, tmpBoard);
							for (x = j + 1, y = i + 1; y < 8 && x < 8; x++, y++)
							{
								t = getPieceAt(board, y, x);
								if (t == EMPTY)
								{
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, y - 1, x - 1, EMPTY);
									if (!kingInCheck(tmpBoard, WHITE))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
								} else if (isWhite(t)) {
									break;	//collided with piece on same team
								} else {
									//this is an enemy piece, capture it
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, y - 1, x - 1, EMPTY);
									if (!kingInCheck(tmpBoard, WHITE))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
									break;
								}
							}

							//lower left
							cloneBoard(board, tmpBoard);
							for (x = j - 1, y = i - 1; y >= 0 && x >= 0; x--, y--)
							{
								t = getPieceAt(board, y, x);
								if (t == EMPTY)
								{
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, y + 1, x + 1, EMPTY);
									if (!kingInCheck(tmpBoard, WHITE))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
								} else if (isWhite(t)) {
									break;	//collided with piece on same team
								} else {
									//this is an enemy piece, capture it
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, y + 1, x + 1, EMPTY);
									if (!kingInCheck(tmpBoard, WHITE))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
									break;
								}
							}

							//lower right
							cloneBoard(board, tmpBoard);
							for (x = j + 1, y = i - 1; y >= 0 && x < 8; x++, y--)
							{
								t = getPieceAt(board, y, x);
								if (t == EMPTY)
								{
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, y + 1, x - 1, EMPTY);
									if (!kingInCheck(tmpBoard, WHITE))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
								} else if (isWhite(t)) {
									break;	//collided with piece on same team
								} else {
									//this is an enemy piece, capture it
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, y + 1, x - 1, EMPTY);
									if (!kingInCheck(tmpBoard, WHITE))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
									break;
								}
							}

							break;
						case KING:
							//up
							y = i + 1;
							x = j;
							cloneBoard(board, tmpBoard);
							if (i < 7)
							{
								t = getPieceAt(board, y, x);
								if (t == EMPTY)
								{
									//empty space
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, i, j, EMPTY);
									if (pieceSafe(tmpBoard, y, x))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
								} else if (isBlack(t)) {
									//this is an enemy piece, capture it
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, i, j, EMPTY);
									if (pieceSafe(tmpBoard, y, x))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
								}
							}

							//down
							y = i - 1;
							x = j;
							cloneBoard(board, tmpBoard);
							if (i > 0)
							{
								t = getPieceAt(board, y, x);
								if (t == EMPTY)
								{
									//empty space
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, i, j, EMPTY);
									if (pieceSafe(tmpBoard, y, x))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
								} else if (isBlack(t)) {
									//this is an enemy piece, capture it
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, i, j, EMPTY);
									if (pieceSafe(tmpBoard, y, x))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
								}
							}

							//left
							y = i;
							x = j - 1;
							cloneBoard(board, tmpBoard);
							if (j > 0)
							{
								t = getPieceAt(board, y, x);
								if (t == EMPTY)
								{
									//empty space
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, i, j, EMPTY);
									if (pieceSafe(tmpBoard, y, x))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
								} else if (isBlack(t)) {
									//this is an enemy piece, capture it
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, i, j, EMPTY);
									if (pieceSafe(tmpBoard, y, x))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
								}
							}

							//right
							y = i;
							x = j + 1;
							cloneBoard(board, tmpBoard);
							if (j < 7)
							{
								t = getPieceAt(board, y, x);
								if (t == EMPTY)
								{
									//empty space
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, i, j, EMPTY);
									if (pieceSafe(tmpBoard, y, x))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
								} else if (isBlack(t)) {
									//this is an enemy piece, capture it
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, i, j, EMPTY);
									if (pieceSafe(tmpBoard, y, x))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
								}
							}

							//upper left
							y = i + 1;
							x = j - 1;
							cloneBoard(board, tmpBoard);
							if (i < 7 && j > 0)
							{
								t = getPieceAt(board, y, x);
								if (t == EMPTY)
								{
									//empty space
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, i, j, EMPTY);
									if (pieceSafe(tmpBoard, y, x))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
								} else if (isBlack(t)) {
									//this is an enemy piece, capture it
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, i, j, EMPTY);
									if (pieceSafe(tmpBoard, y, x))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
								}
							}

							//upper right
							y = i + 1;
							x = j + 1;
							cloneBoard(board, tmpBoard);
							if (i < 7 && j < 7)
							{
								t = getPieceAt(board, y, x);
								if (t == EMPTY)
								{
									//empty space
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, i, j, EMPTY);
									if (pieceSafe(tmpBoard, y, x))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
								} else if (isBlack(t)) {
									//this is an enemy piece, capture it
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, i, j, EMPTY);
									if (pieceSafe(tmpBoard, y, x))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
								}
							}

							//lower left
							y = i - 1;
							x = j - 1;
							cloneBoard(board, tmpBoard);
							if (i > 0 && j > 0)
							{
								t = getPieceAt(board, y, x);
								if (t == EMPTY)
								{
									//empty space
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, i, j, EMPTY);
									if (pieceSafe(tmpBoard, y, x))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
								} else if (isBlack(t)) {
									//this is an enemy piece, capture it
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, i, j, EMPTY);
									if (pieceSafe(tmpBoard, y, x))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
								}
							}

							//lower right
							y = i - 1;
							x = j + 1;
							cloneBoard(board, tmpBoard);
							if (i > 0 && j < 7)
							{
								t = getPieceAt(board, y, x);
								if (t == EMPTY)
								{
									//empty space
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, i, j, EMPTY);
									if (pieceSafe(tmpBoard, y, x))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
								} else if (isBlack(t)) {
									//this is an enemy piece, capture it
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, i, j, EMPTY);
									if (pieceSafe(tmpBoard, y, x))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
								}
							}
							
							//king side castle
							if (whiteKCastleInvalid == FALSE)
							{
								if (spaceSafe(board, WHITE, 0, 5) == TRUE && spaceSafe(board, WHITE, 0, 6) == TRUE &&
									getPieceAt(tmpBoard, 0, 4) == KING && getPieceAt(tmpBoard, 0, 7) == ROOK)
								{
									setPieceAt(tmpBoard, 0, 4, EMPTY);
									setPieceAt(tmpBoard, 0, 7, EMPTY);
									setPieceAt(tmpBoard, 0, 6, KING);
									setPieceAt(tmpBoard, 0, 5, ROOK);
									addMove(moves, p, CASTLE, CASTLE, KINGSIDE, KINGSIDE, evaluateBoard(tmpBoard, evaluator));
								}
							}
							
							//queen side castle
							if (whiteQCastleInvalid == FALSE)
							{
								if (spaceSafe(board, WHITE, 0, 1) == TRUE && spaceSafe(board, WHITE, 0, 2) == TRUE &&
										spaceSafe(board, WHITE, 0, 3) && getPieceAt(tmpBoard, 0, 0) == ROOK &&
										getPieceAt(tmpBoard, 0, 4) == KING)
								{
									setPieceAt(tmpBoard, 0, 0, EMPTY);
									setPieceAt(tmpBoard, 0, 4, EMPTY);
									setPieceAt(tmpBoard, 0, 2, KING);
									setPieceAt(tmpBoard, 0, 3, ROOK);
									addMove(moves, p, CASTLE, CASTLE, QUEENSIDE, QUEENSIDE, evaluateBoard(tmpBoard, evaluator));
								}
							}
							break;
					}
				}
			} else {
				if (isBlack(p))
				{
					//this is a black piece
					switch (p)
					{
						case PAWN + BLACK:
							//try moving forward
							cloneBoard(board, tmpBoard);
							if (i - 1 >= 0 && getPieceAt(tmpBoard, i - 1, j) == EMPTY)
							{
								setPieceAt(tmpBoard, i - 1, j, p);
								setPieceAt(tmpBoard, i, j, EMPTY);
								if (!kingInCheck(tmpBoard, BLACK))
									addMove(moves, p, i, j, i - 1, j, evaluateBoard(tmpBoard, evaluator));
							}

							//try moving diagonally
							cloneBoard(board, tmpBoard);
							if (i - 1 >= 0 && j + 1 < 8 && isWhite(getPieceAt(board, i - 1, j + 1)))
							{
								setPieceAt(tmpBoard, i - 1, j + 1, p);
								setPieceAt(tmpBoard, i, j, EMPTY);
								if (!kingInCheck(tmpBoard, BLACK))
									addMove(moves, p, i, j, i - 1, j + 1, evaluateBoard(tmpBoard, evaluator));
							}

							cloneBoard(board, tmpBoard);
							if (i - 1 >= 0 && j - 1 >= 0 && isWhite(getPieceAt(board, i - 1, j - 1)))
							{
								setPieceAt(tmpBoard, i - 1, j - 1, p);
								setPieceAt(tmpBoard, i, j, EMPTY);
								if (!kingInCheck(tmpBoard, BLACK))
									addMove(moves, p, i, j, i - 1, j - 1, evaluateBoard(tmpBoard, evaluator));
							}

							//try moving 2 spaces forward
							cloneBoard(board, tmpBoard);
							if (i == 6 && getPieceAt(board, 5, j) == EMPTY &&
								getPieceAt(board, 4, j) == EMPTY)
							{
								setPieceAt(tmpBoard, 4, j, p);
								setPieceAt(tmpBoard, 6, j, EMPTY);
								if (!kingInCheck(tmpBoard, BLACK))
									addMove(moves, p, 6, j, 4, j, evaluateBoard(tmpBoard, evaluator));
							}
							
							//check for en passant
							if (i == 3)	//make sure pawn is on its fifth rank
							{
								if (prevMove->endRow == 3 && prevMove->startRow == 1 &&
									prevMove->endCol == j - 1 && prevMove->pieceId == PAWN)
								{
									cloneBoard(board, tmpBoard);
									setPieceAt(tmpBoard, 2, j - 1, p);		//move current pawn
									setPieceAt(tmpBoard, 3, j, EMPTY);		//empty current space
									setPieceAt(tmpBoard, 3, j - 1, EMPTY);	//capture enemy pawn
									if (!kingInCheck(tmpBoard, BLACK))
										addMove(moves, p, 3, j, 2, j - 1, evaluateBoard(tmpBoard, evaluator));
								} else if (prevMove->endRow == 3 && prevMove->startRow == 1 && 
										   prevMove->endCol == j + 1 && prevMove->pieceId == PAWN) {
									cloneBoard(board, tmpBoard);
									setPieceAt(tmpBoard, 2, j + 1, p);		//move current pawn
									setPieceAt(tmpBoard, 3, j, EMPTY);		//empty current space
									setPieceAt(tmpBoard, 3, j + 1, EMPTY);	//capture enemy pawn
									if (!kingInCheck(tmpBoard, BLACK))
										addMove(moves, p, 3, j, 2, j + 1, evaluateBoard(tmpBoard, evaluator));
								}
							}
							break;
						case ROOK + BLACK:
							//try moving up
							cloneBoard(board, tmpBoard);
							for (y = i + 1; y < 8; y++)
							{
								t = getPieceAt(board, y, j);
								if (t == EMPTY)
								{
									setPieceAt(tmpBoard, y, j, p);
									setPieceAt(tmpBoard, y - 1, j, EMPTY);
									if (!kingInCheck(tmpBoard, BLACK))
										addMove(moves, p, i, j, y, j, evaluateBoard(tmpBoard, evaluator));
								} else if (isBlack(t)) {
									break;	//collided with piece on same team
								} else {
									//this is an enemy piece, capture it
									setPieceAt(tmpBoard, y, j, p);
									setPieceAt(tmpBoard, y - 1, j, EMPTY);
									if (!kingInCheck(tmpBoard, BLACK))
										addMove(moves, p, i, j, y, j, evaluateBoard(tmpBoard, evaluator));
									break;
								}
							}

							//try moving down
							cloneBoard(board, tmpBoard);
							for (y = i - 1; y >= 0; y--)
							{
								t = getPieceAt(board, y, j);
								if (t == EMPTY)
								{
									setPieceAt(tmpBoard, y, j, p);
									setPieceAt(tmpBoard, y + 1, j, EMPTY);
									if (!kingInCheck(tmpBoard, BLACK))
										addMove(moves, p, i, j, y, j, evaluateBoard(tmpBoard, evaluator));
								} else if (isBlack(t)) {
									break;	//collided with piece on same team
								} else {
									//this is an enemy piece, capture it
									setPieceAt(tmpBoard, y, j, p);
									setPieceAt(tmpBoard, y + 1, j, EMPTY);
									if (!kingInCheck(tmpBoard, BLACK))
										addMove(moves, p, i, j, y, j, evaluateBoard(tmpBoard, evaluator));
									break;
								}
							}

							//try moving left
							cloneBoard(board, tmpBoard);
							for (x = j - 1; x >= 0; x--)
							{
								t = getPieceAt(board, i, x);
								if (t == EMPTY)
								{
									setPieceAt(tmpBoard, i, x, p);
									setPieceAt(tmpBoard, i, x + 1, EMPTY);
									if (!kingInCheck(tmpBoard, BLACK))
										addMove(moves, p, i, j, i, x, evaluateBoard(tmpBoard, evaluator));
								} else if (isBlack(t)) {
									break;	//collided with piece on same team
								} else {
									//this is an enemy piece, capture it
									setPieceAt(tmpBoard, i, x, p);
									setPieceAt(tmpBoard, i, x + 1, EMPTY);
									if (!kingInCheck(tmpBoard, BLACK))
										addMove(moves, p, i, j, i, x, evaluateBoard(tmpBoard, evaluator));
									break;
								}
							}

							//try moving right
							cloneBoard(board, tmpBoard);
							for (x = j + 1; x < 8; x++)
							{
								t = getPieceAt(board, i, x);
								if (t == EMPTY)
								{
									setPieceAt(tmpBoard, i, x, p);
									setPieceAt(tmpBoard, i, x - 1, EMPTY);
									if (!kingInCheck(tmpBoard, BLACK))
										addMove(moves, p, i, j, i, x, evaluateBoard(tmpBoard, evaluator));
								} else if (isBlack(t)) {
									break;	//collided with piece on same team
								} else {
									//this is an enemy piece, capture it
									setPieceAt(tmpBoard, i, x, p);
									setPieceAt(tmpBoard, i, x - 1, EMPTY);
									if (!kingInCheck(tmpBoard, BLACK))
										addMove(moves, p, i, j, i, x, evaluateBoard(tmpBoard, evaluator));
									break;
								}
							}

							break;
						case KNIGHT + BLACK:
							//2 up, 1 right
							y = i + 2;
							x = j + 1;
							t = getPieceAt(board, y, x);
							if (y < 8 && x < 8)
							{
								if (t == EMPTY || isWhite(t))
								{
									cloneBoard(board, tmpBoard);
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, i, j, EMPTY);
									if (!kingInCheck(tmpBoard, BLACK))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
								}
							}

							//2 up, 1 left
							y = i + 2;
							x = j - 1;
							t = getPieceAt(board, y, x);
							if (y < 8 && x >= 0)
							{
								if (t == EMPTY || isWhite(t))
								{
									cloneBoard(board, tmpBoard);
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, i, j, EMPTY);
									if (!kingInCheck(tmpBoard, BLACK))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
								}
							}

							//1 up, 2 right
							y = i + 1;
							x = j + 2;
							t = getPieceAt(board, y, x);
							if (y < 8 && x < 8)
							{
								if (t == EMPTY || isWhite(t))
								{
									cloneBoard(board, tmpBoard);
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, i, j, EMPTY);
									if (!kingInCheck(tmpBoard, BLACK))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
								}
							}

							//1 up, 2 left
							y = i + 1;
							x = j - 2;
							t = getPieceAt(board, y, x);
							if (y < 8 && x >= 0)
							{
								if (t == EMPTY || isWhite(t))
								{
									cloneBoard(board, tmpBoard);
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, i, j, EMPTY);
									if (!kingInCheck(tmpBoard, BLACK))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
								}
							}

							//2 down, 1 right
							y = i - 2;
							x = j + 1;
							t = getPieceAt(board, y, x);
							if (y >= 0 && x < 8)
							{
								if (t == EMPTY || isWhite(t))
								{
									cloneBoard(board, tmpBoard);
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, i, j, EMPTY);
									if (!kingInCheck(tmpBoard, BLACK))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
								}
							}

							//2 down 1 left
							y = i - 2;
							x = j - 1;
							t = getPieceAt(board, y, x);
							if (y >= 0 && x >= 0)
							{
								if (t == EMPTY || isWhite(t))
								{
									cloneBoard(board, tmpBoard);
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, i, j, EMPTY);
									if (!kingInCheck(tmpBoard, BLACK))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
								}
							}

							//1 down, 2 right
							y = i - 1;
							x = j + 2;
							t = getPieceAt(board, y, x);
							if (y >= 0 && x < 8)
							{
								if (t == EMPTY || isWhite(t))
								{
									cloneBoard(board, tmpBoard);
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, i, j, EMPTY);
									if (!kingInCheck(tmpBoard, BLACK))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
								}
							}

							//1 down, 2 left
							y = i - 1;
							x = j - 2;
							t = getPieceAt(board, y, x);
							if (y >= 0 && x >= 0)
							{
								if (t == EMPTY || isWhite(t))
								{
									cloneBoard(board, tmpBoard);
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, i, j, EMPTY);
									if (!kingInCheck(tmpBoard, BLACK))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
								}
							}
							break;
						case BISHOP + BLACK:
							//upper left
							cloneBoard(board, tmpBoard);
							for (x = j - 1, y = i + 1; y < 8 && x >= 0; x--, y++)
							{
								t = getPieceAt(board, y, x);
								if (t == EMPTY)
								{
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, y - 1, x + 1, EMPTY);
									if (!kingInCheck(tmpBoard, BLACK))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
								} else if (isBlack(t)) {
									break;	//collided with piece on same team
								} else {
									//this is an enemy piece, capture it
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, y - 1, x + 1, EMPTY);
									if (!kingInCheck(tmpBoard, BLACK))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
									break;
								}
							}

							//upper right
							cloneBoard(board, tmpBoard);
							for (x = j + 1, y = i + 1; y < 8 && x < 8; x++, y++)
							{
								t = getPieceAt(board, y, x);
								if (t == EMPTY)
								{
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, y - 1, x - 1, EMPTY);
									if (!kingInCheck(tmpBoard, BLACK))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
								} else if (isBlack(t)) {
									break;	//collided with piece on same team
								} else {
									//this is an enemy piece, capture it
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, y - 1, x - 1, EMPTY);
									if (!kingInCheck(tmpBoard, BLACK))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
									break;
								}
							}

							//lower left
							cloneBoard(board, tmpBoard);
							for (x = j - 1, y = i - 1; y >= 0 && x >= 0; x--, y--)
							{
								t = getPieceAt(board, y, x);
								if (t == EMPTY)
								{
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, y + 1, x + 1, EMPTY);
									if (!kingInCheck(tmpBoard, BLACK))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
								} else if (isBlack(t)) {
									break;	//collided with piece on same team
								} else {
									//this is an enemy piece, capture it
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, y + 1, x + 1, EMPTY);
									if (!kingInCheck(tmpBoard, BLACK))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
									break;
								}
							}

							//lower right
							cloneBoard(board, tmpBoard);
							for (x = j + 1, y = i - 1; y >= 0 && x < 8; x++, y--)
							{
								t = getPieceAt(board, y, x);
								if (t == EMPTY)
								{
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, y + 1, x - 1, EMPTY);
									if (!kingInCheck(tmpBoard, BLACK))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
								} else if (isBlack(t)) {
									break;	//collided with piece on same team
								} else {
									//this is an enemy piece, capture it
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, y + 1, x - 1, EMPTY);
									if (!kingInCheck(tmpBoard, BLACK))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
									break;
								}
							}

							break;
						case QUEEN + BLACK:
							//try moving up
							cloneBoard(board, tmpBoard);
							for (y = i + 1; y < 8; y++)
							{
								t = getPieceAt(board, y, j);
								if (t == EMPTY)
								{
									setPieceAt(tmpBoard, y, j, p);
									setPieceAt(tmpBoard, y - 1, j, EMPTY);
									if (!kingInCheck(tmpBoard, BLACK))
										addMove(moves, p, i, j, y, j, evaluateBoard(tmpBoard, evaluator));
								} else if (isBlack(t)) {
									break;	//collided with piece on same team
								} else {
									//this is an enemy piece, capture it
									setPieceAt(tmpBoard, y, j, p);
									setPieceAt(tmpBoard, y - 1, j, EMPTY);
									if (!kingInCheck(tmpBoard, BLACK))
										addMove(moves, p, i, j, y, j, evaluateBoard(tmpBoard, evaluator));
									break;
								}
							}

							//try moving down
							cloneBoard(board, tmpBoard);
							for (y = i - 1; y >= 0; y--)
							{
								t = getPieceAt(board, y, j);
								if (t == EMPTY)
								{
									setPieceAt(tmpBoard, y, j, p);
									setPieceAt(tmpBoard, y + 1, j, EMPTY);
									if (!kingInCheck(tmpBoard, BLACK))
										addMove(moves, p, i, j, y, j, evaluateBoard(tmpBoard, evaluator));
								} else if (isBlack(t)) {
									break;	//collided with piece on same team
								} else {
									//this is an enemy piece, capture it
									setPieceAt(tmpBoard, y, j, p);
									setPieceAt(tmpBoard, y + 1, j, EMPTY);
									if (!kingInCheck(tmpBoard, BLACK))
										addMove(moves, p, i, j, y, j, evaluateBoard(tmpBoard, evaluator));
									break;
								}
							}

							//try moving left
							cloneBoard(board, tmpBoard);
							for (x = j - 1; x >= 0; x--)
							{
								t = getPieceAt(board, i, x);
								if (t == EMPTY)
								{
									setPieceAt(tmpBoard, i, x, p);
									setPieceAt(tmpBoard, i, x + 1, EMPTY);
									if (!kingInCheck(tmpBoard, BLACK))
										addMove(moves, p, i, j, i, x, evaluateBoard(tmpBoard, evaluator));
								} else if (isBlack(t)) {
									break;	//collided with piece on same team
								} else {
									//this is an enemy piece, capture it
									setPieceAt(tmpBoard, i, x, p);
									setPieceAt(tmpBoard, i, x + 1, EMPTY);
									if (!kingInCheck(tmpBoard, BLACK))
										addMove(moves, p, i, j, i, x, evaluateBoard(tmpBoard, evaluator));
									break;
								}
							}

							//try moving right
							cloneBoard(board, tmpBoard);
							for (x = j + 1; x < 8; x++)
							{
								t = getPieceAt(board, i, x);
								if (t == EMPTY)
								{
									setPieceAt(tmpBoard, i, x, p);
									setPieceAt(tmpBoard, i, x - 1, EMPTY);
									if (!kingInCheck(tmpBoard, BLACK))
										addMove(moves, p, i, j, i, x, evaluateBoard(tmpBoard, evaluator));
								} else if (isBlack(t)) {
									break;	//collided with piece on same team
								} else {
									//this is an enemy piece, capture it
									setPieceAt(tmpBoard, i, x, p);
									setPieceAt(tmpBoard, i, x - 1, EMPTY);
									if (!kingInCheck(tmpBoard, BLACK))
										addMove(moves, p, i, j, i, x, evaluateBoard(tmpBoard, evaluator));
									break;
								}
							}

							//upper left
							cloneBoard(board, tmpBoard);
							for (x = j - 1, y = i + 1; y < 8 && x >= 0; x--, y++)
							{
								t = getPieceAt(board, y, x);
								if (t == EMPTY)
								{
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, y - 1, x + 1, EMPTY);
									if (!kingInCheck(tmpBoard, BLACK))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
								} else if (isBlack(t)) {
									break;	//collided with piece on same team
								} else {
									//this is an enemy piece, capture it
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, y - 1, x + 1, EMPTY);
									if (!kingInCheck(tmpBoard, BLACK))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
									break;
								}
							}

							//upper right
							cloneBoard(board, tmpBoard);
							for (x = j + 1, y = i + 1; y < 8 && x < 8; x++, y++)
							{
								t = getPieceAt(board, y, x);
								if (t == EMPTY)
								{
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, y - 1, x - 1, EMPTY);
									if (!kingInCheck(tmpBoard, BLACK))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
								} else if (isBlack(t)) {
									break;	//collided with piece on same team
								} else {
									//this is an enemy piece, capture it
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, y - 1, x - 1, EMPTY);
									if (!kingInCheck(tmpBoard, BLACK))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
									break;
								}
							}

							//lower left
							cloneBoard(board, tmpBoard);
							for (x = j - 1, y = i - 1; y >= 0 && x >= 0; x--, y--)
							{
								t = getPieceAt(board, y, x);
								if (t == EMPTY)
								{
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, y + 1, x + 1, EMPTY);
									if (!kingInCheck(tmpBoard, BLACK))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
								} else if (isBlack(t)) {
									break;	//collided with piece on same team
								} else {
									//this is an enemy piece, capture it
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, y + 1, x + 1, EMPTY);
									if (!kingInCheck(tmpBoard, BLACK))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
									break;
								}
							}

							//lower right
							cloneBoard(board, tmpBoard);
							for (x = j + 1, y = i - 1; y >= 0 && x < 8; x++, y--)
							{
								t = getPieceAt(board, y, x);
								if (t == EMPTY)
								{
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, y + 1, x - 1, EMPTY);
									if (!kingInCheck(tmpBoard, BLACK))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
								} else if (isBlack(t)) {
									break;	//collided with piece on same team
								} else {
									//this is an enemy piece, capture it
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, y + 1, x - 1, EMPTY);
									if (!kingInCheck(tmpBoard, BLACK))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
									break;
								}
							}

							break;
						case KING + BLACK:
							//up
							y = i + 1;
							x = j;
							cloneBoard(board, tmpBoard);
							if (i < 7)
							{
								t = getPieceAt(board, y, x);
								if (t == EMPTY)
								{
									//empty space
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, i, j, EMPTY);
									if (pieceSafe(tmpBoard, y, x))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
								} else if (isWhite(t)) {
									//this is an enemy piece, capture it
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, i, j, EMPTY);
									if (pieceSafe(tmpBoard, y, x))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
								}
							}

							//down
							y = i - 1;
							x = j;
							cloneBoard(board, tmpBoard);
							if (i > 0)
							{
								t = getPieceAt(board, y, x);
								if (t == EMPTY)
								{
									//empty space
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, i, j, EMPTY);
									if (pieceSafe(tmpBoard, y, x))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
								} else if (isWhite(t)) {
									//this is an enemy piece, capture it
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, i, j, EMPTY);
									if (pieceSafe(tmpBoard, y, x))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
								}
							}

							//left
							y = i;
							x = j - 1;
							cloneBoard(board, tmpBoard);
							if (j > 0)
							{
								t = getPieceAt(board, y, x);
								if (t == EMPTY)
								{
									//empty space
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, i, j, EMPTY);
									if (pieceSafe(tmpBoard, y, x))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
								} else if (isWhite(t)) {
									//this is an enemy piece, capture it
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, i, j, EMPTY);
									if (pieceSafe(tmpBoard, y, x))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
								}
							}

							//right
							y = i;
							x = j + 1;
							cloneBoard(board, tmpBoard);
							if (j < 7)
							{
								t = getPieceAt(board, y, x);
								if (t == EMPTY)
								{
									//empty space
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, i, j, EMPTY);
									if (pieceSafe(tmpBoard, y, x))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
								} else if (isWhite(t)) {
									//this is an enemy piece, capture it
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, i, j, EMPTY);
									if (pieceSafe(tmpBoard, y, x))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
								}
							}

							//upper left
							y = i + 1;
							x = j - 1;
							cloneBoard(board, tmpBoard);
							if (i < 7 && j > 0)
							{
								t = getPieceAt(board, y, x);
								if (t == EMPTY)
								{
									//empty space
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, i, j, EMPTY);
									if (pieceSafe(tmpBoard, y, x))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
								} else if (isWhite(t)) {
									//this is an enemy piece, capture it
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, i, j, EMPTY);
									if (pieceSafe(tmpBoard, y, x))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
								}
							}

							//upper right
							y = i + 1;
							x = j + 1;
							cloneBoard(board, tmpBoard);
							if (i < 7 && j < 7)
							{
								t = getPieceAt(board, y, x);
								if (t == EMPTY)
								{
									//empty space
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, i, j, EMPTY);
									if (pieceSafe(tmpBoard, y, x))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
								} else if (isWhite(t)) {
									//this is an enemy piece, capture it
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, i, j, EMPTY);
									if (pieceSafe(tmpBoard, y, x))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
								}
							}

							//lower left
							y = i - 1;
							x = j - 1;
							cloneBoard(board, tmpBoard);
							if (i > 0 && j > 0)
							{
								t = getPieceAt(board, y, x);
								if (t == EMPTY)
								{
									//empty space
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, i, j, EMPTY);
									if (pieceSafe(tmpBoard, y, x))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
								} else if (isWhite(t)) {
									//this is an enemy piece, capture it
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, i, j, EMPTY);
									if (pieceSafe(tmpBoard, y, x))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
								}
							}

							//lower right
							y = i - 1;
							x = j + 1;
							cloneBoard(board, tmpBoard);
							if (i > 0 && j < 7)
							{
								t = getPieceAt(board, y, x);
								if (t == EMPTY)
								{
									//empty space
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, i, j, EMPTY);
									if (pieceSafe(tmpBoard, y, x))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
								} else if (isWhite(t)) {
									//this is an enemy piece, capture it
									setPieceAt(tmpBoard, y, x, p);
									setPieceAt(tmpBoard, i, j, EMPTY);
									if (pieceSafe(tmpBoard, y, x))
										addMove(moves, p, i, j, y, x, evaluateBoard(tmpBoard, evaluator));
								}
							}
							
							//king side castle
							if (blackKCastleInvalid == FALSE)
							{
								if (spaceSafe(board, BLACK, 7, 5) == TRUE && spaceSafe(board, BLACK, 7, 6) == TRUE &&
									getPieceAt(tmpBoard, 7, 4) == BLACK + KING && getPieceAt(tmpBoard, 7, 7) == BLACK + KING)
								{
									setPieceAt(tmpBoard, 7, 4, EMPTY);
									setPieceAt(tmpBoard, 7, 7, EMPTY);
									setPieceAt(tmpBoard, 7, 6, BLACK + KING);
									setPieceAt(tmpBoard, 7, 5, BLACK + ROOK);
									addMove(moves, p, CASTLE, CASTLE, KINGSIDE, KINGSIDE, evaluateBoard(tmpBoard, evaluator));
								}
							}
							
							//queen side castle
							if (blackQCastleInvalid == FALSE)
							{
								if (spaceSafe(board, BLACK, 7, 1) == TRUE && spaceSafe(board, BLACK, 7, 2) == TRUE &&
									spaceSafe(board, BLACK, 7, 3) && getPieceAt(tmpBoard, 7, 0) == BLACK + ROOK &&
									getPieceAt(tmpBoard, 7, 4) == BLACK + KING)
								{
									setPieceAt(tmpBoard, 7, 0, EMPTY);
									setPieceAt(tmpBoard, 7, 4, EMPTY);
									setPieceAt(tmpBoard, 7, 2, BLACK + KING);
									setPieceAt(tmpBoard, 7, 3, BLACK + ROOK);
									addMove(moves, p, CASTLE, CASTLE, QUEENSIDE, QUEENSIDE, evaluateBoard(tmpBoard, evaluator));
								}
							}
							break;
					}
				}
			}
		}
	}

	free(tmpBoard);
	return moves;
}

/*
 * Create a new MoveSet structure.
 */
MoveSet* newMoveSet()
{
	MoveSet* set = (MoveSet*) malloc(sizeof(MoveSet));
	set->cap = 100;
	set->size = 0;
	set->moves = (Move**) malloc(sizeof(Move*) * set->cap);
	return set;
}

/*
 * Add a Move to a MoveSet.
 */
void addMove(MoveSet* set, char pieceId, int startRow, int startCol, int endRow, int endCol, int boardScore)
{
	//Check for expansion
	if (set->size >= set->cap)
	{
		set->cap *= 2;
		int i;
		Move** oMoves = set->moves;
		set->moves = (Move**) malloc(sizeof(Move*) * set->cap);
		for (i = 0; i < set->size; i++)
			set->moves[i] = oMoves[i];
		free(oMoves);
	}

	//Add move
	Move* move = (Move*) malloc(sizeof(Move));
	move->pieceId = pieceId;
	move->startRow = (char) startRow;
	move->startCol = (char) startCol;
	move->endRow = (char) endRow;
	move->endCol = (char) endCol;
	move->boardScore = boardScore;
	set->moves[set->size++] = move;
}

/*
 * Get move from MoveSet.
 */
Move* getMove(MoveSet* set, int index)
{
	return set->moves[index];
}

/* Get board score.  Expressed as a difference between white and black.
 * Positive numbers show a better white score and negative numbers
 * indicate a better black score.
 */
int evaluateBoard(char* board, int functionId)
{
	int count;
	char i, j;
	count = 0;

	int pawnValue, rookValue, knightValue, bishopValue, queenValue;
	int pawnAdvanceValue, safetyRatio;
	switch (functionId)
	{
		case ROB:			//Rob's values
			pawnValue = 100;
			rookValue = 500;
			knightValue = 300;
			bishopValue = 350;
			queenValue = 900;
			pawnAdvanceValue = 4;
			safetyRatio = 30;
			break;
		case TURING:		//Turing's values
			pawnValue = 100;
			rookValue = 500;
			knightValue = 300;
			bishopValue = 350;
			queenValue = 1000;
			pawnAdvanceValue = 0;
			safetyRatio = 0;
			break;
		case BERLINER:
			pawnValue = 100;
			rookValue = 510;
			knightValue = 320;
			bishopValue = 333;
			queenValue = 880;
			pawnAdvanceValue = 6;
			safetyRatio = 0;
			break;
	}

	for (i = 0; i < 8; i++)
	{
		for (j = 0; j < 8; j++)
		{
			char p = getPieceAt(board, i, j);
			if (p != EMPTY)
			{
				if (!isBlack(p))
				{
					//white piece
					switch(p)
					{
						case PAWN:
							count += pawnValue;
							count += pawnAdvanceValue * i;
							if (safetyRatio > 0 && pieceSafe(board, i, j))
								count += pawnValue / safetyRatio;
							break;
						case ROOK:
							count += rookValue;
							if (safetyRatio > 0 && pieceSafe(board, i, j))
								count += rookValue / safetyRatio;
							break;
						case KNIGHT:
							count += knightValue;
							if (safetyRatio > 0 && pieceSafe(board, i, j))
								count += knightValue / safetyRatio;
							break;
						case BISHOP:
							count += bishopValue;
							if (safetyRatio > 0 && pieceSafe(board, i, j))
								count += bishopValue / safetyRatio;
							break;
						case QUEEN:
							count += queenValue;
							if (safetyRatio > 0 && pieceSafe(board, i, j))
								count += queenValue / safetyRatio;
							break;
					}
				} else {
					//black piece
					switch(p)
					{
						case BLACK + PAWN:
							count -= pawnAdvanceValue * (7 - i);
							count -= pawnValue;
							if (safetyRatio > 0 && pieceSafe(board, i, j))
								count -= pawnValue / safetyRatio;
							break;
						case BLACK + ROOK:
							count -= rookValue;
							if (safetyRatio > 0 && pieceSafe(board, i, j))
								count -= rookValue / safetyRatio;
							break;
						case BLACK + KNIGHT:
							count -= knightValue;
							if (safetyRatio > 0 && pieceSafe(board, i, j))
								count -= knightValue / safetyRatio;
							break;
						case BLACK + BISHOP:
							count -= bishopValue;
							if (safetyRatio > 0 && pieceSafe(board, i, j))
								count -= bishopValue / safetyRatio;
							break;
						case BLACK + QUEEN:
							count -= queenValue;
							if (safetyRatio > 0 && pieceSafe(board, i, j))
								count -= queenValue / safetyRatio;
							break;
					}
				}
			}
		}
	}

	return count;
}

/*
 * Execute a move on the input board.
 */
void executeMove(char* board, Move* move, Move* prevMove)
{
	char pieceId = getPieceAt(board, move->startRow, move->startCol);
	if (move->startRow == CASTLE && move->endRow == KINGSIDE)
	{
		//castle kingside
		if (isBlack(move->pieceId) == TRUE)
		{
			//black
			setPieceAt(board, 7, 4, EMPTY);
			setPieceAt(board, 7, 7, EMPTY);
			setPieceAt(board, 7, 6, BLACK + KING);
			setPieceAt(board, 7, 5, BLACK + ROOK);
			blackKCastleInvalid = TRUE;
			blackQCastleInvalid = TRUE;
		} else {
			//white
			setPieceAt(board, 0, 4, EMPTY);
			setPieceAt(board, 0, 7, EMPTY);
			setPieceAt(board, 0, 6, KING);
			setPieceAt(board, 0, 5, ROOK);
			whiteKCastleInvalid = TRUE;
			whiteQCastleInvalid = TRUE;
		}
	} else if (move->startRow == CASTLE && move->endRow == QUEENSIDE) {
		//castle queenside
		if (isBlack(move->pieceId) == TRUE)
		{
			//black
			setPieceAt(board, 7, 0, EMPTY);
			setPieceAt(board, 7, 4, EMPTY);
			setPieceAt(board, 7, 2, BLACK + KING);
			setPieceAt(board, 7, 3, BLACK + ROOK);
			blackQCastleInvalid = TRUE;
			blackKCastleInvalid = TRUE;
		} else {
			//white
			setPieceAt(board, 0, 0, EMPTY);
			setPieceAt(board, 0, 4, EMPTY);
			setPieceAt(board, 0, 2, KING);
			setPieceAt(board, 0, 3, ROOK);
			whiteQCastleInvalid = TRUE;
			whiteQCastleInvalid = TRUE;
		}
	} else {
		if (pieceId == KING)
		{
			whiteKCastleInvalid = TRUE;
			whiteQCastleInvalid = TRUE;
		} else if (pieceId == BLACK + KING) {
			blackKCastleInvalid = TRUE;
			blackQCastleInvalid = TRUE;
		} else if (pieceId == ROOK) {
			if (move->startRow == 0 && move->startCol == 0)
			{
				whiteQCastleInvalid = TRUE;
			} else if (move->startRow == 0 && move->startCol == 7) {
				whiteKCastleInvalid = TRUE;
			} else if (move->startRow == 7 && move->startCol == 0) {
				blackQCastleInvalid = TRUE;
			} else if (move->startRow == 7 && move->startCol == 7) {
				blackKCastleInvalid = TRUE;
			}
		} else if (pieceId == PAWN) {
			//check for white en passant case
			if (move->startRow == 4 && move->endRow == 5 && move->startCol != move->endCol)
			{
				if (prevMove->pieceId == PAWN + BLACK && prevMove->startCol == prevMove->endCol &&
					prevMove->endRow == 4 && (prevMove->endCol == move->startCol + 1 ||
											  prevMove->endCol == move->startCol - 1))
				{
					//capture pawn
					setPieceAt(board, 4, prevMove->endCol, EMPTY);
				}
			}
		} else if (pieceId == PAWN + BLACK) {
			//check for black en passant case
			if (move->startRow == 3 && move->endRow == 2 && move->startCol != move->endCol)
			{
				if (prevMove->pieceId == PAWN && prevMove->startCol == prevMove->endCol &&
					prevMove->endRow == 3 && (prevMove->endCol == move->startCol + 1 ||
											  prevMove->endCol == move->startCol - 1))
				{
					//capture pawn
					setPieceAt(board, 3, prevMove->endCol, EMPTY);
				}
			}
		}
		
		setPieceAt(board, move->startRow, move->startCol, EMPTY);
		setPieceAt(board, move->endRow, move->endCol, pieceId);
	}

	//convert pawn to queen if necessary
	if (move->endRow == 7 && pieceId == PAWN)
		setPieceAt(board, move->endRow, move->endCol, QUEEN);

	if (move->endRow == 0 && pieceId == BLACK + PAWN)
		setPieceAt(board, move->endRow, move->endCol, BLACK + QUEEN);
}

/*
 * Prints a textual representation of a move.
 */
void printMove(Move* move)
{
	if (move->startRow == CASTLE && move->endRow == KINGSIDE)
		if (isBlack(move->pieceId))
			printf("BLACK CASTLE KINGSIDE.\n");
		else
			printf("WHITE CASTLE KINGSIDE.\n");
	else if (move->startRow == CASTLE && move->endRow == QUEENSIDE)
		if (isBlack(move->pieceId))
			printf("BLACK CASTLE QUEENSIDE.\n");
		else
			printf("WHITE CASTLE QUEENSIDE.\n");
	else
		printf("%s at %s to %s.\n", toString(move->pieceId), positionString(move->startRow, move->startCol), positionString(move->endRow, move->endCol));
	fflush(stdout);
}

/*
 * Returns a textual representation of the move for logging.
 */
char* moveToString(Move* move)
{
	char* str = (char*) malloc(6);
	if (move->startRow == CASTLE && move->endRow == KINGSIDE)
			sprintf(str, "CK\n");
	else if (move->startRow == CASTLE && move->endRow == QUEENSIDE)
			sprintf(str, "CQ\n");
	else
		sprintf(str, "%c%d%c%d\n", getColumnChar(move->startCol), move->startRow + 1, getColumnChar(move->endCol), move->endRow + 1);
	
	return str;
}

/* Get the column character for an input int. */
char getColumnChar(int col)
{
	switch (col)
	{
		case 0:
			return 'A';
		case 1:
			return 'B';
		case 2:
			return 'C';
		case 3:
			return 'D';
		case 4:
			return 'E';
		case 5:
			return 'F';
		case 6:
			return 'G';
		case 7:
			return 'H';
		default:
			return '\0';
	}
}

/*
 * Prints a textual representation of a set of moves.
 */
void printMoves(MoveSet* moveSet)
{
	int i;
	for (i = 0; i < moveSet->size; i++)
	{
		printMove(moveSet->moves[i]);
	}
}

/*
 * Free MoveSet memory.
 */
void destroyMoveSet(MoveSet* set)
{
	int i;
	for (i = 0; i < set->size; i++)
		free(set->moves[i]);
	free(set->moves);
	free(set);
}

/*
 * Compute best move for a given board configuration and player. Depth specifies
 * "depth" of search.
 */
Move* bestMove(char* board, int color, Move* prevMove, int depth, int evaluator)
{
	MoveSet* legalMoves = getLegalMoves(board, color, prevMove, evaluator);
	
	if (legalMoves == NULL)
	{
		//No moves left
		return NULL;
	} else {
		//Divide up problem space
		int i, j;
		int proc = COMM_WORLD.Get_rank();
		int numProcs = COMM_WORLD.Get_size();
		int segSize, procCap;
		if (legalMoves->size >= numProcs - 1)
		{
			//There were more moves than processors
			segSize = legalMoves->size / (numProcs - 1);
			procCap = numProcs;
		} else {
			//There are more processors than legal moves
			segSize = 1;
			procCap = legalMoves->size + 1;
		}

		//Distribute board and move data to slave nodes
		MoveSet* sub;
		for (i = 1, j = 0; i < procCap - 1; i++, j++)
		{
			sub = subMoveSet(legalMoves, j * segSize, segSize);
			COMM_WORLD.Send(&color, 1, MPI_INT, i, 0);
			COMM_WORLD.Send(&depth, 1, MPI_INT, i, 0);
			COMM_WORLD.Send(&evaluator, 1, MPI_INT, i, 0);
			COMM_WORLD.Send(board, 64, MPI_CHAR, i, 0);
			sendMoveSet(i, sub);
			sendMove(i, prevMove);
			free(sub);
		}

		//Distribute last (potentially larger) segment
		sub = subMoveSet(legalMoves, j * segSize, segSize + legalMoves->size % (procCap - 1));
		COMM_WORLD.Send(&color, 1, MPI_INT, i, 0);
		COMM_WORLD.Send(&depth, 1, MPI_INT, i, 0);
		COMM_WORLD.Send(&evaluator, 1, MPI_INT, i, 0);
		COMM_WORLD.Send(board, 64, MPI_CHAR, i, 0);
		sendMoveSet(i, sub);
		sendMove(i, prevMove);
		free(sub);

		//Initialize "best" variable
		MoveDef* best = (MoveDef*) malloc(sizeof(MoveDef));
		best->index = 0;
		if (color == WHITE)
			best->value = -INFINITY;
		else
			best->value = INFINITY;

		//Retrieve other processed segments and determine best
		MoveDef* tmp;
		MoveDef* m;
		for (i = 1; i < procCap; i++)
		{
			m = receiveMoveDef(i);
			if (color == WHITE && m->value > best->value)
			{
				//New best found (white), update pointers
				tmp = best;
				best = m;
				
				//Make index relative to "legalMoves"
				best->index = (i - 1) * segSize + best->index;
			} else if (color == BLACK && m->value < best->value) {
				//New best found (black), update pointers
				tmp = best;
				best = m;
				
				//Make index relative to "legalMoves"
				best->index = (i - 1) * segSize + best->index;
			} else {
				tmp = m;
			}
			free(tmp);
		}
		
		//Return move
		Move* move = getMove(legalMoves, best->index);
		return move;
	}
}

/*
 * Recursive helper function for bestMove()
 */
MoveDef* bestMoveHelper(char* board, int color, int depth, MoveSet* legalMoves, Move* prevMove, int evaluator)
{
	//Save state of castle variables
	char tmpWKC = whiteKCastleInvalid;
	char tmpWQC = whiteQCastleInvalid;
	char tmpBKC = blackKCastleInvalid;
	char tmpBQC = blackQCastleInvalid;
	
	if (depth == 0)
	{
		//we have reached the end of the search (base case)
		//determine best greedy legal move
		int i, best;
		if (color == WHITE)
			best = -INFINITY;
		else
			best = INFINITY;
		for (i = 0; i < legalMoves->size; i++)
		{
			Move* move = getMove(legalMoves, i);
			if (color == WHITE)
			{
				//white
				if (move->boardScore > best)
					best = move->boardScore;	//update best move
			} else {
				//black
				if (move->boardScore < best)
					best = move->boardScore;	//update best move
			}
		}
		
		//choose random best move if multiple close to best
		int icount = 0;
		int* indices = (int*) malloc(sizeof(int) * legalMoves->size);
		for (i = 0; i < legalMoves->size; i++)
		{
			Move* move = getMove(legalMoves, i);
			if (abs(best - move->boardScore) <= RAND_RANGE)
				indices[icount++] = i;
		}
		
		MoveDef* m = (MoveDef*) malloc(sizeof(MoveDef));
		if (icount == 0)
			m->index = indices[0];
		else
			m->index = indices[(int) rand() % icount];
		m->value = best;
		free(indices);
		
		return m;
	} else {
		//recursive case
		//execute each move and recurse
		int i;
		
		//declare variables for the next move and an empty board
		char* nextBoard = buildNewBoard();
		MoveSet* nextLegal;
		char nextColor;
		
		//initialize variables, best holds the "best" next move
		MoveDef* best;
		best = (MoveDef*) malloc(sizeof(MoveDef));
		best->index = 0;
		if (color == WHITE)
		{
			nextColor = BLACK;
			best->value = -INFINITY;
		} else {
			nextColor = WHITE;
			best->value = INFINITY;
		}
		
		//recursively search each next legal move for the one
		//that leads to the best end case
		for (i = 0; i < legalMoves->size; i++)
		{
			cloneBoard(board, nextBoard);	//copy board
			Move* testMove = getMove(legalMoves, i);	//get move from the set of legal moves
			executeMove(nextBoard, testMove, prevMove);	//execute it
			nextLegal = getLegalMoves(nextBoard, nextColor, testMove, evaluator);	//get the next set of legal moves available
			MoveDef* m;
			if (nextLegal->size == 0)
			{
				//no legal moves available, either a checkmate or stalemate
				m = (MoveDef*) malloc(sizeof(MoveDef));
				m->index = 0;
				if (color == WHITE)
				{
					//check for stalemate
					if (kingInCheck(nextBoard, BLACK))
					{
						m->value = CHECKMATE_VALUE;	//black's king in checkmate
					} else {
						m->value = 0;	//stalemate
					}
				} else {
					//check for stalemate
					if (kingInCheck(nextBoard, WHITE))
					{
						m->value = -CHECKMATE_VALUE;	//white's king in checkmate
					} else {
						m->value = 0;	//stalemate
					}
				}
			} else {
				//recurse
				m = bestMoveHelper(nextBoard, nextColor, depth - 1, nextLegal, testMove, evaluator);
			}

			if (color == WHITE)
			{
				//white
				if (m->value > best->value)
				{
					//update current best move found
					best->index = i;
					best->value = m->value;
				}
			} else {
				//black
				if (m->value < best->value)
				{
					//update current best most found
					best->index = i;
					best->value = m->value;
				}
			}
			
			//free memory
			free(m);
			destroyMoveSet(nextLegal);
		}
		
		//free memory
		free(nextBoard);
		
		//restore castling variables
		whiteKCastleInvalid = tmpWKC;
		whiteQCastleInvalid = tmpWQC;
		blackKCastleInvalid = tmpBKC;
		blackQCastleInvalid = tmpBQC;
		
		//return best index
		return best;
	}
}

/* Create a new move struct. */
Move* newMove()
{
	Move* m = (Move*) malloc(sizeof(Move));
	m->pieceId = EMPTY;
	m->startRow = -1;
	m->endRow = -1;
	m->startCol = -1;
	m->endCol = -1;
	m->boardScore = 0;
}

/* Create a new move struct. */
Move* constructMove(char pieceId, char startRow, char startCol, char endRow, char endCol, int boardScore)
{
	Move* m = (Move*) malloc(sizeof(Move));
	m->pieceId = pieceId;
	m->startRow = startRow;
	m->endRow = endRow;
	m->startCol = startCol;
	m->endCol = endCol;
	m->boardScore = boardScore;
	return m;
}

/* Clone a move struct. */
Move* cloneMove(Move* m)
{
	Move* n = (Move*) malloc(sizeof(Move));
	n->pieceId = m->pieceId;
	n->startRow = m->startRow;
	n->startCol = m->startCol;
	n->endRow = m->endRow;
	n->endCol = m->endCol;
	n->boardScore = m->boardScore;
	return n;
}

/*
 * Returns a subset of a MoveSet structure.  Starts at
 * the move indexed by "start" and pulls out "length"
 * moves.
 */
MoveSet* subMoveSet(MoveSet* set, int start, int length)
{
	int i, j;
	MoveSet* s = (MoveSet*) malloc(sizeof(MoveSet));
	s->size = length;
	s->cap = length;
	s->moves = (Move**) malloc(sizeof(Move*) * length);
	
	for (j = 0, i = start; j < length; i++, j++)
		s->moves[j] = set->moves[i];
	return s;
}

/* Send part of a MoveSet structure to another node. */
void sendMoveSet(int receiver, MoveSet* set)
{
	//Send set size
	int i;
	int size = set->size;
	COMM_WORLD.Send(&size, 1, MPI_INT, receiver, 0);
	
	//Send move data
	for (i = 0; i < size; i++)
	{
		COMM_WORLD.Send(&set->moves[i]->pieceId, 1, MPI_CHAR, receiver, 0);
		COMM_WORLD.Send(&set->moves[i]->startRow, 1, MPI_CHAR, receiver, 0);
		COMM_WORLD.Send(&set->moves[i]->startCol, 1, MPI_CHAR, receiver, 0);
		COMM_WORLD.Send(&set->moves[i]->endRow, 1, MPI_CHAR, receiver, 0);
		COMM_WORLD.Send(&set->moves[i]->endCol, 1, MPI_CHAR, receiver, 0);
		COMM_WORLD.Send(&set->moves[i]->boardScore, 1, MPI_INT, receiver, 0);
	}
}

/* Receive a MoveSet structure from the head node. */
MoveSet* receiveMoveSet()
{
	//Get set size
	int i;
	int size;
	COMM_WORLD.Recv(&size, 1, MPI_INT, HEAD, 0);
	
	//Allocate MoveSet structure
	MoveSet* set = (MoveSet*) malloc(sizeof(MoveSet));
	set->size = size;
	set->cap = size;
	set->moves  = (Move**) malloc(sizeof(Move*) * size);
	
	//Receive move data
	for (i = 0; i < size; i++)
	{
		Move* m = (Move*) malloc(sizeof(Move));
		COMM_WORLD.Recv(&m->pieceId, 1, MPI_CHAR, HEAD, 0);
		COMM_WORLD.Recv(&m->startRow, 1, MPI_CHAR, HEAD, 0);
		COMM_WORLD.Recv(&m->startCol, 1, MPI_CHAR, HEAD, 0);
		COMM_WORLD.Recv(&m->endRow, 1, MPI_CHAR, HEAD, 0);
		COMM_WORLD.Recv(&m->endCol, 1, MPI_CHAR, HEAD, 0);
		COMM_WORLD.Recv(&m->boardScore, 1, MPI_INT, HEAD, 0);
		set->moves[i] = m;
	}
	
	//Return MoveSet
	return set;
}

/* Sends best move to head node. */
void sendMoveDef(MoveDef* best)
{
	int bestIndex = best->index;
	int bestValue = best->value;
	COMM_WORLD.Send(&bestIndex, 1, MPI_INT, HEAD, 0);
	COMM_WORLD.Send(&bestValue, 1, MPI_INT, HEAD, 0);
}

/* Receive best move from slave node. */
MoveDef* receiveMoveDef(int sender)
{
	int bestIndex;
	int bestValue;
	COMM_WORLD.Recv(&bestIndex, 1, MPI_INT, sender, 0);
	COMM_WORLD.Recv(&bestValue, 1, MPI_INT, sender, 0);
	MoveDef* best = (MoveDef*) malloc(sizeof(MoveDef));
	best->index = bestIndex;
	best->value = bestValue;
	return best;
}

/* Send a Move structure to a slave node. */
void sendMove(int receiver, Move* move)
{
	COMM_WORLD.Send(&move->pieceId, 1, MPI_CHAR, receiver, 0);
	COMM_WORLD.Send(&move->startRow, 1, MPI_CHAR, receiver, 0);
	COMM_WORLD.Send(&move->startCol, 1, MPI_CHAR, receiver, 0);
	COMM_WORLD.Send(&move->endRow, 1, MPI_CHAR, receiver, 0);
	COMM_WORLD.Send(&move->endCol, 1, MPI_CHAR, receiver, 0);
	COMM_WORLD.Send(&move->boardScore, 1, MPI_INT, receiver, 0);
}

/* Receive a move structure from the head node. */
Move* receiveMove()
{
	Move* m = (Move*) malloc(sizeof(Move));
	COMM_WORLD.Recv(&m->pieceId, 1, MPI_CHAR, HEAD, 0);
	COMM_WORLD.Recv(&m->startRow, 1, MPI_CHAR, HEAD, 0);
	COMM_WORLD.Recv(&m->startCol, 1, MPI_CHAR, HEAD, 0);
	COMM_WORLD.Recv(&m->endRow, 1, MPI_CHAR, HEAD, 0);
	COMM_WORLD.Recv(&m->endCol, 1, MPI_CHAR, HEAD, 0);
	COMM_WORLD.Recv(&m->boardScore, 1, MPI_INT, HEAD, 0);
	return m;
}
