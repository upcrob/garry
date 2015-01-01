/*
 * chess.c
 * Main chess program file.
 * by Rob Upcraft
 */

// Includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "stddef.h"
#include "strutil.h"
#include "boardutil.h"
#include "moves.h"
#include <mpi.h>

//General defines
#define MQ_SIZE 20
#define MAX_REPEATS 3
#define HUMAN 0
#define AI 1

//Setup defines
#define SEARCH_DEPTH 4
#define TYPE_WHITE HUMAN
#define TYPE_BLACK AI
#define FUNCTION_WHITE TURING
#define FUNCTION_BLACK TURING

//Use MPI namespace
using namespace MPI;

//Global variables
Move* prev;
int proc;
int numProcs;
Move** moveQueue;	// Queue for storing previously executed moves
int queueIndex;		// Current position in array queue
int queueFull;		// Whether queue has been filled yet

//Prototypes
Move* strToMove(char* str);
void printScore(int score);
int movesEqual(Move* move1, Move* move2);
char toUpperCase(int c);
char getColumnChar(int col);
void initLog(const char* filename);
void logMove(const char* filename, Move* move);
int computeMove(int userType, char* board, MoveSet* moves, int colorFlag, int evaluator);
void sendKillSig();
void enqueueMove(Move* move);
int moveLoopDetected();

/*
 * Main function.
 */
int main(int argc, char* argv[])
{	
	//Initialize MPI
	Init(argc, argv);
	proc = COMM_WORLD.Get_rank();
	numProcs = COMM_WORLD.Get_size();

	//Require at least 1 slave node to run
	if (numProcs < 2)
	{
		if (proc == HEAD)
			printf("Program requires at least 1 slave node to run.\nTerminating.\n");
		Finalize();
		return 0;
	}

	//Divide between head and slave node program
	if (proc == HEAD)
	{
		//Head node code
		//======================================================================
		
		//initialize variables
		queueIndex = 0;
		queueFull = FALSE;
		moveQueue = (Move**) malloc(sizeof(Move*) * MQ_SIZE);
		int colorFlag = WHITE;
		MoveSet* moves;
		char* board = buildNewBoard();
		
		//setup empty previous move
		prev = newMove();
	
		//resume play from log file if requested
		if (streq(argv[argc - 1], (char*) "-r"))
		{
			printf("\n* * RESUMING GAME FROM LOG * *\n\n");
			
			int i = 0;
			char buffer[6];
			
			FILE* fp = fopen("chesslog.txt", "r");
			int c = fgetc(fp);
			while (c != EOF)
			{
				buffer[i++] = (char) c;
				if (c == '\n')
				{
					//Move read, execute it
					buffer[i] = '\0';
					Move* move = strToMove(buffer);
					if (move->startRow == CASTLE)
					{
						//Set player that is castling
						if (colorFlag == WHITE)
							move->pieceId = KING;
						else
							move->pieceId = KING + BLACK;
					}
					executeMove(board, move, prev);
					free(move);
					
					//Switch players
					if (colorFlag == WHITE)
						colorFlag = BLACK;
					else
						colorFlag = WHITE;
					i = 0;
				}
				c = fgetc(fp);
			}
			fclose(fp);
		} else {
			//initialize (clear) log file
			initLog("chesslog.txt");
		}
		
		//show board
		printBoard(board);
		printf("\n");
		
		//main game loop
		int i, j;
		while (TRUE)
		{
			if (colorFlag == WHITE)
				moves = getLegalMoves(board, WHITE, prev, 1);
			else
				moves = getLegalMoves(board, BLACK, prev, 1);

			//check for checkmate/stalemate
			if (moves->size == 0)
			{
				if (!kingInCheck(board, WHITE) && !kingInCheck(board, BLACK))
				{
					//stalemate
					printf("STALEMATE.\n");
				} else {
					//checkmate, determine color
					if (colorFlag == WHITE)
						printf("CHECKMATE: BLACK WINS.\n");
					else
						printf("CHECKMATE: WHITE WINS.\n");
				}
				
				//End program
				sendKillSig();
				free(board);
				Finalize();
				return 0;
			}

			//execute turns (ai or human)
			int status;
			if (colorFlag == WHITE)
			{
				//white's turn
				status = computeMove(TYPE_WHITE, board, moves, WHITE, FUNCTION_WHITE);		
				if (status == 0)
				{
					Finalize();
					return 0;
				}
			} else {
				//black's turn
				status = computeMove(TYPE_BLACK, board, moves, BLACK, FUNCTION_BLACK);
				if (status == 0)
				{
					Finalize();
					return 0;
				}
			}
			destroyMoveSet(moves);

			//Add move to queue
			Move* prevClone = cloneMove(prev);
			enqueueMove(prevClone);
			
			//Check queue for repeats
			if (moveLoopDetected())
			{
				printf("REPEATED MOVES DETECTED. GAME ENDS IN A DRAW.\n");
				sendKillSig();
				free(board);
				Finalize();
				return 0;
				
			}
			
			//Log move
			logMove("chesslog.txt", prev);
			
			//Swap turns
			if (colorFlag == WHITE)
				colorFlag = BLACK;
			else
				colorFlag = WHITE;
		}

		//free memory
		free(board);
		//======================================================================
	} else {
		//Slave node code
		//======================================================================
		int color;
		int depth;
		int evaluator;
		char board[64];
		while (TRUE)
		{
			//Receive data from HEAD node
			COMM_WORLD.Recv(&color, 1, MPI_INT, HEAD, 0);
			
			//Terminate program if color is -1
			if (color == -1)
				break;
			
			COMM_WORLD.Recv(&depth, 1, MPI_INT, HEAD, 0);
			COMM_WORLD.Recv(&evaluator, 1, MPI_INT, HEAD, 0);
			COMM_WORLD.Recv(&board, 64, MPI_CHAR, HEAD, 0);
			MoveSet* set = receiveMoveSet();
			Move* prevMove = receiveMove();
			
			//Determine best move
			MoveDef* bmd = bestMoveHelper(board, color, depth, set, prevMove, evaluator);
			
			//Send MoveDef back to head node
			sendMoveDef(bmd);
			
			//Free memory
			destroyMoveSet(set);
			free(bmd);
			free(prevMove);
		}
		//======================================================================
	}
	
	//Finalize MPI
	Finalize();
	return 0;
}

/*
 * Sends a signal to other processors instructing
 * them to terminate execution.
 */
void sendKillSig()
{
	int i;
	int sig = -1;
	for (i = 1; i < numProcs; i++) 
		COMM_WORLD.Send(&sig, 1, MPI_INT, i, 0);
}

/*
 * Add a move to the move queue.
 */
void enqueueMove(Move* move)
{
	if (queueFull == TRUE)
		free(moveQueue[queueIndex]);
	
	moveQueue[queueIndex++] = move;
	if (queueIndex >= MQ_SIZE)
	{
		queueFull = TRUE;
		queueIndex = 0;
	}
}

/*
 * Check for repeats in the move queue.
 */
int moveLoopDetected()
{
	// setup variables
	int i, j, type, len, dupCount;
	int duplicates[MQ_SIZE];
	if (queueFull == TRUE)
		len = MQ_SIZE;
	else
		len = queueIndex;
	
	// clear duplicate array
	for (i = 0; i < len; i++)
		duplicates[i] = 0;
	
	// check for duplicates
	type = 1;
	for (i = 0; i < len; i++)
	{
		// go to next move if this has been classified
		if (duplicates[i] != 0)
			continue;
		
		// classify
		duplicates[i] = type;
		
		// classify next moves
		for (j = i + 1; j < len; j++)
		{
			// skip if classified
			if (duplicates[j] != 0)
				continue;
		
			Move* a = moveQueue[i];
			Move* b = moveQueue[j];
			if (a->pieceId == b->pieceId && a->startRow == b->startRow &&
				a->startCol == b->startCol && a->endRow == b->endRow &&
				a->endCol == b->endCol)
			{
				// moves were the same
				duplicates[j] = type;
			}
		}
				
		// next type
		type++;
	}
	
	// count duplicates
	int playerDup = 0;
	for (i = 1; i <= type; i++)
	{
		dupCount = 0;
		for (j = 0; j < len; j++)
		{
			if (duplicates[j] == i)
				dupCount++;
		}
		
		if (dupCount > MAX_REPEATS)
		{
			playerDup++;
			if (playerDup > 1)
				return TRUE;
		}
	}
	
	return FALSE;
}

/*
 * Carry out a move. Either get input from a human user or have AI compute move.
 */
int computeMove(int userType, char* board, MoveSet* moves, int colorFlag, int evaluator)
{
	int i;
	if (userType == HUMAN)
	{
		//Human move
		Move* move;
		char text[10];
		
		while (TRUE)
		{
			printf("ENTER MOVE: ");
			fflush(stdout);
			fgets(text, sizeof(text), stdin);
			
			//Check for end program request
			if (streq(text, (char*) "q\n"))
			{
				//Send termination signal to slave nodes
				sendKillSig();
				
				//Exit
				printf("Program Terminated.\n");
				free(board);
				return 0;
			}
			
			move = strToMove(text);
			if (move != NULL)
			{
				int legalFlag = FALSE;
				//determine if move was legal
				for (i = 0; i < moves->size; i++)
				{
					if (movesEqual(getMove(moves, i), move))
					{
						legalFlag = TRUE;
						break;
					}
				}
				
				if (legalFlag == TRUE)
				{
					if (move->startRow == CASTLE)
						move->pieceId = KING;
					else
						move->pieceId = getPieceAt(board, move->startRow, move->startCol);
					move->boardScore = evaluateBoard(board, 1);
					printf("\n");
					break;
				} else {
					printf("ILLEGAL MOVE!\n");
				}
			} else {
				printf("INVALID INPUT!\n");
			}
		}
		srand((unsigned int) time(NULL));
		
		//execute move
		executeMove(board, move, prev);
		
		//set previous move to current
		free(prev);
		prev = cloneMove(move);
		
		//print output
		printBoard(board);
		printf("\n");
		printMove(move);
		printScore(evaluateBoard(board, evaluator));
		printf("\n");
	} else {
		//AI move
		Move* best;
		
		//determine best move, increase search depth if fewer pieces on board
		if (totalPieces(board) > 9)
			best = bestMove(board, colorFlag, prev, SEARCH_DEPTH, evaluator);
		else if (totalPieces(board) > 5)
			best = bestMove(board, colorFlag, prev, SEARCH_DEPTH + 1, evaluator);
		else
			best = bestMove(board, colorFlag, prev, SEARCH_DEPTH + 2, evaluator);
		
		if (best == NULL)
		{
			printf("CHECKMATE: WHITE WINS.\n");
			
			//End program
			sendKillSig();
			free(board);
			return 0;
		} else {
			//execute best move
			executeMove(board, best, prev);
			
			//set previous move to current
			free(prev);
			prev = cloneMove(best);
			
			//print output
			printBoard(board);
			printf("\n");
			printMove(best);
			printScore(evaluateBoard(board, evaluator));
			printf("\n");
		}
	}

	return 1;
}


/*
 * Determine if two moves are equal.
 */
int movesEqual(Move* move1, Move* move2)
{
	if (move1->startRow == move2->startRow && move1->startCol == move2->startCol &&
		move1->endRow == move2->endRow && move1->endCol == move2->endCol)
		return TRUE;
	else
		return FALSE;
}

/*
 * Convert string to move.
 */
Move* strToMove(char* str)
{
	if (toUpperCase(str[0]) == 'C' && toUpperCase(str[1]) == 'K') {
		//kingside castle
		Move* move = (Move*) malloc(sizeof(Move));
		move->startRow = CASTLE;
		move->startCol = CASTLE;
		move->endRow = KINGSIDE;
		move->endCol = KINGSIDE;
		
		move->pieceId = EMPTY;
		move->boardScore = 0;
		
		return move;
	} else if (toUpperCase(str[0]) == 'C' && toUpperCase(str[1]) == 'Q') {
		printf("CASTLE QUEEN\n");
		Move* move = (Move*) malloc(sizeof(Move));
		move->startRow = CASTLE;
		move->startCol = CASTLE;
		move->endRow = QUEENSIDE;
		move->endCol = QUEENSIDE;
		
		move->pieceId = EMPTY;
		move->boardScore = 0;
		
		return move;
	} else if (strlen(str) < 4) {
		return NULL;
	} else {
		int startRow, startCol, endRow, endCol;
		char startc, endc;
		startc = toUpperCase(str[0]);
		endc = toUpperCase(str[2]);
		switch (startc)
		{
			case 'A':
				startCol = 0;
				break;
			case 'B':
				startCol = 1;
				break;
			case 'C':
				startCol = 2;
				break;
			case 'D':
				startCol = 3;
				break;
			case 'E':
				startCol = 4;
				break;
			case 'F':
				startCol = 5;
				break;
			case 'G':
				startCol = 6;
				break;
			case 'H':
				startCol = 7;
				break;
			default:
				return NULL;
		}

		startRow = str[1] - 48 - 1;
		if (startRow > 7 || startRow < 0)
			return NULL;

		switch (endc)
		{
			case 'A':
				endCol = 0;
				break;
			case 'B':
				endCol = 1;
				break;
			case 'C':
				endCol = 2;
				break;
			case 'D':
				endCol = 3;
				break;
			case 'E':
				endCol = 4;
				break;
			case 'F':
				endCol = 5;
				break;
			case 'G':
				endCol = 6;
				break;
			case 'H':
				endCol = 7;
				break;
			default:
				return NULL;
		}

		endRow = str[3] - 48 - 1;
		if (endRow > 7 || endRow < 0)
			return NULL;

		Move* move = (Move*) malloc(sizeof(Move));
		move->startRow = (char) startRow;
		move->startCol = (char) startCol;
		move->endRow = (char) endRow;
		move->endCol = (char) endCol;

		move->pieceId = EMPTY;
		move->boardScore = 0;

		return move;
	}
}

/*
 * Print a board score.
 */
void printScore(int score)
{
	if (score > 0)
		printf("Board Score = %d (WHITE).\n", score);
	else if (score < 0)
		printf("Board Score = %d (BLACK).\n", -score);
	else
		printf("Board Score = EVEN.\n");
}

/* Returns an upper case version of a character. */
char toUpperCase(int c)
{
	if (c >= 97 && c <= 122)
	{
		return (char) (c - 32);
	} else {
		return (char) c;
	}
}

/* Clears log file. */
void initLog(const char* filename)
{
	FILE* fp = fopen(filename, "w");
	fprintf(fp, "");
	fclose(fp);
}

/* Logs a textual representation of a move in a file. */
void logMove(const char* filename, Move* move)
{
	//Open file
	FILE* fp = fopen(filename, "a");
	
	//Log move
	char* moveStr = moveToString(move);
	fprintf(fp, "%s", moveStr);
	free(moveStr);
	
	//Close file
	fclose(fp);
}
